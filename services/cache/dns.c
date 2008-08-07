/*
 * services/cache/dns.c - Cache services for DNS using msg and rrset caches.
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
 * This file contains the DNS cache.
 */
#include "config.h"
#include "iterator/iter_delegpt.h"
#include "services/cache/dns.h"
#include "services/cache/rrset.h"
#include "util/data/msgreply.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/module.h"
#include "util/net_help.h"
#include "util/regional.h"

/** store rrsets in the rrset cache. 
 * @param env: module environment with caches.
 * @param rep: contains list of rrsets to store.
 * @param now: current time.
 */
static void
store_rrsets(struct module_env* env, struct reply_info* rep, uint32_t now)
{
        size_t i;
        /* see if rrset already exists in cache, if not insert it. */
        for(i=0; i<rep->rrset_count; i++) {
                rep->ref[i].key = rep->rrsets[i];
                rep->ref[i].id = rep->rrsets[i]->id;
                if(rrset_cache_update(env->rrset_cache, &rep->ref[i],
                        env->alloc, now)) /* it was in the cache */
                        rep->rrsets[i] = rep->ref[i].key;
        }
}

void 
dns_cache_store_msg(struct module_env* env, struct query_info* qinfo,
	hashvalue_t hash, struct reply_info* rep)
{
	struct msgreply_entry* e;
	uint32_t ttl = rep->ttl;
	size_t i;

	/* store RRsets */
        for(i=0; i<rep->rrset_count; i++) {
		rep->ref[i].key = rep->rrsets[i];
		rep->ref[i].id = rep->rrsets[i]->id;
	}
	reply_info_sortref(rep);
	reply_info_set_ttls(rep, *env->now);
	store_rrsets(env, rep, *env->now);
	if(ttl == 0) {
		/* we do not store the message, but we did store the RRs,
		 * which could be useful for delegation information */
		verbose(VERB_ALGO, "TTL 0: dropped msg from cache");
		free(rep);
		return;
	}

	/* store msg in the cache */
	reply_info_sortref(rep);
	if(!(e = query_info_entrysetup(qinfo, rep, hash))) {
		log_err("store_msg: malloc failed");
		return;
	}
	slabhash_insert(env->msg_cache, hash, &e->entry, rep, env->alloc);
}

/** allocate rrset in region - no more locks needed */
static struct ub_packed_rrset_key*
copy_rrset(struct ub_packed_rrset_key* key, struct regional* region, 
	uint32_t now)
{
	struct ub_packed_rrset_key* ck = regional_alloc(region, 
		sizeof(struct ub_packed_rrset_key));
	struct packed_rrset_data* d;
	struct packed_rrset_data* data = (struct packed_rrset_data*)
		key->entry.data;
	size_t dsize, i;
	if(!ck)
		return NULL;
	ck->id = key->id;
	memset(&ck->entry, 0, sizeof(ck->entry));
	ck->entry.hash = key->entry.hash;
	ck->entry.key = ck;
	ck->rk = key->rk;
	ck->rk.dname = regional_alloc_init(region, key->rk.dname, 
		key->rk.dname_len);
	if(!ck->rk.dname)
		return NULL;
	dsize = packed_rrset_sizeof(data);
	d = (struct packed_rrset_data*)regional_alloc_init(region, data, dsize);
	if(!d)
		return NULL;
	ck->entry.data = d;
	packed_rrset_ptr_fixup(d);
	/* make TTLs relative - once per rrset */
	for(i=0; i<d->count + d->rrsig_count; i++)
		d->rr_ttl[i] -= now;
	d->ttl -= now;
	return ck;
}

/** find closest NS or DNAME and returns the rrset (locked) */
static struct ub_packed_rrset_key*
find_closest_of_type(struct module_env* env, uint8_t* qname, size_t qnamelen, 
	uint16_t qclass, uint32_t now, uint16_t searchtype, int stripfront)
{
	struct ub_packed_rrset_key *rrset;
	uint8_t lablen;

	if(stripfront) {
		/* strip off so that DNAMEs have strict subdomain match */
		lablen = *qname;
		qname += lablen + 1;
		qnamelen -= lablen + 1;
	}

	/* snip off front part of qname until the type is found */
	while(qnamelen > 0) {
		if((rrset = rrset_cache_lookup(env->rrset_cache, qname, 
			qnamelen, searchtype, qclass, 0, now, 0)))
			return rrset;

		/* snip off front label */
		lablen = *qname;
		qname += lablen + 1;
		qnamelen -= lablen + 1;
	}
	return NULL;
}

