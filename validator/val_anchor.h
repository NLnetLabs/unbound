/*
 * validator/val_anchor.h - validator trust anchor storage.
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
 * This file contains storage for the trust anchors for the validator.
 */

#ifndef VALIDATOR_VAL_ANCHOR_H
#define VALIDATOR_VAL_ANCHOR_H
#include "util/rbtree.h"
struct region;
struct trust_anchor;
struct config_file;
struct ub_packed_rrset_key;

/**
 * Trust anchor store.
 */
struct val_anchors {
	/** region where trust anchors are allocated */
	struct region* region;
	/**
	 * Anchors are store in this tree. Sort order is chosen, so that
	 * dnames are in nsec-like order. A lookup on class, name will return
	 * an exact match of the closest match, with the ancestor needed.
	 * contents of type trust_anchor.
	 */
	rbtree_t* tree;
};

/**
 * Trust anchor key
 */
struct ta_key {
	/** next in list */
	struct ta_key* next;
	/** rdata, in wireformat of the key RR. starts with rdlength. */
	uint8_t* data;
	/** length of the rdata (including rdlength). */
	size_t len;
	/** DNS type (host format) of the key, DS or DNSKEY */
	uint16_t type;
};

/**
 * A trust anchor in the trust anchor store.
 * Unique by name, class.
 */
struct trust_anchor {
	/** rbtree node, key is this structure */
	rbnode_t node;
	/** name of this trust anchor */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** number of labels in name of rrset */
	int namelabs;
	/** the ancestor in the trustanchor tree */
	struct trust_anchor* parent;
	/** 
	 * List of DS or DNSKEY rrs that form the trust anchor.
	 * It is allocated in the region.
	 */
	struct ta_key* keylist;
	/** number of DSs in the keylist */
	size_t numDS;
	/** number of DNSKEYs in the keylist */
	size_t numDNSKEY;
	/** the DS RRset */
	struct ub_packed_rrset_key* ds_rrset;
	/** The DNSKEY RRset */
	struct ub_packed_rrset_key* dnskey_rrset;
	/** class of the trust anchor */
	uint16_t dclass;
};

/**
 * Create trust anchor storage
 * @return new storage or NULL on error.
 */
struct val_anchors* anchors_create();

/**
 * Delete trust anchor storage.
 * @param anchors: to delete.
 */
void anchors_delete(struct val_anchors* anchors);

/**
 * Process trust anchor config.
 * @param anchors: struct anchor storage
 * @param cfg: config options.
 * @return 0 on error.
 */
int anchors_apply_cfg(struct val_anchors* anchors, struct config_file* cfg);

/**
 * Given a qname/qclass combination, find the trust anchor closest above it.
 * Or return NULL if none exists.
 *
 * @param anchors: struct anchor storage
 * @param qname: query name, uncompressed wireformat.
 * @param qname_len: length of qname.
 * @param qclass: class to query for.
 * @return the trust anchor or NULL if none is found.
 */
struct trust_anchor* anchors_lookup(struct val_anchors* anchors,
	uint8_t* qname, size_t qname_len, uint16_t qclass);

/**
 * Store one string as trust anchor RR.
 * @param anchors: anchor storage.
 * @param buffer: parsing buffer, to generate the RR wireformat in.
 * @param str: string.
 * @return false on error.
 */
int anchor_store_str(struct val_anchors* anchors, ldns_buffer* buffer,
	const char* str);

#endif /* VALIDATOR_VAL_ANCHOR_H */
