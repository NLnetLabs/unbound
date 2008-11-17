/*
 * iterator/iter_hints.c - iterative resolver module stub and root hints.
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
 * Keep track of stub and root hints, and read those from config.
 */
#include "config.h"
#include "iterator/iter_hints.h"
#include "iterator/iter_delegpt.h"
#include "util/regional.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "util/data/dname.h"

struct iter_hints* 
hints_create()
{
	struct iter_hints* hints = (struct iter_hints*)calloc(1,
		sizeof(struct iter_hints));
	if(!hints)
		return NULL;
	hints->region = regional_create();
	if(!hints->region) {
		hints_delete(hints);
		return NULL;
	}
	return hints;
}

void 
hints_delete(struct iter_hints* hints)
{
	if(!hints) 
		return;
	regional_destroy(hints->region);
	free(hints);
}

/** add hint to delegation hints */
static int
ah(struct delegpt* dp, struct regional* r, const char* sv, const char* ip)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	ldns_rdf* rdf = ldns_dname_new_frm_str(sv);
	if(!rdf) {
		log_err("could not parse %s", sv);
		return 0;
	}
	if(!delegpt_add_ns(dp, r, ldns_rdf_data(rdf)) ||
	   !extstrtoaddr(ip, &addr, &addrlen) ||
	   !delegpt_add_target(dp, r, ldns_rdf_data(rdf), ldns_rdf_size(rdf),
		&addr, addrlen, 0, 1)) {
		ldns_rdf_deep_free(rdf);
		return 0;
	}
	ldns_rdf_deep_free(rdf);
	return 1;
}

/** obtain compiletime provided root hints */
static struct delegpt* 
compile_time_root_prime(struct regional* r, int do_ip4, int do_ip6)
{
	/* from:
	 ;       This file is made available by InterNIC
	 ;       under anonymous FTP as
	 ;           file                /domain/named.cache
	 ;           on server           FTP.INTERNIC.NET
	 ;       -OR-                    RS.INTERNIC.NET
	 ;
	 ;       related version of root zone:   2008051300
	 */
	struct delegpt* dp = delegpt_create(r);
	if(!dp)
		return NULL;
	if(!delegpt_set_name(dp, r, (uint8_t*)"\000"))
		return NULL;
      if(do_ip4) {
	if(!ah(dp, r, "A.ROOT-SERVERS.NET.", "198.41.0.4"))	return 0;
	if(!ah(dp, r, "B.ROOT-SERVERS.NET.", "192.228.79.201")) return 0;
	if(!ah(dp, r, "C.ROOT-SERVERS.NET.", "192.33.4.12"))	return 0;
	if(!ah(dp, r, "D.ROOT-SERVERS.NET.", "128.8.10.90"))	return 0;
	if(!ah(dp, r, "E.ROOT-SERVERS.NET.", "192.203.230.10")) return 0;
	if(!ah(dp, r, "F.ROOT-SERVERS.NET.", "192.5.5.241"))	return 0;
	if(!ah(dp, r, "G.ROOT-SERVERS.NET.", "192.112.36.4"))	return 0;
	if(!ah(dp, r, "H.ROOT-SERVERS.NET.", "128.63.2.53"))	return 0;
	if(!ah(dp, r, "I.ROOT-SERVERS.NET.", "192.36.148.17"))	return 0;
	if(!ah(dp, r, "J.ROOT-SERVERS.NET.", "192.58.128.30"))	return 0;
	if(!ah(dp, r, "K.ROOT-SERVERS.NET.", "193.0.14.129"))	return 0;
	if(!ah(dp, r, "L.ROOT-SERVERS.NET.", "199.7.83.42"))	return 0;
	if(!ah(dp, r, "M.ROOT-SERVERS.NET.", "202.12.27.33"))	return 0;
      }
      if(do_ip6) {
	if(!ah(dp, r, "A.ROOT-SERVERS.NET.", "2001:503:ba3e::2:30")) return 0;
	if(!ah(dp, r, "F.ROOT-SERVERS.NET.", "2001:500:2f::f")) return 0;
	if(!ah(dp, r, "H.ROOT-SERVERS.NET.", "2001:500:1::803f:235")) return 0;
	if(!ah(dp, r, "J.ROOT-SERVERS.NET.", "2001:503:c27::2:30")) return 0;
	if(!ah(dp, r, "K.ROOT-SERVERS.NET.", "2001:7fd::1")) return 0;
	if(!ah(dp, r, "M.ROOT-SERVERS.NET.", "2001:dc3::35")) return 0;
      }
	return dp;
}

