/*
 * iterator/iterator.c - iterative resolver DNS query response module
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
 * This file contains a module that performs recusive iterative DNS query
 * processing.
 */

#include "config.h"
#include "iterator/iterator.h"
#include "util/module.h"
#include "util/netevent.h"
#include "util/net_help.h"
#include "util/storage/slabhash.h"
#include "util/region-allocator.h"
#include "services/cache/rrset.h"
#include "iterator/iter_utils.h"
#include "iterator/iter_hints.h"

/** iterator init */
static int 
iter_init(struct module_env* env, int id)
{
	struct iter_env* iter_env = (struct iter_env*)calloc(1,
		sizeof(struct iter_env));
	if(!iter_env) {
		log_err("malloc failure");
		return 0;
	}
	env->modinfo[id] = (void*)iter_env;
	if(!iter_apply_cfg(iter_env, env->cfg)) {
		log_err("iterator: could not apply configuration settings.");
		return 0;
	}
	return 1;
}

/** iterator deinit */
static void 
iter_deinit(struct module_env* env, int id)
{
	struct iter_env* iter_env;
	if(!env || !env->modinfo)
		return;
	iter_env = (struct iter_env*)env->modinfo[id];
	free(iter_env->target_fetch_policy);
	hints_delete(iter_env->hints);
	if(iter_env)
		free(iter_env);
}

/** store rrsets in the rrset cache. */
static void
store_rrsets(struct module_env* env, struct reply_info* rep, uint32_t now)
{
        size_t i;
        /* see if rrset already exists in cache, if not insert it. */
        for(i=0; i<rep->rrset_count; i++) {
                rep->ref[i].key = rep->rrsets[i];
                rep->ref[i].id = rep->rrsets[i]->id;
		if(rrset_cache_update(env->rrset_cache, &rep->ref[i], 
			env->alloc, now)) /* it was in the cache */
			rep->rrsets[i] = rep->ref[i].key;
        }
}


/** store message in the cache */
static void
store_msg(struct module_qstate* qstate, struct query_info* qinfo,
	struct reply_info* rep)
{
	struct msgreply_entry* e;
	uint32_t now = time(NULL);
	reply_info_set_ttls(rep, now);
	store_rrsets(qstate->env, rep, now);
	if(rep->ttl == 0) {
		log_info("TTL 0: dropped msg from cache");
		return;
	}
	reply_info_sortref(rep);
	/* store msg in the cache */
	if(!(e = query_info_entrysetup(qinfo, rep, qstate->query_hash))) {
		log_err("store_msg: malloc failed");
		return;
	}
	slabhash_insert(qstate->env->msg_cache, qstate->query_hash,
		&e->entry, rep, &qstate->env->alloc);
}

/** new query for iterator */
static int
iter_new(struct module_qstate* qstate, int id)
{
	struct iter_qstate* iq = (struct iter_qstate*)region_alloc(
		qstate->region, sizeof(struct iter_qstate));
	struct module_env* env = qstate->env;
	struct iter_env* ie = (struct iter_env*)env->modinfo[id];
	struct outbound_entry* e;
	uint16_t flags = 0; /* opcode=query, no flags */
	int dnssec = 1; /* always get dnssec info */
	qstate->minfo[id] = iq;
	if(!iq) 
		return 0;
	memset(iq, 0, sizeof(*iq));
	outbound_list_init(&iq->outlist);
	iq->num_target_queries = -1; /* default our targetQueries counter. */
	if(qstate->qinfo.has_cd)
		flags |= BIT_CD;
	e = (*env->send_query)(qstate->qinfo.qname, qstate->qinfo.qnamesize,
		qstate->qinfo.qtype, qstate->qinfo.qclass, flags, dnssec, 
		&ie->fwd_addr, ie->fwd_addrlen, qstate);
	if(!e) 
		return 0;
	outbound_list_insert(&iq->outlist, e);
	qstate->ext_state[id] = module_wait_reply;
	return 1;
}

/** iterator handle reply from authoritative server */
static int
iter_handlereply(struct module_qstate* qstate, int id,
        struct outbound_entry* ATTR_UNUSED(outbound))
{
	struct module_env* env = qstate->env;
	uint16_t us = qstate->edns.udp_size;
	struct query_info reply_qinfo;
	struct reply_info* reply_msg;
	struct edns_data reply_edns;
	int r;
	if((r=reply_info_parse(qstate->reply->c->buffer, env->alloc, 
		&reply_qinfo, &reply_msg, qstate->scratch, 
		&reply_edns))!=0)
		return 0;

	qstate->edns.edns_version = EDNS_ADVERTISED_VERSION;
	qstate->edns.udp_size = EDNS_ADVERTISED_SIZE;
	qstate->edns.ext_rcode = 0;
	qstate->edns.bits &= EDNS_DO;
	if(!reply_info_answer_encode(&reply_qinfo, reply_msg, 0, 
		qstate->query_flags, qstate->buf, 0, 0, 
		qstate->scratch, us, &qstate->edns))
		return 0;
	store_msg(qstate, &reply_qinfo, reply_msg);
	qstate->ext_state[id] = module_finished;
	return 1;
}

/** iterator operate on a query */
static void 
iter_operate(struct module_qstate* qstate, enum module_ev event, int id,
	struct outbound_entry* outbound)
{
	verbose(VERB_ALGO, "iterator[module %d] operate: extstate:%s event:%s", 
		id, strextstate(qstate->ext_state[id]), strmodulevent(event));
	if(event == module_event_new) {
		if(!iter_new(qstate, id))
			qstate->ext_state[id] = module_error;
		return;
	}
	/* it must be a query reply */
	if(!outbound) {
		verbose(VERB_ALGO, "query reply was not serviced");
		qstate->ext_state[id] = module_error;
		return;
	}
	if(event == module_event_timeout || event == module_event_error) {
		qstate->ext_state[id] = module_error;
		return;
	}
	if(event == module_event_reply) {
		if(!iter_handlereply(qstate, id, outbound))
			qstate->ext_state[id] = module_error;
		return;
	}
	log_err("bad event for iterator");
	qstate->ext_state[id] = module_error;
}

/** iterator cleanup query state */
static void 
iter_clear(struct module_qstate* qstate, int id)
{
	struct iter_qstate* iq;
	if(!qstate)
		return;
	iq = (struct iter_qstate*)qstate->minfo[id];
	outbound_list_clear(&iq->outlist);
	qstate->minfo[id] = NULL;
}

/**
 * The iterator function block 
 */
static struct module_func_block iter_block = {
	"iterator",
	&iter_init, &iter_deinit, &iter_operate, &iter_clear
};

struct module_func_block* 
iter_get_funcblock()
{
	return &iter_block;
}

const char* 
iter_state_to_string(enum iter_state state)
{
	switch (state)
	{
	case INIT_REQUEST_STATE :
		return "INIT REQUEST STATE";
	case INIT_REQUEST_2_STATE :
		return "INIT REQUEST STATE (stage 2)";
	case INIT_REQUEST_3_STATE:
		return "INIT REQUEST STATE (stage 3)";
	case QUERYTARGETS_STATE :
		return "QUERY TARGETS STATE";
	case PRIME_RESP_STATE :
		return "PRIME RESPONSE STATE";
	case QUERY_RESP_STATE :
		return "QUERY RESPONSE STATE";
	case TARGET_RESP_STATE :
		return "TARGET RESPONSE STATE";
	case FINISHED_STATE :
		return "FINISHED RESPONSE STATE";
	default :
		return "UNKNOWN ITER STATE";
	}
}
