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
struct config_file;

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
 * Authoritative local zones storage, shared.
 * This tree is fixed at startup, so, readonly, no locks or mutexes necessary.
 */
struct local_zones {
	/** rbtree of struct local_zone */
	rbtree_t ztree;
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
	size_t namelen;
	/** number of labels in zone name */
	int namelabs;
	/** the class of this zone. 
	 * uses 'dclass' to not conflict with c++ keyword class. */
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
	struct ub_packed_rrset_key* soa;
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
	/** the data rrsets, with different types, linked list */
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

/**
 * Create local zones storage
 * @return new struct or NULL on error.
 */
struct local_zones* local_zones_create();

/**
 * Delete local zones storage
 * @param zones: to delete.
 */
void local_zones_delete(struct local_zones* zones);

/**
 * Apply config settings; setup the local authoritative data. 
 * @param zones: is set up.
 * @param cfg: config data.
 * @return false on error.
 */
int local_zones_apply_cfg(struct local_zones* zones, struct config_file* cfg);

/**
 * Compare two local_zone entries in rbtree. Sort hierarchical but not
 * canonical
 * @param z1: zone 1
 * @param z2: zone 2
 * @return: -1, 0, +1 comparison value.
 */
int local_zone_cmp(const void* z1, const void* z2);

/**
 * Compare two local_data entries in rbtree. Sort canonical.
 * @param d1: data 1
 * @param d2: data 2
 * @return: -1, 0, +1 comparison value.
 */
int local_data_cmp(const void* d1, const void* d2);

/**
 * Delete one zone
 * @param z: to delete.
 */
void local_zone_delete(struct local_zone* z);

/**
 * Lookup zone that contains the given name, class.
 * @param zones: the zones tree
 * @param name: dname to lookup
 * @param len: length of name.
 * @param labs: labelcount of name.
 * @param dclass: class to lookup.
 * @return closest local_zone or NULL if no covering zone is found.
 */
struct local_zone* local_zones_lookup(struct local_zones* zones, 
	uint8_t* name, size_t len, int labs, uint16_t dclass);

/**
 * Debug helper. Print all zones 
 * @param zones: the zones tree
 */
void local_zones_print(struct local_zones* zones);

#endif /* SERVICES_LOCALZONE_H */
