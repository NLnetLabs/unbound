/*
 * subnetmod -- Vandergaast module. Must be called before validator and 
 * iterator. 
 *
 * Copyright (c) 2013, NLnet Labs.  See LICENSE for license.
 */

/**
 * \file
 * subnet module for unbound.
 */

#ifndef SUBNETMOD_H
#define SUBNETMOD_H
#include "util/module.h"
#include "services/outbound_list.h"
#include "util/net_help.h"
#include "util/storage/slabhash.h"
#include "edns-subnet/addrtree.h"

/**
 * Global state for the subnet module. 
 */
struct subnet_env {
	/** shared message cache */
	struct slabhash* subnet_msg_cache;
};

struct subnet_msg_cache_data {
	struct addrtree* tree4;
	struct addrtree* tree6;
};

struct subnet_qstate {
	/** We need the hash for both cache lookup and insert */
	hashvalue_t qinfo_hash;
};

void subnet_data_delete(void* d, void* ATTR_UNUSED(arg));
size_t msg_cache_sizefunc(void* k, void* d);

/**
 * Get the module function block.
 * @return: function block with function pointers to module methods.
 */
struct module_func_block* subnetmod_get_funcblock(void);

/** subnet module init */
int subnetmod_init(struct module_env* env, int id);

/** subnet module deinit */
void subnetmod_deinit(struct module_env* env, int id);

/** subnet module operate on a query */
void subnetmod_operate(struct module_qstate* qstate, enum module_ev event, int id, struct outbound_entry* outbound);

/** subnet module  */
void subnetmod_inform_super(struct module_qstate* qstate, int id, struct module_qstate* super);

/** subnet module cleanup query state */
void subnetmod_clear(struct module_qstate* qstate, int id);

/** subnet module alloc size routine */
size_t subnetmod_get_mem(struct module_env* env, int id);
#endif /* SUBNETMOD_H */
