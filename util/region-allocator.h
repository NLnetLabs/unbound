/*
 * region-allocator.h -- region based memory allocator.
 *
 * Copyright (c) 2001-2006, NLnet Labs. All rights reserved.
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

#ifndef _REGION_ALLOCATOR_H_
#define _REGION_ALLOCATOR_H_

#include <stdio.h>

typedef struct region region_type;

#define DEFAULT_CHUNK_SIZE         4096
#define DEFAULT_LARGE_OBJECT_SIZE  (DEFAULT_CHUNK_SIZE / 8)
#define DEFAULT_INITIAL_CLEANUP_SIZE 16

/*
 * Create a new region.
 */
region_type *region_create(void *(*allocator)(size_t),
			   void (*deallocator)(void *));


/*
 * Create a new region, with chunk size and large object size.
 * Note that large_object_size must be <= chunk_size.
 * Anything larger than the large object size is individually alloced.
 * large_object_size = chunk_size/8 is reasonable;
 * initial_cleanup_size is the number of prealloced ptrs for cleanups.
 * The cleanups are in a growing array, and it must start larger than zero.
 * If recycle is true, environmentally friendly memory recycling is be enabled.
 */
region_type *region_create_custom(void *(*allocator)(size_t),
				  void (*deallocator)(void *),
				  size_t chunk_size,
				  size_t large_object_size,
				  size_t initial_cleanup_size,
				  int recycle);


/*
 * Destroy REGION.  All memory associated with REGION is freed as if
 * region_free_all was called.
 */
void region_destroy(region_type *region);


/*
 * Add a cleanup to REGION.  ACTION will be called with DATA as
 * parameter when the region is freed or destroyed.
 *
 * Returns 0 on failure.
 */
size_t region_add_cleanup(region_type *region,
			  void (*action)(void *),
			  void *data);


/*
 * Allocate SIZE bytes of memory inside REGION.  The memory is
 * deallocated when region_free_all is called for this region.
 */
void *region_alloc(region_type *region, size_t size);


/*
 * Allocate SIZE bytes of memory inside REGION and copy INIT into it.
 * The memory is deallocated when region_free_all is called for this
 * region.
 */
void *region_alloc_init(region_type *region, const void *init, size_t size);


/*
 * Allocate SIZE bytes of memory inside REGION that are initialized to
 * 0.  The memory is deallocated when region_free_all is called for
 * this region.
 */
void *region_alloc_zero(region_type *region, size_t size);


/*
 * Run the cleanup actions and free all memory associated with REGION.
 */
void region_free_all(region_type *region);


/*
 * Duplicate STRING and allocate the result in REGION.
 */
char *region_strdup(region_type *region, const char *string);

/*
 * Recycle an allocated memory block. Pass size used to alloc it.
 * Does nothing if recycling is not enabled for the region.
 */
void region_recycle(region_type *region, void *block, size_t size);

/*
 * Print some REGION statistics to OUT.
 */
void region_dump_stats(region_type *region, FILE *out);

/* get size of recyclebin */
size_t region_get_recycle_size(region_type* region);

/* Debug print REGION statistics to LOG. */
void region_log_stats(region_type *region);

#endif /* _REGION_ALLOCATOR_H_ */
