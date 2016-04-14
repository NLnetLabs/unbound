/*
 * cachedb/cachedb.c - cache from a database external to the program module
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

#include "config.h"
#ifdef USE_CACHEDB
#include "cachedb/cachedb.h"
#include "util/regional.h"
#include "sldns/parseutil.h"
#include "sldns/wire2str.h"

/** apply configuration to cachedb module 'global' state */
static int
cachedb_apply_cfg(struct cachedb_env* cachedb_env, struct config_file* cfg)
{
	/* TODO */
	return 1;
}

int 
cachedb_init(struct module_env* env, int id)
{
	struct cachedb_env* cachedb_env = (struct cachedb_env*)calloc(1,
		sizeof(struct cachedb_env));
	if(!cachedb_env) {
		log_err("malloc failure");
		return 0;
	}
	env->modinfo[id] = (void*)cachedb_env;
	if(!cachedb_apply_cfg(cachedb_env, env->cfg)) {
		log_err("cachedb: could not apply configuration settings.");
		return 0;
	}
	return 1;
}

void 
cachedb_deinit(struct module_env* env, int id)
{
	struct cachedb_env* cachedb_env;
	if(!env || !env->modinfo[id])
		return;
	cachedb_env = (struct cachedb_env*)env->modinfo[id];
	/* free contents */
	/* TODO */

	free(cachedb_env);
	env->modinfo[id] = NULL;
}

/** new query for cachedb */
static int
cachedb_new(struct module_qstate* qstate, int id)
{
	struct cachedb_qstate* iq = (struct cachedb_qstate*)regional_alloc(
		qstate->region, sizeof(struct cachedb_qstate));
	qstate->minfo[id] = iq;
	if(!iq) 
		return 0;
	memset(iq, 0, sizeof(*iq));
	/* initialise it */
	/* TODO */

	return 1;
}

/**
 * Return an error
 * @param qstate: our query state
 * @param id: module id
 * @param rcode: error code (DNS errcode).
 * @return: 0 for use by caller, to make notation easy, like:
 * 	return error_response(..). 
 */
static int
error_response(struct module_qstate* qstate, int id, int rcode)
{
	verbose(VERB_QUERY, "return error response %s", 
		sldns_lookup_by_id(sldns_rcodes, rcode)?
		sldns_lookup_by_id(sldns_rcodes, rcode)->name:"??");
	qstate->return_rcode = rcode;
	qstate->return_msg = NULL;
	qstate->ext_state[id] = module_finished;
	return 0;
}

/**
 * Handle a cachedb module event
 * @param qstate: query state (from the mesh), passed between modules.
 * 	contains qstate->env module environment with global caches and so on.
 * @param iq: query state specific for this module.  per-query.
 * @param ie: environment specific for this module.  global.
 * @param id: module id.
 */
static void
cachedb_handle(struct module_qstate* qstate, struct cachedb_qstate* iq,
	struct cachedb_env* ie, int id)
{
	/* figure out if this is a lookup or a store moment */
	/* TODO */
}

void 
cachedb_operate(struct module_qstate* qstate, enum module_ev event, int id,
	struct outbound_entry* outbound)
{
	struct cachedb_env* ie = (struct cachedb_env*)qstate->env->modinfo[id];
	struct cachedb_qstate* iq = (struct cachedb_qstate*)qstate->minfo[id];
	verbose(VERB_QUERY, "cachedb[module %d] operate: extstate:%s event:%s", 
		id, strextstate(qstate->ext_state[id]), strmodulevent(event));
	if(iq) log_query_info(VERB_QUERY, "cachedb operate: query", 
		&qstate->qinfo);

	/* perform cachedb state machine */
	if((event == module_event_new || event == module_event_pass) && 
		iq == NULL) {
		if(!cachedb_new(qstate, id)) {
			(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
			return;
		}
		iq = (struct cachedb_qstate*)qstate->minfo[id];
		cachedb_handle(qstate, iq, ie, id);
		return;
	}
	if(iq && event == module_event_pass) {
		cachedb_handle(qstate, iq, ie, id);
		return;
	}
	if(iq && outbound) {
		/* cachedb does not need to process responses at this time
		 * ignore it.
		cachedb_process_response(qstate, iq, ie, id, outbound, event);
		*/
		return;
	}
	if(event == module_event_error) {
		verbose(VERB_ALGO, "got called with event error, giving up");
		(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		return;
	}

	log_err("bad event for cachedb");
	(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
}

void
cachedb_inform_super(struct module_qstate* ATTR_UNUSED(qstate),
	int ATTR_UNUSED(id), struct module_qstate* ATTR_UNUSED(super))
{
	/* cachedb does not use subordinate requests at this time */
}

void 
cachedb_clear(struct module_qstate* qstate, int id)
{
	struct cachedb_qstate* iq;
	if(!qstate)
		return;
	iq = (struct cachedb_qstate*)qstate->minfo[id];
	if(iq) {
		/* free contents of iq */
		/* TODO */
	}
	qstate->minfo[id] = NULL;
}

size_t 
cachedb_get_mem(struct module_env* env, int id)
{
	struct cachedb_env* ie = (struct cachedb_env*)env->modinfo[id];
	if(!ie)
		return 0;
	return sizeof(*ie); /* TODO - more mem */
}

/**
 * The cachedb function block 
 */
static struct module_func_block cachedb_block = {
	"cachedb",
	&cachedb_init, &cachedb_deinit, &cachedb_operate,
	&cachedb_inform_super, &cachedb_clear, &cachedb_get_mem
};

struct module_func_block* 
cachedb_get_funcblock(void)
{
	return &cachedb_block;
}
#endif /* USE_CACHEDB */
