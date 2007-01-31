/*
 * rbtree.h -- generic red-black tree
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
 *
 */

#ifndef _RBTREE_H_
#define	_RBTREE_H_

#include "region-allocator.h"

/*
 * This structure must be the first member of the data structure in
 * the rbtree.  This allows easy casting between an rbnode_t and the
 * user data (poor man's inheritance).
 */
typedef struct rbnode_t rbnode_t;
struct rbnode_t {
	rbnode_t   *parent;
	rbnode_t   *left;
	rbnode_t   *right;
	const void *key;
	uint8_t	    color;
};

#define	RBTREE_NULL &rbtree_null_node
extern	rbnode_t	rbtree_null_node;

typedef struct rbtree_t rbtree_t;
struct rbtree_t {
	region_type *region;
	
	/* The root of the red-black tree */
	rbnode_t    *root;

	/* The number of the nodes in the tree */
	size_t       count;

	/* Current node for walks... */
	rbnode_t    *_node;

	/* Key compare function. <0,0,>0 like strcmp. Return 0 on two NULL ptrs. */
	int (*cmp) (const void *, const void *);
};

/* rbtree.c */
rbtree_t *rbtree_create(region_type *region, int (*cmpf)(const void *, const void *));
rbnode_t *rbtree_insert(rbtree_t *rbtree, rbnode_t *data);
/* returns node that is now unlinked from the tree. User to delete it. 
 * returns 0 if node not present */
rbnode_t *rbtree_delete(rbtree_t *rbtree, const void *key);
rbnode_t *rbtree_search(rbtree_t *rbtree, const void *key);
/* returns true if exact match in result. Else result points to <= element,
   or NULL if key is smaller than the smallest key. */
int rbtree_find_less_equal(rbtree_t *rbtree, const void *key, rbnode_t **result);
rbnode_t *rbtree_first(rbtree_t *rbtree);
rbnode_t *rbtree_last(rbtree_t *rbtree);
rbnode_t *rbtree_next(rbnode_t *rbtree);
rbnode_t *rbtree_previous(rbnode_t *rbtree);

#define	RBTREE_WALK(rbtree, k, d) \
	for((rbtree)->_node = rbtree_first(rbtree);\
		(rbtree)->_node != RBTREE_NULL && ((k) = (rbtree)->_node->key) && \
		((d) = (void *) (rbtree)->_node); (rbtree)->_node = rbtree_next((rbtree)->_node))

/* call with node=variable of struct* with rbnode_t as first element.
   with type is the type of a pointer to that struct. */
#define RBTREE_FOR(node, type, rbtree) \
	for(node=(type)rbtree_first(rbtree); \
		(rbnode_t*)node != RBTREE_NULL; \
		node = (type)rbtree_next((rbnode_t*)node))

#endif /* _RBTREE_H_ */
