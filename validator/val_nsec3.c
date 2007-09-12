/*
 * validator/val_nsec3.c - validator NSEC3 denial of existance functions.
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
 * The functions help with NSEC3 checking, the different NSEC3 proofs
 * for denial of existance, and proofs for presence of types.
 */
#include "config.h"
#include "validator/val_nsec3.h"
#include "validator/validator.h"
#include "validator/val_kentry.h"
#include "util/region-allocator.h"
#include "util/rbtree.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/data/msgreply.h"

/**
 * The NSEC3 hash result storage.
 * Consists of an rbtree, with these nodes in it.
 * The nodes detail how a set of parameters (from nsec3 rr) plus
 * a dname result in a hash.
 */
struct nsec3_cached_hash {
	/** rbtree node, key is this structure */
	rbnode_t node;
	/** where are the parameters for conversion, in this rrset data */
	struct ub_packed_rrset_key* nsec3;
	/** where are the parameters for conversion, this RR number in data */
	int rr;
	/** the name to convert */
	uint8_t* dname;
	/** length of the dname */
	size_t dname_len;
	/** the hash result (not base32 encoded) */
	uint8_t* hash;
	/** length of hash in bytes */
	size_t hash_len;
	/** the hash result in base32 encoding */
	uint8_t* b32;
	/** length of base32 encoding (as a label) */
	size_t b32_len;
};

/**
 * Closest encloser (ce) proof results
 * Contains the ce and the next-closer (nc) proof.
 */
struct ce_response {
	/** the closest encloser name */
	uint8_t* ce;
	/** length of ce */
	size_t ce_len;
	/** NSEC3 record that proved ce. rrset */
	struct ub_packed_rrset_key* ce_rrset;
	/** NSEC3 record that proved ce. rr number */
	size_t ce_rr;
	/** NSEC3 record that proved nc. rrset */
	struct ub_packed_rrset_key* nc_rrset;
	/** NSEC3 record that proved nc. rr*/
	size_t nc_rr;
};

/**
 * Filter conditions for NSEC3 proof
 * Used to iterate over the applicable NSEC3 RRs.
 */
struct nsec3_filter {
	/** Zone name, only NSEC3 records for this zone are considered */
	uint8_t* zone;
	/** length of the zonename */
	size_t zone_len;
	/** the list of NSEC3s to filter; array */
	struct ub_packed_rrset_key** list;
	/** number of rrsets in list */
	size_t num;
	/** class of records for the NSEC3, only this class applies */
	uint16_t fclass;
};

/** return number of rrs in an rrset */
static size_t
rrset_get_count(struct ub_packed_rrset_key* rrset)
{
        struct packed_rrset_data* d = (struct packed_rrset_data*)
	        rrset->entry.data;
        if(!d) return 0;
        return d->count;
}

/** return if nsec3 RR has unknown flags */
static int
nsec3_unknown_flags(struct ub_packed_rrset_key* rrset, int r)
{
        struct packed_rrset_data* d = (struct packed_rrset_data*)
	        rrset->entry.data;
	log_assert(d && r < (int)d->count);
	if(d->rr_len[r] < 2+2)
		return 0; /* malformed */
	return (int)(d->rr_data[r][2+1] & NSEC3_UNKNOWN_FLAGS);
}

/** return nsec3 RR algorithm */
static int
nsec3_get_algo(struct ub_packed_rrset_key* rrset, int r)
{
        struct packed_rrset_data* d = (struct packed_rrset_data*)
	        rrset->entry.data;
	log_assert(d && r < (int)d->count);
	if(d->rr_len[r] < 2+1)
		return 0; /* malformed */
	return (int)(d->rr_data[r][2+0]);
}

