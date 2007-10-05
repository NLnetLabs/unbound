/*
 * iterator/iter_hints.h - iterative resolver module stub and root hints.
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

#ifndef ITERATOR_ITER_HINTS_H
#define ITERATOR_ITER_HINTS_H
#include "util/rbtree.h"
struct iter_env;
struct config_file;
struct delegpt;
struct region;

/**
 * Iterator hints structure
 */
struct iter_hints {
	/** region where hints are allocated */
	struct region* region;
	/** 
	 * Hints are stored in this tree. Sort order is specially chosen.
	 * first sorted on qtype. Then on dname in nsec-like order, so that
	 * a lookup on class, name will return an exact match or the closest
	 * match which gives the ancestor needed.
	 * contents of type iter_hints_stub. The class IN root is in here.
	 */
	rbtree_t* tree;
};

/**
 * Iterator hints for a particular stub.
 */
struct iter_hints_stub {
	/** redblacktree node, key is this structure: class and name */
	rbnode_t node;
	/** name */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** number of labels in name */
	int namelabs;
	/** delegation point with hint information for this stub. */
	struct delegpt* dp;
	/** pointer to parent in stub hint tree (or NULL if none) */
	struct iter_hints_stub* parent;
	/** class of hints. host order. */
	uint16_t hint_class;
};

/**
 * Create hints 
 * @return new hints or NULL on error.
 */
struct iter_hints* hints_create();

/**
 * Delete hints.
 * @param hints: to delete.
 */
void hints_delete(struct iter_hints* hints);

/**
 * Process hints config. Sets default values for root hints if no config.
 * @param hints: where to store.
 * @param cfg: config options.
 * @return 0 on error.
 */
int hints_apply_cfg(struct iter_hints* hints, struct config_file* cfg);

/**
 * Find root hints for the given class.
 * @param hints: hint storage.
 * @param qclass: class for which root hints are requested. host order.
 * @return: NULL if no hints, or a ptr to stored hints.
 */
struct delegpt* hints_lookup_root(struct iter_hints* hints, uint16_t qclass);

/**
 * Given a qname/qclass combination, and the delegation point from the cache
 * for this qname/qclass, determine if this combination indicates that a
 * stub hint exists and must be primed.
 *
 * @param hints: hint storage.
 * @param qname: The qname that generated the delegation point.
 * @param qclass: The qclass that generated the delegation point.
 * @param dp: The cache generated delegation point.
 * @return: A priming delegation point if there is a stub hint that must
 *         be primed, otherwise null.
 */
struct delegpt* hints_lookup_stub(struct iter_hints* hints, 
	uint8_t* qname, uint16_t qclass, struct delegpt* dp);

/**
 * Get memory in use by hints
 * @param hints: hint storage.
 * @return bytes in use
 */
size_t hints_get_mem(struct iter_hints* hints);

/** compare two hint entries */
int stub_cmp(const void* k1, const void* k2);

#endif /* ITERATOR_ITER_HINTS_H */
