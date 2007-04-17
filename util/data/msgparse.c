/* 
 * util/data/msgparse.c - parse wireformat DNS messages.
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
 * Routines for message parsing a packet buffer to a descriptive structure.
 */
#include "config.h"
#include "util/data/msgparse.h"
#include "util/net_help.h"
#include "util/data/dname.h"
#include "util/data/packed_rrset.h"
#include "util/storage/lookup3.h"
#include "util/region-allocator.h"

/** smart comparison of (compressed, valid) dnames from packet. */
static int
smart_compare(ldns_buffer* pkt, uint8_t* dnow, 
	uint8_t *dprfirst, uint8_t* dprlast)
{
	if( (*dnow & 0xc0) == 0xc0) {
		/* ptr points to a previous dname */
		uint8_t* p = ldns_buffer_at(pkt, (dnow[0]&0x3f)<<8 | dnow[1]);
		if( p == dprfirst || p == dprlast )
			return 0;
		/* prev dname is also a ptr, both ptrs are the same. */
		/* if( (*dprfirst & 0xc0) == 0xc0 &&
			dprfirst[0] == dnow[0] && dprfirst[1] == dnow[1])
			return 0; */
		if( (*dprlast & 0xc0) == 0xc0 &&
			dprlast[0] == dnow[0] && dprlast[1] == dnow[1])
			return 0;
	/* checks for prev dnames pointing forwards in the packet
	} else {
		if( (*dprfirst & 0xc0) == 0xc0 ) {
			if(ldns_buffer_at(pkt, (dprfirst[0]&0x3f)<<8 | 
				dprfirst[1]) == dnow)
			return 0;
		}
		if( (*dprlast & 0xc0) == 0xc0 ) {
			if(ldns_buffer_at(pkt, (dprlast[0]&0x3f)<<8 | 
				dprlast[1]) == dnow)
			return 0;
		}
	*/
	}
	return dname_pkt_compare(pkt, dnow, dprlast);
}

/** See if next rrset is nsec at zone apex. */
static int
nsec_at_apex(ldns_buffer* pkt)
{
	/* we are at ttl position in packet. */
	size_t pos = ldns_buffer_position(pkt);
	uint16_t rdatalen;
	if(ldns_buffer_remaining(pkt) < 7) /* ttl+len+root */
		return 0; /* eek! */
	ldns_buffer_skip(pkt, 4); /* ttl */;
	rdatalen = ldns_buffer_read_u16(pkt);
	if(ldns_buffer_remaining(pkt) < rdatalen) {
		ldns_buffer_set_position(pkt, pos);
		return 0; /* parse error happens later */
	}
	/* must validate the nsec next domain name format */
	if(pkt_dname_len(pkt) == 0) {
		ldns_buffer_set_position(pkt, pos);
		return 0; /* parse error */
	}

	/* see if SOA bit is set. */
	if(ldns_buffer_position(pkt) < pos+rdatalen) {
		/* nsec type bitmap contains items */
		uint8_t win, blen, bits;
		/* need: windownum, bitmap len, firstbyte */
		if(ldns_buffer_position(pkt)+3 <= pos+rdatalen) {
			ldns_buffer_set_position(pkt, pos);
			return 0; /* malformed nsec */
		}
		win = ldns_buffer_read_u8(pkt);
		blen = ldns_buffer_read_u8(pkt);
		bits = ldns_buffer_read_u8(pkt);
		/* 0window always first window. bitlen >=1 or parse
		   error really. bit 0x2 is SOA. */
		if(win == 0 && blen >= 1 && (bits & 0x02)) {
			ldns_buffer_set_position(pkt, pos);
			return 1;
		}
	}

	ldns_buffer_set_position(pkt, pos);
	return 0;
}

