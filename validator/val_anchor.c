/*
 * validator/val_anchor.c - validator trust anchor storage.
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
 * This file contains storage for the trust anchors for the validator.
 */
#include "config.h"
#include "validator/val_anchor.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/region-allocator.h"
#include "util/config_file.h"

/** compare two trust anchors */
static int
anchor_cmp(const void* k1, const void* k2)
{
	int m;
	struct trust_anchor* n1 = (struct trust_anchor*)k1;
	struct trust_anchor* n2 = (struct trust_anchor*)k2;
	/* no need to ntohs(class) because sort order is irrelevant */
	if(n1->dclass != n2->dclass) {
		if(n1->dclass < n2->dclass)
			return -1;
		return 1;
	}
	return dname_lab_cmp(n1->name, n1->namelabs, n2->name, n2->namelabs, 
		&m);
}

struct val_anchors* 
anchors_create()
{
	struct val_anchors* a = (struct val_anchors*)calloc(1, sizeof(*a));
	if(!a)
		return NULL;
	a->region = region_create(malloc, free);
	if(!a->region) {
		free(a);
		return NULL;
	}
	a->tree = rbtree_create(anchor_cmp);
	if(!a->tree) {
		anchors_delete(a);
		return NULL;
	}
	return a;
}

void 
anchors_delete(struct val_anchors* anchors)
{
	if(!anchors)
		return;
	free(anchors->tree);
	region_destroy(anchors->region);
	free(anchors);
}

/** initialise parent pointers in the tree */
static void
init_parents(struct val_anchors* anchors)
{
	struct trust_anchor* node, *prev = NULL, *p;
	int m; 
	RBTREE_FOR(node, struct trust_anchor*, anchors->tree) {
		node->parent = NULL;
		if(!prev || prev->dclass != node->dclass) {
			prev = node;
			continue;
		}
		(void)dname_lab_cmp(prev->name, prev->namelabs, node->name, 
			node->namelabs, &m); /* we know prev is smaller */
		/* sort order like: . com. bla.com. zwb.com. net. */
		/* find the previous, or parent-parent-parent */
		for(p = prev; p; p = p->parent)
			/* looking for name with few labels, a parent */
			if(p->namelabs <= m) {
				/* ==: since prev matched m, this is closest*/
				/* <: prev matches more, but is not a parent,
			 	* this one is a (grand)parent */
				node->parent = p;
				break;
			}
		prev = node;
	}
}

/**
 * Find a trust anchor. Exact matching.
 * @param anchors: anchor storage.
 * @param name: name of trust anchor (wireformat)
 * @param namelabs: labels in name
 * @param namelen: length of name
 * @param dclass: class of trust anchor
 * @return NULL if not found.
 */
static struct trust_anchor*
anchor_find(struct val_anchors* anchors, uint8_t* name, int namelabs,
	size_t namelen, uint16_t dclass)
{
	struct trust_anchor key;
	rbnode_t* n;
	key.node.key = &key;
	key.name = name;
	key.namelabs = namelabs;
	key.namelen = namelen;
	key.dclass = dclass;
	n = rbtree_search(anchors->tree, &key);
	if(!n)
		return NULL;
	return (struct trust_anchor*)n->key;
}

/** create new trust anchor object */
static struct trust_anchor*
anchor_new_ta(struct val_anchors* anchors, uint8_t* name, int namelabs,
	size_t namelen, uint16_t dclass)
{
	rbnode_t* r;
	struct trust_anchor* ta = (struct trust_anchor*)region_alloc(
		anchors->region, sizeof(struct trust_anchor));
	if(!ta)
		return NULL;
	memset(ta, 0, sizeof(*ta));
	ta->node.key = ta;
	ta->name = region_alloc_init(anchors->region, name, namelen);
	if(!ta->name)
		return NULL;
	ta->namelabs = namelabs;
	ta->namelen = namelen;
	ta->dclass = dclass;
	r = rbtree_insert(anchors->tree, &ta->node);
	log_assert(r != NULL);
	return ta;
}

