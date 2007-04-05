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
#include "util/data/packed_rrset.h"
struct comm_reply;
struct alloc_cache;

/**
 * Structure to store query information that makes answers to queries
 * different.
 */
struct query_info {
	/** 
	 * Salient data on the query: qname, in wireformat. 
	 * can be allocated or a pointer to outside buffer.
	 * User has to keep track on the status of this.
	 */
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
 * Information to reference an rrset
 */
struct rrset_ref {
	/** the key with lock, and ptr to packed data. */
	struct packed_rrset_key* key;
	/** id needed */
	rrset_id_t id;
};

/**
 * Structure to store DNS query and the reply packet.
 * To use it, copy over the flags from reply and modify using flags from
 * the query (RD,CD if not AA). prepend ID. 
 *
 * Memory layout is:
 *	o struct
 *	o rrset_ref array
 *	o packed_rrset_key* array.
 */
struct reply_info {
	/** the reply packet, skips ID and flags, 
	 * starts with opcode/rcode word */
	uint8_t* reply;
	/** the reply size */
	size_t replysize;

	/** the flags for the answer, host byte order. */
	uint16_t flags;

	/** 
	 * TTL of the entire reply (for negative caching).
	 * only for use when there are 0 RRsets in this message.
	 * if there are RRsets, check those instead.
	 */
	uint32_t ttl;

	/** 
	 * network order counts: qdcount ancount nscount arcount. 
	 * so this is wireformat for the counts as they appear in the message.
	 * If qdcount is not 0, then it is 1, and the data that appears
	 * in the reply is the same as the query_info.
	 */
	uint16_t counts[4];

	/** Total number of rrsets in reply: ancount+nscount+arcount. 
	 * Use the accessor function to get this value.
	 */
	size_t num_rrsets;

	/** 
	 * List of pointers (only) to the rrsets in the order in which 
	 * they appear in the reply message.  
	 * Number of elements is ancount+nscount+arcount.
	 * This is a pointer to that array. 
	 * Use the accessor function for access.
	 */
	struct packed_rrset_key** rrsets;

	/** 
	 * Packed array of ids (see counts) and pointers to packed_rrset_key.
	 * The number equals ancount+nscount+arcount. 
	 * These are sorted in ascending pointer, the locking order. So
	 * this list can be locked (and id, ttl checked), to see if 
	 * all the data is available and recent enough.
	 *
	 * This is defined as an array of size 1, so that the compiler 
	 * associates the identifier with this position in the structure.
	 * Array bound overflow on this array then gives access to the further
	 * elements of the array, which are allocated after the main structure.
	 *
	 * It could be more pure to define as array of size 0, ref[0].
	 * But ref[1] may be less confusing for compilers.
	 * Use the accessor function for access.
	 */
	struct rrset_ref ref[1];
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
 * Parse query reply.
 * Fills in preallocated query_info structure (with ptr into buffer).
 * Allocates reply_info and packed_rrsets. These are not yet added to any
 * caches or anything, this is only parsing. Returns formerror on qdcount > 1.
 * @param pkt: the packet buffer. Must be positioned after the query section.
 * @param alloc: creates packed rrset key structures.
 * @param rep: allocated reply_info is returned (only on no error).
 * @param qinf: query_info is returned (only on no error).
 * @return: zero is OK, or error code in case of error.
 */
int reply_info_parse(ldns_buffer* pkt, struct alloc_cache* alloc,
	struct query_info* qinf, struct reply_info* rep);

/** 
 * Delete reply_info and packed_rrsets (while they are not yet added to the
 * hashtables.). Returns rrsets to the alloc cache.
 * @param rep: reply_info to delete.
 * @param alloc: where to return rrset structures to.
 */
void reply_info_parsedelete(struct reply_info* rep, struct alloc_cache* alloc);

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
