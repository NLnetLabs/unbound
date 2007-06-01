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
#include "util/net_help.h"
#include "util/module.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/region-allocator.h"
#include "util/data/msgparse.h"
	
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
        struct delegpt_addr* a, uint8_t* name, size_t namelen, time_t now)
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

struct delegpt_addr* iter_server_selection(struct iter_env* iter_env, 
	struct module_env* env, struct delegpt* dp, 
	uint8_t* name, size_t namelen)
{
	int got_one = 0, got_rtt = 0;
	struct delegpt_addr* got = NULL, *got_prev = NULL, *a, *prev = NULL;
	time_t now = time(NULL);

	for(a = dp->result_list; a; a = a->next_result) {
		/* filter out unsuitable targets */
		int thisrtt = iter_filter_unsuitable(iter_env, env, a, name, 
			namelen, now);
		if(thisrtt == -1) {
			prev = a;
			continue;
		}
		if(!got_one) {
			got_rtt = thisrtt;
			got = a;
			got_prev = prev;
			got_one = 1;
		} else {
			if(thisrtt < got_rtt) {
				got_rtt = thisrtt;
				got = a;
				got_prev = prev;
			}
		}
		prev = a;
	}
	if(got) {
		/* remove it from list */
		if(got_prev)
			got_prev->next_result = got->next_result;
		else	dp->result_list = got->next_result;
	}
	return got;
}

struct dns_msg* 
dns_alloc_msg(struct msg_parse* msg, struct region* region)
{
	struct dns_msg* m = (struct dns_msg*)region_alloc(region,
		sizeof(struct dns_msg));
	if(!m)
		return NULL;
	memset(m, 0, sizeof(*m));
	return m;
}
