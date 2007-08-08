/*
 * validator/val_sigcrypt.c - validator signature crypto functions.
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains helper functions for the validator module.
 * The functions help with signature verification and checking, the
 * bridging between RR wireformat data and crypto calls.
 */
#include "config.h"
#include "validator/val_sigcrypt.h"
#include "util/data/msgreply.h"
#include "util/data/dname.h"
#include "util/module.h"
#include "util/region-allocator.h"

#ifndef HAVE_SSL
#error "Need SSL library to do digital signature cryptography"
#endif

/** get rdata pointer and size */
static void
rrset_get_rdata(struct ub_packed_rrset_key* k, size_t idx, uint8_t** rdata,
	size_t* len)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)k->entry.data;
	log_assert(d && idx < (d->count + d->rrsig_count));
	*rdata = d->rr_data[idx];
	*len = d->rr_len[idx];
}

/**
 * Get DNSKEY RR signature algorithm
 * @param k: DNSKEY rrset.
 * @param idx: which DNSKEY RR.
 * @return algorithm or 0 if DNSKEY too short.
 */
static int
dnskey_get_algo(struct ub_packed_rrset_key* k, size_t idx)
{
	uint8_t* rdata;
	size_t len;
	rrset_get_rdata(k, idx, &rdata, &len);
	if(len < 2+4)
		return 0;
	return (int)rdata[2+3];
}

/**
 * Get DS RR digest algorithm
 * @param k: DS rrset.
 * @param idx: which DS.
 * @return algorithm or 0 if DS too short.
 */
static int
ds_get_digest_algo(struct ub_packed_rrset_key* k, size_t idx)
{
	uint8_t* rdata;
	size_t len;
	rrset_get_rdata(k, idx, &rdata, &len);
	if(len < 2+4)
		return 0;
	return (int)rdata[2+3];
}

/**
 * Return pointer to the digest in a DS RR.
 * @param k: DS rrset.
 * @param idx: which DS.
 * @param digest: digest data is returned.
 *	on error, this is NULL.
 * @param len: length of digest is returned.
 *	on error, the length is 0.
 */
static void
ds_get_sigdata(struct ub_packed_rrset_key* k, size_t idx, uint8_t** digest,
        size_t* len)
{
	uint8_t* rdata;
	size_t rdlen;
	rrset_get_rdata(k, idx, &rdata, &rdlen);
	if(rdlen < 2+5) {
		*digest = NULL;
		*len = 0;
		return;
	}
	*digest = rdata + 2 + 4;
	*len = rdlen - 2 - 4;
}

/**
 * Return size of DS digest according to its hash algorithm.
 * @param k: DS rrset.
 * @param idx: which DS.
 * @return size in bytes of digest, or 0 if not supported. 
 */
static size_t
ds_digest_size_algo(struct ub_packed_rrset_key* k, size_t idx)
{
	switch(ds_get_digest_algo(k, idx)) {
#ifdef SHA_DIGEST_LENGTH
		case LDNS_SHA1:
			return SHA_DIGEST_LENGTH;
#endif
#ifdef SHA256_DIGEST_LENGTH
		case LDNS_SHA256:
			return SHA256_DIGEST_LENGTH;
#endif
		default: break;
	}
	return 0;
}

/**
 * Create a DS digest for a DNSKEY entry.
 *
 * @param env: module environment. Uses scratch space.
 * @param dnskey_rrset: DNSKEY rrset.
 * @param dnskey_idx: index of RR in rrset.
 * @param ds_rrset: DS rrset
 * @param ds_idx: index of RR in DS rrset.
 * @param digest: digest is returned in here (must be correctly sized).
 * @return false on error.
 */
static int
ds_create_dnskey_digest(struct module_env* env, 
	struct ub_packed_rrset_key* dnskey_rrset, size_t dnskey_idx,
	struct ub_packed_rrset_key* ds_rrset, size_t ds_idx,
	uint8_t* digest)
{
	ldns_buffer* b = env->scratch_buffer;
	uint8_t* dnskey_rdata;
	size_t dnskey_len;
	rrset_get_rdata(dnskey_rrset, dnskey_idx, &dnskey_rdata, &dnskey_len);

	/* create digest source material in buffer 
	 * digest = digest_algorithm( DNSKEY owner name | DNSKEY RDATA);
	 *	DNSKEY RDATA = Flags | Protocol | Algorithm | Public Key. */
	ldns_buffer_clear(b);
	ldns_buffer_write(b, dnskey_rrset->rk.dname, 
		dnskey_rrset->rk.dname_len);
	query_dname_tolower(ldns_buffer_begin(b), dnskey_rrset->rk.dname_len);
	ldns_buffer_write(b, dnskey_rdata+2, dnskey_len-2); /* skip rdatalen*/
	ldns_buffer_flip(b);
	
	switch(ds_get_digest_algo(ds_rrset, ds_idx)) {
#ifdef SHA_DIGEST_LENGTH
		case LDNS_SHA1:
			(void)SHA1((unsigned char*)ldns_buffer_begin(b),
				ldns_buffer_limit(b), (unsigned char*)digest);
			return 1;
#endif
#ifdef SHA256_DIGEST_LENGTH
		case LDNS_SHA256:
			(void)SHA256((unsigned char*)ldns_buffer_begin(b),
				ldns_buffer_limit(b), (unsigned char*)digest);
			return 1;
#endif
		default: break;
	}
	return 0;
}

int ds_digest_match_dnskey(struct module_env* env,
	struct ub_packed_rrset_key* dnskey_rrset, size_t dnskey_idx,
	struct ub_packed_rrset_key* ds_rrset, size_t ds_idx)
{
	uint8_t* ds;	/* DS digest */
	size_t dslen;
	uint8_t* digest; /* generated digest */
	size_t digestlen = ds_digest_size_algo(ds_rrset, ds_idx);
	
	if(digestlen == 0)
		return 0; /* not supported, or DS RR format error */
	/* check digest length in DS with length from hash function */
	ds_get_sigdata(ds_rrset, ds_idx, &ds, &dslen);
	if(!ds || dslen != digestlen)
		return 0; /* DS algorithm and digest do not match */

	digest = region_alloc(env->scratch, digestlen);
	if(!digest)
		return 0; /* mem error */
	if(!ds_create_dnskey_digest(env, dnskey_rrset, dnskey_idx, ds_rrset, 
		ds_idx, digest))
		return 0; /* digest algo failed */
	if(memcmp(digest, ds, dslen) != 0)
		return 0; /* digest different */
	return 1;
}

int 
ds_algo_is_supported(struct ub_packed_rrset_key* ds_rrset, size_t ds_idx)
{
	return (ds_digest_size_algo(ds_rrset, ds_idx) != 0);
}

uint16_t 
dnskey_calc_keytag(struct ub_packed_rrset_key* dnskey_rrset, size_t dnskey_idx)
{
	uint8_t* data;
	size_t len;
	rrset_get_rdata(dnskey_rrset, dnskey_idx, &data, &len);
	/* do not pass rdatalen to ldns */
	return ldns_calc_keytag_raw(data+2, len-2);
}

int dnskey_algo_is_supported(struct ub_packed_rrset_key* dnskey_rrset,
        size_t dnskey_idx)
{
	switch(dnskey_get_algo(dnskey_rrset, dnskey_idx)) {
		case LDNS_DSA:
		case LDNS_DSA_NSEC3:
		case LDNS_RSASHA1:
		case LDNS_RSASHA1_NSEC3:
		case LDNS_RSAMD5:
			return 1;
		default:
			return 0;
	}
}
