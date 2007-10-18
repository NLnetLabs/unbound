/*
 * util/alloc.h - memory allocation service. 
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
 * This file contains memory allocation functions.
 *
 * The reasons for this service are:
 *	o Avoid locking costs of getting global lock to call malloc().
 *	o The packed rrset type needs to be kept on special freelists,
 *	  so that they are reused for other packet rrset allocations.
 *
 */

#ifndef UTIL_ALLOC_H
#define UTIL_ALLOC_H

#include "util/locks.h"
struct ub_packed_rrset_key;
struct regional;

/** The special type, packed rrset. Not allowed to be used for other memory */
typedef struct ub_packed_rrset_key alloc_special_t;
/** clean the special type. Pass pointer. */
#define alloc_special_clean(x) (x)->id = 0;
/** access next pointer. (in available spot). Pass pointer. */
#define alloc_special_next(x) ((alloc_special_t*)((x)->entry.overflow_next))
/** set next pointer. (in available spot). Pass pointers. */
#define alloc_set_special_next(x, y) \
	((x)->entry.overflow_next) = (struct lruhash_entry*)(y);

/** how many blocks to cache locally. */
#define ALLOC_SPECIAL_MAX 10

/**
 * Structure that provides allocation. Use one per thread.
 * The one on top has a NULL super pointer.
 */
struct alloc_cache {
	/** lock, only used for the super. */
	lock_quick_t lock;
	/** global allocator above this one. NULL for none (malloc/free) */
	struct alloc_cache* super;
	/** singly linked lists of special type. These are free for use. */
	alloc_special_t* quar;
	/** number of items in quarantine. */
	size_t num_quar;
	/** thread number for id creation */
	int thread_num;
	/** next id number to pass out */
	uint64_t next_id;
	/** last id number possible */
	uint64_t last_id;

	/** how many regional blocks to keep back max */
	size_t max_reg_blocks;
	/** how many regional blocks are kept now */
	size_t num_reg_blocks;
	/** linked list of regional blocks, using regional->next */
	struct regional* reg_list;
};

/**
 * Init alloc (zeroes the struct).
 * @param alloc: this parameter is allocated by the caller.
 * @param super: super to use (init that before with super_init).
 *    Pass this argument NULL to init the toplevel alloc structure.
 * @param thread_num: thread number for id creation of special type.
 */
void alloc_init(struct alloc_cache* alloc, struct alloc_cache* super,
	int thread_num);

/**
 * Free the alloc. Pushes all the cached items into the super structure.
 * Or deletes them if alloc->super is NULL.
 * Does not free the alloc struct itself (it was also allocated by caller).
 * @param alloc: is almost zeroed on exit (except some stats).
 */
void alloc_clear(struct alloc_cache* alloc);

/**
 * Get a new special_t element.
 * @param alloc: where to alloc it.
 * @return: memory block. Will not return NULL (instead fatal_exit).
 *    The block is zeroed.
 */
alloc_special_t* alloc_special_obtain(struct alloc_cache* alloc);

/**
 * Return special_t back to pool.
 * The block is cleaned up (zeroed) which also invalidates the ID inside.
 * @param alloc: where to alloc it.
 * @param mem: block to free.
 */
void alloc_special_release(struct alloc_cache* alloc, alloc_special_t* mem);

/**
 * Set ID number of special type to a fresh new ID number.
 * In case of ID number overflow, the rrset cache has to be cleared.
 * @param alloc: the alloc cache
 * @return: fresh id is returned.
 */
uint64_t alloc_get_id(struct alloc_cache* alloc);

/**
 * Get memory size of alloc cache, alloc structure including special types.
 * @param alloc: on what alloc.
 * @return size in bytes.
 */
size_t alloc_get_mem(struct alloc_cache* alloc);

/**
 * Print debug information (statistics).
 * @param alloc: on what alloc.
 */
void alloc_stats(struct alloc_cache* alloc);

/**
 * Get a new regional for query states
 * @param alloc: where to alloc it.
 * @return regional for use or NULL on alloc failure.
 */
struct regional* alloc_reg_obtain(struct alloc_cache* alloc);

/**
 * Put regional for query states back into alloc cache.
 * @param alloc: where to alloc it.
 * @param r: regional to put back.
 */
void alloc_reg_release(struct alloc_cache* alloc, struct regional* r);

#endif /* UTIL_ALLOC_H */
