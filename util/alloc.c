/*
 * util/alloc.c - memory allocation service. 
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
 */

#include "config.h"
#include "util/alloc.h"
#include "util/data/packed_rrset.h"

/** number of bits for ID part of uint64, rest for number of threads. */
#define THRNUM_SHIFT	48	/* for 65k threads, 2^48 rrsets per thr. */

/** setup new special type */
static void
alloc_setup_special(alloc_special_t* t)
{
	memset(t, 0, sizeof(*t));
	lock_rw_init(&t->entry.lock);
	t->entry.key = t;
}

/** prealloc some entries in the cache. To minimize contention. 
 * Result is 1 lock per alloc_max newly created entries.
 * @param alloc: the structure to fill up.
 */
static void
prealloc(struct alloc_cache* alloc)
{
	alloc_special_t* p;
	int i;
	for(i=0; i<ALLOC_SPECIAL_MAX; i++) {
		if(!(p = (alloc_special_t*)malloc(sizeof(alloc_special_t))))
			fatal_exit("prealloc: out of memory");
		alloc_setup_special(p);
		alloc_set_special_next(p, alloc->quar);
		alloc->quar = p;
		alloc->num_quar++;
	}
}

void 
alloc_init(struct alloc_cache* alloc, struct alloc_cache* super,
	int thread_num)
{
	memset(alloc, 0, sizeof(*alloc));
	alloc->super = super;
	alloc->thread_num = thread_num;
	alloc->next_id = (uint64_t)thread_num; 	/* in steps, so that type */
	alloc->next_id <<= THRNUM_SHIFT; 	/* of *_id is used. */
	alloc->last_id = 1; 			/* so no 64bit constants, */
	alloc->last_id <<= THRNUM_SHIFT; 	/* or implicit 'int' ops. */
	alloc->last_id -= 1; 			/* for compiler portability. */
	alloc->last_id |= alloc->next_id;
	alloc->next_id += 1;			/* because id=0 is special. */
	if(!alloc->super) {
		lock_quick_init(&alloc->lock);
		lock_protect(&alloc->lock, alloc, sizeof(*alloc));
	}
}

void 
alloc_clear(struct alloc_cache* alloc)
{
	alloc_special_t* p, *np;
	if(!alloc)
		return;
	if(!alloc->super) {
		lock_quick_destroy(&alloc->lock);
	}
	if(alloc->super && alloc->quar) {
		/* push entire list into super */
		p = alloc->quar;
		while(alloc_special_next(p)) /* find last */
			p = alloc_special_next(p);
		lock_quick_lock(&alloc->super->lock);
		alloc_set_special_next(p, alloc->super->quar);
		alloc->super->quar = alloc->quar;
		alloc->super->num_quar += alloc->num_quar;
		lock_quick_unlock(&alloc->super->lock);
	} else {
		/* free */
		p = alloc->quar;
		while(p) {
			np = alloc_special_next(p);
			/* deinit special type */
			lock_rw_destroy(&p->entry.lock);
			free(p);
			p = np;
		}
	}
	alloc->quar = 0;
	alloc->num_quar = 0;
}

uint64_t
alloc_get_id(struct alloc_cache* alloc)
{
	uint64_t id = alloc->next_id++;
	if(id == alloc->last_id) {
		/* TODO: clear the rrset cache */
		log_warn("Out of ids. Clearing cache.");
		/* start back at first number */   	/* like in alloc_init*/
		alloc->next_id = (uint64_t)alloc->thread_num; 	
		alloc->next_id <<= THRNUM_SHIFT; 	/* in steps for comp. */
		alloc->next_id += 1;			/* portability. */
		/* and generate new and safe id */
		id = alloc->next_id++;
	}
	return id;
}

