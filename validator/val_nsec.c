/*
 * validator/val_nsec.c - validator NSEC denial of existance functions.
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
 * The functions help with NSEC checking, the different NSEC proofs
 * for denial of existance, and proofs for presence of types.
 */
#include "config.h"
#include "validator/val_nsec.h"
#include "validator/val_utils.h"
#include "util/data/msgreply.h"
#include "util/data/dname.h"

/** Check type present in NSEC typemap with bitmap arg */
static int
nsec_has_type_rdata(uint8_t* bitmap, size_t len, uint16_t type)
{
	/* bitmasks for determining type-lowerbits presence */
	uint8_t masks[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint8_t type_window = type>>8;
	uint8_t type_low = type&0xff;
	uint8_t win, winlen;
	/* read each of the type bitmap windows and see if the searched
	 * type is amongst it */
	while(len > 0) {
		if(len < 3) /* bad window, at least window# winlen bitmap */
			return 0;
		win = *bitmap++;
		winlen = *bitmap++;
		len -= 2;
		if(len < winlen || winlen < 1 || winlen > 32) 
			return 0;	/* bad window length */
		if(win == type_window) {
			/* search window bitmap for the correct byte */
			/* mybyte is 0 if we need the first byte */
			size_t mybyte = type_low>>3;
			if(winlen <= mybyte)
				return 0; /* window too short */
			return bitmap[mybyte] & masks[type_low&0x7];
		} else {
			/* not the window we are looking for */
			bitmap += winlen;
			len -= winlen;
		}
	}
	/* end of bitmap reached, no type found */
	return 0;
}

int
unitest_nsec_has_type_rdata(char* bitmap, size_t len, uint16_t type)
{
	return nsec_has_type_rdata((uint8_t*)bitmap, len, type);
}

/**
 * Check if type is present in the NSEC typemap
 * @param nsec: the nsec RRset.
 *	If there are multiple RRs, then each must have the same typemap,
 *	since the typemap represents the types at this domain node.
 * @param type: type to check for, host order.
 * @return true if present
 */
static int
nsec_has_type(struct ub_packed_rrset_key* nsec, uint16_t type)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)nsec->
		entry.data;
	size_t len;
	if(!d || d->count == 0 || d->rr_len[0] < 2+1)
		return 0;
	len = dname_valid(d->rr_data[0]+2, d->rr_len[0]-2);
	if(!len)
		return 0;
	nsec_has_type_rdata(d->rr_data[0]+2+len, d->rr_len[0]-2-len, type);
}

/**
 * For an NSEC that matches the DS queried for, check absence of DS type.
 *
 * @param nsec: NSEC for proof, must be trusted.
 * @param qinfo: what is queried for.
 * @return if secure the nsec proves that no DS is present, or 
 *	insecure if it proves it is not a delegation point.
 *	or bogus if something was wrong.
 */
enum sec_status 
val_nsec_proves_no_ds(struct ub_packed_rrset_key* nsec, 
	struct query_info* qinfo)
{
	log_assert(qinfo->qtype == LDNS_RR_TYPE_DS);
	log_assert(ntohs(nsec->rk.type) == LDNS_RR_TYPE_NSEC);
	/* this proof may also work if qname is a subdomain */
	log_assert(query_dname_compare(nsec->rk.dname, qinfo->qname) == 0);

	return sec_status_bogus;
}

enum sec_status 
val_nsec_prove_nodata_ds(struct module_env* env, struct val_env* ve, 
	struct query_info* qinfo, struct reply_info* rep, 
	struct key_entry_key* kkey, uint32_t* proof_ttl)
{
	struct ub_packed_rrset_key* nsec = reply_find_rrset_section_ns(
		rep, qinfo->qname, qinfo->qname_len, LDNS_RR_TYPE_NSEC, 
		qinfo->qclass);

	/* If we have a NSEC at the same name, it must prove one 
	 * of two things
	 * --
	 * 1) this is a delegation point and there is no DS
	 * 2) this is not a delegation point */
	if(nsec) {
		enum sec_status sec = val_verify_rrset_entry(env, ve, nsec, 
			kkey);
		if(sec != sec_status_secure) {
			verbose(VERB_ALGO, "NSEC RRset for the "
				"referral did not verify.");
			return sec_status_bogus;
		}
		sec = val_nsec_proves_no_ds(nsec, qinfo);
		if(sec == sec_status_bogus) {
			/* something was wrong. */
			return sec;
		} else if(sec == sec_status_insecure) {
			/* this wasn't a delegation point. */
			return sec;
		} else if(sec == sec_status_secure) {
			/* this proved no DS. */
			*proof_ttl = ub_packed_rrset_ttl(nsec);
			return sec;
		}
		/* if unchecked, fall through to next proof */
	}

	/* Otherwise, there is no NSEC at qname. This could be an ENT. 
	 * (ENT=empty non terminal). If not, this is broken. */
	
	/* verify NSEC rrsets in auth section, call */
	/* ValUtils.nsecProvesNodata, if so: NULL entry */

	return sec_status_bogus;
}
