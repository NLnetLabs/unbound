/*
 * services/rpz.c - rpz service
 *
 * Copyright (c) 2019, NLnet Labs. All rights reserved.
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
 * This file contains functions to enable RPZ service.
 */

#include "config.h"
#include "services/rpz.h"
#include "util/config_file.h"
#include "sldns/wire2str.h"
#include "util/data/dname.h"
#include "util/net_help.h"
#include "util/log.h"
#include "util/data/dname.h"
#include "util/locks.h"

/** string for RPZ action enum */
static const char*
rpz_action_to_string(enum rpz_action a)
{
	switch(a) {
	case RPZ_NXDOMAIN_ACTION:	return "NXDOMAIN ACTION";
	case RPZ_NODATA_ACTION:		return "NODATA ACTION";
	case RPZ_PASSTHRU_ACTION:	return "PASSTHRU ACTION";
	case RPZ_DROP_ACTION:		return "DROP ACTION";
	case RPZ_TCP_ONLY_ACTION:	return "TCP ONLY ACTION";
	case RPZ_INVALID_ACTION:	return "INVALID ACTION";
	case RPZ_LOCAL_DATA_ACTION:	return "LOCAL DATA ACTION";
	}
	return "UNKNOWN RPZ ACTION";
}

/** string for RPZ trigger enum */
static const char*
rpz_trigger_to_string(enum rpz_trigger r)
{
	switch(r) {
	case RPZ_QNAME_TRIGGER:		return "QNAME TRIGGER";
	case RPZ_CLIENT_IP_TRIGGER:	return "CLIENT IP TRIGGER";
	case RPZ_RESPONSE_IP_TRIGGER:	return "RESPONSE IP TRIGGER";
	case RPZ_NSDNAME_TRIGGER:	return "NSDNAME TRIGGER";
	case RPZ_NSIP_TRIGGER:		return "NSIP TRIGGER";
	}
	return "UNKNOWN RPZ TRIGGER";
}

/**
 * Get the label that is just before the root label.
 * @param dname: dname to work on
 * @return: pointer to TLD label
 */
static uint8_t*
get_tld_label(uint8_t* dname)
{
	uint8_t* prevlab = dname;

	/* only root label */
	if(*dname == 0)
		return NULL;

	while(*dname) {
		dname = dname+*dname+1;
		if(*dname != 0)
			prevlab = dname;
	}
	return prevlab;
}

/**
 * Classify RPZ action for RR type/rdata
 * @param rr_type: the RR type
 * @param rdatawl: RDATA with 2 bytes length
 * @param rdatalen: the length of rdatawl (including its 2 bytes length)
 * @return: the RPZ action
 */
static enum rpz_action
rpz_rr_to_action(uint16_t rr_type, uint8_t* rdatawl, size_t rdatalen)
{
	char* endptr;
	uint8_t* rdata;
	int rdatalabs;
	uint8_t* tldlab = NULL;

	switch(rr_type) {
		case LDNS_RR_TYPE_SOA:
		case LDNS_RR_TYPE_NS:
		case LDNS_RR_TYPE_DNAME:
		/* all DNSSEC-related RRs must be ignored */
		case LDNS_RR_TYPE_DNSKEY:
		case LDNS_RR_TYPE_DS:
		case LDNS_RR_TYPE_RRSIG:
		case LDNS_RR_TYPE_NSEC:
		case LDNS_RR_TYPE_NSEC3:
			return RPZ_INVALID_ACTION;
		case LDNS_RR_TYPE_CNAME:
			break;
		default:
			return RPZ_LOCAL_DATA_ACTION;
	}

	/* use CNAME target to determine RPZ action */
	log_assert(rr_type == LDNS_RR_TYPE_CNAME);
	if(rdatalen < 3)
		return RPZ_INVALID_ACTION;

	rdata = rdatawl + 2; /* 2 bytes of rdata length */
	if(dname_valid(rdata, rdatalen-2) != rdatalen-2)
		return RPZ_INVALID_ACTION;

	rdatalabs = dname_count_labels(rdata);
	if(rdatalabs == 1)
		return RPZ_NXDOMAIN_ACTION;
	else if(rdatalabs == 2) {
		if(dname_subdomain_c(rdata, (uint8_t*)&"\001*\000"))
			return RPZ_NODATA_ACTION;
		else if(dname_subdomain_c(rdata,
			(uint8_t*)&"\014rpz-passthru\000"))
			return RPZ_PASSTHRU_ACTION;
		else if(dname_subdomain_c(rdata, (uint8_t*)&"\010rpz-drop\000"))
			return RPZ_DROP_ACTION;
		else if(dname_subdomain_c(rdata,
			(uint8_t*)&"\014rpz-tcp-only\000"))
			return RPZ_TCP_ONLY_ACTION;
	}

	/* all other TLDs starting with "rpz-" are invalid */
	tldlab = get_tld_label(rdata);
	if(tldlab && dname_lab_startswith(tldlab, "rpz-", &endptr))
		return RPZ_INVALID_ACTION;

	/* no special label found */
	return RPZ_LOCAL_DATA_ACTION;
}