/** Calculate hash value for rrset in packet. */
static hashvalue_t
pkt_hash_rrset(struct msg_parse* msg, ldns_buffer* pkt, uint8_t* dname, 
	uint16_t type, uint16_t dclass, uint32_t* rrset_flags)
{
	hashvalue_t h = 0xab;
	if(msg->flags & BIT_CD)
		*rrset_flags = PACKED_RRSET_CD;
	else	*rrset_flags = 0;
	if(type == htons(LDNS_RR_TYPE_NSEC) && nsec_at_apex(pkt))
		*rrset_flags |= PACKED_RRSET_NSEC_AT_APEX;
	
	h = hashlittle(&type, sizeof(type), h);
	h = hashlittle(&dclass, sizeof(dclass), h);
	h = hashlittle(rrset_flags, sizeof(uint32_t), h);
	h = dname_pkt_hash(pkt, dname, h);
	return h;
}

/** compare rrset_parse with data. */
static int
rrset_parse_equals(struct rrset_parse* p, ldns_buffer* pkt, hashvalue_t h, 
	uint32_t rrset_flags, uint8_t* dname, size_t dnamelen, 
	uint16_t type, uint16_t dclass)
{
	if(p->hash == h && p->dname_len == dnamelen && p->type == type &&
		p->rrset_class == dclass && p->flags == rrset_flags &&
		dname_pkt_compare(pkt, dname, p->dname) == 0)
		return 1;
	return 0;
}


/**
 * Lookup in msg hashtable to find a rrset
 */
static struct rrset_parse*
hashtable_lookup(struct msg_parse* msg, ldns_buffer* pkt, hashvalue_t h, 
	uint32_t rrset_flags, uint8_t* dname, size_t dnamelen, 
	uint16_t type, uint16_t dclass)
{
	struct rrset_parse* p = msg->hashtable[h & (PARSE_TABLE_SIZE-1)];
	while(p) {
		if(rrset_parse_equals(p, pkt, h, rrset_flags, dname, dnamelen,
			type, dclass))
			return p;
		p = p->rrset_bucket_next;
	}
	return NULL;
}

/** Find rrset. If equal to previous it is fast. hash if not so.
 * @param msg: the message with hash table.
 * @param pkt: the packet in wireformat (needed for compression ptrs).
 * @param dname: pointer to start of dname (compressed) in packet.
 * @param dnamelen: uncompressed wirefmt length of dname.
 * @param type: type of current rr.
 * @param dclass: class of current rr.
 * @param hash: hash value is returned if the rrset could not be found.
 * @param rrset_flags: is returned if the rrset could not be found.
 * @param prev_dname_first: dname of last seen RR. First seen dname.
 * @param prev_dname_last: dname of last seen RR. Last seen dname.
 * @param prev_dnamelen: dname len of last seen RR.
 * @param prev_type: type of last seen RR.
 * @param prev_dclass: class of last seen RR.
 * @param rrset_prev: last seen RRset.
 * @return the rrset if found, or null if no matching rrset exists.
 */
static struct rrset_parse*
find_rrset(struct msg_parse* msg, ldns_buffer* pkt, uint8_t* dname, 
	size_t dnamelen, uint16_t type, uint16_t dclass, hashvalue_t* hash, 
	uint32_t* rrset_flags,
	uint8_t** prev_dname_first, uint8_t** prev_dname_last,
	size_t* prev_dnamelen, uint16_t* prev_type,
	uint16_t* prev_dclass, struct rrset_parse** rrset_prev)
{
	if(rrset_prev) {
		/* check if equal to previous item */
		if(type == *prev_type && dclass == *prev_dclass &&
			dnamelen == *prev_dnamelen &&
			smart_compare(pkt, dname, *prev_dname_first, 
				*prev_dname_last) == 0) {
			/* same as previous */
			*prev_dname_last = dname;
			return *rrset_prev;
		}

	}
	/* find by hashing and lookup in hashtable */
	*hash = pkt_hash_rrset(msg, pkt, dname, type, dclass, rrset_flags);
	*rrset_prev = hashtable_lookup(msg, pkt, *hash, *rrset_flags, 
		dname, dnamelen, type, dclass);
	if(*rrset_prev)
		*prev_dname_first = (*rrset_prev)->dname;
	else 	*prev_dname_first = dname;
	*prev_dname_last = dname;
	*prev_dnamelen = dnamelen;
	*prev_type = type;
	*prev_dclass = dclass;
	return *rrset_prev;
}

