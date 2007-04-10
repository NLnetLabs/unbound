/*
 * region-allocator.c -- region based memory allocator.
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

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "region-allocator.h"

#ifdef ALIGNMENT
#  undef ALIGNMENT
#endif
#define ALIGN_UP(x, s)     (((x) + s - 1) & (~(s - 1)))
#define ALIGNMENT          (sizeof(void *))
#define CHECK_DOUBLE_FREE 0 /* set to 1 to perform expensive check for double recycle() */

typedef struct cleanup cleanup_type;
struct cleanup
{
	void (*action)(void *);
	void *data;
};

struct recycle_elem {
	struct recycle_elem* next;
};

struct region
{
	size_t        total_allocated;
	size_t        small_objects;
	size_t        large_objects;
	size_t        chunk_count;
	size_t        unused_space; /* Unused space due to alignment, etc. */
	
	size_t        allocated;
	char         *initial_data;
	char         *data;

	void         *(*allocator)(size_t);
	void          (*deallocator)(void *);
    
	size_t        maximum_cleanup_count;
	size_t        cleanup_count;
	cleanup_type *cleanups;

	size_t        chunk_size;
	size_t        large_object_size;

	/* if not NULL recycling is enabled.
	 * It is an array of linked lists of parts held for recycle.
	 * The parts are all pointers to within the allocated chunks.
	 * Array [i] points to elements of size i. */
	struct recycle_elem** recycle_bin;
	/* amount of memory in recycle storage */
	size_t		recycle_size;
};


static region_type *
alloc_region_base(void *(*allocator)(size_t size),
		  void (*deallocator)(void *),
		  size_t initial_cleanup_count)
{
	region_type *result = (region_type *) allocator(sizeof(region_type));
	if (!result) return NULL;

	result->total_allocated = 0;
	result->small_objects = 0;
	result->large_objects = 0;
	result->chunk_count = 1;
	result->unused_space = 0;
	result->recycle_bin = NULL;
	result->recycle_size = 0;
	
	result->allocated = 0;
	result->data = NULL;
	result->initial_data = NULL;

	result->allocator = allocator;
	result->deallocator = deallocator;

	assert(initial_cleanup_count > 0);
	result->maximum_cleanup_count = initial_cleanup_count;
	result->cleanup_count = 0;
	result->cleanups = (cleanup_type *) allocator(
		result->maximum_cleanup_count * sizeof(cleanup_type));
	if (!result->cleanups) {
		deallocator(result);
		return NULL;
	}

	result->chunk_size = DEFAULT_CHUNK_SIZE;
	result->large_object_size = DEFAULT_LARGE_OBJECT_SIZE;
	return result;
}

region_type *
region_create(void *(*allocator)(size_t size),
	      void (*deallocator)(void *))
{
	region_type* result = alloc_region_base(allocator, deallocator, 
		DEFAULT_INITIAL_CLEANUP_SIZE);
	if(!result)
		return NULL;
	result->data = (char *) allocator(result->chunk_size);
	if (!result->data) {
		deallocator(result->cleanups);
		deallocator(result);
		return NULL;
	}
	result->initial_data = result->data;
    
	return result;
}


region_type *region_create_custom(void *(*allocator)(size_t),
				  void (*deallocator)(void *),
				  size_t chunk_size,
				  size_t large_object_size,
				  size_t initial_cleanup_size,
				  int recycle)
{
	region_type* result = alloc_region_base(allocator, deallocator, 
		initial_cleanup_size);
	if(!result)
		return NULL;
	assert(large_object_size <= chunk_size);
	result->chunk_size = chunk_size;
	result->large_object_size = large_object_size;
	if(result->chunk_size > 0) {
		result->data = (char *) allocator(result->chunk_size);
		if (!result->data) {
			deallocator(result->cleanups);
			deallocator(result);
			return NULL;
		}
		result->initial_data = result->data;
	}
	if(recycle) {
		result->recycle_bin = allocator(sizeof(struct recycle_elem*)
			* result->large_object_size);
		if(!result->recycle_bin) {
			region_destroy(result);
			return NULL;
		}
		memset(result->recycle_bin, 0, sizeof(struct recycle_elem*)
			* result->large_object_size);
	}
	return result;
}