/** return if nsec3 RR has known algorithm */
static int
nsec3_known_algo(struct ub_packed_rrset_key* rrset, int r)
{
        struct packed_rrset_data* d = (struct packed_rrset_data*)
	        rrset->entry.data;
	log_assert(d && r < (int)d->count);
	if(d->rr_len[r] < 2+1)
		return 0; /* malformed */
	switch(d->rr_data[r][2+0]) {
		case NSEC3_HASH_SHA1:
			return 1;
	}
	return 0;
}

/** return nsec3 RR iteration count */
static size_t
nsec3_get_iter(struct ub_packed_rrset_key* rrset, int r)
{
	uint16_t i;
        struct packed_rrset_data* d = (struct packed_rrset_data*)
	        rrset->entry.data;
	log_assert(d && r < (int)d->count);
	if(d->rr_len[r] < 2+4)
		return 0; /* malformed */
	memmove(&i, d->rr_data[r]+2+2, sizeof(i));
	i = ntohs(i);
	return (size_t)i;
}

/** return nsec3 RR salt */
static int
nsec3_get_salt(struct ub_packed_rrset_key* rrset, int r,
	uint8_t** salt, size_t* saltlen)
{
        struct packed_rrset_data* d = (struct packed_rrset_data*)
	        rrset->entry.data;
	log_assert(d && r < (int)d->count);
	if(d->rr_len[r] < 2+5) {
		*salt = 0;
		*saltlen = 0;
		return 0; /* malformed */
	}
	*saltlen = (size_t)d->rr_data[r][2+4];
	if(d->rr_len[r] < 2+5+(size_t)*saltlen) {
		*salt = 0;
		*saltlen = 0;
		return 0; /* malformed */
	}
	*salt = d->rr_data[r]+2+5;
	return 1;
}

/** 
 * Iterate through NSEC3 list, per RR 
 * This routine gives the next RR in the list (or sets rrset null). 
 * Usage:
 *
 * size_t rrsetnum;
 * int rrnum;
 * struct ub_packed_rrset_key* rrset;
 * for(rrset=filter_first(filter, &rrsetnum, &rrnum); rrset; 
 *	rrset=filter_next(filter, &rrsetnum, &rrnum))
 *		do_stuff;
 * 
 * Also filters out 
 * 	o unknown flag NSEC3s
 * 	o unknown algorithm NSEC3s.
 * @param filter: nsec3 filter structure.
 * @param rrsetnum: in/out rrset number to look at.
 * @param rrnum: in/out rr number in rrset to look at.
 * @returns ptr to the next rrset (or NULL at end).
 */
static struct ub_packed_rrset_key*
filter_next(struct nsec3_filter* filter, size_t* rrsetnum, int* rrnum)
{
	size_t i;
	int r;
	uint8_t* nm;
	size_t nmlen;
	if(!filter->zone) /* empty list */
		return NULL;
	for(i=*rrsetnum; i<filter->num; i++) {
		/* see if RRset qualifies */
		if(ntohs(filter->list[i]->rk.type) != LDNS_RR_TYPE_NSEC3 ||
			ntohs(filter->list[i]->rk.rrset_class) != 
			filter->fclass) 
			continue;
		/* check RRset zone */
		nm = filter->list[i]->rk.dname;
		nmlen = filter->list[i]->rk.dname_len;
		dname_remove_label(&nm, &nmlen);
		if(query_dname_compare(nm, filter->zone) != 0)
			continue;
		if(i == *rrsetnum)
			r = (*rrnum) + 1; /* continue at next RR */
		else	r = 0;		/* new RRset start at first RR */
		for(; r < (int)rrset_get_count(filter->list[i]); r++) {
			/* skip unknown flags, algo */
			if(nsec3_unknown_flags(filter->list[i], r) ||
				!nsec3_known_algo(filter->list[i], r))
				continue;
			/* this one is a good target */
			*rrsetnum = i;
			*rrnum = r;
			return filter->list[i];
		}
	}
	return NULL;
}

