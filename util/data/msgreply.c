/*
 * util/data/msgreply.c - store message and reply data. 
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

#include "config.h"
#include "util/data/msgreply.h"
#include "util/storage/lookup3.h"
#include "util/log.h"

/** determine length of a dname in buffer, no compression pointers allowed. */
size_t
query_dname_len(ldns_buffer* query)
{
	size_t len = 0;
	size_t labellen;
	while(1) {
		if(ldns_buffer_remaining(query) < 1)
			return 0; /* parse error, need label len */
		labellen = ldns_buffer_read_u8(query);
		if(labellen & 0xC0)
			return 0; /* no compression allowed in queries */
		len += labellen + 1;
		if(len > LDNS_MAX_DOMAINLEN)
			return 0; /* too long */
		if(labellen == 0)
			return len;
		if(ldns_buffer_remaining(query) < labellen)
			return 0; /* parse error, need content */
		ldns_buffer_skip(query, (ssize_t)labellen);
	}
}

int query_info_parse(struct query_info* m, ldns_buffer* query)
{
	uint8_t* q = ldns_buffer_begin(query);
	/* minimum size: header + \0 + qtype + qclass */
	if(ldns_buffer_limit(query) < LDNS_HEADER_SIZE + 5)
		return 0;
	log_assert(!LDNS_QR_WIRE(q));
	log_assert(LDNS_OPCODE_WIRE(q) == LDNS_PACKET_QUERY);
	log_assert(LDNS_QDCOUNT(q) == 1);
	log_assert(ldns_buffer_position(query) == 0);
	m->has_cd = (int)LDNS_CD_WIRE(q);
	ldns_buffer_skip(query, LDNS_HEADER_SIZE);
	q = ldns_buffer_current(query);
	if((m->qnamesize = query_dname_len(query)) == 0)
		return 0; /* parse error */
	if(!(m->qname = (uint8_t*)malloc(m->qnamesize))) {
		log_err("query_info_parse: out of memory");
		return 0; /* out of memory */
	}
	memmove(m->qname, q, m->qnamesize);

	if(ldns_buffer_remaining(query) < 4)
		return 0; /* need qtype, qclass */
	m->qtype = ldns_buffer_read_u16(query);
	m->qclass = ldns_buffer_read_u16(query);
	return 1;
}

/** tiny subroutine for msgreply_compare */
#define COMPARE_IT(x, y) \
	if( (x) < (y) ) return -1; \
	else if( (x) > (y) ) return +1; \
	log_assert( (x) == (y) );
int query_info_compare(void* m1, void* m2)
{
	struct query_info* msg1 = (struct query_info*)m1;
	struct query_info* msg2 = (struct query_info*)m2;
	int mc;
	/* from most different to least different for speed */
	COMPARE_IT(msg1->qtype, msg2->qtype);
	COMPARE_IT(msg1->qnamesize, msg2->qnamesize);
	mc = memcmp(msg1->qname, msg2->qname, msg1->qnamesize);
	if(mc != 0)
		return mc;
	COMPARE_IT(msg1->has_cd, msg2->has_cd);
	COMPARE_IT(msg1->qclass, msg2->qclass);
	return 0;
#undef COMPARE_IT
}

void query_info_clear(struct query_info* m)
{
	free(m->qname);
	m->qname = NULL;
}

void reply_info_clear(struct reply_info* m)
{
	free(m->reply);
	m->reply = NULL;
}

size_t msgreply_sizefunc(void* k, void* d)
{
	struct query_info* q = (struct query_info*)k;
	struct reply_info* r = (struct reply_info*)d;
	return sizeof(struct msgreply_entry) + sizeof(struct reply_info)
		+ r->replysize + q->qnamesize;
}

void query_info_delete(void *k, void* ATTR_UNUSED(arg))
{
	struct query_info* q = (struct query_info*)k;
	query_info_clear(q);
	free(q);
}

void reply_info_delete(void* d, void* ATTR_UNUSED(arg))
{
	struct reply_info* r = (struct reply_info*)d;
	reply_info_clear(r);
	free(r);
}

hashvalue_t query_info_hash(struct query_info *q)
{
	hashvalue_t h = 0xab;
	h = hashlittle(&q->qtype, sizeof(q->qtype), h);
	h = hashlittle(&q->qclass, sizeof(q->qclass), h);
	h = hashlittle(&q->has_cd, sizeof(q->has_cd), h);
	h = hashlittle(q->qname, q->qnamesize, h);
	return h;
}

void reply_info_answer(struct reply_info* rep, uint16_t qflags, 
	ldns_buffer* buffer)
{
	uint16_t flags;
	ldns_buffer_clear(buffer);
	ldns_buffer_skip(buffer, 2); /* ID */
	flags = ldns_read_uint16(rep->reply+2);
	flags |= (qflags & 0x0100); /* copy RD bit */
	log_info("flags %x", flags);
	ldns_buffer_write_u16(buffer, flags);
	ldns_buffer_write(buffer, rep->reply+2, rep->replysize-2);
	ldns_buffer_flip(buffer);
}

struct msgreply_entry* query_info_entrysetup(struct query_info* q,
	struct reply_info* r, hashvalue_t h)
{
	struct msgreply_entry* e = (struct msgreply_entry*)malloc( 
		sizeof(struct msgreply_entry));
	if(!e) return NULL;
	memcpy(&e->key, q, sizeof(*q));
	e->entry.hash = h;
	e->entry.key = e;
	e->entry.data = r;
	lock_rw_init(&e->entry.lock);
	lock_protect(&e->entry.lock, e, sizeof(*e));
	lock_protect(&e->entry.lock, e->key.qname, e->key.qnamesize);
	q->qname = NULL;
	return e;
}
