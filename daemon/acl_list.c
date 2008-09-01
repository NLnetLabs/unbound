/*
 * daemon/acl_list.h - client access control storage for the server.
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
 * This file helps the server keep out queries from outside sources, that
 * should not be answered.
 */
#include "config.h"
#include "daemon/acl_list.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"

int
acl_list_cmp(const void* k1, const void* k2)
{
	struct acl_addr* n1 = (struct acl_addr*)k1;
	struct acl_addr* n2 = (struct acl_addr*)k2;
	int r = sockaddr_cmp_addr(&n1->addr, n1->addrlen, &n2->addr, 
		n2->addrlen);
	if(r != 0) return r;
	if(n1->net < n2->net)
		return -1;
	if(n1->net > n2->net)
		return 1;
	return 0;
}

struct acl_list* 
acl_list_create()
{
	struct acl_list* acl = (struct acl_list*)calloc(1,
		sizeof(struct acl_list));
	if(!acl)
		return NULL;
	acl->region = regional_create();
	if(!acl->region) {
		acl_list_delete(acl);
		return NULL;
	}
	return acl;
}

void 
acl_list_delete(struct acl_list* acl)
{
	if(!acl) 
		return;
	regional_destroy(acl->region);
	free(acl->tree);
	free(acl);
}

/** insert new address into acl_list structure */
static int
acl_list_insert(struct acl_list* acl, struct sockaddr_storage* addr, 
	socklen_t addrlen, int net, enum acl_access control, 
	int complain_duplicates)
{
	struct acl_addr* node = regional_alloc(acl->region,
		sizeof(struct acl_addr));
	if(!node)
		return 0;
	node->node.key = node;
	memcpy(&node->addr, addr, addrlen);
	node->addrlen = addrlen;
	node->net = net;
	node->parent = NULL;
	node->control = control;
	if(!rbtree_insert(acl->tree, &node->node)) {
		if(complain_duplicates)
			verbose(VERB_QUERY, "duplicate acl address ignored.");
	}
	return 1;
}

/** apply acl_list string */
static int
acl_list_str_cfg(struct acl_list* acl, const char* str, const char* s2,
	int complain_duplicates)
{
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;
	enum acl_access control;
	if(strcmp(s2, "allow") == 0)
		control = acl_allow;
	else if(strcmp(s2, "deny") == 0)
		control = acl_deny;
	else if(strcmp(s2, "refuse") == 0)
		control = acl_refuse;
	else if(strcmp(s2, "allow_snoop") == 0)
		control = acl_allow_snoop;
	else {
		log_err("access control type %s unknown", str);
		return 0;
	}
	if(!netblockstrtoaddr(str, UNBOUND_DNS_PORT, &addr, &addrlen, &net)) {
		log_err("cannot parse access control: %s %s", str, s2);
		return 0;
	}
	if(!acl_list_insert(acl, &addr, addrlen, net, control, 
		complain_duplicates)) {
		log_err("out of memory");
		return 0;
	}
	return 1;
}

/** read acl_list config */
static int 
read_acl_list(struct acl_list* acl, struct config_file* cfg)
{
	struct config_str2list* p;
	for(p = cfg->acls; p; p = p->next) {
		log_assert(p->str && p->str2);
		if(!acl_list_str_cfg(acl, p->str, p->str2, 1))
			return 0;
	}
	return 1;
}

/** initialise parent pointers in the tree */
static void
acl_list_init_parents(struct acl_list* acl)
{
	struct acl_addr* node, *prev = NULL, *p;
	int m;
	RBTREE_FOR(node, struct acl_addr*, acl->tree) {
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
acl_list_apply_cfg(struct acl_list* acl, struct config_file* cfg)
{
	regional_free_all(acl->region);
	free(acl->tree);
	acl->tree = rbtree_create(acl_list_cmp);
	if(!acl->tree)
		return 0;
	if(!read_acl_list(acl, cfg))
		return 0;
	/* insert defaults, with '0' to ignore them if they are duplicates */
	if(!acl_list_str_cfg(acl, "0.0.0.0/0", "refuse", 0))
		return 0;
	if(!acl_list_str_cfg(acl, "127.0.0.0/8", "allow", 0))
		return 0;
	if(cfg->do_ip6) {
		if(!acl_list_str_cfg(acl, "::0/0", "refuse", 0))
			return 0;
		if(!acl_list_str_cfg(acl, "::1", "allow", 0))
			return 0;
		if(!acl_list_str_cfg(acl, "::ffff:127.0.0.1", "allow", 0))
			return 0;
	}
	acl_list_init_parents(acl);
	return 1;
}

enum acl_access 
acl_list_lookup(struct acl_list* acl, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	/* lookup in the tree */
	rbnode_t* res = NULL;
	struct acl_addr* result;
	struct acl_addr key;
	key.node.key = &key;
	memcpy(&key.addr, addr, addrlen);
	key.addrlen = addrlen;
	key.net = (addr_is_ip6(addr, addrlen)?128:32);
	if(rbtree_find_less_equal(acl->tree, &key, &res)) {
		/* exact */
		result = (struct acl_addr*)res;
		return result->control;
	} else {
		/* smaller element (or no element) */
		int m;
		result = (struct acl_addr*)res;
		if(!result || result->addrlen != addrlen)
			return acl_deny;
		/* count number of bits matched */
		m = addr_in_common(&result->addr, result->net, addr, 
			key.net, addrlen);
		while(result) { /* go up until addr is inside netblock */
			if(result->net <= m)
				return result->control;
			result = result->parent;
		}
	}
	return acl_deny;
}

size_t 
acl_list_get_mem(struct acl_list* acl)
{
	if(!acl) return 0;
	return sizeof(*acl) + regional_get_mem(acl->region);
}