/**
 * Start iterating over NSEC3 records.
 * @param filter: the filter structure, must have been filter_init-ed.
 * @param rrsetnum: can be undefined on call, inited.
 * @param rrnum: can be undefined on call, inited.
 * @return first rrset of an NSEC3, together with rrnum this points to
 *	the first RR to examine. Is NULL on empty list.
 */
static struct ub_packed_rrset_key*
filter_first(struct nsec3_filter* filter, size_t* rrsetnum, int* rrnum)
{
	*rrsetnum = 0;
	*rrnum = -1;
	return filter_next(filter, rrsetnum, rrnum);
}

/** see if at least one RR is known (flags, algo) */
static int
nsec3_rrset_has_known(struct ub_packed_rrset_key* s)
{
	int r;
	for(r=0; r < (int)rrset_get_count(s); r++) {
		if(!nsec3_unknown_flags(s, r) && nsec3_known_algo(s, r))
			return 1;
	}
	return 0;
}

/** 
 * Initialize the filter structure.
 * Finds the zone by looking at available NSEC3 records and best match.
 * 	(skips the unknown flag and unknown algo NSEC3s).
 *
 * @param filter: nsec3 filter structure.
 * @param list: list of rrsets, an array of them.
 * @param num: number of rrsets in list.
 * @param qinfo: 
 *	query name to match a zone for.
 *	query type (if DS a higher zone must be chosen)
 *	qclass, to filter NSEC3s with.
 */
static void
filter_init(struct nsec3_filter* filter, struct ub_packed_rrset_key** list,
	size_t num, struct query_info* qinfo)
{
	size_t i;
	uint8_t* nm;
	size_t nmlen;
	filter->zone = NULL;
	filter->zone_len = 0;
	filter->list = list;
	filter->num = num;
	filter->fclass = qinfo->qclass;
	for(i=0; i<num; i++) {
		/* ignore other stuff in the list */
		if(ntohs(list[i]->rk.type) != LDNS_RR_TYPE_NSEC3 ||
			ntohs(list[i]->rk.rrset_class) != qinfo->qclass) 
			continue;
		/* skip unknown flags, algo */
		if(!nsec3_rrset_has_known(list[i]))
			continue;

		/* since NSECs are base32.zonename, we can find the zone
		 * name by stripping off the first label of the record */
		nm = list[i]->rk.dname;
		nmlen = list[i]->rk.dname_len;
		dname_remove_label(&nm, &nmlen);
		/* if we find a domain that can prove about the qname,
		 * and if this domain is closer to the qname */
		if(dname_subdomain_c(qinfo->qname, nm) && (!filter->zone ||
			dname_subdomain_c(nm, filter->zone))) {
			/* for a type DS do not accept a zone equal to qname*/
			if(qinfo->qtype == LDNS_RR_TYPE_DS && 
				query_dname_compare(qinfo->qname, nm) == 0)
				continue;
			filter->zone = nm;
			filter->zone_len = nmlen;
		}
	}
}

/**
 * Find max iteration count using config settings and key size
 * @param ve: validator environment with iteration count config settings.
 * @param bits: key size
 * @return max iteration count
 */
static size_t
get_max_iter(struct val_env* ve, size_t bits)
{
	int i;
	log_assert(ve->nsec3_keyiter_count > 0);
	/* round up to nearest config keysize, linear search, keep it small */
	for(i=0; i<ve->nsec3_keyiter_count; i++) {
		if(bits <= ve->nsec3_keysize[i])
			return ve->nsec3_maxiter[i];
	}
	/* else, use value for biggest key */
	return ve->nsec3_maxiter[ve->nsec3_keyiter_count-1];
}

/** 
 * Determine if any of the NSEC3 rrs iteration count is too high, from key.
 * @param ve: validator environment with iteration count config settings.
 * @param filter: what NSEC3s to loop over.
 * @param kkey: key entry used for verification; used for iteration counts.
 * @return 1 if some nsec3s are above the max iteration count.
 */
