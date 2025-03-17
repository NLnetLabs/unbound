/*
 * util/tsig.c - utility functions for TSIG.
 *
 * Copyright (c) 2025, NLnet Labs. All rights reserved.
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

#include "util/tsig.h"
#include "util/log.h"
#include "sldns/pkthdr.h"
#include "sldns/rrdef.h"
#include "util/data/msgparse.h"
#include "util/data/dname.h"
#include <openssl/evp.h>


/** 
 * Verify pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, an error code otherwise.
 */
int
tsig_verify(sldns_buffer* pkt, const uint8_t* name, size_t name_len,
		const uint8_t* secret, size_t secret_len)
{
	size_t   n_rrs;
	uint16_t rdlength;
	uint8_t  mac[1024];
	uint16_t mac_size;
	uint8_t  hmac_result[1024];
	size_t   hmac_result_len;
	size_t   pos;
	uint16_t other_len;
	size_t   algname_size;
	const EVP_MD* digester;

	assert(LDNS_QDCOUNT(sldns_buffer_begin(pkt)) == 1);

	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1) {
		log_err("No TSIG found (not in ARCOUNT)");
		return -1;
	}
	LDNS_ARCOUNT_SET( sldns_buffer_begin(pkt)
	                , LDNS_ARCOUNT(sldns_buffer_begin(pkt)) - 1);
	n_rrs = LDNS_ANCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_NSCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_ARCOUNT(sldns_buffer_begin(pkt));

	sldns_buffer_rewind(pkt);
	sldns_buffer_skip(pkt, LDNS_HEADER_SIZE);
	pkt_dname_len(pkt); /* skip qname */
	sldns_buffer_skip(pkt, 2 * sizeof(uint16_t));
	if(sldns_buffer_remaining(pkt) < sizeof(uint16_t))
		return -1;
	if(!skip_pkt_rrs(pkt, n_rrs))
		return -1;
	pkt_dname_len(pkt); /* skip TSIG name (TODO, compare with name) */
	pos = sldns_buffer_position(pkt); /* Append pos */
	if(sldns_buffer_read_u16(pkt) != LDNS_RR_TYPE_TSIG) {
		log_err("No TSIG found!");
		return -1;
	}
	sldns_buffer_skip(pkt, sizeof(uint16_t) + sizeof(uint32_t)); /* skip class + TTL */
	rdlength = sldns_buffer_read_u16(pkt);
	algname_size = pkt_dname_len(pkt);
	sldns_buffer_skip(pkt, 8); /* skip time signed and fudge */
	mac_size = sldns_buffer_read_u16(pkt);
	memcpy(mac, sldns_buffer_current(pkt), mac_size);
	sldns_buffer_skip(pkt, mac_size);
	/* Set ID to original ID */
	sldns_buffer_write_u16_at(pkt, 0, sldns_buffer_read_u16(pkt));
	sldns_buffer_skip(pkt, sizeof(uint16_t)); /* error */
	other_len = sldns_buffer_read_u16(pkt);

	/* move CLASS and TTL -> TYPE position */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, pos + 2)
	       , sizeof(uint16_t) + sizeof(uint32_t));
	pos +=   sizeof(uint16_t) + sizeof(uint32_t);

	/* Append Algorithm Name, Time signed, Fudge */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, pos + 4)
	       , algname_size + 6 /* sizeof(uint48_t) */ + sizeof(uint16_t));
	pos +=   algname_size + 6 /* sizeof(uint48_t) */ + sizeof(uint16_t);

	/* Append Error, Other Len, Other Data */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, sldns_buffer_position(pkt) - 4)
	       , 4 + other_len);
	pos +=   4 + other_len;

	digester = EVP_sha256();
	hmac_result_len = sizeof(hmac_result);
	HMAC( digester, secret, secret_len, sldns_buffer_begin(pkt), pos
	    , &hmac_result, &hmac_result_len);
	if(memcmp(mac, hmac_result, hmac_result_len) == 0) {
		log_err("Correct TSIG found (YAY!), MAC size: %d", mac_size);
		return 0;
	}
	log_err("TSIG verification failed!");
	return -1;
}

