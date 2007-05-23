/*
 * iterator/iter_hints.c - iterative resolver module stub and root hints.
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
 * Keep track of stub and root hints, and read those from config.
 */
#include "config.h"
#include "iterator/iter_hints.h"
#include "iterator/iter_delegpt.h"
#include "util/region-allocator.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/data/dname.h"

/** compare two hint entries */
static int
stub_cmp(const void* k1, const void* k2)
{
	int m;
	struct iter_hints_stub* n1 = (struct iter_hints_stub*)k1;
	struct iter_hints_stub* n2 = (struct iter_hints_stub*)k2;
	if(n1->hint_class != n2->hint_class) {
		if(n1->hint_class < n2->hint_class)
			return -1;
		return 1;
	}
	return dname_lab_cmp(n1->name, n1->namelabs, n2->name, n2->namelabs, 
		&m);
}

struct iter_hints* 
hints_create()
{
	struct iter_hints* hints = (struct iter_hints*)calloc(1,
		sizeof(struct iter_hints));
	if(!hints)
		return NULL;
	hints->region = region_create(malloc, free);
	if(!hints->region) {
		hints_delete(hints);
		return NULL;
	}
	return hints;
}

void 
hints_delete(struct iter_hints* hints)
{
	if(!hints) 
		return;
	region_destroy(hints->region);
	free(hints->tree);
	free(hints);
}

/** add hint to delegation hints */
static int
ah(struct delegpt* dp, struct region* r, const char* sv, const char* ip)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	ldns_rdf* rdf = ldns_dname_new_frm_str(sv);
	if(!rdf) {
		log_err("could not parse %s", sv);
		return 0;
	}
	if(!delegpt_add_ns(dp, r, ldns_rdf_data(rdf)) ||
	   !ipstrtoaddr(ip, UNBOUND_DNS_PORT, &addr, &addrlen) ||
	   !delegpt_add_target(dp, r, ldns_rdf_data(rdf), ldns_rdf_size(rdf),
		&addr, addrlen)) {
		ldns_rdf_deep_free(rdf);
		return 0;
	}
	ldns_rdf_deep_free(rdf);
	return 1;
}

/** obtain compiletime provided root hints */
static struct delegpt* 
compile_time_root_prime(struct region* r)
{
	/* from:
	 ;       This file is made available by InterNIC
	 ;       under anonymous FTP as
	 ;           file                /domain/named.cache
	 ;           on server           FTP.INTERNIC.NET
	 ;       -OR-                    RS.INTERNIC.NET
	 ;
	 ;       last update:    Jan 29, 2004
	 ;       related version of root zone:   2004012900
	 */
	struct delegpt* dp = delegpt_create(r);
	if(!dp)
		return NULL;
	if(!delegpt_set_name(dp, r, (uint8_t*)"\000"))
		return NULL;
	if(!ah(dp, r, "A.ROOT-SERVERS.NET.", "198.41.0.4"))	return 0;
	if(!ah(dp, r, "B.ROOT-SERVERS.NET.", "192.228.79.201")) return 0;
	if(!ah(dp, r, "C.ROOT-SERVERS.NET.", "192.33.4.12"))	return 0;
	if(!ah(dp, r, "D.ROOT-SERVERS.NET.", "128.8.10.90"))	return 0;
	if(!ah(dp, r, "E.ROOT-SERVERS.NET.", "192.203.230.10")) return 0;
	if(!ah(dp, r, "F.ROOT-SERVERS.NET.", "192.5.5.241"))	return 0;
	if(!ah(dp, r, "G.ROOT-SERVERS.NET.", "192.112.36.4"))	return 0;
	if(!ah(dp, r, "H.ROOT-SERVERS.NET.", "128.63.2.53"))	return 0;
	if(!ah(dp, r, "I.ROOT-SERVERS.NET.", "192.36.148.17"))	return 0;
	if(!ah(dp, r, "J.ROOT-SERVERS.NET.", "192.58.128.30"))	return 0;
	if(!ah(dp, r, "K.ROOT-SERVERS.NET.", "193.0.14.129"))	return 0;
	if(!ah(dp, r, "L.ROOT-SERVERS.NET.", "198.32.64.12"))	return 0;
	if(!ah(dp, r, "M.ROOT-SERVERS.NET.", "202.12.27.33"))	return 0;
	return dp;
}

/** insert new hint info into hint structure */
static int
hints_insert(struct iter_hints* hints, uint16_t c, uint8_t* name, 
	size_t namelen, int namelabs, struct delegpt* dp)
{
	struct iter_hints_stub* node = region_alloc(hints->region,
		sizeof(struct iter_hints_stub));
	if(!node)
		return 0;
	node->node.key = node;
	node->hint_class = c;
	node->name = region_alloc_init(hints->region, name, namelen);
	if(!node->name)
		return 0;
	node->namelen = namelen;
	node->namelabs = namelabs;
	node->dp = dp;
	if(!rbtree_insert(hints->tree, &node->node)) {
		log_err("second hints ignored.");
	}
	return 1;
}

/** initialise parent pointers in the tree */
static void
init_parents(struct iter_hints* hints)
{
	struct iter_hints_stub* node, *prev = NULL, *p;
	int m;
	RBTREE_FOR(node, struct iter_hints_stub*, hints->tree) {
		node->parent = NULL;
		if(!prev || prev->hint_class != node->hint_class) {
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

int 
hints_apply_cfg(struct iter_hints* hints, struct config_file* ATTR_UNUSED(cfg))
{
	struct delegpt* dp;
	free(hints->tree);
	hints->tree = rbtree_create(stub_cmp);
	if(!hints->tree)
		return 0;
	/* TODO: read hints from file named in cfg */

	/* use fallback compiletime root hints */
	dp = compile_time_root_prime(hints->region);
	if(!dp) 
		return 0;
	if(!hints_insert(hints, LDNS_RR_CLASS_IN, dp->name, dp->namelen, 
		dp->namelabs, dp))
		return 0;
	delegpt_log(dp);

	init_parents(hints);
	return 1;
}

struct delegpt* 
hints_lookup_root(struct iter_hints* hints, uint16_t qclass)
{
	uint8_t rootlab = 0;
	struct iter_hints_stub key, *stub;
	key.node.key = &key;
	key.hint_class = qclass;
	key.name = &rootlab;
	key.namelen = 1;
	key.namelabs = 1;
	stub = (struct iter_hints_stub*)rbtree_search(hints->tree, &key);
	if(!stub)
		return NULL;
	return stub->dp;
}
