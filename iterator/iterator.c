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
#include "iterator/iter_utils.h"
#include "iterator/iter_hints.h"
#include "iterator/iter_resptype.h"
#include "services/cache/dns.h"
#include "util/module.h"
#include "util/netevent.h"
#include "util/net_help.h"
#include "util/region-allocator.h"
#include "util/data/dname.h"

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

/** new query for iterator */
static int
iter_new(struct module_qstate* qstate, int id)
{
	struct iter_qstate* iq = (struct iter_qstate*)region_alloc(
		qstate->region, sizeof(struct iter_qstate));
	qstate->minfo[id] = iq;
	if(!iq) 
		return 0;
	memset(iq, 0, sizeof(*iq));
	iq->state = INIT_REQUEST_STATE;
	iq->final_state = FINISHED_STATE;
	iq->prepend_list = NULL;
	iq->prepend_last = NULL;
	iq->dp = NULL;
	iq->current_target = NULL;
	iq->num_target_queries = -1; /* default our targetQueries counter. */
	iq->num_current_queries = 0;
	iq->query_restart_count = 0;
	iq->referral_count = 0;
	iq->priming_stub = 0;
	iq->orig_qflags = qstate->query_flags;
	outbound_list_init(&iq->outlist);
	return 1;
}

/** new query for iterator in forward mode */
static int
fwd_new(struct module_qstate* qstate, int id)
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
	e = (*env->send_query)(qstate->qinfo.qname, qstate->qinfo.qname_len,
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
	dns_cache_store_msg(qstate->env, &reply_qinfo, qstate->query_hash, 
		reply_msg);
	qstate->ext_state[id] = module_finished;
	return 1;
}

/** perform forwarder functionality */
static void 
perform_forward(struct module_qstate* qstate, enum module_ev event, int id,
	struct outbound_entry* outbound)
{
	verbose(VERB_ALGO, "iterator: forwarding");
	if(event == module_event_new) {
		if(!fwd_new(qstate, id))
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
	log_err("bad event for iterator[forwarding]");
	qstate->ext_state[id] = module_error;
}

/**
 * Transition to the next state. This can be used to advance a currently
 * processing event. It cannot be used to reactivate a forEvent.
 *
 * @param qstate: query state
 * @param iq: iterator query state
 * @param nextstate The state to transition to.
 * @return true. This is so this can be called as the return value for the
 *         actual process*State() methods. (Transitioning to the next state
 *         implies further processing).
 */
static int
next_state(struct module_qstate* qstate, struct iter_qstate* iq, 
	enum iter_state nextstate)
{
	/* If transitioning to a "response" state, make sure that there is a
	 * response */
	if(iter_state_is_responsestate(nextstate)) {
		if(qstate->reply == NULL) {
			log_err("transitioning to response state sans "
				"response.");
		}
	}
	iq->state = nextstate;
	return 1;
}

/**
 * Transition an event to its final state. Final states always either return
 * a result up the module chain, or reactivate a dependent event. Which
 * final state to transtion to is set in the module state for the event when
 * it was created, and depends on the original purpose of the event.
 *
 * The response is stored in the qstate->buf buffer.
 *
 * @param qstate: query state
 * @param iq: iterator query state
 * @return false. This is so this method can be used as the return value for
 *         the processState methods. (Transitioning to the final state
 */
static int
final_state(struct module_qstate* qstate, struct iter_qstate* iq)
{
	return next_state(qstate, iq, iq->final_state);
}

/**
 * Return an error to the client
 */
static int
error_response(struct module_qstate* qstate, struct iter_qstate* iq, int rcode)
{
	log_info("err response %s", ldns_lookup_by_id(ldns_rcodes, rcode)?
		ldns_lookup_by_id(ldns_rcodes, rcode)->name:"??");
	qinfo_query_encode(qstate->buf, &qstate->qinfo);
	LDNS_RCODE_SET(ldns_buffer_begin(qstate->buf), rcode);
	LDNS_QR_SET(ldns_buffer_begin(qstate->buf));
	return final_state(qstate, iq);
}

#if 0
/** prepend the prepend list in the answer section of dns_msg */
static int
iter_prepend(struct iter_qstate* iq, struct dns_msg* msg, 
	struct region* region)
{
	struct packed_rrset_list* p;
	struct ub_packed_rrset_key** sets;
	size_t num = 0;
	for(p = iq->prepend_list; p; p = p->next)
		num++;
	if(num == 0)
		return 1;
	sets = region_alloc(region, (num+msg->rep->rrset_count) *
		sizeof(struct ub_packed_rrset_key*));
	if(!sets) 
		return 0;
	memcpy(sets+num, msg->rep->rrsets, msg->rep->rrset_count *
		sizeof(struct ub_packed_rrset_key*));
	num = 0;
	for(p = iq->prepend_list; p; p = p->next) {
		sets[num] = (struct ub_packed_rrset_key*)region_alloc(region,
			sizeof(struct ub_packed_rrset_key));
		if(!sets[num])
			return 0;
		sets[num]->rk = *p->rrset.k;
		sets[num]->entry.data = p->rrset.d;
		num++;
	}
	msg->rep->rrsets = sets;
	return 1;
}

/**
 * Encode response message for iterator responses. Into response buffer.
 * On error an error message is encoded.
 * @param qstate: query state. With qinfo information.
 * @param iq: iterator query state. With qinfo original and prepend list.
 * @param msg: answer message.
 */
static void 
iter_encode_respmsg(struct module_qstate* qstate, struct iter_qstate* iq, 
	struct dns_msg* msg)
{
	struct query_info qinf = qstate->qinfo;
	uint32_t now = time(NULL);
	struct edns_data edns;
	if(iq->orig_qname) {
		qinf.qname = iq->orig_qname;
		qinf.qname_len = iq->orig_qnamelen;
	}
	if(iq->prepend_list) {
		if(!iter_prepend(iq, msg, qstate->region)) {
			error_response(qstate, iq, LDNS_RCODE_SERVFAIL);
			return;
		}
	}

