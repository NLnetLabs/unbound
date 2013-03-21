/*
 * edns-subnet/subnetmod.c - process EDNS subnet option.
 *
 * Copyright (c) 2013, NLnet Labs. All rights reserved.
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
 *//**
 * \file
 * subnet module for unbound.
 */

#include "config.h"

#ifdef CLIENT_SUBNET /* keeps splint happy */

#include <ldns/ldns.h>

#include "edns-subnet/subnetmod.h"
#include "edns-subnet/edns-subnet.h"
#include "services/mesh.h"
#include "util/module.h"
#include "util/regional.h"

int subnetmod_init(struct module_env* env, int id)
{
	return 1;
}

void subnetmod_deinit(struct module_env* env, int id)
{
}

void subnetmod_inform_super(struct module_qstate* qstate, int id, 
	struct module_qstate* super)
{
}

void cp_edns_bad_response(struct edns_data* target, struct edns_data* source)
{
	target->subnet_scope_mask  = 0;
	target->subnet_source_mask = source->subnet_source_mask;
	target->subnet_addr_fam    = source->subnet_addr_fam;
	memcpy(target->subnet_addr,  source->subnet_addr, INET6_SIZE);
	target->subnet_validdata = 1;
}

enum module_ext_state eval_response(struct module_qstate* qstate)
{
	if (!qstate->edns_server_out.subnet_sent) {
		if (qstate->edns_server_in.subnet_validdata)
			verbose(VERB_QUERY, "subnet: received spurious data");
		if (qstate->edns_client_in.subnet_downstream) {
			/* Copy question back to client */
			cp_edns_bad_response(&qstate->edns_client_out, 
								&qstate->edns_client_in);
		}
		return module_finished;
	}
	
	/* subnet sent but nothing came back */
	if (!qstate->edns_server_in.subnet_validdata) {
		/** The authority indicated no support for vandergaast. As a
		 * consequence the answer ended up in the regular cache. It
		 * is still usefull to put it in the vandergaast cache for 
		 * when a client explicitly asks for subnet specific answer. */
		verbose(VERB_QUERY, "subnet: Authority indicates no support");
		// TODO PUT IT IN OUR SPECIAL CACHE
		if (qstate->edns_client_in.subnet_downstream) {
			cp_edns_bad_response(&qstate->edns_client_out, 
								&qstate->edns_client_in);
		}
		return module_finished;
	}
	
	/** Being here means we have asked for and got a subnet specific answer.
	 * Also the answer from the authority is not yet cached anywhere. */
	
	/* can we accept response? */
	size_t sn_octs, remainder;
	sn_octs = qstate->edns_server_out.subnet_source_mask / 8;
	assert(sn_octs <= INET6_SIZE); /* Enforced by msgparse */
	remainder = 8 - (size_t)(qstate->edns_server_out.subnet_source_mask % 8);
	if(qstate->edns_server_out.subnet_addr_fam != qstate->edns_server_in.subnet_addr_fam ||
		qstate->edns_server_out.subnet_source_mask != qstate->edns_server_in.subnet_source_mask ||
		memcmp(qstate->edns_server_out.subnet_addr, qstate->edns_server_in.subnet_addr, sn_octs) != 0 ||
		//YBS should this just be equal?
		(qstate->edns_server_out.subnet_addr[sn_octs]^qstate->edns_server_in.subnet_addr[sn_octs])>>remainder) {
		/* we can not, restart query without option */
		verbose(VERB_QUERY, "subnet: forged data");
		qstate->edns_server_out.subnet_validdata = 0;
		qstate->edns_server_out.subnet_sent = 0;
		return module_wait_module;
	}
	
	/* TODO PUT IT IN OUR SPECIAL CACHE */
	
	if (qstate->edns_client_in.subnet_downstream) {
		/* Client wants to see the answer, echo option back
		 * and adjust the scope. */
		memcpy(&qstate->edns_client_out, &qstate->edns_client_in, 
			sizeof(qstate->edns_client_in));
		verbose(VERB_QUERY, "subnet: attach");
		qstate->edns_client_out.subnet_scope_mask = 
			qstate->edns_server_in.subnet_scope_mask;
	}
	verbose(VERB_QUERY, "subnet: done");
	return module_finished;
}

