/*
 * util/edns.c - handle base EDNS options.
 *
 * Copyright (c) 2018, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains functions for base EDNS options.
 */

#include "config.h"
#include "util/edns.h"
#include "util/config_file.h"
#include "util/netevent.h"
#include "util/regional.h"
#include "util/data/msgparse.h"
#include "util/data/msgreply.h"
#include "sldns/sbuffer.h"

#ifdef HAVE_SSL
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#endif

static int edns_keepalive(struct edns_data* edns_out, struct edns_data* edns_in,
		struct comm_point* c, struct regional* region)
{
	if(c->type == comm_udp)
		return 1;

	/* To respond with a Keepalive option, the client connection
	 * must have received one message with a TCP Keepalive EDNS option,
	 * and that option must have 0 length data. Subsequent messages
	 * sent on that connection will have a TCP Keepalive option.
	 */
	if(c->tcp_keepalive ||
		edns_opt_list_find(edns_in->opt_list, LDNS_EDNS_KEEPALIVE)) {
		int keepalive = c->tcp_timeout_msec / 100;
		uint8_t data[2];
		data[0] = (uint8_t)((keepalive >> 8) & 0xff);
		data[1] = (uint8_t)(keepalive & 0xff);
		if(!edns_opt_list_append(&edns_out->opt_list, LDNS_EDNS_KEEPALIVE,
			sizeof(data), data, region))
			return 0;
		c->tcp_keepalive = 1;
	}
	return 1;
}

int siphash(const uint8_t *in, const size_t inlen,
		const uint8_t *k, uint8_t *out, const size_t outlen);

static int aes_server_cookie(uint8_t *server_cookie_out,
		const uint8_t *secret, size_t secret_len, const uint8_t *in)
{
#ifndef HAVE_SSL
	(void)server_cookie_out;
	(void)secret;
	(void)secret_len;
	(void)in;
	return 0;
#else
# if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
	EVP_CIPHER_CTX evp_ctx_spc;
	EVP_CIPHER_CTX *evp_ctx = &evp_ctx_spc;
# else
	EVP_CIPHER_CTX *evp_ctx;
# endif
	const EVP_CIPHER *aes_ecb;
	uint8_t out[16];
	int out_len, success;

	switch(secret_len) {
	case 16: aes_ecb = EVP_aes_128_ecb(); break;
	case 24: aes_ecb = EVP_aes_192_ecb(); break;
	case 32: aes_ecb = EVP_aes_256_ecb(); break;
	default: return 0;
	}
# if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
	EVP_CIPHER_CTX_init(evp_ctx);
# else
	if (!(evp_ctx = EVP_CIPHER_CTX_new()))
		return 0;
# endif
	if((success = EVP_EncryptInit(evp_ctx, aes_ecb, secret, NULL)
	           && EVP_EncryptUpdate(evp_ctx, out, &out_len, in, 16)
	           && out_len == 16)) {
		size_t i;

		for (i = 0; i < 8; i++)
			server_cookie_out[i] = out[i] ^ out[i + 8];
	}
# if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
	EVP_CIPHER_CTX_cleanup(evp_ctx);
# else
	EVP_CIPHER_CTX_free(evp_ctx);
# endif
	return success;
#endif
}

/** RFC 1918 comparison, uses unsigned integers, and tries to avoid
 * compiler optimization (eg. by avoiding a-b<0 comparisons),
 * this routine matches compare_serial(), for SOA serial number checks */
static int
compare_1918(uint32_t a, uint32_t b)
{
	/* for 32 bit values */
	const uint32_t cutoff = ((uint32_t) 1 << (32 - 1));

	if (a == b) {
		return 0;
	} else if ((a < b && b - a < cutoff) || (a > b && a - b > cutoff)) {
		return -1;
	} else {
		return 1;
	}
}

/** if we know that b is larger than a, return the difference between them,
 * that is the distance between them. in RFC1918 arith */
static uint32_t
subtract_1918(uint32_t a, uint32_t b)
{
	/* for 32 bit values */
	const uint32_t cutoff = ((uint32_t) 1 << (32 - 1));

	if(a == b)
		return 0;
	if(a < b && b - a < cutoff) {
		return b-a;
	}
	if(a > b && a - b > cutoff) {
		return ((uint32_t)0xffffffff) - (a-b-1);
	}
	/* wrong case, b smaller than a */
	return 0;
}


int edns_cookie_validate(struct config_file* cfg,
		struct edns_option* cookie_opt, time_t now)
{
	uint8_t server_cookie[8];
	uint32_t cookie_time, now_uint32 = now;

	/* We support only draft-sury-toorop-dns-cookies-algorithms
	 * sized cookies
	 */
	if(cookie_opt->opt_len != 24)
		return 0;

	if(cookie_opt->opt_data[8] != 1)
		return 0;

	cookie_time =
		(time_t)ntohl(sldns_read_uint32(cookie_opt->opt_data + 12));

	if(compare_1918(now_uint32, cookie_time) > 0) {
		if (subtract_1918(cookie_time, now_uint32) > 3600)
			return 0; /* Not older than 1 hour */

	} else if (subtract_1918(now_uint32, cookie_time) > 300)
		/* ignore cookies > 5 minutes in future */
		return 0;

	switch(cookie_opt->opt_data[9]) {
	case 3:	if (aes_server_cookie(server_cookie, cfg->cookie_secret,
				cfg->cookie_secret_len, cookie_opt->opt_data))
			break;
		else	return 0;
	case 4:	if(cfg->cookie_secret_len != 8)
			return 0;
		siphash(cookie_opt->opt_data, 16,
				cfg->cookie_secret, server_cookie, 8);
		break;
	default:
		return 0;
	}
	return memcmp(cookie_opt->opt_data + 16, server_cookie, 8) == 0;
}

static int edns_cookie(struct edns_data* edns_out, struct edns_data* edns_in,
	struct config_file* cfg, struct comm_point* c, time_t now,
	struct regional* region)
{
	struct edns_option *opt;

	if(c->type != comm_udp)
		return 1;

	if(!cfg->do_answer_cookie)
		return 1;

	opt = edns_opt_list_find(edns_in->opt_list, LDNS_EDNS_COOKIE);
	if (opt && opt->opt_len >= 8) {
		uint8_t data_out[24];

		log_assert(cfg->cookie_secret_len >= 8);

		(void)memcpy(data_out, opt->opt_data, 8);
		data_out[ 8] = 1;   /* Version */
		                    /* Algorithm, 3 = AES, 4 = SipHash2.4 */
		data_out[ 9] = cfg->cookie_secret_len == 8 ? 4 : 3;
		data_out[10] = 0;   /* Reserved */
		data_out[11] = 0;   /* Reserved */
		sldns_write_uint32(data_out + 12, htonl(time(NULL)));
		if (data_out[9] == 4)
			siphash(data_out, 16,
				cfg->cookie_secret, data_out + 16, 8);

		else if (!aes_server_cookie(data_out + 16, cfg->cookie_secret,
				cfg->cookie_secret_len, data_out))
			return 0;

		if(!edns_opt_list_append(&edns_out->opt_list, LDNS_EDNS_COOKIE,
				sizeof(data_out), data_out, region))
			return 0;
	}
	return 1;
}

int apply_edns_options(struct edns_data* edns_out, struct edns_data* edns_in,
	struct config_file* cfg, struct comm_point* c, time_t now,
	struct regional* region)
{
	if(cfg->do_tcp_keepalive &&
		!edns_keepalive(edns_out, edns_in, c, region))
		return 0;

	if(!edns_cookie(edns_out, edns_in, cfg, c, now, region))
		return 0;

	return 1;
}
