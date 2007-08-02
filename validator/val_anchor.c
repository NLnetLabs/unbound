/*
 * validator/val_anchor.c - validator trust anchor storage.
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
 * This file contains storage for the trust anchors for the validator.
 */
#include "config.h"
#include "validator/val_anchor.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/log.h"
#include "util/region-allocator.h"
#include "util/config_file.h"

/** compare two trust anchors */
static int
anchor_cmp(const void* k1, const void* k2)
{
	int m;
	struct trust_anchor* n1 = (struct trust_anchor*)k1;
	struct trust_anchor* n2 = (struct trust_anchor*)k2;
	/* no need to ntohs(class) because sort order is irrelevant */
	if(n1->dclass != n2->dclass) {
		if(n1->dclass < n2->dclass)
			return -1;
		return 1;
	}
	return dname_lab_cmp(n1->name, n1->namelabs, n2->name, n2->namelabs, 
		&m);
}

struct val_anchors* 
anchors_create()
{
	struct val_anchors* a = (struct val_anchors*)calloc(1, sizeof(*a));
	if(!a)
		return NULL;
	a->region = region_create(malloc, free);
	if(!a->region) {
		free(a);
		return NULL;
	}
	a->tree = rbtree_create(anchor_cmp);
	if(!a->tree) {
		anchors_delete(a);
		return NULL;
	}
	return a;
}

void 
anchors_delete(struct val_anchors* anchors)
{
	if(!anchors)
		return;
	free(anchors->tree);
	region_destroy(anchors->region);
	free(anchors);
}

/** initialise parent pointers in the tree */
static void
init_parents(struct val_anchors* anchors)
{
	struct trust_anchor* node, *prev = NULL, *p;
	int m; 
	RBTREE_FOR(node, struct trust_anchor*, anchors->tree) {
		node->parent = NULL;
		if(!prev || prev->dclass != node->dclass) {
			prev = node;
			continue;
		}
		(void)dname_lab_cmp(prev->name, prev->namelabs, node->name, 
			node->namelabs, &m); /* we know prev is smaller */
		/* sort order like: . com. bla.com. zwb.com. net. */
		/* find the previous, or parent-parent-parent */
		for(p = prev; p; p = p->parent)
			/* looking for name with few labels, a parent */
			if(p->namelabs <= m) {
				/* ==: since prev matched m, this is closest*/
				/* <: prev matches more, but is not a parent,
			 	* this one is a (grand)parent */
				node->parent = p;
				break;
			}
		prev = node;
	}
}

int 
anchors_apply_cfg(struct val_anchors* anchors, struct config_file* cfg)
{
	if(cfg->trust_anchor_file && cfg->trust_anchor_file[0]) {
		/* read trust anchor file */
	}
	init_parents(anchors);
	return 1;
}

struct trust_anchor* 
anchors_lookup(struct val_anchors* anchors,
        uint8_t* qname, size_t qname_len, uint16_t qclass)
{
	return NULL;
}