void subnetmod_operate(struct module_qstate* qstate, enum module_ev event, 
	int id, struct outbound_entry* ATTR_UNUSED(outbound))
{
#if 0
	void* cachehit;
	int max_mask;
#endif
	struct subnet_env* sne = (struct subnet_env*)qstate->env->modinfo[id];
	struct subnet_qstate* snq = (struct subnet_qstate*)qstate->minfo[id];
	
	verbose(VERB_QUERY, "subnet[module %d] operate: extstate:%s "
		"event:%s", id, strextstate(qstate->ext_state[id]), 
		strmodulevent(event));
	log_query_info(VERB_QUERY, "subnet operate: query", &qstate->qinfo);

	if(event == module_event_new) {
		if(!qstate->edns_client_in.subnet_validdata) {
			/* No clients are interested in result or we could not
			 * parse it, we don't do client subnet */
			qstate->edns_server_out.subnet_validdata = 0;
			verbose(VERB_ALGO, "subnet: pass to next module");
			qstate->ext_state[id] = module_wait_module;
			return;
		}
		/* copy information from client request to upstream query */
		memcpy(&qstate->edns_server_out, &qstate->edns_client_in, sizeof(struct edns_data));
		qstate->edns_server_out.subnet_scope_mask = 0;
		qstate->edns_server_out.subnet_sent = 0;
#if 0
		while(1) {
			/* cache returns valid answer with largest mask. */
			cachehit = NULL;//DUMMY
			//~ cache_lookup(qstate, snq, cachehit); //TBD//
			/* cachehit is null or has same sourcemask.
			 * scopemask is largest available smaller then sourcemask 
			 * 		or if none available. smallest scopemask larger than sourcemask
			 * */
			
			if(cachehit) {
				/* We have the most specific in our hands covering request. so 
				 * we know: hit.sourcemask <= snq.sourcemask */
				
				/* We have an optimal answer */
				if(snq->subnet_source_mask >= cachehit.scopemask) {
					//TBD: do stuff and asnwer
					verbose(VERB_ALGO, "subnet: nextmodule returned");
					qstate->ext_state[id] = module_finished;
					return;
				}
				/* According to our cache, the server wants to see more of our
				 * private bits. */
				
				/* If the request came directly from client we will not 
				 * modify request */
				if(snq->subnet_downstream) {
					//TBD: do stuff and asnwer
					verbose(VERB_ALGO, "subnet: nextmodule returned");
					qstate->ext_state[id] = module_finished;
					return;
				}
				/* Now see if we allow some stretch */
				max_mask = min(cachehit.scopemask, EDNSSUBNET_MAX_SUBNET_IP4);
				if(max_mask <= snq->subnet_source_mask) {
					/* We refuse giving more detail so we use the cache anyway */
					//TBD: do stuff and asnwer
					verbose(VERB_ALGO, "subnet: nextmodule returned");
					qstate->ext_state[id] = module_finished;
					return;
				}
				/* Need new resolve with new sourcemesk */
				snq->subnet_source_mask = max_mask;
				continue;
			}
			break;
		}
#endif
		verbose(VERB_QUERY, "subnet: ahh, not found in sncache");
		
		/* pass request to next module */
		verbose(VERB_ALGO, "subnet: pass to next module");
		qstate->ext_state[id] = module_wait_module;
		return;
	}
	/* Query handed back by next module, we have a 'final' answer */
	if(event == module_event_moddone) {
		verbose(VERB_QUERY, "subnet: done");
		qstate->ext_state[id] = eval_response(qstate);
		return;
	}
	/* We are being revisited */
	if(event == module_event_pass) {
		/* Just pass it on, we already did the work */
		verbose(VERB_ALGO, "subnet: pass to next module");
		qstate->ext_state[id] = module_wait_module;
		return;
	}
	log_err("subnet: bad event %s", strmodulevent(event));
	qstate->ext_state[id] = module_error;
	return;

}

void subnetmod_clear(struct module_qstate* qstate, int id)
{
}

size_t subnetmod_get_mem(struct module_env* env, int id)
{
	verbose(VERB_ALGO, "subnetmod: get_mem, id: %d, NOTIMPL", id);
	return 550;
}

/**
 * The module function block 
 */
static struct module_func_block subnetmod_block = {
	"subnet",
	&subnetmod_init, &subnetmod_deinit, &subnetmod_operate, &subnetmod_inform_super, 
	&subnetmod_clear, &subnetmod_get_mem
};

struct module_func_block* subnetmod_get_funcblock(void)
{
	return &subnetmod_block;
}

#endif  /* CLIENT_SUBNET */
