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
		alloc_special_next(p) = alloc->quar;
		alloc->quar = p;
		alloc->num_quar++;
	}
}

void 
alloc_init(struct alloc_cache* alloc, struct alloc_cache* super)
{
	memset(alloc, 0, sizeof(*alloc));
	alloc->super = super;
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
	if(!alloc->super)
		lock_quick_destroy(&alloc->lock);
	if(alloc->super && alloc->quar) {
		/* push entire list into super */
		p = alloc->quar;
		while(alloc_special_next(p)) /* find last */
			p = alloc_special_next(p);
		lock_quick_lock(&alloc->super->lock);
		alloc_special_next(p) = alloc->super->quar;
		alloc->super->quar = alloc->quar;
		alloc->super->num_quar += alloc->num_quar;
		lock_quick_unlock(&alloc->super->lock);
	} else {
		/* free */
		p = alloc->quar;
		while(p) {
			np = alloc_special_next(p);
			free(p);
			p = np;
		}
	}
	alloc->quar = 0;
	alloc->num_quar = 0;
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
		alloc_special_clean(p);
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
			alloc_special_clean(p);
			return p;
		}
	}
	/* allocate new */
	prealloc(alloc);
	if(!(p = (alloc_special_t*)malloc(sizeof(alloc_special_t))))
		fatal_exit("alloc_special_obtain: out of memory");
	alloc_special_clean(p);
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
	alloc_special_next(mem) = alloc->quar;
	for(i=1; i<ALLOC_SPECIAL_MAX/2; i++) {
		p = alloc_special_next(p);
	}
	alloc->quar = alloc_special_next(p);
	alloc->num_quar -= ALLOC_SPECIAL_MAX/2;

	/* dump mem+list into the super quar list */
	lock_quick_lock(&alloc->super->lock);
	alloc_special_next(p) = alloc->super->quar;
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
	alloc_special_clean(mem);
	if(alloc->super && alloc->num_quar >= ALLOC_SPECIAL_MAX) {
		/* push it to the super structure */
		pushintosuper(alloc, mem);
		return;
	}

	alloc_special_next(mem) = alloc->quar;
	alloc->quar = mem;
	alloc->num_quar++;
}

void 
alloc_stats(struct alloc_cache* alloc)
{
	log_info("%salloc: %d in cache.", alloc->super?"":"sup",
		(int)alloc->num_quar);
}
