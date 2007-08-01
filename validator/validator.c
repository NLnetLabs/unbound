/*
 * validator/validator.c - secure validator DNS query response module
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
 * This file contains a module that performs validation of DNS queries.
 * According to RFC 4034.
 */
#include "config.h"
#include "validator/validator.h"
#include "util/module.h"
#include "util/log.h"

/** validator init */
static int
val_init(struct module_env* env, int id)
{
	struct val_env* val_env = (struct val_env*)calloc(1,
	sizeof(struct val_env));
	if(!val_env) {
		log_err("malloc failure");
		return 0;
	}
	env->modinfo[id] = (void*)val_env;
	/*if(!val_apply_cfg(val_env, env->cfg)) {
		log_err("validator: could not apply configuration settings.");
		return 0;
	}*/
	return 1;
}

/** validator deinit */
static void
val_deinit(struct module_env* env, int id)
{
	struct val_env* val_env;
	if(!env || !env->modinfo || !env->modinfo[id])
		return;
	val_env = (struct val_env*)env->modinfo[id];
	free(val_env);
}

/** validator operate on a query */
static void
val_operate(struct module_qstate* qstate, enum module_ev event, int id,
        struct outbound_entry* outbound)
{
	struct val_env* ve = (struct val_env*)qstate->env->modinfo[id];
	struct val_qstate* vq = (struct val_qstate*)qstate->minfo[id];
	verbose(VERB_DETAIL, "validator[module %d] operate: extstate:%s "
		"event:%s", id, strextstate(qstate->ext_state[id]), 
		strmodulevent(event));
	if(vq) log_query_info(VERB_DETAIL, "iterator operate: query",
		&qstate->qinfo);
	(void)ve;
	(void)outbound;
}

/** 
 * inform validator super.
 * 
 * @param qstate: query state that finished.
 * @param id: module id.
 * @param super: the qstate to inform.
 */
static void
val_inform_super(struct module_qstate* qstate, int id,
	struct module_qstate* super)
{
}


/** validator cleanup query state */
static void
val_clear(struct module_qstate* qstate, int id)
{
	if(!qstate)
		return;
	/* everything is allocated in the region, so assign NULL */
	qstate->minfo[id] = NULL;
}

/**
 * The validator function block 
 */
static struct module_func_block val_block = {
	"validator",
	&val_init, &val_deinit, &val_operate, &val_inform_super, &val_clear
};

struct module_func_block* 
val_get_funcblock()
{
	return &val_block;
}

const char* 
val_state_to_string(enum val_state state)
{
	switch(state) {
		case VAL_STATE_INIT: return "VAL_STATE_INIT";
	}
	return "UNKNOWN VALIDATOR STATE";
}
