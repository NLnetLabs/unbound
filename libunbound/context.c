/*
 * libunbound/context.c - validating context for unbound internal use
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
 * This file contains the validator context structure.
 */
#include "config.h"
#include "libunbound/context.h"
#include "util/module.h"
#include "util/config_file.h"
#include "services/modstack.h"
#include "services/localzone.h"
#include "services/cache/rrset.h"
#include "services/cache/infra.h"
#include "util/data/msgreply.h"
#include "util/storage/slabhash.h"

int 
context_finalize(struct ub_val_ctx* ctx)
{
	struct config_file* cfg = ctx->env->cfg;
	verbosity = cfg->verbosity;
	log_init(cfg->logfile, cfg->use_syslog, NULL);
	config_apply(cfg);
	if(!modstack_setup(&ctx->mods, cfg->module_conf, ctx->env))
		return UB_INITFAIL;
	ctx->local_zones = local_zones_create();
	if(!ctx->local_zones)
		return UB_NOMEM;
	if(!local_zones_apply_cfg(ctx->local_zones, cfg))
		return UB_INITFAIL;
	if(!ctx->env->msg_cache ||
	   cfg->msg_cache_size != slabhash_get_size(ctx->env->msg_cache) || 
	   cfg->msg_cache_slabs != ctx->env->msg_cache->size) {
		slabhash_delete(ctx->env->msg_cache);
		ctx->env->msg_cache = slabhash_create(cfg->msg_cache_slabs,
			HASH_DEFAULT_STARTARRAY, cfg->msg_cache_size,
			msgreply_sizefunc, query_info_compare,
			query_entry_delete, reply_info_delete, NULL);
		if(!ctx->env->msg_cache)
			return UB_NOMEM;
	}
	ctx->env->rrset_cache = rrset_cache_adjust(ctx->env->rrset_cache,
		ctx->env->cfg, ctx->env->alloc);
	if(!ctx->env->rrset_cache)
		return UB_NOMEM;
	ctx->env->infra_cache = infra_adjust(ctx->env->infra_cache, cfg);
	if(!ctx->env->infra_cache)
		return UB_NOMEM;
	ctx->finalized = 1;
	return UB_NOERROR;
}

int context_query_cmp(const void* a, const void* b)
{
	if( *(int*)a < *(int*)b )
		return -1;
	if( *(int*)a > *(int*)b )
		return 1;
	return 0;
}

/** How many times to try to find an unused query-id-number for async */
#define NUM_ID_TRIES 100000
/** find next useful id number of 0 on error */
static int
find_id(struct ub_val_ctx* ctx, int* id)
{
	size_t tries = 0;
	while(rbtree_search(&ctx->queries, &ctx->next_querynum)) {
		ctx->next_querynum++; /* numerical wraparound is fine */
		if(tries++ > NUM_ID_TRIES)
			return 0;
	}
	*id = ctx->next_querynum;
	ctx->next_querynum++;
	return 1;
}

struct ctx_query* 
context_new(struct ub_val_ctx* ctx, char* name, int rrtype, int rrclass, 
	ub_val_callback_t cb, void* cbarg)
{
	struct ctx_query* q = (struct ctx_query*)calloc(1, sizeof(*q));
	if(!q) return NULL;
	lock_basic_lock(&ctx->cfglock);
	if(!find_id(ctx, &q->querynum)) {
		lock_basic_unlock(&ctx->cfglock);
		free(q);
		return NULL;
	}
	lock_basic_unlock(&ctx->cfglock);
	q->node.key = &q->querynum;
	q->async = (cb != NULL);
	q->cb = cb;
	q->cb_arg = cbarg;
	q->res = (struct ub_val_result*)calloc(1, sizeof(*q->res));
	if(!q->res) {
		free(q);
		return NULL;
	}
	q->res->qname = strdup(name);
	if(!q->res->qname) {
		free(q->res);
		free(q);
		return NULL;
	}
	q->res->qtype = rrtype;
	q->res->qclass = rrclass;

	/* add to query list */
	lock_basic_lock(&ctx->cfglock);
	if(q->async)
		ctx->num_async ++;
	(void)rbtree_insert(&ctx->queries, &q->node);
	lock_basic_unlock(&ctx->cfglock);
	return q;
}

struct alloc_cache* 
context_obtain_alloc(struct ub_val_ctx* ctx)
{
	struct alloc_cache* a;
	int tnum = 0;
	lock_basic_lock(&ctx->cfglock);
	a = ctx->alloc_list;
	if(a)
		ctx->alloc_list = a->super; /* snip off list */
	else	tnum = ctx->thr_next_num++;
	lock_basic_unlock(&ctx->cfglock);
	if(a) {
		a->super = &ctx->superalloc;
		return a;
	}
	a = (struct alloc_cache*)calloc(1, sizeof(*a));
	if(!a)
		return NULL;
	alloc_init(a, &ctx->superalloc, tnum);
	return a;
}

void 
context_release_alloc(struct ub_val_ctx* ctx, struct alloc_cache* alloc)
{
	if(!ctx || !alloc)
		return;
	lock_basic_lock(&ctx->cfglock);
	alloc->super = ctx->alloc_list;
	ctx->alloc_list = alloc;
	lock_basic_unlock(&ctx->cfglock);
}
