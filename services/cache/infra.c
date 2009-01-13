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
#include "util/data/dname.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/config_file.h"

size_t 
infra_host_sizefunc(void* k, void* ATTR_UNUSED(d))
{
	struct infra_host_key* key = (struct infra_host_key*)k;
	return sizeof(*key) + sizeof(struct infra_host_data) 
		+ lock_get_mem(&key->entry.lock);
}

int 
infra_host_compfunc(void* key1, void* key2)
{
	struct infra_host_key* k1 = (struct infra_host_key*)key1;
	struct infra_host_key* k2 = (struct infra_host_key*)key2;
	return sockaddr_cmp(&k1->addr, k1->addrlen, &k2->addr, k2->addrlen);
}

void 
infra_host_delkeyfunc(void* k, void* ATTR_UNUSED(arg))
{
	struct infra_host_key* key = (struct infra_host_key*)k;
	if(!key)
		return;
	lock_rw_destroy(&key->entry.lock);
	free(key);
}

void 
infra_host_deldatafunc(void* d, void* ATTR_UNUSED(arg))
{
	struct infra_host_data* data = (struct infra_host_data*)d;
	lruhash_delete(data->lameness);
	free(data);
}

struct infra_cache* 
infra_create(struct config_file* cfg)
{
	struct infra_cache* infra = (struct infra_cache*)calloc(1, 
		sizeof(struct infra_cache));
	/* the size of the lameness tables are not counted */
	size_t maxmem = cfg->infra_cache_numhosts * 
		(sizeof(struct infra_host_key)+sizeof(struct infra_host_data));
	infra->hosts = slabhash_create(cfg->infra_cache_slabs,
		INFRA_HOST_STARTSIZE, maxmem, &infra_host_sizefunc,
		&infra_host_compfunc, &infra_host_delkeyfunc,
		&infra_host_deldatafunc, NULL);
	if(!infra->hosts) {
		free(infra);
		return NULL;
	}
	infra->host_ttl = cfg->host_ttl;
	infra->lame_ttl = cfg->lame_ttl;
	infra->max_lame_size = cfg->infra_cache_lame_size;
	return infra;
}

void 
infra_delete(struct infra_cache* infra)
{
	if(!infra)
		return;
	slabhash_delete(infra->hosts);
	free(infra);
}

struct infra_cache* 
infra_adjust(struct infra_cache* infra, struct config_file* cfg)
{
	size_t maxmem;
	if(!infra)
		return infra_create(cfg);
	infra->host_ttl = cfg->host_ttl;
	infra->lame_ttl = cfg->lame_ttl;
	infra->max_lame_size = cfg->infra_cache_lame_size;
	maxmem = cfg->infra_cache_numhosts * 
		(sizeof(struct infra_host_key)+sizeof(struct infra_host_data));
	if(maxmem != slabhash_get_size(infra->hosts) ||
		cfg->infra_cache_slabs != infra->hosts->size) {
		infra_delete(infra);
		infra = infra_create(cfg);
	}
	return infra;
}

/** calculate the hash value for a host key */
static hashvalue_t
hash_addr(struct sockaddr_storage* addr, socklen_t addrlen)
{
	hashvalue_t h = 0xab;
	/* select the pieces to hash, some OS have changing data inside */
	if(addr_is_ip6(addr, addrlen)) {
		struct sockaddr_in6* in6 = (struct sockaddr_in6*)addr;
		h = hashlittle(&in6->sin6_family, sizeof(in6->sin6_family), h);
		h = hashlittle(&in6->sin6_port, sizeof(in6->sin6_port), h);
		h = hashlittle(&in6->sin6_addr, INET6_SIZE, h);
	} else {
		struct sockaddr_in* in = (struct sockaddr_in*)addr;
		h = hashlittle(&in->sin_family, sizeof(in->sin_family), h);
		h = hashlittle(&in->sin_port, sizeof(in->sin_port), h);
		h = hashlittle(&in->sin_addr, INET_SIZE, h);
	}
	return h;
}