/**
 * Get RPZ trigger for dname
 * @param dname: dname containing RPZ trigger
 * @return: RPZ trigger enum
 */
static enum rpz_trigger
rpz_dname_to_trigger(uint8_t* dname)
{
	uint8_t* tldlab;
	char* endptr;
	tldlab = get_tld_label(dname);
	if(!tldlab || !dname_lab_startswith(tldlab, "rpz-", &endptr))
		return RPZ_QNAME_TRIGGER;

	if(dname_subdomain_c(tldlab,
		(uint8_t*)&"\015rpz-client-ip\000"))
		return RPZ_CLIENT_IP_TRIGGER;
	else if(dname_subdomain_c(tldlab, (uint8_t*)&"\006rpz-ip\000"))
		return RPZ_RESPONSE_IP_TRIGGER;
	else if(dname_subdomain_c(tldlab, (uint8_t*)&"\013rpz-nsdname\000"))
		return RPZ_NSDNAME_TRIGGER;
	else if(dname_subdomain_c(tldlab, (uint8_t*)&"\010rpz-nsip\000"))
		return RPZ_NSIP_TRIGGER;

	return RPZ_QNAME_TRIGGER;
}

void rpz_delete(struct rpz* r)
{
	if(!r)
		return;
	local_zones_delete(r->local_zones);
	free(r->taglist);
	free(r);
}

int
rpz_clear_lz(struct rpz* r)
{
	/* must hold write lock on auth_zone */
	local_zones_delete(r->local_zones);
	if(!(r->local_zones = local_zones_create())){
		return 0;
	}
	return 1;
}

struct rpz*
rpz_create(struct config_auth* p)
{
	struct rpz* r = calloc(1, sizeof(*r));
	if(!r)
		return 0;

	if(!(r->local_zones = local_zones_create())){
		free(r);
		return 0;
	}
	r->taglist = memdup(p->rpz_taglist, p->rpz_taglistlen);
	r->taglistlen = p->rpz_taglistlen;
	return r;
}

/** Remove RPZ zone name from dname */
static size_t
strip_dname_origin(uint8_t* dname, size_t dnamelen, size_t originlen,
	uint8_t* newdname)
{
	size_t newdnamelen;
	if(dnamelen < originlen)
		return 0;
	newdnamelen = dnamelen - originlen;
	memmove(newdname, dname, newdnamelen);
	return newdnamelen + 1;	/* + 1 for root label */
}

