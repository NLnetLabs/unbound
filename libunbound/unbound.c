/*
 * unbound.c - unbound validating resolver public API implementation
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
 * This file contains functions to resolve DNS queries and 
 * validate the answers. Synchonously and asynchronously.
 *
 */

/* include the public api first, it should be able to stand alone */
#include "libunbound/unbound.h"
#include "config.h"
#include "libunbound/context.h"
#include "libunbound/worker.h"
#include "util/locks.h"
#include "util/config_file.h"
#include "util/alloc.h"
#include "util/module.h"
#include "util/log.h"
#include "services/modstack.h"
#include "services/localzone.h"
#include "services/cache/infra.h"
#include "services/cache/rrset.h"

struct ub_val_ctx* 
ub_val_ctx_create()
{
	struct ub_val_ctx* ctx = (struct ub_val_ctx*)calloc(1, sizeof(*ctx));
	if(!ctx) {
		errno = ENOMEM;
		return NULL;
	}
	checklock_start();
	log_ident_set("libunbound");
	verbosity = 0; /* errors only */
	log_init(NULL, 0, NULL); /* logs to stderr */
	alloc_init(&ctx->superalloc, NULL, 0);
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, ctx->qqpipe) == -1) {
		free(ctx);
		return NULL;
	}
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, ctx->rrpipe) == -1) {
		int e = errno;
		close(ctx->qqpipe[0]);
		close(ctx->qqpipe[1]);
		free(ctx);
		errno = e;
		return NULL;
	}
	lock_basic_init(&ctx->qqpipe_lock);
	lock_basic_init(&ctx->rrpipe_lock);
	lock_basic_init(&ctx->cfglock);
	ctx->env = (struct module_env*)calloc(1, sizeof(*ctx->env));
	if(!ctx->env) {
		ub_val_ctx_delete(ctx);
		errno = ENOMEM;
		return NULL;
	}
	ctx->env->cfg = config_create_forlib();
	if(!ctx->env->cfg) {
		ub_val_ctx_delete(ctx);
		errno = ENOMEM;
		return NULL;
	}
	ctx->env->alloc = &ctx->superalloc;
	ctx->env->worker = NULL;
	ctx->env->need_to_validate = 0;
	modstack_init(&ctx->mods);
	rbtree_init(&ctx->queries, &context_query_cmp);
	return ctx;
}

/** delete q */
static void
delq(rbnode_t* n, void* ATTR_UNUSED(arg))
{
	struct ctx_query* q = (struct ctx_query*)n;
	if(!q) return;
	ub_val_result_free(q->res);
	free(q);
}

void 
ub_val_ctx_delete(struct ub_val_ctx* ctx)
{
	struct alloc_cache* a, *na;
	if(!ctx) return;
	modstack_desetup(&ctx->mods, ctx->env);
	a = ctx->alloc_list;
	while(a) {
		na = a->super;
		a->super = &ctx->superalloc;
		alloc_clear(a);
		free(a);
		a = na;
	}
	local_zones_delete(ctx->local_zones);
	lock_basic_destroy(&ctx->qqpipe_lock);
	lock_basic_destroy(&ctx->rrpipe_lock);
	lock_basic_destroy(&ctx->cfglock);
	close(ctx->qqpipe[0]);
	close(ctx->qqpipe[1]);
	close(ctx->rrpipe[0]);
	close(ctx->rrpipe[1]);
	if(ctx->env) {
		slabhash_delete(ctx->env->msg_cache);
		rrset_cache_delete(ctx->env->rrset_cache);
		infra_delete(ctx->env->infra_cache);
		config_delete(ctx->env->cfg);
		free(ctx->env);
	}
	alloc_clear(&ctx->superalloc);
	traverse_postorder(&ctx->queries, delq, NULL);
	free(ctx);
}

