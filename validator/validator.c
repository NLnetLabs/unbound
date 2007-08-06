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
#include "validator/val_anchor.h"
#include "validator/val_kcache.h"
#include "services/cache/dns.h"
#include "util/module.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/region-allocator.h"

/** apply config settings to validator */
static int
val_apply_cfg(struct val_env* val_env, struct config_file* cfg)
{
	if(!val_env->anchors)
		val_env->anchors = anchors_create();
	if(!val_env->anchors) {
		log_err("out of memory");
		return 0;
	}
	if(!val_env->kcache)
		val_env->kcache = key_cache_create(cfg);
	if(!val_env->kcache) {
		log_err("out of memory");
		return 0;
	}
	if(!anchors_apply_cfg(val_env->anchors, cfg)) {
		log_err("validator: error in trustanchors config");
		return 0;
	}
	return 1;
}

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
	if(!val_apply_cfg(val_env, env->cfg)) {
		log_err("validator: could not apply configuration settings.");
		return 0;
	}
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
	anchors_delete(val_env->anchors);
	key_cache_delete(val_env->kcache);
	free(val_env);
}

/** allocate new validator query state */
static struct val_qstate*
val_new(struct module_qstate* qstate, int id)
{
	struct val_qstate* vq = (struct val_qstate*)region_alloc(
		qstate->region, sizeof(*vq));
	log_assert(!qstate->minfo[id]);
	if(!vq)
		return NULL;
	memset(vq, 0, sizeof(*vq));
	qstate->minfo[id] = vq;
	vq->state = VAL_INIT_STATE;
	if(!qstate->return_msg || qstate->return_rcode != LDNS_RCODE_NOERROR) {
		/* create a message to verify */
		verbose(VERB_ALGO, "constructing reply for validation");
		vq->orig_msg = (struct dns_msg*)region_alloc(qstate->region,
			sizeof(struct dns_msg));
		if(!vq->orig_msg)
			return NULL;
		vq->orig_msg->qinfo = qstate->qinfo;
		vq->orig_msg->rep = (struct reply_info*)region_alloc(
			qstate->region, sizeof(struct reply_info));
		if(!vq->orig_msg->rep)
			return NULL;
		memset(vq->orig_msg->rep, 0, sizeof(struct reply_info));
		vq->orig_msg->rep->flags = (uint16_t)(qstate->return_rcode&0xf)
			|BIT_QR|BIT_RA|(qstate->query_flags|(BIT_CD|BIT_RD));
		vq->orig_msg->rep->qdcount = 1;
	} else {
		vq->orig_msg = qstate->return_msg;
	}
	vq->qchase = qstate->qinfo;
	vq->chase_reply = vq->orig_msg->rep;
	return vq;
}

/** 
 * Process init state for validator.
 *
 * @param qstate: query state.
 * @param vq: validator query state.
 * @param ve: validator shared global environment.
 * @param id: module id.
 * @return true to continue processing
 */
static int
processInit(struct module_qstate* qstate, struct val_qstate* vq, 
	struct val_env* ve, int id)
{
	return 0;
}

/** 
 * Handle validator state.
 * If a method returns true, the next state is started. If false, then
 * processing will stop.
 * @param qstate: query state.
 * @param vq: validator query state.
 * @param ve: validator shared global environment.
 * @param id: module id.
 */
static void
val_handle(struct module_qstate* qstate, struct val_qstate* vq, 
	struct val_env* ve, int id)
{
	int cont = 1;
	while(cont) {
		verbose(VERB_ALGO, "val handle processing q with state %s",
			val_state_to_string(vq->state));
		switch(vq->state) {
			case VAL_INIT_STATE:
				cont = processInit(qstate, vq, ve, id);
				break;
			default:
				log_warn("validator: invalid state %d",
					vq->state);
				cont = 0;
				break;
		}
	}
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
	log_query_info(VERB_DETAIL, "validator operate: query",
		&qstate->qinfo);
	(void)outbound;
	if(event == module_event_new || event == module_event_pass) {
		/* pass request to next module, to get it */
		verbose(VERB_ALGO, "validator: pass to next module");
		qstate->ext_state[id] = module_wait_module;
		return;
	}
	if(event == module_event_moddone) {
		/* check if validation is needed */
		verbose(VERB_ALGO, "validator: nextmodule returned");
		if(qstate->return_rcode != LDNS_RCODE_NOERROR && 
			qstate->return_rcode != LDNS_RCODE_NXDOMAIN &&
			qstate->return_rcode != LDNS_RCODE_YXDOMAIN) {
			verbose(VERB_ALGO, "rcode for error not validated");
			qstate->ext_state[id] = module_finished;
			return;
		}
		if(qstate->query_flags & BIT_CD) {
			/* no need to validate this */
			verbose(VERB_ALGO, "CD bit set: query not validated");
			qstate->ext_state[id] = module_finished;
			return;
		}
		/* create state to start validation */
		qstate->ext_state[id] = module_error; /* override this */
		if(!vq) {
			vq = val_new(qstate, id);
			if(!vq) {
				log_err("validator: malloc failure");
				qstate->ext_state[id] = module_error;
				return;
			}
		}
		val_handle(qstate, vq, ve, id);
		return;
	}
	log_err("validator: bad event %s", strmodulevent(event));
	qstate->ext_state[id] = module_error;
	return;
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
	log_query_info(VERB_ALGO, "validator: inform_super, sub=",
		&qstate->qinfo);
	log_query_info(VERB_ALGO, "super=", &super->qinfo);
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
		case VAL_INIT_STATE: return "VAL_INIT_STATE";
		case VAL_PRIME_RESP_STATE: return "VAL_PRIME_RESP_STATE";
		case VAL_FINDKEY_STATE: return "VAL_FINDKEY_STATE";
		case VAL_FINDKEY_DS_RESP_STATE: 
				return "VAL_FINDKEY_DS_RESP_STATE";
		case VAL_FINDKEY_DNSKEY_RESP_STATE: 
				return "VAL_FINDKEY_DNSKEY_RESP_STATE";
		case VAL_VALIDATE_STATE: return "VAL_VALIDATE_STATE";
		case VAL_FINISHED_STATE: return "VAL_FINISHED_STATE";
	}
	return "UNKNOWN VALIDATOR STATE";
}
