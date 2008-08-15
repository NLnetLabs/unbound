/*
 * validator/val_neg.h - validator aggressive negative caching functions.
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
 * This file contains helper functions for the validator module.
 * The functions help with aggressive negative caching.
 * This creates new denials of existance, and proofs for absence of types
 * from cached NSEC records.
 */

#ifndef VALIDATOR_VAL_NEG_H
#define VALIDATOR_VAL_NEG_H
#include "util/locks.h"
#include "util/rbtree.h"
#include "util/data/msgreply.h"
struct val_neg_data;
struct config_file;
struct reply_info;

/**
 * The negative cache. It is shared between the threads, so locked. 
 * Kept as validator-module-state.  It refers back to the rrset cache for
 * data elements.  It can be out of date and contain conflicting data 
 * from zone content changes.  
 */
struct val_neg_cache {
	/** the big lock on the negative cache
	 * Because we use a rbtree for the data (quick lookup), we need
	 * a big lock */
	lock_basic_t lock;
	/** The data rbtree for NSEC. contents of type val_neg_data */
	rbtree_t tree;
	/** the first and last in linked list of LRU of val_neg_data */
	struct val_neg_data* first;
	/** last in lru (least recently used element) */
	struct val_neg_data* last;
	/** current memory in use (bytes) */
	size_t use;
	/** max memory to use (bytes) */
	size_t max;
};

/**
 * Data element for aggressive negative caching.
 */
struct val_neg_data {
	/** rbtree node element, key is this struct: the name */
	rbnode_t node;
	/** name; the key */
	uint8_t* name;
	/** length of name */
	size_t len;
	/** labels in name */
	int labs;

	/** pointer to parent node in the negative cache */
	struct val_neg_data* parent;

	/** previous in LRU */
	struct val_neg_data* prev;
	/** next in LRU (next element was less recently used) */
	struct val_neg_data* next;

	/** reference to NSEC rrset (parent side of a zone cut) */
	struct rrset_ref nsec_above;
	/** reference to SOA record for zone of nsec_above */
	struct rrset_ref soa_above;
	/** reference to NSEC rrset (child side of a zone cut - or elsewhere) */
	struct rrset_ref nsec_below;
	/** reference to SOA record for zone of nsec_below */
	struct rrset_ref soa_below;

	/** class of node; host order */
	uint16_t dclass;
};

/**
 * Create negative cache
 * @return neg cache, empty or NULL on failure.
 */
struct val_neg_cache* val_neg_create();

/**
 * Apply configuration settings to negative cache
 * @param neg: negative cache.
 * @param cfg: config options.
 * @return false on error.
 */
int val_neg_apply_cfg(struct val_neg_cache* neg, struct config_file* cfg);

/**
 * see how much memory is in use by the negative cache.
 * @param neg: negative cache
 * @return number of bytes in use.
 */
size_t val_neg_get_mem(struct val_neg_cache* neg);

/**
 * Destroy negative cache. There must no longer be any other threads.
 * @param neg: negative cache.
 */
void neg_cache_delete(struct val_neg_cache* neg);

/** 
 * Comparison function for rbtree val neg data elements
 */
int val_neg_compare(const void* a, const void* b);

/**
 * Insert NSECs from this message into the negative cache for reference.
 * @param neg: negative cache
 * @param rep: reply with NSECs.
 * Errors are ignored, means that storage is omitted.
 */
void val_neg_addreply(struct val_neg_cache* neg, struct reply_info* rep);

/**
 * Perform a DLV style lookup
 * During the lookup, we could find out that data has expired. In that
 * case the neg_cache entries are removed, and lookup fails.
 *
 * @param neg: negative cache.
 * @param qname: name to look for
 * @param len: length of qname.
 * @param qclass: class to look in.
 * @return 
 *	0 on error
 *	0 if no proof of negative
 *	1 if indeed negative was proven
 *	  thus, qname DLV qclass does not exist.
 */
int val_neg_dlvlookup(struct val_neg_cache* neg, uint8_t* qname, size_t len,
	uint16_t qclass);

#endif /* VALIDATOR_VAL_NEG_H */
