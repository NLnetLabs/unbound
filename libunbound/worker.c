/*
 * libunbound/worker.c - worker thread or process that resolves
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
 * This file contains the worker process or thread that performs
 * the DNS resolving and validation. The worker is called by a procedure
 * and if in the background continues until exit, if in the foreground
 * returns from the procedure when done.
 */
#include "config.h"
#include "libunbound/worker.h"
#include "libunbound/context.h"
#include "libunbound/unbound.h"

/** delete libworker struct */
static void
libworker_delete(struct libworker* w)
{
	if(!w) return;
	lock_basic_lock(&w->ctx->cfglock);
	/* return alloc */
	lock_basic_unlock(&w->ctx->cfglock);

}

/** setup fresh libworker struct */
static struct libworker*
libworker_setup(struct ub_val_ctx* ctx)
{
	struct libworker* w = (struct libworker*)calloc(1, sizeof(*w));
	if(!w) return NULL;
	w->ctx = ctx;
	lock_basic_lock(&ctx->cfglock);
	/* obtain a new alloc */
	lock_basic_unlock(&ctx->cfglock);
	/* setup event base */
	/* setup outnet */
	/* setup rnd */
	/* setup env */
	/* setup env.scratch */
	/* setup env.scratch_buffer */
	/* setup env.worker */
	/* setup env.mesh */
	/* setup env.alloc */
	/* setup env.rnd */
	/* setup env - callback ptrs */
	return w;
}

/** the background thread func */
static void*
libworker_dobg(void* arg)
{
	struct ub_val_ctx* ctx = (struct ub_val_ctx*)arg;
	struct libworker* w = libworker_setup(ctx);
	log_thread_set(&w->thread_num);
	/* TODO do bg thread */
	libworker_delete(w);
	return NULL;
}

int libworker_bg(struct ub_val_ctx* ctx)
{
	/* fork or threadcreate */
	lock_basic_lock(&ctx->cfglock);
	if(ctx->dothread) {
		ub_thread_t t;
		lock_basic_unlock(&ctx->cfglock);
		ub_thread_create(&t, libworker_dobg, ctx);
		/* ctx.tid = t or so */
	} else {
		pid_t p;
		lock_basic_unlock(&ctx->cfglock);
		switch((p=fork())) {
			case 0:
				(void)libworker_dobg(ctx);
				exit(0);
				break;
			case -1:
				/* TODO make UB_FORKFAILED */
				return UB_SOCKET;
			default:
				/* ctx.pid = p or so */
				break;
		}
	}
	return UB_NOERROR;
}

int libworker_fg(struct ub_val_ctx* ctx, struct ctx_query* q)
{
	struct libworker* w = libworker_setup(ctx);
	if(!w)
		return UB_NOMEM;
	/* TODO */
	q->res->bogus = 1;
	libworker_delete(w);
	return UB_NOERROR;
}