/** add addr to additional section */
static void
addr_to_additional(struct ub_packed_rrset_key* rrset, struct regional* region,
	struct dns_msg* msg, uint32_t now)
{
	if((msg->rep->rrsets[msg->rep->rrset_count] = 
		copy_rrset(rrset, region, now))) {
		msg->rep->ar_numrrsets++;
		msg->rep->rrset_count++;
	}
}

/** find and add A and AAAA records for nameservers in delegpt */
static int
find_add_addrs(struct module_env* env, uint16_t qclass, 
	struct regional* region, struct delegpt* dp, uint32_t now, 
	struct dns_msg** msg)
{
	struct delegpt_ns* ns;
	struct ub_packed_rrset_key* akey;
	for(ns = dp->nslist; ns; ns = ns->next) {
		akey = rrset_cache_lookup(env->rrset_cache, ns->name, 
			ns->namelen, LDNS_RR_TYPE_A, qclass, 0, now, 0);
		if(akey) {
			if(!delegpt_add_rrset_A(dp, region, akey)) {
				lock_rw_unlock(&akey->entry.lock);
				return 0;
			}
			if(msg)
				addr_to_additional(akey, region, *msg, now);
			lock_rw_unlock(&akey->entry.lock);
		}
		akey = rrset_cache_lookup(env->rrset_cache, ns->name, 
			ns->namelen, LDNS_RR_TYPE_AAAA, qclass, 0, now, 0);
		if(akey) {
			if(!delegpt_add_rrset_AAAA(dp, region, akey)) {
				lock_rw_unlock(&akey->entry.lock);
				return 0;
			}
			if(msg)
				addr_to_additional(akey, region, *msg, now);
			lock_rw_unlock(&akey->entry.lock);
		}
	}
	return 1;
}

/** find and add A and AAAA records for missing nameservers in delegpt */
int
cache_fill_missing(struct module_env* env, uint16_t qclass, 
	struct regional* region, struct delegpt* dp)
{
	struct delegpt_ns* ns;
	struct ub_packed_rrset_key* akey;
	uint32_t now = *env->now;
	for(ns = dp->nslist; ns; ns = ns->next) {
		if(ns->resolved)
			continue;
		akey = rrset_cache_lookup(env->rrset_cache, ns->name, 
			ns->namelen, LDNS_RR_TYPE_A, qclass, 0, now, 0);
		if(akey) {
			if(!delegpt_add_rrset_A(dp, region, akey)) {
				lock_rw_unlock(&akey->entry.lock);
				return 0;
			}
			log_nametypeclass(VERB_ALGO, "found in cache",
				ns->name, LDNS_RR_TYPE_A, qclass);
			lock_rw_unlock(&akey->entry.lock);
		}
		akey = rrset_cache_lookup(env->rrset_cache, ns->name, 
			ns->namelen, LDNS_RR_TYPE_AAAA, qclass, 0, now, 0);
		if(akey) {
			if(!delegpt_add_rrset_AAAA(dp, region, akey)) {
				lock_rw_unlock(&akey->entry.lock);
				return 0;
			}
			log_nametypeclass(VERB_ALGO, "found in cache",
				ns->name, LDNS_RR_TYPE_AAAA, qclass);
			lock_rw_unlock(&akey->entry.lock);
		}
	}
	return 1;
}

/** find and add DS or NSEC to delegation msg */
static void
find_add_ds(struct module_env* env, struct regional* region, 
	struct dns_msg* msg, struct delegpt* dp, uint32_t now)
{
	/* Lookup the DS or NSEC at the delegation point. */
	struct ub_packed_rrset_key* rrset = rrset_cache_lookup(
		env->rrset_cache, dp->name, dp->namelen, LDNS_RR_TYPE_DS, 
		msg->qinfo.qclass, 0, now, 0);
	if(!rrset) {
		/* NOTE: this won't work for alternate NSEC schemes 
		 *	(opt-in, NSEC3) */
		rrset = rrset_cache_lookup(env->rrset_cache, dp->name, 
			dp->namelen, LDNS_RR_TYPE_NSEC, msg->qinfo.qclass, 
			0, now, 0);
		/* Note: the PACKED_RRSET_NSEC_AT_APEX flag is not used.
		 * since this is a referral, we need the NSEC at the parent
		 * side of the zone cut, not the NSEC at apex side. */
	}
	if(rrset) {
		/* add it to auth section. This is the second rrset. */
		if((msg->rep->rrsets[msg->rep->rrset_count] = 
			copy_rrset(rrset, region, now))) {
			msg->rep->ns_numrrsets++;
			msg->rep->rrset_count++;
		}
		lock_rw_unlock(&rrset->entry.lock);
	}
}

