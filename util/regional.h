/*
 * regional.h -- region based memory allocator.
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
 * Region allocator. Allocates small portions of of larger chunks.
 *
 * Different from (nsd) region-allocator.h
 * 	o does not have recycle bin
 * 	o does not collect stats
 * 	o does not keep cleanup list
 * 	o does not have function pointers to setup
 * 	o allocs the regional struct inside the first block.
 * 	o can take a block to create regional from.
 */

#ifndef UTIL_REGIONAL_H_
#define UTIL_REGIONAL_H_

/** 
 * the regional* is the first block*.
 * every block has a ptr to the next in first bytes.
 * and so does the regional struct, which is the first block.
 */
struct regional
{
	/** next chunk. NULL if first chunk is the only chunk. */
	char* next;
	/** number of bytes allocated in the current chunk. */
	size_t allocated;
	/** initial chunk size */
	size_t first_size;
	/** current chunk size */
	size_t size;
	/** total large size */
	size_t total_large;
	/** 
	 * current chunk 
	 * can be cast to char** to obtain next chunk pointer.
	 */
	char *data;
	/** first large object, cast to char** to obtain next ptr */
	char* large_list;
};

/**
 * Create a new regional.
 */
struct regional* regional_create();

/**
 * Create a new region, with custom settings.
 * @param block: initial data block 
 * 	The regional returned is allocated in the block (start of it).
 * @param size: length of block.
 */
struct regional* regional_create_custom(void* block, size_t size);
	
/**
 * Free all memory associated with REGION. Only keeps the first block with
 * the regional inside it.
 * @param r: the region.
 */
void regional_free_all(struct regional *r);

/**
 * Destroy regional.  All memory associated with regional is freed as if
 * regional_free_all was called, as well as destroying the regional struct.
 * @param r: to delete.
 */
void regional_destroy(struct regional *r);

/**
 * Allocate SIZE bytes of memory inside REGION.  The memory is
 * deallocated when region_free_all is called for this region.
 * @param r: the region.
 * @param size: number of bytes.
 * @return: pointer to memory allocated.
 */
void *regional_alloc(struct regional *r, size_t size);

/**
 * Allocate SIZE bytes of memory inside REGION and copy INIT into it.
 * The memory is deallocated when region_free_all is called for this
 * region.
 * @param r: the region.
 * @param init: to copy.
 * @param size: number of bytes.
 * @return: pointer to memory allocated.
 */
void *regional_alloc_init(struct regional* r, const void *init, size_t size);

/**
 * Allocate SIZE bytes of memory inside REGION that are initialized to
 * 0.  The memory is deallocated when region_free_all is called for
 * this region.
 * @param r: the region.
 * @param size: number of bytes.
 * @return: pointer to memory allocated.
 */
void *regional_alloc_zero(struct regional *r, size_t size);

/**
 * Duplicate STRING and allocate the result in REGION.
 * @param r: the region.
 * @param string: null terminated string.
 */
char *regional_strdup(struct regional *r, const char *string);

/** Debug print REGION statistics to LOG */
void regional_log_stats(struct regional *r);

/** get total memory size in use by region */
size_t regional_get_mem(struct regional* r);

#endif /* UTIL_REGIONAL_H_ */
