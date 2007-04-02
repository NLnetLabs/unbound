/*
 * util/data/msgreply.h - store message and reply data. 
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
 * This file contains a data structure to store a message and its reply.
 */

#ifndef UTIL_DATA_MSGREPLY_H
#define UTIL_DATA_MSGREPLY_H
#include "util/storage/lruhash.h"
struct comm_reply;

/**
 * Structure to store query information that makes answers to queries
 * different.
 */
struct query_info {
	/** salient data on the query: qname, in wireformat. */
	uint8_t* qname;
	/** length of qname (including last 0 octet) */
	size_t qnamesize;
	/** qtype */
	uint16_t qtype;
	/** qclass */
	uint16_t qclass;
	/** does query have CD bit set */
	int has_cd;
};

/**
 * Structure to store DNS query and the reply packet.
 * To use it, copy over the flags from reply and modify using flags from
 * the query (RD,CD if not AA). prepend ID. 
 */
struct reply_info {
	/** the reply packet, skips ID and flags, 
	 * starts with opcode/rcode word */
	uint8_t* reply;
	/** the reply size */
	size_t replysize;
	/** the flags for the answer, host order. */
	uint16_t flags;
};

/**
 * Structure to keep hash table entry for message replies.
 */
struct msgreply_entry {
	/** the hash table key */
	struct query_info key;
	/** the hash table entry, data is struct reply_info* */
	struct lruhash_entry entry;
};

/** 
 * Parse wire query into a queryinfo structure, return 0 on parse error. 
 * initialises the (prealloced) queryinfo structure as well.
 * This query structure contains a pointer back info the buffer!
 * This pointer avoids memory allocation. allocqname does memory allocation.
 * @param m: the prealloced queryinfo structure to put query into.
 *    must be unused, or _clear()ed.
 * @param query: the wireformat packet query. starts with ID.
 * @return: 0 on format error.
 */
int query_info_parse(struct query_info* m, ldns_buffer* query);

/**
 * Allocate and copy the qname (obtained from query_info_parse()).
 * @param m: the queryinfo structure.
 * @return: 0 on alloc failure.
 */
int query_info_allocqname(struct query_info* m);

/**
 * Compare two queryinfo structures, on query and type, class. 
 * It is _not_ sorted in canonical ordering.
 * @param m1: struct query_info* , void* here to ease use as function pointer.
 * @param m2: struct query_info* , void* here to ease use as function pointer.
 * @return: 0 = same, -1 m1 is smaller, +1 m1 is larger.
 */
int query_info_compare(void* m1, void* m2);

/** clear out query info structure. */
void query_info_clear(struct query_info* m);

/** helper routine, determine length of dname in buffer, no compression ptrs
  * allowed, returns 0 on parse failure. */
size_t query_dname_len(ldns_buffer* query);

/** lowercase query dname */
void query_dname_tolower(uint8_t* dname, size_t len);

/** clear out reply info structure */
void reply_info_clear(struct reply_info* m);

/** calculate size of struct query_info + reply_info */
size_t msgreply_sizefunc(void* k, void* d);

/** delete msgreply_entry key structure */
void query_entry_delete(void *q, void* arg);

/** delete reply_info data structure */
void reply_info_delete(void* d, void* arg);

/** calculate hash value of query_info, lowercases the qname. */
hashvalue_t query_info_hash(struct query_info *q);

/** 
 * Generate answer from reply_info.
 * @param rep: reply to fill in.
 * @param qflags: flags word from the query.
 * @param buf: buffer to put reply into. Note that the query ID must 
 *	be put in the buffer by caller.
 *	The buffer must be large enough.
 */
void reply_info_answer(struct reply_info* rep, uint16_t qflags, 
	ldns_buffer* buf);

/**
 * Generate and send out answer from reply_info.
 * @param rep: reply to fill in.
 * @param qid: query id, in network byte order.
 * @param qflags: flags word from the query.
 * @param comrep: communication reply point.
 * @param cached: set true if a cached reply (so no AA bit).
 *	set false for the first reply.
 */
void reply_info_answer_iov(struct reply_info* rep, uint16_t qid,
	uint16_t qflags, struct comm_reply* comrep, int cached);

/**
 * Setup query info entry
 * @param q: query info to copy. Emptied as if clear is called.
 * @param r: reply to init data.
 * @param h: hash value.
 * @return: newly allocated message reply cache item.
 */
struct msgreply_entry* query_info_entrysetup(struct query_info* q,
	struct reply_info* r, hashvalue_t h);

#endif /* UTIL_DATA_MSGREPLY_H */
