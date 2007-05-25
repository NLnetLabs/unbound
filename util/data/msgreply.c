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
#include "util/alloc.h"
#include "util/netevent.h"
#include "util/net_help.h"
#include "util/data/dname.h"
#include "util/region-allocator.h"
#include "util/data/msgparse.h"

/** return code that means the function ran out of memory. negative so it does
 * not conflict with DNS rcodes. */
#define RETVAL_OUTMEM	-2
/** return code that means the data did not fit (completely) in the packet */
#define RETVAL_TRUNC	-4
/** return code that means all is peachy keen. Equal to DNS rcode NOERROR */
#define RETVAL_OK	0

/** allocate qinfo, return 0 on error. */
static int
parse_create_qinfo(ldns_buffer* pkt, struct msg_parse* msg, 
	struct query_info* qinf)
{
	if(msg->qname) {
		qinf->qname = (uint8_t*)malloc(msg->qname_len);
		if(!qinf->qname) return 0;
		dname_pkt_copy(pkt, qinf->qname, msg->qname);
	} else	qinf->qname = 0;
	qinf->qnamesize = msg->qname_len;
	qinf->qtype = msg->qtype;
	qinf->qclass = msg->qclass;
	qinf->has_cd = 0;
	if(msg->flags & BIT_CD)
		qinf->has_cd = 1;
	return 1;
}

/** allocate replyinfo, return 0 on error. */
static int
parse_create_repinfo(struct msg_parse* msg, struct reply_info** rep)
{
	/* rrset_count-1 because the first ref is part of the struct. */
	*rep = (struct reply_info*)malloc(sizeof(struct reply_info) + 
		sizeof(struct rrset_ref) * (msg->rrset_count-1)  +
		sizeof(struct ub_packed_rrset_key*) * msg->rrset_count);
	if(!*rep) return 0;
	(*rep)->flags = msg->flags;
	(*rep)->qdcount = msg->qdcount;
	(*rep)->ttl = 0;
	(*rep)->an_numrrsets = msg->an_rrsets;
	(*rep)->ns_numrrsets = msg->ns_rrsets;
	(*rep)->ar_numrrsets = msg->ar_rrsets;
	(*rep)->rrset_count = msg->rrset_count;
	/* array starts after the refs */
	(*rep)->rrsets = (struct ub_packed_rrset_key**)
		&((*rep)->ref[msg->rrset_count]);
	/* zero the arrays to assist cleanup in case of malloc failure */
	memset( (*rep)->rrsets, 0, 
		sizeof(struct ub_packed_rrset_key*) * msg->rrset_count);
	memset( &(*rep)->ref[0], 0, 
		sizeof(struct rrset_ref) * msg->rrset_count);
	return 1;
}

/** allocate (special) rrset keys, return 0 on error. */
static int
parse_alloc_rrset_keys(struct msg_parse* msg, struct reply_info* rep,
	struct alloc_cache* alloc)
{
	size_t i;
	for(i=0; i<msg->rrset_count; i++) {
		rep->rrsets[i] = alloc_special_obtain(alloc);
		if(!rep->rrsets[i])
			return 0;
		rep->rrsets[i]->entry.data = NULL;
	}
	return 1;
}

/** do the rdata copy */
static int
rdata_copy(ldns_buffer* pkt, struct packed_rrset_data* data, uint8_t* to, 
	struct rr_parse* rr, uint32_t* rr_ttl, uint16_t type)
{
	uint16_t pkt_len;
	const ldns_rr_descriptor* desc;
	ldns_buffer_set_position(pkt, (size_t)
		(rr->ttl_data - ldns_buffer_begin(pkt)));
	log_assert(ldns_buffer_remaining(pkt) >= 6 /* ttl + rdatalen */);
	*rr_ttl = ldns_buffer_read_u32(pkt);
	/* RFC 2181 Section 8. if msb of ttl is set treat as if zero. */
	if(*rr_ttl & 0x80000000U)
		*rr_ttl = 0;
	if(*rr_ttl < data->ttl)
		data->ttl = *rr_ttl;
	/* insert decompressed size into rdata len stored in memory */
	/* -2 because rdatalen bytes are not included. */
	pkt_len = htons(rr->size - 2);
	memmove(to, &pkt_len, sizeof(uint16_t));
	to += 2;
	/* read packet rdata len */
	pkt_len = ldns_buffer_read_u16(pkt);
	if(ldns_buffer_remaining(pkt) < pkt_len)
		return 0;
	log_assert((size_t)pkt_len+2 <= rr->size);
	desc = ldns_rr_descript(type);
	if(pkt_len > 0 && desc && desc->_dname_count > 0) {
		int count = (int)desc->_dname_count;
		int rdf = 0;
		size_t len;
		size_t oldpos;
		/* decompress dnames. */
		while(pkt_len > 0 && count) {
			switch(desc->_wireformat[rdf]) {
			case LDNS_RDF_TYPE_DNAME:
				oldpos = ldns_buffer_position(pkt);
				dname_pkt_copy(pkt, to, 
					ldns_buffer_current(pkt));
				to += pkt_dname_len(pkt);
				pkt_len -= ldns_buffer_position(pkt)-oldpos;
				count--;
				len = 0;
				break;
			case LDNS_RDF_TYPE_STR:
				len = ldns_buffer_current(pkt)[0] + 1;
				break;
			default:
				len = get_rdf_size(desc->_wireformat[rdf]);
				break;
			}
			if(len) {
				memmove(to, ldns_buffer_current(pkt), len);
				to += len;
				ldns_buffer_skip(pkt, (ssize_t)len);
				log_assert(len <= pkt_len);
				pkt_len -= len;
			}
			rdf++;
		}
	}
	/* copy remaining rdata */
	if(pkt_len >  0)
		memmove(to, ldns_buffer_current(pkt), pkt_len);
	