/** insert new hint info into hint structure */
static int
hints_insert(struct iter_hints* hints, uint16_t c, struct delegpt* dp,
	int noprime)
{
	struct iter_hints_stub* node = regional_alloc(hints->region,
		sizeof(struct iter_hints_stub));
	uint8_t* nm;
	if(!node)
		return 0;
	nm = regional_alloc_init(hints->region, dp->name, dp->namelen);
	if(!nm)
		return 0;
	node->dp = dp;
	node->noprime = (uint8_t)noprime;
	if(!name_tree_insert(&hints->tree, &node->node, nm, dp->namelen,
		dp->namelabs, c)) {
		log_err("second hints ignored.");
	}
	return 1;
}

/** set stub name */
static int 
read_stubs_name(struct iter_hints* hints, struct config_stub* s, 
	struct delegpt* dp)
{
	ldns_rdf* rdf;
	if(!s->name) {
		log_err("stub zone without a name");
		return 0;
	}
	rdf = ldns_dname_new_frm_str(s->name);
	if(!rdf) {
		log_err("cannot parse stub zone name %s", s->name);
		return 0;
	}
	if(!delegpt_set_name(dp, hints->region, ldns_rdf_data(rdf))) {
		ldns_rdf_deep_free(rdf);
		log_err("out of memory");
		return 0;
	}
	ldns_rdf_deep_free(rdf);
	return 1;
}

/** set stub host names */
static int 
read_stubs_host(struct iter_hints* hints, struct config_stub* s, 
	struct delegpt* dp)
{
	struct config_strlist* p;
	ldns_rdf* rdf;
	for(p = s->hosts; p; p = p->next) {
		log_assert(p->str);
		rdf = ldns_dname_new_frm_str(p->str);
		if(!rdf) {
			log_err("cannot parse stub %s nameserver name: '%s'", 
				s->name, p->str);
			return 0;
		}
		if(!delegpt_add_ns(dp, hints->region, ldns_rdf_data(rdf))) {
			ldns_rdf_deep_free(rdf);
			log_err("out of memory");
			return 0;
		}
		ldns_rdf_deep_free(rdf);
	}
	return 1;
}

/** set stub server addresses */
static int 
read_stubs_addr(struct iter_hints* hints, struct config_stub* s, 
	struct delegpt* dp)
{
	struct config_strlist* p;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	for(p = s->addrs; p; p = p->next) {
		log_assert(p->str);
		if(!extstrtoaddr(p->str, &addr, &addrlen)) {
			log_err("cannot parse stub %s ip address: '%s'", 
				s->name, p->str);
			return 0;
		}
		if(!delegpt_add_addr(dp, hints->region, &addr, addrlen, 0, 1)) {
			log_err("out of memory");
			return 0;
		}
	}
	return 1;
}

/** read stubs config */
static int 
read_stubs(struct iter_hints* hints, struct config_file* cfg)
{
	struct config_stub* s;
	for(s = cfg->stubs; s; s = s->next) {
		struct delegpt* dp = delegpt_create(hints->region);
		if(!dp) {
			log_err("out of memory");
			return 0;
		}
		if(!read_stubs_name(hints, s, dp) ||
			!read_stubs_host(hints, s, dp) ||
			!read_stubs_addr(hints, s, dp))
			return 0;
		if(!hints_insert(hints, LDNS_RR_CLASS_IN, dp, !s->isprime))
			return 0;
		delegpt_log(VERB_QUERY, dp);
	}
	return 1;
}

