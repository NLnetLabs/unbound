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
#include "services/cache/dns.h"
#include "util/regional.h"
#include "util/data/dname.h"
#include "util/data/packed_rrset.h"
#include "util/data/msgreply.h"
#include "util/net_help.h"

struct delegpt* 
delegpt_create(struct regional* region)
{
	struct delegpt* dp=(struct delegpt*)regional_alloc(
		region, sizeof(*dp));
	if(!dp)
		return NULL;
	memset(dp, 0, sizeof(*dp));
	return dp;
}

struct delegpt* delegpt_copy(struct delegpt* dp, struct regional* region)
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
		copy->nslist->got4 = ns->got4;
		copy->nslist->got6 = ns->got6;
	}
	for(a = dp->target_list; a; a = a->next_target) {
		if(!delegpt_add_addr(copy, region, &a->addr, a->addrlen, 
			a->bogus, 0))
			return NULL;
	}
	return copy;
}

int 
delegpt_set_name(struct delegpt* dp, struct regional* region, uint8_t* name)
{
	dp->namelabs = dname_count_size_labels(name, &dp->namelen);
	dp->name = regional_alloc_init(region, name, dp->namelen);
	return dp->name != 0;
}

int 
delegpt_add_ns(struct delegpt* dp, struct regional* region, uint8_t* name)
{
	struct delegpt_ns* ns;
	size_t len;
	(void)dname_count_size_labels(name, &len);
	/* slow check for duplicates to avoid counting failures when
	 * adding the same server as a dependency twice */
	if(delegpt_find_ns(dp, name, len))
		return 1;
	ns = (struct delegpt_ns*)regional_alloc(region,
		sizeof(struct delegpt_ns));
	if(!ns)
		return 0;
	ns->next = dp->nslist;
	ns->namelen = len;
	dp->nslist = ns;
	ns->name = regional_alloc_init(region, name, ns->namelen);
	ns->resolved = 0;
	ns->got4 = 0;
	ns->got6 = 0;
	return 1;
}

