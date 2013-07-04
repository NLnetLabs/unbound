/*
 * subnet-whitelist -- Hosts we actively try to send subnet option to.
 *
 * Copyright (c) 2013, NLnet Labs.  See LICENSE for license.
 */

/**
 * \file
 *
 * Keep track of the white listed servers for subnet option. Based
 * on acl_list.c|h
 */

#include "config.h"

#ifdef CLIENT_SUBNET /* keeps splint happy */
#include "edns-subnet/edns-subnet.h"
#include "edns-subnet/subnet-whitelist.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "util/storage/dnstree.h"

struct ednssubnet_upstream* 
upstream_create(void)
{
	struct ednssubnet_upstream* upstream = 
		(struct ednssubnet_upstream*)calloc(1,
		sizeof(struct ednssubnet_upstream));
	if(!upstream)
		return NULL;
	upstream->region = regional_create();
	if(!upstream->region) {
		upstream_delete(upstream);
		return NULL;
	}
	return upstream;
}

void 
upstream_delete(struct ednssubnet_upstream* upstream)
{
	if(!upstream) 
		return;
	regional_destroy(upstream->region);
	free(upstream);
}

/** insert new address into upstream structure */
static int
upstream_insert(struct ednssubnet_upstream* upstream, struct sockaddr_storage* addr, 
	socklen_t addrlen, int net)
{
	struct addr_tree_node* node = (struct addr_tree_node*)regional_alloc(
		upstream->region, sizeof(*node));
	if(!node)
		return 0;
	if(!addr_tree_insert(&upstream->tree, node, addr, addrlen, net)) {
		verbose(VERB_QUERY, "duplicate send-client-subnet address ignored.");
	}
	return 1;
}

/** apply edns-subnet string */
static int
upstream_str_cfg(struct ednssubnet_upstream* upstream, const char* str)
{
	struct sockaddr_storage addr;
	int net;
	socklen_t addrlen;
	verbose(VERB_ALGO, "send-client-subnet: %s", str);
	if(!netblockstrtoaddr(str, UNBOUND_DNS_PORT, &addr, &addrlen, &net)) {
		log_err("cannot parse send-client-subnet netblock: %s", str);
		return 0;
	}
	if(!upstream_insert(upstream, &addr, addrlen, net)) {
		log_err("out of memory");
		return 0;
	}
	return 1;
}

/** read client_subnet config */
static int 
read_upstream(struct ednssubnet_upstream* upstream, struct config_file* cfg)
{
	struct config_strlist* p;
	for(p = cfg->client_subnet; p; p = p->next) {
		log_assert(p->str);
		if(!upstream_str_cfg(upstream, p->str))
			return 0;
	}
	return 1;
}

int 
upstream_apply_cfg(struct ednssubnet_upstream* upstream, struct config_file* cfg)
{
	regional_free_all(upstream->region);
	addr_tree_init(&upstream->tree);
	if(!read_upstream(upstream, cfg))
		return 0;
	addr_tree_init_parents(&upstream->tree);
	return 1;
}

int 
upstream_is_whitelisted(struct ednssubnet_upstream* upstream, struct sockaddr_storage* addr,
        socklen_t addrlen)
{
	return addr_tree_lookup(&upstream->tree, addr, addrlen) != NULL;
}

size_t 
upstream_get_mem(struct ednssubnet_upstream* upstream)
{
	if(!upstream) return 0;
	return sizeof(*upstream) + regional_get_mem(upstream->region);
}

#endif /* CLIENT_SUBNET */