/** read root hints from file */
static int 
read_root_hints(struct iter_hints* hints, char* fname)
{
	int lineno = 0;
	uint32_t default_ttl = 0;
	ldns_rdf* origin = NULL;
	ldns_rdf* prev_rr = NULL;
	struct delegpt* dp;
	ldns_rr* rr = NULL;
	ldns_status status;
	uint16_t c = LDNS_RR_CLASS_IN;
	FILE* f = fopen(fname, "r");
	if(!f) {
		log_err("could not read root hints %s: %s",
			fname, strerror(errno));
		return 0;
	}
	dp = delegpt_create(hints->region);
	if(!dp) {
		log_err("out of memory reading root hints");
		fclose(f);
		return 0;
	}
	verbose(VERB_QUERY, "Reading root hints from %s", fname);
	while(!feof(f)) {
		status = ldns_rr_new_frm_fp_l(&rr, f, 
			&default_ttl, &origin, &prev_rr, &lineno);
		if(status == LDNS_STATUS_SYNTAX_EMPTY ||
			status == LDNS_STATUS_SYNTAX_TTL ||
			status == LDNS_STATUS_SYNTAX_ORIGIN)
			continue;
		if(status != LDNS_STATUS_OK) {
			log_err("reading root hints %s %d: %s", fname,
				lineno, ldns_get_errorstr_by_id(status));
			goto stop_read;
		}
		if(ldns_rr_get_type(rr) == LDNS_RR_TYPE_NS) {
			if(!delegpt_add_ns(dp, hints->region,
				ldns_rdf_data(ldns_rr_rdf(rr, 0)))) {
				log_err("out of memory reading root hints");
				goto stop_read;
			}
			c = ldns_rr_get_class(rr);
			if(!dp->name) {
				if(!delegpt_set_name(dp, hints->region, 
					ldns_rdf_data(ldns_rr_owner(rr)))){
					log_err("out of memory.");
					goto stop_read;
				}
			}
		} else if(ldns_rr_get_type(rr) == LDNS_RR_TYPE_A) {
			struct sockaddr_in sa;
			socklen_t len = (socklen_t)sizeof(sa);
			memset(&sa, 0, len);
			sa.sin_family = AF_INET;
			sa.sin_port = (in_port_t)htons(UNBOUND_DNS_PORT);
			memmove(&sa.sin_addr, 
				ldns_rdf_data(ldns_rr_rdf(rr, 0)), INET_SIZE);
			if(!delegpt_add_target(dp, hints->region,
					ldns_rdf_data(ldns_rr_owner(rr)),
					ldns_rdf_size(ldns_rr_owner(rr)),
					(struct sockaddr_storage*)&sa, len, 
					0, 1)) {
				log_err("out of memory reading root hints");
				goto stop_read;
			}
		} else if(ldns_rr_get_type(rr) == LDNS_RR_TYPE_AAAA) {
			struct sockaddr_in6 sa;
			socklen_t len = (socklen_t)sizeof(sa);
			memset(&sa, 0, len);
			sa.sin6_family = AF_INET6;
			sa.sin6_port = (in_port_t)htons(UNBOUND_DNS_PORT);
			memmove(&sa.sin6_addr, 
				ldns_rdf_data(ldns_rr_rdf(rr, 0)), INET6_SIZE);
			if(!delegpt_add_target(dp, hints->region,
					ldns_rdf_data(ldns_rr_owner(rr)),
					ldns_rdf_size(ldns_rr_owner(rr)),
					(struct sockaddr_storage*)&sa, len,
					0, 1)) {
				log_err("out of memory reading root hints");
				goto stop_read;
			}
		} else {
			log_warn("root hints %s:%d skipping type %d",
				fname, lineno, ldns_rr_get_type(rr));
		}

		ldns_rr_free(rr);
	}

	if (origin)
		ldns_rdf_deep_free(origin);
	if (prev_rr)
		ldns_rdf_deep_free(prev_rr);
	fclose(f);
	if(!dp->name) {
		log_warn("root hints %s: no NS content", fname);
		return 1;
	}
	if(!hints_insert(hints, c, dp, 0)) {
		return 0;
	}
	delegpt_log(VERB_QUERY, dp);
	return 1;

stop_read:
	if (origin)
		ldns_rdf_deep_free(origin);
	if (prev_rr)
		ldns_rdf_deep_free(prev_rr);
	fclose(f);
	return 0;
}

