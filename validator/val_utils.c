/*
 * validator/val_utils.c - validator utility functions.
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
 * This file contains helper functions for the validator module.
 */
#include "config.h"
#include "validator/val_utils.h"
#include "util/data/msgreply.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/net_help.h"

enum val_classification 
val_classify_response(struct query_info* qinf, struct reply_info* rep)
{
	int rcode = (int)FLAGS_GET_RCODE(rep->flags);
	size_t i;

	/* Normal Name Error's are easy to detect -- but don't mistake a CNAME
	 * chain ending in NXDOMAIN. */
	if(rcode == LDNS_RCODE_NXDOMAIN && rep->an_numrrsets == 0)
		return VAL_CLASS_NAMEERROR;

	log_assert(rcode == LDNS_RCODE_NOERROR);
	/* Next is NODATA */
	if(rep->an_numrrsets == 0)
		return VAL_CLASS_NODATA;
	
	/* We distinguish between CNAME response and other positive/negative
	 * responses because CNAME answers require extra processing. */

	/* We distinguish between ANY and CNAME or POSITIVE because 
	 * ANY responses are validated differently. */
	if(qinf->qtype == LDNS_RR_TYPE_ANY)
		return VAL_CLASS_ANY;
	
	/* Note that DNAMEs will be ignored here, unless qtype=DNAME. Unless
	 * qtype=CNAME, this will yield a CNAME response. */
	for(i=0; i<rep->an_numrrsets; i++) {
		if(ntohs(rep->rrsets[i]->rk.type) == qinf->qtype)
			return VAL_CLASS_POSITIVE;
		if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_CNAME)
			return VAL_CLASS_CNAME;
	}
	log_dns_msg("validator: failed to classify response message: ",
		qinf, rep);
	return VAL_CLASS_UNKNOWN;
}

/** Get signer name from RRSIG */
static void
rrsig_get_signer(uint8_t* data, size_t len, uint8_t** sname, size_t* slen)
{
	/* RRSIG rdata is not allowed to be compressed, it is stored
	 * uncompressed in memory as well, so return a ptr to the name */
	if(len < 21) {
		/* too short RRSig:
		 * short, byte, byte, long, long, long, short, "." is
		 * 2	1	1	4	4  4	2	1 = 19
		 * 			and a skip of 18 bytes to the name.
		 * +2 for the rdatalen is 21 bytes len for root label */
		*sname = NULL;
		*slen = 0;
		return;
	}
	data += 20; /* skip the fixed size bits */
	len -= 20;
	*slen = dname_valid(data, len);
	if(!*slen) {
		/* bad dname in this rrsig. */
		*sname = NULL;
		return;
	}
	*sname = data;
}

/**
 * Find the signer name for an RRset.
 * @param rrset: the rrset.
 * @param sname: signer name is returned or NULL if not signed.
 * @param slen: length of sname (or 0).
 */
static void 
val_find_rrset_signer(struct ub_packed_rrset_key* rrset, uint8_t** sname,
	size_t* slen)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)
		rrset->entry.data;
	/* return signer for first signature, or NULL */
	if(d->rrsig_count == 0) {
		*sname = NULL;
		*slen = 0;
		return;
	}
	/* get rrsig signer name out of the signature */
	rrsig_get_signer(d->rr_data[d->count], d->rr_len[d->count], 
		sname, slen);
}

void 
val_find_signer(struct query_info* qinf, struct reply_info* rep,
	uint8_t** signer_name, size_t* signer_len)
{
	enum val_classification subtype = val_classify_response(qinf, rep);
	size_t i;
	
	if(subtype == VAL_CLASS_POSITIVE || subtype == VAL_CLASS_CNAME 
		|| subtype == VAL_CLASS_ANY) {
		/* check for the answer rrset */
		for(i=0; i<rep->an_numrrsets; i++) {
			if(query_dname_compare(qinf->qname, 
				rep->rrsets[i]->rk.dname) == 0) {
				val_find_rrset_signer(rep->rrsets[i], 
					signer_name, signer_len);
				return;
			}
		}
		*signer_name = NULL;
		*signer_len = 0;
	} else if(subtype == VAL_CLASS_NAMEERROR 
		|| subtype == VAL_CLASS_NODATA) {
		/*Check to see if the AUTH section NSEC record(s) have rrsigs*/
		for(i=rep->an_numrrsets; i<
			rep->an_numrrsets+rep->ns_numrrsets; i++) {
			if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_NSEC
				|| ntohs(rep->rrsets[i]->rk.type) ==
				LDNS_RR_TYPE_NSEC3) {
				val_find_rrset_signer(rep->rrsets[i], 
					signer_name, signer_len);
				return;
			}
		}
	} else {
		verbose(VERB_ALGO, "find_signer: could not find signer name"
			" for unknown type response");
		*signer_name = NULL;
		*signer_len = 0;
	}
}