/** lookup version that does not check host ttl (you check it) */
static struct lruhash_entry* 
infra_lookup_host_nottl(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen, int wr)
{
	struct infra_host_key k;
	k.addrlen = addrlen;
	memcpy(&k.addr, addr, addrlen);
	k.entry.hash = hash_addr(addr, addrlen);
	k.entry.key = (void*)&k;
	k.entry.data = NULL;
	return slabhash_lookup(infra->hosts, k.entry.hash, &k, wr);
}

struct infra_host_data* 
infra_lookup_host(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen, int wr,
        uint32_t timenow, struct infra_host_key** key)
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
 * @param infra: infra structure with config parameters.
 * @param addr: host address.
 * @param addrlen: length of addr.
 * @param tm: time now.
 * @return: the new entry or NULL on malloc failure.
 */
static struct lruhash_entry*
new_host_entry(struct infra_cache* infra, struct sockaddr_storage* addr, 
	socklen_t addrlen, uint32_t tm)
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
	data->ttl = tm + infra->host_ttl;
	data->lameness = NULL;
	data->edns_version = 0;
	data->edns_lame_known = 0;
	rtt_init(&data->rtt);
	return &key->entry;
}

int 
infra_host(struct infra_cache* infra, struct sockaddr_storage* addr,
        socklen_t addrlen, uint32_t timenow, int* edns_vs, 
	uint8_t* edns_lame_known, int* to)
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
			data->ttl = timenow + infra->host_ttl;
			rtt_init(&data->rtt);
			/* do not touch lameness, it may be valid still */
			data->edns_version = 0;
			data->edns_lame_known = 0;
		}
	}
	if(!e) {
		/* insert new entry */
		if(!(e = new_host_entry(infra, addr, addrlen, timenow)))
			return 0;
		data = (struct infra_host_data*)e->data;
		*to = rtt_timeout(&data->rtt);
		*edns_vs = data->edns_version;
		*edns_lame_known = data->edns_lame_known;
		slabhash_insert(infra->hosts, e->hash, e, data, NULL);
		return 1;
	}
	/* use existing entry */
	data = (struct infra_host_data*)e->data;
	*to = rtt_timeout(&data->rtt);
	*edns_vs = data->edns_version;
	*edns_lame_known = data->edns_lame_known;
	lock_rw_unlock(&e->lock);
	return 1;
}

/** hash lameness key */
static hashvalue_t
hash_lameness(uint8_t* name)
{
	return dname_query_hash(name, 0xab);
}

int 
infra_lookup_lame(struct infra_host_data* host,
        uint8_t* name, size_t namelen, uint32_t timenow,
	int* dlame, int* rlame, int* alame, int* olame)
{
	struct lruhash_entry* e;
	struct infra_lame_key k;
	struct infra_lame_data *d;
	if(!host->lameness)
		return 0;
	k.entry.hash = hash_lameness(name);
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
	*dlame = d->isdnsseclame;
	*rlame = d->rec_lame;
	*alame = d->lame_type_A;
	*olame = d->lame_other;
	lock_rw_unlock(&e->lock);
	return *dlame || *rlame || *alame || *olame;
}

size_t 
infra_lame_sizefunc(void* k, void* ATTR_UNUSED(d))
{
	struct infra_lame_key* key = (struct infra_lame_key*)k;
	return sizeof(*key) + sizeof(struct infra_lame_data)
		+ key->namelen + lock_get_mem(&key->entry.lock);
}

int 
infra_lame_compfunc(void* key1, void* key2) 
{
	struct infra_lame_key* k1 = (struct infra_lame_key*)key1;
	struct infra_lame_key* k2 = (struct infra_lame_key*)key2;
	if(k1->namelen != k2->namelen) {
		if(k1->namelen < k2->namelen)
			return -1;
		return 1;
	}
	return query_dname_compare(k1->zonename, k2->zonename);
}