alloc_special_t* 
alloc_special_obtain(struct alloc_cache* alloc)
{
	alloc_special_t* p;
	log_assert(alloc);
	/* see if in local cache */
	if(alloc->quar) {
		p = alloc->quar;
		alloc->quar = alloc_special_next(p);
		alloc->num_quar--;
		p->id = alloc_get_id(alloc);
		return p;
	}
	/* see if in global cache */
	if(alloc->super) {
		/* could maybe grab alloc_max/2 entries in one go,
		 * but really, isn't that just as fast as this code? */
		lock_quick_lock(&alloc->super->lock);
		if((p = alloc->super->quar)) {
			alloc->super->quar = alloc_special_next(p);
			alloc->super->num_quar--;
		}
		lock_quick_unlock(&alloc->super->lock);
		if(p) {
			p->id = alloc_get_id(alloc);
			return p;
		}
	}
	/* allocate new */
	prealloc(alloc);
	if(!(p = (alloc_special_t*)malloc(sizeof(alloc_special_t))))
		fatal_exit("alloc_special_obtain: out of memory");
	alloc_setup_special(p);
	p->id = alloc_get_id(alloc);
	return p;
}

/** push mem and some more items to the super */
static void 
pushintosuper(struct alloc_cache* alloc, alloc_special_t* mem)
{
	int i;
	alloc_special_t *p = alloc->quar;
	log_assert(p);
	log_assert(alloc && alloc->super && 
		alloc->num_quar >= ALLOC_SPECIAL_MAX);
	/* push ALLOC_SPECIAL_MAX/2 after mem */
	alloc_set_special_next(mem, alloc->quar);
	for(i=1; i<ALLOC_SPECIAL_MAX/2; i++) {
		p = alloc_special_next(p);
	}
	alloc->quar = alloc_special_next(p);
	alloc->num_quar -= ALLOC_SPECIAL_MAX/2;

	/* dump mem+list into the super quar list */
	lock_quick_lock(&alloc->super->lock);
	alloc_set_special_next(p, alloc->super->quar);
	alloc->super->quar = mem;
	alloc->super->num_quar += ALLOC_SPECIAL_MAX/2 + 1;
	lock_quick_unlock(&alloc->super->lock);
	/* so 1 lock per mem+alloc/2 deletes */
}

void 
alloc_special_release(struct alloc_cache* alloc, alloc_special_t* mem)
{
	log_assert(alloc);
	if(!mem)
		return;
	if(!alloc->super) { 
		lock_quick_lock(&alloc->lock); /* superalloc needs locking */
	}

	alloc_special_clean(mem);
	if(alloc->super && alloc->num_quar >= ALLOC_SPECIAL_MAX) {
		/* push it to the super structure */
		pushintosuper(alloc, mem);
		return;
	}

	alloc_set_special_next(mem, alloc->quar);
	alloc->quar = mem;
	alloc->num_quar++;
	if(!alloc->super) {
		lock_quick_unlock(&alloc->lock);
	}
}

void 
alloc_stats(struct alloc_cache* alloc)
{
	log_info("%salloc: %d in cache.", alloc->super?"":"sup",
		(int)alloc->num_quar);
}

size_t alloc_get_mem(struct alloc_cache* alloc)
{
	alloc_special_t* p;
	size_t s = sizeof(*alloc);
	if(!alloc->super) { 
		lock_quick_lock(&alloc->lock); /* superalloc needs locking */
	}
	s += sizeof(alloc_special_t) * alloc->num_quar;
	for(p = alloc->quar; p; p = alloc_special_next(p)) {
		s += lock_get_mem(&p->entry.lock);
	}
	if(!alloc->super) {
		lock_quick_unlock(&alloc->lock);
	}
	return s;
}