/** Insert RR into RPZ's local-zone */
static int
rpz_insert_qname_trigger(struct rpz* r, uint8_t* dname, size_t dnamelen,
	enum rpz_action a, uint16_t rrtype, uint16_t rrclass, uint32_t ttl,
	uint8_t* rdata, size_t rdata_len, uint8_t* rr, size_t rr_len)
{
	struct local_zone* z;
	enum localzone_type tp = local_zone_always_transparent;
	int dnamelabs = dname_count_labels(dname);
	char* rrstr;

	if(a == RPZ_NXDOMAIN_ACTION)
		tp = local_zone_always_nxdomain;
	else if(a == RPZ_NODATA_ACTION)
		tp = local_zone_always_nodata;
	else if(a == RPZ_DROP_ACTION)
		tp = local_zone_deny;
	else if(a == RPZ_PASSTHRU_ACTION)
		tp = local_zone_always_transparent;
	else if(a == RPZ_LOCAL_DATA_ACTION)
		tp = local_zone_redirect;
	else {
		verbose(VERB_ALGO, "RPZ: skipping unusupported action: %s",
			rpz_action_to_string(a));
		return 0;
	}

	lock_rw_wrlock(&r->local_zones->lock);
	/* exact match */
	z = local_zones_find(r->local_zones, dname, dnamelen, dnamelabs,
		LDNS_RR_CLASS_IN);
	if(z && a != RPZ_LOCAL_DATA_ACTION) {
		rrstr = sldns_wire2str_rr(rr, rr_len);
		verbose(VERB_ALGO, "RPZ: skipping duplicate record: '%s'",
			rrstr);
		free(rrstr);
		lock_rw_unlock(&r->local_zones->lock);
		return 0;
	}
	if(!z) {
		z = local_zones_add_zone(r->local_zones, dname, dnamelen,
			dnamelabs, rrclass, tp);
	}
	if(!z) {
		log_warn("RPZ create failed");
		lock_rw_unlock(&r->local_zones->lock);
		return 0;
	}
	if(a == RPZ_LOCAL_DATA_ACTION) {
		rrstr = sldns_wire2str_rr(rr, rr_len);
		/* TODO non region alloc so rrs can be free after IXFR deletion?
		 * */
		local_zone_enter_rr(z, dname, dnamelen, dnamelabs,
			rrtype, rrclass, ttl, rdata, rdata_len, rrstr);
		free(rrstr);
	}
	lock_rw_unlock(&r->local_zones->lock);
	return 1;
}

void
rpz_insert_rr(struct rpz* r, size_t aznamelen, uint8_t* dname,
	size_t dnamelen, uint16_t rr_type, uint16_t rr_class, uint32_t rr_ttl,
	uint8_t* rdatawl, size_t rdatalen, uint8_t* rr, size_t rr_len)
{
	size_t policydnamelen;
	/* name is free'd in local_zone delete */
	uint8_t* policydname = calloc(1, LDNS_MAX_DOMAINLEN + 1);
	enum rpz_trigger t;
	enum rpz_action a;
	
	a = rpz_rr_to_action(rr_type, rdatawl, rdatalen);
	if(!(policydnamelen = strip_dname_origin(dname, dnamelen, aznamelen,
		policydname))) {
		free(policydname);
		return;
	}
	t = rpz_dname_to_trigger(policydname);
	if(t == RPZ_QNAME_TRIGGER) {
		rpz_insert_qname_trigger(r, policydname, policydnamelen,
			a, rr_type, rr_class, rr_ttl, rdatawl, rdatalen, rr,
			rr_len);
	}
	else {
		free(policydname);
		verbose(VERB_ALGO, "RPZ: skipping unusupported trigger: %s",
			rpz_trigger_to_string(t));
	}
}

/**
 * Find RPZ local-zone by qname.
 * @param r: rpz containing local-zone tree
 * @param qinfo: qinfo struct
 * @param only_exact: if 1 only excact (non wildcard) matches are returned
 * @param wr: get write lock for local-zone if 1, read lock if 0
 * @return: NULL or local-zone holding rd or wr lock
 */