/** find trustanchor key by exact data match */
static struct ta_key*
anchor_find_key(struct trust_anchor* ta, uint8_t* rdata, size_t rdata_len,
	uint16_t type)
{
	struct ta_key* k;
	for(k = ta->keylist; k; k = k->next) {
		if(k->type == type && k->len == rdata_len &&
			memcmp(k->data, rdata, rdata_len) == 0)
			return k;
	}
	return NULL;
}
	
/** create new trustanchor key */
static struct ta_key*
anchor_new_ta_key(struct val_anchors* anchors, uint8_t* rdata, size_t rdata_len,
	uint16_t type)
{
	struct ta_key* k = (struct ta_key*)region_alloc(anchors->region,
		sizeof(*k));
	if(!k)
		return NULL;
	memset(k, 0, sizeof(*k));
	k->data = region_alloc_init(anchors->region, rdata, rdata_len);
	if(!k->data)
		return NULL;
	k->len = rdata_len;
	k->type = type;
	return k;
}

/**
 * This routine adds a new RR to a trust anchor. The trust anchor may not
 * exist yet, and is created if not. The RR can be DS or DNSKEY.
 * This routine will also remove duplicates; storing them only once.
 * @param anchors: anchor storage.
 * @param name: name of trust anchor (wireformat)
 * @param type: type or RR
 * @param dclass: class of RR
 * @param rdata: rdata wireformat, starting with rdlength.
 * @param rdata_len: length of rdata including rdlength.
 * @return: 0 on error.
 */
static int
anchor_store_new_key(struct val_anchors* anchors, uint8_t* name, uint16_t type,
	uint16_t dclass, uint8_t* rdata, size_t rdata_len)
{
	struct ta_key* k;
	struct trust_anchor* ta;
	int namelabs;
	size_t namelen;
	namelabs = dname_count_size_labels(name, &namelen);
	if(type != LDNS_RR_TYPE_DS && type != LDNS_RR_TYPE_DNSKEY) {
		log_err("Bad type for trust anchor");
		return 0;
	}
	/* lookup or create trustanchor */
	ta = anchor_find(anchors, name, namelabs, namelen, dclass);
	if(!ta) {
		ta = anchor_new_ta(anchors, name, namelabs, namelen, dclass);
		if(!ta)
			return 0;
	}
	/* look for duplicates */
	if(anchor_find_key(ta, rdata, rdata_len, type)) {
		return 1;
	}
	k = anchor_new_ta_key(anchors, rdata, rdata_len, type);
	if(!k)
		return 0;
	/* add new key */
	if(type == LDNS_RR_TYPE_DS)
		ta->numDS++;
	else	ta->numDNSKEY++;
	k->next = ta->keylist;
	ta->keylist = k;
	return 1;
}

/**
 * Add new RR. It converts ldns RR to wire format.
 * @param anchors: anchor storage.
 * @param buffer: parsing buffer.
 * @param rr: the rr (allocated by caller).
 * @return false on error.
 */
static int
anchor_store_new_rr(struct val_anchors* anchors, ldns_buffer* buffer, 
	ldns_rr* rr)
{
	ldns_rdf* owner = ldns_rr_owner(rr);
	ldns_status status;
	ldns_buffer_clear(buffer);
	ldns_buffer_skip(buffer, 2); /* skip rdatalen */
	status = ldns_rr_rdata2buffer_wire(buffer, rr);
	if(status != LDNS_STATUS_OK) {
		log_err("error converting trustanchor to wireformat: %s", 
			ldns_get_errorstr_by_id(status));
		return 0;
	}
	ldns_buffer_flip(buffer);
	ldns_buffer_write_u16_at(buffer, 0, ldns_buffer_limit(buffer) - 2);

	if(!anchor_store_new_key(anchors, ldns_rdf_data(owner), 
		ldns_rr_get_type(rr), ldns_rr_get_class(rr),
		ldns_buffer_begin(buffer), ldns_buffer_limit(buffer))) {
		return 0;
	}
	log_nametypeclass(VERB_DETAIL, "adding trusted key",
		ldns_rdf_data(owner), 
		ldns_rr_get_type(rr), ldns_rr_get_class(rr));
	return 1;
}

