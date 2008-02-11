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
#include "libunbound/libworker.h"
#include "util/locks.h"
#include "util/config_file.h"
#include "util/alloc.h"
#include "util/module.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/random.h"
#include "util/net_help.h"
#include "services/modstack.h"
#include "services/localzone.h"
#include "services/cache/infra.h"
#include "services/cache/rrset.h"

struct ub_ctx* 
ub_ctx_create()
{
	struct ub_ctx* ctx;
	unsigned int seed;
	log_init(NULL, 0, NULL); /* logs to stderr */
	log_ident_set("libunbound");
	verbosity = 0; /* errors only */
	checklock_start();
	ctx = (struct ub_ctx*)calloc(1, sizeof(*ctx));
	if(!ctx) {
		errno = ENOMEM;
		return NULL;
	}
	alloc_init(&ctx->superalloc, NULL, 0);
	seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
	if(!(ctx->seed_rnd = ub_initstate(seed, NULL))) {
		seed = 0;
		ub_randfree(ctx->seed_rnd);
		free(ctx);
		errno = ENOMEM;
		return NULL;
	}
	seed = 0;
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, ctx->qqpipe) == -1) {
		ub_randfree(ctx->seed_rnd);
		free(ctx);
		return NULL;
	}
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, ctx->rrpipe) == -1) {
		int e = errno;
		close(ctx->qqpipe[0]);
		close(ctx->qqpipe[1]);
		ub_randfree(ctx->seed_rnd);
		free(ctx);
		errno = e;
		return NULL;
	}
	if(!fd_set_nonblock(ctx->rrpipe[0]) ||
	   !fd_set_nonblock(ctx->rrpipe[1]) ||
	   !fd_set_nonblock(ctx->qqpipe[0]) ||
	   !fd_set_nonblock(ctx->qqpipe[1])) {
		int e = errno;
		close(ctx->rrpipe[0]);
		close(ctx->rrpipe[1]);
		close(ctx->qqpipe[0]);
		close(ctx->qqpipe[1]);
		ub_randfree(ctx->seed_rnd);
		free(ctx);
		errno = e;
		return NULL;
	}
	lock_basic_init(&ctx->qqpipe_lock);
	lock_basic_init(&ctx->rrpipe_lock);
	lock_basic_init(&ctx->cfglock);
	ctx->env = (struct module_env*)calloc(1, sizeof(*ctx->env));
	if(!ctx->env) {
		close(ctx->rrpipe[0]);
		close(ctx->rrpipe[1]);
		close(ctx->qqpipe[0]);
		close(ctx->qqpipe[1]);
		ub_randfree(ctx->seed_rnd);
		free(ctx);
		errno = ENOMEM;
		return NULL;
	}
	ctx->env->cfg = config_create_forlib();
	if(!ctx->env->cfg) {
		close(ctx->rrpipe[0]);
		close(ctx->rrpipe[1]);
		close(ctx->qqpipe[0]);
		close(ctx->qqpipe[1]);
		free(ctx->env);
		ub_randfree(ctx->seed_rnd);
		free(ctx);
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
	context_query_delete(q);
}

void 
ub_ctx_delete(struct ub_ctx* ctx)
{
	struct alloc_cache* a, *na;
	if(!ctx) return;
	/* stop the bg thread */
	lock_basic_lock(&ctx->cfglock);
	if(ctx->created_bg) {
		uint8_t* msg;
		uint32_t len;
		uint32_t cmd = UB_LIBCMD_QUIT;
		lock_basic_unlock(&ctx->cfglock);
		lock_basic_lock(&ctx->qqpipe_lock);
		(void)libworker_write_msg(ctx->qqpipe[1], (uint8_t*)&cmd, 
			(uint32_t)sizeof(cmd), 0);
		lock_basic_unlock(&ctx->qqpipe_lock);
		lock_basic_lock(&ctx->rrpipe_lock);
		while(libworker_read_msg(ctx->rrpipe[0], &msg, &len, 0)) {
			/* discard all results except a quit confirm */
			if(context_serial_getcmd(msg, len) == UB_LIBCMD_QUIT) {
				free(msg);
				break;
			}
			free(msg);
		}
		lock_basic_unlock(&ctx->rrpipe_lock);

		/* if bg worker is a thread, wait for it to exit, so that all
	 	 * resources are really gone. */
		lock_basic_lock(&ctx->cfglock);
		if(ctx->dothread) {
			lock_basic_unlock(&ctx->cfglock);
			ub_thread_join(ctx->bg_tid);
		} else {
			lock_basic_unlock(&ctx->cfglock);
		}
	}
	else {
		lock_basic_unlock(&ctx->cfglock);
	}


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
	if(ctx->qqpipe[0] != -1)
		close(ctx->qqpipe[0]);
	if(ctx->qqpipe[1] != -1)
		close(ctx->qqpipe[1]);
	if(ctx->rrpipe[0] != -1)
		close(ctx->rrpipe[0]);
	if(ctx->rrpipe[1] != -1)
		close(ctx->rrpipe[1]);
	ctx->qqpipe[0] = -1;
	ctx->qqpipe[1] = -1;
	ctx->rrpipe[0] = -1;
	ctx->rrpipe[1] = -1;
	if(ctx->env) {
		slabhash_delete(ctx->env->msg_cache);
		rrset_cache_delete(ctx->env->rrset_cache);
		infra_delete(ctx->env->infra_cache);
		config_delete(ctx->env->cfg);
		free(ctx->env);
	}
	ub_randfree(ctx->seed_rnd);
	alloc_clear(&ctx->superalloc);
	traverse_postorder(&ctx->queries, delq, NULL);
	free(ctx);
}

