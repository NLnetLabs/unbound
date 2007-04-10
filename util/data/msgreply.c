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
#include "util/netevent.h"
#include "util/net_help.h"
#include "util/data/dname.h"

int reply_info_parse(ldns_buffer* pkt, struct alloc_cache* alloc,
        struct query_info* qinf, struct reply_info** rep)
{
	return LDNS_RCODE_SERVFAIL;
}

void 
reply_info_parsedelete(struct reply_info* rep, struct alloc_cache* alloc)
{
	size_t i;
	if(!rep) 
		return;
	/* no need to lock, since not shared in hashtables. */
	for(i=0; i<rep->rrset_count; i++) {
		ub_packed_rrset_parsedelete(rep->rrsets[i], alloc);
	}
	free(rep);
}

int 
query_info_parse(struct query_info* m, ldns_buffer* query)
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
	m->qname = ldns_buffer_current(query);
	if((m->qnamesize = query_dname_len(query)) == 0)
		return 0; /* parse error */
	if(ldns_buffer_remaining(query) < 4)
		return 0; /* need qtype, qclass */
	m->qtype = ldns_buffer_read_u16(query);
	m->qclass = ldns_buffer_read_u16(query);
	return 1;
}

int 
query_info_allocqname(struct query_info* m)
{
	uint8_t* q = m->qname;
	if(!(m->qname = (uint8_t*)malloc(m->qnamesize))) {
		log_err("query_info_allocqname: out of memory");
		return 0; /* out of memory */
	}
	memcpy(m->qname, q, m->qnamesize);
	return 1;
}

/** tiny subroutine for msgreply_compare */
#define COMPARE_IT(x, y) \
	if( (x) < (y) ) return -1; \
	else if( (x) > (y) ) return +1; \
	log_assert( (x) == (y) );

int 
query_info_compare(void* m1, void* m2)
{
	struct query_info* msg1 = (struct query_info*)m1;
	struct query_info* msg2 = (struct query_info*)m2;
	int mc;
	/* from most different to least different for speed */
	COMPARE_IT(msg1->qtype, msg2->qtype);
	COMPARE_IT(msg1->qnamesize, msg2->qnamesize);
	if((mc = memcmp(msg1->qname, msg2->qname, msg1->qnamesize)) != 0)
		return mc;
	COMPARE_IT(msg1->has_cd, msg2->has_cd);
	COMPARE_IT(msg1->qclass, msg2->qclass);
	return 0;
#undef COMPARE_IT
}

void 
query_info_clear(struct query_info* m)
{
	free(m->qname);
	m->qname = NULL;
}

void 
reply_info_clear(struct reply_info* m)
{
	free(m->reply);
	m->reply = NULL;
}

size_t 
msgreply_sizefunc(void* k, void* d)
{
	struct query_info* q = (struct query_info*)k;
	struct reply_info* r = (struct reply_info*)d;
	return sizeof(struct msgreply_entry) + sizeof(struct reply_info)
		+ r->replysize + q->qnamesize;
}

void 
query_entry_delete(void *k, void* ATTR_UNUSED(arg))
{
	struct msgreply_entry* q = (struct msgreply_entry*)k;
	lock_rw_destroy(&q->entry.lock);
	query_info_clear(&q->key);
	free(q);
}

void 
reply_info_delete(void* d, void* ATTR_UNUSED(arg))
{
	struct reply_info* r = (struct reply_info*)d;
	reply_info_clear(r);
	free(r);
}

hashvalue_t 
query_info_hash(struct query_info *q)
{
	hashvalue_t h = 0xab;
	h = hashlittle(&q->qtype, sizeof(q->qtype), h);
	h = hashlittle(&q->qclass, sizeof(q->qclass), h);
	h = hashlittle(&q->has_cd, sizeof(q->has_cd), h);
	query_dname_tolower(q->qname, q->qnamesize);
	h = hashlittle(q->qname, q->qnamesize, h);
	return h;
}

void 
reply_info_answer(struct reply_info* rep, uint16_t qflags, 
	ldns_buffer* buffer)
{
	uint16_t flags;
	ldns_buffer_clear(buffer);
	ldns_buffer_skip(buffer, 2); /* ID */
	flags = rep->flags | (qflags & BIT_RD); /* copy RD bit */
	log_assert(flags & BIT_QR); /* QR bit must be on in our replies */
	ldns_buffer_write_u16(buffer, flags);
	ldns_buffer_write(buffer, rep->reply, rep->replysize);
	ldns_buffer_flip(buffer);
}

void 
reply_info_answer_iov(struct reply_info* rep, uint16_t qid,
        uint16_t qflags, struct comm_reply* comrep, int cached)
{
	/* [0]=reserved for tcplen, [1]=id, [2]=flags, [3]=message */
	struct iovec iov[4];

	iov[1].iov_base = (void*)&qid;
	iov[1].iov_len = sizeof(uint16_t);
	if(!cached) {
		/* original flags, copy RD bit from query. */
		qflags = rep->flags | (qflags & BIT_RD); 
	} else {
		/* remove AA bit, copy RD and CD bits from query. */
		qflags = (rep->flags & ~BIT_AA) | (qflags & (BIT_RD|BIT_CD)); 
	}
	log_assert(qflags & BIT_QR); /* QR bit must be on in our replies */
	qflags = htons(qflags);
	iov[2].iov_base = (void*)&qflags;
	iov[2].iov_len = sizeof(uint16_t);
	iov[3].iov_base = (void*)rep->reply;
	iov[3].iov_len = rep->replysize;
	comm_point_send_reply_iov(comrep, iov, 4);
}

struct msgreply_entry* 
query_info_entrysetup(struct query_info* q, struct reply_info* r, 
	hashvalue_t h)
{
	struct msgreply_entry* e = (struct msgreply_entry*)malloc( 
		sizeof(struct msgreply_entry));
	if(!e) return NULL;
	memcpy(&e->key, q, sizeof(*q));
	e->entry.hash = h;
	e->entry.key = e;
	e->entry.data = r;
	lock_rw_init(&e->entry.lock);
	lock_protect(&e->entry.lock, &e->key, sizeof(e->key));
	lock_protect(&e->entry.lock, &e->entry.hash, sizeof(e->entry.hash) +
		sizeof(e->entry.key) + sizeof(e->entry.data));
	lock_protect(&e->entry.lock, e->key.qname, e->key.qnamesize);
	q->qname = NULL;
	return e;
}
