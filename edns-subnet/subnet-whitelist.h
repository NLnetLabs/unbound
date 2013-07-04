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

#ifndef EDNSSUBNET_WHITELIST_H
#define EDNSSUBNET_WHITELIST_H
#include "util/storage/dnstree.h"

struct config_file;
struct regional;

/**
 * ednssubnet_upstream structure
 */
struct ednssubnet_upstream {
	/** regional for allocation */
	struct regional* region;
	/** 
	 * Tree of the address spans that are whitelisted.
	 * contents of type addr_tree_node. Each node is an address span 
	 * Unbound will append subnet option for.
	 */
	rbtree_t tree;
};

/**
 * Create ednssubnet_upstream structure 
 * @return new structure or NULL on error.
 */
struct ednssubnet_upstream* upstream_create(void);

/**
 * Delete ednssubnet_upstream structure.
 * @param upstream: to delete.
 */
void upstream_delete(struct ednssubnet_upstream* upstream);

/**
 * Process ednssubnet_upstream config.
 * @param upstream: where to store.
 * @param cfg: config options.
 * @return 0 on error.
 */
int upstream_apply_cfg(struct ednssubnet_upstream* upstream, struct config_file* cfg);

/**
 * See if an address is whitelisted.
 * @param upstream: structure for address storage.
 * @param addr: address to check
 * @param addrlen: length of addr.
 * @return: true if the address is whitelisted for subnet option. 
 */
int upstream_is_whitelisted(struct ednssubnet_upstream* upstream, struct sockaddr_storage* addr,
	socklen_t addrlen);

/**
 * Get memory used by ednssubnet_upstream structure.
 * @param upstream: structure for address storage.
 * @return bytes in use.
 */
size_t upstream_get_mem(struct ednssubnet_upstream* upstream);

#endif /* EDNSSUBNET_WHITELIST_H */
