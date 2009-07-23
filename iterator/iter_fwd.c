/*
 * iterator/iter_fwd.c - iterative resolver module forward zones.
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
 * This file contains functions to assist the iterator module.
 * Keep track of forward zones and config settings.
 */
#include "config.h"
#include "iterator/iter_fwd.h"
#include "iterator/iter_delegpt.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "util/data/dname.h"

int
fwd_cmp(const void* k1, const void* k2)
{
	int m;
	struct iter_forward_zone* n1 = (struct iter_forward_zone*)k1;
	struct iter_forward_zone* n2 = (struct iter_forward_zone*)k2;
	if(n1->dclass != n2->dclass) {
		if(n1->dclass < n2->dclass)
			return -1;
		return 1;
	}
	return dname_lab_cmp(n1->name, n1->namelabs, n2->name, n2->namelabs, 
		&m);
}

struct iter_forwards* 
forwards_create()
{
	struct iter_forwards* fwd = (struct iter_forwards*)calloc(1,
		sizeof(struct iter_forwards));
	if(!fwd)
		return NULL;
	fwd->region = regional_create();
	if(!fwd->region) {
		forwards_delete(fwd);
		return NULL;
	}
	return fwd;
}

void 
forwards_delete(struct iter_forwards* fwd)
{
	if(!fwd) 
		return;
	regional_destroy(fwd->region);
	free(fwd->tree);
	free(fwd);
}

/** insert info into forward structure */
static int
forwards_insert_data(struct iter_forwards* fwd, uint16_t c, uint8_t* nm, 
	size_t nmlen, int nmlabs, struct delegpt* dp)
{
	struct iter_forward_zone* node = regional_alloc(fwd->region,
		sizeof(struct iter_forward_zone));
	if(!node)
		return 0;
	node->node.key = node;
	node->dclass = c;
	node->name = regional_alloc_init(fwd->region, nm, nmlen);
	if(!node->name)
		return 0;
	node->namelen = nmlen;
	node->namelabs = nmlabs;
	node->dp = dp;
	if(!rbtree_insert(fwd->tree, &node->node)) {
		log_err("duplicate forward zone ignored.");
	}
	return 1;
}

/** insert new info into forward structure given dp */
static int
forwards_insert(struct iter_forwards* fwd, uint16_t c, struct delegpt* dp)
{
	return forwards_insert_data(fwd, c, dp->name, dp->namelen,
		dp->namelabs, dp);
}