/** create referral message with NS and query */
static struct dns_msg*
create_msg(uint8_t* qname, size_t qnamelen, uint16_t qtype, uint16_t qclass, 
	struct regional* region, struct ub_packed_rrset_key* nskey, 
	struct packed_rrset_data* nsdata, uint32_t now)
{
	struct dns_msg* msg = (struct dns_msg*)regional_alloc(region,
		sizeof(struct dns_msg));
	if(!msg)
		return NULL;
	msg->qinfo.qname = regional_alloc_init(region, qname, qnamelen);
	if(!msg->qinfo.qname)
		return NULL;
	msg->qinfo.qname_len = qnamelen;
	msg->qinfo.qtype = qtype;
	msg->qinfo.qclass = qclass;
	/* non-packed reply_info, because it needs to grow the array */
	msg->rep = (struct reply_info*)regional_alloc(region, 
		sizeof(struct reply_info)-sizeof(struct rrset_ref));
	if(!msg->rep)
		return NULL;
	memset(msg->rep, 0, 
		sizeof(struct reply_info)-sizeof(struct rrset_ref));
	msg->rep->flags = BIT_QR; /* with QR, no AA */
	msg->rep->qdcount = 1;
	/* allocate the array to as much as we could need:
	 *	NS rrset + DS/NSEC rrset +
	 *	A rrset for every NS RR
	 *	AAAA rrset for every NS RR
	 */
	msg->rep->rrsets = (struct ub_packed_rrset_key**)
		regional_alloc(region, 
		(2 + nsdata->count*2)*sizeof(struct ub_packed_rrset_key*));
	if(!msg->rep->rrsets)
		return NULL;
	msg->rep->rrsets[0] = copy_rrset(nskey, region, now);
	if(!msg->rep->rrsets[0])
		return NULL;
	msg->rep->ns_numrrsets++;
	msg->rep->rrset_count++;
	return msg;
}

struct delegpt* 
dns_cache_find_delegation(struct module_env* env, uint8_t* qname, 
	size_t qnamelen, uint16_t qtype, uint16_t qclass, 
	struct regional* region, struct dns_msg** msg, uint32_t now)
{
	/* try to find closest NS rrset */
	struct ub_packed_rrset_key* nskey;
	struct packed_rrset_data* nsdata;
	struct delegpt* dp;

	nskey = find_closest_of_type(env, qname, qnamelen, qclass, now,
		LDNS_RR_TYPE_NS, 0);
	if(!nskey) /* hope the caller has hints to prime or something */
		return NULL;
	nsdata = (struct packed_rrset_data*)nskey->entry.data;
	/* got the NS key, create delegation point */
	dp = delegpt_create(region);
	if(!dp || !delegpt_set_name(dp, region, nskey->rk.dname)) {
		lock_rw_unlock(&nskey->entry.lock);
		log_err("find_delegation: out of memory");
		return NULL;
	}
	/* create referral message */
	if(msg) {
		*msg = create_msg(qname, qnamelen, qtype, qclass, region, 
			nskey, nsdata, now);
		if(!*msg) {
			lock_rw_unlock(&nskey->entry.lock);
			log_err("find_delegation: out of memory");
			return NULL;
		}
	}
	if(!delegpt_rrset_add_ns(dp, region, nskey))
		log_err("find_delegation: addns out of memory");
	lock_rw_unlock(&nskey->entry.lock); /* first unlock before next lookup*/
	/* find and add DS/NSEC (if any) */
	if(msg)
		find_add_ds(env, region, *msg, dp, now);
	/* find and add A entries */
	if(!find_add_addrs(env, qclass, region, dp, now, msg))
		log_err("find_delegation: addrs out of memory");
	return dp;
}

