/*
 * iterator/iter_utils.c - iterative resolver module utility functions.
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
 * This file contains functions to assist the iterator module.
 * Configuration options. Forward zones. 
 */
#include "config.h"
#include "iterator/iter_utils.h"
#include "iterator/iterator.h"
#include "iterator/iter_hints.h"
#include "iterator/iter_delegpt.h"
#include "services/cache/infra.h"
#include "services/cache/dns.h"
#include "services/cache/rrset.h"
#include "util/net_help.h"
#include "util/module.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/region-allocator.h"
#include "util/data/msgparse.h"
#include "util/random.h"

int 
iter_apply_cfg(struct iter_env* iter_env, struct config_file* cfg)
{
	int i;
	/* target fetch policy */
	iter_env->max_dependency_depth = 4;
	iter_env->target_fetch_policy = (int*)calloc(
		(size_t)iter_env->max_dependency_depth+1, sizeof(int));
	if(iter_env->max_dependency_depth >= 1)
		iter_env->target_fetch_policy[1] = 3;
	if(iter_env->max_dependency_depth >= 2)
		iter_env->target_fetch_policy[2] = 1;
	/* TODO read setting from config */
	if(!iter_env->target_fetch_policy)
		return 0;
	for(i=0; i<iter_env->max_dependency_depth+1; i++)
		verbose(VERB_DETAIL, "target fetch policy for level %d is %d",
			i, iter_env->target_fetch_policy[i]);
	
	if(!iter_env->hints)
		iter_env->hints = hints_create();
	if(!iter_env->hints || !hints_apply_cfg(iter_env->hints, cfg)) {
		log_err("Could not set root or stub hints");
		return 0;
	}
	

	/* forwarder address */
	if(cfg->fwd_address && cfg->fwd_address[0]) {
		if(!ipstrtoaddr(cfg->fwd_address, cfg->fwd_port,
			&iter_env->fwd_addr, &iter_env->fwd_addrlen)) {
			log_err("iterator: could not set forwarder address");
			return 0;
		}
		verbose(VERB_ALGO, "iterator: fwd queries to: %s %d",
			cfg->fwd_address, cfg->fwd_port);
	}
	return 1;
}

/** filter out unsuitable targets, return rtt or -1 */
static int
iter_filter_unsuitable(struct iter_env* iter_env, struct module_env* env,
	uint8_t* name, size_t namelen, time_t now, struct delegpt_addr* a)
{
	int rtt;
	int lame;
	/* TODO: check ie->donotqueryaddrs for a */
	if(!iter_env->supports_ipv6 && addr_is_ip6(&a->addr)) {
		return -1;
	}
	/* check lameness - need zone , class info */
	if(infra_get_lame_rtt(env->infra_cache, &a->addr, a->addrlen, 
		name, namelen, &lame, &rtt, now)) {
		if(lame)
			return -1;
		else 	return rtt;
	}
	/* no server information present */
	return UNKNOWN_SERVER_NICENESS;
}

/** filter the addres list, putting best targets at front,
 * returns number of best targets (or 0, no suitable targets) */
static int
iter_filter_order(struct iter_env* iter_env, struct module_env* env,
	uint8_t* name, size_t namelen, time_t now, struct delegpt* dp)
{
	int got_num = 0, got_rtt = 0, thisrtt, swap_to_front;
	struct delegpt_addr* a, *n, *prev=NULL;

	a = dp->result_list;
	while(a) {
		/* filter out unsuitable targets */
		thisrtt = iter_filter_unsuitable(iter_env, env, name, namelen, 
			now, a);
		if(thisrtt == -1) {
			prev = a;
			a = a->next_result;
			continue;
		}
		/* classify the server address and determine what to do */
		swap_to_front = 0;
		if(got_num == 0) {
			got_rtt = thisrtt;
			got_num = 1;
			swap_to_front = 1;
		} else if(thisrtt == got_rtt) {
			got_num++;
			swap_to_front = 1;
		} else if(thisrtt < got_rtt) {
			got_rtt = thisrtt;
			got_num = 1; /* start back at count of 1 */
			swap_to_front = 1;
		}
		/* swap to front if necessary, or move to next result */
		if(swap_to_front && prev) {
			n = a->next_result;
			prev->next_result = n;
			a->next_result = dp->result_list;
			dp->result_list = a;
			a = n;
		} else {
			prev = a;
			a = a->next_result;
		}
	}
	return got_num;
}

struct delegpt_addr* 
iter_server_selection(struct iter_env* iter_env, 
	struct module_env* env, struct delegpt* dp, 
	uint8_t* name, size_t namelen)
{
	time_t now = time(NULL);
	int sel;
	struct delegpt_addr* a, *prev;
	int num = iter_filter_order(iter_env, env, name, namelen, now, dp);

	if(num == 0)
		return NULL;
	if(num == 1) {
		a = dp->result_list;
		dp->result_list = a->next_result;
		return a;
	}
	/* randomly select a target from the list */
	log_assert(num > 1);
	/* we do not need secure random numbers here, but
	 * we do need it to be threadsafe, so we use this */
	sel = ub_random(env->rnd) % num; 
	a = dp->result_list;
	prev = NULL;
	while(sel > 0 && a) {
		prev = a;
		a = a->next_result;
		sel--;
	}
	if(!a)  /* robustness */
		return NULL;
	/* remove it from list */
	if(prev)
		prev->next_result = a->next_result;
	else	dp->result_list = a->next_result;
	return a;
}

struct dns_msg* 
dns_alloc_msg(ldns_buffer* pkt, struct msg_parse* msg, struct region* region)
{
	struct dns_msg* m = (struct dns_msg*)region_alloc(region,
		sizeof(struct dns_msg));
	if(!m)
		return NULL;
	memset(m, 0, sizeof(*m));
	if(!parse_create_msg(pkt, msg, NULL, &m->qinfo, &m->rep, region)) {
		log_err("malloc failure: allocating incoming dns_msg");
		return NULL;
	}
	return m;
}

int 
iter_dns_store(struct module_env* env, struct dns_msg* msg, int is_referral)
{
	struct reply_info* rep = NULL;
	/* alloc, malloc properly (not in region, like msg is) */
	rep = reply_info_copy(msg->rep, env->alloc, NULL);
	if(!rep)
		return 0;

	if(is_referral) {
		/* store rrsets */
		struct rrset_ref ref;
		uint32_t now = time(NULL);
		size_t i;
		reply_info_set_ttls(rep, now);
		for(i=0; i<rep->rrset_count; i++) {
			ref.key = rep->rrsets[i];
			ref.id = rep->rrsets[i]->id;
			/*ignore ret: it was in the cache, ref updated */
			(void)rrset_cache_update(env->rrset_cache, &ref, 
				env->alloc, now);
		}
		return 1;
	} else {
		/* store msg, and rrsets */
		struct query_info qinf;
		hashvalue_t h;

		qinf = msg->qinfo;
		qinf.qname = memdup(msg->qinfo.qname, msg->qinfo.qname_len);
		if(!qinf.qname)
			return 0;
		h = query_info_hash(&qinf);
		dns_cache_store_msg(env, &qinf, h, rep);
		free(qinf.qname);
	}
	return 1;
}
