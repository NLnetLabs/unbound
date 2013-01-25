/**
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

/** fill in message structure */
static struct subnet_qstate*
sub_new_getmsg(struct module_qstate* qstate, struct subnet_qstate* snq)
{
	//~ snq->edns.subnet_downstream = 0;
	//~ snq->edns.subnet_validdata = 0;
	return snq;
}

/** allocate new subnet query state */
static struct subnet_qstate*
sub_new(struct module_qstate* qstate, int id)
{
	struct subnet_qstate* snq = (struct subnet_qstate*)regional_alloc(
		qstate->region, sizeof(*snq));
	log_assert(!qstate->minfo[id]);
	if(!snq)
		return NULL;
	memset(snq, 0, sizeof(*snq));
	qstate->minfo[id] = snq;
	return sub_new_getmsg(qstate, snq);
}

int subnetmod_init(struct module_env* env, int id)
{
	//~ env->subnet_module_id = id;
	return 1;
}

void subnetmod_deinit(struct module_env* env, int id)
{
}

void subnetmod_inform_super(struct module_qstate* qstate, int id, struct module_qstate* super)
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

void subnetmod_operate(struct module_qstate* qstate, enum module_ev event, 
	int id, struct outbound_entry* ATTR_UNUSED(outbound))
{
#if 0
	void* cachehit;
	int max_mask;
#endif
	struct edns_data* edns_from_client; //from client
	struct subnet_env* sne = (struct subnet_env*)qstate->env->modinfo[id];
	struct subnet_qstate* snq = (struct subnet_qstate*)qstate->minfo[id];
	
	verbose(VERB_QUERY, "subnet[module %d] operate: extstate:%s "
		"event:%s", id, strextstate(qstate->ext_state[id]), 
		strmodulevent(event));
	log_query_info(VERB_QUERY, "subnet operate: query",
		&qstate->qinfo);
	(void)outbound;
	/* This query is new for us */
	if(event == module_event_new || 
		(event == module_event_pass && snq == NULL)) {
		snq = sub_new(qstate, id);
		edns_from_client = qstate->edns_from_client;
		
		if(!edns_from_client || !edns_from_client->subnet_validdata) {
			/* No clients are interested in result or we could not
			 * parse it, we don't do vandergaast */
			qstate->edns_out.subnet_validdata = 0;
			verbose(VERB_ALGO, "subnet: pass to next module");
			qstate->ext_state[id] = module_wait_module;
			return;
		}
		/* copy information from client request to upstream query */
		memcpy(&qstate->edns_out, edns_from_client, sizeof(struct edns_data));
		qstate->edns_out.subnet_scope_mask = 0;
		qstate->edns_out.subnet_sent = 0;
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
				max_mask = min(cachehit.scopemask, MAX_CLIENT_SUBNET_IP4);
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
		qstate->edns_to_client = NULL;
		if (!qstate->edns_out.subnet_sent) {
			if (qstate->edns_in.subnet_validdata) {
				verbose(VERB_QUERY, "subnet: received spurious data");
			}
			if (qstate->edns_out.subnet_downstream) {
				qstate->edns_to_client = &qstate->edns_in;
				cp_edns_bad_response(qstate->edns_to_client, 
									qstate->edns_from_client);
			}
		} else {
			/* subnet sent but nothing came back */
			if (!qstate->edns_in.subnet_validdata) {
				// TODO PUT IT IN OUR SPECIAL CACHE
				if (qstate->edns_out.subnet_downstream) {
					qstate->edns_to_client = &qstate->edns_in;
					cp_edns_bad_response(qstate->edns_to_client, 
										qstate->edns_from_client);
				}
			} else {
				verbose(VERB_QUERY, "subnet: is not cached");
				/* can we accept response? */
				size_t sn_octs, remainder;
				sn_octs = qstate->edns_out.subnet_source_mask / 8;
				remainder = 8 - (size_t)(qstate->edns_out.subnet_source_mask % 8);
				if(qstate->edns_out.subnet_addr_fam != qstate->edns_in.subnet_addr_fam ||
					qstate->edns_out.subnet_source_mask != qstate->edns_in.subnet_source_mask ||
					memcmp(qstate->edns_out.subnet_addr, qstate->edns_in.subnet_addr, sn_octs) != 0 ||
					(qstate->edns_out.subnet_addr[sn_octs]^qstate->edns_in.subnet_addr[sn_octs])>>remainder) {
					/* we can not, restart query without option */
					verbose(VERB_QUERY, "subnet: forged data");
					qstate->edns_out.subnet_validdata = 0;
					qstate->ext_state[id] = module_wait_module;
					return;
				}
				verbose(VERB_QUERY, "subnet: now cache it");
				// TODO PUT IT IN OUR SPECIAL CACHE
				if (qstate->edns_out.subnet_downstream) {
					qstate->edns_to_client = qstate->edns_from_client;
					verbose(VERB_QUERY, "subnet: attach");
					qstate->edns_to_client->subnet_scope_mask = 
						qstate->edns_in.subnet_scope_mask;
				}
			}
		}
		qstate->ext_state[id] = module_finished;
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
