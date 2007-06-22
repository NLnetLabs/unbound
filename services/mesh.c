/*
 * services/mesh.c - deal with mesh of query states and handle events for that.
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
 * This file contains functions to assist in dealing with a mesh of
 * query states. This mesh is supposed to be thread-specific.
 * It consists of query states (per qname, qtype, qclass) and connections
 * between query states and the super and subquery states, and replies to
 * send back to clients.
 */
#include "config.h"
#include "services/mesh.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/module.h"
#include "util/region-allocator.h"

/** compare two mesh_states */
static int
mesh_state_compare(const void* ap, const void* bp)
{
	struct mesh_state* a = (struct mesh_state*)ap;
	struct mesh_state* b = (struct mesh_state*)bp;

	if(a->is_priming && !b->is_priming)
		return -1;
	if(!a->is_priming && b->is_priming)
		return 1;

	if((a->s.query_flags&BIT_RD) && !(b->s.query_flags&BIT_RD))
		return -1;
	if(!(a->s.query_flags&BIT_RD) && (b->s.query_flags&BIT_RD))
		return 1;

	return query_info_compare(&a->s.qinfo, &b->s.qinfo);
}

/** compare two mesh references */
static int
mesh_state_ref_compare(const void* ap, const void* bp)
{
	struct mesh_state_ref* a = (struct mesh_state_ref*)ap;
	struct mesh_state_ref* b = (struct mesh_state_ref*)bp;
	return mesh_state_compare(a->s, b->s);
}

struct mesh_area* 
mesh_create(int num_modules, struct module_func_block** modfunc)
{
	struct mesh_area* mesh = calloc(1, sizeof(struct mesh_area));
	if(!mesh) {
		log_err("mesh area alloc: out of memory");
		return NULL;
	}
	mesh->num_modules = num_modules;
	mesh->modfunc = modfunc;
	rbtree_init(&mesh->run, &mesh_state_compare);
	rbtree_init(&mesh->all, &mesh_state_compare);
	mesh->num_reply_addrs = 0;
	mesh->num_reply_states = 0;
	mesh->num_detached_states = 0;
	return mesh;
}

void 
mesh_delete(struct mesh_area* mesh)
{
	struct mesh_state* mstate;
	if(!mesh)
		return;
	/* free all query states */
	RBTREE_FOR(mstate, struct mesh_state*, &mesh->all) {
		mesh_state_cleanup(mstate);
	}
	free(mesh);
}

struct mesh_state* 
mesh_state_create(struct module_env* env, struct query_info* qinfo, 
	uint16_t qflags, int prime)
{
	region_type* region = region_create(malloc, free);
	struct mesh_state* mstate;
	int i;
	if(!region)
		return NULL;
	mstate = (struct mesh_state*)region_alloc(region, 
		sizeof(struct mesh_state));
	if(!mstate) {
		region_destroy(region);
		return NULL;
	}
	mstate->node = *RBTREE_NULL;
	mstate->run_node = *RBTREE_NULL;
	mstate->node.key = mstate;
	mstate->run_node.key = mstate;
	mstate->is_priming = prime;
	mstate->reply_list = NULL;
	rbtree_init(&mstate->super_set, &mesh_state_ref_compare);
	rbtree_init(&mstate->sub_set, &mesh_state_ref_compare);
	/* init module qstate */
	mstate->s.qinfo.qtype = qinfo->qtype;
	mstate->s.qinfo.qclass = qinfo->qclass;
	mstate->s.qinfo.qname_len = qinfo->qname_len;
	mstate->s.qinfo.qname = region_alloc_init(region, qinfo->qname,
		qinfo->qname_len);
	if(!mstate->s.qinfo.qname) {
		region_destroy(region);
		return NULL;
	}
	/* remove all weird bits from qflags */
	mstate->s.query_flags = (qflags & BIT_RD);
	mstate->s.reply = NULL;
	mstate->s.region = region;
	mstate->s.curmod = 0;
	mstate->s.env = env;
	mstate->s.mesh_info = mstate;
	/* init modules */
	for(i=0; i<env->mesh->num_modules; i++) {
		mstate->s.minfo[i] = NULL;
		mstate->s.ext_state[i] = module_state_initial;
	}
	return mstate;
}

void 
mesh_state_cleanup(struct mesh_state* mstate)
{
	struct mesh_area* mesh;
	int i;
	if(!mstate)
		return;
	/* de-init modules */
	mesh = mstate->s.env->mesh;
	for(i=0; i<mesh->num_modules; i++) {
		(*mesh->modfunc[i]->clear)(&mstate->s, i);
		mstate->s.minfo[i] = NULL;
		mstate->s.ext_state[i] = module_finished;
	}
	region_destroy(mstate->s.region);
}
