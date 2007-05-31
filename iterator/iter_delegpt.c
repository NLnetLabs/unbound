/*
 * iterator/iter_delegpt.c - delegation point with NS and address information.
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
 * This file implements the Delegation Point. It contains a list of name servers
 * and their addresses if known.
 */
#include "config.h"
#include "iterator/iter_delegpt.h"
#include "util/region-allocator.h"
#include "util/data/dname.h"
#include "util/net_help.h"

struct delegpt* 
delegpt_create(struct region* region)
{
	struct delegpt* dp=(struct delegpt*)region_alloc(region, sizeof(*dp));
	if(!dp)
		return NULL;
	memset(dp, 0, sizeof(*dp));
	return dp;
}

struct delegpt* delegpt_copy(struct delegpt* dp, struct region* region)
{
	struct delegpt* copy = delegpt_create(region);
	struct delegpt_ns* ns;
	struct delegpt_addr* a;
	if(!copy) 
		return NULL;
	if(!delegpt_set_name(copy, region, dp->name))
		return NULL;
	for(ns = dp->nslist; ns; ns = ns->next) {
		if(!delegpt_add_ns(copy, region, ns->name))
			return NULL;
		copy->nslist->resolved = ns->resolved;
	}
	for(a = dp->target_list; a; a = a->next_target) {
		if(!delegpt_add_addr(copy, region, &a->addr, a->addrlen))
			return NULL;
	}
	return copy;
}

int 
delegpt_set_name(struct delegpt* dp, struct region* region, uint8_t* name)
{
	dp->namelabs = dname_count_size_labels(name, &dp->namelen);
	dp->name = region_alloc_init(region, name, dp->namelen);
	return dp->name != 0;
}

int 
delegpt_add_ns(struct delegpt* dp, struct region* region, uint8_t* name)
{
	struct delegpt_ns* ns = (struct delegpt_ns*)region_alloc(region,
		sizeof(struct delegpt_ns));
	if(!ns)
		return 0;
	ns->next = dp->nslist;
	dp->nslist = ns;
	(void)dname_count_size_labels(name, &ns->namelen);
	ns->name = region_alloc_init(region, name, ns->namelen);
	ns->resolved = 0;
	return 1;
}

/** find name in deleg list */
static struct delegpt_ns*
delegpt_find_ns(struct delegpt* dp, uint8_t* name, size_t namelen)
{
	struct delegpt_ns* p = dp->nslist;
	while(p) {
		if(namelen == p->namelen && 
			memcmp(name, p->name, namelen) == 0) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}

int 
delegpt_add_target(struct delegpt* dp, struct region* region, 
	uint8_t* name, size_t namelen, struct sockaddr_storage* addr, 
	socklen_t addrlen)
{
	struct delegpt_ns* ns = delegpt_find_ns(dp, name, namelen);
	if(!ns) {
		/* ignore it */
		return 1;
	}
	ns->resolved = 1;
	return delegpt_add_addr(dp, region, addr, addrlen);
}

int 
delegpt_add_addr(struct delegpt* dp, struct region* region, 
	struct sockaddr_storage* addr, socklen_t addrlen)
{
	struct delegpt_addr* a = (struct delegpt_addr*)region_alloc(region,
		sizeof(struct delegpt_addr));
	if(!a)
		return 0;
	a->next_target = dp->target_list;
	dp->target_list = a;
	a->next_result = 0;
	a->next_usable = dp->usable_list;
	dp->usable_list = a;
	memcpy(&a->addr, addr, addrlen);
	a->addrlen = addrlen;
	return 1;
}

void delegpt_log(struct delegpt* dp)
{
	char buf[LDNS_MAX_DOMAINLEN+1];
	struct delegpt_ns* ns;
	struct delegpt_addr* a;
	dname_str(dp->name, buf);
	if(dp->nslist == NULL && dp->target_list == NULL) {
		log_info("DelegationPoint<%s>: empty", buf);
		return;
	}
	log_info("DelegationPoint<%s>:", buf);
	for(ns = dp->nslist; ns; ns = ns->next) {
		dname_str(ns->name, buf);
		log_info("  %s%s", buf, (ns->resolved?"*":""));
	}
	for(a = dp->target_list; a; a = a->next_target) {
		log_addr("  ", &a->addr, a->addrlen);
	}
}

void 
delegpt_add_unused_targets(struct delegpt* dp)
{
	struct delegpt_addr* usa = dp->usable_list;
	dp->usable_list = NULL;
	while(usa) {
		usa->next_result = dp->result_list;
		dp->result_list = usa;
		usa = usa->next_usable;
	}
}

size_t 
delegpt_count_missing_targets(struct delegpt* dp)
{
	struct delegpt_ns* ns;
	size_t n = 0;
	for(ns = dp->nslist; ns; ns = ns->next)
		if(!ns->resolved)
			n++;
	return n;
}
