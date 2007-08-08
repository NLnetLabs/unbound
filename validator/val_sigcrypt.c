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

/** return number of rrs in an rrset */
static size_t
rrset_get_count(struct ub_packed_rrset_key* rrset)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)
	rrset->entry.data;
	if(!d) return 0;
	return d->count;
}

/**
 * Get RR signature count
 */
static size_t
rrset_get_sigcount(struct ub_packed_rrset_key* k)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)k->entry.data;
	return d->rrsig_count;
}

/**
 * Get signature keytag value
 * @param k: rrset (with signatures)
 * @param sig_idx: signature index.
 * @return keytag or 0 if malformed rrsig.
 */
static uint16_t 
rrset_get_sig_keytag(struct ub_packed_rrset_key* k, size_t sig_idx)
{
	uint16_t t;
	struct packed_rrset_data* d = (struct packed_rrset_data*)k->entry.data;
	log_assert(sig_idx < d->rrsig_count);
	if(d->rr_len[d->count + sig_idx] < 2+18)
		return 0;
	memmove(&t, d->rr_data[d->count + sig_idx]+2+16, 2);
	return t;
}

/**
 * Get signature signing algorithm value
 * @param k: rrset (with signatures)
 * @param sig_idx: signature index.
 * @return algo or 0 if malformed rrsig.
 */
static int 
rrset_get_sig_algo(struct ub_packed_rrset_key* k, size_t sig_idx)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)k->entry.data;
	log_assert(sig_idx < d->rrsig_count);
	if(d->rr_len[d->count + sig_idx] < 2+3)
		return 0;
	return (int)d->rr_data[d->count + sig_idx][2+2];
}

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

int
dnskey_get_algo(struct ub_packed_rrset_key* k, size_t idx)
{
	uint8_t* rdata;
	size_t len;
	rrset_get_rdata(k, idx, &rdata, &len);
	if(len < 2+4)
		return 0;
	return (int)rdata[2+3];
}

int
ds_get_key_algo(struct ub_packed_rrset_key* k, size_t idx)
{
	uint8_t* rdata;
	size_t len;
	rrset_get_rdata(k, idx, &rdata, &len);
	if(len < 2+3)
		return 0;
	return (int)rdata[2+2];
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

uint16_t 
ds_get_keytag(struct ub_packed_rrset_key* ds_rrset, size_t ds_idx)
{
	uint16_t t;
	uint8_t* rdata;
	size_t len;
	rrset_get_rdata(ds_rrset, ds_idx, &rdata, &len);
	if(len < 2+2)
		return 0;
	memmove(&t, rdata+2, 2);
	return t;
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
ds_digest_algo_is_supported(struct ub_packed_rrset_key* ds_rrset, 
	size_t ds_idx)
{
	return (ds_digest_size_algo(ds_rrset, ds_idx) != 0);
}

/** return true if DNSKEY algorithm id is supported */
static int
dnskey_algo_id_is_supported(int id)
{
	switch(id) {
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

int 
ds_key_algo_is_supported(struct ub_packed_rrset_key* ds_rrset, 
	size_t ds_idx)
{
	return dnskey_algo_id_is_supported(ds_get_key_algo(ds_rrset, ds_idx));
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
	return dnskey_algo_id_is_supported(dnskey_get_algo(dnskey_rrset, 
		dnskey_idx));
}

enum sec_status 
dnskeyset_verify_rrset(struct module_env* env, struct val_env* ve,
        struct ub_packed_rrset_key* rrset, struct ub_packed_rrset_key* dnskey)
{
	enum sec_status sec;
	size_t i, num;
	num = rrset_get_sigcount(rrset);
	if(num == 0) {
		verbose(VERB_ALGO, "rrset failed to verify due to a lack of "
			"signatures");
		return sec_status_bogus;
	}
	for(i=0; i<num; i++) {
		sec = dnskeyset_verify_rrset_sig(env, ve, rrset, dnskey, i);
		if(sec == sec_status_secure)
			return sec;
	}
	verbose(VERB_ALGO, "rrset failed to verify: all signatures are bogus");
	return sec_status_bogus;
}

enum sec_status 
dnskey_verify_rrset(struct module_env* env, struct val_env* ve,
        struct ub_packed_rrset_key* rrset, struct ub_packed_rrset_key* dnskey,
	        size_t dnskey_idx)
{
	enum sec_status sec;
	size_t i, num;
	num = rrset_get_sigcount(rrset);
	if(num == 0) {
		verbose(VERB_ALGO, "rrset failed to verify due to a lack of "
			"signatures");
		return sec_status_bogus;
	}
	for(i=0; i<num; i++) {
		sec = dnskey_verify_rrset_sig(env, ve, rrset, dnskey, 
			dnskey_idx, i);
		if(sec == sec_status_secure)
			return sec;
	}
	verbose(VERB_ALGO, "rrset failed to verify: all signatures are bogus");
	return sec_status_bogus;
}

enum sec_status 
dnskeyset_verify_rrset_sig(struct module_env* env, struct val_env* ve,
        struct ub_packed_rrset_key* rrset, struct ub_packed_rrset_key* dnskey,
	        size_t sig_idx)
{
	/* find matching keys and check them */
	enum sec_status sec = sec_status_bogus;
	uint16_t tag = rrset_get_sig_keytag(rrset, sig_idx);
	int algo = rrset_get_sig_algo(rrset, sig_idx);
	size_t i, num = rrset_get_count(dnskey);
	size_t numchecked = 0;
	
	for(i=0; i<num; i++) {
		/* see if key matches keytag and algo */
		if(algo != dnskey_get_algo(dnskey, i) ||
			tag != dnskey_calc_keytag(dnskey, i))
			continue;

		numchecked ++;
		/* see if key verifies */
		sec = dnskey_verify_rrset_sig(env, ve, rrset, dnskey, 
			i, sig_idx);
		if(sec == sec_status_secure)
			return sec;
	}
	if(numchecked == 0) {
		verbose(VERB_ALGO, "could not find appropriate key");
		return sec_status_bogus;
	}
	return sec_status_bogus;
}

enum sec_status 
dnskey_verify_rrset_sig(struct module_env* env, struct val_env* ve,
        struct ub_packed_rrset_key* rrset, struct ub_packed_rrset_key* dnskey,
	        size_t dnskey_idx, size_t sig_idx)
{
	/* verify as many fields in rrsig as possible */
	/* verify key dname == sig signer name */
	/* verify covered type */
	/* verify keytag and sig algo (possibly again) */
	/* verify labels is in a valid range */
	/* original ttl, always ok */
	/* verify inception, expiration dates */

	/* create rrset canonical format in buffer, ready for signature */

	/* verify */
	return sec_status_unchecked;
}