/** allocate dns_msg from query_info and reply_info */
static struct dns_msg*
gen_dns_msg(struct regional* region, struct query_info* q, size_t num)
{
	struct dns_msg* msg = (struct dns_msg*)regional_alloc(region, 
		sizeof(struct dns_msg));
	if(!msg)
		return NULL;
	memcpy(&msg->qinfo, q, sizeof(struct query_info));
	msg->qinfo.qname = regional_alloc_init(region, q->qname, q->qname_len);
	if(!msg->qinfo.qname)
		return NULL;
	/* allocate replyinfo struct and rrset key array separately */
	msg->rep = (struct reply_info*)regional_alloc(region,
		sizeof(struct reply_info) - sizeof(struct rrset_ref));
	if(!msg->rep)
		return NULL;
	msg->rep->rrsets = (struct ub_packed_rrset_key**)
		regional_alloc(region,
		num * sizeof(struct ub_packed_rrset_key*));
	if(!msg->rep->rrsets)
		return NULL;
	return msg;
}

/** generate dns_msg from cached message */
static struct dns_msg*
tomsg(struct module_env* env, struct msgreply_entry* e, struct reply_info* r, 
	struct regional* region, uint32_t now, struct regional* scratch)
{
	struct dns_msg* msg;
	size_t i;
	if(now > r->ttl)
		return NULL;
	msg = gen_dns_msg(region, &e->key, r->rrset_count);
	if(!msg)
		return NULL;
	msg->rep->flags = r->flags;
	msg->rep->qdcount = r->qdcount;
	msg->rep->ttl = r->ttl;
	msg->rep->security = r->security;
	msg->rep->an_numrrsets = r->an_numrrsets;
	msg->rep->ns_numrrsets = r->ns_numrrsets;
	msg->rep->ar_numrrsets = r->ar_numrrsets;
	msg->rep->rrset_count = r->rrset_count;
	if(!rrset_array_lock(r->ref, r->rrset_count, now))
		return NULL;
	for(i=0; i<msg->rep->rrset_count; i++) {
		msg->rep->rrsets[i] = copy_rrset(r->rrsets[i], region, now);
		if(!msg->rep->rrsets[i]) {
			rrset_array_unlock(r->ref, r->rrset_count);
			return NULL;
		}
	}
	rrset_array_unlock_touch(env->rrset_cache, scratch, r->ref, 
		r->rrset_count);
	return msg;
}

/** synthesize RRset-only response from cached RRset item */
static struct dns_msg*
rrset_msg(struct ub_packed_rrset_key* rrset, struct regional* region, 
	uint32_t now, struct query_info* q)
{
	struct dns_msg* msg;
	struct packed_rrset_data* d = (struct packed_rrset_data*)
		rrset->entry.data;
	if(now > d->ttl)
		return NULL;
	msg = gen_dns_msg(region, q, 1); /* only the CNAME (or other) RRset */
	if(!msg)
		return NULL;
	msg->rep->flags = BIT_QR; /* reply, no AA, no error */
	msg->rep->qdcount = 1;
	msg->rep->ttl = d->ttl - now;
	msg->rep->security = sec_status_unchecked;
	msg->rep->an_numrrsets = 1;
	msg->rep->ns_numrrsets = 0;
	msg->rep->ar_numrrsets = 0;
	msg->rep->rrset_count = 1;
	msg->rep->rrsets[0] = copy_rrset(rrset, region, now);
	if(!msg->rep->rrsets[0]) /* copy CNAME */
		return NULL;
	return msg;
}

