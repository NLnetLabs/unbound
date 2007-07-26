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
#include "iterator/iter_fwd.h"
#include "iterator/iter_donotq.h"
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

/** count number of integers in fetch policy string */
static int
fetch_count(const char* s)
{
	/* format ::= (sp num)+ sp 	*/
	/* num ::= [-](0-9)+ 		*/
	/* sp ::= (space|tab)* 		*/
	int num = 0;
	while(*s) {
		while(*s && isspace(*s))
			s++;
		if(!*s)	/* end of string */
			break;
		if(*s == '-')
			s++;
		if(!*s) /* only - not allowed */
			return 0;
		if(!isdigit(*s)) /* bad character */
			return 0;
		while(*s && isdigit(*s))
			s++;
		num++;
	}
	return num;
}

/** fillup fetch policy array */
static void
fetch_fill(struct iter_env* ie, const char* str)
{
	char* s = (char*)str, *e;
	int i;
	for(i=0; i<ie->max_dependency_depth+1; i++) {
		ie->target_fetch_policy[i] = strtol(s, &e, 10);
		log_assert(s != e); /* parsed syntax already */
		s = e;
	}
}

/** Read config string that represents the target fetch policy */
static int
read_fetch_policy(struct iter_env* ie, const char* str)
{
	int count = fetch_count(str);
	if(count < 1) {
		log_err("Cannot parse target fetch policy: \"%s\"", str);
		return 0;
	}
	ie->max_dependency_depth = count - 1;
	ie->target_fetch_policy = (int*)calloc(
		(size_t)ie->max_dependency_depth+1, sizeof(int));
	if(!ie->target_fetch_policy) {
		log_err("alloc fetch policy: out of memory");
		return 0;
	}
	fetch_fill(ie, str);
	return 1;
}

int 
iter_apply_cfg(struct iter_env* iter_env, struct config_file* cfg)
{
	int i;
	/* target fetch policy */
	if(!read_fetch_policy(iter_env, cfg->target_fetch_policy))
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
	if(!iter_env->fwds)
		iter_env->fwds = forwards_create();
	if(!iter_env->fwds || !forwards_apply_cfg(iter_env->fwds, cfg)) {
		log_err("Could not set forward zones");
		return 0;
	}
	if(!iter_env->donotq)
		iter_env->donotq = donotq_create();
	if(!iter_env->donotq || !donotq_apply_cfg(iter_env->donotq, cfg)) {
		log_err("Could not set donotqueryaddresses");
		return 0;
	}
	iter_env->supports_ipv6 = cfg->do_ip6;
	return 1;
}

/** filter out unsuitable targets, return rtt or -1 */
static int
iter_filter_unsuitable(struct iter_env* iter_env, struct module_env* env,
	uint8_t* name, size_t namelen, time_t now, struct delegpt_addr* a)
{
	int rtt;
	int lame;
	if(donotq_lookup(iter_env->donotq, &a->addr, a->addrlen)) {
		return -1; /* server is on the donotquery list */
	}
	if(!iter_env->supports_ipv6 && addr_is_ip6(&a->addr)) {
		return -1; /* there is no ip6 available */
	}
	/* check lameness - need zone , class info */
	if(infra_get_lame_rtt(env->infra_cache, &a->addr, a->addrlen, 
		name, namelen, &lame, &rtt, now)) {
		if(lame)
			return -1; /* server is lame */
		else if(rtt >= USEFUL_SERVER_TOP_TIMEOUT)
			return -1; /* server is unresponsive */
		else	return rtt;
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
	/* remove it from the delegation point result list */
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

struct dns_msg* 
dns_copy_msg(struct dns_msg* from, struct region* region)
{
	struct dns_msg* m = (struct dns_msg*)region_alloc(region,
		sizeof(struct dns_msg));
	if(!m)
		return NULL;
	m->qinfo = from->qinfo;
	if(!(m->qinfo.qname = region_alloc_init(region, from->qinfo.qname,
		from->qinfo.qname_len)))
		return NULL;
	if(!(m->rep = reply_info_copy(from->rep, NULL, region)))
		return NULL;
	return m;
}

int 
iter_dns_store(struct module_env* env, struct query_info* msgqinf,
	struct reply_info* msgrep, int is_referral)
{
	struct reply_info* rep = NULL;
	/* alloc, malloc properly (not in region, like msg is) */
	rep = reply_info_copy(msgrep, env->alloc, NULL);
	if(!rep)
		return 0;

	if(is_referral) {
		/* store rrsets */
		struct rrset_ref ref;
		uint32_t now = time(NULL);
		size_t i;
		for(i=0; i<rep->rrset_count; i++) {
			packed_rrset_ttl_add((struct packed_rrset_data*)
				rep->rrsets[i]->entry.data, now);
			ref.key = rep->rrsets[i];
			ref.id = rep->rrsets[i]->id;
			/*ignore ret: it was in the cache, ref updated */
			(void)rrset_cache_update(env->rrset_cache, &ref, 
				env->alloc, now);
		}
		free(rep);
		return 1;
	} else {
		/* store msg, and rrsets */
		struct query_info qinf;
		hashvalue_t h;

		qinf = *msgqinf;
		qinf.qname = memdup(msgqinf->qname, msgqinf->qname_len);
		if(!qinf.qname) {
			reply_info_parsedelete(rep, env->alloc);
			return 0;
		}
		/* fixup flags to be sensible for a reply based on the cache */
		/* this module means that RA is available. It is an answer QR. 
		 * Not AA from cache. Not CD in cache (depends on client bit). */
		rep->flags |= (BIT_RA | BIT_QR);
		rep->flags &= ~(BIT_AA | BIT_CD);
		h = query_info_hash(&qinf);
		dns_cache_store_msg(env, &qinf, h, rep);
		/* qname is used inside query_info_entrysetup, and set to 
		 * NULL. If it has not been used, free it. free(0) is safe. */
		free(qinf.qname);
	}
	return 1;
}

int 
iter_ns_probability(struct ub_randstate* rnd, int n, int m)
{
	int sel;
	if(n == m) /* 100% chance */
		return 1;
	/* we do not need secure random numbers here, but
	 * we do need it to be threadsafe, so we use this */
	sel = ub_random(rnd) % m; 
	return (sel < n);
}

/** detect dependency cycle for query and target */
static int
causes_cycle(struct module_qstate* qstate, uint8_t* name, size_t namelen,
	uint16_t t, uint16_t c)
{
	struct query_info qinf;
	qinf.qname = name;
	qinf.qname_len = namelen;
	qinf.qtype = t;
	qinf.qclass = c;
	return (*qstate->env->detect_cycle)(qstate, &qinf);
}

void 
iter_mark_cycle_targets(struct module_qstate* qstate, struct delegpt* dp)
{
	struct delegpt_ns* ns;
	for(ns = dp->nslist; ns; ns = ns->next) {
		if(ns->resolved)
			continue;
		/* see if this ns as target causes dependency cycle */
		if(causes_cycle(qstate, ns->name, ns->namelen, 
			LDNS_RR_TYPE_AAAA, qstate->qinfo.qclass) ||
		   causes_cycle(qstate, ns->name, ns->namelen, 
			LDNS_RR_TYPE_A, qstate->qinfo.qclass))
			ns->resolved = 1;
	}
}
