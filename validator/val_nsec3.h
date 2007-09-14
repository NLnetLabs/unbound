/*
 * validator/val_nsec3.h - validator NSEC3 denial of existance functions.
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
 * The functions help with NSEC3 checking, the different NSEC3 proofs
 * for denial of existance, and proofs for presence of types.
 *
 * NSEC3
 *                      1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Hash Alg.   |     Flags     |          Iterations           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Salt Length  |                     Salt                      /
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Hash Length  |             Next Hashed Owner Name            /
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * /                         Type Bit Maps                         /
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * NSEC3PARAM
 *                      1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Hash Alg.   |     Flags     |          Iterations           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Salt Length  |                     Salt                      /
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#ifndef VALIDATOR_VAL_NSEC3_H
#define VALIDATOR_VAL_NSEC3_H
struct val_env;
struct module_env;
struct ub_packed_rrset_key;
enum sec_status;
struct reply_info;
struct query_info;
struct key_entry_key;

/**
 *     0 1 2 3 4 5 6 7
 *    +-+-+-+-+-+-+-+-+
 *    |             |O|
 *    +-+-+-+-+-+-+-+-+
 * The OPT-OUT bit in the NSEC3 flags field.
 * If enabled, there can be zero or more unsigned delegations in the span.
 * If disabled, there are zero unsigned delegations in the span.
 */
#define NSEC3_OPTOUT	0x01
/**
 * The unknown flags in the NSEC3 flags field.
 * They must be zero, or the NSEC3 is ignored.
 */
#define NSEC3_UNKNOWN_FLAGS 0xFE

/** The SHA1 hash algorithm for NSEC3 */
#define NSEC3_HASH_SHA1	0x01

/**
 * Determine if the set of NSEC3 records provided with a response prove NAME
 * ERROR. This means that the NSEC3s prove a) the closest encloser exists,
 * b) the direct child of the closest encloser towards qname doesn't exist,
 * and c) *.closest encloser does not exist.
 *
 * @param env: module environment with temporary region and buffer.
 * @param ve: validator environment, with iteration count settings.
 * @param list: array of RRsets, some of which are NSEC3s.
 * @param num: number of RRsets in the array to examine.
 * @param qinfo: query that is verified for.
 * @param kkey: key entry that signed the NSEC3s.
 * @return:
 * 	sec_status SECURE of the Name Error is proven by the NSEC3 RRs, 
 * 	BOGUS if not, INSECURE if all of the NSEC3s could be validly ignored.
 */
enum sec_status
nsec3_prove_nameerror(struct module_env* env, struct val_env* ve,
	struct ub_packed_rrset_key** list, size_t num, 
	struct query_info* qinfo, struct key_entry_key* kkey);

/**
 * Determine if the NSEC3s provided in a response prove the NOERROR/NODATA
 * status. There are a number of different variants to this:
 * 
 * 1) Normal NODATA -- qname is matched to an NSEC3 record, type is not
 * present.
 * 
 * 2) ENT NODATA -- because there must be NSEC3 record for
 * empty-non-terminals, this is the same as #1.
 * 
 * 3) NSEC3 ownername NODATA -- qname matched an existing, lone NSEC3
 * ownername, but qtype was not NSEC3. NOTE: as of nsec-05, this case no
 * longer exists.
 * 
 * 4) Wildcard NODATA -- A wildcard matched the name, but not the type.
 * 
 * 5) Opt-In DS NODATA -- the qname is covered by an opt-in span and qtype ==
 * DS. (or maybe some future record with the same parent-side-only property)
 *
 * @param env: module environment with temporary region and buffer.
 * @param ve: validator environment, with iteration count settings.
 * @param list: array of RRsets, some of which are NSEC3s.
 * @param num: number of RRsets in the array to examine.
 * @param qinfo: query that is verified for.
 * @param kkey: key entry that signed the NSEC3s.
 * @return:
 * 	sec_status SECURE of the proposition is proven by the NSEC3 RRs, 
 * 	BOGUS if not, INSECURE if all of the NSEC3s could be validly ignored.
 */
enum sec_status
nsec3_prove_nodata(struct module_env* env, struct val_env* ve,
	struct ub_packed_rrset_key** list, size_t num, 
	struct query_info* qinfo, struct key_entry_key* kkey);


/**
 * Prove that a positive wildcard match was appropriate (no direct match
 * RRset).
 *
 * @param env: module environment with temporary region and buffer.
 * @param ve: validator environment, with iteration count settings.
 * @param list: array of RRsets, some of which are NSEC3s.
 * @param num: number of RRsets in the array to examine.
 * @param qinfo: query that is verified for.
 * @param kkey: key entry that signed the NSEC3s.
 * @param wc: The purported wildcard that matched.
 * @return:
 * 	sec_status SECURE of the proposition is proven by the NSEC3 RRs, 
 * 	BOGUS if not, INSECURE if all of the NSEC3s could be validly ignored.
 */
enum sec_status
nsec3_prove_wildcard(struct module_env* env, struct val_env* ve,
	struct ub_packed_rrset_key** list, size_t num, 
	struct query_info* qinfo, struct key_entry_key* kkey, uint8_t* wc);

/**
 * Prove that a DS response either had no DS, or wasn't a delegation point.
 *
 * Fundamentally there are two cases here: normal NODATA and Opt-In NODATA.
 *
 * @param env: module environment with temporary region and buffer.
 * @param ve: validator environment, with iteration count settings.
 * @param list: array of RRsets, some of which are NSEC3s.
 * @param num: number of RRsets in the array to examine.
 * @param qinfo: query that is verified for.
 * @param kkey: key entry that signed the NSEC3s.
 * @return:
 * 	sec_status SECURE of the proposition is proven by the NSEC3 RRs, 
 * 	BOGUS if not, INSECURE if all of the NSEC3s could be validly ignored.
 * 	or if there was no DS in an insecure (i.e., opt-in) way,
 * 	INDETERMINATE if it was clear that this wasn't a delegation point.
 */
enum sec_status
nsec3_prove_nods(struct module_env* env, struct val_env* ve,
	struct ub_packed_rrset_key** list, size_t num, 
	struct query_info* qinfo, struct key_entry_key* kkey);

/**
 * Prove NXDOMAIN or NODATA.
 *
 * @param env: module environment with temporary region and buffer.
 * @param ve: validator environment, with iteration count settings.
 * @param list: array of RRsets, some of which are NSEC3s.
 * @param num: number of RRsets in the array to examine.
 * @param qinfo: query that is verified for.
 * @param kkey: key entry that signed the NSEC3s.
 * @param nodata: if return value is secure, this indicates if nodata or
 * 	nxdomain was proven.
 * @return:
 * 	sec_status SECURE of the proposition is proven by the NSEC3 RRs, 
 * 	BOGUS if not, INSECURE if all of the NSEC3s could be validly ignored.
 */
enum sec_status
nsec3_prove_nxornodata(struct module_env* env, struct val_env* ve,
	struct ub_packed_rrset_key** list, size_t num, 
	struct query_info* qinfo, struct key_entry_key* kkey, int* nodata);

#endif /* VALIDATOR_VAL_NSEC3_H */