/** synthesize DNAME+CNAME response from cached DNAME item */
static struct dns_msg*
synth_dname_msg(struct ub_packed_rrset_key* rrset, struct regional* region, 
	uint32_t now, struct query_info* q)
{
	struct dns_msg* msg;
	struct ub_packed_rrset_key* ck;
	struct packed_rrset_data* newd, *d = (struct packed_rrset_data*)
		rrset->entry.data;
	uint8_t* newname, *dtarg = NULL;
	size_t newlen, dtarglen;
	if(now > d->ttl)
		return NULL;
	/* only allow validated (with DNSSEC) DNAMEs used from cache 
	 * for insecure DNAMEs, query again. */
	if(d->security != sec_status_secure)
		return NULL;
	msg = gen_dns_msg(region, q, 2); /* DNAME + CNAME RRset */
	if(!msg)
		return NULL;
	msg->rep->flags = BIT_QR; /* reply, no AA, no error */
	msg->rep->qdcount = 1;
	msg->rep->ttl = d->ttl - now;
	msg->rep->security = sec_status_unchecked;
	msg->rep->an_numrrsets = 1;
	msg->rep->ns_numrrsets = 0;
	msg->rep->ar_numrrsets = 0;
	msg->rep->rrset_count = 1;
	msg->rep->rrsets[0] = copy_rrset(rrset, region, now);
	if(!msg->rep->rrsets[0]) /* copy DNAME */
		return NULL;
	/* synth CNAME rrset */
	get_cname_target(rrset, &dtarg, &dtarglen);
	if(!dtarg)
		return NULL;
	newlen = q->qname_len + dtarglen - rrset->rk.dname_len;
	if(newlen > LDNS_MAX_DOMAINLEN) {
		msg->rep->flags |= LDNS_RCODE_YXDOMAIN;
		return msg;
	}
	newname = (uint8_t*)regional_alloc(region, newlen);
	if(!newname)
		return NULL;
	/* new name is concatenation of qname front (without DNAME owner)
	 * and DNAME target name */
	memcpy(newname, q->qname, q->qname_len-rrset->rk.dname_len);
	memmove(newname+(q->qname_len-rrset->rk.dname_len), dtarg, dtarglen);
	/* create rest of CNAME rrset */
	ck = (struct ub_packed_rrset_key*)regional_alloc(region, 
		sizeof(struct ub_packed_rrset_key));
	if(!ck)
		return NULL;
	memset(&ck->entry, 0, sizeof(ck->entry));
	msg->rep->rrsets[1] = ck;
	ck->entry.key = ck;
	ck->rk.type = htons(LDNS_RR_TYPE_CNAME);
	ck->rk.rrset_class = rrset->rk.rrset_class;
	ck->rk.flags = 0;
	ck->rk.dname = regional_alloc_init(region, q->qname, q->qname_len);
	if(!ck->rk.dname)
		return NULL;
	ck->rk.dname_len = q->qname_len;
	ck->entry.hash = rrset_key_hash(&ck->rk);
	newd = (struct packed_rrset_data*)regional_alloc(region,
		sizeof(struct packed_rrset_data) + sizeof(size_t) + 
		sizeof(uint8_t*) + sizeof(uint32_t) + sizeof(uint16_t) 
		+ newlen);
	if(!newd)
		return NULL;
	ck->entry.data = newd;
	newd->ttl = 0; /* 0 for synthesized CNAME TTL */
	newd->count = 1;
	newd->rrsig_count = 0;
	newd->trust = rrset_trust_ans_noAA;
	newd->rr_len = (size_t*)((uint8_t*)newd + 
		sizeof(struct packed_rrset_data));
	newd->rr_len[0] = newlen + sizeof(uint16_t);
	packed_rrset_ptr_fixup(newd);
	newd->rr_ttl[0] = newd->ttl;
	msg->rep->ttl = newd->ttl;
	ldns_write_uint16(newd->rr_data[0], newlen);
	memmove(newd->rr_data[0] + sizeof(uint16_t), newname, newlen);
	msg->rep->an_numrrsets ++;
	msg->rep->rrset_count ++;
	return msg;
}

struct dns_msg* 
dns_cache_lookup(struct module_env* env,
	uint8_t* qname, size_t qnamelen, uint16_t qtype, uint16_t qclass,
	struct regional* region, struct regional* scratch)
{
	struct lruhash_entry* e;
	struct query_info k;
	hashvalue_t h;
	uint32_t now = *env->now;
	struct ub_packed_rrset_key* rrset;

	/* lookup first, this has both NXdomains and ANSWER responses */
	k.qname = qname;
	k.qname_len = qnamelen;
	k.qtype = qtype;
	k.qclass = qclass;
	h = query_info_hash(&k);
	e = slabhash_lookup(env->msg_cache, h, &k, 0);
	if(e) {
		struct msgreply_entry* key = (struct msgreply_entry*)e->key;
		struct reply_info* data = (struct reply_info*)e->data;
		struct dns_msg* msg = tomsg(env, key, data, region, now, 
			scratch);
		if(msg) {
			lock_rw_unlock(&e->lock);
			return msg;
		}
		/* could be msg==NULL; due to TTL or not all rrsets available */
		lock_rw_unlock(&e->lock);
	}

