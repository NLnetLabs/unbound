/*
 * validator/val_utils.c - validator utility functions.
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
 */
#include "config.h"
#include "validator/val_utils.h"
#include "validator/val_kentry.h"
#include "validator/val_sigcrypt.h"
#include "util/data/msgreply.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/net_help.h"

enum val_classification 
val_classify_response(struct query_info* qinf, struct reply_info* rep)
{
	int rcode = (int)FLAGS_GET_RCODE(rep->flags);
	size_t i;

	/* Normal Name Error's are easy to detect -- but don't mistake a CNAME
	 * chain ending in NXDOMAIN. */
	if(rcode == LDNS_RCODE_NXDOMAIN && rep->an_numrrsets == 0)
		return VAL_CLASS_NAMEERROR;

	log_assert(rcode == LDNS_RCODE_NOERROR);
	/* Next is NODATA */
	if(rep->an_numrrsets == 0)
		return VAL_CLASS_NODATA;
	
	/* We distinguish between CNAME response and other positive/negative
	 * responses because CNAME answers require extra processing. */

	/* We distinguish between ANY and CNAME or POSITIVE because 
	 * ANY responses are validated differently. */
	if(qinf->qtype == LDNS_RR_TYPE_ANY)
		return VAL_CLASS_ANY;
	
	/* Note that DNAMEs will be ignored here, unless qtype=DNAME. Unless
	 * qtype=CNAME, this will yield a CNAME response. */
	for(i=0; i<rep->an_numrrsets; i++) {
		if(ntohs(rep->rrsets[i]->rk.type) == qinf->qtype)
			return VAL_CLASS_POSITIVE;
		if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_CNAME)
			return VAL_CLASS_CNAME;
	}
	log_dns_msg("validator: failed to classify response message: ",
		qinf, rep);
	return VAL_CLASS_UNKNOWN;
}

/** Get signer name from RRSIG */
static void
rrsig_get_signer(uint8_t* data, size_t len, uint8_t** sname, size_t* slen)
{
	/* RRSIG rdata is not allowed to be compressed, it is stored
	 * uncompressed in memory as well, so return a ptr to the name */
	if(len < 21) {
		/* too short RRSig:
		 * short, byte, byte, long, long, long, short, "." is
		 * 2	1	1	4	4  4	2	1 = 19
		 * 			and a skip of 18 bytes to the name.
		 * +2 for the rdatalen is 21 bytes len for root label */
		*sname = NULL;
		*slen = 0;
		return;
	}
	data += 20; /* skip the fixed size bits */
	len -= 20;
	*slen = dname_valid(data, len);
	if(!*slen) {
		/* bad dname in this rrsig. */
		*sname = NULL;
		return;
	}
	*sname = data;
}

/**
 * Find the signer name for an RRset.
 * @param rrset: the rrset.
 * @param sname: signer name is returned or NULL if not signed.
 * @param slen: length of sname (or 0).
 */
static void 
val_find_rrset_signer(struct ub_packed_rrset_key* rrset, uint8_t** sname,
	size_t* slen)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)
		rrset->entry.data;
	/* return signer for first signature, or NULL */
	if(d->rrsig_count == 0) {
		*sname = NULL;
		*slen = 0;
		return;
	}
	/* get rrsig signer name out of the signature */
	rrsig_get_signer(d->rr_data[d->count], d->rr_len[d->count], 
		sname, slen);
}

void 
val_find_signer(struct query_info* qinf, struct reply_info* rep,
	uint8_t** signer_name, size_t* signer_len)
{
	enum val_classification subtype = val_classify_response(qinf, rep);
	size_t i;
	
	if(subtype == VAL_CLASS_POSITIVE || subtype == VAL_CLASS_CNAME 
		|| subtype == VAL_CLASS_ANY) {
		/* check for the answer rrset */
		for(i=0; i<rep->an_numrrsets; i++) {
			if(query_dname_compare(qinf->qname, 
				rep->rrsets[i]->rk.dname) == 0) {
				val_find_rrset_signer(rep->rrsets[i], 
					signer_name, signer_len);
				return;
			}
		}
		*signer_name = NULL;
		*signer_len = 0;
	} else if(subtype == VAL_CLASS_NAMEERROR 
		|| subtype == VAL_CLASS_NODATA) {
		/*Check to see if the AUTH section NSEC record(s) have rrsigs*/
		for(i=rep->an_numrrsets; i<
			rep->an_numrrsets+rep->ns_numrrsets; i++) {
			if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_NSEC
				|| ntohs(rep->rrsets[i]->rk.type) ==
				LDNS_RR_TYPE_NSEC3) {
				val_find_rrset_signer(rep->rrsets[i], 
					signer_name, signer_len);
				return;
			}
		}
	} else {
		verbose(VERB_ALGO, "find_signer: could not find signer name"
			" for unknown type response");
		*signer_name = NULL;
		*signer_len = 0;
	}
}

