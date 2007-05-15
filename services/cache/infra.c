/*
 * services/cache/infra.c - infrastructure cache, server rtt and capabilities
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
#include "config.h"
#include "services/cache/infra.h"
#include "util/storage/slabhash.h"
#include "util/storage/lookup3.h"
#include "util/log.h"
#include "util/net_help.h"

/** calculate size for the hashtable, does not count size of lameness,
 * so the hashtable is a fixed number of items */
static size_t 
infra_host_sizefunc(void* ATTR_UNUSED(k), void* ATTR_UNUSED(d))
{
	return sizeof(struct infra_host_key) + sizeof(struct infra_host_data);
}

/** compare two addresses, returns -1, 0, or +1 */
static int 
infra_host_compfunc(void* key1, void* key2)
{
	struct infra_host_key* k1 = (struct infra_host_key*)key1;
	struct infra_host_key* k2 = (struct infra_host_key*)key2;
	if(k1->addrlen != k2->addrlen) {
		if(k1->addrlen < k2->addrlen)
			return -1;
		return 1;
	}
	return memcmp(&k1->addr, &k2->addr, k1->addrlen);
}

/** delete key, and destroy the lock */
static void 
infra_host_delkeyfunc(void* k, void* ATTR_UNUSED(arg))
{
	struct infra_host_key* key = (struct infra_host_key*)k;
	if(!key)
		return;
	lock_rw_destroy(&key->entry.lock);
	free(key);
}

/** delete data and destroy the lameness hashtable */
static void 
infra_host_deldatafunc(void* d, void* ATTR_UNUSED(arg))
{
	struct infra_host_data* data = (struct infra_host_data*)d;
	lruhash_delete(data->lameness);
	free(data);
}

struct slabhash* 
infra_create(struct config_file* cfg)
{
	/* TODO: use config settings */
	/* the size of the lameness tables are not counted */
	size_t maxmem = HOST_DEFAULT_SIZE * (sizeof(struct infra_host_key) + 
		sizeof(struct infra_host_data));
	struct slabhash* infra = slabhash_create(HASH_DEFAULT_SLABS,
		INFRA_HOST_STARTSIZE, maxmem, &infra_host_sizefunc,
		&infra_host_compfunc, &infra_host_delkeyfunc,
		&infra_host_deldatafunc, NULL);
	return infra;
}

void 
infra_delete(struct slabhash* infra)
{
	if(!infra)
		return;
	slabhash_delete(infra);
}

/** calculate the hash value for a host key */
static hashvalue_t
hash_addr(struct sockaddr_storage* addr, socklen_t addrlen)
{
	hashvalue_t h = 0xab;
	h = hashlittle(&addrlen, sizeof(addrlen), h);
	h = hashlittle(addr, addrlen, h);
	return h;
}

/** lookup version that does not check host ttl (you check it) */
static struct lruhash_entry* 
infra_lookup_host_nottl(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen, int wr)
{
	struct infra_host_key k;
	k.addrlen = addrlen;
	memcpy(&k.addr, addr, addrlen);
	k.entry.hash = hash_addr(addr, addrlen);
	k.entry.key = (void*)&k;
	k.entry.data = NULL;
	return slabhash_lookup(infra, k.entry.hash, &k, wr);
}

struct infra_host_data* 
infra_lookup_host(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen, int wr,
        time_t timenow, struct infra_host_key** key)
{
	struct infra_host_data* data;
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, wr);
	*key = NULL;
	if(!e)
		return NULL;
	/* check TTL */
	data = (struct infra_host_data*)e->data;
	if(data->ttl < timenow) {
		lock_rw_unlock(&e->lock);
		return NULL;
	}
	*key = (struct infra_host_key*)e->key;
	return data;
}

/** 
 * Create and init a new entry for a host 
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param tm: time now.
 * @return: the new entry or NULL on malloc failure.
 */
