/*
 * cachedb/cachedb.h - cache from a database external to the program module
 *
 * Copyright (c) 2016, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains a module that uses an external database to cache
 * dns responses.
 */
#include "util/module.h"

/**
 * The global variable environment contents for the cachedb
 * Shared between threads, this represents long term information.
 * Like database connections.
 */
struct cachedb_env {
	int todo;
};

/**
 * Per query state for the cachedb module.
 */
struct cachedb_qstate {
	int todo;
};

/** Init the cachedb module */
int cachedb_init(struct module_env* env, int id);
/** Deinit the cachedb module */
void cachedb_deinit(struct module_env* env, int id);
/** Operate on an event on a query (in qstate). */
void cachedb_operate(struct module_qstate* qstate, enum module_ev event,
	int id, struct outbound_entry* outbound);
/** Subordinate query done, inform this super request of its conclusion */
void cachedb_inform_super(struct module_qstate* qstate, int id,
	struct module_qstate* super);
/** clear the cachedb query-specific contents out of qstate */
void cachedb_clear(struct module_qstate* qstate, int id);
/** return memory estimate for cachedb module */
size_t cachedb_get_mem(struct module_env* env, int id);

/**
 * Get the function block with pointers to the cachedb functions
 * @return the function block for "cachedb".
 */
struct module_func_block* cachedb_get_funcblock(void);

