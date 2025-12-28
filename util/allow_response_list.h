/*
 * util/allow_response_list.h - allow-response list storage for the server.
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
 * This file keeps track of the list of clients that are allowed to send
 * responses to the server, so the server can store it in the cache.
 */

#ifndef UTIL_ALLOW_RESPONSE_LIST_H
#define UTIL_ALLOW_RESPONSE_LIST_H
#include "util/storage/dnstree.h"
#include "util/locks.h"
struct config_file;
struct regional;

/**
 * allow response list storage structure
 */
struct arl_list {
	/** regional for allocation */
	struct regional* region;
	/**
	 * Tree of the addresses that are allowed to send responses.
	 * contents of type arl_addr.
	 */
	rbtree_type tree;
};

extern const char* TSIG_NOKEY;
extern const char* TSIG_BLOCKED;

/**
 *
 * An address span that is allowed to send responses
 */
struct arl_addr {
	/** node in address tree */
	struct addr_tree_node node;
	/** tsig key name, NULL means no key needed */
	const char* tsig_key_name;
};

/**
 * Create allow response list structure
 * @return new structure or NULL on error.
 */
struct arl_list* arl_list_create(void);

/**
 * Delete allow response list  structure.
 * @param arl: to delete.
 */
void arl_list_delete(struct arl_list* arl);

/**
 * Process allow response list config.
 * @param arl: where to store.
 * @param cfg: config options.
 * @return 0 on error.
 */
int arl_list_apply_cfg(struct arl_list* arl, struct config_file* cfg);

/**
 * Lookup address to see its allow response list structure
 * @param arl: structure for address storage.
 * @param addr: address to check
 * @param addrlen: length of addr.
 * @return: arl structure from this address.
 */
struct arl_addr*
arl_addr_lookup(struct arl_list* arl, struct sockaddr_storage* addr,
        socklen_t addrlen);

/**
 * Get memory used by allow response list structure.
 * @param arl: structure for address storage.
 * @return bytes in use.
 */
size_t arl_list_get_mem(struct arl_list* arl);

/**
 * Swap internal tree with preallocated entries.
 * @param arl: the allow response list structure.
 * @param data: the data structure used to take elements from. This contains
 * 	the old elements on return.
 */
void arl_list_swap_tree(struct arl_list* arl, struct arl_list* data);

#endif /* UTIL_TCP_CONN_LIMIT_H */