/** return number of rrs in an rrset */
static size_t
rrset_get_count(struct ub_packed_rrset_key* rrset)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)
		rrset->entry.data;
	if(!d) return 0;
	return d->count;
}

/** return TTL of rrset */
static uint32_t
rrset_get_ttl(struct ub_packed_rrset_key* rrset)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)
		rrset->entry.data;
	if(!d) return 0;
	return d->ttl;
}

enum sec_status 
val_verify_rrset(struct module_env* env, struct val_env* ve,
        struct ub_packed_rrset_key* rrset, struct ub_packed_rrset_key* keys)
{
	enum sec_status sec;
	struct packed_rrset_data* d = (struct packed_rrset_data*)rrset->
		entry.data;
	if(d->security == sec_status_secure) {
		/* re-verify all other statuses, because keyset may change*/
		log_nametypeclass(VERB_ALGO, "verify rrset cached", 
			rrset->rk.dname, ntohs(rrset->rk.type), 
			ntohs(rrset->rk.rrset_class));
		return d->security;
	}
	log_nametypeclass(VERB_ALGO, "verify rrset", rrset->rk.dname,
		ntohs(rrset->rk.type), ntohs(rrset->rk.rrset_class));
	sec = dnskeyset_verify_rrset(env, ve, rrset, keys);
	verbose(VERB_ALGO, "verify result: %s", sec_status_to_string(sec));

	/* update rrset security status 
	 * only improves security status */
	if(sec > d->security) {
		d->security = sec;
		if(sec == sec_status_secure)
			d->trust = rrset_trust_validated;
	}

	return sec;
}

enum sec_status 
val_verify_rrset_entry(struct module_env* env, struct val_env* ve,
        struct ub_packed_rrset_key* rrset, struct key_entry_key* kkey)
{
	/* temporary dnskey rrset-key */
	struct ub_packed_rrset_key dnskey;
	struct key_entry_data* kd = (struct key_entry_data*)kkey->entry.data;
	enum sec_status sec;
	dnskey.rk.type = htons(kd->rrset_type);
	dnskey.rk.rrset_class = htons(kkey->key_class);
	dnskey.rk.flags = 0;
	dnskey.rk.dname = kkey->name;
	dnskey.rk.dname_len = kkey->namelen;
	dnskey.entry.key = &dnskey;
	dnskey.entry.data = kd->rrset_data;
	sec = val_verify_rrset(env, ve, rrset, &dnskey);
	return sec;
}

/** verify that a DS RR hashes to a key and that key signs the set */
static enum sec_status
verify_dnskeys_with_ds_rr(struct module_env* env, struct val_env* ve, 
	struct ub_packed_rrset_key* dnskey_rrset, 
        struct ub_packed_rrset_key* ds_rrset, size_t ds_idx)
{
	enum sec_status sec = sec_status_bogus;
	size_t i, num;
	num = rrset_get_count(dnskey_rrset);
	for(i=0; i<num; i++) {
		/* Skip DNSKEYs that don't match the basic criteria. */
		if(ds_get_key_algo(ds_rrset, ds_idx) 
		   != dnskey_get_algo(dnskey_rrset, i)
		   || dnskey_calc_keytag(dnskey_rrset, i)
		   != ds_get_keytag(ds_rrset, ds_idx)) {
			continue;
		}
		verbose(VERB_ALGO, "attempt DS match algo %d keytag %d",
			ds_get_key_algo(ds_rrset, ds_idx),
			ds_get_keytag(ds_rrset, ds_idx));

		/* Convert the candidate DNSKEY into a hash using the 
		 * same DS hash algorithm. */
		if(!ds_digest_match_dnskey(env, dnskey_rrset, i, ds_rrset, 
			ds_idx)) {
			continue;
		}

		/* Otherwise, we have a match! Make sure that the DNSKEY 
		 * verifies *with this key*  */
		sec = dnskey_verify_rrset(env, ve, dnskey_rrset, 
			dnskey_rrset, i);
		if(sec == sec_status_secure) {
			return sec;
		}
		/* If it didn't validate with the DNSKEY, try the next one! */
	}
	return sec_status_bogus;
}