/**
 * Parse query section. 
 * @param pkt: packet, position at call must be at start of query section.
 *	at end position is after query section.
 * @param msg: store results here.
 * @return: 0 if OK, or rcode on error.
 */
static int
parse_query_section(ldns_buffer* pkt, struct msg_parse* msg)
{
	if(msg->qdcount == 0)
		return 0;
	if(msg->qdcount > 1)
		return LDNS_RCODE_FORMERR;
	log_assert(msg->qdcount == 1);
	if(ldns_buffer_remaining(pkt) <= 0)
		return LDNS_RCODE_FORMERR;
	msg->qname = ldns_buffer_current(pkt);
	if((msg->qname_len = query_dname_len(pkt)) == 0)
		return LDNS_RCODE_FORMERR;
	if(ldns_buffer_remaining(pkt) < sizeof(uint16_t)*2)
		return LDNS_RCODE_FORMERR;
	msg->qtype = ldns_buffer_read_u16(pkt);
	msg->qclass = ldns_buffer_read_u16(pkt);
	return 0;
}

/**
 * Allocate new rrset in region, fill with data.
 */
static struct rrset_parse* 
new_rrset(struct msg_parse* msg, uint8_t* dname, size_t dnamelen, 
	uint16_t type, uint16_t dclass, hashvalue_t hash, 
	uint32_t rrset_flags, ldns_pkt_section section, region_type* region)
{
	struct rrset_parse* p = region_alloc(region, sizeof(*p));
	if(!p) return NULL;
	p->rrset_bucket_next = msg->hashtable[hash & (PARSE_TABLE_SIZE-1)];
	msg->hashtable[hash & (PARSE_TABLE_SIZE-1)] = p;
	p->rrset_all_next = 0;
	if(msg->rrset_last)
		msg->rrset_last->rrset_all_next = p;
	else 	msg->rrset_first = p;
	msg->rrset_last = p;
	p->hash = hash;
	p->section = section;
	p->dname = dname;
	p->dname_len = dnamelen;
	p->type = type;
	p->rrset_class = dclass;
	p->flags = rrset_flags;
	p->rr_count = 0;
	p->rr_first = 0;
	p->rr_last = 0;
	return p;
}

/** Add rr (from packet here) to rrset, skips rr */
static int
add_rr_to_rrset(struct rrset_parse* rrset, ldns_buffer* pkt, 
	region_type* region, ldns_pkt_section section)
{
	uint16_t rdatalen;
	/* check section of rrset. */
	if(rrset->section != section) {
		/* silently drop it */
		verbose(VERB_DETAIL, "Packet contains rrset data in "
			"multiple sections, dropped last part.");
	} else {
		/* create rr */
		struct rr_parse* rr = region_alloc(region, sizeof(*rr));
		if(!rr) return LDNS_RCODE_SERVFAIL;
		rr->ttl_data = ldns_buffer_current(pkt);
		rr->next = 0;
		if(rrset->rr_last)
			rrset->rr_last->next = rr;
		else	rrset->rr_first = rr;
		rrset->rr_last = rr;
		rrset->rr_count++;
	}

	/* forwards */
	if(ldns_buffer_remaining(pkt) < 6) /* ttl + rdatalen */
		return LDNS_RCODE_FORMERR;
	ldns_buffer_skip(pkt, 4); /* ttl */
	rdatalen = ldns_buffer_read_u16(pkt);
	if(ldns_buffer_remaining(pkt) < rdatalen)
		return LDNS_RCODE_FORMERR;
	ldns_buffer_skip(pkt, (ssize_t)rdatalen);
	return 0;
}

/**
 * Parse packet RR section, for answer, authority and additional sections. 
 * @param pkt: packet, position at call must be at start of section.
 *	at end position is after section.
 * @param msg: store results here.
 * @param region: how to alloc results.
 * @param section: section enum.
 * @param num_rrs: how many rrs are in the section.
 * @param num_rrsets: returns number of rrsets in the section.
 * @return: 0 if OK, or rcode on error.
 */
