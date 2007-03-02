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
 *	o This service is not there to improve speed of malloc.
 *	  Some caching is performed to help avoid locking costs.
 *      o This service does not prevent fragmentation.
 *	  The caching will help somewhat for this.
 *
 * Design choices:
 *	o The global malloc/free is used to handle fragmentation, etc.
 *	  If freelists become very large, it is returned to the system.
 *	o Only 1k and smaller is cached, bigger uses malloc.
 *	  Because DNS fragments are mostly this size.
 *	o On startup preallocated memory can be given, so threads can
 *	  avoid contention in the startup phase.
 */

#ifndef UTIL_ALLOC_H
#define UTIL_ALLOC_H

#include "util/locks.h"

/** all allocations are in multiples of this size */
#define ALLOC_ALIGN	32	/* bytes */

/** 
 * This size and smaller is kept in cached lists by size.
 * Must be a multiple of ALLOC_ALIGN.
 */
#define ALLOC_LARGESIZE	1024	/* bytes */

/** number of bins */
#define ALLOC_BINS (ALLOC_LARGESIZE / ALLOC_ALIGN)

/** The special type, packed rrset. Not allowed to be used for other memory */
typedef uint64_t alloc_special_t;
/** clean the special type. Pass pointer. */
#define alloc_special_clean(x) memset(x, 0, sizeof(alloc_special_t))
/** access next pointer. (in available spot). Pass pointer. */
#define alloc_special_next(x) ((alloc_special_t*)(*(x)))

/** The number of cached items. */
#define ALLOC_CACHENUM	10	/* memory blocks */

/** Preallocated per thread. This number for every size. */
#define ALLOC_PREALLOC	32	/* memory blocks */

/** shorthand */
typedef struct alloc_cache alloc_t;
/**
 * Structure that provides caching based on size. Used one per thread.
 */
struct alloc_cache {
	/** global allocator above this one. NULL for none (malloc/free) */
	struct alloc_super* super;
	/** singly linked lists per size: [0]32, [1]64, ... [n-1]LARGESIZE */
	void* bins[ALLOC_BINS];
	/** the number of items per bin. */
	size_t nums[ALLOC_BINS];
	/** singly linked lists of special type. These are free for use. */
	alloc_special_t* quar;
	/** number of items in quarantine. */
	size_t num_quar;

	/* some statistics */
	/** amount allocated. */
	size_t bytes_allocated;
	/** number of items allocated. */
	size_t items_allocated;
	/** wasted space due to assigning oversized aligned blocks. */
	size_t internal_frag;
	/** amount in cache. */
	size_t bytes_in_bins;
	/** number of special type allocated */
	size_t special_allocated;
};

/**
 * Structure with lock to provide global cache of special items and allocs.
 */
struct alloc_super {
	/** lock for single access. */
	lock_quick_t lock;
	/** singly linked lists of special type. These are free for use. */
	alloc_special_t* quar;
	/** number of items in quarantine. */
	size_t num_quar;
};

/**
 * Init super alloc. (zeroes the struct, inits the lock).
 * @param super: to init.
 */
void asuper_init(struct alloc_super* super);

/**
 * Init alloc (zeroes the struct).
 * @param alloc: this parameter is allocated by the caller.
 * @param super: super to use (init that before with super_init).
 */
void alloc_init(alloc_t* alloc, struct alloc_super* super);

/**
 * Free the alloc. Pushes all the cached items into the super structure.
 * @param alloc: is almost zeroed on exit (except some stats).
 */
void alloc_delete(alloc_t* alloc);

/**
 * Allocate memory
 * @param alloc: where to alloc it.
 * @param size: how much.
 * @return: memory block. Will not return NULL (instead fatal_exit).
 */
void* alloc_alloc(alloc_t* alloc, size_t size);

/**
 * Free memory.
 * @param alloc: where to alloc it.
 * @param mem: block to free.
 */
void alloc_free(alloc_t* alloc, void* mem);

/**
 * Get a new special_t element.
 * @param alloc: where to alloc it.
 * @return: memory block. Will not return NULL (instead fatal_exit).
 */
alloc_special_t* alloc_special_alloc(alloc_t* alloc);

/**
 * Return special_t back to pool.
 * @param alloc: where to alloc it.
 * @param mem: block to free.
 */
void alloc_special_free(alloc_t* alloc, alloc_special_t* mem);


#endif /* UTIL_ALLOC_H */