int
anchor_store_str(struct val_anchors* anchors, ldns_buffer* buffer,
	const char* str)
{
	ldns_rr* rr = NULL;
	ldns_status status = ldns_rr_new_frm_str(&rr, str, 0, NULL, NULL);
	if(status != LDNS_STATUS_OK) {
		log_err("error parsing trust anchor: %s", 
			ldns_get_errorstr_by_id(status));
		ldns_rr_free(rr);
		return 0;
	}
	if(!anchor_store_new_rr(anchors, buffer, rr)) {
		log_err("out of memory");
		ldns_rr_free(rr);
		return 0;
	}
	ldns_rr_free(rr);
	return 1;
}

/**
 * Read a file with trust anchors
 * @param anchors: anchor storage.
 * @param buffer: parsing buffer.
 * @param fname: string.
 * @return false on error.
 */
static int
anchor_read_file(struct val_anchors* anchors, ldns_buffer* buffer,
	const char* fname)
{
	uint32_t default_ttl = 3600;
	ldns_rdf* origin = NULL, *prev = NULL;
	int line_nr = 1;
	ldns_status status;
	ldns_rr* rr;
	int ok = 1;
	FILE* in = fopen(fname, "r");
	if(!in) {
		log_err("error opening file %s: %s", fname, strerror(errno));
		return 0;
	}
	while(!feof(in)) {
		rr = NULL;
		status = ldns_rr_new_frm_fp_l(&rr, in, &default_ttl, &origin,
			&prev, &line_nr);
		if(status == LDNS_STATUS_SYNTAX_EMPTY /* empty line */
			|| status == LDNS_STATUS_SYNTAX_TTL /* $TTL */
			|| status == LDNS_STATUS_SYNTAX_ORIGIN /* $ORIGIN */)
			continue;
		if(status != LDNS_STATUS_OK) {
			log_err("parse error in %s:%d : %s", fname, line_nr,
				ldns_get_errorstr_by_id(status));
			ldns_rr_free(rr);
			ok = 0;
			break;
		}
		if(ldns_rr_get_type(rr) != LDNS_RR_TYPE_DS && 
			ldns_rr_get_type(rr) != LDNS_RR_TYPE_DNSKEY) {
			ldns_rr_free(rr);
			continue;
		}
		if(!anchor_store_new_rr(anchors, buffer, rr)) {
			log_err("error at %s line %d", fname, line_nr);
			ldns_rr_free(rr);
			ok = 0;
			break;
		}
		ldns_rr_free(rr);
	}
	ldns_rdf_deep_free(origin);
	ldns_rdf_deep_free(prev);
	fclose(in);
	return ok;
}

/** 
 * Assemble an rrset structure for the type 
 * @param region: allocated in this region.
 * @param ta: trust anchor.
 * @param num: number of items to fetch from list.
 * @param type: fetch only items of this type.
 * @return rrset or NULL on error.
 */
static struct ub_packed_rrset_key*
assemble_it(struct region* region, struct trust_anchor* ta, size_t num, 
	uint16_t type)
{
	struct ub_packed_rrset_key* pkey = (struct ub_packed_rrset_key*)
		region_alloc(region, sizeof(*pkey));
	struct packed_rrset_data* pd;
	struct ta_key* tk;
	size_t i;
	if(!pkey)
		return NULL;
	memset(pkey, 0, sizeof(*pkey));
	pkey->rk.dname = region_alloc_init(region, ta->name, ta->namelen);
	if(!pkey->rk.dname)
		return NULL;
	
	pkey->rk.dname_len = ta->namelen;
	pkey->rk.type = htons(type);
	pkey->rk.rrset_class = htons(ta->dclass);
	/* The rrset is build in an uncompressed way. This means it
	 * cannot be copied in the normal way. */
	pd = (struct packed_rrset_data*)region_alloc(region, sizeof(*pd));
	if(!pd)
		return NULL;
	memset(pd, 0, sizeof(*pd));
	pd->count = num;
	pd->trust = rrset_trust_ultimate;
	pd->rr_len = (size_t*)region_alloc(region, num*sizeof(size_t));
	if(!pd->rr_len)
		return NULL;
	pd->rr_ttl = (uint32_t*)region_alloc(region, num*sizeof(uint32_t));
	if(!pd->rr_ttl)
		return NULL;
	pd->rr_data = (uint8_t**)region_alloc(region, num*sizeof(uint8_t*));
	if(!pd->rr_data)
		return NULL;
	/* fill in rrs */
	i=0;
	for(tk = ta->keylist; tk; tk = tk->next) {
		if(tk->type != type)
			continue;
		pd->rr_len[i] = tk->len;
		/* reuse data ptr to allocation in region */
		pd->rr_data[i] = tk->data;
		pd->rr_ttl[i] = 0;
		i++;
	}
	pkey->entry.data = (void*)pd;
	return pkey;
}

