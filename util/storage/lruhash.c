/*
 * util/storage/lrulash.c - hashtable, hash function, LRU keeping.
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
 * This file contains a hashtable with LRU keeping of entries.
 *
 */

#include "config.h"
#include "util/storage/lruhash.h"

/* ------ local helper functions ------------- */

/** delete the hash bin and entries inside it */
static void bin_delete(struct lruhash* table, struct lruhash_bin* bin);

/** find entry in hash bin. You must have locked the bin.
 * @param table: hash table with function pointers.
 * @param bin: hash bin to look into.
 * @param hash: hash value to look for.
 * @param key: key to look for.
 * @return: the entry or NULL if not found.
 */
static struct lruhash_entry* bin_find_entry(struct lruhash* table, 
	struct lruhash_bin* bin, hashvalue_t hash, void* key);

/** 
 * Try to make howmuch space available for a new entry into the table.
 * works by deleting old entries.
 * Assumes that the lock on the hashtable is being held by caller.
 * @param table: hash table.
 * @param howmuch: will try to make max-used at least this amount.
 */
static void reclaim_space(struct lruhash* table, size_t howmuch);

/**
 * Grow the table lookup array. Becomes twice as large.
 * Caller must hold the hash table lock.
 * @param table: hash table.
 * @return false on error (malloc).
 */
static int table_grow(struct lruhash* table);

/* ------ end local helper functions --------- */

struct lruhash* lruhash_create(size_t start_size, size_t maxmem,
        lruhash_sizefunc_t sizefunc, lruhash_compfunc_t compfunc,
	lruhash_delkeyfunc_t delkeyfunc, lruhash_deldatafunc_t deldatafunc,
	void* arg)
{
	struct lruhash* table = (struct lruhash*)calloc(1, 
		sizeof(struct lruhash));
	size_t i;
	if(!table)
		return NULL;
	lock_quick_init(&table->lock);
	table->sizefunc = sizefunc;
	table->compfunc = compfunc;
	table->delkeyfunc = delkeyfunc;
	table->deldatafunc = deldatafunc;
	table->cb_arg = arg;
	table->size = start_size;
	table->size_mask = start_size-1;
	table->lru_start = NULL;
	table->lru_end = NULL;
	table->num = 0;
	table->space_used = 0;
	table->space_max = maxmem;
	table->array = calloc(table->size, sizeof(struct lruhash_bin));
	if(!table->array) {
		lock_quick_destroy(&table->lock);
		free(table);
		return NULL;
	}
	for(i=0; i<table->size; i++) {
		lock_quick_init(&table->array[i].lock);
	}
	lock_protect(&table->lock, table, sizeof(*table));
	lock_protect(&table->lock, table->array, 
		table->size*sizeof(struct lruhash_bin));
	return table;
}

static void bin_delete(struct lruhash* table, struct lruhash_bin* bin)
{
	struct lruhash_entry* p, *np;
	if(!bin)
		return;
	lock_quick_destroy(&bin->lock);
	p = bin->overflow_list;
	bin->overflow_list = NULL;
	while(p) {
		np = p->overflow_next;
		(*table->delkeyfunc)(p->key, table->cb_arg);
		(*table->deldatafunc)(p->data, table->cb_arg);
		p = np;
	}
}

void lruhash_delete(struct lruhash* table)
{
	size_t i;
	if(!table)
		return;
	/* delete lock on hashtable to force check its OK */
	lock_quick_destroy(&table->lock);
	for(i=0; i<table->size; i++)
		bin_delete(table, &table->array[i]);
	free(table->array);
	free(table);
}

static void 
reclaim_space(struct lruhash* table, size_t howmuch)
{
}

static struct lruhash_entry* 
bin_find_entry(struct lruhash* table, 
	struct lruhash_bin* bin, hashvalue_t hash, void* key)
{
	return NULL;
}

static int table_grow(struct lruhash* table)
{
	return 0;
}

void lruhash_insert(struct lruhash* table, hashvalue_t hash,
        struct lruhash_entry* entry, void* data)
{
	struct lruhash_bin* bin;
	struct lruhash_entry* found;
	size_t need_size;
	lock_quick_lock(&table->lock);
	/* see if table memory exceeded and needs clipping. */
	need_size = table->sizefunc(entry->key, data);
	if(table->space_used + need_size > table->space_max)
		reclaim_space(table, need_size);

	/* find bin */
	bin = &table->array[hash & table->size_mask];
	lock_quick_lock(&bin->lock);

	/* see if entry exists already */
	if(!(found=bin_find_entry(table, bin, hash, entry->key))) {
		/* if not: add to bin */
		entry->overflow_next = bin->overflow_list;
		bin->overflow_list = entry;
		table->num++;
		table->space_used += need_size;
	} else {
		/* if so: update data */
		table->space_used += need_size -
			(*table->sizefunc)(found->key, found->data);
		entry->data = NULL;
		(*table->delkeyfunc)(entry->key, table->cb_arg);
		(*table->deldatafunc)(found->data, table->cb_arg);
		found->data = data;
	}
	lock_quick_unlock(&bin->lock);

	/* see if table lookup must be grown up */
	if(table->num == table->size) {
		if(!table_grow(table))
			log_err("hash grow: malloc failed");
			/* continue with smaller array. Slower. */
	}
	lock_quick_unlock(&table->lock);
}

struct lruhash_entry* lruhash_lookup(struct lruhash* table, void* key, int wr)
{
	return NULL;
}

void lruhash_remove(struct lruhash* table, void* key)
{
}