void 
infra_lame_delkeyfunc(void* k, void* ATTR_UNUSED(arg))
{
	struct infra_lame_key* key = (struct infra_lame_key*)k;
	if(!key) 
		return;
	lock_rw_destroy(&key->entry.lock);
	free(key->zonename);
	free(key);
}

void 
infra_lame_deldatafunc(void* d, void* ATTR_UNUSED(arg))
{
	if(!d) 
		return;
	free(d);
}

int 
infra_set_lame(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        uint8_t* name, size_t namelen, uint32_t timenow, int dnsseclame,
	int reclame, uint16_t qtype)
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
	k->entry.hash = hash_lameness(name);
	k->entry.key = (void*)k;
	k->entry.data = (void*)d;
	d->ttl = timenow + infra->lame_ttl;
	d->isdnsseclame = dnsseclame;
	d->rec_lame = reclame;
	d->lame_type_A = (!dnsseclame && !reclame && qtype == LDNS_RR_TYPE_A);
	d->lame_other = (!dnsseclame  && !reclame && qtype != LDNS_RR_TYPE_A);
	k->namelen = namelen;
	e = infra_lookup_host_nottl(infra, addr, addrlen, 1);
	if(!e) {
		/* insert it */
		if(!(e = new_host_entry(infra, addr, addrlen, timenow))) {
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
			infra->max_lame_size, infra_lame_sizefunc, 
			infra_lame_compfunc, infra_lame_delkeyfunc,
			infra_lame_deldatafunc, NULL);
		if(!data->lameness) {
			log_err("set_lame: malloc failure");
			if(needtoinsert) slabhash_insert(infra->hosts, 
				e->hash, e, e->data, NULL);
			else 	{ lock_rw_unlock(&e->lock); }
			free(k->zonename);
			free(k);
			free(d);
			return 0;
		}
	} else {
		/* lookup existing lameness entry (if any) and merge data */
		int dlame, rlame, alame, olame; 
		if(infra_lookup_lame(data, name, namelen, timenow,
			&dlame, &rlame, &alame, &olame)) { 
			/* merge data into new structure */
			if(dlame) d->isdnsseclame = 1;
			if(rlame) d->rec_lame = 1;
			if(alame) d->lame_type_A = 1;
			if(olame) d->lame_other = 1;
		}
	}

	/* inserts new entry, or updates TTL of older entry */
	lruhash_insert(data->lameness, k->entry.hash, &k->entry, d, NULL);
	
	if(needtoinsert)
		slabhash_insert(infra->hosts, e->hash, e, e->data, NULL);
	else 	{ lock_rw_unlock(&e->lock); }
	return 1;
}

void 
infra_update_tcp_works(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen)
{
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 1);
	struct infra_host_data* data;
	if(!e)
		return; /* doesn't exist */
	data = (struct infra_host_data*)e->data;
	if(data->rtt.rto >= RTT_MAX_TIMEOUT)
		/* do not disqualify this server altogether, it is better
		 * than nothing */
		data->rtt.rto = RTT_MAX_TIMEOUT-1;
	lock_rw_unlock(&e->lock);
}

int 
infra_rtt_update(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        int roundtrip, int orig_rtt, uint32_t timenow)
{
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 1);
	struct infra_host_data* data;
	int needtoinsert = 0;
	int rto = 1;
	if(!e) {
		if(!(e = new_host_entry(infra, addr, addrlen, timenow)))
			return 0;
		needtoinsert = 1;
	}
	/* have an entry, update the rtt, and the ttl */
	data = (struct infra_host_data*)e->data;
	data->ttl = timenow + infra->host_ttl;
	if(roundtrip == -1)
		rtt_lost(&data->rtt, orig_rtt);
	else	rtt_update(&data->rtt, roundtrip);
	if(data->rtt.rto > 0)
		rto = data->rtt.rto;

	if(needtoinsert)
		slabhash_insert(infra->hosts, e->hash, e, e->data, NULL);
	else 	{ lock_rw_unlock(&e->lock); }
	return rto;
}

