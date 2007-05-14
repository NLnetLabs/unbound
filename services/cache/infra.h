/*
 * services/cache/infra.h - infrastructure cache, server rtt and capabilities
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
 * This file contains the infrastructure cache.
 */

#ifndef SERVICES_CACHE_INFRA_H
#define SERVICES_CACHE_INFRA_H
#include "util/storage/lruhash.h"
#include "util/rtt.h"
struct slabhash;
struct config_file;

/**
 * Host information kept for every server.
 */
struct infra_host_key {
	/** the host address. */
	struct sockaddr_storage addr;
	/** length of addr. */
	socklen_t addrlen;
	/** hash table entry, data of type infra_host_data. */
	struct lruhash_entry entry;
};

/**
 * Host information encompasses host capabilities and retransmission timeouts.
 */
struct infra_host_data {
	/** TTL value for this entry. absolute time. */
	time_t ttl;
	/** round trip times for timeout calculation */
	struct rtt_info rtt;
	/** Names of the zones that are lame. NULL=no lame zones. */
	struct lruhash* lameness;
	/** edns version that the host supports, -1 means no EDNS */
	int edns_version;
	/** edns message size that the host advertizes, 512 by default. */
	uint16_t edns_size;
};

/**
 * Lameness information, per host, per zone. 
 */
struct infra_lame_key {
	/** key is zone name in wireformat */
	uint8_t* zonename;
	/** length of zonename */
	size_t namelen;
	/** lruhash entry */
	struct lruhash_entry entry;
};

/**
 * Lameness information. Expires.
 * This host is lame because it is in the cache.
 */
struct infra_lame_data {
	/** TTL of this entry. absolute time. */
	time_t ttl;
};

/** default TTL value for host information, in seconds */
#define HOST_TTL 900
/** default TTL for Lameness information, in seconds */
#define HOST_LAME_TTL 900
/** default size of the host cache, number of entries */
#define HOST_DEFAULT_SIZE 1000

/**
 * Create infra cache.
 * @param cfg: config parameters.
 * @return: new infra cache, or NULL.
 */
struct slabhash* infra_create(struct config_file* cfg);

/**
 * Delete infra cache.
 * @param infra: infrastructure cache to delete.
 */
void infra_delete(struct slabhash* infra);

/**
 * Lookup host data
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param wr: set to true to get a writelock on the entry.
 * @param timenow: what time it is now.
 * @return: host data or NULL if not found or expired.
 */
struct infra_data* infra_lookup_host(struct slabhash* infra, 
	struct sockaddr_storage* addr, socklen_t addrlen, int wr, 
	time_t timenow);

/**
 * Find host information to send a packet. Creates new entry if not found.
 * Lameness is empty. EDNS is 0, size is 512, and rtt is returned for 
 * the first message to it.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param timenow: what time it is now.
 * @param edns_vs: edns version it supports, is returned.
 * @param to: timeout to use, is returned.
 * @return: 0 on error.
 */
int infra_host(struct slabhash* infra, struct sockaddr_storage* addr, 
	socklen_t addrlen, time_t timenow, int* edns_vs, int* to);

/**
 * Check for lameness of this server for a particular zone.
 * You must have a lock on the host structure.
 * @param host: infrastructure cache data for the host. Caller holds lock.
 * @param name: domain name of zone apex.
 * @param namelen: length of domain name.
 * @param timenow: what time it is now.
 * @return: 0 if not lame or unknown or timed out, true if lame.
 */
int infra_lookup_lame(struct infra_data* host,
	uint8_t* name, size_t namelen, time_t timenow);

/**
 * Set a host to be lame for the given zone.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param name: domain name of zone apex.
 * @param namelen: length of domain name.
 * @param timenow: what time it is now.
 * @return: 0 on error.
 */
int infra_set_lame(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
	uint8_t* name, size_t namelen, time_t timenow);

/**
 * Update rtt information for the host.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param roundtrip: estimate of roundtrip time or -1 for timeout.
 * @param timenow: what time it is now.
 * @return: 0 on error.
 */
int infra_rtt_update(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
	int roundtrip, time_t timenow);

/**
 * Update edns information for the host.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param edns_version: the version that it publishes.
 * @param udp_size: what udp size it can handle.
 * @param timenow: what time it is now.
 * @return: 0 on error.
 */
int infra_edns_update(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
	int edns_version, uint16_t udp_size, time_t timenow);

#endif /* SERVICES_CACHE_INFRA_H */