void
region_destroy(region_type *region)
{
	void (*deallocator)(void *);
	if (!region)
		return;

	deallocator = region->deallocator;

	region_free_all(region);
	deallocator(region->cleanups);
	deallocator(region->initial_data);
	if(region->recycle_bin)
		deallocator(region->recycle_bin);
	deallocator(region);
}


size_t
region_add_cleanup(region_type *region, void (*action)(void *), void *data)
{
	assert(action);
    
	if (region->cleanup_count >= region->maximum_cleanup_count) {
		cleanup_type *cleanups = (cleanup_type *) region->allocator(
			2 * region->maximum_cleanup_count * sizeof(cleanup_type));
		if (!cleanups) return 0;

		memcpy(cleanups, region->cleanups,
		       region->cleanup_count * sizeof(cleanup_type));
		region->deallocator(region->cleanups);

		region->cleanups = cleanups;
		region->maximum_cleanup_count *= 2;
	}

	region->cleanups[region->cleanup_count].action = action;
	region->cleanups[region->cleanup_count].data = data;

	++region->cleanup_count;
	return region->cleanup_count;
}

void *
region_alloc(region_type *region, size_t size)
{
	size_t aligned_size;
	void *result;

	if (size == 0) {
		size = 1;
	}
	aligned_size = ALIGN_UP(size, ALIGNMENT);

	if (aligned_size >= region->large_object_size) {
		result = region->allocator(size);
		if (!result) return NULL;
        
		if (!region_add_cleanup(region, region->deallocator, result)) {
			region->deallocator(result);
			return NULL;
		}
        
		region->total_allocated += size;
		++region->large_objects;
		
		return result;
	}

	if (region->recycle_bin && region->recycle_bin[aligned_size]) {
		result = (void*)region->recycle_bin[aligned_size];
		region->recycle_bin[aligned_size] = region->recycle_bin[aligned_size]->next; 
		region->recycle_size -= aligned_size;
		region->unused_space += aligned_size - size;
		return result;
	}
    
	if (region->allocated + aligned_size > region->chunk_size) {
		void *chunk = region->allocator(region->chunk_size);
		size_t wasted;
		if (!chunk) return NULL;

		wasted = (region->chunk_size - region->allocated) & (~(ALIGNMENT-1));
		if(wasted >= ALIGNMENT) {
			/* put wasted part in recycle bin for later use */
			region->total_allocated += wasted;
			++region->small_objects;
			region_recycle(region, region->data+region->allocated, wasted);
			region->allocated += wasted;
		}
		++region->chunk_count;
		region->unused_space += region->chunk_size - region->allocated;
		
		region_add_cleanup(region, region->deallocator, chunk);
		region->allocated = 0;
		region->data = (char *) chunk;
	}

	result = region->data + region->allocated;
	region->allocated += aligned_size;

	region->total_allocated += aligned_size;
	region->unused_space += aligned_size - size;
	++region->small_objects;
	
	return result;
}

void *
region_alloc_init(region_type *region, const void *init, size_t size)
{
	void *result = region_alloc(region, size);
	if (!result) return NULL;
	memcpy(result, init, size);
	return result;
}

void *
region_alloc_zero(region_type *region, size_t size)
{
	void *result = region_alloc(region, size);
	if (!result) return NULL;
	memset(result, 0, size);
	return result;
}

void
region_free_all(region_type *region)
{
	size_t i;
	assert(region);
	assert(region->cleanups);
    
	i = region->cleanup_count;
	while (i > 0) {
		--i;
		assert(region->cleanups[i].action);
		region->cleanups[i].action(region->cleanups[i].data);
	}

	if(region->recycle_bin) {
		memset(region->recycle_bin, 0, sizeof(struct recycle_elem*)
			* region->large_object_size);
		region->recycle_size = 0;
	}

	region->data = region->initial_data;
	region->cleanup_count = 0;
	region->allocated = 0;

	region->total_allocated = 0;
	region->small_objects = 0;
	region->large_objects = 0;
	region->chunk_count = 1;
	region->unused_space = 0;
}


