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
#include "iterator/iter_fwd.h"
#include "iterator/iter_donotq.h"
#include "iterator/iter_delegpt.h"
#include "iterator/iter_resptype.h"
#include "iterator/iter_scrub.h"
#include "iterator/iter_priv.h"
#include "validator/val_neg.h"
#include "services/cache/dns.h"
#include "services/cache/infra.h"
#include "util/module.h"
#include "util/netevent.h"
#include "util/net_help.h"
#include "util/regional.h"
#include "util/data/dname.h"
#include "util/data/msgencode.h"
#include "util/fptr_wlist.h"
#include "util/config_file.h"

int 
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

void 
iter_deinit(struct module_env* env, int id)
{
	struct iter_env* iter_env;
	if(!env || !env->modinfo[id])
		return;
	iter_env = (struct iter_env*)env->modinfo[id];
	free(iter_env->target_fetch_policy);
	priv_delete(iter_env->priv);
	hints_delete(iter_env->hints);
	forwards_delete(iter_env->fwds);
	donotq_delete(iter_env->donotq);
	free(iter_env);
	env->modinfo[id] = NULL;
}

/** new query for iterator */
static int
iter_new(struct module_qstate* qstate, int id)
{
	struct iter_qstate* iq = (struct iter_qstate*)regional_alloc(
		qstate->region, sizeof(struct iter_qstate));
	qstate->minfo[id] = iq;
	if(!iq) 
		return 0;
	memset(iq, 0, sizeof(*iq));
	iq->state = INIT_REQUEST_STATE;
	iq->final_state = FINISHED_STATE;
	iq->an_prepend_list = NULL;
	iq->an_prepend_last = NULL;
	iq->ns_prepend_list = NULL;
	iq->ns_prepend_last = NULL;
	iq->dp = NULL;
	iq->depth = 0;
	iq->num_target_queries = 0;
	iq->num_current_queries = 0;
	iq->query_restart_count = 0;
	iq->referral_count = 0;
	iq->wait_priming_stub = 0;
	iq->refetch_glue = 0;
	iq->dnssec_expected = 0;
	iq->chase_flags = qstate->query_flags;
	/* Start with the (current) qname. */
	iq->qchase = qstate->qinfo;
	outbound_list_init(&iq->outlist);
	return 1;
}

/**
 * Transition to the next state. This can be used to advance a currently
 * processing event. It cannot be used to reactivate a forEvent.
 *
 * @param iq: iterator query state
 * @param nextstate The state to transition to.
 * @return true. This is so this can be called as the return value for the
 *         actual process*State() methods. (Transitioning to the next state
 *         implies further processing).
 */
