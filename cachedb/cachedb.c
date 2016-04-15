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
#include "util/config_file.h"
#include "util/data/msgreply.h"
#include "services/cache/dns.h"
#include "validator/val_neg.h"
#include "validator/val_secalgo.h"
#include "sldns/parseutil.h"
#include "sldns/wire2str.h"
#include "sldns/sbuffer.h"

#define CACHEDB_HASHSIZE 256 /* bit hash */

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
	/* see if a backend is selected */
	if(!cachedb_env->backend || !cachedb_env->backend->name)
		return 1;
	if(!(*cachedb_env->backend->init)(env, cachedb_env)) {
		log_err("cachedb: could not init %s backend",
			cachedb_env->backend->name);
		return 0;
	}
	cachedb_env->enabled = 1;
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
	if(cachedb_env->enabled) {
		(*cachedb_env->backend->deinit)(env, cachedb_env);
	}

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
 * Hash the query name, type, class and dbacess-secret into lookup buffer.
 * @param qstate: query state with query info
 * 	and env->cfg with secret.
 * @param buf: returned buffer with hash to lookup
 * @param len: length of the buffer.
 */
static void
calc_hash(struct module_qstate* qstate, char* buf, size_t len)
{
	uint8_t clear[1024];
	size_t clen = 0;
	uint8_t hash[CACHEDB_HASHSIZE/8];
	const char* hex = "0123456789ABCDEF";
	const char* secret = "default"; /* TODO: from qstate->env->cfg */
	size_t i;
	
	/* copy the hash info into the clear buffer */
	if(clen + qstate->qinfo.qname_len < sizeof(clear)) {
		memmove(clear+clen, qstate->qinfo.qname,
			qstate->qinfo.qname_len);
		clen += qstate->qinfo.qname_len;
	}
	if(clen + 4 < sizeof(clear)) {
		uint16_t t = htons(qstate->qinfo.qtype);
		uint16_t c = htons(qstate->qinfo.qclass);
		memmove(clear+clen, &t, 2);
		memmove(clear+clen+2, &c, 2);
		clen += 4;
	}
	if(secret && secret[0] && clen + strlen(secret) < sizeof(clear)) {
		memmove(clear+clen, secret, strlen(secret));
		clen += strlen(secret);
	}
	
	/* hash the buffer */
	secalgo_hash_sha256(clear, clen, hash);
	memset(clear, 0, clen);

	/* hex encode output for portability (some online dbs need
	 * no nulls, no control characters, and so on) */
	log_assert(len >= sizeof(hash)*2 + 1);
	(void)len;
	for(i=0; i<sizeof(hash); i++) {
		buf[i*2] = hex[(hash[i]&0xf0)>>4];
		buf[i*2+1] = hex[hash[i]&0x0f];
	}
	buf[sizeof(hash)*2] = 0;
}

/** convert data from return_msg into the data buffer */
static void
prep_data(struct module_qstate* qstate, struct sldns_buffer* buf)
{
	/* TODO */
}

/** check expiry and query details, return true if matches OK */
static int
good_expiry_and_qinfo(struct module_qstate* qstate, struct sldns_buffer* buf)
{
	/* TODO */
	return 0;
}

/** convert dns message in buffer to return_msg */
static int
parse_data(struct module_qstate* qstate, struct sldns_buffer* buf)
{
	/* TODO */
	return 0;
}

/**
 * Lookup the qstate.qinfo in extcache, store in qstate.return_msg.
 * return true if lookup was successful.
 */
static int
cachedb_extcache_lookup(struct module_qstate* qstate, struct cachedb_env* ie)
{
	char key[(CACHEDB_HASHSIZE/8)*2+1];
	calc_hash(qstate, key, sizeof(key));

	/* call backend to fetch data for key into scratch buffer */
	if( !(*ie->backend->lookup)(qstate->env, ie, key,
		qstate->env->scratch_buffer)) {
		return 0;
	}

	/* check expiry date and check if query-data matches */
	if( !good_expiry_and_qinfo(qstate, qstate->env->scratch_buffer) ) {
		return 0;
	}

	/* parse dns message into return_msg */
	if( !parse_data(qstate, qstate->env->scratch_buffer) ) {
		return 0;
	}
	return 1;
}

/**
 * Store the qstate.return_msg in extcache for key qstate.info
 */
