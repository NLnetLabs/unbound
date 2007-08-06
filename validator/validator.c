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
#include "validator/val_kentry.h"
#include "validator/val_utils.h"
#include "services/cache/dns.h"
#include "util/data/dname.h"
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
 * Exit validation with an error status
 * 
 * @param qstate: query state
 * @param id: validator id.
 * @return false, for use by caller to return to stop processing.
 */
static int
val_error(struct module_qstate* qstate, int id)
{
	qstate->ext_state[id] = module_error;
	qstate->return_rcode = LDNS_RCODE_SERVFAIL;
	return 0;
}

/** 
 * Check to see if a given response needs to go through the validation
 * process. Typical reasons for this routine to return false are: CD bit was
 * on in the original request, the response was already validated, or the
 * response is a kind of message that is unvalidatable (i.e., SERVFAIL,
 * REFUSED, etc.)
 *
 * @param qstate: query state.
 * @param vq: validator query state.
 * @return true if the response could use validation (although this does not
 *         mean we can actually validate this response).
 */
static int
needs_validation(struct module_qstate* qstate, struct val_qstate* vq)
{
	int rcode;

	/* If the CD bit is on in the original request, then we don't bother to
	 * validate anything.*/
	if(qstate->query_flags | BIT_CD) {
		verbose(VERB_ALGO, "not validating response due to CD bit");
		return 0;
	}

	/* TODO: check if already validated */
	/*
	 *     if (response.getStatus() > SecurityStatus.BOGUS)
	 *         {
	 *               log.debug("response has already been validated");
	 *                     return false;
	 *                         }
	 */

	rcode = (int)FLAGS_GET_RCODE(vq->orig_msg->rep->flags);
	if(rcode != LDNS_RCODE_NOERROR && rcode != LDNS_RCODE_NXDOMAIN) {
		verbose(VERB_ALGO, "cannot validate non-answer, rcode %s",
			ldns_lookup_by_id(ldns_rcodes, rcode)?
			ldns_lookup_by_id(ldns_rcodes, rcode)->name:"??");
		return 0;
	}
	return 1;
}

/**
 * Prime trust anchor for use.
 * Generate and dispatch a priming query for the given trust anchor.
 * The trust anchor can be DNSKEY or DS and does not have to be signed.
 *
 * @param qstate: query state.
 * @param vq: validator query state.
 * @param id: module id.
 * @param toprime: what to prime.
 * @return false on a processing error.
 */
static int
prime_trust_anchor(struct module_qstate* qstate, struct val_qstate* vq,
	int id, struct trust_anchor* toprime)
{
	struct module_qstate* newq;
	struct query_info ask;
	ask.qname = toprime->name;
	ask.qname_len = toprime->namelen;
	ask.qtype = LDNS_RR_TYPE_DNSKEY;
	ask.qclass = toprime->dclass;
	log_query_info(VERB_ALGO, "priming trust anchor", &ask);
	if(!(*qstate->env->attach_sub)(qstate, &ask, 
		(uint16_t)(BIT_RD|BIT_CD), 0, &newq)){
		log_err("Could not prime trust anchor: out of memory");
		return 0;
	}
	/* ignore newq; validator does not need state created for that
	 * query, and its a 'normal' for iterator as well */
	vq->wait_prime_ta = 1; /* to elicit PRIME_RESP_STATE processing 
		from the validator inform_super() routine */
	qstate->ext_state[id] = module_wait_subquery;
	return 1;
}

/** 
 * Process init state for validator.
 * Process the INIT state. First tier responses start in the INIT state.
 * This is where they are vetted for validation suitability, and the initial
 * key search is done.
 * 
 * Currently, events the come through this routine will be either promoted
 * to FINISHED/CNAME_RESP (no validation needed), FINDKEY (next step to
 * validation), or will be (temporarily) retired and a new priming request
 * event will be generated.
 *
 * @param qstate: query state.
 * @param vq: validator query state.
 * @param ve: validator shared global environment.
 * @param id: module id.
 * @return true if the event should be processed further on return, false if
 *         not.
 */
static int
processInit(struct module_qstate* qstate, struct val_qstate* vq, 
	struct val_env* ve, int id)
{
	uint8_t* lookup_name;
	size_t lookup_len;
	if(!needs_validation(qstate, vq)) {
		vq->state = vq->final_state;
		return 1;
	}
	vq->trust_anchor = anchors_lookup(ve->anchors, vq->qchase.qname,
		vq->qchase.qname_len, vq->qchase.qclass);
	if(vq->trust_anchor == NULL) {
		/*response isn't under a trust anchor, so we cannot validate.*/
		vq->state = vq->final_state;
		return 1;
	}