	return 1;
}

/** copy over the data into packed rrset */
static int
parse_rr_copy(ldns_buffer* pkt, struct rrset_parse* pset, 
	struct packed_rrset_data* data)
{
	size_t i;
	struct rr_parse* rr = pset->rr_first;
	uint8_t* nextrdata;
	size_t total = pset->rr_count + pset->rrsig_count;
	data->ttl = MAX_TTL;
	data->count = pset->rr_count;
	data->rrsig_count = pset->rrsig_count;
	data->trust = rrset_trust_none;
	/* layout: struct - rr_len - rr_data - rr_ttl - rdata - rrsig */
	data->rr_len = (size_t*)((uint8_t*)data + 
		sizeof(struct packed_rrset_data));
	data->rr_data = (uint8_t**)&(data->rr_len[total]);
	data->rr_ttl = (uint32_t*)&(data->rr_data[total]);
	nextrdata = (uint8_t*)&(data->rr_ttl[total]);
	for(i=0; i<data->count; i++) {
		data->rr_len[i] = rr->size;
		data->rr_data[i] = nextrdata;
		nextrdata += rr->size;
		if(!rdata_copy(pkt, data, data->rr_data[i], rr, 
			&data->rr_ttl[i], pset->type))
			return 0;
		rr = rr->next;
	}
	/* if rrsig, its rdata is at nextrdata */
	rr = pset->rrsig_first;
	for(i=data->count; i<total; i++) {
		data->rr_len[i] = rr->size;
		data->rr_data[i] = nextrdata;
		nextrdata += rr->size;
		if(!rdata_copy(pkt, data, data->rr_data[i], rr, 
			&data->rr_ttl[i], LDNS_RR_TYPE_RRSIG))
			return 0;
		rr = rr->next;
	}
	return 1;
}

/** create rrset return 0 or rcode */
static int
parse_create_rrset(ldns_buffer* pkt, struct rrset_parse* pset,
	struct packed_rrset_data** data)
{
	/* allocate */
	*data = malloc(sizeof(struct packed_rrset_data) + 
		(pset->rr_count + pset->rrsig_count) * 
		(sizeof(size_t)+sizeof(uint8_t*)+sizeof(uint32_t)) + 
		pset->size);
	if(!*data)
		return LDNS_RCODE_SERVFAIL;
	/* copy & decompress */
	if(!parse_rr_copy(pkt, pset, *data))
		return LDNS_RCODE_SERVFAIL;
	return 0;
}

/** get trust value for rrset */
static enum rrset_trust
get_rrset_trust(struct reply_info* rep, size_t i)
{
	uint16_t AA = rep->flags & BIT_AA;
	/* TODO: need scrubber that knows what zone the server serves, so that
	 * it can check if AA bit is warranted.
	 * it can check if rrset_trust_nonauth_ans_AA should be used */
	if(i < rep->an_numrrsets) {
		/* answer section */
		if(AA)	return rrset_trust_ans_AA;
		else	return rrset_trust_ans_noAA;
		
	} else if(i < rep->an_numrrsets+rep->ns_numrrsets) {
		/* authority section */
		if(AA)	return rrset_trust_auth_AA;
		else	return rrset_trust_auth_noAA;
	} else {
		/* addit section */
		if(AA)	return rrset_trust_add_AA;
		else	return rrset_trust_add_noAA;
	}
	return rrset_trust_none;
}

/** 
 * Copy and decompress rrs
 * @param pkt: the packet for compression pointer resolution.
 * @param msg: the parsed message
 * @param rep: reply info to put rrs into.
 * @return 0 or rcode.
 */
static int
parse_copy_decompress(ldns_buffer* pkt, struct msg_parse* msg,
	struct reply_info* rep)
{
	int ret;
	size_t i;
	struct rrset_parse *pset = msg->rrset_first;
	struct packed_rrset_data* data;
	log_assert(rep);
	rep->ttl = MAX_TTL;
	if(rep->rrset_count == 0)
		rep->ttl = NORR_TTL;