static void
cachedb_extcache_store(struct module_qstate* qstate, struct cachedb_env* ie)
{
	char key[(CACHEDB_HASHSIZE/8)*2+1];
	calc_hash(qstate, key, sizeof(key));

	/* prepare data in scratch buffer */
	prep_data(qstate, qstate->env->scratch_buffer);
	
	/* call backend */
	(*ie->backend->store)(qstate->env, ie, key,
		sldns_buffer_begin(qstate->env->scratch_buffer),
		sldns_buffer_limit(qstate->env->scratch_buffer));
}

/**
 * See if unbound's internal cache can answer the query
 */
static int
cachedb_intcache_lookup(struct module_qstate* qstate)
{
	struct dns_msg* msg;
	msg = dns_cache_lookup(qstate->env, qstate->qinfo.qname,
		qstate->qinfo.qname_len, qstate->qinfo.qtype,
		qstate->qinfo.qclass, qstate->query_flags,
		qstate->region, qstate->env->scratch);
	if(!msg && qstate->env->neg_cache) {
		/* lookup in negative cache; may result in 
		 * NOERROR/NODATA or NXDOMAIN answers that need validation */
		msg = val_neg_getmsg(qstate->env->neg_cache, &qstate->qinfo,
			qstate->region, qstate->env->rrset_cache,
			qstate->env->scratch_buffer,
			*qstate->env->now, 1/*add SOA*/, NULL);
	}
	if(!msg)
		return 0;
	/* this is the returned msg */
	qstate->return_rcode = LDNS_RCODE_NOERROR;
	qstate->return_msg = msg;
	return 1;
}

/**
 * Store query into the internal cache of unbound.
 */
static void
cachedb_intcache_store(struct module_qstate* qstate)
{
	if(!qstate->return_msg)
		return;
	(void)dns_cache_store(qstate->env, &qstate->qinfo,
		qstate->return_msg->rep, 0, qstate->prefetch_leeway, 0,
		qstate->region, qstate->query_flags);
}

/**
 * Handle a cachedb module event with a query
 * @param qstate: query state (from the mesh), passed between modules.
 * 	contains qstate->env module environment with global caches and so on.
 * @param iq: query state specific for this module.  per-query.
 * @param ie: environment specific for this module.  global.
 * @param id: module id.
 */
static void
cachedb_handle_query(struct module_qstate* qstate, struct cachedb_qstate* iq,
	struct cachedb_env* ie, int id)
{
	/* check if we are enabled, and skip if so */
	if(!ie->enabled) {
		/* pass request to next module */
		qstate->ext_state[id] = module_wait_module;
		return;
	}

	if(qstate->blacklist) {
		/* cache is blacklisted */
		/* pass request to next module */
		qstate->ext_state[id] = module_wait_module;
		return;
	}

	/* lookup inside unbound's internal cache */
	if(cachedb_intcache_lookup(qstate)) {
		if(verbosity >= VERB_ALGO)
			log_dns_msg("cachedb internal cache lookup",
				&qstate->return_msg->qinfo,
				qstate->return_msg->rep);
		/* we are done with the query */
		qstate->ext_state[id] = module_finished;
		return;
	}

	/* ask backend cache to see if we have data */
	if(cachedb_extcache_lookup(qstate, ie)) {
		if(verbosity >= VERB_ALGO)
			log_dns_msg(ie->backend->name,
				&qstate->return_msg->qinfo,
				qstate->return_msg->rep);
		/* store this result in internal cache */
		cachedb_intcache_store(qstate);
		/* we are done with the query */
		qstate->ext_state[id] = module_finished;
		return;
	}

	/* no cache fetches */
	/* pass request to next module */
	qstate->ext_state[id] = module_wait_module;
}

/**
 * Handle a cachedb module event with a response from the iterator.
 * @param qstate: query state (from the mesh), passed between modules.
 * 	contains qstate->env module environment with global caches and so on.
 * @param iq: query state specific for this module.  per-query.
 * @param ie: environment specific for this module.  global.
 * @param id: module id.
 */
static void
cachedb_handle_response(struct module_qstate* qstate,
	struct cachedb_qstate* iq, struct cachedb_env* ie, int id)
{
	/* check if we are enabled, and skip if not */
	if(!ie->enabled) {
		/* we are done with the query */
		qstate->ext_state[id] = module_finished;
		return;
	}

	/* store the item into the backend cache */
	cachedb_extcache_store(qstate, ie);

	/* we are done with the query */
	qstate->ext_state[id] = module_finished;
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
	}
	if(iq && (event == module_event_pass || event == module_event_new)) {
		cachedb_handle_query(qstate, iq, ie, id);
		return;
	}
	if(iq && (event == module_event_moddone)) {
		cachedb_handle_response(qstate, iq, ie, id);
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
	verbose(VERB_ALGO, "cachedb inform_super was called");
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