	/* see if a DNAME exists. Checked for first, to enforce that DNAMEs
	 * are more important, the CNAME is resynthesized and thus 
	 * consistent with the DNAME */
	if( (rrset=find_closest_of_type(env, qname, qnamelen, qclass, now,
		LDNS_RR_TYPE_DNAME, 1))) {
		/* synthesize a DNAME+CNAME message based on this */
		struct dns_msg* msg = synth_dname_msg(rrset, region, now, &k);
		if(msg) {
			lock_rw_unlock(&rrset->entry.lock);
			return msg;
		}
		lock_rw_unlock(&rrset->entry.lock);
	}

	/* see if we have CNAME for this domain */
	if( (rrset=rrset_cache_lookup(env->rrset_cache, qname, qnamelen, 
		LDNS_RR_TYPE_CNAME, qclass, 0, now, 0))) {
		struct dns_msg* msg = rrset_msg(rrset, region, now, &k);
		if(msg) {
			lock_rw_unlock(&rrset->entry.lock);
			return msg;
		}
		lock_rw_unlock(&rrset->entry.lock);
	}

	/* construct DS, DNSKEY messages from rrset cache. */
	if((qtype == LDNS_RR_TYPE_DS || qtype == LDNS_RR_TYPE_DNSKEY) &&
		(rrset=rrset_cache_lookup(env->rrset_cache, qname, qnamelen, 
		qtype, qclass, 0, now, 0))) {
		/* if the rrset is from the additional section, and the
		 * signatures have fallen off, then do not synthesize a msg
		 * instead, allow a full query for signed results to happen.
		 * Forego all rrset data from additional section, because
		 * some signatures may not be present and cause validation
		 * failure.
		 */
		struct packed_rrset_data *d = (struct packed_rrset_data*)
			rrset->entry.data;
		if(d->trust != rrset_trust_add_noAA && 
			d->trust != rrset_trust_add_AA) {
			struct dns_msg* msg = rrset_msg(rrset, region, now, &k);
			if(msg) {
				lock_rw_unlock(&rrset->entry.lock);
				return msg;
			}
		}
		lock_rw_unlock(&rrset->entry.lock);
	}
	return NULL;
}

int 
dns_cache_store(struct module_env* env, struct query_info* msgqinf,
        struct reply_info* msgrep, int is_referral)
{
	struct reply_info* rep = NULL;
	/* alloc, malloc properly (not in region, like msg is) */
	rep = reply_info_copy(msgrep, env->alloc, NULL);
	if(!rep)
		return 0;

	if(is_referral) {
		/* store rrsets */
		struct rrset_ref ref;
		size_t i;
		for(i=0; i<rep->rrset_count; i++) {
			packed_rrset_ttl_add((struct packed_rrset_data*)
				rep->rrsets[i]->entry.data, *env->now);
			ref.key = rep->rrsets[i];
			ref.id = rep->rrsets[i]->id;
			/*ignore ret: it was in the cache, ref updated */
			(void)rrset_cache_update(env->rrset_cache, &ref, 
				env->alloc, *env->now);
		}
		free(rep);
		return 1;
	} else {
		/* store msg, and rrsets */
		struct query_info qinf;
		hashvalue_t h;

		qinf = *msgqinf;
		qinf.qname = memdup(msgqinf->qname, msgqinf->qname_len);
		if(!qinf.qname) {
			reply_info_parsedelete(rep, env->alloc);
			return 0;
		}
		/* fixup flags to be sensible for a reply based on the cache */
		/* this module means that RA is available. It is an answer QR. 
		 * Not AA from cache. Not CD in cache (depends on client bit). */
		rep->flags |= (BIT_RA | BIT_QR);
		rep->flags &= ~(BIT_AA | BIT_CD);
		h = query_info_hash(&qinf);
		dns_cache_store_msg(env, &qinf, h, rep);
		/* qname is used inside query_info_entrysetup, and set to 
		 * NULL. If it has not been used, free it. free(0) is safe. */
		free(qinf.qname);
	}
	return 1;
}
