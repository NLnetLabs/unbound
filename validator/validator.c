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
#include "util/config_file.h"

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
	val_env->date_override = cfg->val_date_override;
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
	if(qstate->query_flags & BIT_CD) {
		verbose(VERB_ALGO, "not validating response due to CD bit");
		return 0;
	}

	/* validate unchecked, and re-validate bogus messages */
	if (vq->orig_msg->rep->security > sec_status_bogus)
	{
		verbose(VERB_ALGO, "response has already been validated");
		return 0;
	}

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
 * Generate a request for DNS data.
 *
 * @param qstate: query state that is the parent.
 * @param id: module id.
 * @param name: what name to query for.
 * @param namelen: length of name.
 * @param qtype: query type.
 * @param qclass: query class.
 * @return false on alloc failure.
 */
static int
generate_request(struct module_qstate* qstate, int id, uint8_t* name, 
	size_t namelen, uint16_t qtype, uint16_t qclass)
{
	struct module_qstate* newq;
	struct query_info ask;
	ask.qname = name;
	ask.qname_len = namelen;
	ask.qtype = qtype;
	ask.qclass = qclass;
	log_query_info(VERB_ALGO, "generate request", &ask);
	if(!(*qstate->env->attach_sub)(qstate, &ask, 
		(uint16_t)(BIT_RD|BIT_CD), 0, &newq)){
		log_err("Could not generate request: out of memory");
		return 0;
	}
	/* ignore newq; validator does not need state created for that
	 * query, and its a 'normal' for iterator as well */
	qstate->ext_state[id] = module_wait_subquery;
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
	int ret = generate_request(qstate, id, toprime->name, toprime->namelen,
		LDNS_RR_TYPE_DNSKEY, toprime->dclass);
	if(!ret) {
		log_err("Could not prime trust anchor: out of memory");
		return 0;
	}
	/* ignore newq; validator does not need state created for that
	 * query, and its a 'normal' for iterator as well */
	vq->wait_prime_ta = 1; /* to elicit PRIME_RESP_STATE processing 
		from the validator inform_super() routine */
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
		vq->chase_reply->security = sec_status_insecure;
		vq->state = vq->final_state;
		return 1;
	}

	/* otherwise, we have our "closest" cached key -- continue 
	 * processing in the next state. */
	vq->state = VAL_FINDKEY_STATE;
	return 1;
}

/**
 * Process the FINDKEY state. Generally this just calculates the next name
 * to query and either issues a DS or a DNSKEY query. It will check to see
 * if the correct key has already been reached, in which case it will
 * advance the event to the next state.
 *
 * @param qstate: query state.
 * @param vq: validator query state.
 * @param id: module id.
 * @return true if the event should be processed further on return, false if
 *         not.
 */