static struct local_zone*
rpz_find_zone(struct rpz* r, uint8_t* qname, size_t qname_len, uint16_t qclass,
	int only_exact, int wr)
{
	uint8_t* ce;
	size_t ce_len, ce_labs;
	uint8_t wc[LDNS_MAX_DOMAINLEN];
	int exact;
	struct local_zone* z = NULL;
	if(wr)
		lock_rw_wrlock(&r->local_zones->lock);
	else
		lock_rw_rdlock(&r->local_zones->lock);
	z = local_zones_find_le(r->local_zones, qname, qname_len,
		dname_count_labels(qname),
		LDNS_RR_CLASS_IN, &exact);
	if(!z || (only_exact && !exact)) {
		lock_rw_unlock(&r->local_zones->lock);
		return NULL;
	}
	if(wr)
		lock_rw_wrlock(&z->lock);
	else
		lock_rw_rdlock(&z->lock);
	lock_rw_unlock(&r->local_zones->lock);

	if(exact)
		return z;

	/* No exact match found, lookup wildcard. closest encloser must
	 * be the shared parent between the qname and the best local
	 * zone match, append '*' to that and do another lookup. */

	ce = dname_get_shared_topdomain(z->name, qname);
	if(!ce /* should not happen */ || !*ce /* root */) {
		lock_rw_unlock(&z->lock);
		return NULL;
	}
	ce_labs = dname_count_size_labels(ce, &ce_len);
	if(ce_len+2 > sizeof(wc)) {
		lock_rw_unlock(&z->lock);
		return NULL;
	}
	wc[0] = 1; /* length of wildcard label */
	wc[1] = (uint8_t)'*'; /* wildcard label */
	memmove(wc+2, ce, ce_len);
	lock_rw_unlock(&z->lock);

	if(wr)
		lock_rw_wrlock(&r->local_zones->lock);
	else
		lock_rw_rdlock(&r->local_zones->lock);
	z = local_zones_find_le(r->local_zones, wc,
		ce_len+2, ce_labs+1, qclass, &exact);
	if(!z || !exact) {
		lock_rw_unlock(&r->local_zones->lock);
		return NULL;
	}
	if(wr)
		lock_rw_wrlock(&z->lock);
	else
		lock_rw_rdlock(&z->lock);
	lock_rw_unlock(&r->local_zones->lock);
	return z;
}

/**
 * Remove RR from RPZ's local-data
 * @param z: local-zone for RPZ, holding write lock
 * @param policydname: dname of RR to remove
 * @param policydnamelen: lenth of policydname
 * @param rr_type: RR type of RR to remove
 * @param rdata: rdata of RR to remove
 * @param rdatalen: length of rdata
 * @return: 1 if zone must be removed after RR deletion
 */
static int
rpz_data_delete_rr(struct local_zone* z, uint8_t* policydname,
	size_t policydnamelen, uint16_t rr_type, uint8_t* rdata,
	size_t rdatalen)
{
	struct local_data* ld;
	struct packed_rrset_data* d;
	size_t index;
	ld = local_zone_find_data(z, policydname, policydnamelen,
		dname_count_labels(policydname));
	if(ld) {
		struct local_rrset* prev=NULL, *p=ld->rrsets;
		while(p && ntohs(p->rrset->rk.type) != rr_type) {
			prev = p;
			p = p->next;
		}
		if(!p)
			return 0;
		d = (struct packed_rrset_data*)p->rrset->entry.data;
		if(packed_rrset_find_rr(d, rdata, rdatalen, &index)) {
			if(d->count == 1) {
				/* no memory recycling for zone deletions ... */
				if(prev) prev->next = p->next;
				else ld->rrsets = p->next;

			}
			if(d->count > 1) {
				struct packed_rrset_data* new;
				new = packed_rrset_remove_rr(d, index, z->region);
				if(!new)
					return 0;
				p->rrset->entry.data = new;	
			}
		}
	}
	if(ld && ld->rrsets)
		return 0;
	return 1;
}