	for(i=0; i<rep->rrset_count; i++) {
		rep->rrsets[i]->rk.flags = pset->flags;
		rep->rrsets[i]->rk.dname_len = pset->dname_len;
		rep->rrsets[i]->rk.dname = (uint8_t*)malloc(pset->dname_len);
		if(!rep->rrsets[i]->rk.dname)
			return LDNS_RCODE_SERVFAIL;
		/** copy & decompress dname */
		dname_pkt_copy(pkt, rep->rrsets[i]->rk.dname, pset->dname);
		/** copy over type and class */
		rep->rrsets[i]->rk.type = htons(pset->type);
		rep->rrsets[i]->rk.rrset_class = pset->rrset_class;
		/** read data part. */
		if((ret=parse_create_rrset(pkt, pset, &data)) != 0)
			return ret;
		rep->rrsets[i]->entry.data = (void*)data;
		rep->rrsets[i]->entry.key = (void*)rep->rrsets[i];
		rep->rrsets[i]->entry.hash = pset->hash;
		data->trust = get_rrset_trust(rep, i);
		if(data->ttl < rep->ttl)
			rep->ttl = data->ttl;

		pset = pset->rrset_all_next;
	}
	return 0;
}

/** allocate and decompress message and rrsets, returns 0 or rcode. */
static int 
parse_create_msg(ldns_buffer* pkt, struct msg_parse* msg,
	struct alloc_cache* alloc, struct query_info* qinf, 
	struct reply_info** rep)
{
	int ret;
	log_assert(pkt && msg);
	if(!parse_create_qinfo(pkt, msg, qinf))
		return LDNS_RCODE_SERVFAIL;
	if(!parse_create_repinfo(msg, rep))
		return LDNS_RCODE_SERVFAIL;
	if(!parse_alloc_rrset_keys(msg, *rep, alloc))
		return LDNS_RCODE_SERVFAIL;
	if((ret=parse_copy_decompress(pkt, msg, *rep)) != 0)
		return ret;
	return 0;
}

int reply_info_parse(ldns_buffer* pkt, struct alloc_cache* alloc,
        struct query_info* qinf, struct reply_info** rep, struct region* region,
	struct edns_data* edns)
{
	/* use scratch pad region-allocator during parsing. */
	struct msg_parse* msg;
	int ret;
	
	qinf->qname = NULL;
	*rep = NULL;
	if(!(msg = region_alloc(region, sizeof(*msg)))) {
		return LDNS_RCODE_SERVFAIL;
	}
	memset(msg, 0, sizeof(*msg));
	
	ldns_buffer_set_position(pkt, 0);
	if((ret = parse_packet(pkt, msg, region)) != 0) {
		return ret;
	}
	if((ret = parse_extract_edns(msg, edns)) != 0)
		return ret;

	/* parse OK, allocate return structures */
	/* this also performs dname decompression */
	if((ret = parse_create_msg(pkt, msg, alloc, qinf, rep)) != 0) {
		query_info_clear(qinf);
		reply_info_parsedelete(*rep, alloc);
		*rep = NULL;
		return ret;
	}
	return 0;
}

/** helper compare function to sort in lock order */
static int
reply_info_sortref_cmp(const void* a, const void* b)
{
	if(a < b) return -1;
	if(a > b) return 1;
	return 0;
}

void 
reply_info_sortref(struct reply_info* rep)
{
	qsort(&rep->ref[0], rep->rrset_count, sizeof(struct rrset_ref),
		reply_info_sortref_cmp);
}