int 
infra_edns_update(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        int edns_version, uint32_t timenow)
{
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 1);
	struct infra_host_data* data;
	int needtoinsert = 0;
	if(!e) {
		if(!(e = new_host_entry(infra, addr, addrlen, timenow)))
			return 0;
		needtoinsert = 1;
	}
	/* have an entry, update the rtt, and the ttl */
	data = (struct infra_host_data*)e->data;
	data->ttl = timenow + infra->host_ttl;
	data->edns_version = edns_version;
	data->edns_lame_known = 1;

	if(needtoinsert)
		slabhash_insert(infra->hosts, e->hash, e, e->data, NULL);
	else 	{ lock_rw_unlock(&e->lock); }
	return 1;
}

int 
infra_get_lame_rtt(struct infra_cache* infra,
        struct sockaddr_storage* addr, socklen_t addrlen,
        uint8_t* name, size_t namelen, uint16_t qtype, 
	int* lame, int* dnsseclame, int* reclame, int* rtt, uint32_t timenow)
{
	struct infra_host_data* host;
	struct lruhash_entry* e = infra_lookup_host_nottl(infra, addr, 
		addrlen, 0);
	int dlm, rlm, alm, olm;
	if(!e) 
		return 0;
	host = (struct infra_host_data*)e->data;
	*rtt = rtt_unclamped(&host->rtt);
	/* check lameness first, if so, ttl on host does not matter anymore */
	if(infra_lookup_lame(host, name, namelen, timenow, 
		&dlm, &rlm, &alm, &olm)) {
		if(alm && qtype == LDNS_RR_TYPE_A) {
			lock_rw_unlock(&e->lock);
			*lame = 1;
			*dnsseclame = 0;
			*reclame = 0;
			return 1;
		} else if(olm && qtype != LDNS_RR_TYPE_A) {
			lock_rw_unlock(&e->lock);
			*lame = 1;
			*dnsseclame = 0;
			*reclame = 0;
			return 1;
		} else if(dlm) {
			lock_rw_unlock(&e->lock);
			*lame = 0;
			*dnsseclame = 1;
			*reclame = 0;
			return 1;
		} else if(rlm) {
			lock_rw_unlock(&e->lock);
			*lame = 0;
			*dnsseclame = 0;
			*reclame = 1;
			return 1;
		}
		/* no lameness for this type of query */
	}
	*lame = 0;
	*dnsseclame = 0;
	*reclame = 0;
	if(timenow > host->ttl) {
		lock_rw_unlock(&e->lock);
		return 0;
	}
	lock_rw_unlock(&e->lock);
	return 1;
}

/** helper memory count for a host lame cache */
static size_t
count_host_lame(struct lruhash_entry* e)
{
	struct infra_host_data* host_data = (struct infra_host_data*)e->data;
	if(!host_data->lameness)
		return 0;
	return lruhash_get_mem(host_data->lameness);
}

size_t 
infra_get_mem(struct infra_cache* infra)
{
	size_t i, bin;
	size_t s = sizeof(*infra) +
		slabhash_get_mem(infra->hosts);
	struct lruhash_entry* e;
	for(i=0; i<infra->hosts->size; i++) {
		lock_quick_lock(&infra->hosts->array[i]->lock);
		for(bin=0; bin<infra->hosts->array[i]->size; bin++) {
			lock_quick_lock(&infra->hosts->array[i]->
				array[bin].lock);
			/* count data size in bin items. */
			for(e = infra->hosts->array[i]->array[bin].
				overflow_list; e; e = e->overflow_next) {
				lock_rw_rdlock(&e->lock);
				s += count_host_lame(e);
				lock_rw_unlock(&e->lock);
			}
			lock_quick_unlock(&infra->hosts->array[i]->
				array[bin].lock);
		}
		lock_quick_unlock(&infra->hosts->array[i]->lock);
	}
	return s;
}