static struct lruhash_entry*
new_host_entry(struct sockaddr_storage* addr, socklen_t addrlen, time_t tm)
{
	struct infra_host_data* data;
	struct infra_host_key* key = (struct infra_host_key*)malloc(
		sizeof(struct infra_host_key));
	if(!key)
		return NULL;
	data = (struct infra_host_data*)malloc(
		sizeof(struct infra_host_data));
	if(!data) {
		free(key);
		return NULL;
	}
	lock_rw_init(&key->entry.lock);
	key->entry.hash = hash_addr(addr, addrlen);
	key->entry.key = (void*)key;
	key->entry.data = (void*)data;
	key->addrlen = addrlen;
	memcpy(&key->addr, addr, addrlen);
	data->ttl = tm + HOST_TTL;
	data->lameness = NULL;
	data->edns_version = 0;
	rtt_init(&data->rtt);
	return &key->entry;
}

int 
infra_host(struct slabhash* infra, struct sockaddr_storage* addr,
        socklen_t addrlen, time_t timenow, int* edns_vs, int* to)
{
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 0);
	struct infra_host_data* data;
	if(e && ((struct infra_host_data*)e->data)->ttl < timenow) {
		/* it expired, try to reuse existing entry */
		lock_rw_unlock(&e->lock);
		e = infra_lookup_host_nottl(infra, addr, addrlen, 1);
		if(e) {
			/* if its still there we have a writelock, init */
			/* re-initialise */
			data = (struct infra_host_data*)e->data;
			data->ttl = timenow + HOST_TTL;
			rtt_init(&data->rtt);
			/* do not touch lameness, it may be valid still */
			data->edns_version = 0;
		}
	}
	if(!e) {
		/* insert new entry */
		if(!(e = new_host_entry(addr, addrlen, timenow)))
			return 0;
		data = (struct infra_host_data*)e->data;
		*to = rtt_timeout(&data->rtt);
		*edns_vs = data->edns_version;
		slabhash_insert(infra, e->hash, e, data, NULL);
		return 1;
	}
	/* use existing entry */
	data = (struct infra_host_data*)e->data;
	*to = rtt_timeout(&data->rtt);
	*edns_vs = data->edns_version;
	lock_rw_unlock(&e->lock);
	return 1;
}

/** hash lameness key */
static hashvalue_t
hash_lameness(uint8_t* name, size_t namelen)
{
	return hashlittle(name, namelen, 0xab);
}

int 
infra_lookup_lame(struct infra_host_data* host,
        uint8_t* name, size_t namelen, time_t timenow)
{
	struct lruhash_entry* e;
	struct infra_lame_key k;
	struct infra_lame_data *d;
	if(!host->lameness)
		return 0;
	k.entry.hash = hash_lameness(name, namelen);
	k.zonename = name;
	k.namelen = namelen;
	k.entry.key = (void*)&k;
	k.entry.data = NULL;
	e = lruhash_lookup(host->lameness, k.entry.hash, &k, 0);
	if(!e)
		return 0;
	d = (struct infra_lame_data*)e->data;
	if(d->ttl < timenow) {
		lock_rw_unlock(&e->lock);
		return 0;
	}
	lock_rw_unlock(&e->lock);
	return 1;
}

/** calculate size, which is fixed, zonename does not count so that
 * a fixed number of items is stored */
static size_t 
infra_lame_sizefunc(void* ATTR_UNUSED(k), void* ATTR_UNUSED(d))
{
	return sizeof(struct infra_lame_key)+sizeof(struct infra_lame_data);
}

/** compare zone names, returns -1, 0, +1 */
static int 
infra_lame_compfunc(void* key1, void* key2) 
{
	struct infra_lame_key* k1 = (struct infra_lame_key*)key1;
	struct infra_lame_key* k2 = (struct infra_lame_key*)key2;
	if(k1->namelen != k2->namelen) {
		if(k1->namelen < k2->namelen)
			return -1;
		return 1;
	}
	return memcmp(k1->zonename, k2->zonename, k1->namelen);
}

/** free key, lock and zonename */
static void 
infra_lame_delkeyfunc(void* k, void* ATTR_UNUSED(arg))
{
	struct infra_lame_key* key = (struct infra_lame_key*)k;
	if(!key) 
		return;
	lock_rw_destroy(&key->entry.lock);
	free(key->zonename);
	free(key);
}

