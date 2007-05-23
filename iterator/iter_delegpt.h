/*
 * iterator/iter_delegpt.h - delegation point with NS and address information.
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

#ifndef ITERATOR_ITER_DELEGPT_H
#define ITERATOR_ITER_DELEGPT_H
struct region;
struct delegpt_ns;
struct delegpt_addr;

/**
 * Delegation Point.
 * For a domain name, the NS rrset, and the A and AAAA records for those.
 */
struct delegpt {
	/** the domain name of the delegation point. */
	uint8_t* name;
	/** length of the delegation point name */
	size_t namelen;
	/** number of labels in delegation point */
	int namelabs;

	/** the nameservers, names from the NS RRset rdata. */
	struct delegpt_ns* nslist;
	/** the target addresses for delegation */
	struct delegpt_addr* target_list;
	/** the list of usable targets; subset of target_list */
	struct delegpt_addr* usable_list;
	/** the list of returned targets; subset of target_list */
	struct delegpt_addr* result_list;
};

/**
 * Nameservers for a delegation point.
 */
struct delegpt_ns {
	/** next in list */
	struct delegpt_ns* next;
	/** name of nameserver */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** 
	 * If the name has been resolved. false if not queried for yet.
	 * true if the address is known, or marked true if failed.
	 */
	int resolved;
};

/**
 * Address of target nameserver in delegation point.
 */
struct delegpt_addr {
	/** next delegation point in results */
	struct delegpt_addr* next_result;
	/** next delegation point in usable list */
	struct delegpt_addr* next_usable;
	/** next delegation point in all targets list */
	struct delegpt_addr* next_target;

	/** delegation point address */
	struct sockaddr_storage addr;
	/** length of addr */
	socklen_t addrlen;
};

/**
 * Create new delegation point.
 * @param region: where to allocate it.
 * @return new delegation point or NULL on error.
 */
struct delegpt* delegpt_create(struct region* region);

/**
 * Set name of delegation point.
 * @param dp: delegation point.
 * @param region: where to allocate the name copy.
 * @param name: name to use.
 * @return false on error.
 */
int delegpt_set_name(struct delegpt* dp, struct region* region, uint8_t* name);

/**
 * Add a name to the delegation point.
 * @param dp: delegation point.
 * @param region: where to allocate the info.
 * @param name: domain name in wire format.
 * @return false on error.
 */
int delegpt_add_ns(struct delegpt* dp, struct region* region, uint8_t* name);

/**
 * Add address to the delegation point.
 * @param dp: delegation point.
 * @param region: where to allocate the info.
 * @param name: name for which target was found (must be in nslist).
 *	This name is marked resolved.
 * @param namelen: length of name.
 * @param addr: the address.
 * @param addrlen: the length of addr.
 * @return false on error.
 */
int delegpt_add_target(struct delegpt* dp, struct region* region, 
	uint8_t* name, size_t namelen, struct sockaddr_storage* addr, 
	socklen_t addrlen);

/**
 * Print the delegation point to the log. For debugging.
 * @param dp: delegation point.
 */
void delegpt_log(struct delegpt* dp);

#endif /* ITERATOR_ITER_DELEGPT_H */