void 
reply_info_set_ttls(struct reply_info* rep, uint32_t timenow)
{
	size_t i, j;
	rep->ttl += timenow;
	for(i=0; i<rep->rrset_count; i++) {
		struct packed_rrset_data* data = (struct packed_rrset_data*)
			rep->rrsets[i]->entry.data;
		data->ttl += timenow;
		for(j=0; j<data->count + data->rrsig_count; j++)
			data->rr_ttl[j] += timenow;
	}
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
	m->has_cd = LDNS_CD_WIRE(q)?1:0;
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
	if((mc = query_dname_compare(msg1->qname, msg2->qname)) != 0)
		return mc;
	log_assert(msg1->qnamesize == msg2->qnamesize);
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

size_t 
msgreply_sizefunc(void* k, void* d)
{
	struct query_info* q = (struct query_info*)k;
	struct reply_info* r = (struct reply_info*)d;
	size_t s = sizeof(struct msgreply_entry) + sizeof(struct reply_info)
		+ q->qnamesize;
	s += (r->rrset_count-1) * sizeof(struct rrset_ref);
	s += r->rrset_count * sizeof(struct ub_packed_rrset_key*);
	return s;
}

void 
query_entry_delete(void *k, void* ATTR_UNUSED(arg), int is_locked)
{
	struct msgreply_entry* q = (struct msgreply_entry*)k;
	if(is_locked) {
		lock_rw_unlock(&q->entry.lock);
	}
	lock_rw_destroy(&q->entry.lock);
	query_info_clear(&q->key);
	free(q);
}

void 
reply_info_delete(void* d, void* ATTR_UNUSED(arg))
{
	struct reply_info* r = (struct reply_info*)d;
	free(r);
}

hashvalue_t 
query_info_hash(struct query_info *q)
{
	hashvalue_t h = 0xab;
	h = hashlittle(&q->qtype, sizeof(q->qtype), h);
	h = hashlittle(&q->qclass, sizeof(q->qclass), h);
	h = hashlittle(&q->has_cd, sizeof(q->has_cd), h);
	h = dname_query_hash(q->qname, h);
	return h;
}

/**
 * Data structure to help domain name compression in outgoing messages.
 * A tree of dnames and their offsets in the packet is kept.
 * It is kept sorted, not canonical, but by label at least, so that after
 * a lookup of a name you know its closest match, and the parent from that
 * closest match. These are possible compression targets.
 *
 * It is a binary tree, not a rbtree or balanced tree, as the effort
 * of keeping it balanced probably outweighs usefulness (given typical
 * DNS packet size).
 */
struct compress_tree_node {
	/** left node in tree, all smaller to this */
	struct compress_tree_node* left;
	/** right node in tree, all larger than this */
	struct compress_tree_node* right;

	/** the parent node - not for tree, but zone parent. One less label */
	struct compress_tree_node* parent;
	/** the domain name for this node. Pointer to uncompressed memory. */
	uint8_t* dname;
	/** number of labels in domain name, kept to help compare func. */
	int labs;
	/** offset in packet that points to this dname */
	size_t offset;
};

/**
 * Find domain name in tree, returns exact and closest match.
 * @param tree: root of tree.
 * @param dname: pointer to uncompressed dname.
 * @param labs: number of labels in domain name.
 * @param match: closest or exact match.
 *	guaranteed to be smaller or equal to the sought dname.
 *	can be null if the tree is empty.
 * @param matchlabels: number of labels that match with closest match.
 *	can be zero is there is no match.
 * @return: 0 if no exact match.
 */
static int
compress_tree_search(struct compress_tree_node* tree, uint8_t* dname,
	int labs, struct compress_tree_node** match, int* matchlabels)
{
	int c, n, closen=0;
	struct compress_tree_node* p = tree;
	struct compress_tree_node* close = 0;
	while(p) {
		if((c = dname_lab_cmp(dname, labs, p->dname, p->labs, &n)) 
			== 0) {
			*matchlabels = n;
			*match = p;
			return 1;
		}
		if(c<0) p = p->left;
		else	{
			closen = n;
			close = p; /* p->dname is smaller than dname */
			p = p->right;
		}
	}
	*matchlabels = closen;
	*match = close;
	return 0;
}

/**
 * Lookup a domain name in compression tree.
 * @param tree: root of tree (not the node with '.').
 * @param dname: pointer to uncompressed dname.
 * @param labs: number of labels in domain name.
 * @return: 0 if not found or compress treenode with best compression.
 */
static struct compress_tree_node*
compress_tree_lookup(struct compress_tree_node* tree, uint8_t* dname,
	int labs)
{
	struct compress_tree_node* p;
	int m;
	if(labs <= 1)
		return 0; /* do not compress root node */
	if(compress_tree_search(tree, dname, labs, &p, &m)) {
		/* exact match */
		return p;
	}
	/* return some ancestor of p that compresses well. */
	if(m>1) {
		/* www.example.com. (labs=4) matched foo.example.com.(labs=4)
		 * then matchcount = 3. need to go up. */
		while(p && p->labs > m)
			p = p->parent;
		return p;
	}
	return 0;
}

/**
 * Insert node into domain name compression tree.
 * @param tree: root of tree (may be modified)
 * @param dname: pointer to uncompressed dname (stored in tree).
 * @param labs: number of labels in dname.
 * @param offset: offset into packet for dname.
 * @param region: how to allocate memory for new node.
 * @return new node or 0 on malloc failure.
 */
static struct compress_tree_node*
compress_tree_insert(struct compress_tree_node** tree, uint8_t* dname,
	int labs, size_t offset, region_type* region)
{
	int c, m;
	struct compress_tree_node* p, **prev;
	struct compress_tree_node* n = (struct compress_tree_node*)
		region_alloc(region, sizeof(struct compress_tree_node));
	if(!n) return 0;
	n->left = 0;
	n->right = 0;
	n->parent = 0;
	n->dname = dname;
	n->labs = labs;
	n->offset = offset;

	/* find spot to insert it into */
	prev = tree;
	p = *tree;
	while(p) {
		c = dname_lab_cmp(dname, labs, p->dname, p->labs, &m);
		log_assert(c != 0); /* may not already be in tree */
		if(c==0) return p; /* insert only once */
		if(c<0)	{
			prev = &p->left;
			p = p->left;
		} else {
			prev = &p->right;
			p = p->right;
		}
	}
	*prev = n;
	return n;
}

/**
 * Store domain name and ancestors into compression tree.
 * @param tree: root of tree (may be modified)
 * @param dname: pointer to uncompressed dname (stored in tree).
 * @param labs: number of labels in dname.
 * @param offset: offset into packet for dname.
 * @param region: how to allocate memory for new node.
 * @param closest: match from previous lookup, used to compress dname.
 *	may be NULL if no previous match.
 *	if the tree has an ancestor of dname already, this must be it.
 * @return: 0 on memory error.
 */
static int
compress_tree_store(struct compress_tree_node** tree, uint8_t* dname,
	int labs, size_t offset, region_type* region,
	struct compress_tree_node* closest)
{
	uint8_t lablen;
	struct compress_tree_node** lastparentptr = 0;
	struct compress_tree_node* newnode;
	int uplabs = labs-1; /* does not store root in tree */
	if(closest) uplabs = labs - closest->labs;
	log_assert(uplabs >= 0);
	while(uplabs--) {
		if(offset > PTR_MAX_OFFSET) {
			if(lastparentptr) 
				*lastparentptr = closest;
			return 1; /* compression pointer no longer useful */
		}
		/* store dname, labs, offset */
		if(!(newnode = compress_tree_insert(tree, dname, labs, offset, 
			region))) {
			if(lastparentptr) 
				*lastparentptr = closest;
			return 0;
		}
		if(lastparentptr)
			*lastparentptr = newnode;
		lastparentptr = &newnode->parent;

		/* next label */
		lablen = *dname++;
		dname += lablen;
		offset += lablen+1;
		labs--;
	}
	if(lastparentptr)
		*lastparentptr = closest;
	return 1;
}

/** compress a domain name */
static int
write_compressed_dname(ldns_buffer* pkt, uint8_t* dname, int labs,
	struct compress_tree_node* p)
{
	/* compress it */
	int labcopy = labs - p->labs;
	uint8_t lablen;
	uint16_t ptr;

	if(labs == 1) {
		/* write root label */
		if(ldns_buffer_remaining(pkt) < 1)
			return 0;
		ldns_buffer_write_u8(pkt, 0);
		return 1;
	}

	/* copy the first couple of labels */
	while(labcopy--) {
		lablen = *dname++;
		if(ldns_buffer_remaining(pkt) < (size_t)lablen+1)
			return 0;
		ldns_buffer_write_u8(pkt, lablen);
		ldns_buffer_write(pkt, dname, lablen);
		dname += lablen;
	}
	/* insert compression ptr */
	if(ldns_buffer_remaining(pkt) < 2)
		return 0;
	ptr = PTR_CREATE(p->offset);
	ldns_buffer_write_u16(pkt, ptr);
	return 1;
}

/** compress owner name of RR, return RETVAL_OUTMEM RETVAL_TRUNC */
static int
compress_owner(struct ub_packed_rrset_key* key, ldns_buffer* pkt, 
	region_type* region, struct compress_tree_node** tree, 
	size_t owner_pos, uint16_t* owner_ptr, int owner_labs)
{
	struct compress_tree_node* p;
	if(!*owner_ptr) {
		/* compress first time dname */
		if((p = compress_tree_lookup(*tree, key->rk.dname, 
			owner_labs))) {
			if(p->labs == owner_labs) 
				/* avoid ptr chains, since some software is
				 * not capable of decoding ptr after a ptr. */
				*owner_ptr = htons(PTR_CREATE(p->offset));
			if(!write_compressed_dname(pkt, key->rk.dname, 
				owner_labs, p))
				return RETVAL_TRUNC;
			/* check if typeclass+4 ttl + rdatalen is available */
			if(ldns_buffer_remaining(pkt) < 4+4+2)
				return RETVAL_TRUNC;
		} else {
			/* no compress */
			if(ldns_buffer_remaining(pkt) < key->rk.dname_len+4+4+2)
				return RETVAL_TRUNC;
			ldns_buffer_write(pkt, key->rk.dname, 
				key->rk.dname_len);
			if(owner_pos <= PTR_MAX_OFFSET)
				*owner_ptr = htons(PTR_CREATE(owner_pos));
		}
		if(!compress_tree_store(tree, key->rk.dname, 
			owner_labs, owner_pos, region, p))
			return RETVAL_OUTMEM;
	} else {
		/* always compress 2nd-further RRs in RRset */
		if(owner_labs == 1) {
			if(ldns_buffer_remaining(pkt) < 1+4+4+2) 
				return RETVAL_TRUNC;
			ldns_buffer_write_u8(pkt, 0);
		} else {
			if(ldns_buffer_remaining(pkt) < 2+4+4+2) 
				return RETVAL_TRUNC;
			ldns_buffer_write(pkt, owner_ptr, 2);
		}
	}
	return RETVAL_OK;
}

/** compress any domain name to the packet, return RETVAL_* */
static int
compress_any_dname(uint8_t* dname, ldns_buffer* pkt, int labs, 
	region_type* region, struct compress_tree_node** tree)
{
	struct compress_tree_node* p;
	size_t pos = ldns_buffer_position(pkt);
	if((p = compress_tree_lookup(*tree, dname, labs))) {
		if(!write_compressed_dname(pkt, dname, labs, p))
			return RETVAL_TRUNC;
	} else {
		if(!dname_buffer_write(pkt, dname))
			return RETVAL_TRUNC;
	}
	if(!compress_tree_store(tree, dname, labs, pos, region, p))
		return RETVAL_OUTMEM;
	return RETVAL_OK;
}

/** return true if type needs domain name compression in rdata */
static const ldns_rr_descriptor*
type_rdata_compressable(struct ub_packed_rrset_key* key)
{
	uint16_t t = ntohs(key->rk.type);
	if(ldns_rr_descript(t) && 
		ldns_rr_descript(t)->_compress == LDNS_RR_COMPRESS)
		return ldns_rr_descript(t);
	return 0;
}

/** compress domain names in rdata, return RETVAL_* */
static int
compress_rdata(ldns_buffer* pkt, uint8_t* rdata, size_t todolen, 
	region_type* region, struct compress_tree_node** tree, 
	const ldns_rr_descriptor* desc)
{
	int labs, r, rdf = 0;
	size_t dname_len, len, pos = ldns_buffer_position(pkt);
	uint8_t count = desc->_dname_count;

	ldns_buffer_skip(pkt, 2); /* rdata len fill in later */
	/* space for rdatalen checked for already */
	rdata += 2;
	todolen -= 2;
	while(todolen > 0 && count) {
		switch(desc->_wireformat[rdf]) {
		case LDNS_RDF_TYPE_DNAME:
			labs = dname_count_size_labels(rdata, &dname_len);
			if((r=compress_any_dname(rdata, pkt, labs, region, 
				tree)) != RETVAL_OK)
				return r;
			rdata += dname_len;
			todolen -= dname_len;
			count--;
			len = 0;
			break;
		case LDNS_RDF_TYPE_STR:
			len = *rdata + 1;
			break;
		default:
			len = get_rdf_size(desc->_wireformat[rdf]);
		}
		if(len) {
			/* copy over */
			if(ldns_buffer_remaining(pkt) < len)
				return RETVAL_TRUNC;
			ldns_buffer_write(pkt, rdata, len);
			todolen -= len;
			rdata += len;
		}
		rdf++;
	}
	/* copy remainder */
	if(todolen > 0) {
		if(ldns_buffer_remaining(pkt) < todolen)
			return RETVAL_TRUNC;
		ldns_buffer_write(pkt, rdata, todolen);
	}

	/* set rdata len */
	ldns_buffer_write_u16_at(pkt, pos, ldns_buffer_position(pkt)-pos-2);
	return RETVAL_OK;
}

/** store rrset in buffer in wireformat, return RETVAL_* */
static int
packed_rrset_encode(struct ub_packed_rrset_key* key, ldns_buffer* pkt, 
	uint16_t* num_rrs, uint32_t timenow, region_type* region,
	int do_data, int do_sig, struct compress_tree_node** tree)
{
	size_t i, owner_pos;
	int r, owner_labs;
	uint16_t owner_ptr = 0;
	struct packed_rrset_data* data = (struct packed_rrset_data*)
		key->entry.data;

	owner_labs = dname_count_labels(key->rk.dname);
	owner_pos = ldns_buffer_position(pkt);

	if(do_data) {
		const ldns_rr_descriptor* c = type_rdata_compressable(key);
		for(i=0; i<data->count; i++) {
			if((r=compress_owner(key, pkt, region, tree, 
				owner_pos, &owner_ptr, owner_labs))
				!= RETVAL_OK)
				return r;
			ldns_buffer_write(pkt, &key->rk.type, 2);
			ldns_buffer_write(pkt, &key->rk.rrset_class, 2);
			ldns_buffer_write_u32(pkt, data->rr_ttl[i]-timenow);
			if(c) {
				if((r=compress_rdata(pkt, data->rr_data[i],
					data->rr_len[i], region, tree, c))
					!= RETVAL_OK)
					return r;
			} else {
				if(ldns_buffer_remaining(pkt) < data->rr_len[i])
					return RETVAL_TRUNC;
				ldns_buffer_write(pkt, data->rr_data[i], 
					data->rr_len[i]);
			}
		}
	}
	/* insert rrsigs */
	if(do_sig) {
		size_t total = data->count+data->rrsig_count;
		for(i=data->count; i<total; i++) {
			if(owner_ptr && owner_labs != 1) {
				if(ldns_buffer_remaining(pkt) <
					2+4+4+data->rr_len[i]) 
					return RETVAL_TRUNC;
				ldns_buffer_write(pkt, &owner_ptr, 2);
			} else {
				if((r=compress_any_dname(key->rk.dname, 
					pkt, owner_labs, region, tree))
					!= RETVAL_OK)
					return r;
				if(ldns_buffer_remaining(pkt) < 
					4+4+data->rr_len[i])
					return RETVAL_TRUNC;
			}
			ldns_buffer_write_u16(pkt, LDNS_RR_TYPE_RRSIG);
			ldns_buffer_write(pkt, &key->rk.rrset_class, 2);
			ldns_buffer_write_u32(pkt, data->rr_ttl[i]-timenow);
			/* rrsig rdata cannot be compressed, perform 100+ byte
			 * memcopy. */
			ldns_buffer_write(pkt, data->rr_data[i],
				data->rr_len[i]);
		}
	}
	/* change rrnum only after we are sure it fits */
	if(do_data)
		*num_rrs += data->count;
	if(do_sig)
		*num_rrs += data->rrsig_count;

	return RETVAL_OK;
}

/** store msg section in wireformat buffer, return RETVAL_* */
static int
insert_section(struct reply_info* rep, size_t num_rrsets, uint16_t* num_rrs,
	ldns_buffer* pkt, size_t rrsets_before, uint32_t timenow, 
	region_type* region, int addit, struct compress_tree_node** tree)
{
	int r;
	size_t i, setstart;
	*num_rrs = 0;
	if(!addit) {
	  	for(i=0; i<num_rrsets; i++) {
			setstart = ldns_buffer_position(pkt);
			if((r=packed_rrset_encode(rep->rrsets[rrsets_before+i], 
				pkt, num_rrs, timenow, region, 1, 1, tree))
				!= RETVAL_OK) {
				/* Bad, but if due to size must set TC bit */
				/* trim off the rrset neatly. */
				ldns_buffer_set_position(pkt, setstart);
				return r;
			}
		}
	} else {
	  	for(i=0; i<num_rrsets; i++) {
			setstart = ldns_buffer_position(pkt);
			if((r=packed_rrset_encode(rep->rrsets[rrsets_before+i], 
				pkt, num_rrs, timenow, region, 1, 0, tree))
				!= RETVAL_OK) {
				ldns_buffer_set_position(pkt, setstart);
				return r;
			}
		}
	  	for(i=0; i<num_rrsets; i++) {
			setstart = ldns_buffer_position(pkt);
			if((r=packed_rrset_encode(rep->rrsets[rrsets_before+i], 
				pkt, num_rrs, timenow, region, 0, 1, tree))
				!= RETVAL_OK) {
				ldns_buffer_set_position(pkt, setstart);
				return r;
			}
		}
	}
	return RETVAL_OK;
}

int reply_info_encode(struct query_info* qinfo, struct reply_info* rep, 
	uint16_t id, uint16_t flags, ldns_buffer* buffer, uint32_t timenow, 
	region_type* region, uint16_t udpsize)
{
	uint16_t ancount=0, nscount=0, arcount=0;
	struct compress_tree_node* tree = 0;
	int r;

	ldns_buffer_clear(buffer);
	if(udpsize < ldns_buffer_limit(buffer))
		ldns_buffer_set_limit(buffer, udpsize);
	if(ldns_buffer_remaining(buffer) < LDNS_HEADER_SIZE)
		return 0;

	ldns_buffer_write(buffer, &id, sizeof(uint16_t));
	ldns_buffer_write_u16(buffer, flags);
	ldns_buffer_write_u16(buffer, rep->qdcount);
	/* set an, ns, ar counts to zero in case of small packets */
	ldns_buffer_write(buffer, "\000\000\000\000\000\000", 6);

	/* insert query section */
	if(rep->qdcount) {
		if(ldns_buffer_remaining(buffer) < 
			qinfo->qnamesize+sizeof(uint16_t)*2)
			return 0; /* buffer too small */
		if(!compress_tree_store(&tree, qinfo->qname, 
			dname_count_labels(qinfo->qname), 
			ldns_buffer_position(buffer), region, NULL))
			return 0;
		ldns_buffer_write(buffer, qinfo->qname, qinfo->qnamesize);
		ldns_buffer_write_u16(buffer, qinfo->qtype);
		ldns_buffer_write_u16(buffer, qinfo->qclass);
	}

	/* insert answer section */
	if((r=insert_section(rep, rep->an_numrrsets, &ancount, buffer, 
		0, timenow, region, 0, &tree)) != RETVAL_OK) {
		if(r == RETVAL_TRUNC) {
			/* create truncated message */
			ldns_buffer_write_u16_at(buffer, 6, ancount);
			LDNS_TC_SET(ldns_buffer_begin(buffer));
			ldns_buffer_flip(buffer);
			return 1;
		}
		return 0;
	}
	ldns_buffer_write_u16_at(buffer, 6, ancount);

	/* insert auth section */
	if((r=insert_section(rep, rep->ns_numrrsets, &nscount, buffer, 
		rep->an_numrrsets, timenow, region, 0, &tree)) != RETVAL_OK) {
		if(r == RETVAL_TRUNC) {
			/* create truncated message */
			ldns_buffer_write_u16_at(buffer, 8, nscount);
			LDNS_TC_SET(ldns_buffer_begin(buffer));
			ldns_buffer_flip(buffer);
			return 1;
		}
		return 0;
	}
	ldns_buffer_write_u16_at(buffer, 8, nscount);

	/* insert add section */
	if((r=insert_section(rep, rep->ar_numrrsets, &arcount, buffer, 
		rep->an_numrrsets + rep->ns_numrrsets, timenow, region, 
		1, &tree)) != RETVAL_OK) {
		if(r == RETVAL_TRUNC) {
			/* no need to set TC bit, this is the additional */
			ldns_buffer_write_u16_at(buffer, 10, arcount);
			ldns_buffer_flip(buffer);
			return 1;
		}
		return 0;
	}
	ldns_buffer_write_u16_at(buffer, 10, arcount);
	ldns_buffer_flip(buffer);
	return 1;
}

uint16_t
calc_edns_field_size(struct edns_data* edns)
{
	if(!edns || !edns->edns_present) 
		return 0;
	/* domain root '.' + type + class + ttl + rdatalen(=0) */
	return 1 + 2 + 2 + 4 + 2;
}

void
attach_edns_record(ldns_buffer* pkt, struct edns_data* edns)
{
	size_t len;
	if(!edns || !edns->edns_present)
		return;
	/* inc additional count */
	ldns_buffer_write_u16_at(pkt, 10,
		ldns_buffer_read_u16_at(pkt, 10) + 1);
	len = ldns_buffer_limit(pkt);
	ldns_buffer_clear(pkt);
	ldns_buffer_set_position(pkt, len);
	/* write EDNS record */
	ldns_buffer_write_u8(pkt, 0); /* '.' label */
	ldns_buffer_write_u16(pkt, LDNS_RR_TYPE_OPT); /* type */
	ldns_buffer_write_u16(pkt, edns->udp_size); /* class */
	ldns_buffer_write_u8(pkt, edns->ext_rcode); /* ttl */
	ldns_buffer_write_u8(pkt, edns->edns_version);
	ldns_buffer_write_u16(pkt, edns->bits);
	ldns_buffer_write_u16(pkt, 0); /* rdatalen */
	ldns_buffer_flip(pkt);
}

int 
reply_info_answer_encode(struct query_info* qinf, struct reply_info* rep, 
	uint16_t id, uint16_t qflags, ldns_buffer* pkt, uint32_t timenow,
	int cached, struct region* region, uint16_t udpsize, 
	struct edns_data* edns)
{
	uint16_t flags;

	if(!cached) {
		/* original flags, copy RD bit from query. */
		flags = rep->flags | (qflags & BIT_RD); 
	} else {
		/* remove AA bit, copy RD and CD bits from query. */
		flags = (rep->flags & ~BIT_AA) | (qflags & (BIT_RD|BIT_CD)); 
	}
	log_assert(flags & BIT_QR); /* QR bit must be on in our replies */
	if(udpsize < LDNS_HEADER_SIZE + calc_edns_field_size(edns))
		return 0; /* packet too small to contain edns... */
	udpsize -= calc_edns_field_size(edns);
	if(!reply_info_encode(qinf, rep, id, flags, pkt, timenow, region,
		udpsize)) {
		log_err("reply encode: out of memory");
		return 0;
	}
	attach_edns_record(pkt, edns);
	return 1;
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

void 
qinfo_query_encode(ldns_buffer* pkt, struct query_info* qinfo)
{
	uint16_t flags = 0; /* QUERY, NOERROR */
	if(qinfo->has_cd)
		flags |= BIT_CD;
	ldns_buffer_clear(pkt);
	log_assert(ldns_buffer_remaining(pkt) >= 12+255+4/*max query*/);
	ldns_buffer_skip(pkt, 2); /* id done later */
	ldns_buffer_write_u16(pkt, flags);
	ldns_buffer_write_u16(pkt, 1); /* query count */
	ldns_buffer_write(pkt, "\000\000\000\000\000\000", 6); /* counts */
	ldns_buffer_write(pkt, qinfo->qname, qinfo->qnamesize);
	ldns_buffer_write_u16(pkt, qinfo->qtype);
	ldns_buffer_write_u16(pkt, qinfo->qclass);
	ldns_buffer_flip(pkt);
}