	/* Determine the signer/lookup name */
	val_find_signer(&vq->qchase, vq->chase_reply, 
		&vq->signer_name, &vq->signer_len);
	if(vq->signer_name == NULL) {
		lookup_name = vq->qchase.qname;
		lookup_len = vq->qchase.qname_len;
	} else {
		lookup_name = vq->signer_name;
		lookup_len = vq->signer_len;
	}

	vq->key_entry = key_cache_obtain(ve->kcache, lookup_name, lookup_len,
		vq->qchase.qclass, qstate->region);
	
	/* if not key, or if keyentry is *above* the trustanchor, i.e.
	 * the keyentry is based on another (higher) trustanchor */
	if(vq->key_entry == NULL || dname_strict_subdomain_c(
		vq->trust_anchor->name, vq->key_entry->name)) {
		/* fire off a trust anchor priming query. */
		if(!prime_trust_anchor(qstate, vq, id, vq->trust_anchor))
			return val_error(qstate, id);
		/* and otherwise, don't continue processing this event.
		 * (it will be reactivated when the priming query returns). */
		vq->state = VAL_FINDKEY_STATE;
		return 0;
	} else if(key_entry_isnull(vq->key_entry)) {
		/* response is under a null key, so we cannot validate
		 * However, we do set the status to INSECURE, since it is 
		 * essentially proven insecure. */
		/* TODO
		vq->security_state = SEC_INSECURE;
		*/
		vq->state = vq->final_state;
		return 1;
	}

	/* otherwise, we have our "closest" cached key -- continue 
	 * processing in the next state. */
	vq->state = VAL_FINDKEY_STATE;
	return 1;
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
	if(vq && qstate->qinfo.qname != vq->qchase.qname) 
		log_query_info(VERB_DETAIL, "validator operate: chased to",
		&vq->qchase);
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
 * Evaluate the response to a priming request.
 *
 * @param rcode: rcode return value.
 * @param msg: message return value (allocated in a the wrong region).
 * @param ta: trust anchor.
 * @param qstate: qstate that needs key.
 * @param id: module id.
 * @return new key entry or NULL on allocation failure.
 *	The key entry will either contain a validated DNSKEY rrset, or
 *	represent a Null key (query failed, but validation did not), or a
 *	Bad key (validation failed).
 */
static struct key_entry_key*
primeResponseToKE(int rcode, struct dns_msg* msg, struct trust_anchor* ta,
	struct module_qstate* qstate, int id)
{
	struct ub_packed_rrset_key* dnskey_rrset = NULL;
	if(rcode == LDNS_RCODE_NOERROR) {
		dnskey_rrset = 0/*find answer */;
	}
	if(!dnskey_rrset) {
		log_query_info(VERB_ALGO, "failed to prime trust anchor -- "
			"could not fetch DNSKEY rrset", &msg->qinfo);
		/* create NULL key with NULL_KEY_TTL, store in cache. */
		return NULL;
	}
	return NULL;
}

/**
 * Process prime response
 * Sets the key entry in the state.
 *
 * @param qstate: query state that is validating and primed a trust anchor.
 * @param vq: validator query state
 * @param id: module id.
 * @param rcode: rcode result value.
 * @param msg: result message (if rcode is OK).
 */
static void
process_prime_response(struct module_qstate* qstate, struct val_qstate* vq,
	int id, int rcode, struct dns_msg* msg)
{
	/* Fetch and validate the keyEntry that corresponds to the 
	 * current trust anchor. */
	vq->key_entry = primeResponseToKE(rcode, msg, vq->trust_anchor,
		qstate, id);

	/* If the result of the prime is a null key, skip the FINDKEY state.*/
	if(!vq->key_entry || key_entry_isnull(vq->key_entry)) {
		vq->state = VAL_VALIDATE_STATE;
	}
	/* the qstate will be reactivated after inform_super is done */
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
	struct val_qstate* vq = (struct val_qstate*)super->minfo[id];
	log_query_info(VERB_ALGO, "validator: inform_super, sub=",
		&qstate->qinfo);
	log_query_info(VERB_ALGO, "super=", &super->qinfo);
	if(!vq) {
		verbose(VERB_ALGO, "super: has no validator state");
		return;
	}
	if(vq->wait_prime_ta) {
		vq->wait_prime_ta = 0;
		process_prime_response(super, vq, id, qstate->return_rcode,
			qstate->return_msg);
		return;
	}
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