static int
nsec3_iteration_count_high(struct val_env* ve, struct nsec3_filter* filter, 
	struct key_entry_key* kkey)
{
	size_t rrsetnum;
	int rrnum;
	struct ub_packed_rrset_key* rrset;
	/* first determine the max number of iterations */
	size_t bits = key_entry_keysize(kkey);
	size_t max_iter = get_max_iter(ve, bits);
	verbose(VERB_ALGO, "nsec3: keysize %d bits, max iterations %d",
		(int)bits, (int)max_iter);

	for(rrset=filter_first(filter, &rrsetnum, &rrnum); rrset; 
		rrset=filter_next(filter, &rrsetnum, &rrnum)) {
		if(nsec3_get_iter(rrset, rrnum) > max_iter)
			return 1;
	}
	return 0;
}

/** nsec3_cache_compare for rbtree */
static int
nsec3_hash_cmp(const void* c1, const void* c2) 
{
	struct nsec3_cached_hash* h1 = (struct nsec3_cached_hash*)c1;
	struct nsec3_cached_hash* h2 = (struct nsec3_cached_hash*)c2;
	uint8_t* s1, *s2;
	size_t s1len, s2len;
	int c = query_dname_compare(h1->dname, h2->dname);
	if(c != 0)
		return c;
	/* compare parameters */
	/* if both malformed, its equal, robustness */
	if(nsec3_get_algo(h1->nsec3, h1->rr) !=
		nsec3_get_algo(h2->nsec3, h2->rr)) {
		if(nsec3_get_algo(h1->nsec3, h1->rr) <
			nsec3_get_algo(h2->nsec3, h2->rr))
			return -1;
		return 1;
	}
	if(nsec3_get_iter(h1->nsec3, h1->rr) !=
		nsec3_get_iter(h2->nsec3, h2->rr)) {
		if(nsec3_get_iter(h1->nsec3, h1->rr) <
			nsec3_get_iter(h2->nsec3, h2->rr))
			return -1;
		return 1;
	}
	(void)nsec3_get_salt(h1->nsec3, h1->rr, &s1, &s1len);
	(void)nsec3_get_salt(h2->nsec3, h2->rr, &s2, &s2len);
	if(s1len != s2len) {
		if(s1len < s2len)
			return -1;
		return 1;
	}
	return memcmp(s1, s2, s1len);
}

/** perform hash of name */
static int
nsec3_calc_hash(struct region* region, ldns_buffer* buf, 
	struct nsec3_cached_hash* c)
{
	int algo = nsec3_get_algo(c->nsec3, c->rr);
	size_t iter = nsec3_get_iter(c->nsec3, c->rr);
	uint8_t* salt;
	size_t saltlen, i;
	if(!nsec3_get_salt(c->nsec3, c->rr, &salt, &saltlen))
		return -1;
	/* prepare buffer for first iteration */
	ldns_buffer_clear(buf);
	ldns_buffer_write(buf, c->dname, c->dname_len);
	query_dname_tolower(ldns_buffer_begin(buf));
	ldns_buffer_write(buf, salt, saltlen);
	ldns_buffer_flip(buf);
	switch(algo) {
#ifdef SHA_DIGEST_LENGTH
		case NSEC3_HASH_SHA1:
			c->hash_len = SHA_DIGEST_LENGTH;
			c->hash = (uint8_t*)region_alloc(region, c->hash_len);
			if(!c->hash)
				return 0;
			(void)SHA1((unsigned char*)ldns_buffer_begin(buf),
				(unsigned long)ldns_buffer_limit(buf),
				(unsigned char*)c->hash);
			for(i=0; i<iter; i++) {
				ldns_buffer_clear(buf);
				ldns_buffer_write(buf, c->hash, c->hash_len);
				ldns_buffer_write(buf, salt, saltlen);
				ldns_buffer_flip(buf);
				(void)SHA1(
					(unsigned char*)ldns_buffer_begin(buf),
					(unsigned long)ldns_buffer_limit(buf),
					(unsigned char*)c->hash);
			}
#endif /* SHA_DIGEST_LENGTH */
		default:
			log_err("nsec3 hash of unknown algo %d", algo);
			return -1;
	}
	return 1;
}

