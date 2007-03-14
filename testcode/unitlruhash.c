/*
 * testcode/unitlruhash.c - unit test for lruhash table.
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
 *
 */
/**
 * \file
 * Tests the locking LRU keeping hash table implementation.
 */

#include "config.h"
#include "testcode/unitmain.h"
#include "util/log.h"
#include "util/storage/lruhash.h"

/* --- test representation --- */
/** structure contains test key */
struct testkey {
	/** the key id */
	int id;
	/** the entry */
	struct lruhash_entry entry;
};
/** structure contains test data */
struct testdata {
	/** data value */
	int data;
};

/** sizefunc for lruhash */
static size_t test_sizefunc(void*, void*);
/** comparefunc for lruhash */
static int test_compfunc(void*, void*);
/** delkey for lruhash */
static void test_delkey(void*, void*);
/** deldata for lruhash */
static void test_deldata(void*, void*);
/* --- end test representation --- */

/** hash func, very bad to improve collisions. */
static hashvalue_t myhash(int id) {return id & 0x0f;}
/** allocate new key, fill in hash. */
static struct testkey* newkey(int id) {
	struct testkey* k = (struct testkey*)calloc(1, sizeof(struct testkey));
	k->id = id;
	k->entry.hash = myhash(id);
	k->entry.key = k;
	lock_rw_init(&k->entry.lock);
	return k;
}
/** new data el */
static struct testdata* newdata(int val) {
	struct testdata* d = (struct testdata*)calloc(1, 
		sizeof(struct testdata));
	d->data = val;
	return d;
}
/** delete key */
static void delkey(struct testkey* k) {	
	lock_rw_destroy(&k->entry.lock); free(k);}
/** delete data */
static void deldata(struct testdata* d) {free(d);}

/** test bin_find_entry function and bin_overflow_remove */
static void
test_bin_find_entry(struct lruhash* table)
{
	struct testkey* k = newkey(12);
	struct testdata* d = newdata(128);
	struct testkey* k2 = newkey(12 + 1024);
	struct testkey* k3 = newkey(14);
	struct testkey* k4 = newkey(12 + 1024*2);
	hashvalue_t h = myhash(12);
	struct lruhash_bin bin;
	bin_init(&bin, 1);

	/* remove from empty list */
	bin_overflow_remove(&bin, &k->entry);

	/* find in empty list */
	unit_assert( bin_find_entry(table, &bin, h, k) == NULL );

	/* insert */
	lock_quick_lock(&bin.lock);
	bin.overflow_list = &k->entry;
	lock_quick_unlock(&bin.lock);

	/* find, hash not OK. */
	unit_assert( bin_find_entry(table, &bin, myhash(13), k) == NULL );

	/* find, hash OK, but cmp not */
	unit_assert( k->entry.hash == k2->entry.hash );
	unit_assert( bin_find_entry(table, &bin, h, k2) == NULL );

	/* find, hash OK, and cmp too */
	unit_assert( bin_find_entry(table, &bin, h, k) == &k->entry );

	/* remove the element */
	lock_quick_lock(&bin.lock);
	bin_overflow_remove(&bin, &k->entry);
	lock_quick_unlock(&bin.lock);
	unit_assert( bin_find_entry(table, &bin, h, k) == NULL );

	/* prepend two different elements; so the list is long */
	/* one has the same hash, but different cmp */
	lock_quick_lock(&bin.lock);
	unit_assert( k->entry.hash == k4->entry.hash );
	k4->entry.overflow_next = &k->entry;
	k3->entry.overflow_next = &k4->entry;
	bin.overflow_list = &k3->entry;
	lock_quick_unlock(&bin.lock);

	/* find, hash not OK. */
	unit_assert( bin_find_entry(table, &bin, myhash(13), k) == NULL );

	/* find, hash OK, but cmp not */
	unit_assert( k->entry.hash == k2->entry.hash );
	unit_assert( bin_find_entry(table, &bin, h, k2) == NULL );

	/* find, hash OK, and cmp too */
	unit_assert( bin_find_entry(table, &bin, h, k) == &k->entry );

	/* remove middle element */
	unit_assert( bin_find_entry(table, &bin, k4->entry.hash, k4) 
		== &k4->entry );
	lock_quick_lock(&bin.lock);
	bin_overflow_remove(&bin, &k4->entry);
	lock_quick_unlock(&bin.lock);
	unit_assert( bin_find_entry(table, &bin, k4->entry.hash, k4) == NULL);

	/* remove last element */
	lock_quick_lock(&bin.lock);
	bin_overflow_remove(&bin, &k->entry);
	lock_quick_unlock(&bin.lock);
	unit_assert( bin_find_entry(table, &bin, h, k) == NULL );

	lock_quick_destroy(&bin.lock);
	delkey(k);
	delkey(k2);
	delkey(k3);
	delkey(k4);
	deldata(d);
}

/** test lru_front lru_remove */
static void test_lru(struct lruhash* table)
{
	struct testkey* k = newkey(12);
	struct testkey* k2 = newkey(14);
	lock_quick_lock(&table->lock);

	unit_assert( table->lru_start == NULL && table->lru_end == NULL);
	lru_remove(table, &k->entry);
	unit_assert( table->lru_start == NULL && table->lru_end == NULL);

	/* add one */
	lru_front(table, &k->entry);
	unit_assert( table->lru_start == &k->entry && 
		table->lru_end == &k->entry);
	/* remove it */
	lru_remove(table, &k->entry);
	unit_assert( table->lru_start == NULL && table->lru_end == NULL);

	/* add two */
	lru_front(table, &k->entry);
	unit_assert( table->lru_start == &k->entry && 
		table->lru_end == &k->entry);
	lru_front(table, &k2->entry);
	unit_assert( table->lru_start == &k2->entry && 
		table->lru_end == &k->entry);
	/* remove first in list */
	lru_remove(table, &k2->entry);
	unit_assert( table->lru_start == &k->entry && 
		table->lru_end == &k->entry);
	lru_front(table, &k2->entry);
	unit_assert( table->lru_start == &k2->entry && 
		table->lru_end == &k->entry);
	/* remove last in list */
	lru_remove(table, &k->entry);
	unit_assert( table->lru_start == &k2->entry && 
		table->lru_end == &k2->entry);

	/* empty the list */
	lru_remove(table, &k2->entry);
	unit_assert( table->lru_start == NULL && table->lru_end == NULL);
	lock_quick_unlock(&table->lock);
	delkey(k);
	delkey(k2);
}

void lruhash_test()
{
	/* start very very small array, so it can do lots of table_grow() */
	/* also small in size so that reclaim has to be done quickly. */
	struct lruhash* table = lruhash_create(2, 1024, 
		test_sizefunc, test_compfunc, test_delkey, test_deldata, NULL);
	test_bin_find_entry(table);
	test_lru(table);
	/* hashtable tests go here */
	lruhash_delete(table);
}

static size_t test_sizefunc(void* ATTR_UNUSED(key), void* ATTR_UNUSED(data))
{
	return sizeof(struct testkey) + sizeof(struct testdata);
}

static int test_compfunc(void* key1, void* key2)
{
	struct testkey* k1 = (struct testkey*)key1;
	struct testkey* k2 = (struct testkey*)key2;
	if(k1->id == k2->id)
		return 0;
	if(k1->id > k2->id)
		return 1;
	return -1;
}

static void test_delkey(void* key, void* ATTR_UNUSED(arg))
{
	delkey((struct testkey*)key);
}

static void test_deldata(void* data, void* ATTR_UNUSED(arg))
{
	deldata((struct testdata*)data);
}