static int
parse_section(ldns_buffer* pkt, struct msg_parse* msg, region_type* region,
	ldns_pkt_section section, uint16_t num_rrs, size_t* num_rrsets)
{
	uint16_t i;
	uint8_t* dname, *prev_dname_f = NULL, *prev_dname_l = NULL;
	size_t dnamelen, prev_dnamelen = 0;
	uint16_t type, prev_type = 0;
	uint16_t dclass, prev_dclass = 0;
	uint32_t rrset_flags = 0;
	hashvalue_t hash = 0;
	struct rrset_parse* rrset, *rrset_prev = NULL;
	int r;

	if(num_rrs == 0)
		return 0;
	if(ldns_buffer_remaining(pkt) <= 0)
		return LDNS_RCODE_FORMERR;
	for(i=0; i<num_rrs; i++) {
		/* parse this RR. */
		dname = ldns_buffer_current(pkt);
		if((dnamelen = pkt_dname_len(pkt)) == 0)
			return LDNS_RCODE_FORMERR;
		if(ldns_buffer_remaining(pkt) < 10) /* type, class, ttl, len */
			return LDNS_RCODE_FORMERR;
		ldns_buffer_read(pkt, &type, sizeof(type));
		ldns_buffer_read(pkt, &dclass, sizeof(dclass));

		/* see if it is part of an existing RR set */
		if((rrset = find_rrset(msg, pkt, dname, dnamelen, type, dclass,
			&hash, &rrset_flags, &prev_dname_f, &prev_dname_l, 
			&prev_dnamelen, &prev_type, &prev_dclass, 
			&rrset_prev)) != 0) {
			/* check if it fits the existing rrset */
			/* add to rrset. */
		} else {
			/* it is a new RR set. hash&flags already calculated.*/
			(*num_rrsets)++;
			rrset = new_rrset(msg, dname, dnamelen, type, dclass,
				hash, rrset_flags, section, region);
			if(!rrset) return LDNS_RCODE_SERVFAIL;
			rrset_prev = rrset;
		}
		if((r=add_rr_to_rrset(rrset, pkt, region, section)))
			return r;
	}
	return 0;
}

int
parse_packet(ldns_buffer* pkt, struct msg_parse* msg, region_type* region)
{
	int ret;
	if(ldns_buffer_remaining(pkt) < LDNS_HEADER_SIZE)
		return LDNS_RCODE_FORMERR;
	/* read the header */
	ldns_buffer_read(pkt, &msg->id, sizeof(uint16_t));
	msg->flags = ldns_buffer_read_u16(pkt);
	msg->qdcount = ldns_buffer_read_u16(pkt);
	msg->ancount = ldns_buffer_read_u16(pkt);
	msg->nscount = ldns_buffer_read_u16(pkt);
	msg->arcount = ldns_buffer_read_u16(pkt);
	if(msg->qdcount > 1)
		return LDNS_RCODE_FORMERR;
	if((ret = parse_query_section(pkt, msg)) != 0)
		return ret;
	if((ret = parse_section(pkt, msg, region, LDNS_SECTION_ANSWER,
		msg->ancount, &msg->an_rrsets)) != 0)
		return ret;
	if((ret = parse_section(pkt, msg, region, LDNS_SECTION_AUTHORITY,
		msg->nscount, &msg->ns_rrsets)) != 0)
		return ret;
	if((ret = parse_section(pkt, msg, region, LDNS_SECTION_ADDITIONAL, 
		msg->arcount, &msg->ar_rrsets)) != 0)
		return ret;
	if(ldns_buffer_remaining(pkt) > 0) {
		/* spurious data at end of packet. ignore */
		verbose(VERB_DETAIL, "spurious data at end of packet ignored");
	}
	msg->rrset_count = msg->an_rrsets + msg->ns_rrsets + msg->ar_rrsets;
	return 0;
}