void
rpz_remove_rr(struct rpz* r, size_t aznamelen, uint8_t* dname,
	size_t dnamelen, uint16_t rr_type, uint16_t rr_class, uint8_t* rdatawl,
	size_t rdatalen, uint8_t* rr, size_t rr_len)
{
	struct local_zone* z;
	size_t policydnamelen;
	/* name is free'd in local_zone delete */
	uint8_t* policydname = calloc(1, LDNS_MAX_DOMAINLEN + 1);
	enum rpz_trigger t;
	enum rpz_action a;
	int delete_zone = 1;

	/* TODO, use for logging */
	(void)rr;
	(void)rr_len;
	
	a = rpz_rr_to_action(rr_type, rdatawl, rdatalen);
	if(!(policydnamelen = strip_dname_origin(dname, dnamelen, aznamelen,
		policydname))) {
		free(policydname);
		return;
	}
	t = rpz_dname_to_trigger(policydname);
	if(a != RPZ_INVALID_ACTION && t != RPZ_QNAME_TRIGGER) {
		z = rpz_find_zone(r, policydname, policydnamelen, rr_class,
			1 /* only exact */, 1 /* wr lock */);
		if(!z) {
			verbose(VERB_ALGO, "RPZ: cannot remove RR from IXFR, "
				"RPZ domain not found");
			free(policydname);
			return;
		}
		if(a == RPZ_LOCAL_DATA_ACTION)
			delete_zone = rpz_data_delete_rr(z, policydname,
				policydnamelen, rr_type, rdatawl, rdatalen);
		lock_rw_unlock(&z->lock); 
		if(delete_zone) {
			local_zones_del_zone(r->local_zones, z);
		}
	}
	free(policydname);
}

/** print log information for an applied RPZ policy. Based on local-zone's
 * lz_inform_print().
 */
static void
rpz_inform_print(struct local_zone* z, struct query_info* qinfo,
	struct comm_reply* repinfo)
{
	char ip[128], txt[512];
	char zname[LDNS_MAX_DOMAINLEN+1];
	uint16_t port = ntohs(((struct sockaddr_in*)&repinfo->addr)->sin_port);
	dname_str(z->name, zname);
	addr_to_str(&repinfo->addr, repinfo->addrlen, ip, sizeof(ip));
	snprintf(txt, sizeof(txt), "RPZ applied %s %s %s@%u", zname,
		local_zone_type2str(z->type), ip, (unsigned)port);
	log_nametypeclass(0, txt, qinfo->qname, qinfo->qtype, qinfo->qclass);
}

int
rpz_apply_qname_trigger(struct auth_zones* az, struct module_env* env,
	struct query_info* qinfo, struct edns_data* edns, sldns_buffer* buf,
	struct regional* temp, struct comm_reply* repinfo,
	uint8_t* taglist, size_t taglen)
{
	struct rpz* r;
	int ret;
	struct local_zone* z = NULL;
	struct local_data* ld = NULL;
	lock_rw_rdlock(&az->rpz_lock);
	for(r = az->rpz_first; r && !z; r = r->next) {
		if(!r->taglist || taglist_intersect(r->taglist, 
			r->taglistlen, taglist, taglen))
			z = rpz_find_zone(r, qinfo->qname, qinfo->qname_len,
				qinfo->qclass, 0, 0);
	}
	lock_rw_unlock(&az->rpz_lock);
	if(!z)
		return 0;

	if(z->type == local_zone_redirect && local_data_answer(z, env, qinfo,
		edns, repinfo, buf, temp, dname_count_labels(qinfo->qname),
		&ld, z->type, -1, NULL, 0, NULL, 0)) {
		rpz_inform_print(z, qinfo, repinfo);
		lock_rw_unlock(&z->lock);
		return !qinfo->local_alias;
	}

	ret = local_zones_zone_answer(z, env, qinfo, edns, repinfo, buf, temp,
		0 /* no local data used */, z->type);
	rpz_inform_print(z, qinfo, repinfo);
	lock_rw_unlock(&z->lock);

	return ret;
}
