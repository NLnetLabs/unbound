/*
 * util/storage/slabhash.c - hashtable consisting of several smaller tables.
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
 * Implementation of hash table that consists of smaller hash tables.
 * This results in a partitioned lruhash table.
 * It cannot grow, but that gives it the ability to have multiple
 * locks. Also this means there are multiple LRU lists.
 */

#include "config.h"
#include "util/storage/slabhash.h"

struct slabhash* slabhash_create(size_t numtables, size_t start_size, 
	size_t maxmem, lruhash_sizefunc_t sizefunc, 
	lruhash_compfunc_t compfunc, lruhash_delkeyfunc_t delkeyfunc, 
	lruhash_deldatafunc_t deldatafunc, void* arg)
{
	size_t i;
	struct slabhash* sl = (struct slabhash*)calloc(1, 
		sizeof(struct slabhash));
	if(!sl) return NULL;
	sl->size = numtables;
	log_assert(sl->size > 0);
	sl->array = (struct lruhash**)calloc(sl->size, sizeof(struct lruhash*));
	if(!sl->array) {
		free(sl);
		return NULL;
	}
	sl->mask = (uint32_t)(sl->size - 1);
	if(sl->mask == 0) {
		sl->shift = 0;
	} else {
		log_assert( (sl->size & sl->mask) == 0 
			/* size must be power of 2 */ );
		sl->shift = 0;
		while(!(sl->mask & 0x80000000)) {
			sl->mask <<= 1;
			sl->shift ++;
		}
	}
	for(i=0; i<sl->size; i++) {
		sl->array[i] = lruhash_create(start_size, maxmem / sl->size,
			sizefunc, compfunc, delkeyfunc, deldatafunc, arg);
		if(!sl->array[i]) {
			slabhash_delete(sl);
			return NULL;
		}
	}
	return sl;
}

void slabhash_delete(struct slabhash* sl)
{
	if(!sl)
		return;
	if(sl->array) {
		size_t i;
		for(i=0; i<sl->size; i++)
			lruhash_delete(sl->array[i]);
		free(sl->array);
	}
	free(sl);
}

/** helper routine to calculate the slabhash index */
static unsigned int
slab_idx(struct slabhash* sl, hashvalue_t hash)
{
	return ((hash & sl->mask) >> sl->shift);
}

void slabhash_insert(struct slabhash* sl, hashvalue_t hash, 
	struct lruhash_entry* entry, void* data, void* arg)
{
	lruhash_insert(sl->array[slab_idx(sl, hash)], hash, entry, data, arg);
}

struct lruhash_entry* slabhash_lookup(struct slabhash* sl, 
	hashvalue_t hash, void* key, int wr)
{
	return lruhash_lookup(sl->array[slab_idx(sl, hash)], hash, key, wr);
}

void slabhash_remove(struct slabhash* sl, hashvalue_t hash, void* key)
{
	lruhash_remove(sl->array[slab_idx(sl, hash)], hash, key);
}

void slabhash_status(struct slabhash* sl, const char* id, int extended)
{
	size_t i;
	char num[17];
	log_info("Slabhash %s: %u tables mask=%x shift=%d", 
		id, (unsigned)sl->size, sl->mask, sl->shift);
	for(i=0; i<sl->size; i++) {
		snprintf(num, sizeof(num), "table %u", (unsigned)i);
		lruhash_status(sl->array[i], num, extended);
	}
}

size_t slabhash_get_size(struct slabhash* sl)
{
	size_t i, total = 0;
	for(i=0; i<sl->size; i++) {
		lock_quick_lock(&sl->array[i]->lock);
		total += sl->array[i]->space_max;
		lock_quick_unlock(&sl->array[i]->lock);
	}
	return total;
}

size_t slabhash_get_mem(struct slabhash* sl)
{	
	size_t i, total = sizeof(*sl);
	total += sizeof(struct lruhash*)*sl->size;
	for(i=0; i<sl->size; i++) {
		total += lruhash_get_mem(sl->array[i]);
	}
	return total;
}

struct lruhash* slabhash_gettable(struct slabhash* sl, hashvalue_t hash)
{
	return sl->array[slab_idx(sl, hash)];
}