/** This function we get from ldns-compat or from base system */
int b32_ntop_extended_hex(uint8_t const *src, size_t srclength,
             char *target, size_t targsize);

/** perform b32 encoding of hash */
static int
nsec3_calc_b32(struct region* region, ldns_buffer* buf, 
	struct nsec3_cached_hash* c)
{
	int r;
	ldns_buffer_clear(buf);
	r = b32_ntop_extended_hex(c->hash, c->hash_len,
		(char*)ldns_buffer_begin(buf), ldns_buffer_limit(buf));
	if(r < 1) {
		log_err("b32_ntop_extended_hex: error in encoding: %d", r);
		return 0;
	}
	c->b32_len = (size_t)r;
	c->b32 = region_alloc_init(region, ldns_buffer_begin(buf), c->b32_len);
	if(!c->b32)
		return 0;
	return 1;
}

/**
 * Obtain the hash of an owner name.
 * @param table: the cache table. Must be inited at start.
 * @param region: scratch region to use for allocation.
 * @param buf: temporary buffer.
 * @param nsec3: the rrset with parameters
 * @param rr: rr number from d that has the NSEC3 parameters to hash to.
 * @param dname: name to hash
 * @param dname_len: the length of the name.
 * @param hash: the hash node is returned on success.
 * @return:
 * 	1 on success, either from cache or newly hashed hash is returned.
 * 	0 on a malloc failure.
 * 	-1 if the NSEC3 rr was badly formatted (i.e. formerr).
 */
static int
nsec3_hash_name(rbtree_t* table, struct region* region, ldns_buffer* buf,
	struct ub_packed_rrset_key* nsec3, int rr, uint8_t* dname, 
	size_t dname_len, struct nsec3_cached_hash** hash)
{
	struct nsec3_cached_hash* c;
	struct nsec3_cached_hash looki;
	rbnode_t* n;
	int r;
	looki.node.key = &looki;
	looki.nsec3 = nsec3;
	looki.rr = rr;
	looki.dname = dname;
	looki.dname_len = dname_len;
	/* lookup first in cache */
	c = (struct nsec3_cached_hash*)rbtree_search(table, &looki);
	if(c) {
		*hash = c;
		return 1;
	}
	/* create a new entry */
	c = (struct nsec3_cached_hash*)region_alloc(region, sizeof(*c));
	if(!c) return 0;
	c->node.key = c;
	c->nsec3 = nsec3;
	c->rr = rr;
	c->dname = dname;
	c->dname_len = dname_len;
	r = nsec3_calc_hash(region, buf, c);
	if(r != 1)
		return r;
	r = nsec3_calc_b32(region, buf, c);
	if(r != 1)
		return r;
	n = rbtree_insert(table, &c->node);
	log_assert(n); /* cannot be duplicate, just did lookup */
	*hash = c;
	return 1;
}

/**
 * Find matching NSEC3
 * Find the NSEC3Record that matches a hash of a name.
 */

/**
 * nsec3Covers
 * Given a hash and a candidate NSEC3Record, determine if that NSEC3Record
 * covers the hash. Covers specifically means that the hash is in between
 * the owner and next hashes and does not equal either.
 */

/**
 * findCoveringNSEC3
 * Given a pre-hashed name, find a covering NSEC3 from among a list of
 * NSEC3s.
 */

/**
 * findClosestEncloser
 * Given a name and a list of NSEC3s, find the candidate closest encloser.
 * This will be the first ancestor of 'name' (including itself) to have a
 * matching NSEC3 RR.
 */

/**
 * proveClosestEncloser
 * Given a List of nsec3 RRs, find and prove the closest encloser to qname.
 */


