/*
 * iterator/iter_resptype.c - response type information and classification.
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
 * This file defines the response type. DNS Responses can be classified as
 * one of the response types.
 */
#include "config.h"
#include "iterator/iter_resptype.h"
#include "services/cache/dns.h"
#include "util/net_help.h"
#include "util/data/dname.h"

enum response_type 
response_type_from_cache(struct dns_msg* msg, 
	struct query_info* request)
{
	/* If the message is NXDOMAIN, then it is an ANSWER. */
	if(FLAGS_GET_RCODE(msg->rep->flags) == LDNS_RCODE_NXDOMAIN)
		return RESPONSE_TYPE_ANSWER;
	
	/* First we look at the answer section. This can tell us if this is
	 * CNAME or positive ANSWER. */
	if(msg->rep->an_numrrsets > 0) {
		/* Now look at the answer section first. 3 states: 
		 *	o our answer is there directly,
		 *	o our answer is there after a cname,
		 *	o or there is just a cname. */
		uint8_t* mname = request->qname;
		size_t mname_len = request->qname_len;
		size_t i;
		for(i=0; i<msg->rep->an_numrrsets; i++) {
			struct ub_packed_rrset_key* s = msg->rep->rrsets[i];

			/* If we have encountered an answer (before or 
			 * after a CNAME), then we are done! Note that 
			 * if qtype == CNAME then this will be noted as 
			 * an ANSWER before it gets treated as a CNAME, 
			 * as it should */
			if(ntohs(s->rk.type) == request->qtype &&
				ntohs(s->rk.rrset_class) == request->qclass &&
				query_dname_compare(mname, s->rk.dname) == 0) {
				return RESPONSE_TYPE_ANSWER;
			}

			/* If we have encountered a CNAME, make sure that 
			 * it is relevant. */
			if(ntohs(s->rk.type) == LDNS_RR_TYPE_CNAME &&
				query_dname_compare(mname, s->rk.dname) == 0) {
				get_cname_target(s, &mname, &mname_len);
			}
		}

		/* if we encountered a CNAME (or a bunch of CNAMEs), and 
		 * still got to here, then it is a CNAME response. (i.e., 
		 * the CNAME chain didn't terminate in an answer rrset.) */
		if(mname != request->qname) {
			return RESPONSE_TYPE_CNAME;
		}
	}

	/* At this point, since we don't need to detect REFERRAL or LAME 
	 * messages, it can only be an ANSWER. */
	return RESPONSE_TYPE_ANSWER;
}