struct delegpt_ns*
delegpt_find_ns(struct delegpt* dp, uint8_t* name, size_t namelen)
{
	struct delegpt_ns* p = dp->nslist;
	while(p) {
		if(namelen == p->namelen && 
			query_dname_compare(name, p->name) == 0) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}

struct delegpt_addr*
delegpt_find_addr(struct delegpt* dp, struct sockaddr_storage* addr, 
	socklen_t addrlen)
{
	struct delegpt_addr* p = dp->target_list;
	while(p) {
		if(sockaddr_cmp_addr(addr, addrlen, &p->addr, p->addrlen)==0) {
			return p;
		}
		p = p->next_target;
	}
	return NULL;
}

int 
delegpt_add_target(struct delegpt* dp, struct regional* region, 
	uint8_t* name, size_t namelen, struct sockaddr_storage* addr, 
	socklen_t addrlen, int bogus, int nodup)
{
	struct delegpt_ns* ns = delegpt_find_ns(dp, name, namelen);
	if(!ns) {
		/* ignore it */
		return 1;
	}
	if(addr_is_ip6(addr, addrlen))
		ns->got6 = 1;
	else	ns->got4 = 1;
	if(ns->got4 && ns->got6)
		ns->resolved = 1;
	return delegpt_add_addr(dp, region, addr, addrlen, bogus, nodup);
}

int 
delegpt_add_addr(struct delegpt* dp, struct regional* region, 
	struct sockaddr_storage* addr, socklen_t addrlen, int bogus, int nodup)
{
	struct delegpt_addr* a;
	if(nodup) {
		if((a = delegpt_find_addr(dp, addr, addrlen))) {
			if(bogus)
				a->bogus = bogus;
			return 1;
		}
	}

	a = (struct delegpt_addr*)regional_alloc(region,
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
	a->attempts = 0;
	a->bogus = bogus;
	return 1;
}

void
delegpt_count_ns(struct delegpt* dp, size_t* numns, size_t* missing)
{
	struct delegpt_ns* ns;
	*numns = 0;
	*missing = 0;
	for(ns = dp->nslist; ns; ns = ns->next) {
		(*numns)++;
		if(!ns->resolved)
			(*missing)++;
	}
}

void
delegpt_count_addr(struct delegpt* dp, size_t* numaddr, size_t* numres, 
	size_t* numavail)
{
	struct delegpt_addr* a;
	*numaddr = 0;
	*numres = 0;
	*numavail = 0;
	for(a = dp->target_list; a; a = a->next_target) {
		(*numaddr)++;
	}
	for(a = dp->result_list; a; a = a->next_result) {
		(*numres)++;
	}
	for(a = dp->usable_list; a; a = a->next_usable) {
		(*numavail)++;
	}
}

void delegpt_log(enum verbosity_value v, struct delegpt* dp)
{
	char buf[LDNS_MAX_DOMAINLEN+1];
	struct delegpt_ns* ns;
	struct delegpt_addr* a;
	size_t missing=0, numns=0, numaddr=0, numres=0, numavail=0;
	if(verbosity < v)
		return;
	dname_str(dp->name, buf);
	if(dp->nslist == NULL && dp->target_list == NULL) {
		log_info("DelegationPoint<%s>: empty", buf);
		return;
	}
	delegpt_count_ns(dp, &numns, &missing);
	delegpt_count_addr(dp, &numaddr, &numres, &numavail);
	log_info("DelegationPoint<%s>: %u names (%u missing), "
		"%u addrs (%u result, %u avail)", 
		buf, (unsigned)numns, (unsigned)missing, 
		(unsigned)numaddr, (unsigned)numres, (unsigned)numavail);
	if(verbosity >= VERB_ALGO) {
		for(ns = dp->nslist; ns; ns = ns->next) {
			dname_str(ns->name, buf);
			log_info("  %s %s%s%s%s", buf, (ns->resolved?"*":""),
			(ns->got4?" A":""), (ns->got6?" AAAA":""),
			(dp->bogus?" BOGUS":"") );
		}
		for(a = dp->target_list; a; a = a->next_target) {
			if(a->bogus) 
				log_addr(VERB_ALGO, "  BOGUS ", 
					&a->addr, a->addrlen);
			else log_addr(VERB_ALGO, "  ", &a->addr, a->addrlen);
		}
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

/** find NS rrset in given list */
static struct ub_packed_rrset_key*
find_NS(struct reply_info* rep, size_t from, size_t to)
{
	size_t i;
	for(i=from; i<to; i++) {
		if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_NS)
			return rep->rrsets[i];
	}
	return NULL;
}

struct delegpt* 
delegpt_from_message(struct dns_msg* msg, struct regional* region)
{
	struct ub_packed_rrset_key* ns_rrset = NULL;
	struct delegpt* dp;
	size_t i;
	/* look for NS records in the authority section... */
	ns_rrset = find_NS(msg->rep, msg->rep->an_numrrsets, 
		msg->rep->an_numrrsets+msg->rep->ns_numrrsets);

	/* In some cases (even legitimate, perfectly legal cases), the 
	 * NS set for the "referral" might be in the answer section. */
	if(!ns_rrset)
		ns_rrset = find_NS(msg->rep, 0, msg->rep->an_numrrsets);
	
	/* If there was no NS rrset in the authority section, then this 
	 * wasn't a referral message. (It might not actually be a 
	 * referral message anyway) */
	if(!ns_rrset)
		return NULL;
	
	/* If we found any, then Yay! we have a delegation point. */
	dp = delegpt_create(region);
	if(!dp)
		return NULL;
	if(!delegpt_set_name(dp, region, ns_rrset->rk.dname))
		return NULL;
	if(!delegpt_rrset_add_ns(dp, region, ns_rrset))
		return NULL;

	/* add glue, A and AAAA in answer and additional section */
	for(i=0; i<msg->rep->rrset_count; i++) {
		struct ub_packed_rrset_key* s = msg->rep->rrsets[i];
		/* skip auth section. FIXME really needed?*/
		if(msg->rep->an_numrrsets <= i && 
			i < (msg->rep->an_numrrsets+msg->rep->ns_numrrsets))
			continue;

		if(ntohs(s->rk.type) == LDNS_RR_TYPE_A) {
			if(!delegpt_add_rrset_A(dp, region, s, 0))
				return NULL;
		} else if(ntohs(s->rk.type) == LDNS_RR_TYPE_AAAA) {
			if(!delegpt_add_rrset_AAAA(dp, region, s, 0))
				return NULL;
		}
	}
	return dp;
}

int 
delegpt_rrset_add_ns(struct delegpt* dp, struct regional* region,
        struct ub_packed_rrset_key* ns_rrset)
{
	struct packed_rrset_data* nsdata = (struct packed_rrset_data*)
		ns_rrset->entry.data;
	size_t i;
	if(nsdata->security == sec_status_bogus)
		dp->bogus = 1;
	for(i=0; i<nsdata->count; i++) {
		if(nsdata->rr_len[i] < 2+1) continue; /* len + root label */
		if(dname_valid(nsdata->rr_data[i]+2, nsdata->rr_len[i]-2) !=
			(size_t)ldns_read_uint16(nsdata->rr_data[i]))
			continue; /* bad format */
		/* add rdata of NS (= wirefmt dname), skip rdatalen bytes */
		if(!delegpt_add_ns(dp, region, nsdata->rr_data[i]+2))
			return 0;
	}
	return 1;
}

int 
delegpt_add_rrset_A(struct delegpt* dp, struct regional* region,
	struct ub_packed_rrset_key* ak, int nodup)
{
        struct packed_rrset_data* d=(struct packed_rrset_data*)ak->entry.data;
        size_t i;
        struct sockaddr_in sa;
        socklen_t len = (socklen_t)sizeof(sa);
        memset(&sa, 0, len);
        sa.sin_family = AF_INET;
        sa.sin_port = (in_port_t)htons(UNBOUND_DNS_PORT);
        for(i=0; i<d->count; i++) {
                if(d->rr_len[i] != 2 + INET_SIZE)
                        continue;
                memmove(&sa.sin_addr, d->rr_data[i]+2, INET_SIZE);
                if(!delegpt_add_target(dp, region, ak->rk.dname,
                        ak->rk.dname_len, (struct sockaddr_storage*)&sa,
                        len, (d->security==sec_status_bogus), nodup))
                        return 0;
        }
        return 1;
}

int 
delegpt_add_rrset_AAAA(struct delegpt* dp, struct regional* region,
	struct ub_packed_rrset_key* ak, int nodup)
{
        struct packed_rrset_data* d=(struct packed_rrset_data*)ak->entry.data;
        size_t i;
        struct sockaddr_in6 sa;
        socklen_t len = (socklen_t)sizeof(sa);
        memset(&sa, 0, len);
        sa.sin6_family = AF_INET6;
        sa.sin6_port = (in_port_t)htons(UNBOUND_DNS_PORT);
        for(i=0; i<d->count; i++) {
                if(d->rr_len[i] != 2 + INET6_SIZE) /* rdatalen + len of IP6 */
                        continue;
                memmove(&sa.sin6_addr, d->rr_data[i]+2, INET6_SIZE);
                if(!delegpt_add_target(dp, region, ak->rk.dname,
                        ak->rk.dname_len, (struct sockaddr_storage*)&sa,
                        len, (d->security==sec_status_bogus), nodup))
                        return 0;
        }
        return 1;
}

int 
delegpt_add_rrset(struct delegpt* dp, struct regional* region,
        struct ub_packed_rrset_key* rrset)
{
	if(!rrset)
		return 1;
	if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_NS)
		return delegpt_rrset_add_ns(dp, region, rrset);
	else if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_A)
		return delegpt_add_rrset_A(dp, region, rrset, 0);
	else if(ntohs(rrset->rk.type) == LDNS_RR_TYPE_AAAA)
		return delegpt_add_rrset_AAAA(dp, region, rrset, 0);
	log_warn("Unknown rrset type added to delegpt");
	return 1;
}