int 
ub_ctx_set_option(struct ub_ctx* ctx, char* opt, char* val)
{
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_AFTERFINAL;
	}
	if(!config_set_option(ctx->env->cfg, opt, val)) {
		lock_basic_unlock(&ctx->cfglock);
		return UB_SYNTAX;
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_ctx_config(struct ub_ctx* ctx, char* fname)
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
ub_ctx_add_ta(struct ub_ctx* ctx, char* ta)
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
ub_ctx_add_ta_file(struct ub_ctx* ctx, char* fname)
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
ub_ctx_trustedkeys(struct ub_ctx* ctx, char* fname)
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
ub_ctx_debuglevel(struct ub_ctx* ctx, int d)
{
	lock_basic_lock(&ctx->cfglock);
	verbosity = d;
	ctx->env->cfg->verbosity = d;
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int ub_ctx_debugout(struct ub_ctx* ctx, void* out)
{
	lock_basic_lock(&ctx->cfglock);
	log_file((FILE*)out);
	ctx->logfile_override = 1;
	ctx->log_out = out;
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_ctx_async(struct ub_ctx* ctx, int dothread)
{
#if !defined(HAVE_PTHREAD) && !defined(HAVE_SOLARIS_THREADS)
	if(dothread) /* cannot do threading */
		return UB_NOERROR;
#endif
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
pollit(struct ub_ctx* ctx, struct timeval* t)
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
ub_poll(struct ub_ctx* ctx)
{
	struct timeval t;
	memset(&t, 0, sizeof(t));
	/* no need to hold lock while testing for readability. */
	return pollit(ctx, &t);
}

int 
ub_fd(struct ub_ctx* ctx)
{
	return ctx->rrpipe[0];
}

/** process answer from bg worker */
static int
process_answer_detail(struct ub_ctx* ctx, uint8_t* msg, uint32_t len,
	ub_callback_t* cb, void** cbarg, int* err,
	struct ub_result** res)
{
	struct ctx_query* q;
	if(context_serial_getcmd(msg, len) != UB_LIBCMD_ANSWER) {
		log_err("error: bad data from bg worker %d",
			(int)context_serial_getcmd(msg, len));
		return 0;
	}

	lock_basic_lock(&ctx->cfglock);
	q = context_deserialize_answer(ctx, msg, len, err);
	if(!q) {
		lock_basic_unlock(&ctx->cfglock);
		/* probably simply the lookup that failed, i.e.
		 * response returned before cancel was sent out, so noerror */
		return 1;
	}
	log_assert(q->async);

	/* grab cb while locked */
	if(q->cancelled) {
		*cb = NULL;
		*cbarg = NULL;
	} else {
		*cb = q->cb;
		*cbarg = q->cb_arg;
	}
	if(*err) {
		*res = NULL;
		ub_resolve_free(q->res);
	} else {
		/* parse the message, extract rcode, fill result */
		ldns_buffer* buf = ldns_buffer_new(q->msg_len);
		struct regional* region = regional_create();
		*res = q->res;
		(*res)->rcode = LDNS_RCODE_SERVFAIL;
		if(region && buf) {
			ldns_buffer_clear(buf);
			ldns_buffer_write(buf, q->msg, q->msg_len);
			ldns_buffer_flip(buf);
			libworker_enter_result(*res, buf, region,
				q->msg_security);
		}
		ldns_buffer_free(buf);
		regional_destroy(region);
	}
	q->res = NULL;
	/* delete the q from list */
	(void)rbtree_delete(&ctx->queries, q->node.key);
	ctx->num_async--;
	context_query_delete(q);
	lock_basic_unlock(&ctx->cfglock);

	if(*cb) return 2;
	return 1;
}

/** process answer from bg worker */
static int
process_answer(struct ub_ctx* ctx, uint8_t* msg, uint32_t len)
{
	int err;
	ub_callback_t cb;
	void* cbarg;
	struct ub_result* res;
	int r;

	r = process_answer_detail(ctx, msg, len, &cb, &cbarg, &err, &res);

	/* no locks held while calling callback, so that library is
	 * re-entrant. */
	if(r == 2)
		(*cb)(cbarg, err, res);

	return r;
}

int 
ub_process(struct ub_ctx* ctx)
{
	int r;
	uint8_t* msg;
	uint32_t len;
	while(1) {
		msg = NULL;
		lock_basic_lock(&ctx->rrpipe_lock);
		r = libworker_read_msg(ctx->rrpipe[0], &msg, &len, 1);
		lock_basic_unlock(&ctx->rrpipe_lock);
		if(r == 0)
			return UB_PIPE;
		else if(r == -1)
			return UB_NOERROR;
		if(!process_answer(ctx, msg, len)) {
			free(msg);
			return UB_PIPE;
		}
		free(msg);
	}
	return UB_NOERROR;
}

int 
ub_wait(struct ub_ctx* ctx)
{
	int err;
	ub_callback_t cb;
	void* cbarg;
	struct ub_result* res;
	int r;
	uint8_t* msg;
	uint32_t len;
	/* this is basically the same loop as _process(), but with changes.
	 * holds the rrpipe lock and waits with pollit */
	while(1) {
		lock_basic_lock(&ctx->rrpipe_lock);
		lock_basic_lock(&ctx->cfglock);
		if(ctx->num_async == 0) {
			lock_basic_unlock(&ctx->cfglock);
			lock_basic_unlock(&ctx->rrpipe_lock);
			break;
		}
		lock_basic_unlock(&ctx->cfglock);

		/* keep rrpipe locked, while
		 * 	o waiting for pipe readable
		 * 	o parsing message
		 * 	o possibly decrementing num_async
		 * do callback without lock
		 */
		r = pollit(ctx, NULL);
		if(r) {
			r = libworker_read_msg(ctx->rrpipe[0], &msg, &len, 1);
			if(r == 0) {
				lock_basic_unlock(&ctx->rrpipe_lock);
				return UB_PIPE;
			}
			if(r == -1) {
				lock_basic_unlock(&ctx->rrpipe_lock);
				continue;
			}
			r = process_answer_detail(ctx, msg, len, 
				&cb, &cbarg, &err, &res);
			lock_basic_unlock(&ctx->rrpipe_lock);
			if(r == 0)
				return UB_PIPE;
			if(r == 2)
				(*cb)(cbarg, err, res);
		} else {
			lock_basic_unlock(&ctx->rrpipe_lock);
		}
	}
	return UB_NOERROR;
}

int 
ub_resolve(struct ub_ctx* ctx, char* name, int rrtype, 
	int rrclass, struct ub_result** result)
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
	*result = NULL;

	r = libworker_fg(ctx, q);
	if(r) {
		lock_basic_lock(&ctx->cfglock);
		(void)rbtree_delete(&ctx->queries, q->node.key);
		context_query_delete(q);
		lock_basic_unlock(&ctx->cfglock);
		return r;
	}
	*result = q->res;
	q->res = NULL;

	lock_basic_lock(&ctx->cfglock);
	(void)rbtree_delete(&ctx->queries, q->node.key);
	context_query_delete(q);
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_resolve_async(struct ub_ctx* ctx, char* name, int rrtype, 
	int rrclass, void* mydata, ub_callback_t callback, int* async_id)
{
	struct ctx_query* q;
	uint8_t* msg = NULL;
	uint32_t len = 0;

	if(async_id)
		*async_id = 0;
	lock_basic_lock(&ctx->cfglock);
	if(!ctx->finalized) {
		int r = context_finalize(ctx);
		if(r) {
			lock_basic_unlock(&ctx->cfglock);
			return r;
		}
	}
	if(!ctx->created_bg) {
		int r;
		ctx->created_bg = 1;
		lock_basic_unlock(&ctx->cfglock);
		r = libworker_bg(ctx);
		if(r) {
			lock_basic_lock(&ctx->cfglock);
			ctx->created_bg = 0;
			lock_basic_unlock(&ctx->cfglock);
			return r;
		}
	} else {
		lock_basic_unlock(&ctx->cfglock);
	}

	/* create new ctx_query and attempt to add to the list */
	q = context_new(ctx, name, rrtype, rrclass, callback, mydata);
	if(!q)
		return UB_NOMEM;

	/* write over pipe to background worker */
	lock_basic_lock(&ctx->cfglock);
	msg = context_serialize_new_query(q, &len);
	if(!msg) {
		(void)rbtree_delete(&ctx->queries, q->node.key);
		ctx->num_async--;
		context_query_delete(q);
		lock_basic_unlock(&ctx->cfglock);
		return UB_NOMEM;
	}
	if(async_id)
		*async_id = q->querynum;
	lock_basic_unlock(&ctx->cfglock);
	
	lock_basic_lock(&ctx->qqpipe_lock);
	if(!libworker_write_msg(ctx->qqpipe[1], msg, len, 0)) {
		lock_basic_unlock(&ctx->qqpipe_lock);
		free(msg);
		return UB_PIPE;
	}
	lock_basic_unlock(&ctx->qqpipe_lock);
	free(msg);
	return UB_NOERROR;
}

int 
ub_cancel(struct ub_ctx* ctx, int async_id)
{
	struct ctx_query* q;
	uint8_t* msg = NULL;
	uint32_t len = 0;
	lock_basic_lock(&ctx->cfglock);
	q = (struct ctx_query*)rbtree_search(&ctx->queries, &async_id);
	if(!q || !q->async) {
		/* it is not there, so nothing to do */
		lock_basic_unlock(&ctx->cfglock);
		return UB_NOERROR;
	}
	log_assert(q->async);
	q->cancelled = 1;
	
	/* delete it */
	if(!ctx->dothread) { /* if forked */
		(void)rbtree_delete(&ctx->queries, q->node.key);
		ctx->num_async--;
		context_query_delete(q);
		msg = context_serialize_cancel(q, &len);
		lock_basic_unlock(&ctx->cfglock);
		if(!msg) {
			return UB_NOMEM;
		}
		/* send cancel to background worker */
		lock_basic_lock(&ctx->qqpipe_lock);
		if(!libworker_write_msg(ctx->qqpipe[1], msg, len, 0)) {
			lock_basic_unlock(&ctx->qqpipe_lock);
			free(msg);
			return UB_PIPE;
		}
		lock_basic_unlock(&ctx->qqpipe_lock);
		free(msg);
	} else {
		lock_basic_unlock(&ctx->cfglock);
	}
	return UB_NOERROR;
}

void 
ub_resolve_free(struct ub_result* result)
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
ub_strerror(int err)
{
	switch(err) {
		case UB_NOERROR: return "no error";
		case UB_SOCKET: return "socket io error";
		case UB_NOMEM: return "out of memory";
		case UB_SYNTAX: return "syntax error";
		case UB_SERVFAIL: return "server failure";
		case UB_FORKFAIL: return "could not fork";
		case UB_INITFAIL: return "initialization failure";
		case UB_AFTERFINAL: return "setting change after finalize";
		case UB_PIPE: return "error in pipe communication with async";
		case UB_READFILE: return "error reading file";
		default: return "unknown error";
	}
}

int 
ub_ctx_set_fwd(struct ub_ctx* ctx, char* addr)
{
	struct sockaddr_storage storage;
	socklen_t stlen;
	struct config_stub* s;
	char* dupl;
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		errno=EINVAL;
		return UB_AFTERFINAL;
	}
	if(!addr) {
		/* disable fwd mode - the root stub should be first. */
		if(ctx->env->cfg->forwards &&
			strcmp(ctx->env->cfg->forwards->name, ".") == 0) {
			s = ctx->env->cfg->forwards;
			ctx->env->cfg->forwards = s->next;
			s->next = NULL;
			config_delstubs(s);
		}
		lock_basic_unlock(&ctx->cfglock);
		return UB_NOERROR;
	}
	lock_basic_unlock(&ctx->cfglock);

	/* check syntax for addr */
	if(!extstrtoaddr(addr, &storage, &stlen)) {
		errno=EINVAL;
		return UB_SYNTAX;
	}
	
	/* it parses, add root stub in front of list */
	lock_basic_lock(&ctx->cfglock);
	if(!ctx->env->cfg->forwards ||
		strcmp(ctx->env->cfg->forwards->name, ".") != 0) {
		s = calloc(1, sizeof(*s));
		if(!s) {
			lock_basic_unlock(&ctx->cfglock);
			errno=ENOMEM;
			return UB_NOMEM;
		}
		s->name = strdup(".");
		if(!s->name) {
			free(s);
			lock_basic_unlock(&ctx->cfglock);
			errno=ENOMEM;
			return UB_NOMEM;
		}
		s->next = ctx->env->cfg->forwards;
		ctx->env->cfg->forwards = s;
	} else {
		log_assert(ctx->env->cfg->forwards);
		s = ctx->env->cfg->forwards;
	}
	dupl = strdup(addr);
	if(!dupl) {
		lock_basic_unlock(&ctx->cfglock);
		errno=ENOMEM;
		return UB_NOMEM;
	}
	if(!cfg_strlist_insert(&s->addrs, dupl)) {
		free(dupl);
		lock_basic_unlock(&ctx->cfglock);
		errno=ENOMEM;
		return UB_NOMEM;
	}
	lock_basic_unlock(&ctx->cfglock);
	return UB_NOERROR;
}

int 
ub_ctx_resolvconf(struct ub_ctx* ctx, char* fname)
{
	FILE* in;
	int numserv = 0;
	char buf[1024];
	char* parse, *addr;
	int r;
	if(fname == NULL)
		fname = "/etc/resolv.conf";
	in = fopen(fname, "r");
	if(!in) {
		/* error in errno! perror(fname) */
		return UB_READFILE;
	}
	while(fgets(buf, (int)sizeof(buf), in)) {
		buf[sizeof(buf)-1] = 0;
		parse=buf;
		while(*parse == ' ' || *parse == '\t')
			parse++;
		if(strncmp(parse, "nameserver", 10) == 0) {
			numserv++;
			parse += 10; /* skip 'nameserver' */
			/* skip whitespace */
			while(*parse == ' ' || *parse == '\t')
				parse++;
			addr = parse;
			/* skip [0-9a-fA-F.:]*, i.e. IP4 and IP6 address */
			while(isxdigit(*parse) || *parse=='.' || *parse==':')
				parse++;
			/* terminate after the address, remove newline */
			*parse = 0;
			
			if((r = ub_ctx_set_fwd(ctx, addr)) != UB_NOERROR) {
				fclose(in);
				return r;
			}
		}
	}
	fclose(in);
	if(numserv == 0) {
		/* from resolv.conf(5) if none given, use localhost */
		return ub_ctx_set_fwd(ctx, "127.0.0.1");
	}
	return UB_NOERROR;
}

int
ub_ctx_hosts(struct ub_ctx* ctx, char* fname)
{
	FILE* in;
	char buf[1024], ldata[1024];
	char* parse, *addr, *name, *ins;
	lock_basic_lock(&ctx->cfglock);
	if(ctx->finalized) {
		lock_basic_unlock(&ctx->cfglock);
		errno=EINVAL;
		return UB_AFTERFINAL;
	}
	lock_basic_unlock(&ctx->cfglock);
	if(fname == NULL)
		fname = "/etc/hosts";
	in = fopen(fname, "r");
	if(!in) {
		/* error in errno! perror(fname) */
		return UB_READFILE;
	}
	while(fgets(buf, (int)sizeof(buf), in)) {
		buf[sizeof(buf)-1] = 0;
		parse=buf;
		while(*parse == ' ' || *parse == '\t')
			parse++;
		if(*parse == '#')
			continue; /* skip comment */
		/* format: <addr> spaces <name> spaces <name> ... */
		addr = parse;
		/* skip addr */
		while(isxdigit(*parse) || *parse == '.' || *parse == ':')
			parse++;
		if(*parse != ' ' && *parse != '\t') {
			/* must have whitespace after address */
			fclose(in);
			errno=EINVAL;
			return UB_SYNTAX;
		}
		*parse++ = 0; /* end delimiter for addr ... */
		/* go to names and add them */
		while(*parse) {
			while(*parse == ' ' || *parse == '\t' || *parse=='\n')
				parse++;
			if(*parse == 0 || *parse == '#')
				break;
			/* skip name, allows (too) many printable characters */
			name = parse;
			while('!' <= *parse && *parse <= '~')
				parse++;
			if(*parse)
				*parse++ = 0; /* end delimiter for name */
			snprintf(ldata, sizeof(ldata), "%s %s %s",
				name, str_is_ip6(addr)?"AAAA":"A", addr);
			ins = strdup(ldata);
			if(!ins) {
				/* out of memory */
				fclose(in);
				errno=ENOMEM;
				return UB_NOMEM;
			}
			lock_basic_lock(&ctx->cfglock);
			if(!cfg_strlist_insert(&ctx->env->cfg->local_data, 
				ins)) {
				lock_basic_unlock(&ctx->cfglock);
				fclose(in);
				free(ins);
				errno=ENOMEM;
				return UB_NOMEM;
			}
			lock_basic_unlock(&ctx->cfglock);
		}
	}
	fclose(in);
	return UB_NOERROR;
}
