/*
 * validator/val_nsec3.c - validator NSEC3 denial of existance functions.
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
 * The functions help with NSEC3 checking, the different NSEC3 proofs
 * for denial of existance, and proofs for presence of types.
 */
#include "config.h"
#include "validator/val_nsec3.h"
#include "util/region-allocator.h"
#include "util/rbtree.h"

/**
 * The NSEC3 hash result storage.
 * Consists of an rbtree, with these nodes in it.
 * The nodes detail how a set of parameters (from nsec3 rr) plus
 * a dname result in a hash.
 */
struct nsec3_cached_hash {
	/** rbtree node, key is this structure */
	rbnode_t node;
	/** where are the parameters for conversion, in this rrset data */
	struct packed_rrset_data* data;
	/** where are the parameters for conversion, this RR number in data */
	size_t rr;
	/** the name to convert */
	uint8_t* dname;
	/** length of the dname */
	size_t dname_len;
	/** the hash result (not base32 encoded) */
	uint8_t* hash;
	/** length of hash in bytes */
	size_t hashlen;
	/** the hash result in base32 encoding */
	uint8_t* b32;
	/** length of base32 encoding (as a label) */
	size_t b32_len;
};

/**
 * Closest encloser (ce) proof results
 * Contains the ce and the next-closer (nc) proof.
 */
struct ce_response {
	/** the closest encloser name */
	uint8_t* ce;
	/** length of ce */
	size_t ce_len;
	/** NSEC3 record that proved ce. rrset */
	struct ub_packed_rrset_key* ce_rrset;
	/** NSEC3 record that proved ce. rr number */
	size_t ce_rr;
	/** NSEC3 record that proved nc. rrset */
	struct ub_packed_rrset_key* nc_rrset;
	/** NSEC3 record that proved nc. rr*/
	size_t nc_rr;
};

/**
 * Filter conditions for NSEC3 proof
 */
struct nsec3_filter {
	/** Zone name, only NSEC3 records for this zone are considered */
	uint8_t* zone;
	/** length of the zonename */
	size_t zone_len;
	/** the list of NSEC3s to filter; array */
	struct ub_packed_rrset_key** list;
	/** number of rrsets in list */
	size_t num;
};

/** 
 * Iterate through NSEC3 list, per RR 
 * Start with rrset = list, rrnum = 0.
 * End when rrset becomes NULL.
 * This routine gives the next RR in the list (or sets rrset null).
 * 
 * Also filters out 
 * 	o unknown flag NSEC3s
 * 	o unknown algorithm NSEC3s.
 * @param filter: nsec3 filter structure.
 * @param rrset: in/out rrset to look at.
 * @param rrnum: in/out rr number in rrset to look at.
 */
static void
filter_next(struct nsec3_filter* filter, struct ub_packed_rrset_key** rrset,
	size_t rrnum)
{
}

/** 
 * Initialize the filter structure.
 * Finds the zone by looking at available NSEC3 records and best match.
 * 	(skips the unknown flag and unknown algo NSEC3s).
 *
 * @param filter: nsec3 filter structure.
 * @param list: list of rrsets.
 * @param num: number of rrsets in list.
 * @param qtype: query type (if DS a higher zone must be chosen)
 */
static void
filter_init(struct nsec3_filter* filter, struct ub_packed_rrset_key* list,
	size_t num, uint16_t qtype)
{
}

/** 
 * Determine if any of the NSEC3 rrs iteration count is too high, from key.
 * @param ve: validator environment with iteration count config settings.
 * @param filter: what NSEC3s to loop over.
 * @param kkey: key entry used for verification; used for iteration counts.
 * @return 0 if some nsec3s are above the max iteration count.
 */

/** perform hash of name */

/** nsec3_cache_compare for rbtree */

/**
 * Obtain the hash of an owner name.
 * @param table: the cache table.
 * @param region: scratch region to use for allocation.
 * @param d: the rrset data
 * @param rr: rr number from d that has the NSEC3 parameters to hash to.
 * @param dname: name to hash
 * @param dname_len: the length of the name.
 * @param hash: the hash node is returned on success.
 * @return:
 * 	1 on success, either from cache or newly hashed hash is returned.
 * 	0 on a malloc failure.
 * 	-1 if the NSEC3 rr was badly formatted (i.e. formerr).
 */
static int
nsec3_hash_name(rbtree_t* table, struct region* region, 
	struct packed_rrset_data* data, size_t rr, uint8_t* dname, 
	size_t dname_len, struct nsec3_cached_hash** hash)
{
}

/**
 * Find matching NSEC3
 * Find the NSEC3Record that matches a hash of a name.
 */

/**
 * nsec3Covers
 * Given a hash and a candidate NSEC3Record, determine if that NSEC3Record
 * covers the hash. Covers specifically means that the hash is in between
 * the owner and next hashes and does not equal either.
 */

/**
 * findCoveringNSEC3
 * Given a pre-hashed name, find a covering NSEC3 from among a list of
 * NSEC3s.
 */

/**
 * findClosestEncloser
 * Given a name and a list of NSEC3s, find the candidate closest encloser.
 * This will be the first ancestor of 'name' (including itself) to have a
 * matching NSEC3 RR.
 */

/**
 * proveClosestEncloser
 * Given a List of nsec3 RRs, find and prove the closest encloser to qname.
 */