static int
processFindKey(struct module_qstate* qstate, struct val_qstate* vq, int id)
{
	uint8_t* target_key_name, *current_key_name;
	size_t target_key_len, current_key_len;
	int strip_lab;

	verbose(VERB_ALGO, "validator: FindKey");
	/* We know that state.key_entry is not a null or bad key -- if it were,
	 * then previous processing should have directed this event to 
	 * a different state. */
	log_assert(vq->key_entry && !key_entry_isbad(vq->key_entry) && 
		!key_entry_isnull(vq->key_entry));

	target_key_name = vq->signer_name;
	target_key_len = vq->signer_len;
	if(!target_key_name) {
		target_key_name = vq->qchase.qname;
		target_key_len = vq->qchase.qname_len;
	}

	current_key_name = vq->key_entry->name;
	current_key_len = vq->key_entry->namelen;

	/* If our current key entry matches our target, then we are done. */
	if(query_dname_compare(target_key_name, current_key_name) == 0) {
		vq->state = VAL_VALIDATE_STATE;
		return 1;
	}

	if(vq->empty_DS_name) {
		current_key_name = vq->empty_DS_name;
		current_key_len = vq->empty_DS_len;
	}

	log_nametypeclass(VERB_ALGO, "current keyname", current_key_name,
		LDNS_RR_TYPE_DNSKEY, LDNS_RR_CLASS_IN);
	log_nametypeclass(VERB_ALGO, "target keyname", target_key_name,
		LDNS_RR_TYPE_DNSKEY, LDNS_RR_CLASS_IN);
	/* assert we are walking down the DNS tree */
	log_assert(dname_subdomain_c(target_key_name, current_key_name));
	/* so this value is >= 0 */
	strip_lab = dname_count_labels(target_key_name) - 
		dname_count_labels(current_key_name) - 1;
	log_assert(strip_lab >= 0);
	verbose(VERB_ALGO, "striplab %d", strip_lab);
	dname_remove_labels(&target_key_name, &target_key_len, strip_lab);
	log_nametypeclass(VERB_ALGO, "next keyname", target_key_name,
		LDNS_RR_TYPE_DNSKEY, LDNS_RR_CLASS_IN);

	/* The next step is either to query for the next DS, or to query 
	 * for the next DNSKEY. */

	if(!vq->ds_rrset || query_dname_compare(vq->ds_rrset->rk.dname,
		target_key_name) != 0) {
		if(!generate_request(qstate, id, target_key_name, 
			target_key_len, LDNS_RR_TYPE_DS, vq->qchase.qclass)) {
			log_err("mem error generating DS request");
			qstate->ext_state[id] = module_error;
			return 0;
		}
		return 0;
	}

	/* Otherwise, it is time to query for the DNSKEY */
	if(!generate_request(qstate, id, vq->ds_rrset->rk.dname, 
		vq->ds_rrset->rk.dname_len, LDNS_RR_TYPE_DNSKEY, 
		vq->qchase.qclass)) {
		log_err("mem error generating DNSKEY request");
		qstate->ext_state[id] = module_error;
		return 0;
	}

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
			case VAL_FINDKEY_STATE: 
				cont = processFindKey(qstate, vq, id);
				break;
			case VAL_PRIME_RESP_STATE: 
			case VAL_FINDKEY_DS_RESP_STATE: 
			case VAL_FINDKEY_DNSKEY_RESP_STATE: 
			case VAL_VALIDATE_STATE: 
			case VAL_FINISHED_STATE: 
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
	if(event == module_event_new || 
		(event == module_event_pass && vq == NULL)) {
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
	if(event == module_event_pass) {
		qstate->ext_state[id] = module_error; /* override this */
		/* continue processing, since val_env exists */
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
	struct val_env* ve = (struct val_env*)qstate->env->modinfo[id];
	struct ub_packed_rrset_key* dnskey_rrset = NULL;
	struct key_entry_key* kkey = NULL;
	enum sec_status sec = sec_status_unchecked;

	if(rcode == LDNS_RCODE_NOERROR) {
		dnskey_rrset = reply_find_rrset_section_an(msg->rep,
			ta->name, ta->namelen, LDNS_RR_TYPE_DNSKEY,
			ta->dclass);
	}
	if(!dnskey_rrset) {
		log_query_info(VERB_ALGO, "failed to prime trust anchor -- "
			"could not fetch DNSKEY rrset", &msg->qinfo);
		kkey = key_entry_create_null(qstate->region, ta->name,
			ta->namelen, ta->dclass, time(0)+NULL_KEY_TTL);
		if(!kkey) {
			log_err("out of memory: allocate null prime key");
			return NULL;
		}
		key_cache_insert(ve->kcache, kkey);
		return kkey;
	}
	/* attempt to verify with trust anchor DS and DNSKEY */
	if(ta->ds_rrset) {
		kkey = val_verify_new_DNSKEYs(qstate->region, qstate->env, ve, 
			dnskey_rrset, ta->ds_rrset);
		if(!kkey) {
			log_err("out of memory: verifying prime DS");
			return NULL;
		}
		if(key_entry_isgood(kkey))
			sec = sec_status_secure;
		else
			sec = sec_status_bogus;
		log_info("priming DS result %s", sec_status_to_string(sec));
	}
	if(sec != sec_status_secure && ta->dnskey_rrset) {
		sec = val_verify_rrset(qstate->env, ve, dnskey_rrset,
			ta->dnskey_rrset);
		if(sec == sec_status_secure) {
			kkey = key_entry_create_rrset(qstate->region, 
				ta->name, ta->namelen, ta->dclass, 
				dnskey_rrset);
			if(!kkey) {
				log_err("out of memory: allocate primed key");
				return NULL;
			}
		}
	}

	if(sec != sec_status_secure) {
		log_query_info(VERB_ALGO, "failed to prime trust anchor -- "
			"could not fetch DNSKEY rrset", &msg->qinfo);
		/* NOTE: in this case, we should probably reject the trust 
		 * anchor for longer, perhaps forever. */
		kkey = key_entry_create_null(qstate->region, ta->name,
			ta->namelen, ta->dclass, time(0)+NULL_KEY_TTL);
		if(!kkey) {
			log_err("out of memory: allocate null prime key");
			return NULL;
		}
		key_cache_insert(ve->kcache, kkey);
		return kkey;
	}

	log_query_info(VERB_ALGO, "Successfully primed trust anchor", 
		&msg->qinfo);
	/* store the freshly primed entry in the cache */
	key_cache_insert(ve->kcache, kkey);
	return kkey;
}

/**
 * Process DS response. Called from inform_supers.
 *
 * @param qstate: query state that is validating and asked for a DS.
 * @param vq: validator query state
 * @param id: module id.
 * @param rcode: rcode result value.
 * @param msg: result message (if rcode is OK).
 * @param qinfo: from the sub query state, query info.
 */
static void
process_ds_response(struct module_qstate* qstate, struct val_qstate* vq,
	int id, int rcode, struct dns_msg* msg, struct query_info* qinfo)
{
	struct key_entry_key* dske = NULL;
	/* TODO 
	if(!ds_response_to_ke(qstate, vq, id, rcode, msg, &dske)) {
			@@@ */ if(0) {
			log_err("malloc failure in DStoKE");
			vq->key_entry = NULL; /* make it error */
			vq->state = VAL_VALIDATE_STATE;
			return;
	}
	if(dske == NULL) {
		vq->empty_DS_name = qinfo->qname;
		vq->empty_DS_len = qinfo->qname_len;
		/* ds response indicated that we aren't on a delegation point.
		 * Keep the forState.state on FINDKEY. */
	} else if(key_entry_isgood(dske)) {
		/* TODO
		vq->ds_rrset = key_entry_getrrset(dske);
		*/
		if(!vq->ds_rrset) {
			log_err("malloc failure in process DS");
			vq->key_entry = NULL; /* make it error */
			vq->state = VAL_VALIDATE_STATE;
			return;
		}
		/* Keep the forState.state on FINDKEY. */
	} else {
		/* NOTE: the reason for the DS to be not good (that is, 
		 * either bad or null) should have been logged by 
		 * dsResponseToKE. */
		vq->key_entry = dske;
		/* The FINDKEY phase has ended, so move on. */
		vq->state = VAL_VALIDATE_STATE;
	}
}

/**
 * Process DNSKEY response. Called from inform_supers.
 * Sets the key entry in the state.
 *
 * @param qstate: query state that is validating and asked for a DNSKEY.
 * @param vq: validator query state
 * @param id: module id.
 * @param rcode: rcode result value.
 * @param msg: result message (if rcode is OK).
 */
static void
process_dnskey_response(struct module_qstate* qstate, struct val_qstate* vq,
	int id, int rcode, struct dns_msg* msg)
{
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
	if(qstate->qinfo.qtype == LDNS_RR_TYPE_DS) {
		process_ds_response(super, vq, id, qstate->return_rcode,
			qstate->return_msg, &qstate->qinfo);
		return;
	} else if(qstate->qinfo.qtype == LDNS_RR_TYPE_DNSKEY) {
		process_dnskey_response(super, vq, id, qstate->return_rcode,
			qstate->return_msg);
		return;
	}
	log_err("internal error in validator: no inform_supers possible");
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
