/*
 * util/tsig.c - handle TSIG signatures.
 *
 * Copyright (c) 2023, NLnet Labs. All rights reserved.
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
 * This file contains functions for dealing with TSIG records and signatures.
 */

#include "config.h"
#include "util/tsig.h"
#include "util/log.h"
#include "sldns/pkthdr.h"
#include "sldns/rrdef.h"
#include "sldns/sbuffer.h"
#include "util/data/msgparse.h"
#include "util/data/dname.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>


/**
 * Verify pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, an error code otherwise.
 */
int
tsig_verify(sldns_buffer* pkt, const uint8_t* name, const uint8_t* alg,
		const uint8_t* secret, size_t secret_len, uint64_t now)
{
	size_t   n_rrs;
	size_t   end_of_message;
	uint16_t rdlength;
	uint8_t  mac[1024];
	uint16_t mac_sz;
	uint8_t  hmac_result[1024];
	unsigned int hmac_result_len;
	size_t   pos;
	uint16_t other_len;
	size_t   algname_size;
	uint64_t time_signed;
	uint16_t fudge;
	const EVP_MD* digester;

	assert(LDNS_QDCOUNT(sldns_buffer_begin(pkt)) == 1);

	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1) {
		log_err("No TSIG found (ARCOUNT == 0)");
		return -1;
	}
	LDNS_ARCOUNT_SET( sldns_buffer_begin(pkt)
	                , LDNS_ARCOUNT(sldns_buffer_begin(pkt)) - 1);
	n_rrs = LDNS_ANCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_NSCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_ARCOUNT(sldns_buffer_begin(pkt));

	sldns_buffer_rewind(pkt);
	sldns_buffer_skip(pkt, LDNS_HEADER_SIZE);
	pkt_dname_len(pkt);                           /* skip qname */
	sldns_buffer_skip(pkt, 2 * sizeof(uint16_t)); /* skip type and class */
	if(!skip_pkt_rrs(pkt, n_rrs))                 /* skip all rrs */
		return -1;
	end_of_message = sldns_buffer_position(pkt);
	if(query_dname_compare(name, sldns_buffer_current(pkt)))
		return LDNS_TSIG_ERROR_BADKEY;
	pkt_dname_len(pkt);                           /* skip TSIG name */
	pos = sldns_buffer_position(pkt);             /* Append pos */
	if(sldns_buffer_read_u16(pkt) != LDNS_RR_TYPE_TSIG) {
		log_err("No TSIG found!");
		return -1;
	}
	sldns_buffer_skip(pkt, sizeof(uint16_t)       /* skip class */
	                     + sizeof(uint32_t)       /* skip TTLS */
			     + sizeof(uint16_t));     /* skip rdlength */
	if(query_dname_compare(alg, sldns_buffer_current(pkt)))
		return LDNS_TSIG_ERROR_BADKEY;
	algname_size = pkt_dname_len(pkt);            /* skip alg name */
	time_signed = sldns_buffer_read_u48(pkt);     /* time signed */
	fudge = sldns_buffer_read_u16(pkt);           /* fudge */
	mac_sz = sldns_buffer_read_u16(pkt);          /* mac size */
	memcpy(mac,sldns_buffer_current(pkt),mac_sz); /* copy mac */
	sldns_buffer_skip(pkt, mac_sz);               /* skip mac */
	/* Set ID to original ID */
	sldns_buffer_write_u16_at(pkt, 0, sldns_buffer_read_u16(pkt));
	sldns_buffer_skip(pkt, sizeof(uint16_t));     /* skip error */
	other_len = sldns_buffer_read_u16(pkt);       /* other len */

	/* move CLASS (uint16_t) and TTL (uint32_t) -> TYPE position */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, pos + sizeof(uint16_t) /* type */)
	       , sizeof(uint16_t) + sizeof(uint32_t));
	pos +=   sizeof(uint16_t) + sizeof(uint32_t);

	/* Append Algorithm Name, Time signed (uint48_t), Fudge (uint16_t) */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, pos + sizeof(uint16_t) /* type */
	                                  + sizeof(uint16_t) /* rdlength */)
	       , algname_size + 6 /* sizeof(uint48_t) */ + sizeof(uint16_t));
	pos +=   algname_size + 6 /* sizeof(uint48_t) */ + sizeof(uint16_t);

	/* Append Error (uint16_t), Other Len (uint16_t), Other Data */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, sldns_buffer_position(pkt)
	                            - 2 * sizeof(uint16_t))
	       , 2 * sizeof(uint16_t) + other_len);
	pos +=   2 * sizeof(uint16_t) + other_len;

	digester = EVP_sha256();
	hmac_result_len = sizeof(hmac_result);
	HMAC( digester, secret, secret_len, sldns_buffer_begin(pkt), pos
	    , hmac_result, &hmac_result_len);
	if(CRYPTO_memcmp(mac, hmac_result, hmac_result_len) == 0) {
		return now > time_signed ?
		     ( time_signed - now > fudge ? LDNS_TSIG_ERROR_BADTIME : 0 )
		     : now - time_signed > fudge ? LDNS_TSIG_ERROR_BADTIME : 0 ;
		sldns_buffer_set_position(pkt, end_of_message);
		return 0;
	}
	return LDNS_TSIG_ERROR_BADSIG;
}