/** global debug value to keep track of total memory mallocs */
size_t unbound_mem_alloc = 0;
/** global debug value to keep track of total memory frees */
size_t unbound_mem_freed = 0;
#ifdef UNBOUND_ALLOC_STATS
/** special value to know if the memory is being tracked */
uint64_t mem_special = (uint64_t)0xfeed43327766abcdLL;
#ifdef malloc
#undef malloc
#endif
/** malloc with stats */
void *unbound_stat_malloc(size_t size)
{
	void* res;
	if(size == 0) size = 1;
	res = malloc(size+16);
	if(!res) return NULL;
	unbound_mem_alloc += size;
	log_info("stat %p=malloc(%u)", res+16, (unsigned)size);
	memcpy(res, &size, sizeof(size));
	memcpy(res+8, &mem_special, sizeof(mem_special));
	return res+16;
}
#ifdef calloc
#undef calloc
#endif
/** calloc with stats */
void *unbound_stat_calloc(size_t nmemb, size_t size)
{
	size_t s = (nmemb*size==0)?(size_t)1:nmemb*size;
	void* res = calloc(1, s+16);
	if(!res) return NULL;
	log_info("stat %p=calloc(%u, %u)", res+16, (unsigned)nmemb, (unsigned)size);
	unbound_mem_alloc += s;
	memcpy(res, &s, sizeof(s));
	memcpy(res+8, &mem_special, sizeof(mem_special));
	return res+16;
}
#ifdef free
#undef free
#endif
/** free with stats */
void unbound_stat_free(void *ptr)
{
	size_t s;
	if(!ptr) return;
	if(memcmp(ptr-8, &mem_special, sizeof(mem_special)) != 0) {
		free(ptr);
		return;
	}
	ptr-=16;
	memcpy(&s, ptr, sizeof(s));
	log_info("stat free(%p) size %u", ptr+16, (unsigned)s);
	memset(ptr+8, 0, 8);
	unbound_mem_freed += s;
	free(ptr);
}
#ifdef realloc
#undef realloc
#endif
/** realloc with stats */
void *unbound_stat_realloc(void *ptr, size_t size)
{
	size_t cursz;
	void* res;
	if(!ptr) return unbound_stat_malloc(size);
	if(memcmp(ptr-8, &mem_special, sizeof(mem_special)) != 0) {
		return realloc(ptr, size);
	}
	if(size==0) {
		unbound_stat_free(ptr);
		return NULL;
	}
	ptr -= 16;
	memcpy(&cursz, ptr, sizeof(cursz));
	if(cursz == size) {
		/* nothing changes */
		return ptr;
	}
	res = malloc(size+16);
	if(!res) return NULL;
	unbound_mem_alloc += size;
	unbound_mem_freed += cursz;
	log_info("stat realloc(%p, %u) from %u", ptr+16, (unsigned)size, (unsigned)cursz);
	if(cursz > size) {
		memcpy(res+16, ptr+16, size);
	} else if(size > cursz) {
		memcpy(res+16, ptr+16, cursz);
	}
	memset(ptr+8, 0, 8);
	free(ptr);
	memcpy(res, &size, sizeof(size));
	memcpy(res+8, &mem_special, sizeof(mem_special));
	return res+16;
}

/** log to file where alloc was done */
void *unbound_stat_malloc_log(size_t size, const char* file, int line,
        const char* func)
{
	log_info("%s:%d %s malloc(%u)", file, line, func, (unsigned)size);
	return unbound_stat_malloc(size);
}

/** log to file where alloc was done */
void *unbound_stat_calloc_log(size_t nmemb, size_t size, const char* file,
        int line, const char* func)
{
	log_info("%s:%d %s calloc(%u, %u)", file, line, func, 
		(unsigned) nmemb, (unsigned)size);
	return unbound_stat_calloc(nmemb, size);
}

/** log to file where free was done */
void unbound_stat_free_log(void *ptr, const char* file, int line,
        const char* func)
{
	if(ptr && memcmp(ptr-8, &mem_special, sizeof(mem_special)) == 0) {
		size_t s;
		memcpy(&s, ptr-16, sizeof(s));
		log_info("%s:%d %s free(%p) size %u", 
			file, line, func, ptr, (unsigned)s);
	} else
		log_info("%s:%d %s unmatched free(%p)", file, line, func, ptr);
	unbound_stat_free(ptr);
}

/** log to file where alloc was done */
void *unbound_stat_realloc_log(void *ptr, size_t size, const char* file,
        int line, const char* func)
{
	log_info("%s:%d %s realloc(%p, %u)", file, line, func, 
		ptr, (unsigned)size);
	return unbound_stat_realloc(ptr, size);
}

/** special routine for region-allocator, print to log */
void *unbound_stat_malloc_region(size_t size)
{
	log_info("region malloc(%u)", (unsigned)size);
	return unbound_stat_malloc(size);
}

/** special routine for region-allocator, print to log */
void unbound_stat_free_region(void *ptr)
{
	if(ptr && memcmp(ptr-8, &mem_special, sizeof(mem_special)) == 0) {
		size_t s;
		memcpy(&s, ptr-16, sizeof(s));
		log_info("region free(%p) size %u", ptr, (unsigned)s);
	} else
		log_info("region unmatched free(%p)", ptr);
	unbound_stat_free(ptr);
}

#endif /* UNBOUND_ALLOC_STATS */
