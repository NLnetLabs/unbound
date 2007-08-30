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
#include "util/region-allocator.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"

/** compare two donotq entries */
static int
donotq_cmp(const void* k1, const void* k2)
{
	struct iter_donotq_addr* n1 = (struct iter_donotq_addr*)k1;
	struct iter_donotq_addr* n2 = (struct iter_donotq_addr*)k2;
	return sockaddr_cmp(&n1->addr, n1->addrlen, &n2->addr, n2->addrlen);
}

struct iter_donotq* 
donotq_create()
{
	struct iter_donotq* dq = (struct iter_donotq*)calloc(1,
		sizeof(struct iter_donotq));
	if(!dq)
		return NULL;
	dq->region = region_create(malloc, free);
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
	region_destroy(dq->region);
	free(dq->tree);
	free(dq);
}

/** insert new address into donotq structure */
static int
donotq_insert(struct iter_donotq* dq, struct sockaddr_storage* addr, 
	socklen_t addrlen)
{
	struct iter_donotq_addr* node = region_alloc(dq->region,
		sizeof(struct iter_donotq_addr));
	if(!node)
		return 0;
	node->node.key = node;
	memcpy(&node->addr, addr, addrlen);
	node->addrlen = addrlen;
	if(!rbtree_insert(dq->tree, &node->node)) {
		log_warn("duplicate donotquery address ignored.");
	}
	return 1;
}

/** read donotq config */
static int 
read_donotq(struct iter_donotq* dq, struct config_file* cfg)
{
	struct config_strlist* p;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	for(p = cfg->donotqueryaddrs; p; p = p->next) {
		log_assert(p->str);
		if(!extstrtoaddr(p->str, &addr, &addrlen)) {
			log_err("cannot parse donotquery ip address: '%s'", 
				p->str);
			return 0;
		}
		if(!donotq_insert(dq, &addr, addrlen)) {
			log_err("out of memory");
			return 0;
		}
	}
	return 1;
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
	return 1;
}

int 
donotq_lookup(struct iter_donotq* donotq, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	/* lookup in the tree */
	struct iter_donotq_addr key;
	key.node.key = &key;
	memcpy(&key.addr, addr, addrlen);
	key.addrlen = addrlen;
	if(rbtree_search(donotq->tree, &key))
		return 1;
	return 0;
}

size_t 
donotq_get_mem(struct iter_donotq* donotq)
{
	if(!donotq) return 0;
	return sizeof(*donotq) + region_get_mem(donotq->region);
}