/**
 * Assemble structures for the trust DS and DNSKEY rrsets.
 * @param anchors: trust anchor storage.
 * @param ta: trust anchor
 * @return: false on error.
 */
static int
anchors_assemble(struct val_anchors* anchors, struct trust_anchor* ta)
{
	if(ta->numDS > 0) {
		ta->ds_rrset = assemble_it(anchors->region, ta,
			ta->numDS, LDNS_RR_TYPE_DS);
		if(!ta->ds_rrset)
			return 0;
	}
	if(ta->numDNSKEY > 0) {
		ta->dnskey_rrset = assemble_it(anchors->region, ta,
			ta->numDNSKEY, LDNS_RR_TYPE_DNSKEY);
		if(!ta->dnskey_rrset)
			return 0;
	}
	return 1;
}

/**
 * Assemble the rrsets in the anchors, ready for use by validator.
 * @param anchors: trust anchor storage.
 * @return: false on error.
 */
static int
anchors_assemble_rrsets(struct val_anchors* anchors)
{
	struct trust_anchor* ta;
	RBTREE_FOR(ta, struct trust_anchor*, anchors->tree) {
		if(!anchors_assemble(anchors, ta)) {
			log_err("out of memory");
			return 0;
		}
	}
	return 1;
}

int 
anchors_apply_cfg(struct val_anchors* anchors, struct config_file* cfg)
{
	struct config_strlist* f;
	ldns_buffer* parsebuf = ldns_buffer_new(65535);
	for(f = cfg->trust_anchor_file_list; f; f = f->next) {
		if(!f->str || f->str[0] == 0) /* empty "" */
			continue;
		if(!anchor_read_file(anchors, parsebuf, f->str)) {
			log_err("error reading trust-anchor-file: %s", f->str);
			ldns_buffer_free(parsebuf);
			return 0;
		}
	}
	for(f = cfg->trust_anchor_list; f; f = f->next) {
		if(!f->str || f->str[0] == 0) /* empty "" */
			continue;
		if(!anchor_store_str(anchors, parsebuf, f->str)) {
			log_err("error in trust-anchor: \"%s\"", f->str);
			ldns_buffer_free(parsebuf);
			return 0;
		}
	}
	init_parents(anchors);
	anchors_assemble_rrsets(anchors);
	ldns_buffer_free(parsebuf);
	return 1;
}

struct trust_anchor* 
anchors_lookup(struct val_anchors* anchors,
        uint8_t* qname, size_t qname_len, uint16_t qclass)
{
	struct trust_anchor key;
	struct trust_anchor* result;
	rbnode_t* res = NULL;
	key.node.key = &key;
	key.name = qname;
	key.namelabs = dname_count_labels(qname);
	key.namelen = qname_len;
	key.dclass = qclass;
	if(rbtree_find_less_equal(anchors->tree, &key, &res)) {
		/* exact */
		result = (struct trust_anchor*)res;
	} else {
		/* smaller element (or no element) */
		int m;
		result = (struct trust_anchor*)res;
		if(!result || result->dclass != qclass)
			return NULL;
		/* count number of labels matched */
		(void)dname_lab_cmp(result->name, result->namelabs, key.name,
			key.namelabs, &m);
		while(result) { /* go up until qname is subdomain of stub */
			if(result->namelabs <= m)
				break;
			result = result->parent;
		}
	}
	return result;
}