struct key_entry_key* 
val_verify_new_DNSKEYs(struct region* region, struct module_env* env, 
	struct val_env* ve, struct ub_packed_rrset_key* dnskey_rrset, 
	struct ub_packed_rrset_key* ds_rrset)
{
	/* as long as this is false, we can consider this DS rrset to be
	 * equivalent to no DS rrset. */
	int has_useful_ds = 0;
	size_t i, num;
	enum sec_status sec;

	if(dnskey_rrset->rk.dname_len != ds_rrset->rk.dname_len ||
		query_dname_compare(dnskey_rrset->rk.dname, ds_rrset->rk.dname)
		!= 0) {
		verbose(VERB_ALGO, "DNSKEY RRset did not match DS RRset "
			"by name");
		return key_entry_create_bad(region, ds_rrset->rk.dname,
			ds_rrset->rk.dname_len, 
			ntohs(ds_rrset->rk.rrset_class));
	}

	num = rrset_get_count(ds_rrset);
	for(i=0; i<num; i++) {
		/* Check to see if we can understand this DS. */
		if(!ds_digest_algo_is_supported(ds_rrset, i) ||
			!ds_key_algo_is_supported(ds_rrset, i)) {
			continue;
		}

		/* Once we see a single DS with a known digestID and 
		 * algorithm, we cannot return INSECURE (with a 
		 * "null" KeyEntry). */
		has_useful_ds = true;

		sec = verify_dnskeys_with_ds_rr(env, ve, dnskey_rrset, 
			ds_rrset, i);
		if(sec == sec_status_secure) {
			verbose(VERB_ALGO, "DS matched DNSKEY.");
			return key_entry_create_rrset(region, 
				ds_rrset->rk.dname, ds_rrset->rk.dname_len,
				ntohs(ds_rrset->rk.rrset_class), dnskey_rrset);
		}
	}

	/* None of the DS's worked out. */

	/* If no DSs were understandable, then this is OK. */
	if(!has_useful_ds) {
		verbose(VERB_ALGO, "No usable DS records were found -- "
			"treating as insecure.");
		return key_entry_create_null(region, ds_rrset->rk.dname,
			ds_rrset->rk.dname_len, 
			ntohs(ds_rrset->rk.rrset_class),
			rrset_get_ttl(ds_rrset));
	}
	/* If any were understandable, then it is bad. */
	verbose(VERB_ALGO, "Failed to match any usable DS to a DNSKEY.");
	return key_entry_create_bad(region, ds_rrset->rk.dname,
		ds_rrset->rk.dname_len, ntohs(ds_rrset->rk.rrset_class));
}

int 
val_dsset_isusable(struct ub_packed_rrset_key* ds_rrset)
{
	size_t i;
	for(i=0; i<rrset_get_count(ds_rrset); i++) {
		if(ds_digest_algo_is_supported(ds_rrset, i) &&
			ds_key_algo_is_supported(ds_rrset, i))
			return 1;
	}
	return 0;
}

/** get label count for a signature */
static uint8_t
rrsig_get_labcount(struct packed_rrset_data* d, size_t sig)
{
	if(d->rr_len[sig] < 2+4)
		return 0; /* bad sig length */
	return d->rr_data[sig][2+3];
}

int 
val_rrset_wildcard(struct ub_packed_rrset_key* rrset, uint8_t** wc)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)rrset->
		entry.data;
	uint8_t labcount;
	int labdiff;
	size_t i;
	if(d->rrsig_count == 0) {
		*wc = NULL;
		return 0;
	}
	labcount = rrsig_get_labcount(d, d->count + 0);
	/* check rest of signatures identical */
	for(i=1; i<d->rrsig_count; i++) {
		if(labcount != rrsig_get_labcount(d, d->count + i)) {
			*wc = NULL;
			return 0;
		}
	}
	/* OK the rrsigs check out */
	/* if the RRSIG label count is shorter than the number of actual 
	 * labels, then this rrset was synthesized from a wildcard.
	 * Note that the RRSIG label count doesn't count the root label. */
	labdiff = (dname_count_labels(rrset->rk.dname) - 1) - (int)labcount;
	if(labdiff > 0) {
		size_t wl = rrset->rk.dname_len;
		*wc = rrset->rk.dname;
		dname_remove_labels(wc, &wl, labdiff);
		return 1;
	}
	*wc = NULL;
	return 1;
}
