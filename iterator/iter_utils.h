/*
 * iterator/iter_utils.h - iterative resolver module utility functions.
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

#ifndef ITERATOR_ITER_UTILS_H
#define ITERATOR_ITER_UTILS_H
struct iter_env;
struct config_file;
struct module_env;
struct delegpt_addr;
struct delegpt;
struct regional;
struct msg_parse;
struct ub_randstate;
struct query_info;
struct reply_info;
struct module_qstate;

/**
 * Process config options and set iterator module state.
 * Sets default values if no config is found.
 * @param iter_env: iterator module state.
 * @param cfg: config options.
 * @return 0 on error.
 */
int iter_apply_cfg(struct iter_env* iter_env, struct config_file* cfg);

/**
 * Select a valid, nice target to send query to.
 * Sorting and removing unsuitable targets is combined.
 *
 * @param iter_env: iterator module global state, with ip6 enabled and 
 *	do-not-query-addresses.
 * @param env: environment with infra cache (lameness, rtt info).
 * @param dp: delegation point with result list.
 * @param name: zone name (for lameness check).
 * @param namelen: length of name.
 * @return best target or NULL if no target.
 *	if not null, that target is removed from the result list in the dp.
 */
struct delegpt_addr* iter_server_selection(struct iter_env* iter_env, 
	struct module_env* env, struct delegpt* dp, uint8_t* name, 
	size_t namelen);

/**
 * Allocate dns_msg from parsed msg, in regional.
 * @param pkt: packet.
 * @param msg: parsed message (cleaned and ready for regional allocation).
 * @param regional: regional to use for allocation.
 * @return newly allocated dns_msg, or NULL on memory error.
 */
struct dns_msg* dns_alloc_msg(ldns_buffer* pkt, struct msg_parse* msg, 
	struct regional* regional);

/**
 * Copy a dns_msg to this regional.
 * @param from: dns message, also in regional.
 * @param regional: regional to use for allocation.
 * @return newly allocated dns_msg, or NULL on memory error.
 */
struct dns_msg* dns_copy_msg(struct dns_msg* from, struct regional* regional);

/**
 * Allocate a dns_msg with malloc/alloc structure and store in dns cache.
 * @param env: environment, with alloc structure and dns cache.
 * @param qinf: query info, the query for which answer is stored.
 * @param rep: reply in dns_msg from dns_alloc_msg for example.
 * @param is_referral: If true, then the given message to be stored is a
 *	referral. The cache implementation may use this as a hint.
 * @return 0 on alloc error (out of memory).
 */
int iter_dns_store(struct module_env* env, struct query_info* qinf,
	struct reply_info* rep, int is_referral);

/**
 * Select randomly with n/m probability.
 * For shuffle NS records for address fetching.
 * @param rnd: random table
 * @param n: probability.
 * @param m: divisor for probability.
 * @return true with n/m probability.
 */
int iter_ns_probability(struct ub_randstate* rnd, int n, int m);

/**
 * Mark targets that result in a dependency cycle as done, so they
 * will not get selected as targets.
 * @param qstate: query state.
 * @param dp: delegpt to mark ns in.
 */
void iter_mark_cycle_targets(struct module_qstate* qstate, struct delegpt* dp);

/**
 * See if delegation is useful or offers immediately no targets for 
 * further recursion.
 * @param qstate: query state with RD flag and query name.
 * @param dp: delegpt to check.
 */
int iter_dp_is_useless(struct module_qstate* qstate, struct delegpt* dp);

/**
 * See if delegation is expected to have DNSSEC information (RRSIGs) in 
 * its answers, or not. Inspects delegation point (name), trust anchors,
 * and delegation message (DS RRset) to determine this.
 * @param env: module env with trust anchors.
 * @param dp: delegation point.
 * @param msg: delegation message, with DS if a secure referral.
 * @return 1 if dnssec is expected, 0 if not.
 */
int iter_indicates_dnssec(struct module_env* env, struct delegpt* dp,
	struct dns_msg* msg);

/**
 * See if a message contains DNSSEC.
 * This is examined by looking for RRSIGs. With DNSSEC a valid answer, 
 * nxdomain, nodata, referral or cname reply has RRSIGs in answer or auth 
 * sections, sigs on answer data, SOA, DS, or NSEC/NSEC3 records.
 * @param msg: message to examine.
 * @return true if DNSSEC information was found.
 */
int iter_msg_has_dnssec(struct dns_msg* msg);

#endif /* ITERATOR_ITER_UTILS_H */
