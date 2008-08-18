/*
 * validator/val_neg.c - validator aggressive negative caching functions.
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
 * The functions help with aggressive negative caching.
 * This creates new denials of existance, and proofs for absence of types
 * from cached NSEC records.
 */
#include "config.h"
#include "validator/val_neg.h"
#include "util/data/dname.h"
#include "util/log.h"
#include "util/net_help.h"

int val_neg_data_compare(const void* a, const void* b)
{
	struct val_neg_data* x = (struct val_neg_data*)a;
	struct val_neg_data* y = (struct val_neg_data*)b;
	int m;
	return dname_canon_lab_cmp(x->name, x->labs, y->name, y->labs, &m);
}

int val_neg_zone_compare(const void* a, const void* b)
{
	struct val_neg_zone* x = (struct val_neg_zone*)a;
	struct val_neg_zone* y = (struct val_neg_zone*)b;
	int m;
	if(x->dclass != y->dclass) {
		if(x->dclass < y->dclass)
			return -1;
		return 1;
	}
	return dname_canon_lab_cmp(x->name, x->labs, y->name, y->labs, &m);
}

struct val_neg_cache* val_neg_create()
{
	struct val_neg_cache* neg = (struct val_neg_cache*)calloc(1, 
		sizeof(*neg));
	if(!neg) {
		log_err("Could not create neg cache: out of memory");
		return NULL;
	}
	neg->max = 1024*1024; /* 1 M is thousands of entries */
	rbtree_init(&neg->tree, &val_neg_zone_compare);
	lock_basic_init(&neg->lock);
	lock_protect(&neg->lock, neg, sizeof(*neg));
	return neg;
}

int val_neg_apply_cfg(struct val_neg_cache* neg, struct config_file* cfg)
{
	/* TODO read max mem size from cfg */
	return 1;
}

size_t val_neg_get_mem(struct val_neg_cache* neg)
{
	size_t result;
	lock_basic_lock(&neg->lock);
	result = sizeof(*neg) + neg->use;
	lock_basic_unlock(&neg->lock);
	return result;
}

void neg_cache_delete(struct val_neg_cache* neg)
{
	struct val_neg_data* p, *np;
	if(!neg) return;
	lock_basic_destroy(&neg->lock);
	/* delete all the zonedata elements */
	p = neg->first;
	while(p) {
		np = p->next;
		free(p->name);
		free(p);
		p = np;
	}
	/* delete all the zones in the tree */
	/* TODO */
	free(neg);
}

/**
 * Create more space in negative cache
 * The oldest elements are deleted until enough space is present.
 * Empty zones are deleted.
 * @param neg: negative cache.
 * @param need: how many bytes are needed.
 */
static void neg_make_space(struct val_neg_cache* neg, size_t need)
{
	/* delete elements until enough space or its empty */
	while(neg->last && (neg->max - neg->use) < need) {
		/* Delete data, zone */
		/* update parent ptrs of items beneath it */
	}
}

/**
 * Find the given zone, from the SOA owner name and class
 * @param neg: negative cache
 * @param soa: what to look for.
 * @return zone or NULL if not found.
 */
static struct val_neg_zone* neg_find_zone(struct val_neg_cache* neg, 
	struct ub_packed_rrset_key* soa)
{
	struct val_neg_zone lookfor;
	struct val_neg_zone* result;
	lookfor.node.key = &lookfor;
	lookfor.name = soa->rk.dname;
	lookfor.len = soa->rk.dname_len;
	lookfor.labs = dname_count_labels(lookfor.name);
	lookfor.dclass = ntohs(soa->rk.rrset_class);

	result = (struct val_neg_zone*)
		rbtree_search(&neg->tree, lookfor.node.key);
	return result;
}

/**
 * Create a new zone.
 * @param neg: negative cache
 * @param soa: what to look for.
 * @return zone or NULL if out of memory.
 * 	Other data may be deleted to make room for the new zone.
 */
static struct val_neg_zone* neg_create_zone(struct val_neg_cache* neg,
	struct ub_packed_rrset_key* soa)
{
	struct val_neg_zone* zone;
	size_t need;

	/* make space */
	need = sizeof(struct val_neg_zone) + soa->rk.dname_len;
	neg_make_space(neg, need);

	/* create new item */
	zone = (struct val_neg_zone*)calloc(1, sizeof(*zone));
	if(!zone) {
		return NULL;
	}
	zone->node.key = zone;
	zone->name = memdup(soa->rk.dname, soa->rk.dname_len);
	if(!zone->name) {
		return NULL;
	}
	zone->len = soa->rk.dname_len;
	zone->labs = dname_count_labels(zone->name);
	zone->dclass = ntohs(soa->rk.rrset_class);

	zone->soa_hash = soa->entry.hash;
	rbtree_init(&zone->tree, &val_neg_data_compare);

	/* insert in tree */
	(void)rbtree_insert(&neg->tree, &zone->node);

	/* find zone->parent */
	

	/* set this zone as parent for lower zones */
}

/** find zone name of message, returns the SOA record */
static struct ub_packed_rrset_key* reply_find_soa(struct reply_info* rep)
{
	size_t i;
	for(i=rep->an_numrrsets; i< rep->an_numrrsets+rep->ns_numrrsets; i++){
		if(ntohs(rep->rrsets[i]->rk.type) != LDNS_RR_TYPE_SOA)
			return rep->rrsets[i];
	}
	return NULL;
}

void val_neg_addreply(struct val_neg_cache* neg, struct reply_info* rep)
{
	size_t i;
	struct ub_packed_rrset_key* soa;
	struct val_neg_zone* zone;
	/* find the zone name in message */
	soa = reply_find_soa(rep);
	if(!soa)
		return;

	/* find or create the zone entry */
	lock_basic_lock(&neg->lock);
	zone = neg_find_zone(neg, soa);
	if(!zone) {
		if(!(zone = neg_create_zone(neg, soa))) {
			lock_basic_unlock(&neg->lock);
			log_err("out of memory adding negative zone");
			return;
		}
	}

	/* insert the NSECs */
	for(i=rep->an_numrrsets; i< rep->an_numrrsets+rep->ns_numrrsets; i++){
		if(ntohs(rep->rrsets[i]->rk.type) != LDNS_RR_TYPE_NSEC)
			continue;
	}
	lock_basic_unlock(&neg->lock);
}

int val_neg_dlvlookup(struct val_neg_cache* neg, uint8_t* qname, size_t len,
        uint16_t qclass)
{
	/* lookup closest data record */

	/* examine proof */

	/* delete closest data record if expired */

	/* if OK touch the LRU for it */
}