	edns.edns_present = qstate->edns.edns_present;
	edns.edns_version = EDNS_ADVERTISED_VERSION;
	edns.udp_size = EDNS_ADVERTISED_SIZE;
	edns.ext_rcode = 0;
	edns.bits = qstate->edns.bits & EDNS_DO;
	if(!reply_info_answer_encode(&qinf, msg->rep, 0, iq->orig_qflags, 
		qstate->buf, now, 1, qstate->scratch, qstate->edns.udp_size, 
		&edns)) {
		/* encode servfail */
		error_response(qstate, iq, LDNS_RCODE_SERVFAIL);
		return;
	}
}
#endif

/**
 * Add rrset to prepend list
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param rrset: rrset to add.
 * @return false on failure (malloc).
 */
static int
iter_add_prepend(struct module_qstate* qstate, struct iter_qstate* iq,
	struct ub_packed_rrset_key* rrset)
{
	struct iter_prep_list* p = (struct iter_prep_list*)region_alloc(
		qstate->region, sizeof(struct iter_prep_list));
	if(!p)
		return 0;
	p->rrset = rrset;
	p->next = NULL;
	/* add at end */
	if(iq->prepend_last)
		iq->prepend_last->next = p;
	else	iq->prepend_list = p;
	iq->prepend_last = p;
	return 1;
}

/**
 * Given a CNAME response (defined as a response containing a CNAME or DNAME
 * that does not answer the request), process the response, modifying the
 * state as necessary. This follows the CNAME/DNAME chain and returns the
 * final query name.
 *
 * sets the new query name, after following the CNAME/DNAME chain.
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param msg: the response.
 * @param mname: returned target new query name.
 * @param mname_len: length of mname.
 * @return false on (malloc) error.
 */
static int
handle_cname_response(struct module_qstate* qstate, struct iter_qstate* iq,
        struct dns_msg* msg, uint8_t** mname, size_t* mname_len)
{
	size_t i;
	/* Start with the (current) qname. */
	*mname = qstate->qinfo.qname;
	*mname_len = qstate->qinfo.qname_len;

