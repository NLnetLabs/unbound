/*
 * iterator/iter_donotq.c - iterative resolver donotqueryaddresses storage.
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
 * The donotqueryaddresses are stored and looked up. These addresses
 * (like 127.0.0.1) must not be used to send queries to, and can be
 * discarded immediately from the server selection.
 */
#include "config.h"
#include "iterator/iter_donotq.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"

int
donotq_cmp(const void* k1, const void* k2)
{
	struct iter_donotq_addr* n1 = (struct iter_donotq_addr*)k1;
	struct iter_donotq_addr* n2 = (struct iter_donotq_addr*)k2;
	int r = sockaddr_cmp_addr(&n1->addr, n1->addrlen, &n2->addr, 
		n2->addrlen);
	if(r != 0) return r;
	if(n1->net < n2->net)
		return -1;
	if(n1->net > n2->net)
		return 1;
	return 0;
}

struct iter_donotq* 
donotq_create()
{
	struct iter_donotq* dq = (struct iter_donotq*)calloc(1,
		sizeof(struct iter_donotq));
	if(!dq)
		return NULL;
	dq->region = regional_create();
	if(!dq->region) {
		donotq_delete(dq);
		return NULL;
	}
	return dq;
}

void 
donotq_delete(struct iter_donotq* dq)
{
	if(!dq) 
		return;
	regional_destroy(dq->region);
	free(dq->tree);
	free(dq);
}

/** insert new address into donotq structure */
static int
donotq_insert(struct iter_donotq* dq, struct sockaddr_storage* addr, 
	socklen_t addrlen, int net)
{
	struct iter_donotq_addr* node = regional_alloc(dq->region,
		sizeof(struct iter_donotq_addr));
	if(!node)
		return 0;
	node->node.key = node;
	memcpy(&node->addr, addr, addrlen);
	node->addrlen = addrlen;
	node->net = net;
	node->parent = NULL;
	if(!rbtree_insert(dq->tree, &node->node)) {
		verbose(VERB_QUERY, "duplicate donotquery address ignored.");
	}
	return 1;
}

/** apply donotq string */
static int
donotq_str_cfg(struct iter_donotq* dq, const char* str)
{
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;
	if(!netblockstrtoaddr(str, UNBOUND_DNS_PORT, &addr, &addrlen, &net)) {
		log_err("cannot parse donotquery netblock: %s", str);
		return 0;
	}
	if(!donotq_insert(dq, &addr, addrlen, net)) {
		log_err("out of memory");
		return 0;
	}
	return 1;
}

/** read donotq config */
static int 
read_donotq(struct iter_donotq* dq, struct config_file* cfg)
{
	struct config_strlist* p;
	for(p = cfg->donotqueryaddrs; p; p = p->next) {
		log_assert(p->str);
		if(!donotq_str_cfg(dq, p->str))
			return 0;
	}
	return 1;
}

/** initialise parent pointers in the tree */
static void
donotq_init_parents(struct iter_donotq* donotq)
{
	struct iter_donotq_addr* node, *prev = NULL, *p;
	int m;
	RBTREE_FOR(node, struct iter_donotq_addr*, donotq->tree) {
		node->parent = NULL;
		if(!prev || prev->addrlen != node->addrlen) {
			prev = node;
			continue;
		}
		m = addr_in_common(&prev->addr, prev->net, &node->addr, 
			node->net, node->addrlen);
		/* sort order like: ::/0, 1::/2, 1::/4, ... 2::/2 */
		/* find the previous, or parent-parent-parent */
		for(p = prev; p; p = p->parent)
			if(p->net <= m) {
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
donotq_apply_cfg(struct iter_donotq* dq, struct config_file* cfg)
{
	free(dq->tree);
	dq->tree = rbtree_create(donotq_cmp);
	if(!dq->tree)
		return 0;
	if(!read_donotq(dq, cfg))
		return 0;
	if(cfg->donotquery_localhost) {
		if(!donotq_str_cfg(dq, "127.0.0.0/8"))
			return 0;
		if(!donotq_str_cfg(dq, "::1"))
			return 0;
	}
	donotq_init_parents(dq);
	return 1;
}

int 
donotq_lookup(struct iter_donotq* donotq, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	/* lookup in the tree */
	rbnode_t* res = NULL;
	struct iter_donotq_addr* result;
	struct iter_donotq_addr key;
	key.node.key = &key;
	memcpy(&key.addr, addr, addrlen);
	key.addrlen = addrlen;
	key.net = (addr_is_ip6(addr, addrlen)?128:32);
	if(rbtree_find_less_equal(donotq->tree, &key, &res)) {
		/* exact */
		return 1;
	} else {
		/* smaller element (or no element) */
		int m;
		result = (struct iter_donotq_addr*)res;
		if(!result || result->addrlen != addrlen)
			return 0;
		/* count number of bits matched */
		m = addr_in_common(&result->addr, result->net, addr, 
			key.net, addrlen);
		while(result) { /* go up until addr is inside netblock */
			if(result->net <= m)
				return 1;
			result = result->parent;
		}
	}
	return 0;
}

size_t 
donotq_get_mem(struct iter_donotq* donotq)
{
	if(!donotq) return 0;
	return sizeof(*donotq) + regional_get_mem(donotq->region);
}
