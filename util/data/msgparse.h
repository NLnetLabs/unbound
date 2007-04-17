/*
 * util/data/msgparse.h - parse wireformat DNS messages.
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

#ifndef UTIL_DATA_MSGPARSE_H
#define UTIL_DATA_MSGPARSE_H
#include "util/storage/lruhash.h"
struct rrset_parse;
struct rr_parse;
struct region;

/** number of buckets in parse rrset hash table. Must be power of 2. */
#define PARSE_TABLE_SIZE 1024
/** Maximum TTL that is allowed. */
#define MAX_TTL	3600*24*365*10 /* ten years */

/**
 * Data stored in scratch pad memory during parsing.
 * Stores the data that will enter into the msgreply and packet result.
 */
struct msg_parse {
	/** id from message, network format. */
	uint16_t id;
	/** flags from message, host format. */
	uint16_t flags;
	/** count of RRs, host format */
	uint16_t qdcount;
	/** count of RRs, host format */
	uint16_t ancount;
	/** count of RRs, host format */
	uint16_t nscount;
	/** count of RRs, host format */
	uint16_t arcount;
	/** count of RRsets per section. */
	size_t an_rrsets;
	/** count of RRsets per section. */
	size_t ns_rrsets; 
	/** count of RRsets per section. */
	size_t ar_rrsets;
	/** total number of rrsets found. */
	size_t rrset_count;

	/** query dname (pointer to start location in packet, NULL if none */
	uint8_t* qname;
	/** length of query dname in octets, 0 if none */
	size_t qname_len;
	/** query type, network order. 0 if qdcount=0 */
	uint16_t qtype;
	/** query class, network order. 0 if qdcount=0 */
	uint16_t qclass;

	/**
	 * Hash table array used during parsing to lookup rrset types.
	 * Based on name, type, class.  Same hash value as in rrset cache.
	 */
	struct rrset_parse* hashtable[PARSE_TABLE_SIZE];
	
	/** linked list of rrsets that have been found (in order). */
	struct rrset_parse* rrset_first;
	/** last element of rrset list. */
	struct rrset_parse* rrset_last;
};

/**
 * Data stored for an rrset during parsing.
 */
struct rrset_parse {
	/** next in hash bucket */
	struct rrset_parse* rrset_bucket_next;
	/** next in list of all rrsets */
	struct rrset_parse* rrset_all_next;
	/** hash value of rrset */
	hashvalue_t hash;
	/** which section was it found in: one of
	 * LDNS_SECTION_ANSWER, LDNS_SECTION_AUTHORITY, LDNS_SECTION_ADDITIONAL
	 */
	ldns_pkt_section section;
	/** start of (possibly compressed) dname in packet */
	uint8_t* dname;
	/** length of the dname uncompressed wireformat */
	size_t dname_len;
	/** type, network order. */
	uint16_t type;
	/** class, network order. name so that it is not a c++ keyword. */
	uint16_t rrset_class;
	/** the flags for the rrset, like for packedrrset */
	uint32_t flags;
	/** number of RRs in the rr list */
	size_t rr_count;
	/** linked list of RRs in this rrset. */
	struct rr_parse* rr_first;
	/** last in list of RRs in this rrset. */
	struct rr_parse* rr_last;
};

/**
 * Data stored for an RR during parsing.
 */
struct rr_parse {
	/** 
	 * Pointer to the RR. Points to start of TTL value in the packet.
	 * Rdata length and rdata follow it.
	 * its dname, type and class are the same and stored for the rrset.
	 */
	uint8_t* ttl_data;
	/** the length of the rdata if allocated (with no dname compression)*/
	size_t size;
	/** next in list of RRs. */
	struct rr_parse* next;
};

/**
 * Parse the packet.
 * @param pkt: packet, position at call must be at start of packet.
 *	at end position is after packet.
 * @param msg: where to store results.
 * @param region: how to alloc results.
 * @return: 0 if OK, or rcode on error.
 */
int parse_packet(ldns_buffer* pkt, struct msg_parse* msg, 
	struct region* region);

#endif /* UTIL_DATA_MSGPARSE_H */