	/* Iterate over the ANSWER rrsets in order, looking for CNAMEs and 
	 * DNAMES. */
	for(i=0; i<msg->rep->an_numrrsets; i++) {
		struct ub_packed_rrset_key* r = msg->rep->rrsets[i];
		/* If there is a (relevant) DNAME, add it to the list.
		 * We always expect there to be CNAME that was generated 
		 * by this DNAME following, so we don't process the DNAME 
		 * directly.  */
		if(ntohs(r->rk.type) == LDNS_RR_TYPE_DNAME &&
			dname_strict_subdomain_c(*mname, r->rk.dname)) {
			if(!iter_add_prepend(qstate, iq, r))
				return 0;
			continue;
		}

		if(ntohs(r->rk.type) == LDNS_RR_TYPE_CNAME &&
			query_dname_compare(*mname, r->rk.dname) == 0) {
			/* Add this relevant CNAME rrset to the prepend list.*/
			if(!iter_add_prepend(qstate, iq, r))
				return 0;
			get_cname_target(r, mname, mname_len);
		}

		/* Other rrsets in the section are ignored. */
	}
	return 1;
}

/**
 * Generate a subrequest.
 * Generate a local request event. Local events are tied to this module, and
 * have a correponding (first tier) event that is waiting for this event to
 * resolve to continue.
 *
 * @param qname The query name for this request.
 * @param qnamelen length of qname
 * @param qtype The query type for this request.
 * @param qclass The query class for this request.
 * @param qstate The event that is generating this event.
 * @param id: module id.
 * @param initial_state The initial response state (normally this
 *          is QUERY_RESP_STATE, unless it is known that the request won't
 *          need iterative processing
 * @param final_state The final state for the response to this
 *          request.
 * @return generated subquerystate, or NULL on error (malloc).
 */
static struct module_qstate* 
generate_sub_request(uint8_t* qname, size_t qnamelen, uint16_t qtype, 
	uint16_t qclass, struct module_qstate* qstate, int id,
	enum iter_state initial_state, enum iter_state final_state)
{
	struct module_qstate* subq = (struct module_qstate*)malloc(
		sizeof(struct module_qstate));
	struct iter_qstate* subiq;
	if(!subq)
		return NULL;
	memset(subq, 0, sizeof(*subq));
	subq->qinfo.qname = memdup(qname, qnamelen);
	if(!subq->qinfo.qname) {
		free(subq);
		return NULL;
	}
	subq->qinfo.qname_len = qnamelen;
	subq->qinfo.qtype = qtype;
	subq->qinfo.qclass = qclass;
	subq->query_hash = query_info_hash(&subq->qinfo);
	subq->query_flags = 0; /* OPCODE QUERY, no flags */
	subq->edns.udp_size = 65535;
	subq->buf = qstate->buf;
	subq->scratch = qstate->scratch;
	subq->region = region_create(malloc, free);
	if(!subq->region) {
		free(subq->qinfo.qname);
		free(subq);
		return NULL;
	}
	subq->curmod = id;
	subq->ext_state[id] = module_state_initial;
	subq->minfo[id] = region_alloc(subq->region, 
		sizeof(struct iter_qstate));
	if(!subq->minfo[id]) {
		region_destroy(subq->region);
		free(subq->qinfo.qname);
		free(subq);
		return NULL;
	}
	subq->env = qstate->env;
	subq->work_info = qstate->work_info;
	subq->parent = qstate;
	subq->subquery_next = qstate->subquery_first;
	qstate->subquery_first = subq;

	subiq = (struct iter_qstate*)subq->minfo[id];
	memset(subiq, 0, sizeof(*subiq));
	subiq->num_target_queries = -1; /* default our targetQueries counter. */
	outbound_list_init(&subiq->outlist);
	subiq->state = initial_state;
	subiq->final_state = final_state;

	/* RD should be set only when sending the query back through the INIT
	 * state. */
	if(initial_state == INIT_REQUEST_STATE)
		subq->query_flags |= BIT_RD;
	/* We set the CD flag so we can send this through the "head" of 
	 * the resolution chain, which might have a validator. We are 
	 * uninterested in validating things not on the direct resolution 
	 * path.  */
	subq->query_flags |= BIT_CD;
	