char *
region_strdup(region_type *region, const char *string)
{
	return (char *) region_alloc_init(region, string, strlen(string) + 1);
}

void 
region_recycle(region_type *region, void *block, size_t size)
{
	size_t aligned_size;
	size_t i;

	if(!block || !region->recycle_bin)
		return;

	if (size == 0) {
		size = 1;
	}
	aligned_size = ALIGN_UP(size, ALIGNMENT);

	if(aligned_size < region->large_object_size) {
		struct recycle_elem* elem = (struct recycle_elem*)block;
		/* we rely on the fact that ALIGNMENT is void* so the next will fit */
		assert(aligned_size >= sizeof(struct recycle_elem));

		if(CHECK_DOUBLE_FREE) {
			/* make sure the same ptr is not freed twice. */
			struct recycle_elem *p = region->recycle_bin[aligned_size];
			while(p) {
				assert(p != elem); 
				p = p->next;
			}
		}

		elem->next = region->recycle_bin[aligned_size];
		region->recycle_bin[aligned_size] = elem;
		region->recycle_size += aligned_size;
		region->unused_space -= aligned_size - size;
		return;
	}

	/* a large allocation */
	region->total_allocated -= size;
	--region->large_objects;
	for(i=0; i<region->cleanup_count; i++) {
		while(region->cleanups[i].data == block) {
			/* perform action (deallocator) on block */
			region->cleanups[i].action(block);
			region->cleanups[i].data = NULL;
			/* remove cleanup - move last entry here, check this one again */
			--region->cleanup_count;
			region->cleanups[i].action = 
				region->cleanups[region->cleanup_count].action;
			region->cleanups[i].data = 
				region->cleanups[region->cleanup_count].data;
		}
	}
}

void
region_dump_stats(region_type *region, FILE *out)
{
	fprintf(out, "%lu objects (%lu small/%lu large), %lu bytes allocated (%lu wasted) in %lu chunks, %lu cleanups, %lu in recyclebin",
		(unsigned long) (region->small_objects + region->large_objects),
		(unsigned long) region->small_objects,
		(unsigned long) region->large_objects,
		(unsigned long) region->total_allocated,
		(unsigned long) region->unused_space,
		(unsigned long) region->chunk_count,
		(unsigned long) region->cleanup_count,
		(unsigned long) region->recycle_size);
	if(1 && region->recycle_bin) {
		/* print details of the recycle bin */
		size_t i;
		for(i=0; i<region->large_object_size; i++) {
			size_t count = 0;
			struct recycle_elem* el = region->recycle_bin[i];
			while(el) {
				count++;
				el = el->next;
			}
			if(i%ALIGNMENT == 0 && i!=0)
				fprintf(out, " %lu", (unsigned long)count);
		}
	}
}

size_t region_get_recycle_size(region_type* region)
{
	return region->recycle_size;
}

/* debug routine, includes here to keep base region-allocator independent */
#undef ALIGN_UP
#include "util/log.h"
void
region_log_stats(region_type *region)
{
	char buf[10240], *str=buf;
	int len=0;
	sprintf(str, "%lu objects (%lu small/%lu large), %lu bytes allocated (%lu wasted) in %lu chunks, %lu cleanups, %lu in recyclebin%n",
		(unsigned long) (region->small_objects + region->large_objects),
		(unsigned long) region->small_objects,
		(unsigned long) region->large_objects,
		(unsigned long) region->total_allocated,
		(unsigned long) region->unused_space,
		(unsigned long) region->chunk_count,
		(unsigned long) region->cleanup_count,
		(unsigned long) region->recycle_size,
		&len);
	str+=len;
	if(1 && region->recycle_bin) {
		/* print details of the recycle bin */
		size_t i;
		for(i=0; i<region->large_object_size; i++) {
			size_t count = 0;
			struct recycle_elem* el = region->recycle_bin[i];
			while(el) {
				count++;
				el = el->next;
			}
			if(i%ALIGNMENT == 0 && i!=0) {
				sprintf(str, " %lu%n", (unsigned long)count, 
					&len);
				str+=len;
			}
		}
	}
	log_info("memory: %s", buf);
}