/** free the lameness data */
static void 
infra_lame_deldatafunc(void* d, void* ATTR_UNUSED(arg))
{
	if(!d) 
		return;
	free(d);
}

int 
infra_set_lame(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        uint8_t* name, size_t namelen, time_t timenow)
{
	struct infra_host_data* data;
	struct lruhash_entry* e;
	int needtoinsert = 0;
	struct infra_lame_key* k;
	struct infra_lame_data* d;
	/* allocate at start, easier cleanup (no locks held) */
	k = (struct infra_lame_key*)malloc(sizeof(*k));
	if(!k) {
		log_err("set_lame: malloc failure");
		return 0;
	}
	d = (struct infra_lame_data*)malloc(sizeof(*d));
	if(!d) {
		free(k);
		log_err("set_lame: malloc failure");
		return 0;
	}
	k->zonename = memdup(name, namelen);
	if(!k->zonename) {
		free(d);
		free(k);
		log_err("set_lame: malloc failure");
		return 0;
	}
	lock_rw_init(&k->entry.lock);
	k->entry.hash = hash_lameness(name, namelen);
	k->entry.key = (void*)k;
	k->entry.data = (void*)d;
	d->ttl = timenow + HOST_LAME_TTL;
	k->namelen = namelen;
	e = infra_lookup_host_nottl(infra, addr, addrlen, 1);
	if(!e) {
		/* insert it */
		if(!(e = new_host_entry(addr, addrlen, timenow))) {
			free(k->zonename);
			free(k);
			free(d);
			log_err("set_lame: malloc failure");
			return 0;
		}
		needtoinsert = 1;
	}
	/* got an entry, now set the zone lame */
	data = (struct infra_host_data*)e->data;
	if(!data->lameness) {
		/* create hash table if not there already */
		data->lameness = lruhash_create(INFRA_LAME_STARTSIZE,
			INFRA_LAME_MAXMEM*(sizeof(struct infra_lame_key)+
			sizeof(struct infra_lame_data)), infra_lame_sizefunc, 
			infra_lame_compfunc, infra_lame_delkeyfunc,
			infra_lame_deldatafunc, NULL);
		if(!data->lameness) {
			log_err("set_lame: malloc failure");
			if(needtoinsert) slabhash_insert(infra, e->hash, e, 
				e->data, NULL);
			else 	lock_rw_unlock(&e->lock);
			free(k->zonename);
			free(k);
			free(d);
			return 0;
		}
	}
	/* inserts new entry, or updates TTL of older entry */
	lruhash_insert(data->lameness, k->entry.hash, &k->entry, d, NULL);
	
	if(needtoinsert)
		slabhash_insert(infra, e->hash, e, e->data, NULL);
	else 	lock_rw_unlock(&e->lock);
	return 1;
}

int 
infra_rtt_update(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        int roundtrip, time_t timenow)
{
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 1);
	struct infra_host_data* data;
	int needtoinsert = 0;
	if(!e) {
		if(!(e = new_host_entry(addr, addrlen, timenow)))
			return 0;
		needtoinsert = 1;
	}
	/* have an entry, update the rtt, and the ttl */
	data = (struct infra_host_data*)e->data;
	data->ttl = timenow + HOST_TTL;
	if(roundtrip == -1)
		rtt_lost(&data->rtt);
	else	rtt_update(&data->rtt, roundtrip);

	if(needtoinsert)
		slabhash_insert(infra, e->hash, e, e->data, NULL);
	else 	lock_rw_unlock(&e->lock);
	return 1;
}

int 
infra_edns_update(struct slabhash* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        int edns_version, time_t timenow)
{
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 1);
	struct infra_host_data* data;
	int needtoinsert = 0;
	if(!e) {
		if(!(e = new_host_entry(addr, addrlen, timenow)))
			return 0;
		needtoinsert = 1;
	}
	/* have an entry, update the rtt, and the ttl */
	data = (struct infra_host_data*)e->data;
	data->ttl = timenow + HOST_TTL;
	data->edns_version = edns_version;

	if(needtoinsert)
		slabhash_insert(infra, e->hash, e, e->data, NULL);
	else 	lock_rw_unlock(&e->lock);
	return 1;
}
