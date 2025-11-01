/*
 * util/allow_response_list.c - allow response list storage for the server.
 *
 * Copyright (c) 2025, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file helps the server discard excess TCP connections.
 */
#include "config.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "util/allow_response_list.h"
#include "services/localzone.h"
#include "sldns/str2wire.h"

const char* TSIG_NOKEY   = "NOKEY";
const char* TSIG_BLOCKED = "BLOCKED";

struct arl_list*
arl_list_create(void)
{
	struct arl_list* arl = (struct arl_list*)calloc(1,
		sizeof(struct arl_list));
	if(!arl)
		return NULL;
	arl->region = regional_create();
	if(!arl->region) {
		arl_list_delete(arl);
		return NULL;
	}
	return arl;
}

static void
arl_list_free_node(rbnode_type* node, void* ATTR_UNUSED(arg))
{
	struct arl_addr* n = (struct arl_addr*) node;
#ifdef THREADS_DISABLED
	(void)n;
#endif
}

void
arl_list_delete(struct arl_list* arl)
{
	if(!arl)
		return;
	traverse_postorder(&arl->tree, arl_list_free_node, NULL);
	regional_destroy(arl->region);
	free(arl);
}

/** insert new address into arl_list structure */
static struct arl_addr*
arl_list_insert(struct arl_list* arl, struct sockaddr_storage* addr,
	socklen_t addrlen, int net, const char* tsig_key_name,
	int complain_duplicates)
{
	struct arl_addr* node = regional_alloc_zero(arl->region,
		sizeof(struct arl_addr));
	if(!node)
		return NULL;
	if(!tsig_key_name)
	       ;
	else if(strcmp(tsig_key_name, TSIG_NOKEY) == 0)
		node->tsig_key_name = TSIG_NOKEY;
	else if(strcmp(tsig_key_name, TSIG_BLOCKED) == 0)
		node->tsig_key_name = TSIG_BLOCKED;
	else
		node->tsig_key_name = tsig_key_name;
	if(!addr_tree_insert(&arl->tree, &node->node, addr, addrlen, net)) {
		if(complain_duplicates)
			verbose(VERB_QUERY, "duplicate arl address ignored.");
	}
	return node;
}

/** apply arl_list string */
static int
arl_list_str_cfg(struct arl_list* arl, const char* str, const char* s2,
	int complain_duplicates)
{
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;
	if(!netblockstrtoaddr(str, UNBOUND_DNS_PORT, &addr, &addrlen, &net)) {
		log_err("cannot parse allow response netblock: %s", str);
		return 0;
	}
	if(!arl_list_insert(arl, &addr, addrlen, net, s2,
		complain_duplicates)) {
		log_err("out of memory");
		return 0;
	}
	return 1;
}

/** read arl_list config */
static int
read_arl_list(struct arl_list* arl, struct config_file* cfg)
{
	struct config_str2list* p;
	for(p = cfg->allow_response_list; p; p = p->next) {
		log_assert(p->str && p->str2);
		if(!arl_list_str_cfg(arl, p->str, p->str2, 1))
			return 0;
	}
	return 1;
}

int
arl_list_apply_cfg(struct arl_list* arl, struct config_file* cfg)
{
	regional_free_all(arl->region);
	addr_tree_init(&arl->tree);
	if(!read_arl_list(arl, cfg))
		return 0;
	addr_tree_init_parents(&arl->tree);
	return 1;
}

struct arl_addr*
arl_addr_lookup(struct arl_list* arl, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	return (struct arl_addr*)addr_tree_lookup(&arl->tree,
		addr, addrlen);
}

size_t
arl_list_get_mem(struct arl_list* arl)
{
	if(!arl) return 0;
	return sizeof(*arl) + regional_get_mem(arl->region);
}

void arl_list_swap_tree(struct arl_list* arl, struct arl_list* data)
{
	/* swap tree and region */
	rbtree_type oldtree = arl->tree;
	struct regional* oldregion = arl->region;
	arl->tree = data->tree;
	arl->region = data->region;
	data->tree = oldtree;
	data->region = oldregion;
}

