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
struct iovec;
struct region;
struct edns_data;

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
	/** qtype, host byte order */
	uint16_t qtype;
	/** qclass, host byte order */
	uint16_t qclass;
	/** does query have CD bit set */
	int has_cd;
};

/**
 * Information to reference an rrset
 */
struct rrset_ref {
	/** the key with lock, and ptr to packed data. */
	struct ub_packed_rrset_key* key;
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
	/** the flags for the answer, host byte order. */
	uint16_t flags;

	/**
	 * Number of RRs in the query section.
	 * If qdcount is not 0, then it is 1, and the data that appears
	 * in the reply is the same as the query_info.
	 * Host byte order.
	 */
	uint16_t qdcount;

	/** 
	 * TTL of the entire reply (for negative caching).
	 * only for use when there are 0 RRsets in this message.
	 * if there are RRsets, check those instead.
	 */
	uint32_t ttl;

	/**
	 * Number of RRsets in each section.
	 * The answer section. Add up the RRs in every RRset to calculate
	 * the number of RRs, and the count for the dns packet. 
	 * The number of RRs in RRsets can change due to RRset updates.
	 */
	size_t an_numrrsets;

	/** Count of authority section RRsets */
	size_t ns_numrrsets;

	/** Count of additional section RRsets */
	size_t ar_numrrsets;

	/** number of RRsets: an_numrrsets + ns_numrrsets + ar_numrrsets */
	size_t rrset_count;

	/** 
	 * List of pointers (only) to the rrsets in the order in which 
	 * they appear in the reply message.  
	 * Number of elements is ancount+nscount+arcount RRsets.
	 * This is a pointer to that array. 
	 * Use the accessor function for access.
	 */
	struct ub_packed_rrset_key** rrsets;

	/** 
	 * Packed array of ids (see counts) and pointers to packed_rrset_key.
	 * The number equals ancount+nscount+arcount RRsets. 
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
 * @param region: where to store temporary data (for parsing).
 * @param edns: where to store edns information, does not need to be inited.
 * @return: zero is OK, or DNS error code in case of error
 *	o FORMERR for parse errors.
 *	o SERVFAIL for memory allocation errors.
 */
int reply_info_parse(ldns_buffer* pkt, struct alloc_cache* alloc,
	struct query_info* qinf, struct reply_info** rep, 
	struct region* region, struct edns_data* edns);

/**
 * Sorts the ref array.
 * @param rep: reply info. rrsets must be filled in.
 */
void reply_info_sortref(struct reply_info* rep);

/**
 * Set TTLs inside the replyinfo to absolute values.
 * @param rep: reply info. rrsets must be filled in.
 * @param timenow: the current time.
 */
void reply_info_set_ttls(struct reply_info* rep, uint32_t timenow);

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
 * @param qinf: query information that provides query section in packet.
 * @param rep: reply to fill in.
 * @param id: id word from the query.
 * @param qflags: flags word from the query.
 * @param dest: buffer to put message into; will truncate if it does not fit.
 * @param timenow: time to subtract.
 * @param cached: set true if a cached reply (so no AA bit).
 *	set false for the first reply.
 * @param region: where to allocate temp variables (for compression).
 * @param udpsize: size of the answer, 512, from EDNS, or 64k for TCP.
 * @param edns: EDNS data included in the answer, NULL for none.
 *	or if edns_present = 0, it is not included.
 * @return: 0 on error (server failure).
 */
int reply_info_answer_encode(struct query_info* qinf, struct reply_info* rep, 
	uint16_t id, uint16_t qflags, ldns_buffer* dest, uint32_t timenow,
	int cached, struct region* region, uint16_t udpsize, 
	struct edns_data* edns);

/**
 * Regenerate the wireformat from the stored msg reply.
 * If the buffer is too small then the message is truncated at a whole
 * rrset and the TC bit set, or whole rrsets are left out of the additional
 * and the TC bit is not set.
 * @param qinfo: query info to store.
 * @param rep: reply to store.
 * @param id: id value to store, network order.
 * @param flags: flags value to store, host order.
 * @param buffer: buffer to store the packet into.
 * @param timenow: time now, to adjust ttl values.
 * @param region: to store temporary data in.
 * @param udpsize: size of the answer, 512, from EDNS, or 64k for TCP.
 * @return: nonzero is success, or 
 *	0 on error: malloc failure (no log_err has been done).
 */
int reply_info_encode(struct query_info* qinfo, struct reply_info* rep, 
	uint16_t id, uint16_t flags, ldns_buffer* buffer, uint32_t timenow, 
	struct region* region, uint16_t udpsize);

/**
 * Encode query packet. Assumes the buffer is large enough.
 * @param pkt: where to store the packet.
 * @param qinfo: query info.
 */
void qinfo_query_encode(ldns_buffer* pkt, struct query_info* qinfo);

/**
 * Setup query info entry
 * @param q: query info to copy. Emptied as if clear is called.
 * @param r: reply to init data.
 * @param h: hash value.
 * @return: newly allocated message reply cache item.
 */
struct msgreply_entry* query_info_entrysetup(struct query_info* q,
	struct reply_info* r, hashvalue_t h);

/**
 * Estimate size of EDNS record in packet. EDNS record will be no larger.
 * @param edns: edns data or NULL.
 * @return octets to reserve for EDNS.
 */
uint16_t calc_edns_field_size(struct edns_data* edns);

/**
 * Attach EDNS record to buffer. Buffer has complete packet. There must
 * be enough room left for the EDNS record.
 * @param pkt: packet added to.
 * @param edns: if NULL or present=0, nothing is added to the packet.
 */
void attach_edns_record(ldns_buffer* pkt, struct edns_data* edns);


#endif /* UTIL_DATA_MSGREPLY_H */
