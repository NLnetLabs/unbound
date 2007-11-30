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
#include "util/locks.h"
#include "util/config_file.h"
#include "util/alloc.h"

/**
 * The context structure
 *
 * Contains two pipes for async service
 *	qq : write queries to the async service pid/tid.
 *	rr : read results from the async service pid/tid.
 */
struct ub_val_ctx {
	/** mutex on query write pipe */
	lock_basic_t qqpipe_lock;
	/** the query write pipe, [0] read from, [1] write on */
	int qqpipe[2];
	/** mutex on result read pipe */
	lock_basic_t rrpipe_lock;
	/** the result read pipe, [0] read from, [1] write on */
	int rrpipe[2];

	/** configuration options */
	struct config_file* cfg;
	/** do threading (instead of forking) */
	int dothread;

	/** shared caches, and so on */
	struct alloc_cache superalloc;
	/** module env master value */
	struct module_env* env;
	/** number of modules active, ids from 0 to num-1. */
	int num_modules;
	/** the module callbacks, array of num_modules length */
	struct module_func_block** modfunc;
	/** local authority zones */
	struct local_zones* local_zones;

	/** TODO list of outstanding queries */
};

/**
 * The error constants
 */
enum ub_ctx_err {
	/** no error */
	UB_NOERROR = 0,
	/** alloc failure */
	UB_NOMEM,
	/** socket operation */
	UB_SOCKET,
	/** syntax error */
	UB_SYNTAX,
	/** DNS service failed */
	UB_SERVFAIL
};


struct ub_val_ctx* 
ub_val_ctx_create()
{
	struct ub_val_ctx* ctx = (struct ub_val_ctx*)calloc(1, sizeof(*ctx));
	if(!ctx) {
		errno = ENOMEM;
		return NULL;
	}
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
	ctx->cfg = config_create();
	if(!ctx->cfg) {
		ub_val_ctx_delete(ctx);
		errno = ENOMEM;
		return NULL;
	}
	return ctx;
}

void 
ub_val_ctx_delete(struct ub_val_ctx* ctx)
{
	if(!ctx) return;
	lock_basic_destroy(&ctx->qqpipe_lock);
	lock_basic_destroy(&ctx->rrpipe_lock);
	close(ctx->qqpipe[0]);
	close(ctx->qqpipe[1]);
	close(ctx->rrpipe[0]);
	close(ctx->rrpipe[1]);
	config_delete(ctx->cfg);
	free(ctx);
}

int 
ub_val_ctx_config(struct ub_val_ctx* ctx, char* fname)
{
	if(!config_read(ctx->cfg, fname)) {
		return UB_SYNTAX;
	}
	return UB_NOERROR;
}

int 
ub_val_ctx_add_ta(struct ub_val_ctx* ctx, char* ta)
{
	char* dup = strdup(ta);
	if(!dup) return UB_NOMEM;
	if(!cfg_strlist_insert(&ctx->cfg->trust_anchor_list, dup)) {
		free(dup);
		return UB_NOMEM;
	}
	return UB_NOERROR;
}

int 
ub_val_ctx_trustedkeys(struct ub_val_ctx* ctx, char* fname)
{
	char* dup = strdup(fname);
	if(!dup) return UB_NOMEM;
	if(!cfg_strlist_insert(&ctx->cfg->trusted_keys_file_list, dup)) {
		free(dup);
		return UB_NOMEM;
	}
	return UB_NOERROR;
}

int 
ub_val_ctx_async(struct ub_val_ctx* ctx, int dothread)
{
	ctx->dothread = dothread;
	return UB_NOERROR;
}

static int 
pollit(struct ub_val_ctx* ctx, struct timeval* t)
{
	fd_set r;
	FD_ZERO(&r);
	FD_SET(ctx->rrpipe[0], &r);
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
	/* TODO until no more queries outstanding */
	while(1) {
		lock_basic_lock(&ctx->rrpipe_lock);
		(void)pollit(ctx, NULL);
		lock_basic_unlock(&ctx->rrpipe_lock);
		ub_val_ctx_process(ctx);
	}
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
	return UB_NOERROR;
}

int 
ub_val_resolve(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, int* secure, int* data, struct ub_val_result** result)
{
	/* become a resolver thread for a bit */

	return UB_NOERROR;
}

int 
ub_val_resolve_async(struct ub_val_ctx* ctx, char* name, int rrtype, 
	int rrclass, void* mydata, ub_val_callback_t callback, int* async_id)
{
	return UB_NOERROR;
}

int 
ub_val_cancel(struct ub_val_ctx* ctx, int async_id)
{
	return UB_NOERROR;
}

void 
ub_val_result_free(struct ub_val_result* result)
{
	char** p;
	if(!result) return;
	free(result->qname);
	free(result->canonname);
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
		default: return "unknown error";
	}
}