/** initialise parent pointers in the tree */
static void
fwd_init_parents(struct iter_forwards* fwd)
{
	struct iter_forward_zone* node, *prev = NULL, *p;
	int m;
	RBTREE_FOR(node, struct iter_forward_zone*, fwd->tree) {
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

/** set zone name */
static int 
read_fwds_name(struct iter_forwards* fwd, struct config_stub* s, 
	struct delegpt* dp)
{
	ldns_rdf* rdf;
	if(!s->name) {
		log_err("forward zone without a name (use name \".\" to forward everything)");
		return 0;
	}
	rdf = ldns_dname_new_frm_str(s->name);
	if(!rdf) {
		log_err("cannot parse forward zone name %s", s->name);
		return 0;
	}
	if(!delegpt_set_name(dp, fwd->region, ldns_rdf_data(rdf))) {
		ldns_rdf_deep_free(rdf);
		log_err("out of memory");
		return 0;
	}
	ldns_rdf_deep_free(rdf);
	return 1;
}

/** set fwd host names */
static int 
read_fwds_host(struct iter_forwards* fwd, struct config_stub* s, 
	struct delegpt* dp)
{
	struct config_strlist* p;
	ldns_rdf* rdf;
	for(p = s->hosts; p; p = p->next) {
		log_assert(p->str);
		rdf = ldns_dname_new_frm_str(p->str);
		if(!rdf) {
			log_err("cannot parse forward %s server name: '%s'", 
				s->name, p->str);
			return 0;
		}
		if(!delegpt_add_ns(dp, fwd->region, ldns_rdf_data(rdf))) {
			ldns_rdf_deep_free(rdf);
			log_err("out of memory");
			return 0;
		}
		ldns_rdf_deep_free(rdf);
	}
	return 1;
}

/** set fwd server addresses */
static int 
read_fwds_addr(struct iter_forwards* fwd, struct config_stub* s, 
	struct delegpt* dp)
{
	struct config_strlist* p;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	for(p = s->addrs; p; p = p->next) {
		log_assert(p->str);
		if(!extstrtoaddr(p->str, &addr, &addrlen)) {
			log_err("cannot parse forward %s ip address: '%s'", 
				s->name, p->str);
			return 0;
		}
		if(!delegpt_add_addr(dp, fwd->region, &addr, addrlen, 0, 1)) {
			log_err("out of memory");
			return 0;
		}
	}
	return 1;
}

/** read forwards config */
static int 
read_forwards(struct iter_forwards* fwd, struct config_file* cfg)
{
	struct config_stub* s;
	for(s = cfg->forwards; s; s = s->next) {
		struct delegpt* dp = delegpt_create(fwd->region);
		if(!dp) {
			log_err("out of memory");
			return 0;
		}
		if(!read_fwds_name(fwd, s, dp) ||
			!read_fwds_host(fwd, s, dp) ||
			!read_fwds_addr(fwd, s, dp))
			return 0;
		if(!forwards_insert(fwd, LDNS_RR_CLASS_IN, dp))
			return 0;
		verbose(VERB_QUERY, "Forward zone server list:");
		delegpt_log(VERB_QUERY, dp);
	}
	return 1;
}

/** see if zone needs to have a hole inserted */
static int
need_hole_insert(rbtree_t* tree, struct iter_forward_zone* zone)
{
	struct iter_forward_zone k;
	if(rbtree_search(tree, zone))
		return 0; /* exact match exists */
	k = *zone;
	k.node.key = &k;
	/* search up the tree */
	do {
		dname_remove_label(&k.name, &k.namelen);
		k.namelabs --;
		if(rbtree_search(tree, &k))
			return 1; /* found an upper forward zone, need hole */
	} while(k.namelabs > 1);
	return 0; /* no forwards above, no holes needed */
}

/** make NULL entries for stubs */
static int
make_stub_holes(struct iter_forwards* fwd, struct config_file* cfg)
{
	struct config_stub* s;
	struct iter_forward_zone key;
	key.node.key = &key;
	key.dclass = LDNS_RR_CLASS_IN;
	for(s = cfg->stubs; s; s = s->next) {
		ldns_rdf* rdf = ldns_dname_new_frm_str(s->name);
		if(!rdf) {
			log_err("cannot parse stub name '%s'", s->name);
			return 0;
		}
		key.name = ldns_rdf_data(rdf);
		key.namelabs = dname_count_size_labels(key.name, &key.namelen);
		if(!need_hole_insert(fwd->tree, &key)) {
			ldns_rdf_deep_free(rdf);
			continue;
		}
		if(!forwards_insert_data(fwd, key.dclass, key.name, 
			key.namelen, key.namelabs, NULL)) {
			ldns_rdf_deep_free(rdf);
			log_err("out of memory");
			return 0;
		}
		ldns_rdf_deep_free(rdf);
	}
	return 1;
}

int 
forwards_apply_cfg(struct iter_forwards* fwd, struct config_file* cfg)
{
	free(fwd->tree);
	regional_free_all(fwd->region);
	fwd->tree = rbtree_create(fwd_cmp);
	if(!fwd->tree)
		return 0;

	/* read forward zones */
	if(!read_forwards(fwd, cfg))
		return 0;
	if(!make_stub_holes(fwd, cfg))
		return 0;
	fwd_init_parents(fwd);
	return 1;
}

struct delegpt* 
forwards_lookup(struct iter_forwards* fwd, uint8_t* qname, uint16_t qclass)
{
	/* lookup the forward zone in the tree */
	rbnode_t* res = NULL;
	struct iter_forward_zone *result;
	struct iter_forward_zone key;
	key.node.key = &key;
	key.dclass = qclass;
	key.name = qname;
	key.namelabs = dname_count_size_labels(qname, &key.namelen);
	if(rbtree_find_less_equal(fwd->tree, &key, &res)) {
		/* exact */
		result = (struct iter_forward_zone*)res;
	} else {
		/* smaller element (or no element) */
		int m;
		result = (struct iter_forward_zone*)res;
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
	if(result)
		return result->dp;
	return NULL;
}

struct delegpt* 
forwards_lookup_root(struct iter_forwards* fwd, uint16_t qclass)
{
	uint8_t root = 0;
	return forwards_lookup(fwd, &root, qclass);
}

size_t 
forwards_get_mem(struct iter_forwards* fwd)
{
	if(!fwd)
		return 0;
	return sizeof(*fwd) + sizeof(*fwd->tree) + 
		regional_get_mem(fwd->region);
}

int 
forwards_add_zone(struct iter_forwards* fwd, uint16_t c, struct delegpt* dp)
{
	if(!forwards_insert(fwd, c, dp))
		return 0;
	fwd_init_parents(fwd);
	return 1;
}

void 
forwards_delete_zone(struct iter_forwards* fwd, uint16_t c, uint8_t* nm)
{
	struct iter_forward_zone key;
	key.node.key = &key;
	key.dclass = c;
	key.name = nm;
	key.namelabs = dname_count_size_labels(nm, &key.namelen);
	if(!rbtree_search(fwd->tree, &key))
		return; /* nothing to do */
	(void)rbtree_delete(fwd->tree, &key);
	fwd_init_parents(fwd);
}