static int
next_state(struct iter_qstate* iq, enum iter_state nextstate)
{
	/* If transitioning to a "response" state, make sure that there is a
	 * response */
	if(iter_state_is_responsestate(nextstate)) {
		if(iq->response == NULL) {
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
 * @param iq: iterator query state
 * @return false. This is so this method can be used as the return value for
 *         the processState methods. (Transitioning to the final state
 */
static int
final_state(struct iter_qstate* iq)
{
	return next_state(iq, iq->final_state);
}

/**
 * Callback routine to handle errors in parent query states
 * @param qstate: query state that failed.
 * @param id: module id.
 * @param super: super state.
 */
static void
error_supers(struct module_qstate* qstate, int id, struct module_qstate* super)
{
	struct iter_qstate* super_iq = (struct iter_qstate*)super->minfo[id];

	if(qstate->qinfo.qtype == LDNS_RR_TYPE_A ||
		qstate->qinfo.qtype == LDNS_RR_TYPE_AAAA) {
		/* mark address as failed. */
		struct delegpt_ns* dpns = NULL;
		if(super_iq->dp)
			dpns = delegpt_find_ns(super_iq->dp, 
				qstate->qinfo.qname, qstate->qinfo.qname_len);
		if(!dpns) {
			/* not interested */
			verbose(VERB_ALGO, "subq error, but not interested");
			log_query_info(VERB_ALGO, "superq", &super->qinfo);
			if(super_iq->dp)
				delegpt_log(VERB_ALGO, super_iq->dp);
			log_assert(0);
			return;
		}
		dpns->resolved = 1; /* mark as failed */
		super_iq->num_target_queries--; 
	}
	if(qstate->qinfo.qtype == LDNS_RR_TYPE_NS) {
		/* prime failed to get delegation */
		super_iq->dp = NULL;
	}
	/* evaluate targets again */
	super_iq->state = QUERYTARGETS_STATE; 
	/* super becomes runnable, and will process this change */
}

/**
 * Return an error to the client
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
		ldns_lookup_by_id(ldns_rcodes, rcode)?
		ldns_lookup_by_id(ldns_rcodes, rcode)->name:"??");
	qstate->return_rcode = rcode;
	qstate->return_msg = NULL;
	qstate->ext_state[id] = module_finished;
	return 0;
}

/** check if prepend item is duplicate item */
static int
prepend_is_duplicate(struct ub_packed_rrset_key** sets, size_t to,
	struct ub_packed_rrset_key* dup)
{
	size_t i;
	for(i=0; i<to; i++) {
		if(sets[i]->rk.type == dup->rk.type &&
			sets[i]->rk.rrset_class == dup->rk.rrset_class &&
			sets[i]->rk.dname_len == dup->rk.dname_len &&
			query_dname_compare(sets[i]->rk.dname, dup->rk.dname)
			== 0)
			return 1;
	}
	return 0;
}

/** prepend the prepend list in the answer and authority section of dns_msg */
static int
iter_prepend(struct iter_qstate* iq, struct dns_msg* msg, 
	struct regional* region)
{
	struct iter_prep_list* p;
	struct ub_packed_rrset_key** sets;
	size_t num_an = 0, num_ns = 0;;
	for(p = iq->an_prepend_list; p; p = p->next)
		num_an++;
	for(p = iq->ns_prepend_list; p; p = p->next)
		num_ns++;
	if(num_an + num_ns == 0)
		return 1;
	verbose(VERB_ALGO, "prepending %d rrsets", (int)num_an + (int)num_ns);
	sets = regional_alloc(region, (num_an+num_ns+msg->rep->rrset_count) *
		sizeof(struct ub_packed_rrset_key*));
	if(!sets) 
		return 0;
	/* ANSWER section */
	num_an = 0;
	for(p = iq->an_prepend_list; p; p = p->next) {
		sets[num_an++] = p->rrset;
	}
	memcpy(sets+num_an, msg->rep->rrsets, msg->rep->an_numrrsets *
		sizeof(struct ub_packed_rrset_key*));
	/* AUTH section */
	num_ns = 0;
	for(p = iq->ns_prepend_list; p; p = p->next) {
		if(prepend_is_duplicate(sets+msg->rep->an_numrrsets+num_an,
			num_ns, p->rrset) || prepend_is_duplicate(
			msg->rep->rrsets+msg->rep->an_numrrsets, 
			msg->rep->ns_numrrsets, p->rrset))
			continue;
		sets[msg->rep->an_numrrsets + num_an + num_ns++] = p->rrset;
	}
	memcpy(sets + num_an + msg->rep->an_numrrsets + num_ns, 
		msg->rep->rrsets + msg->rep->an_numrrsets, 
		(msg->rep->ns_numrrsets + msg->rep->ar_numrrsets) *
		sizeof(struct ub_packed_rrset_key*));

	/* if the rcode was NXDOMAIN, and we prepended DNAME/CNAMEs, then
	 * it should now be NOERROR. */
	if(FLAGS_GET_RCODE(msg->rep->flags) == LDNS_RCODE_NXDOMAIN) {
		FLAGS_SET_RCODE(msg->rep->flags, LDNS_RCODE_NOERROR);
	}
	msg->rep->rrset_count += num_an + num_ns;
	msg->rep->an_numrrsets += num_an;
	msg->rep->ns_numrrsets += num_ns;
	msg->rep->rrsets = sets;
	return 1;
}

/**
 * Add rrset to ANSWER prepend list
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param rrset: rrset to add.
 * @return false on failure (malloc).
 */
static int
iter_add_prepend_answer(struct module_qstate* qstate, struct iter_qstate* iq,
	struct ub_packed_rrset_key* rrset)
{
	struct iter_prep_list* p = (struct iter_prep_list*)regional_alloc(
		qstate->region, sizeof(struct iter_prep_list));
	if(!p)
		return 0;
	p->rrset = rrset;
	p->next = NULL;
	/* add at end */
	if(iq->an_prepend_last)
		iq->an_prepend_last->next = p;
	else	iq->an_prepend_list = p;
	iq->an_prepend_last = p;
	return 1;
}

/**
 * Add rrset to AUTHORITY prepend list
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param rrset: rrset to add.
 * @return false on failure (malloc).
 */
static int
iter_add_prepend_auth(struct module_qstate* qstate, struct iter_qstate* iq,
	struct ub_packed_rrset_key* rrset)
{
	struct iter_prep_list* p = (struct iter_prep_list*)regional_alloc(
		qstate->region, sizeof(struct iter_prep_list));
	if(!p)
		return 0;
	p->rrset = rrset;
	p->next = NULL;
	/* add at end */
	if(iq->ns_prepend_last)
		iq->ns_prepend_last->next = p;
	else	iq->ns_prepend_list = p;
	iq->ns_prepend_last = p;
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
	*mname = iq->qchase.qname;
	*mname_len = iq->qchase.qname_len;

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
			if(!iter_add_prepend_answer(qstate, iq, r))
				return 0;
			continue;
		}

		if(ntohs(r->rk.type) == LDNS_RR_TYPE_CNAME &&
			query_dname_compare(*mname, r->rk.dname) == 0) {
			/* Add this relevant CNAME rrset to the prepend list.*/
			if(!iter_add_prepend_answer(qstate, iq, r))
				return 0;
			get_cname_target(r, mname, mname_len);
		}

		/* Other rrsets in the section are ignored. */
	}
	/* add authority rrsets to authority prepend, for wildcarded CNAMEs */
	for(i=msg->rep->an_numrrsets; i<msg->rep->an_numrrsets +
		msg->rep->ns_numrrsets; i++) {
		struct ub_packed_rrset_key* r = msg->rep->rrsets[i];
		/* only add NSEC/NSEC3, as they may be needed for validation */
		if(ntohs(r->rk.type) == LDNS_RR_TYPE_NSEC ||
			ntohs(r->rk.type) == LDNS_RR_TYPE_NSEC3) {
			if(!iter_add_prepend_auth(qstate, iq, r))
				return 0;
		}
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
 * @param iq: The iterator state that is generating this event.
 * @param initial_state The initial response state (normally this
 *          is QUERY_RESP_STATE, unless it is known that the request won't
 *          need iterative processing
 * @param final_state The final state for the response to this
 *          request.
 * @param subq_ret: if newly allocated, the subquerystate, or NULL if it does
 * 	not need initialisation.
 * @param v: if true, validation is done on the subquery.
 * @return false on error (malloc).
 */
static int
generate_sub_request(uint8_t* qname, size_t qnamelen, uint16_t qtype, 
	uint16_t qclass, struct module_qstate* qstate, int id,
	struct iter_qstate* iq, enum iter_state initial_state, 
	enum iter_state final_state, struct module_qstate** subq_ret, int v)
{
	struct module_qstate* subq = NULL;
	struct iter_qstate* subiq = NULL;
	uint16_t qflags = 0; /* OPCODE QUERY, no flags */
	struct query_info qinf;
	int prime = (final_state == PRIME_RESP_STATE)?1:0;
	qinf.qname = qname;
	qinf.qname_len = qnamelen;
	qinf.qtype = qtype;
	qinf.qclass = qclass;

	/* RD should be set only when sending the query back through the INIT
	 * state. */
	if(initial_state == INIT_REQUEST_STATE)
		qflags |= BIT_RD;
	/* We set the CD flag so we can send this through the "head" of 
	 * the resolution chain, which might have a validator. We are 
	 * uninterested in validating things not on the direct resolution 
	 * path.  */
	if(!v)
		qflags |= BIT_CD;

	/* attach subquery, lookup existing or make a new one */
	fptr_ok(fptr_whitelist_modenv_attach_sub(qstate->env->attach_sub));
	if(!(*qstate->env->attach_sub)(qstate, &qinf, qflags, prime, &subq)) {
		return 0;
	}
	*subq_ret = subq;
	if(subq) {
		/* initialise the new subquery */
		subq->curmod = id;
		subq->ext_state[id] = module_state_initial;
		subq->minfo[id] = regional_alloc(subq->region, 
			sizeof(struct iter_qstate));
		if(!subq->minfo[id]) {
			log_err("init subq: out of memory");
			fptr_ok(fptr_whitelist_modenv_kill_sub(
				qstate->env->kill_sub));
			(*qstate->env->kill_sub)(subq);
			return 0;
		}
		subiq = (struct iter_qstate*)subq->minfo[id];
		memset(subiq, 0, sizeof(*subiq));
		subiq->num_target_queries = 0;
		subiq->num_current_queries = 0;
		subiq->depth = iq->depth+1;
		outbound_list_init(&subiq->outlist);
		subiq->state = initial_state;
		subiq->final_state = final_state;
		subiq->qchase = subq->qinfo;
		subiq->chase_flags = subq->query_flags;
		subiq->refetch_glue = 0;
	}
	return 1;
}

/**
 * Generate and send a root priming request.
 * @param qstate: the qtstate that triggered the need to prime.
 * @param iq: iterator query state.
 * @param ie: iterator global state.
 * @param id: module id.
 * @param qclass: the class to prime.
 */
static int
prime_root(struct module_qstate* qstate, struct iter_qstate* iq, 
	struct iter_env* ie, int id, uint16_t qclass)
{
	struct delegpt* dp;
	struct module_qstate* subq;
	verbose(VERB_DETAIL, "priming . %s NS", 
		ldns_lookup_by_id(ldns_rr_classes, (int)qclass)?
		ldns_lookup_by_id(ldns_rr_classes, (int)qclass)->name:"??");
	dp = hints_lookup_root(ie->hints, qclass);
	if(!dp) {
		verbose(VERB_ALGO, "Cannot prime due to lack of hints");
		return 0;
	}
	/* copy dp; to avoid messing up available list for other thr/queries */
	dp = delegpt_copy(dp, qstate->region);
	if(!dp) {
		log_err("out of memory priming root, copydp");
		return 0;
	}
	/* Priming requests start at the QUERYTARGETS state, skipping 
	 * the normal INIT state logic (which would cause an infloop). */
	if(!generate_sub_request((uint8_t*)"\000", 1, LDNS_RR_TYPE_NS, 
		qclass, qstate, id, iq, QUERYTARGETS_STATE, PRIME_RESP_STATE,
		&subq, 0)) {
		log_err("out of memory priming root");
		return 0;
	}
	if(subq) {
		struct iter_qstate* subiq = 
			(struct iter_qstate*)subq->minfo[id];
		/* Set the initial delegation point to the hint. */
		subiq->dp = dp;
		/* there should not be any target queries. */
		subiq->num_target_queries = 0; 
		subiq->dnssec_expected = iter_indicates_dnssec(
			qstate->env, subiq->dp, NULL, subq->qinfo.qclass);
	}
	
	/* this module stops, our submodule starts, and does the query. */
	qstate->ext_state[id] = module_wait_subquery;
	return 1;
}

/**
 * Generate and process a stub priming request. This method tests for the
 * need to prime a stub zone, so it is safe to call for every request.
 *
 * @param qstate: the qtstate that triggered the need to prime.
 * @param iq: iterator query state.
 * @param ie: iterator global state.
 * @param id: module id.
 * @param qname: request name.
 * @param qclass: the class to prime.
 * @return true if a priming subrequest was made, false if not. The will only
 *         issue a priming request if it detects an unprimed stub.
 */
static int
prime_stub(struct module_qstate* qstate, struct iter_qstate* iq, 
	struct iter_env* ie, int id, uint8_t* qname, uint16_t qclass)
{
	/* Lookup the stub hint. This will return null if the stub doesn't 
	 * need to be re-primed. */
	struct iter_hints_stub* stub = hints_lookup_stub(ie->hints, 
		qname, qclass, iq->dp);
	struct delegpt* stub_dp;
	struct module_qstate* subq;
	/* The stub (if there is one) does not need priming. */
	if(!stub)
		return 0;
	stub_dp = stub->dp;

	/* is it a noprime stub (always use) */
	if(stub->noprime) {
		iq->dp = delegpt_copy(stub_dp, qstate->region);
		if(!iq->dp) {
			log_err("out of memory priming stub");
			(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
			return 1; /* return 1 to make module stop, with error */
		}
		log_nametypeclass(VERB_DETAIL, "use stub", stub_dp->name, 
			LDNS_RR_TYPE_NS, qclass);
		return 0;
	}

	/* Otherwise, we need to (re)prime the stub. */
	log_nametypeclass(VERB_DETAIL, "priming stub", stub_dp->name, 
		LDNS_RR_TYPE_NS, qclass);

	/* Stub priming events start at the QUERYTARGETS state to avoid the
	 * redundant INIT state processing. */
	if(!generate_sub_request(stub_dp->name, stub_dp->namelen, 
		LDNS_RR_TYPE_NS, qclass, qstate, id, iq,
		QUERYTARGETS_STATE, PRIME_RESP_STATE, &subq, 0)) {
		log_err("out of memory priming stub");
		(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		return 1; /* return 1 to make module stop, with error */
	}
	if(subq) {
		struct iter_qstate* subiq = 
			(struct iter_qstate*)subq->minfo[id];

		/* Set the initial delegation point to the hint. */
		/* make copy to avoid use of stub dp by different qs/threads */
		subiq->dp = delegpt_copy(stub_dp, subq->region);
		if(!subiq->dp) {
			log_err("out of memory priming stub, copydp");
			fptr_ok(fptr_whitelist_modenv_kill_sub(
				qstate->env->kill_sub));
			(*qstate->env->kill_sub)(subq);
			(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
			return 1; /* return 1 to make module stop, with error */
		}
		/* there should not be any target queries -- although there 
		 * wouldn't be anyway, since stub hints never have 
		 * missing targets. */
		subiq->num_target_queries = 0; 
		subiq->wait_priming_stub = 1;
		subiq->dnssec_expected = iter_indicates_dnssec(
			qstate->env, subiq->dp, NULL, subq->qinfo.qclass);
	}
	
	/* this module stops, our submodule starts, and does the query. */
	qstate->ext_state[id] = module_wait_subquery;
	return 1;
}

/**
 * Generate A and AAAA checks for glue that is in-zone for the referral
 * we just got to obtain authoritative information on the adresses.
 *
 * @param qstate: the qtstate that triggered the need to prime.
 * @param iq: iterator query state.
 * @param id: module id.
 */
static void
generate_a_aaaa_check(struct module_qstate* qstate, struct iter_qstate* iq, 
	int id)
{
	struct module_qstate* subq;
	size_t i;
	struct reply_info* rep = iq->response->rep;
	struct ub_packed_rrset_key* s;
	log_assert(iq->dp);

	/* walk through additional, and check if in-zone,
	 * only relevant A, AAAA are left after scrub anyway */
	for(i=rep->an_numrrsets+rep->ns_numrrsets; i<rep->rrset_count; i++) {
		s = rep->rrsets[i];
		/* check *ALL* addresses that are transmitted in additional*/
		/* is it an address ? */
		if( !(ntohs(s->rk.type)==LDNS_RR_TYPE_A ||
			ntohs(s->rk.type)==LDNS_RR_TYPE_AAAA)) {
			continue;
		}
		/* is this query the same as the A/AAAA check for it */
		if(qstate->qinfo.qtype == ntohs(s->rk.type) &&
			qstate->qinfo.qclass == ntohs(s->rk.rrset_class) &&
			query_dname_compare(qstate->qinfo.qname, 
				s->rk.dname)==0 &&
			(qstate->query_flags&BIT_RD) && 
			!(qstate->query_flags&BIT_CD))
			continue;

		/* generate subrequest for it */
		log_nametypeclass(VERB_ALGO, "schedule addr fetch", 
			s->rk.dname, ntohs(s->rk.type), 
			ntohs(s->rk.rrset_class));
		if(!generate_sub_request(s->rk.dname, s->rk.dname_len, 
			ntohs(s->rk.type), ntohs(s->rk.rrset_class),
			qstate, id, iq,
			INIT_REQUEST_STATE, FINISHED_STATE, &subq, 1)) {
			log_err("out of memory generating ns check");
			return;
		}
		/* ignore subq - not need for more init */
	}
}

/**
 * Generate a NS check request to obtain authoritative information
 * on an NS rrset.
 *
 * @param qstate: the qtstate that triggered the need to prime.
 * @param iq: iterator query state.
 * @param id: module id.
 */
static void
generate_ns_check(struct module_qstate* qstate, struct iter_qstate* iq, int id)
{
	struct module_qstate* subq;
	log_assert(iq->dp);

	/* is this query the same as the nscheck? */
	if(qstate->qinfo.qtype == LDNS_RR_TYPE_NS &&
		query_dname_compare(iq->dp->name, qstate->qinfo.qname)==0 &&
		(qstate->query_flags&BIT_RD) && !(qstate->query_flags&BIT_CD)){
		/* spawn off A, AAAA queries for in-zone glue to check */
		generate_a_aaaa_check(qstate, iq, id);
		return;
	}

	log_nametypeclass(VERB_ALGO, "schedule ns fetch", 
		iq->dp->name, LDNS_RR_TYPE_NS, iq->qchase.qclass);
	if(!generate_sub_request(iq->dp->name, iq->dp->namelen, 
		LDNS_RR_TYPE_NS, iq->qchase.qclass, qstate, id, iq,
		INIT_REQUEST_STATE, FINISHED_STATE, &subq, 1)) {
		log_err("out of memory generating ns check");
		return;
	}
	if(subq) {
		struct iter_qstate* subiq = 
			(struct iter_qstate*)subq->minfo[id];

		/* make copy to avoid use of stub dp by different qs/threads */
		/* refetch glue to start higher up the tree */
		subiq->refetch_glue = 1;
		subiq->dp = delegpt_copy(iq->dp, subq->region);
		if(!subiq->dp) {
			log_err("out of memory generating ns check, copydp");
			fptr_ok(fptr_whitelist_modenv_kill_sub(
				qstate->env->kill_sub));
			(*qstate->env->kill_sub)(subq);
			return;
		}
	}
}

/**
 * See if the query needs forwarding.
 * 
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param ie: iterator shared global environment.
 * @return true if the request is forwarded, false if not.
 * 	If returns true but, iq->dp is NULL then a malloc failure occurred.
 */
static int
forward_request(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie)
{
	struct delegpt* dp = forwards_lookup(ie->fwds, iq->qchase.qname,
		iq->qchase.qclass);
	if(!dp) 
		return 0;
	/* send recursion desired to forward addr */
	iq->chase_flags |= BIT_RD; 
	iq->dp = delegpt_copy(dp, qstate->region);
	/* iq->dp checked by caller */
	verbose(VERB_ALGO, "forwarding request");
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
	uint8_t* delname;
	size_t delnamelen;
	struct dns_msg* msg;

	log_query_info(VERB_DETAIL, "resolving", &qstate->qinfo);
	/* check effort */

	/* We enforce a maximum number of query restarts. This is primarily a
	 * cheap way to prevent CNAME loops. */
	if(iq->query_restart_count > MAX_RESTART_COUNT) {
		verbose(VERB_QUERY, "request has exceeded the maximum number"
			" of query restarts with %d", iq->query_restart_count);
		return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
	}

	/* We enforce a maximum recursion/dependency depth -- in general, 
	 * this is unnecessary for dependency loops (although it will 
	 * catch those), but it provides a sensible limit to the amount 
	 * of work required to answer a given query. */
	verbose(VERB_ALGO, "request has dependency depth of %d", iq->depth);
	if(iq->depth > ie->max_dependency_depth) {
		verbose(VERB_QUERY, "request has exceeded the maximum "
			"dependency depth with depth of %d", iq->depth);
		return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
	}

	/* Resolver Algorithm Step 1 -- Look for the answer in local data. */

	/* This either results in a query restart (CNAME cache response), a
	 * terminating response (ANSWER), or a cache miss (null). */
	
	msg = dns_cache_lookup(qstate->env, iq->qchase.qname, 
		iq->qchase.qname_len, iq->qchase.qtype, 
		iq->qchase.qclass, qstate->region, qstate->env->scratch);
	if(!msg && qstate->env->neg_cache) {
		/* lookup in negative cache; may result in 
		 * NOERROR/NODATA or NXDOMAIN answers that need validation */
		msg = val_neg_getmsg(qstate->env->neg_cache, &iq->qchase,
			qstate->region, qstate->env->rrset_cache,
			qstate->env->scratch_buffer, *qstate->env->now);
	}
	if(msg) {
		/* handle positive cache response */
		enum response_type type = response_type_from_cache(msg, 
			&iq->qchase);
		if(verbosity >= VERB_ALGO)
			log_dns_msg("msg from cache lookup", &msg->qinfo, 
				msg->rep);

		if(type == RESPONSE_TYPE_CNAME) {
			uint8_t* sname = 0;
			size_t slen = 0;
			verbose(VERB_ALGO, "returning CNAME response from "
				"cache");
			if(!handle_cname_response(qstate, iq, msg, 
				&sname, &slen))
				return error_response(qstate, id, 
					LDNS_RCODE_SERVFAIL);
			iq->qchase.qname = sname;
			iq->qchase.qname_len = slen;
			/* This *is* a query restart, even if it is a cheap 
			 * one. */
			iq->dp = NULL;
			iq->refetch_glue = 0;
			iq->query_restart_count++;
			return next_state(iq, INIT_REQUEST_STATE);
		}

		/* it is an answer, response, to final state */
		verbose(VERB_ALGO, "returning answer from cache.");
		iq->response = msg;
		return final_state(iq);
	}
	
	/* attempt to forward the request */
	if(forward_request(qstate, iq, ie))
	{
		if(!iq->dp) {
			log_err("alloc failure for forward dp");
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		}
		iq->refetch_glue = 0;
		/* the request has been forwarded.
		 * forwarded requests need to be immediately sent to the 
		 * next state, QUERYTARGETS. */
		return next_state(iq, QUERYTARGETS_STATE);
	}

	/* Resolver Algorithm Step 2 -- find the "best" servers. */

	/* first, adjust for DS queries. To avoid the grandparent problem, 
	 * we just look for the closest set of server to the parent of qname.
	 * When re-fetching glue we also need to ask the parent.
	 */
	if(iq->refetch_glue) {
		if(!iq->dp) {
			log_err("internal or malloc fail: no dp for refetch");
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		}
		delname = iq->dp->name;
		delnamelen = iq->dp->namelen;
	} else {
		delname = iq->qchase.qname;
		delnamelen = iq->qchase.qname_len;
	}
	if((iq->qchase.qtype == LDNS_RR_TYPE_DS || iq->refetch_glue)
		&& !dname_is_root(delname)) {
		/* do not adjust root label, remove first label from delname */
		dname_remove_label(&delname, &delnamelen);
		iq->refetch_glue = 0; /* if CNAME causes restart, no refetch */
	}
	while(1) {
		
		/* Lookup the delegation in the cache. If null, then the 
		 * cache needs to be primed for the qclass. */
		iq->dp = dns_cache_find_delegation(qstate->env, delname, 
			delnamelen, iq->qchase.qtype, iq->qchase.qclass, 
			qstate->region, &iq->deleg_msg, *qstate->env->now);

		/* If the cache has returned nothing, then we have a 
		 * root priming situation. */
		if(iq->dp == NULL) {
			/* Note that the result of this will set a new
			 * DelegationPoint based on the result of priming. */
			if(!prime_root(qstate, iq, ie, id, iq->qchase.qclass))
				return error_response(qstate, id, 
					LDNS_RCODE_REFUSED);

			/* priming creates and sends a subordinate query, with 
			 * this query as the parent. So further processing for 
			 * this event will stop until reactivated by the 
			 * results of priming. */
			return 0;
		}

		/* see if this dp not useless.
		 * It is useless if:
		 *	o all NS items are required glue. 
		 *	  or the query is for NS item that is required glue.
		 *	o no addresses are provided.
		 *	o RD qflag is on.
		 * Instead, go up one level, and try to get even further
		 * If the root was useless, use safety belt information. 
		 * Only check cache returns, because replies for servers
		 * could be useless but lead to loops (bumping into the
		 * same server reply) if useless-checked.
		 */
		if(iter_dp_is_useless(&qstate->qinfo, qstate->query_flags, 
			iq->dp)) {
			if(dname_is_root(iq->dp->name)) {
				/* use safety belt */
				verbose(VERB_QUERY, "Cache has root NS but "
				"no addresses. Fallback to the safety belt.");
				iq->dp = hints_lookup_root(ie->hints, 
					iq->qchase.qclass);
				/* note deleg_msg is from previous lookup,
				 * but RD is on, so it is not used */
				if(!iq->dp) {
					log_err("internal error: no hints dp");
					return error_response(qstate, id, 
						LDNS_RCODE_REFUSED);
				}
				iq->dp = delegpt_copy(iq->dp, qstate->region);
				if(!iq->dp) {
					log_err("out of memory in safety belt");
					return error_response(qstate, id, 
						LDNS_RCODE_SERVFAIL);
				}
				break;
			} else {
				verbose(VERB_ALGO, 
					"cache delegation was useless:");
				delegpt_log(VERB_ALGO, iq->dp);
				/* go up */
				delname = iq->dp->name;
				delnamelen = iq->dp->namelen;
				dname_remove_label(&delname, &delnamelen);
			}
		} else break;
	}

	verbose(VERB_ALGO, "cache delegation returns delegpt");
	delegpt_log(VERB_ALGO, iq->dp);

	/* Otherwise, set the current delegation point and move on to the 
	 * next state. */
	return next_state(iq, INIT_REQUEST_2_STATE);
}

/** 
 * Process the second part of the initial request handling. This state
 * basically exists so that queries that generate root priming events have
 * the same init processing as ones that do not. Request events that reach
 * this state must have a valid currentDelegationPoint set.
 *
 * This part is primarly handling stub zone priming. Events that reach this
 * state must have a current delegation point.
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param ie: iterator shared global environment.
 * @param id: module id.
 * @return true if the event needs more request processing immediately,
 *         false if not.
 */
static int
processInitRequest2(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	log_query_info(VERB_QUERY, "resolving (init part 2): ", 
		&qstate->qinfo);

	/* Check to see if we need to prime a stub zone. */
	if(prime_stub(qstate, iq, ie, id, iq->qchase.qname, 
		iq->qchase.qclass)) {
		/* A priming sub request was made */
		return 0;
	}

	/* most events just get forwarded to the next state. */
	return next_state(iq, INIT_REQUEST_3_STATE);
}

/** 
 * Process the third part of the initial request handling. This state exists
 * as a separate state so that queries that generate stub priming events
 * will get the tail end of the init process but not repeat the stub priming
 * check.
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @return true, advancing the event to the QUERYTARGETS_STATE.
 */
static int
processInitRequest3(struct module_qstate* qstate, struct iter_qstate* iq)
{
	log_query_info(VERB_QUERY, "resolving (init part 3): ", 
		&qstate->qinfo);
	/* if the cache reply dp equals a validation anchor or msg has DS,
	 * then DNSSEC RRSIGs are expected in the reply */
	iq->dnssec_expected = iter_indicates_dnssec(qstate->env, iq->dp, 
		iq->deleg_msg, iq->qchase.qclass);

	/* If the RD flag wasn't set, then we just finish with the 
	 * cached referral as the response. */
	if(!(qstate->query_flags & BIT_RD)) {
		iq->response = iq->deleg_msg;
		if(verbosity >= VERB_ALGO)
			log_dns_msg("no RD requested, using delegation msg", 
				&iq->response->qinfo, iq->response->rep);
		return final_state(iq);
	}

	/* After this point, unset the RD flag -- this query is going to 
	 * be sent to an auth. server. */
	iq->chase_flags &= ~BIT_RD;

	/* Jump to the next state. */
	return next_state(iq, QUERYTARGETS_STATE);
}

/**
 * Given a basic query, generate a "target" query. These are subordinate
 * queries for missing delegation point target addresses.
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param id: module id.
 * @param name: target qname.
 * @param namelen: target qname length.
 * @param qtype: target qtype (either A or AAAA).
 * @param qclass: target qclass.
 * @return true on success, false on failure.
 */
static int
generate_target_query(struct module_qstate* qstate, struct iter_qstate* iq,
        int id, uint8_t* name, size_t namelen, uint16_t qtype, uint16_t qclass)
{
	struct module_qstate* subq;
	if(!generate_sub_request(name, namelen, qtype, qclass, qstate, 
		id, iq, INIT_REQUEST_STATE, FINISHED_STATE, &subq, 0))
		return 0;
	if(subq) {
		struct iter_qstate* subiq = 
			(struct iter_qstate*)subq->minfo[id];
		if(dname_subdomain_c(name, iq->dp->name)) {
			verbose(VERB_ALGO, "refetch of target glue");
			subiq->refetch_glue = 1;
			subiq->dp = delegpt_copy(iq->dp, subq->region);
			subiq->dnssec_expected = iter_indicates_dnssec(
				qstate->env, subiq->dp, NULL, 
				subq->qinfo.qclass);
		}
	}
	log_nametypeclass(VERB_QUERY, "new target", name, qtype, qclass);
	return 1;
}

/**
 * Given an event at a certain state, generate zero or more target queries
 * for it's current delegation point.
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param ie: iterator shared global environment.
 * @param id: module id.
 * @param maxtargets: The maximum number of targets to query for.
 *	if it is negative, there is no maximum number of targets.
 * @param num: returns the number of queries generated and processed, 
 *	which may be zero if there were no missing targets.
 * @return false on error.
 */
static int
query_for_targets(struct module_qstate* qstate, struct iter_qstate* iq,
        struct iter_env* ie, int id, int maxtargets, int* num)
{
	int query_count = 0;
	struct delegpt_ns* ns = iq->dp->nslist;
	int missing;
	int toget = 0;

	iter_mark_cycle_targets(qstate, iq->dp);
	missing = (int)delegpt_count_missing_targets(iq->dp);
	log_assert(maxtargets != 0); /* that would not be useful */

	/* Generate target requests. Basically, any missing targets 
	 * are queried for here, regardless if it is necessary to do 
	 * so to continue processing. */
	if(maxtargets < 0 || maxtargets > missing)
		toget = missing;
	else	toget = maxtargets;
	if(toget == 0) {
		*num = 0;
		return 1;
	}
	/* select 'toget' items from the total of 'missing' items */
	log_assert(toget <= missing);

	/* loop over missing targets */
	for(ns = iq->dp->nslist; ns; ns = ns->next) {
		if(ns->resolved)
			continue;

		/* randomly select this item with probability toget/missing */
		if(!iter_ns_probability(qstate->env->rnd, toget, missing)) {
			/* do not select this one, next; select toget number
			 * of items from a list one less in size */
			missing --;
			continue;
		}

		if(ie->supports_ipv6 && !ns->got6) {
			/* Send the AAAA request. */
			if(!generate_target_query(qstate, iq, id, 
				ns->name, ns->namelen,
				LDNS_RR_TYPE_AAAA, iq->qchase.qclass))
				return 0;
			query_count++;
		}
		/* Send the A request. */
		if(!ns->got4) {
			if(!generate_target_query(qstate, iq, id, 
				ns->name, ns->namelen, 
				LDNS_RR_TYPE_A, iq->qchase.qclass))
				return 0;
			query_count++;
		}

		/* mark this target as in progress. */
		ns->resolved = 1;
		missing--;
		toget--;
		if(toget == 0)
			break;
	}
	*num = query_count;
	if(query_count > 0)
		qstate->ext_state[id] = module_wait_subquery;

	return 1;
}

/** 
 * This is the request event state where the request will be sent to one of
 * its current query targets. This state also handles issuing target lookup
 * queries for missing target IP addresses. Queries typically iterate on
 * this state, both when they are just trying different targets for a given
 * delegation point, and when they change delegation points. This state
 * roughly corresponds to RFC 1034 algorithm steps 3 and 4.
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param ie: iterator shared global environment.
 * @param id: module id.
 * @return true if the event requires more request processing immediately,
 *         false if not. This state only returns true when it is generating
 *         a SERVFAIL response because the query has hit a dead end.
 */
static int
processQueryTargets(struct module_qstate* qstate, struct iter_qstate* iq,
	struct iter_env* ie, int id)
{
	int tf_policy;
	struct delegpt_addr* target;
	struct outbound_entry* outq;

	/* NOTE: a request will encounter this state for each target it 
	 * needs to send a query to. That is, at least one per referral, 
	 * more if some targets timeout or return throwaway answers. */

	log_query_info(VERB_QUERY, "processQueryTargets:", &qstate->qinfo);
	verbose(VERB_ALGO, "processQueryTargets: targetqueries %d, "
		"currentqueries %d", iq->num_target_queries, 
		iq->num_current_queries);
	qstate->ext_state[id] = module_wait_reply;

	/* Make sure that we haven't run away */
	/* FIXME: is this check even necessary? */
	if(iq->referral_count > MAX_REFERRAL_COUNT) {
		verbose(VERB_QUERY, "request has exceeded the maximum "
			"number of referrrals with %d", iq->referral_count);
		return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
	}
	
	/* Make sure we have a delegation point, otherwise priming failed
	 * or another failure occurred */
	if(!iq->dp) {
		verbose(VERB_QUERY, "Failed to get a delegation, giving up");
		return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
	}
	delegpt_log(VERB_ALGO, iq->dp);

	if(iq->num_current_queries>0) {
		/* already busy answering a query, this restart is because
		 * more delegpt addrs became available, wait for existing
		 * query. */
		verbose(VERB_ALGO, "woke up, but wait for outstanding query");
		return 0;
	}

	tf_policy = 0;
	if(iq->depth <= ie->max_dependency_depth) {
		tf_policy = ie->target_fetch_policy[iq->depth];
	}

	/* if in 0x20 fallback get as many targets as possible */
	if(iq->caps_fallback) {
		int extra = 0;
		size_t naddr, nres, navail;
		if(!query_for_targets(qstate, iq, ie, id, -1, &extra)) {
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		}
		iq->num_target_queries += extra;
		if(iq->num_target_queries > 0) {
			/* wait to get all targets, we want to try em */
			verbose(VERB_ALGO, "wait for all targets for fallback");
			return 0;
		}
		/* did we do enough fallback queries already? */
		delegpt_count_addr(iq->dp, &naddr, &nres, &navail);
		/* the current caps_server is the number of fallbacks sent.
		 * the original query is one that matched too, so we have
		 * caps_server+1 number of matching queries now */
		if(iq->caps_server+1 >= naddr*3) {
			/* we're done, process the response */
			verbose(VERB_ALGO, "0x20 fallback had %d responses "
				"match for %d wanted, done.", 
				(int)iq->caps_server+1, (int)naddr*3);
			iq->caps_fallback = 0;
			iq->state = QUERY_RESP_STATE;
			return 1;
		}
		verbose(VERB_ALGO, "0x20 fallback number %d", 
			(int)iq->caps_server);

	/* if there is a policy to fetch missing targets 
	 * opportunistically, do it. we rely on the fact that once a 
	 * query (or queries) for a missing name have been issued, 
	 * they will not be show up again. */
	} else if(tf_policy != 0) {
		int extra = 0;
		verbose(VERB_ALGO, "attempt to get extra %d targets", 
			tf_policy);
		if(!query_for_targets(qstate, iq, ie, id, tf_policy, &extra)) {
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		}
		iq->num_target_queries += extra;
	}

	/* Add the current set of unused targets to our queue. */
	delegpt_add_unused_targets(iq->dp);

	/* Select the next usable target, filtering out unsuitable targets. */
	target = iter_server_selection(ie, qstate->env, iq->dp, 
		iq->dp->name, iq->dp->namelen, iq->qchase.qtype,
		&iq->dnssec_expected, &iq->chase_to_rd);

	/* If no usable target was selected... */
	if(!target) {
		/* Here we distinguish between three states: generate a new 
		 * target query, just wait, or quit (with a SERVFAIL).
		 * We have the following information: number of active 
		 * target queries, number of active current queries, 
		 * the presence of missing targets at this delegation 
		 * point, and the given query target policy. */
		
		/* Check for the wait condition. If this is true, then 
		 * an action must be taken. */
		if(iq->num_target_queries==0 && iq->num_current_queries==0) {
			/* If there is nothing to wait for, then we need 
			 * to distinguish between generating (a) new target 
			 * query, or failing. */
			if(delegpt_count_missing_targets(iq->dp) > 0) {
				int qs = 0;
				verbose(VERB_ALGO, "querying for next "
					"missing target");
				if(!query_for_targets(qstate, iq, ie, id, 
					1, &qs)) {
					return error_response(qstate, id,
						LDNS_RCODE_SERVFAIL);
				}
				iq->num_target_queries += qs;
			}
			/* Since a target query might have been made, we 
			 * need to check again. */
			if(iq->num_target_queries == 0) {
				verbose(VERB_QUERY, "out of query targets -- "
					"returning SERVFAIL");
				/* fail -- no more targets, no more hope 
				 * of targets, no hope of a response. */
				return error_response(qstate, id,
					LDNS_RCODE_SERVFAIL);
			}
		}

		/* otherwise, we have no current targets, so submerge 
		 * until one of the target or direct queries return. */
		if(iq->num_target_queries>0 && iq->num_current_queries>0)
			verbose(VERB_ALGO, "no current targets -- waiting "
				"for %d targets to resolve or %d outstanding"
				" queries to respond", iq->num_target_queries, 
				iq->num_current_queries);
		else if(iq->num_target_queries>0)
			verbose(VERB_ALGO, "no current targets -- waiting "
				"for %d targets to resolve.",
				iq->num_target_queries);
		else 	verbose(VERB_ALGO, "no current targets -- waiting "
				"for %d outstanding queries to respond.",
				iq->num_current_queries);
		return 0;
	}

	/* We have a valid target. */
	log_query_info(VERB_QUERY, "sending query:", &iq->qchase);
	log_name_addr(VERB_QUERY, "sending to target:", iq->dp->name, 
		&target->addr, target->addrlen);
	fptr_ok(fptr_whitelist_modenv_send_query(qstate->env->send_query));
	outq = (*qstate->env->send_query)(
		iq->qchase.qname, iq->qchase.qname_len, 
		iq->qchase.qtype, iq->qchase.qclass, 
		iq->chase_flags | (iq->chase_to_rd?BIT_RD:0), EDNS_DO|BIT_CD, 
		&target->addr, target->addrlen, qstate);
	if(!outq) {
		verbose(VERB_OPS, "error sending query to auth server; "
			"skip this address");
		log_addr(VERB_OPS, "error for address:", 
			&target->addr, target->addrlen);
		return next_state(iq, QUERYTARGETS_STATE);
	}
	outbound_list_insert(&iq->outlist, outq);
	iq->num_current_queries++;
	qstate->ext_state[id] = module_wait_reply;

	return 0;
}

/** find NS rrset in given list */
static struct ub_packed_rrset_key*
find_NS(struct reply_info* rep, size_t from, size_t to)
{
	size_t i;
	for(i=from; i<to; i++) {
		if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_NS)
			return rep->rrsets[i];
	}
	return NULL;
}


/** 
 * Process the query response. All queries end up at this state first. This
 * process generally consists of analyzing the response and routing the
 * event to the next state (either bouncing it back to a request state, or
 * terminating the processing for this event).
 * 
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param id: module id.
 * @return true if the event requires more immediate processing, false if
 *         not. This is generally only true when forwarding the request to
 *         the final state (i.e., on answer).
 */
static int
processQueryResponse(struct module_qstate* qstate, struct iter_qstate* iq,
	int id)
{
	int dnsseclame = 0;
	enum response_type type;
	iq->num_current_queries--;
	if(iq->response == NULL) {
		iq->chase_to_rd = 0;
		verbose(VERB_ALGO, "query response was timeout");
		return next_state(iq, QUERYTARGETS_STATE);
	}
	type = response_type_from_server(
		(int)((iq->chase_flags&BIT_RD) || iq->chase_to_rd),
		iq->response, &iq->qchase, iq->dp);
	iq->chase_to_rd = 0;
	if(type == RESPONSE_TYPE_REFERRAL && (iq->chase_flags&BIT_RD)) {
		/* When forwarding (RD bit is set), we handle referrals 
		 * differently. No queries should be sent elsewhere */
		type = RESPONSE_TYPE_ANSWER;
	}
	if(iq->dnssec_expected && !(iq->chase_flags&BIT_RD) 
		&& type != RESPONSE_TYPE_LAME 
		&& type != RESPONSE_TYPE_REC_LAME 
		&& type != RESPONSE_TYPE_THROWAWAY 
		&& type != RESPONSE_TYPE_UNTYPED) {
		/* a possible answer, see if it is missing DNSSEC */
		/* but not when forwarding, so we dont mark fwder lame */
		/* also make sure the answer is from the zone we expected,
		 * otherwise, (due to parent,child on same server), we
		 * might mark the server,zone lame inappropriately */
		if(!iter_msg_has_dnssec(iq->response) &&
			iter_msg_from_zone(iq->response, iq->dp, type,
				iq->qchase.qclass)) {
			type = RESPONSE_TYPE_LAME;
			dnsseclame = 1;
		}
	}
	/* see if referral brings us close to the target */
	if(type == RESPONSE_TYPE_REFERRAL) {
		struct ub_packed_rrset_key* ns = find_NS(
			iq->response->rep, iq->response->rep->an_numrrsets,
			iq->response->rep->an_numrrsets 
			+ iq->response->rep->ns_numrrsets);
		if(!ns) ns = find_NS(iq->response->rep, 0, 
				iq->response->rep->an_numrrsets);
		if(!ns || !dname_strict_subdomain_c(ns->rk.dname, iq->dp->name) 
			|| !dname_subdomain_c(iq->qchase.qname, ns->rk.dname)){
			verbose(VERB_ALGO, "bad referral, throwaway");
			if(!ns) log_info("no ns");
			log_query_info(0, "qchase", &iq->qchase);
			log_nametypeclass(0, "dp", iq->dp->name, 0, 0);
			log_nametypeclass(0, "ns", ns->rk.dname, 0, 0);
			type = RESPONSE_TYPE_THROWAWAY;
		}
	}

	/* handle each of the type cases */
	if(type == RESPONSE_TYPE_ANSWER) {
		/* ANSWER type responses terminate the query algorithm, 
		 * so they sent on their */
		verbose(VERB_DETAIL, "query response was ANSWER");
		if(!iter_dns_store(qstate->env, &iq->response->qinfo,
			iq->response->rep, 0))
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		/* close down outstanding requests to be discarded */
		outbound_list_clear(&iq->outlist);
		iq->num_current_queries = 0;
		fptr_ok(fptr_whitelist_modenv_detach_subs(
			qstate->env->detach_subs));
		(*qstate->env->detach_subs)(qstate);
		iq->num_target_queries = 0;
		return final_state(iq);
	} else if(type == RESPONSE_TYPE_REFERRAL) {
		/* REFERRAL type responses get a reset of the 
		 * delegation point, and back to the QUERYTARGETS_STATE. */
		verbose(VERB_DETAIL, "query response was REFERRAL");

		/* if hardened, only store referral if we asked for it */
		if(!qstate->env->cfg->harden_referral_path ||
		    (  qstate->qinfo.qtype == LDNS_RR_TYPE_NS 
			&& (qstate->query_flags&BIT_RD) 
			&& !(qstate->query_flags&BIT_CD)
			   /* we know that all other NS rrsets are scrubbed
			    * away, thus on referral only one is left.
			    * see if that equals the query name... */
			&& reply_find_rrset_section_ns(iq->response->rep,
				qstate->qinfo.qname, qstate->qinfo.qname_len,
				LDNS_RR_TYPE_NS, qstate->qinfo.qclass)
		    )) {
			/* Store the referral under the current query */
			if(!iter_dns_store(qstate->env, &iq->response->qinfo,
				iq->response->rep, 1))
				return error_response(qstate, id, 
					LDNS_RCODE_SERVFAIL);
			if(qstate->env->neg_cache)
				val_neg_addreferral(qstate->env->neg_cache, 
					iq->response->rep, iq->dp->name);
		}

		/* Reset the event state, setting the current delegation 
		 * point to the referral. */
		iq->deleg_msg = iq->response;
		iq->dp = delegpt_from_message(iq->response, qstate->region);
		if(!iq->dp)
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		if(!cache_fill_missing(qstate->env, iq->qchase.qclass, 
			qstate->region, iq->dp))
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		delegpt_log(VERB_ALGO, iq->dp);
		/* Count this as a referral. */
		iq->referral_count++;
		/* see if the next dp is a trust anchor, or a DS was sent
		 * along, indicating dnssec is expected for next zone */
		iq->dnssec_expected = iter_indicates_dnssec(qstate->env, 
			iq->dp, iq->response, iq->qchase.qclass);

		/* spawn off NS and addr to auth servers for the NS we just
		 * got in the referral. This gets authoritative answer
		 * (answer section trust level) rrset. 
		 * right after, we detach the subs, answer goes to cache. */
		if(qstate->env->cfg->harden_referral_path)
			generate_ns_check(qstate, iq, id);

		/* stop current outstanding queries. 
		 * FIXME: should the outstanding queries be waited for and
		 * handled? Say by a subquery that inherits the outbound_entry.
		 */
		outbound_list_clear(&iq->outlist);
		iq->num_current_queries = 0;
		fptr_ok(fptr_whitelist_modenv_detach_subs(
			qstate->env->detach_subs));
		(*qstate->env->detach_subs)(qstate);
		iq->num_target_queries = 0;
		verbose(VERB_ALGO, "cleared outbound list for next round");
		return next_state(iq, QUERYTARGETS_STATE);
	} else if(type == RESPONSE_TYPE_CNAME) {
		uint8_t* sname = NULL;
		size_t snamelen = 0;
		/* CNAME type responses get a query restart (i.e., get a 
		 * reset of the query state and go back to INIT_REQUEST_STATE).
		 */
		verbose(VERB_DETAIL, "query response was CNAME");
		if(verbosity >= VERB_ALGO)
			log_dns_msg("cname msg", &iq->response->qinfo, 
				iq->response->rep);
		/* Process the CNAME response. */
		if(!handle_cname_response(qstate, iq, iq->response, 
			&sname, &snamelen))
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		/* cache the CNAME response under the current query */
		/* NOTE : set referral=1, so that rrsets get stored but not 
		 * the partial query answer (CNAME only). */
		if(!iter_dns_store(qstate->env, &iq->response->qinfo,
			iq->response->rep, 1))
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		/* set the current request's qname to the new value. */
		iq->qchase.qname = sname;
		iq->qchase.qname_len = snamelen;
		/* Clear the query state, since this is a query restart. */
		iq->deleg_msg = NULL;
		iq->dp = NULL;
		iq->dnssec_expected = 0;
		/* Note the query restart. */
		iq->query_restart_count++;

		/* stop current outstanding queries. 
		 * FIXME: should the outstanding queries be waited for and
		 * handled? Say by a subquery that inherits the outbound_entry.
		 */
		outbound_list_clear(&iq->outlist);
		iq->num_current_queries = 0;
		fptr_ok(fptr_whitelist_modenv_detach_subs(
			qstate->env->detach_subs));
		(*qstate->env->detach_subs)(qstate);
		iq->num_target_queries = 0;
		verbose(VERB_ALGO, "cleared outbound list for query restart");
		/* go to INIT_REQUEST_STATE for new qname. */
		return next_state(iq, INIT_REQUEST_STATE);
	} else if(type == RESPONSE_TYPE_LAME) {
		/* Cache the LAMEness. */
		verbose(VERB_DETAIL, "query response was %sLAME",
			dnsseclame?"DNSSEC ":"");
		if(!dname_subdomain_c(iq->qchase.qname, iq->dp->name)) {
			log_err("mark lame: mismatch in qname and dpname");
			/* throwaway this reply below */
		} else if(qstate->reply) {
			/* need addr for lameness cache, but we may have
			 * gotten this from cache, so test to be sure */
			if(!infra_set_lame(qstate->env->infra_cache, 
				&qstate->reply->addr, qstate->reply->addrlen, 
				iq->dp->name, iq->dp->namelen, 
				*qstate->env->now, dnsseclame, 0,
				iq->qchase.qtype))
				log_err("mark host lame: out of memory");
		} else log_err("%slame response from cache",
			dnsseclame?"DNSSEC ":"");
	} else if(type == RESPONSE_TYPE_REC_LAME) {
		/* Cache the LAMEness. */
		verbose(VERB_DETAIL, "query response REC_LAME: "
			"recursive but not authoritative server");
		if(!dname_subdomain_c(iq->qchase.qname, iq->dp->name)) {
			log_err("mark rec_lame: mismatch in qname and dpname");
			/* throwaway this reply below */
		} else if(qstate->reply) {
			/* need addr for lameness cache, but we may have
			 * gotten this from cache, so test to be sure */
			verbose(VERB_DETAIL, "mark as REC_LAME");
			if(!infra_set_lame(qstate->env->infra_cache, 
				&qstate->reply->addr, qstate->reply->addrlen, 
				iq->dp->name, iq->dp->namelen, 
				*qstate->env->now, 0, 1, iq->qchase.qtype))
				log_err("mark host lame: out of memory");
		} 
	} else if(type == RESPONSE_TYPE_THROWAWAY) {
		/* LAME and THROWAWAY responses are handled the same way. 
		 * In this case, the event is just sent directly back to 
		 * the QUERYTARGETS_STATE without resetting anything, 
		 * because, clearly, the next target must be tried. */
		verbose(VERB_DETAIL, "query response was THROWAWAY");
	} else {
		log_warn("A query response came back with an unknown type: %d",
			(int)type);
	}

	/* LAME, THROWAWAY and "unknown" all end up here.
	 * Recycle to the QUERYTARGETS state to hopefully try a 
	 * different target. */
	return next_state(iq, QUERYTARGETS_STATE);
}

/**
 * Return priming query results to interestes super querystates.
 * 
 * Sets the delegation point and delegation message (not nonRD queries).
 * This is a callback from walk_supers.
 *
 * @param qstate: priming query state that finished.
 * @param id: module id.
 * @param forq: the qstate for which priming has been done.
 */
static void
prime_supers(struct module_qstate* qstate, int id, struct module_qstate* forq)
{
	struct iter_qstate* foriq = (struct iter_qstate*)forq->minfo[id];
	struct delegpt* dp = NULL;

	log_assert(qstate->is_priming || foriq->wait_priming_stub);
	log_assert(qstate->return_rcode == LDNS_RCODE_NOERROR);
	/* Convert our response to a delegation point */
	dp = delegpt_from_message(qstate->return_msg, forq->region);
	if(!dp) {
		/* if there is no convertable delegation point, then 
		 * the ANSWER type was (presumably) a negative answer. */
		verbose(VERB_ALGO, "prime response was not a positive "
			"ANSWER; failing");
		foriq->dp = NULL;
		foriq->state = QUERYTARGETS_STATE;
		return;
	}

	log_query_info(VERB_DETAIL, "priming successful for", &qstate->qinfo);
	delegpt_log(VERB_ALGO, dp);
	foriq->dp = dp;
	foriq->deleg_msg = dns_copy_msg(qstate->return_msg, forq->region);
	if(!foriq->deleg_msg) {
		log_err("copy prime response: out of memory");
		foriq->dp = NULL;
		foriq->state = QUERYTARGETS_STATE;
		return;
	}

	/* root priming responses go to init stage 2, priming stub 
	 * responses to to stage 3. */
	if(foriq->wait_priming_stub) {
		foriq->state = INIT_REQUEST_3_STATE;
		foriq->wait_priming_stub = 0;
	} else	foriq->state = INIT_REQUEST_2_STATE;
	/* because we are finished, the parent will be reactivated */
}

/** 
 * This handles the response to a priming query. This is used to handle both
 * root and stub priming responses. This is basically the equivalent of the
 * QUERY_RESP_STATE, but will not handle CNAME responses and will treat
 * REFERRALs as ANSWERS. It will also update and reactivate the originating
 * event.
 *
 * @param qstate: query state.
 * @param id: module id.
 * @return true if the event needs more immediate processing, false if not.
 *         This state always returns false.
 */
static int
processPrimeResponse(struct module_qstate* qstate, int id)
{
	struct iter_qstate* iq = (struct iter_qstate*)qstate->minfo[id];
	enum response_type type = response_type_from_server(0, iq->response, 
		&iq->qchase, iq->dp);
	if(type == RESPONSE_TYPE_ANSWER) {
		qstate->return_rcode = LDNS_RCODE_NOERROR;
		qstate->return_msg = iq->response;
	} else {
		qstate->return_rcode = LDNS_RCODE_SERVFAIL;
		qstate->return_msg = NULL;
	}

	/* validate the root or stub after priming (if enabled).
	 * This is the same query as the prime query, but with validation.
	 * Now that we are primed, the additional queries that validation
	 * may need can be resolved, such as DLV. */
	if(qstate->env->cfg->harden_referral_path) {
		struct module_qstate* subq = NULL;
		log_nametypeclass(VERB_ALGO, "schedule prime validation", 
			qstate->qinfo.qname, qstate->qinfo.qtype,
			qstate->qinfo.qclass);
		if(!generate_sub_request(qstate->qinfo.qname, 
			qstate->qinfo.qname_len, qstate->qinfo.qtype,
			qstate->qinfo.qclass, qstate, id, iq,
			INIT_REQUEST_STATE, FINISHED_STATE, &subq, 1)) {
			log_err("out of memory generating prime check");
		}
		generate_a_aaaa_check(qstate, iq, id);
	}

	/* This event is finished. */
	qstate->ext_state[id] = module_finished;
	return 0;
}

/** 
 * Do final processing on responses to target queries. Events reach this
 * state after the iterative resolution algorithm terminates. This state is
 * responsible for reactiving the original event, and housekeeping related
 * to received target responses (caching, updating the current delegation
 * point, etc).
 * Callback from walk_supers for every super state that is interested in 
 * the results from this query.
 *
 * @param qstate: query state.
 * @param id: module id.
 * @param forq: super query state.
 */
static void
processTargetResponse(struct module_qstate* qstate, int id,
	struct module_qstate* forq)
{
	struct iter_qstate* iq = (struct iter_qstate*)qstate->minfo[id];
	struct iter_qstate* foriq = (struct iter_qstate*)forq->minfo[id];
	struct ub_packed_rrset_key* rrset;
	struct delegpt_ns* dpns;
	log_assert(qstate->return_rcode == LDNS_RCODE_NOERROR);

	foriq->state = QUERYTARGETS_STATE;
	log_query_info(VERB_ALGO, "processTargetResponse", &qstate->qinfo);
	log_query_info(VERB_ALGO, "processTargetResponse super", &forq->qinfo);

	/* check to see if parent event is still interested (in orig name).  */
	dpns = delegpt_find_ns(foriq->dp, qstate->qinfo.qname,
			qstate->qinfo.qname_len);
	if(!dpns) {
		/* FIXME: maybe store this nameserver address in the cache
		 * anyways? */
		/* If not, just stop processing this event */
		verbose(VERB_ALGO, "subq: parent not interested anymore");
		/* this is an error, and will cause parent to be reactivated
		 * even though nothing has happened */
		log_assert(0);
		return;
	}

	/* Tell the originating event that this target query has finished
	 * (regardless if it succeeded or not). */
	foriq->num_target_queries--;

	/* This response is relevant to the current query, so we 
	 * add (attempt to add, anyway) this target(s) and reactivate 
	 * the original event. 
	 * NOTE: we could only look for the AnswerRRset if the 
	 * response type was ANSWER. */
	rrset = reply_find_answer_rrset(&iq->qchase, qstate->return_msg->rep);
	if(rrset) {
		/* if CNAMEs have been followed - add new NS to delegpt. */
		/* BTW. RFC 1918 says NS should not have got CNAMEs. Robust. */
		if(!delegpt_find_ns(foriq->dp, rrset->rk.dname, 
			rrset->rk.dname_len)) {
			if(!delegpt_add_ns(foriq->dp, forq->region, 
				rrset->rk.dname))
				log_err("out of memory adding cnamed-ns");
		}
		if(!delegpt_add_rrset(foriq->dp, forq->region, rrset))
			log_err("out of memory adding targets");
		verbose(VERB_ALGO, "added target response");
		delegpt_log(VERB_ALGO, foriq->dp);
	} else {
		verbose(VERB_ALGO, "iterator TargetResponse failed");
		dpns->resolved = 1; /* fail the target */
	}
}

/** 
 * This handles the final state for first-tier responses (i.e., responses to
 * externally generated queries).
 *
 * @param qstate: query state.
 * @param iq: iterator query state.
 * @param id: module id.
 * @return true if the event needs more processing, false if not. Since this
 *         is the final state for an event, it always returns false.
 */
static int
processFinished(struct module_qstate* qstate, struct iter_qstate* iq,
	int id)
{
	log_query_info(VERB_QUERY, "finishing processing for", 
		&qstate->qinfo);

	if(!iq->response) {
		verbose(VERB_ALGO, "No response is set, servfail");
		return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
	}

	/* Make sure that the RA flag is set (since the presence of 
	 * this module means that recursion is available) */
	iq->response->rep->flags |= BIT_RA;

	/* Clear the AA flag */
	/* FIXME: does this action go here or in some other module? */
	iq->response->rep->flags &= ~BIT_AA;

	/* make sure QR flag is on */
	iq->response->rep->flags |= BIT_QR;

	/* we have finished processing this query */
	qstate->ext_state[id] = module_finished;

	/* TODO:  we are using a private TTL, trim the response. */
	/* if (mPrivateTTL > 0){IterUtils.setPrivateTTL(resp, mPrivateTTL); } */

	/* prepend any items we have accumulated */
	if(iq->an_prepend_list || iq->ns_prepend_list) {
		if(!iter_prepend(iq, iq->response, qstate->region)) {
			log_err("prepend rrsets: out of memory");
			return error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		}
		/* reset the query name back */
		iq->response->qinfo = qstate->qinfo;
		/* store message with the finished prepended items,
		 * but only if we did recursion. The nonrecursion referral
		 * from cache does not need to be stored in the msg cache. */
		if(qstate->query_flags&BIT_RD) {
			if(!iter_dns_store(qstate->env, &qstate->qinfo, 
				iq->response->rep, 0))
				return error_response(qstate, id, 
					LDNS_RCODE_SERVFAIL);
		}
	}
	qstate->return_rcode = LDNS_RCODE_NOERROR;
	qstate->return_msg = iq->response;
	return 0;
}

/*
 * Return priming query results to interestes super querystates.
 * 
 * Sets the delegation point and delegation message (not nonRD queries).
 * This is a callback from walk_supers.
 *
 * @param qstate: query state that finished.
 * @param id: module id.
 * @param super: the qstate to inform.
 */
void
iter_inform_super(struct module_qstate* qstate, int id, 
	struct module_qstate* super)
{
	if(qstate->return_rcode != LDNS_RCODE_NOERROR)
		error_supers(qstate, id, super);
	else if(qstate->is_priming)
		prime_supers(qstate, id, super);
	else	processTargetResponse(qstate, id, super);
}

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
			case INIT_REQUEST_2_STATE:
				cont = processInitRequest2(qstate, iq, ie, id);
				break;
			case INIT_REQUEST_3_STATE:
				cont = processInitRequest3(qstate, iq);
				break;
			case QUERYTARGETS_STATE:
				cont = processQueryTargets(qstate, iq, ie, id);
				break;
			case QUERY_RESP_STATE:
				cont = processQueryResponse(qstate, iq, id);
				break;
			case PRIME_RESP_STATE:
				cont = processPrimeResponse(qstate, id);
				break;
			case FINISHED_STATE:
				cont = processFinished(qstate, iq, id);
				break;
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
	struct iter_env* ie, int id, struct outbound_entry* outbound,
	enum module_ev event)
{
	struct msg_parse* prs;
	struct edns_data edns;
	ldns_buffer* pkt;

	verbose(VERB_ALGO, "process_response: new external response event");
	iq->response = NULL;
	iq->state = QUERY_RESP_STATE;
	if(event == module_event_noreply || event == module_event_error) {
		goto handle_it;
	}
	if( (event != module_event_reply && event != module_event_capsfail)
		|| !qstate->reply) {
		log_err("Bad event combined with response");
		outbound_list_remove(&iq->outlist, outbound);
		(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		return;
	}

	/* parse message */
	prs = (struct msg_parse*)regional_alloc(qstate->env->scratch, 
		sizeof(struct msg_parse));
	if(!prs) {
		log_err("out of memory on incoming message");
		/* like packet got dropped */
		goto handle_it;
	}
	memset(prs, 0, sizeof(*prs));
	memset(&edns, 0, sizeof(edns));
	pkt = qstate->reply->c->buffer;
	ldns_buffer_set_position(pkt, 0);
	if(parse_packet(pkt, prs, qstate->env->scratch) != LDNS_RCODE_NOERROR) {
		verbose(VERB_ALGO, "parse error on reply packet");
		goto handle_it;
	}
	/* edns is not examined, but removed from message to help cache */
	if(parse_extract_edns(prs, &edns) != LDNS_RCODE_NOERROR)
		goto handle_it;
	/* remove CD-bit, we asked for in case we handle validation ourself */
	prs->flags &= ~BIT_CD;

	/* normalize and sanitize: easy to delete items from linked lists */
	if(!scrub_message(pkt, prs, &iq->qchase, iq->dp->name, 
		qstate->env->scratch, qstate->env, ie))
		goto handle_it;

	/* allocate response dns_msg in region */
	iq->response = dns_alloc_msg(pkt, prs, qstate->region);
	if(!iq->response)
		goto handle_it;
	log_query_info(VERB_DETAIL, "response for", &qstate->qinfo);
	log_name_addr(VERB_DETAIL, "reply from", iq->dp->name, 
		&qstate->reply->addr, qstate->reply->addrlen);
	if(verbosity >= VERB_ALGO)
		log_dns_msg("incoming scrubbed packet:", &iq->response->qinfo, 
			iq->response->rep);
	
	if(event == module_event_capsfail) {
		if(!iq->caps_fallback) {
			/* start fallback */
			iq->caps_fallback = 1;
			iq->caps_server = 0;
			iq->caps_reply = iq->response->rep;
			iq->state = QUERYTARGETS_STATE;
			iq->num_current_queries--;
			verbose(VERB_DETAIL, "Capsforid: starting fallback");
			goto handle_it;
		} else {
			/* check if reply is the same, otherwise, fail */
			if(!reply_equal(iq->response->rep, iq->caps_reply)) {
				verbose(VERB_DETAIL, "Capsforid fallback: "
					"getting different replies, failed");
				outbound_list_remove(&iq->outlist, outbound);
				(void)error_response(qstate, id, 
					LDNS_RCODE_SERVFAIL);
				return;
			}
			/* continue the fallback procedure at next server */
			iq->caps_server++;
			iq->state = QUERYTARGETS_STATE;
			iq->num_current_queries--;
			verbose(VERB_DETAIL, "Capsforid: reply is equal. "
				"go to next fallback");
			goto handle_it;
		}
	}
	iq->caps_fallback = 0; /* if we were in fallback, 0x20 is OK now */

handle_it:
	outbound_list_remove(&iq->outlist, outbound);
	iter_handle(qstate, iq, ie, id);
}

void 
iter_operate(struct module_qstate* qstate, enum module_ev event, int id,
	struct outbound_entry* outbound)
{
	struct iter_env* ie = (struct iter_env*)qstate->env->modinfo[id];
	struct iter_qstate* iq = (struct iter_qstate*)qstate->minfo[id];
	verbose(VERB_QUERY, "iterator[module %d] operate: extstate:%s event:%s", 
		id, strextstate(qstate->ext_state[id]), strmodulevent(event));
	if(iq) log_query_info(VERB_QUERY, "iterator operate: query", 
		&qstate->qinfo);
	if(iq && qstate->qinfo.qname != iq->qchase.qname)
		log_query_info(VERB_QUERY, "iterator operate: chased to", 
			&iq->qchase);

	/* perform iterator state machine */
	if((event == module_event_new || event == module_event_pass) && 
		iq == NULL) {
		if(!iter_new(qstate, id)) {
			(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
			return;
		}
		iq = (struct iter_qstate*)qstate->minfo[id];
		process_request(qstate, iq, ie, id);
		return;
	}
	if(iq && event == module_event_pass) {
		iter_handle(qstate, iq, ie, id);
		return;
	}
	if(iq && outbound) {
		process_response(qstate, iq, ie, id, outbound, event);
		return;
	}
	if(event == module_event_error) {
		verbose(VERB_ALGO, "got called with event error, giving up");
		(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
		return;
	}

	log_err("bad event for iterator");
	(void)error_response(qstate, id, LDNS_RCODE_SERVFAIL);
}

void 
iter_clear(struct module_qstate* qstate, int id)
{
	struct iter_qstate* iq;
	if(!qstate)
		return;
	iq = (struct iter_qstate*)qstate->minfo[id];
	if(iq) {
		outbound_list_clear(&iq->outlist);
		iq->num_current_queries = 0;
	}
	qstate->minfo[id] = NULL;
}

size_t 
iter_get_mem(struct module_env* env, int id)
{
	struct iter_env* ie = (struct iter_env*)env->modinfo[id];
	if(!ie)
		return 0;
	return sizeof(*ie) + sizeof(int)*((size_t)ie->max_dependency_depth+1)
		+ hints_get_mem(ie->hints) + forwards_get_mem(ie->fwds)
		+ donotq_get_mem(ie->donotq);
}

/**
 * The iterator function block 
 */
static struct module_func_block iter_block = {
	"iterator",
	&iter_init, &iter_deinit, &iter_operate, &iter_inform_super, 
	&iter_clear, &iter_get_mem
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
