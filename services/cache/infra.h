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

/**
 * Infra cache 
 */
struct infra_cache {
	/** The hash table with hosts */
	struct slabhash* hosts;
	/** TTL value for host information, in seconds */
	int host_ttl;
	/** TTL for Lameness information, in seconds */
	int lame_ttl;
	/** infra lame cache max memory per host, for this many entries */
	size_t max_lame;
};

/** infra host cache default hash lookup size */
#define INFRA_HOST_STARTSIZE 32
/** infra lame cache default hash lookup size */
#define INFRA_LAME_STARTSIZE 2

/**
 * Create infra cache.
 * @param cfg: config parameters or NULL for defaults.
 * @return: new infra cache, or NULL.
 */
struct infra_cache* infra_create(struct config_file* cfg);

/**
 * Delete infra cache.
 * @param infra: infrastructure cache to delete.
 */
void infra_delete(struct infra_cache* infra);

/**
 * Adjust infra cache to use updated configuration settings.
 * This may clean the cache. Operates a bit like realloc.
 * There may be no threading or use by other threads.
 * @param infra: existing cache. If NULL a new infra cache is returned.
 * @param cfg: config options.
 * @return the new infra cache pointer or NULL on error.
 */
struct infra_cache* infra_adjust(struct infra_cache* infra, 
	struct config_file* cfg);

/**
 * Lookup host data
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param wr: set to true to get a writelock on the entry.
 * @param timenow: what time it is now.
 * @param key: the key for the host, returned so caller can unlock when done.
 * @return: host data or NULL if not found or expired.
 */
struct infra_host_data* infra_lookup_host(struct infra_cache* infra, 
	struct sockaddr_storage* addr, socklen_t addrlen, int wr, 
	time_t timenow, struct infra_host_key** key);

/**
 * Find host information to send a packet. Creates new entry if not found.
 * Lameness is empty. EDNS is 0 (try with first), and rtt is returned for 
 * the first message to it.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param timenow: what time it is now.
 * @param edns_vs: edns version it supports, is returned.
 * @param to: timeout to use, is returned.
 * @return: 0 on error.
 */
int infra_host(struct infra_cache* infra, struct sockaddr_storage* addr, 
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
int infra_lookup_lame(struct infra_host_data* host,
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
int infra_set_lame(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
	uint8_t* name, size_t namelen, time_t timenow);

/**
 * Update rtt information for the host.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param roundtrip: estimate of roundtrip time in milliseconds or -1 for 
 * 	timeout.
 * @param timenow: what time it is now.
 * @return: 0 on error.
 */
int infra_rtt_update(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
	int roundtrip, time_t timenow);

/**
 * Update edns information for the host.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param edns_version: the version that it publishes.
 * @param timenow: what time it is now.
 * @return: 0 on error.
 */
int infra_edns_update(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
	int edns_version, time_t timenow);

/**
 * Get Lameness information and average RTT if host is in the cache.
 * @param infra: infrastructure cache.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param name: zone name.
 * @param namelen: zone name length.
 * @param lame: if function returns true, this returns lameness of the zone.
 * @param rtt: if function returns true, this returns avg rtt of the server.
 * 	The rtt value is unclamped and reflects recent timeouts.
 * @param timenow: what time it is now.
 * @return if found in cache, or false if not (or TTL bad).
 */
int infra_get_lame_rtt(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen, 
	uint8_t* name, size_t namelen, int* lame, int* rtt, time_t timenow);

/**
 * Get memory used by the infra cache.
 * @param infra: infrastructure cache.
 * @return memory in use in bytes.
 */
size_t infra_get_mem(struct infra_cache* infra);

#endif /* SERVICES_CACHE_INFRA_H */