	return subq;
}

/**
 * Generate and send a root priming request.
 * @param qstate: the qtstate that triggered the need to prime.
 * @param ie: iterator global state.
 * @param id: module id.
 * @param qclass: the class to prime.
 */
static int
prime_root(struct module_qstate* qstate, struct iter_env* ie, int id, 
	uint16_t qclass)
{
	struct delegpt* dp;
	struct module_qstate* subq;
	struct iter_qstate* subiq;
	verbose(VERB_ALGO, "priming <./%s>", 
		ldns_lookup_by_id(ldns_rr_classes, (int)qclass)?
		ldns_lookup_by_id(ldns_rr_classes, (int)qclass)->name:"??");
	dp = hints_lookup_root(ie->hints, qclass);
	if(!dp) {
		verbose(VERB_ALGO, "Cannot prime due to lack of hints");
		return 0;
	}
	/* Priming requests start at the QUERYTARGETS state, skipping 
	 * the normal INIT state logic (which would cause an infloop). */
	subq = generate_sub_request((uint8_t*)"\000", 1, LDNS_RR_TYPE_NS, 
		qclass, qstate, id, QUERYTARGETS_STATE, PRIME_RESP_STATE);
	if(!subq) {
		log_err("out of memory priming root");
		return 0;
	}
	subiq = (struct iter_qstate*)subq->minfo[id];

	/* Set the initial delegation point to the hint. */
	subiq->dp = dp;
	/* suppress any target queries. */
	subiq->num_target_queries = 0; 
	
	/* this module stops, our submodule starts, and does the query. */
	qstate->ext_state[id] = module_wait_subquery;
	return 1;
}

/** 
 * Process the initial part of the request handling. This state roughly
 * corresponds to resolver algorithms steps 1 (find answer in cache) and 2
 * (find the best servers to ask).
 *
 * Note that all requests start here, and query restarts revisit this state.
 *
 * This state either generates: 1) a response, from cache or error, 2) a
 * priming event, or 3) forwards the request to the next state (init2,
 * generally).
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param ie: iterator shared global environment.
 * @param id: module id.
 * @return true if the event needs more request processing immediately,
 *         false if not.
 */
static int
processInitRequest(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	int d;
	uint8_t* delname;
	size_t delnamelen;
	struct dns_msg* msg;

	log_nametypeclass("resolving", qstate->qinfo.qname, 
		qstate->qinfo.qtype, qstate->qinfo.qclass);
	/* check effort */

	/* We enforce a maximum number of query restarts. This is primarily a
	 * cheap way to prevent CNAME loops. */
	if(iq->query_restart_count > MAX_RESTART_COUNT) {
		verbose(VERB_DETAIL, "request has exceeded the maximum number"
			" of query restarts with %d", iq->query_restart_count);
		return error_response(qstate, iq, LDNS_RCODE_SERVFAIL);
	}

	/* We enforce a maximum recursion/dependency depth -- in general, 
	 * this is unnecessary for dependency loops (although it will 
	 * catch those), but it provides a sensible limit to the amount 
	 * of work required to answer a given query. */
	d = module_subreq_depth(qstate);
	verbose(VERB_ALGO, "request has dependency depth of %d", d);
	if(d > ie->max_dependency_depth) {
		verbose(VERB_DETAIL, "request has exceeded the maximum "
			"dependency depth with depth of %d", d);
		return error_response(qstate, iq, LDNS_RCODE_SERVFAIL);
	}

	/* Resolver Algorithm Step 1 -- Look for the answer in local data. */

	/* This either results in a query restart (CNAME cache response), a
	 * terminating response (ANSWER), or a cache miss (null). */
	
	msg = dns_cache_lookup(qstate->env, qstate->qinfo.qname, 
		qstate->qinfo.qname_len, qstate->qinfo.qtype, 
		qstate->qinfo.qclass, qstate->region, qstate->scratch);
	if(msg) {
		/* handle positive cache response */
		enum response_type type = response_type_from_cache(msg, 
			&qstate->qinfo);

		if(type == RESPONSE_TYPE_CNAME) {
			uint8_t* sname = 0;
			size_t slen = 0;
			verbose(VERB_ALGO, "returning CNAME response from "
				"cache");
			if(!iq->orig_qname) {
				iq->orig_qname = qstate->qinfo.qname;
				iq->orig_qnamelen = qstate->qinfo.qname_len;
			}
			if(!handle_cname_response(qstate, iq, msg, 
				&sname, &slen))
				return error_response(qstate, iq,
					LDNS_RCODE_SERVFAIL);
			qstate->qinfo.qname = sname;
			qstate->qinfo.qname_len = slen;
			/* This *is* a query restart, even if it is a cheap 
			 * one. */
			iq->query_restart_count++;
			return next_state(qstate, iq, INIT_REQUEST_STATE);
		}

		/* it is an answer, response, to final state */
		verbose(VERB_ALGO, "returning answer from cache.");
		iq->response = msg;
		return final_state(qstate, iq);
	}
	
	/* TODO attempt to forward the request */
	/* if (forwardRequest(event, state, req))
	   {
		// the request has been forwarded.
		// forwarded requests need to be immediately sent to the 
		// next state, QUERYTARGETS.
		return nextState(event, req, state, 
			IterEventState.QUERYTARGETS_STATE);
		}
	*/

	/* TODO attempt to find a covering DNAME in the cache */
	/* resp = mDNSCache.findDNAME(req.getQName(), req.getQType(), req
	        .getQClass());
	    if (resp != null)
	{
log.trace("returning synthesized CNAME response from cache: " + resp);
Name cname = handleCNAMEResponse(state, req, resp);
// At this point, we just initiate the query restart.
// This might not be a query restart situation (e.g., qtype == CNAME),
// but
// the answer returned from findDNAME() is likely to be one that we
// don't want to return.
// Thus we allow the cache and other resolution mojo kick in regardless.
req.setQName(cname);
state.queryRestartCount++;
return nextState(event, req, state, IterEventState.INIT_REQUEST_STATE);
}
	*/

	/* Resolver Algorithm Step 2 -- find the "best" servers. */

	/* first, adjust for DS queries. To avoid the grandparent problem, 
	 * we just look for the closest set of server to the parent of qname.
	 */
	delname = qstate->qinfo.qname;
	delnamelen = qstate->qinfo.qname_len;
	if(qstate->qinfo.qtype == LDNS_RR_TYPE_DS && delname[0] != 0) {
		/* do not adjust root label, remove first label from delname */
		size_t lablen = delname[0] + 1;
		delname += lablen;
		delnamelen -= lablen;
	}
	
	/* Lookup the delegation in the cache. If null, then the cache needs 
	 * to be primed for the qclass. */
	iq->dp = dns_cache_find_delegation(qstate->env, delname, delnamelen,
		qstate->qinfo.qclass, qstate->region);

	/* If the cache has returned nothing, then we have a root priming
	 * situation. */
	if(iq->dp == NULL) {
		/* Note that the result of this will set a new
		 * DelegationPoint based on the result of priming. */
		if(!prime_root(qstate, ie, id, qstate->qinfo.qclass))
			return error_response(qstate, iq, LDNS_RCODE_REFUSED);

		/* priming creates an sends a subordinate query, with 
		 * this query as the parent. So further processing for 
		 * this event will stop until reactivated by the results 
		 * of priming. */
		return false;
	}

	/* Reset the RD flag. If this is a query restart, then the RD 
	 * will have been turned off. */
	 /*
	 TODO store original flags and original qinfo 
	qstate->query_flags |= (qstate->orig_query_flags & BIT_RD);
	*/

	/* Otherwise, set the current delegation point and move on to the 
	 * next state. */
	return next_state(qstate, iq, INIT_REQUEST_2_STATE);
}

#if 0
/** TODO */
static int
processInitRequest2(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}

/** TODO */
static int
processInitRequest3(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}

/** TODO */
static int
processQueryTargets(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}

/** TODO */
static int
processQueryResponse(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}

/** TODO */
static int
processPrimeResponse(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}

/** TODO */
static int
processTargetResponse(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}

/** TODO */
static int
processFinished(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	return 0;
}
#endif

/**
 * Handle iterator state.
 * Handle events. This is the real processing loop for events, responsible
 * for moving events through the various states. If a processing method
 * returns true, then it will be advanced to the next state. If false, then
 * processing will stop.
 *
 * @param qstate: query state.
 * @param ie: iterator shared global environment.
 * @param iq: iterator query state.
 * @param id: module id.
 */
static void
iter_handle(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	int cont = 1;
	while(cont) {
		verbose(VERB_ALGO, "iter_handle processing q with state %s",
			iter_state_to_string(iq->state));
		switch(iq->state) {
			case INIT_REQUEST_STATE:
				cont = processInitRequest(qstate, iq, ie, id);
				break;
#if 0
			case INIT_REQUEST_2_STATE:
				cont = processInitRequest2(qstate, iq, ie, id);
				break;
			case INIT_REQUEST_3_STATE:
				cont = processInitRequest3(qstate, iq, ie, id);
				break;
			case QUERYTARGETS_STATE:
				cont = processQueryTargets(qstate, iq, ie, id);
				break;
			case QUERY_RESP_STATE:
				cont = processQueryResponse(qstate, iq, ie, id);
				break;
			case PRIME_RESP_STATE:
				cont = processPrimeResponse(qstate, iq, ie, id);
				break;
			case TARGET_RESP_STATE:
				cont = processTargetResponse(qstate, iq, ie, id);
				break;
			case FINISHED_STATE:
				cont = processFinished(qstate, iq, ie, id);
				break;
#endif
			default:
				log_warn("iterator: invalid state: %d",
					iq->state);
				cont = 0;
				break;
		}
	}
}

/** 
 * This is the primary entry point for processing request events. Note that
 * this method should only be used by external modules.
 * @param qstate: query state.
 * @param ie: iterator shared global environment.
 * @param iq: iterator query state.
 * @param id: module id.
 */
static void
process_request(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	/* external requests start in the INIT state, and finish using the
	 * FINISHED state. */
	iq->state = INIT_REQUEST_STATE;
	iq->final_state = FINISHED_STATE;
	verbose(VERB_ALGO, "process_request: new external request event");
	iter_handle(qstate, iq, ie, id);
}

/** process authoritative server reply */
static void
process_response(struct module_qstate* qstate, struct iter_qstate* iq, 
	struct iter_env* ie, int id, struct outbound_entry* outbound)
{
	verbose(VERB_ALGO, "process_response: new external response event");
	/* TODO outbound: use it for scrubbing and so on */
	iq->state = QUERY_RESP_STATE;
	iter_handle(qstate, iq, ie, id);
}

/** iterator operate on a query */
static void 
iter_operate(struct module_qstate* qstate, enum module_ev event, int id,
	struct outbound_entry* outbound)
{
	struct iter_env* ie = (struct iter_env*)qstate->env->modinfo[id];
	struct iter_qstate* iq = (struct iter_qstate*)qstate->minfo[id];
	verbose(VERB_ALGO, "iterator[module %d] operate: extstate:%s event:%s", 
		id, strextstate(qstate->ext_state[id]), strmodulevent(event));
	if(ie->fwd_addrlen != 0) {
		perform_forward(qstate, event, id, outbound);
		return;
	}
	/* perform iterator state machine */
	if(event == module_event_new && iq == NULL) {
		log_info("iter state machine");
		if(!iter_new(qstate, id)) {
			qstate->ext_state[id] = module_error;
			return;
		}
		iq = (struct iter_qstate*)qstate->minfo[id];
		process_request(qstate, iq, ie, id);
		return;
	}
	if(event == module_event_pass) {
		iter_handle(qstate, iq, ie, id);
		return;
	}
	if(event == module_event_reply) {
		process_response(qstate, iq, ie, id, outbound);
		return;
	}
	/* TODO: uhh */

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
	if(iq->orig_qname) {
		/* so the correct qname gets free'd */
		qstate->qinfo.qname = iq->orig_qname;
		qstate->qinfo.qname_len = iq->orig_qnamelen;
	}
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

int 
iter_state_is_responsestate(enum iter_state s)
{
	switch(s) {
		case INIT_REQUEST_STATE :
		case INIT_REQUEST_2_STATE :
		case INIT_REQUEST_3_STATE :
		case QUERYTARGETS_STATE :
			return 0;
		default:
			break;
	}
	return 1;
}
