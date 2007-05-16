/*
 * services/cache/rrset.c - Resource record set cache.
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
 * This file contains the rrset cache.
 */
#include "config.h"
#include "services/cache/rrset.h"
#include "util/storage/slabhash.h"
#include "util/config_file.h"
#include "util/data/packed_rrset.h"
#include "util/data/msgreply.h"

struct rrset_cache* rrset_cache_create(struct config_file* cfg, 
	struct alloc_cache* alloc)
{
	size_t slabs = (cfg?cfg->rrset_cache_slabs:HASH_DEFAULT_SLABS);
	size_t startarray = HASH_DEFAULT_STARTARRAY;
	size_t maxmem = (cfg?cfg->rrset_cache_size:HASH_DEFAULT_MAXMEM);

	struct rrset_cache *r = (struct rrset_cache*)slabhash_create(slabs,
		startarray, maxmem, ub_rrset_sizefunc, ub_rrset_compare,
		ub_rrset_key_delete, rrset_data_delete, alloc);
	return r;
}

void rrset_cache_delete(struct rrset_cache* r)
{
	if(!r) 
		return;
	slabhash_delete(&r->table);
}

struct rrset_cache* rrset_cache_adjust(struct rrset_cache *r, 
	struct config_file* cfg, struct alloc_cache* alloc)
{
	if(!r || !cfg || cfg->rrset_cache_slabs != r->table.size ||
		cfg->rrset_cache_size != slabhash_get_size(&r->table))
	{
		rrset_cache_delete(r);
		r = rrset_cache_create(cfg, alloc);
	}
	return r;
}

void 
rrset_cache_touch(struct rrset_cache* r, struct ub_packed_rrset_key* key,
        hashvalue_t hash, rrset_id_t id)
{
	struct lruhash* table = slabhash_gettable(&r->table, hash);
	/* 
	 * This leads to locking problems, deadlocks, if the caller is 
	 * holding any other rrset lock.
	 * Because a lookup through the hashtable does:
	 *	tablelock -> entrylock  (for that entry caller holds)
	 * And this would do
	 *	entrylock(already held) -> tablelock
	 * And if two threads do this, it results in deadlock.
	 * So, the caller must not hold entrylock.
	 */
	lock_quick_lock(&table->lock);
	/* we have locked the hash table, the item can still be deleted.
	 * because it could already have been reclaimed, but not yet set id=0.
	 * This is because some lruhash routines have lazy deletion.
	 * so, we must acquire a lock on the item to verify the id != 0.
	 * also, with hash not changed, we are using the right slab.
	 */
	lock_rw_rdlock(&key->entry.lock);
	if(key->id == id && key->entry.hash == hash) {
		lru_touch(table, &key->entry);
	}
	lock_rw_unlock(&key->entry.lock);
	lock_quick_unlock(&table->lock);
}

/** see if rrset needs to be updated in the cache */
static int
need_to_update_rrset(void* nd, void* cd, uint32_t timenow)
{
	struct packed_rrset_data* newd = (struct packed_rrset_data*)nd;
	struct packed_rrset_data* cached = (struct packed_rrset_data*)cd;
        /*      o if current RRset is more trustworthy - insert it */
        if( newd->trust > cached->trust )
                return 1;
	/*	o item in cache has expired */
	if( cached->ttl < timenow )
		return 1;
        /*      o same trust, but different in data - insert it */
        if( newd->trust == cached->trust &&
                !rrsetdata_equal(newd, cached))
                return 1;
        /*      o see if TTL is better than TTL in cache. */
        /*        if so, see if rrset+rdata is the same */
        /*        if so, update TTL in cache, even if trust is worse. */
        if( newd->ttl > cached->ttl &&
                rrsetdata_equal(newd, cached)) {
		/* since all else is the same, use the best trust value */
		if(newd->trust < cached->trust)
			newd->trust = cached->trust;
                return 1;
	}
        return 0;
}

int 
rrset_cache_update(struct rrset_cache* r, struct rrset_ref* ref,
	struct alloc_cache* alloc, uint32_t timenow)
{
	struct lruhash_entry* e;
	struct ub_packed_rrset_key* k = ref->key;
	hashvalue_t h = k->entry.hash;
	/* looks up item with a readlock - no editing! */
	if((e=slabhash_lookup(&r->table, h, k, 0)) != 0) {
		/* return id and key as they will be used in the cache
		 * since the lruhash_insert, if item already exists, deallocs
		 * the passed key in favor of the already stored key.
		 * because of the small gap (see below) this key ptr and id
		 * may prove later to be already deleted, which is no problem
		 * as it only makes a cache miss. 
		 */
		ref->key = (struct ub_packed_rrset_key*)e->key;
		ref->id = ref->key->id;
		if(!need_to_update_rrset(k->entry.data, e->data, timenow)) {
			/* cache is superior, return that value */
			lock_rw_unlock(&e->lock);
			ub_packed_rrset_parsedelete(k, alloc);
			return 1;
		}
		lock_rw_unlock(&e->lock);
		/* Go on and insert the passed item.
		 * small gap here, where entry is not locked.
		 * possibly entry is updated with something else.
		 * we then overwrite that with our data.
		 * this is just too bad, its cache anyway. */
		/* use insert to update entry to manage lruhash
		 * cache size values nicely. */
	}
	slabhash_insert(&r->table, h, &k->entry, k->entry.data, alloc);
	if(e)
		return 1;
	return 0;
}
