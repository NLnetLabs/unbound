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
#include "validator/val_nsec.h"
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
 * Given a "positive" response -- a response that contains an answer to the
 * question, and no CNAME chain, validate this response. This generally
 * consists of verifying the answer RRset and the authority RRsets.
 * 
 * Note that by the time this method is called, the process of finding the
 * trusted DNSKEY rrset that signs this response must already have been
 * completed.
 * 
 * @param env: module env for verify.
 * @param ve: validator env for verify.
 * @param qchase: query that was made.
 * @param chase_reply: answer to that query to validate.
 * @param key_entry: the key entry, which is trusted, and which matches
 * 	the signer of the answer. The key entry isgood().
 */
static void
validate_positive_response(struct module_env* env, struct val_env* ve, 
	struct query_info* qchase, struct reply_info* chase_reply, 
	struct key_entry_key* key_entry)
{
	uint8_t* wc = NULL;
	int wc_NSEC_ok = 0;
	int dname_seen = 0;
	int nsec3s_seen = 0;
	size_t i;
	struct ub_packed_rrset_key* s;
	enum sec_status sec;

	/* validate the ANSWER section - this will be the answer itself */
	for(i=0; i<chase_reply->an_numrrsets; i++) {
		s = chase_reply->rrsets[i];
		/* Skip the CNAME following a (validated) DNAME.
		 * Because of the normalization routines in the iterator, 
		 * there will always be an unsigned CNAME following a DNAME 
		 * (unless qtype=DNAME). */
		if(dname_seen && ntohs(s->rk.type) == LDNS_RR_TYPE_CNAME) {
			dname_seen = 0;
			continue;
		}

		/* Verify the answer rrset */
		sec = val_verify_rrset_entry(env, ve, s, key_entry);
		/* If the (answer) rrset failed to validate, then this 
		 * message is BAD. */
		if(sec != sec_status_secure) {
			log_nametypeclass(VERB_ALGO, "Positive response has "
				"failed ANSWER rrset: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}
		/* Check to see if the rrset is the result of a wildcard 
		 * expansion. If so, an additional check will need to be 
		 * made in the authority section. */
		if(!val_rrset_wildcard(s, &wc)) {
			log_nametypeclass(VERB_ALGO, "Positive response has "
				"inconsistent wildcard sigs: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}
		
		/* Notice a DNAME that should be followed by an unsigned 
		 * CNAME. */
		if(qchase->qtype != LDNS_RR_TYPE_DNAME && 
			ntohs(s->rk.type) == LDNS_RR_TYPE_DNAME) {
			dname_seen = 1;
		}
	}

	/* validate the AUTHORITY section as well - this will generally be 
	 * the NS rrset (which could be missing, no problem) */
	for(i=chase_reply->an_numrrsets; i<chase_reply->an_numrrsets+
		chase_reply->ns_numrrsets; i++) {
		s = chase_reply->rrsets[i];
		sec = val_verify_rrset_entry(env, ve, s, key_entry);
		/* If anything in the authority section fails to be secure, 
		 * we have a bad message. */
		if(sec != sec_status_secure) {
			log_nametypeclass(VERB_ALGO, "Positive response has "
				"failed AUTHORITY rrset: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}

		/* If this is a positive wildcard response, and we have a 
		 * (just verified) NSEC record, try to use it to 1) prove 
		 * that qname doesn't exist and 2) that the correct wildcard 
		 * was used. */
		if(wc != NULL && ntohs(s->rk.type) == LDNS_RR_TYPE_NSEC) {
			if(val_nsec_proves_positive_wildcard(s, qchase, wc)) {
				wc_NSEC_ok = 1;
			}
			/* if not, continue looking for proof */
		}

		/* Otherwise, if this is a positive wildcard response and 
		 * we have NSEC3 records */
		if(wc != NULL && ntohs(s->rk.type) == LDNS_RR_TYPE_NSEC3) {
			nsec3s_seen = 1;
		}
	}

	/* If this was a positive wildcard response that we haven't already
	 * proven, and we have NSEC3 records, try to prove it using the NSEC3
	 * records. */
	if(wc != NULL && !wc_NSEC_ok && nsec3s_seen) {
		/* TODO NSEC3 positive wildcard proof */
		/* possibly: wc_NSEC_ok = 1; */
	}

	/* If after all this, we still haven't proven the positive wildcard
	 * response, fail. */
	if(wc != NULL && !wc_NSEC_ok) {
		verbose(VERB_ALGO, "positive response was wildcard "
			"expansion and did not prove original data "
			"did not exist");
		chase_reply->security = sec_status_bogus;
		return;
	}

	verbose(VERB_ALGO, "Successfully validated positive response");
	chase_reply->security = sec_status_secure;
}

/** 
 * Validate a NOERROR/NODATA signed response -- a response that has a
 * NOERROR Rcode but no ANSWER section RRsets. This consists of verifying
 * the authority section rrsets and making certain that the authority
 * section NSEC/NSEC3s proves that the qname does exist and the qtype doesn't.
 *
 * Note that by the time this method is called, the process of finding the
 * trusted DNSKEY rrset that signs this response must already have been
 * completed.
 *
 * @param env: module env for verify.
 * @param ve: validator env for verify.
 * @param qchase: query that was made.
 * @param chase_reply: answer to that query to validate.
 * @param key_entry: the key entry, which is trusted, and which matches
 * 	the signer of the answer. The key entry isgood().
 */
static void
validate_nodata_response(struct module_env* env, struct val_env* ve, 
	struct query_info* qchase, struct reply_info* chase_reply, 
	struct key_entry_key* key_entry)
{
	/* Since we are here, there must be nothing in the ANSWER section to
	 * validate. */
	/* (Note: CNAME/DNAME responses will not directly get here --
	 * instead they are broken down into individual CNAME/DNAME/final answer
	 * responses. - TODO this will change though) */
	
	/* validate the AUTHORITY section */
	int has_valid_nsec = 0; /* If true, then the NODATA has been proven.*/
	uint8_t* ce = NULL; /* for wildcard nodata responses. This is the 
				proven closest encloser. */
	uint8_t* wc = NULL; /* for wildcard nodata responses. wildcard nsec */
	int nsec3s_seen = 0; /* nsec3s seen */
	struct ub_packed_rrset_key* s; 
	enum sec_status sec;
	size_t i;

	for(i=chase_reply->an_numrrsets; i<chase_reply->an_numrrsets+
		chase_reply->ns_numrrsets; i++) {
		s = chase_reply->rrsets[i];
		sec = val_verify_rrset_entry(env, ve, s, key_entry);
		if(sec != sec_status_secure) {
			log_nametypeclass(VERB_ALGO, "NODATA response has "
				"failed AUTHORITY rrset: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}

		/* If we encounter an NSEC record, try to use it to prove 
		 * NODATA.
		 * This needs to handle the ENT NODATA case. */
		if(ntohs(s->rk.type) == LDNS_RR_TYPE_NSEC) {
			if(nsec_proves_nodata(s, qchase)) {
				has_valid_nsec = 1;
				if(dname_is_wild(s->rk.dname))
					wc = s->rk.dname;
			} 
			if(val_nsec_proves_name_error(s, qchase->qname)) {
				ce = nsec_closest_encloser(qchase->qname, s);
			}
		} else if(ntohs(s->rk.type) == LDNS_RR_TYPE_NSEC3) {
			nsec3s_seen = 1;
		}
	}

	/* check to see if we have a wildcard NODATA proof. */

	/* The wildcard NODATA is 1 NSEC proving that qname does not exists 
	 * (and also proving what the closest encloser is), and 1 NSEC 
	 * showing the matching wildcard, which must be *.closest_encloser. */
	if(wc && !ce)
		has_valid_nsec = 0;
	else if(wc && ce) {
		log_assert(dname_is_wild(wc));
		/* first label wc is \001*, so remove and compare to ce */
		if(query_dname_compare(wc+2, ce) != 0) {
			has_valid_nsec = 0;
		}
	}
	
	if(!has_valid_nsec && nsec3s_seen) {
		/* TODO handle NSEC3 proof here */
		/* and set has_valid_nsec=1; if so */
	}

	if(!has_valid_nsec) {
		verbose(VERB_ALGO, "NODATA response failed to prove NODATA "
			"status with NSEC/NSEC3");
		if(verbosity >= VERB_ALGO)
			log_dns_msg("Failed NODATA", qchase, chase_reply);
		chase_reply->security = sec_status_bogus;
		return;
	}

	verbose(VERB_ALGO, "successfully validated NODATA response.");
	chase_reply->security = sec_status_secure;
}

/** 
 * Validate a NAMEERROR signed response -- a response that has a NXDOMAIN
 * Rcode. This consists of verifying the authority section rrsets and making
 * certain that the authority section NSEC proves that the qname doesn't
 * exist and the covering wildcard also doesn't exist..
 * 
 * Note that by the time this method is called, the process of finding the
 * trusted DNSKEY rrset that signs this response must already have been
 * completed.
 *
 * @param env: module env for verify.
 * @param ve: validator env for verify.
 * @param qchase: query that was made.
 * @param chase_reply: answer to that query to validate.
 * @param key_entry: the key entry, which is trusted, and which matches
 * 	the signer of the answer. The key entry isgood().
 */
static void
validate_nameerror_response(struct module_env* env, struct val_env* ve, 
	struct query_info* qchase, struct reply_info* chase_reply, 
	struct key_entry_key* key_entry)
{
	/* FIXME: should we check to see if there is anything in the answer
	 * section? if so, what should the result be? */

	int has_valid_nsec = 0;
	int has_valid_wnsec = 0;
	int nsec3s_seen = 0;
	struct ub_packed_rrset_key* s; 
	enum sec_status sec;
	size_t i;

	for(i=chase_reply->an_numrrsets; i<chase_reply->an_numrrsets+
		chase_reply->ns_numrrsets; i++) {
		s = chase_reply->rrsets[i];
		sec = val_verify_rrset_entry(env, ve, s, key_entry);
		if(sec != sec_status_secure) {
			log_nametypeclass(VERB_ALGO, "NameError response has "
				"failed AUTHORITY rrset: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}
		if(ntohs(s->rk.type) == LDNS_RR_TYPE_NSEC) {
			if(val_nsec_proves_name_error(s, qchase->qname))
				has_valid_nsec = 1;
			if(val_nsec_proves_no_wc(s, qchase->qname, 
				qchase->qname_len))
				has_valid_wnsec = 1;
		} else if(ntohs(s->rk.type) == LDNS_RR_TYPE_NSEC3)
			nsec3s_seen = 1;
	}

	if(!has_valid_nsec || !has_valid_wnsec) {
		/* TODO: use NSEC3 proof */
	}

	/* If the message fails to prove either condition, it is bogus. */
	if(!has_valid_nsec) {
		verbose(VERB_ALGO, "NameError response has failed to prove: "
		          "qname does not exist");
		chase_reply->security = sec_status_bogus;
		return;
	}

	if(!has_valid_wnsec) {
		verbose(VERB_ALGO, "NameError response has failed to prove: "
		          "covering wildcard does not exist");
		chase_reply->security = sec_status_bogus;
		return;
	}

	/* Otherwise, we consider the message secure. */
	verbose(VERB_ALGO, "successfully validated NAME ERROR response.");
	chase_reply->security = sec_status_secure;
}

/** 
 * Given an "ANY" response -- a response that contains an answer to a
 * qtype==ANY question, with answers. This consists of simply verifying all
 * present answer/auth RRsets, with no checking that all types are present.
 * 
 * NOTE: it may be possible to get parent-side delegation point records
 * here, which won't all be signed. Right now, this routine relies on the
 * upstream iterative resolver to not return these responses -- instead
 * treating them as referrals.
 * 
 * NOTE: RFC 4035 is silent on this issue, so this may change upon
 * clarification.
 * 
 * Note that by the time this method is called, the process of finding the
 * trusted DNSKEY rrset that signs this response must already have been
 * completed.
 * 
 * @param env: module env for verify.
 * @param ve: validator env for verify.
 * @param qchase: query that was made.
 * @param chase_reply: answer to that query to validate.
 * @param key_entry: the key entry, which is trusted, and which matches
 * 	the signer of the answer. The key entry isgood().
 */
static void
validate_any_response(struct module_env* env, struct val_env* ve, 
	struct query_info* qchase, struct reply_info* chase_reply, 
	struct key_entry_key* key_entry)
{
	struct ub_packed_rrset_key* s; 
	enum sec_status sec;
	size_t i;
	if(qchase->qtype != LDNS_RR_TYPE_ANY) {
		log_err("internal error: ANY validation called for non-ANY");
		chase_reply->security = sec_status_bogus;
		return;
	}

	/* validate the ANSWER section. */
	for(i=0; i<chase_reply->an_numrrsets; i++) {
		s = chase_reply->rrsets[i];
		sec = val_verify_rrset_entry(env, ve, s, key_entry);
		/* If the (answer) rrset failed to validate, then this 
		 * message is BAD. */
		if(sec != sec_status_secure) {
			log_nametypeclass(VERB_ALGO, "ANY response has "
				"failed ANSWER rrset: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}
	}

	/* validate the AUTHORITY section as well - this will be the NS rrset
	 * (which could be missing, no problem) */
	for(i=chase_reply->an_numrrsets; i<chase_reply->an_numrrsets+
		chase_reply->ns_numrrsets; i++) {
		s = chase_reply->rrsets[i];
		sec = val_verify_rrset_entry(env, ve, s, key_entry);
		/* If anything in the authority section fails to be 
		 * secure, we have a bad message. */
		if(sec != sec_status_secure) {
			log_nametypeclass(VERB_ALGO, "ANY response has "
				"failed AUTHORITY rrset: ", s->rk.dname,
				ntohs(s->rk.type), ntohs(s->rk.rrset_class));
			chase_reply->security = sec_status_bogus;
			return;
		}
	}

	verbose(VERB_ALGO, "Successfully validated positive ANY response");
	chase_reply->security = sec_status_secure;
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
		verbose(VERB_ALGO, "prime trust anchor");
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
 * Process the VALIDATE stage, the init and findkey stages are finished,
 * and the right keys are available to validate the response.
 * Or, there are no keys available, in order to invalidate the response.
 *
 * After validation, the status is recorded in the message and rrsets,
 * and finished state is started.
 *
 * @param qstate: query state.
 * @param vq: validator query state.
 * @param ve: validator shared global environment.
 * @param id: module id.
 * @return true if the event should be processed further on return, false if
 *         not.
 */
static int
processValidate(struct module_qstate* qstate, struct val_qstate* vq, 
	struct val_env* ve, int id)
{
	enum val_classification subtype;

	if(!vq->key_entry) {
		verbose(VERB_ALGO, "validate: no key entry, failed");
		qstate->ext_state[id] = module_error;
		return 0;
	}

	/* This is the default next state. */
	vq->state = VAL_FINISHED_STATE;

	/* signerName being null is the indicator that this response was 
	 * unsigned */
	if(vq->signer_name == NULL) {
		log_query_info(VERB_ALGO, "processValidate: state has no "
			"signer name", &vq->qchase);
		/* Unsigned responses must be underneath a "null" key entry.*/
		if(key_entry_isnull(vq->key_entry)) {
			verbose(VERB_ALGO, "Unsigned response was proven to "
				"be validly INSECURE");
			vq->chase_reply->security = sec_status_insecure;
			return 1;
		}
		verbose(VERB_ALGO, "Could not establish validation of "
		          "INSECURE status of unsigned response.");
		vq->chase_reply->security = sec_status_bogus;
		return 1;
	}

	if(key_entry_isbad(vq->key_entry)) {
		log_nametypeclass(VERB_ALGO, "Could not establish a chain "
			"of trust to keys for", vq->key_entry->name,
			LDNS_RR_TYPE_DNSKEY, vq->key_entry->key_class);
		vq->chase_reply->security = sec_status_bogus;
		return 1;
	}

	if(key_entry_isnull(vq->key_entry)) {
		verbose(VERB_ALGO, "Verified that response is INSECURE");
		vq->chase_reply->security = sec_status_insecure;
		return 1;
	}

	subtype = val_classify_response(&vq->qchase, vq->chase_reply);
	switch(subtype) {
		case VAL_CLASS_POSITIVE:
			verbose(VERB_ALGO, "Validating a positive response");
			validate_positive_response(qstate->env, ve, 
				&vq->qchase, vq->chase_reply, vq->key_entry);
			break;
			
		case VAL_CLASS_NODATA:
			verbose(VERB_ALGO, "Validating a nodata response");
			validate_nodata_response(qstate->env, ve,
				&vq->qchase, vq->chase_reply, vq->key_entry);
			break;

		case VAL_CLASS_NAMEERROR:
			verbose(VERB_ALGO, "Validating a nxdomain response");
			validate_nameerror_response(qstate->env, ve,
				&vq->qchase, vq->chase_reply, vq->key_entry);
			break;

		case VAL_CLASS_CNAME:
			verbose(VERB_ALGO, "Validating a cname response");
			/*
			 * TODO special CNAME state or routines 
			validate_cname_response(vq->qchase, vq->chase_reply,
				vq->key_entry);
			*/
			break;

		case VAL_CLASS_ANY:
			verbose(VERB_ALGO, "Validating a positive ANY "
				"response");
			validate_any_response(qstate->env, ve,
				&vq->qchase, vq->chase_reply, vq->key_entry);
			break;

		default:
			log_err("validate: unhandled response subtype: %d",
				subtype);
	}

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
			case VAL_FINDKEY_STATE: 
				cont = processFindKey(qstate, vq, id);
				break;
			case VAL_VALIDATE_STATE: 
				cont = processValidate(qstate, vq, ve, id);
				break;
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
			ta->namelen, ta->dclass, NULL_KEY_TTL);
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
			ta->namelen, ta->dclass, NULL_KEY_TTL);
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
 * In inform supers, with the resulting message and rcode and the current
 * keyset in the super state, validate the DS response, returning a KeyEntry.
 *
 * @param qstate: query state that is validating and asked for a DS.
 * @param vq: validator query state
 * @param id: module id.
 * @param rcode: rcode result value.
 * @param msg: result message (if rcode is OK).
 * @param qinfo: from the sub query state, query info.
 * @param ke: the key entry to return. It returns
 *	bad if the DS response fails to validate, null if the
 *	DS response indicated an end to secure space, good if the DS
 *	validated. It returns null if the DS response indicated that the
 *	request wasn't a delegation point.
 * @return 0 on servfail error (malloc failure).
 */
static int
ds_response_to_ke(struct module_qstate* qstate, struct val_qstate* vq,
        int id, int rcode, struct dns_msg* msg, struct query_info* qinfo,
	struct key_entry_key** ke)
{
	struct val_env* ve = (struct val_env*)qstate->env->modinfo[id];
	enum val_classification subtype;
	if(rcode != LDNS_RCODE_NOERROR) {
		/* errors here pretty much break validation */
		verbose(VERB_ALGO, "DS response was error, thus bogus");
		goto return_bogus;
	}

	subtype = val_classify_response(qinfo, msg->rep);
	if(subtype == VAL_CLASS_POSITIVE) {
		struct ub_packed_rrset_key* ds;
		enum sec_status sec;
		ds = reply_find_answer_rrset(qinfo, msg->rep);
		/* If there was no DS rrset, then we have mis-classified 
		 * this message. */
		if(!ds) {
			log_warn("internal error: POSITIVE DS response was "
				"missing DS.");
			goto return_bogus;
		}
		/* Verify only returns BOGUS or SECURE. If the rrset is 
		 * bogus, then we are done. */
		sec = val_verify_rrset_entry(qstate->env, ve, ds, 
			vq->key_entry);
		if(sec != sec_status_secure) {
			verbose(VERB_ALGO, "DS rrset in DS response did "
				"not verify");
			goto return_bogus;
		}

		/* If the DS rrset validates, we still have to make sure 
		 * that they are usable. */
		if(!val_dsset_isusable(ds)) {
			/* If they aren't usable, then we treat it like 
			 * there was no DS. */
			*ke = key_entry_create_null(qstate->region, 
				qinfo->qname, qinfo->qname_len, qinfo->qclass, 
				ub_packed_rrset_ttl(ds));
			return (*ke) != NULL;
		}

		/* Otherwise, we return the positive response. */
		log_query_info(VERB_ALGO, "DS rrset was good.", qinfo);
		*ke = key_entry_create_rrset(qstate->region,
			qinfo->qname, qinfo->qname_len, qinfo->qclass, ds);
		return (*ke) != NULL;
	} else if(subtype == VAL_CLASS_NODATA) {
		/* NODATA means that the qname exists, but that there was 
		 * no DS.  This is a pretty normal case. */
		uint32_t proof_ttl = 0;

		/* Try to prove absence of the DS with NSEC */
		enum sec_status sec = val_nsec_prove_nodata_dsreply(
			qstate->env, ve, qinfo, msg->rep, vq->key_entry, 
			&proof_ttl);
		switch(sec) {
			case sec_status_secure:
				verbose(VERB_ALGO, "NSEC RRset for the "
					"referral proved no DS.");
				*ke = key_entry_create_null(qstate->region, 
					qinfo->qname, qinfo->qname_len, 
					qinfo->qclass, proof_ttl);
				return (*ke) != NULL;
			case sec_status_insecure:
				verbose(VERB_ALGO, "NSEC RRset for the "
				  "referral proved not a delegation point");
				*ke = NULL;
				return 1;
			case sec_status_bogus:
				verbose(VERB_ALGO, "NSEC RRset for the "
					"referral did not prove no DS.");
				goto return_bogus;
			case sec_status_unchecked:
			default:
				/* NSEC proof did not work, try next */
				break;
		}

		/* Or it could be using NSEC3. TODO */

		/* Apparently, no available NSEC/NSEC3 proved NODATA, so 
		 * this is BOGUS. */
		verbose(VERB_ALGO, "DS ran out of options, so return bogus");
		goto return_bogus;
	} else if(subtype == VAL_CLASS_NAMEERROR) {
		verbose(VERB_ALGO, "DS response was NAMEERROR, thus bogus.");
		goto return_bogus;
	} else {
		verbose(VERB_ALGO, "Encountered an unhandled type of "
			"DS response, thus bogus.");
return_bogus:
		*ke = key_entry_create_bad(qstate->region, qinfo->qname,
			qinfo->qname_len, qinfo->qclass);
		return (*ke) != NULL;
	}
	/* unreachable */
	log_assert(0);
}

/**
 * Process DS response. Called from inform_supers.
 * Because it is in inform_supers, the mesh itself is busy doing callbacks
 * for a state that is to be deleted soon; don't touch the mesh; instead
 * set a state in the super, as the super will be reactivated soon.
 * Perform processing to determine what state to set in the super.
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
	if(!ds_response_to_ke(qstate, vq, id, rcode, msg, qinfo, &dske)) {
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
		vq->ds_rrset = key_entry_get_rrset(dske, qstate->region);
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
 * Because it is in inform_supers, the mesh itself is busy doing callbacks
 * for a state that is to be deleted soon; don't touch the mesh; instead
 * set a state in the super, as the super will be reactivated soon.
 * Perform processing to determine what state to set in the super.
 *
 * @param qstate: query state that is validating and asked for a DNSKEY.
 * @param vq: validator query state
 * @param id: module id.
 * @param rcode: rcode result value.
 * @param msg: result message (if rcode is OK).
 * @param qinfo: from the sub query state, query info.
 */
static void
process_dnskey_response(struct module_qstate* qstate, struct val_qstate* vq,
	int id, int rcode, struct dns_msg* msg, struct query_info* qinfo)
{
	struct val_env* ve = (struct val_env*)qstate->env->modinfo[id];
	struct ub_packed_rrset_key* dnskey = NULL;

	if(rcode == LDNS_RCODE_NOERROR)
		dnskey = reply_find_answer_rrset(qinfo, msg->rep);

	if(dnskey == NULL) {
		/* bad response */
		verbose(VERB_ALGO, "Missing DNSKEY RRset in response to "
			"DNSKEY query.");
		vq->key_entry = key_entry_create_bad(qstate->region, 
			qinfo->qname, qinfo->qname_len, qinfo->qclass);
		if(!vq->key_entry) {
			log_err("alloc failure in missing dnskey response");
			/* key_entry is NULL for failure in Validate */
		}
		vq->state = VAL_VALIDATE_STATE;
		return;
	}
	if(!vq->ds_rrset) {
		log_err("internal error: no DS rrset for new DNSKEY response");
		vq->key_entry = NULL;
		vq->state = VAL_VALIDATE_STATE;
		return;
	}
	vq->key_entry = val_verify_new_DNSKEYs(qstate->region, qstate->env,
		ve, dnskey, vq->ds_rrset);

	if(!vq->key_entry) {
		log_err("out of memory in verify new DNSKEYs");
		vq->state = VAL_VALIDATE_STATE;
		return;
	}
	/* If the key entry isBad or isNull, then we can move on to the next
	 * state. */
	if(!key_entry_isgood(vq->key_entry)) {
		if(key_entry_isbad(vq->key_entry))
			verbose(VERB_ALGO, "Did not match a DS to a DNSKEY, "
				"thus bogus.");
		vq->state = VAL_VALIDATE_STATE;
		return;
	}

	/* The DNSKEY validated, so cache it as a trusted key rrset. */
	key_cache_insert(ve->kcache, vq->key_entry);

	/* If good, we stay in the FINDKEY state. */
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
	log_query_info(VERB_ALGO, "validator: inform_super, sub is",
		&qstate->qinfo);
	log_query_info(VERB_ALGO, "super is", &super->qinfo);
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
			qstate->return_msg, &qstate->qinfo);
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
		case VAL_FINDKEY_STATE: return "VAL_FINDKEY_STATE";
		case VAL_VALIDATE_STATE: return "VAL_VALIDATE_STATE";
		case VAL_FINISHED_STATE: return "VAL_FINISHED_STATE";
	}
	return "UNKNOWN VALIDATOR STATE";
}