/** read root hints list */
static int 
read_root_hints_list(struct iter_hints* hints, struct config_file* cfg)
{
	struct config_strlist* p;
	for(p = cfg->root_hints; p; p = p->next) {
		log_assert(p->str);
		if(p->str && p->str[0]) {
			char* f = p->str;
			if(cfg->chrootdir && cfg->chrootdir[0] &&
				strncmp(p->str, cfg->chrootdir, 
				strlen(cfg->chrootdir)) == 0)
				f += strlen(cfg->chrootdir);
			if(!read_root_hints(hints, f))
				return 0;
		}
	}
	return 1;
}

int 
hints_apply_cfg(struct iter_hints* hints, struct config_file* cfg)
{
	regional_free_all(hints->region);
	name_tree_init(&hints->tree);
	
	/* read root hints */
	if(!read_root_hints_list(hints, cfg))
		return 0;

	/* read stub hints */
	if(!read_stubs(hints, cfg))
		return 0;

	/* use fallback compiletime root hints */
	if(!hints_lookup_root(hints, LDNS_RR_CLASS_IN)) {
		struct delegpt* dp = compile_time_root_prime(hints->region,
			cfg->do_ip4, cfg->do_ip6);
		verbose(VERB_ALGO, "no config, using builtin root hints.");
		if(!dp) 
			return 0;
		if(!hints_insert(hints, LDNS_RR_CLASS_IN, dp, 0))
			return 0;
	}

	name_tree_init_parents(&hints->tree);
	return 1;
}

struct delegpt* 
hints_lookup_root(struct iter_hints* hints, uint16_t qclass)
{
	uint8_t rootlab = 0;
	struct iter_hints_stub *stub;
	stub = (struct iter_hints_stub*)name_tree_find(&hints->tree,
		&rootlab, 1, 1, qclass);
	if(!stub)
		return NULL;
	return stub->dp;
}

struct iter_hints_stub* 
hints_lookup_stub(struct iter_hints* hints, uint8_t* qname, 
	uint16_t qclass, struct delegpt* cache_dp)
{
	size_t len;
	int labs;
	struct iter_hints_stub *r;

	/* first lookup the stub */
	labs = dname_count_size_labels(qname, &len);
	r = (struct iter_hints_stub*)name_tree_lookup(&hints->tree, qname,
		len, labs, qclass);
	if(!r) return NULL;

	/*
	 * If the stub is same as the delegation we got
	 * And has noprime set, we need to 'prime' to use this stub instead.
	 */
	if(r->noprime && query_dname_compare(cache_dp->name, r->dp->name)==0)
		return r; /* use this stub instead of cached dp */
	
	/* 
	 * If our cached delegation point is above the hint, we need to prime.
	 */
	if(dname_strict_subdomain(r->dp->name, r->dp->namelabs,
		cache_dp->name, cache_dp->namelabs))
		return r; /* need to prime this stub */
	return NULL;
}

size_t 
hints_get_mem(struct iter_hints* hints)
{
	if(!hints) return 0;
	return sizeof(*hints) + regional_get_mem(hints->region);
}
