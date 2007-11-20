/*
 * services/localzone.h - local zones authority service.
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
 * This file contains functions to enable local zone authority service.
 */

#ifndef SERVICES_LOCALZONE_H
#define SERVICES_LOCALZONE_H
#include "util/rbtree.h"
struct ub_packed_rrset_key;
struct regional;

/**
 * Local zone type
 * This type determines processing for queries that did not match
 * local-data directly.
 */
enum localzone_type {
	/** drop query */
	local_zone_deny = 0,
	/** answer with error */
	local_zone_refuse,
	/** answer nxdomain or nodata */
	local_zone_static,
	/** resolve normally */
	local_zone_transparent,
	/** answer with data at zone apex */
	local_zone_redirect,
	/** remove default AS112 blocking contents for zone
	 * nodefault is used in config not during service. */
	local_zone_nodefault
};

/**
 * Local zones storage, shared.
 * Fixed at startup, so, readonly, no locks or mutexes necessary.
 */
struct local_zones {
	/** rbtree of struct local_zone */
	rbtree_t zones;
};

/**
 * Local zone. A locally served authoritative zone.
 */
struct local_zone {
	/** rbtree node, key is name and class */
	rbnode_t node;
	/** parent zone, if any. */
	struct local_zone* parent;

	/** zone name, in uncompressed wireformat */
	uint8_t* name;
	/** length of zone name */
	size_t name_len;
	/** number of labels in zone name */
	int name_labs;
	/** the class of this zone */
	uint16_t dclass;

	/** how to process zone */
	enum localzone_type type;

	/** in this region the zone's data is allocated.
	 * the struct local_zone itself is malloced. */
	struct regional* region;
	/** local data for this zone
	 * rbtree of struct local_data */
	rbtree_t data;
	/** if data contains zone apex SOA data, this is a ptr to it. */
	struct ub_packed_rrset_key* apex;
};

/**
 * Local data. One domain name, and the RRs to go with it.
 */
struct local_data {
	/** rbtree node, key is name only */
	rbnode_t node;
	/** domain name */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** number of labels in name */
	int namelabs;
	/** the data rrsets, match type and class, linked list */
	struct local_rrset* rrsets;
};

/**
 * A local data RRset
 */
struct local_rrset {
	/** next in list */
	struct local_rrset* next;
	/** RRset data item */
	struct ub_packed_rrset_key* rrset;
};

#endif /* SERVICES_LOCALZONE_H */
