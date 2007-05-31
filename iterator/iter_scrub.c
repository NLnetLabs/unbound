/*
 * iterator/iter_scrub.c - scrubbing, normalization, sanitization of DNS msgs.
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
 * This file has routine(s) for cleaning up incoming DNS messages from 
 * possible useless or malicious junk in it.
 */
#include "config.h"
#include "iterator/iter_scrub.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/data/msgparse.h"
#include "util/data/dname.h"
#include "util/data/msgreply.h"

/**
 * This routine normalizes a response. This includes removing "irrelevant"
 * records from the answer and additional sections and (re)synthesizing
 * CNAMEs from DNAMEs, if present.
 *
 * @param pkt: packet.
 * @param msg: msg to normalize.
 * @param qinfo: original query.
 * @param region: where to allocate synthesized CNAMEs.
 * @return 0 on error.
 */
static int
scrub_normalize(ldns_buffer* pkt, struct msg_parse* msg, 
	struct query_info* qinfo, struct region* region)
{
	uint8_t* sname = qinfo->qname;
	size_t snamelen = qinfo->qname_len;
	struct rrset_parse* rrset, *prev=NULL;

	if(FLAGS_GET_RCODE(msg->flags) != LDNS_RCODE_NOERROR &&
		FLAGS_GET_RCODE(msg->flags) != LDNS_RCODE_NXDOMAIN)
		return 1;

	/* For the ANSWER section, remove all "irrelevant" records and add
	 * synthesized CNAMEs from DNAMEs
	 * This will strip out-of-order CNAMEs as well. */

	for(rrset = msg->rrset_first; rrset; (prev=rrset),(rrset=rrset->rrset_all_next)) 
	{
		if(rrset->section != LDNS_SECTION_ANSWER)
			continue;
		if(rrset->type == LDNS_RR_TYPE_DNAME && 
			dname_strict_subdomain_c(sname, rrset->dname)) {
			if(rrset->rr_count != 1) {
				verbose(VERB_ALGO, "Found DNAME rrset with "
					"size > 1: %d", rrset->rr_count);
				return 0;
			}
			/* check if next rrset is correct CNAME. else,
			 * synthesize a CNAME */
			/* if nametoolong, return 0 (should been YXDOMAIN) */
			continue;
		}

		/* The only records in the ANSWER section not allowed to */
		if(dname_pkt_compare(pkt, sname, rrset->dname) != 0) {
			/* TODO remove_rrset(pkt, msg, prev, rrset); */
			continue;
		}

		/* Follow the CNAME chain. */
		if(rrset->type == LDNS_RR_TYPE_CNAME) {
			if(rrset->rr_count != 1) {
				verbose(VERB_ALGO, "Found CNAME rrset with "
					"size > 1: %d", rrset->rr_count);
				return 0;
			}
			if(rrset->rr_first->size < sizeof(uint16_t)+1)
				return 0; /* CNAME rdata too small */
			sname = rrset->rr_first->ttl_data + sizeof(uint32_t)
				+ sizeof(uint16_t); /* skip ttl, rdatalen */
			snamelen = rrset->rr_first->size - sizeof(uint16_t);
		}

		/* Otherwise, make sure that the RRset matches the qtype. */
		if(qinfo->qtype != LDNS_RR_TYPE_ANY && 
			qinfo->qtype != rrset->type) {
			/*TODO: remove_rrset(); */
		}

		/* Otherwise, fetch the additional names from the 
		 * relevant rrset. */
		/* store additional names from rrset rdata */
	}

	/* Get additional names from AUTHORITY */
	/* go through authority section, store add. names from rrdat rdata */

	/* For each record in the additional section, remove it if it is an
	 * address record and not in the collection of additional names 
	 * found in ANSWER and AUTHORITY. */
	
	/* FIXME: what about other types? */
	return 1;
}

int 
scrub_message(ldns_buffer* pkt, struct msg_parse* msg, 
	struct query_info* qinfo, uint8_t* zonename, size_t zonelen,
	struct region* region)
{
	/* things to check:
	 * if qdcount > 0 : qinfo.
	 * from normalize() from NSclient.
	 * from sanitize() from iterator.
	 */
	/* basic sanity checks */
	if(msg->qdcount > 1)
		return 0;
	if( !(msg->flags&BIT_QR) )
		return 0;
	
	/* if a query is echoed back, make sure it is correct. Otherwise,
	 * this may be not a reply to our query. */
	if(msg->qdcount == 1) {
		if(dname_pkt_compare(pkt, msg->qname, qinfo->qname) != 0)
			return 0;
		if(msg->qtype != qinfo->qtype || msg->qclass != qinfo->qclass)
			return 0;
	}

	/* normalize the response */
	if(!scrub_normalize(pkt, msg, qinfo, region))
		return 0;
	return 1;
}