int 
ub_val_ctx_config(struct ub_val_ctx* ctx, char* fname)
{
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_AFTERFINAL;
	}
	if(!config_read(ctx->env->cfg, fname)) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_SYNTAX;
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_val_ctx_add_ta(struct ub_val_ctx* ctx, char* ta)
{
	char* dup = strdup(ta);
	if(!dup) return UB_NOMEM;
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_AFTERFINAL;
	}
	if(!cfg_strlist_insert(&ctx->env->cfg->trust_anchor_list, dup)) {
		lock_basic_unlock(&ctx->cfglock);
		free(dup);
		return UB_NOMEM;
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_val_ctx_add_ta_file(struct ub_val_ctx* ctx, char* fname)
{
	char* dup = strdup(fname);
	if(!dup) return UB_NOMEM;
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_AFTERFINAL;
	}
	if(!cfg_strlist_insert(&ctx->env->cfg->trust_anchor_file_list, dup)) {
		lock_basic_unlock(&ctx->cfglock);
		free(dup);
		return UB_NOMEM;
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_val_ctx_trustedkeys(struct ub_val_ctx* ctx, char* fname)
{
	char* dup = strdup(fname);
	if(!dup) return UB_NOMEM;
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_AFTERFINAL;
	}
	if(!cfg_strlist_insert(&ctx->env->cfg->trusted_keys_file_list, dup)) {
		lock_basic_unlock(&ctx->cfglock);
		free(dup);
		return UB_NOMEM;
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_val_ctx_async(struct ub_val_ctx* ctx, int dothread)
{
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_AFTERFINAL;
	}
	ctx->dothread = dothread;
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

/** perform a select() on the result read pipe */
static int 
pollit(struct ub_val_ctx* ctx, struct timeval* t)
{
	fd_set r;
#ifndef S_SPLINT_S
	FD_ZERO(&r);
	FD_SET(ctx->rrpipe[0], &r);
#endif
	if(select(ctx->rrpipe[0]+1, &r, NULL, NULL, t) == -1) {
		return 0;
	}
	errno = 0;
	return FD_ISSET(ctx->rrpipe[0], &r);
}

int 
ub_val_ctx_poll(struct ub_val_ctx* ctx)
{
	struct timeval t;
	int r;
	memset(&t, 0, sizeof(t));
	lock_basic_lock(&ctx->rrpipe_lock);
	r = pollit(ctx, &t);
	lock_basic_unlock(&ctx->rrpipe_lock);
	return r;
}

int 
ub_val_ctx_wait(struct ub_val_ctx* ctx)
{
	lock_basic_lock(&ctx->cfglock);
	while(ctx->num_async > 0) {
		lock_basic_lock(&ctx->rrpipe_lock);
		lock_basic_unlock(&ctx->cfglock);
		(void)pollit(ctx, NULL);
		lock_basic_unlock(&ctx->rrpipe_lock);
		ub_val_ctx_process(ctx);
		lock_basic_lock(&ctx->cfglock);
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_val_ctx_fd(struct ub_val_ctx* ctx)
{
	return ctx->rrpipe[0];
}

int 
ub_val_ctx_process(struct ub_val_ctx* ctx)
{
	/* TODO */
	/* ctx->num_asynx-- when handled; */
	return UB_NOMEM;
}

int 
ub_val_resolve(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, int* secure, int* data, struct ub_val_result** result)
{
	struct ctx_query* q;
	int r;

	lock_basic_lock(&ctx->cfglock);
	if(!ctx->finalized) {
		r = context_finalize(ctx);
		if(r) {
			lock_basic_unlock(&ctx->cfglock);
			return r;
		}
	}
	/* create new ctx_query and attempt to add to the list */
	lock_basic_unlock(&ctx->cfglock);
	q = context_new(ctx, name, rrtype, rrclass, NULL, NULL);
	if(!q)
		return UB_NOMEM;
	/* become a resolver thread for a bit */
	*secure = 0;
	*data = 0;
	*result = NULL;

	r = libworker_fg(ctx, q);
	if(r) {
		ub_val_result_free(q->res);
		free(q);
		return r;
	}

	if(q->msg_security == sec_status_secure)
		*secure = 1;
	if(q->res->data && q->res->data[0])
		*data = 1;
	*result = q->res;

	(void)rbtree_delete(&ctx->queries, q->node.key);
	free(q->msg);
	free(q);
	return UB_NOERROR;
}

int 
ub_val_resolve_async(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, void* mydata, ub_val_callback_t callback, int* async_id)
{
	struct ctx_query* q;

	lock_basic_lock(&ctx->cfglock);
	if(!ctx->finalized) {
		int r = context_finalize(ctx);
		if(r) {
			lock_basic_unlock(&ctx->cfglock);
			return r;
		}
	}
	if(!ctx->created_bg) {
		int r = libworker_bg(ctx);
		if(r) {
			lock_basic_unlock(&ctx->cfglock);
			return r;
		}
		ctx->created_bg = 1;
	}
	lock_basic_unlock(&ctx->cfglock);

	/* create new ctx_query and attempt to add to the list */
	q = context_new(ctx, name, rrtype, rrclass, callback, mydata);
	if(!q)
		return UB_NOMEM;
	/* TODO write over pipe to background worker */
	*async_id = q->querynum;
	return UB_NOMEM;
}

int 
ub_val_cancel(struct ub_val_ctx* ctx, int async_id)
{
	struct ctx_query* q;
	lock_basic_lock(&ctx->cfglock);
	q = (struct ctx_query*)rbtree_search(&ctx->queries, &async_id);
	lock_basic_unlock(&ctx->cfglock);
	if(!q || !q->async) /* it is not there, so nothing to do */
		return UB_NOERROR;
	/* TODO ; send cancel to background worker */

	lock_basic_lock(&ctx->cfglock);
	(void)rbtree_delete(&ctx->queries, &async_id);
	ctx->num_async--;
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOMEM;
}

void 
ub_val_result_free(struct ub_val_result* result)
{
	char** p;
	if(!result) return;
	free(result->qname);
	if(result->canonname != result->qname)
		free(result->canonname);
	if(result->data)
		for(p = result->data; *p; p++)
			free(*p);
	free(result->data);
	free(result->len);
	free(result);
}

const char* 
ub_val_strerror(int err)
{
	switch(err) {
		case UB_NOERROR: return "no error";
		case UB_NOMEM: return "out of memory";
		case UB_SOCKET: return "socket io error";
		case UB_SYNTAX: return "syntax error";
		case UB_SERVFAIL: return "server failure";
		case UB_FORKFAIL: return "could not fork";
		case UB_INITFAIL: return "initialization failure";
		case UB_AFTERFINAL: return "setting change after finalize";
		default: return "unknown error";
	}
}
