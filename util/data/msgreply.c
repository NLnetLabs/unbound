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

/** copy and allocate an uncompressed dname. */
static uint8_t*
copy_uncompr(uint8_t* dname, size_t len) 
{
	uint8_t* p = (uint8_t*)malloc(len);
	if(!p) 
		return 0;
	memmove(p, dname, len);
	return p;
}

/** allocate qinfo, return 0 on error. */
static int
parse_create_qinfo(struct msg_parse* msg, struct query_info* qinf)
{
	if(msg->qname) {
		if(!(qinf->qname = copy_uncompr(msg->qname, msg->qname_len)))
			return 0;
	} else	qinf->qname = 0;
	qinf->qnamesize = msg->qname_len;
	qinf->qtype = msg->qtype;
	qinf->qclass = msg->qclass;
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
	(*rep)->reply = 0; /* unused */
	(*rep)->replysize = 0; /* unused */
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
	/* layout: struct - rr_len - rr_data - rdata - rrsig */
	data->rr_len = (size_t*)((uint8_t*)data + 
		sizeof(struct packed_rrset_data));
	data->rr_ttl = (uint32_t*)&(data->rr_len[total]);
	data->rr_data = (uint8_t**)&(data->rr_ttl[total]);
	nextrdata = (uint8_t*)&(data->rr_data[total]);
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
	uint16_t t;
	struct rrset_parse *pset = msg->rrset_first;
	struct packed_rrset_data* data;
	log_assert(rep);

	for(i=0; i<rep->rrset_count; i++) {
		rep->rrsets[i]->rk.flags = pset->flags;
		rep->rrsets[i]->rk.dname_len = pset->dname_len;
		rep->rrsets[i]->rk.dname = (uint8_t*)malloc(
			pset->dname_len + 4 /* size of type and class */ );
		if(!rep->rrsets[i]->rk.dname)
			return LDNS_RCODE_SERVFAIL;
		/** copy & decompress dname */
		dname_pkt_copy(pkt, rep->rrsets[i]->rk.dname, pset->dname);
		/** copy over type and class */
		t = htons(pset->type);
		memmove(&rep->rrsets[i]->rk.dname[pset->dname_len], 
			&t, sizeof(uint16_t));
		memmove(&rep->rrsets[i]->rk.dname[pset->dname_len+2], 
			&pset->rrset_class, sizeof(uint16_t));
		/** read data part. */
		if((ret=parse_create_rrset(pkt, pset, &data)) != 0)
			return ret;
		rep->rrsets[i]->entry.data = (void*)data;
		rep->rrsets[i]->entry.hash = pset->hash;

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
	if(!parse_create_qinfo(msg, qinf))
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
        struct query_info* qinf, struct reply_info** rep)
{
	/* use scratch pad region-allocator during parsing. */
	region_type* region = region_create(malloc, free);
	struct msg_parse* msg;
	int ret;
	
	qinf->qname = NULL;
	*rep = NULL;
	if(!(msg = region_alloc(region, sizeof(*msg)))) {
		region_free_all(region);
		region_destroy(region);
		return LDNS_RCODE_SERVFAIL;
	}
	memset(msg, 0, sizeof(*msg));
	
	log_assert(ldns_buffer_position(pkt) == 0);
	if((ret = parse_packet(pkt, msg, region)) != 0) {
		region_free_all(region);
		region_destroy(region);
		return ret;
	}

	/* parse OK, allocate return structures */
	/* this also performs dname decompression */
	if((ret = parse_create_msg(pkt, msg, alloc, qinf, rep)) != 0) {
		query_info_clear(qinf);
		reply_info_parsedelete(*rep, alloc);
		*rep = NULL;
		region_free_all(region);
		region_destroy(region);
		return ret;
	}

	/* exit and cleanup */
	region_free_all(region);
	region_destroy(region);
	return 0;
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
	h = dname_query_hash(q->qname, h);
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
static void
write_compressed_dname(ldns_buffer* pkt, uint8_t* dname, int labs,
	struct compress_tree_node* p)
{
	/* compress it */
	int labcopy = labs - p->labs;
	uint8_t lablen;
	uint16_t ptr;

	/* copy the first couple of labels */
	while(labcopy--) {
		lablen = *dname++;
		ldns_buffer_write_u8(pkt, lablen);
		ldns_buffer_write(pkt, dname, lablen);
		dname += lablen;
	}
	/* insert compression ptr */
	ptr = (uint16_t)(0xc000 | p->offset);
	ldns_buffer_write_u16(pkt, ptr);
}

/** compress owner name of RR */
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
				*owner_ptr = htons((uint16_t)(0xc000 | 
					p->offset));
			write_compressed_dname(pkt, key->rk.dname, 
				owner_labs, p);
			ldns_buffer_write(pkt, &key->rk.dname[
				key->rk.dname_len], 4);
		} else {
			/* no compress */
			ldns_buffer_write(pkt, key->rk.dname, 
				key->rk.dname_len+4);
			if(owner_pos <= PTR_MAX_OFFSET)
				*owner_ptr = htons((uint16_t)(0xc000 | 
					owner_pos));
		}
		if(!compress_tree_store(tree, key->rk.dname, 
			owner_labs, owner_pos, region, p))
			return 0;
	} else {
		/* always compress 2nd-further RRs in RRset */
		ldns_buffer_write(pkt, owner_ptr, 2);
		ldns_buffer_write(pkt, &key->rk.dname[key->rk.dname_len], 4);
	}
	return 1;
}

/** compress any domain name to the packet */
static int
compress_any_dname(uint8_t* dname, ldns_buffer* pkt, int labs, 
	region_type* region, struct compress_tree_node** tree)
{
	struct compress_tree_node* p;
	size_t pos = ldns_buffer_position(pkt);
	if((p = compress_tree_lookup(*tree, dname, labs))) {
		write_compressed_dname(pkt, dname, labs, p);
	} else {
		dname_buffer_write(pkt, dname);
	}
	return compress_tree_store(tree, dname, labs, pos, region, p);
}

/** return true if type needs domain name compression in rdata */
static const ldns_rr_descriptor*
type_rdata_compressable(struct ub_packed_rrset_key* key)
{
	uint16_t t;
	memmove(&t, &key->rk.dname[key->rk.dname_len], sizeof(t));
	t = ntohs(t);
	if(ldns_rr_descript(t) && 
		ldns_rr_descript(t)->_compress == LDNS_RR_COMPRESS)
		return ldns_rr_descript(t);
	return 0;
}

/** compress domain names in rdata */
static int
compress_rdata(ldns_buffer* pkt, uint8_t* rdata, size_t todolen, 
	region_type* region, struct compress_tree_node** tree, 
	const ldns_rr_descriptor* desc)
{
	int labs, rdf = 0;
	size_t dname_len, len, pos = ldns_buffer_position(pkt);
	uint8_t count = desc->_dname_count;

	ldns_buffer_skip(pkt, 2); /* rdata len fill in later */
	rdata += 2;
	todolen -= 2;
	while(todolen > 0 && count) {
		switch(desc->_wireformat[rdf]) {
		case LDNS_RDF_TYPE_DNAME:
			labs = dname_count_size_labels(rdata, &dname_len);
			if(!compress_any_dname(rdata, pkt, labs, region, tree))
				return 0;
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
			ldns_buffer_write(pkt, rdata, len);
			todolen -= len;
			rdata += len;
		}
		rdf++;
	}
	/* copy remainder */
	if(todolen > 0) {
		ldns_buffer_write(pkt, rdata, todolen);
	}

	/* set rdata len */
	ldns_buffer_write_u16_at(pkt, pos, ldns_buffer_position(pkt)-pos-2);
	return 1;
}

/** store rrset in iov vector */
static int
packed_rrset_iov(struct ub_packed_rrset_key* key, ldns_buffer* pkt, 
	uint16_t* num_rrs, uint32_t timenow, region_type* region,
	int do_data, int do_sig, struct compress_tree_node** tree)
{
	size_t i, owner_pos;
	int owner_labs;
	uint16_t owner_ptr = 0;
	struct packed_rrset_data* data = (struct packed_rrset_data*)
		key->entry.data;

	owner_labs = dname_count_labels(key->rk.dname);
	owner_pos = ldns_buffer_position(pkt);

	if(do_data) {
		const ldns_rr_descriptor* c = type_rdata_compressable(key);
		*num_rrs += data->count;
		for(i=0; i<data->count; i++) {
			if(1) { /* compression */
				if(!compress_owner(key, pkt, region, tree, 
					owner_pos, &owner_ptr, owner_labs))
					return 0;
			} else {
				/* no compression */
				ldns_buffer_write(pkt, key->rk.dname, 
					key->rk.dname_len + 4);
			}
			ldns_buffer_write_u32(pkt, data->rr_ttl[i]-timenow);
			if(c) {
				if(!compress_rdata(pkt, data->rr_data[i],
					data->rr_len[i], region, tree, c))
					return 0;
			} else ldns_buffer_write(pkt, data->rr_data[i], 
				data->rr_len[i]);
		}
	}
	/* insert rrsigs */
	if(do_sig) {
		size_t total = data->count+data->rrsig_count;
		*num_rrs += data->rrsig_count;
		for(i=data->count; i<total; i++) {
			if(1) { /* compression */
				if(owner_ptr)
					ldns_buffer_write(pkt, &owner_ptr, 2);
				else 	{
					if(!compress_any_dname(key->rk.dname, 
						pkt, owner_labs, region, tree))
						return 0;
				}
			} else {
				/* no compression */
				ldns_buffer_write(pkt, key->rk.dname, 
					key->rk.dname_len);
			}
			ldns_buffer_write_u16(pkt, LDNS_RR_TYPE_RRSIG);
			ldns_buffer_write(pkt, &(key->rk.dname[
				key->rk.dname_len+2]), sizeof(uint16_t));
			ldns_buffer_write_u32(pkt, data->rr_ttl[i]-timenow);
			/* rrsig rdata cannot be compressed, perform 100+ byte
			 * memcopy. */
			ldns_buffer_write(pkt, data->rr_data[i],
				data->rr_len[i]);
		}
	}

	return 1;
}

/** store msg section in iov vector */
static int
insert_section(struct reply_info* rep, size_t num_rrsets, uint16_t* num_rrs,
	ldns_buffer* pkt, size_t rrsets_before, uint32_t timenow, 
	region_type* region, int addit, struct compress_tree_node** tree)
{
	size_t i;
	*num_rrs = 0;
	if(!addit) {
	  	for(i=0; i<num_rrsets; i++)
			if(!packed_rrset_iov(rep->rrsets[rrsets_before+i], pkt,
				num_rrs, timenow, region, 1, 1, tree))
			return 0;
	} else {
	  	for(i=0; i<num_rrsets; i++)
			if(!packed_rrset_iov(rep->rrsets[rrsets_before+i], pkt,
				num_rrs, timenow, region, 1, 0, tree))
			return 0;
	  	for(i=0; i<num_rrsets; i++)
			if(!packed_rrset_iov(rep->rrsets[rrsets_before+i], pkt,
				num_rrs, timenow, region, 0, 1, tree))
			return 0;
	}
	return 1;
}

int reply_info_encode(struct query_info* qinfo, struct reply_info* rep, 
	uint16_t id, uint16_t flags, ldns_buffer* buffer, uint32_t timenow, 
	region_type* region)
{
	uint16_t ancount=0, nscount=0, arcount=0;
	struct compress_tree_node* tree = 0;

	ldns_buffer_clear(buffer);
	if(ldns_buffer_capacity(buffer) < LDNS_HEADER_SIZE)
		return 0;

	ldns_buffer_write(buffer, &id, sizeof(uint16_t));
	ldns_buffer_write_u16(buffer, flags);
	ldns_buffer_write_u16(buffer, rep->qdcount);
	/* skip an, ns, ar counts */
	ldns_buffer_set_position(buffer, LDNS_HEADER_SIZE);

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
	if(!insert_section(rep, rep->an_numrrsets, &ancount, buffer, 
		0, timenow, region, 0, &tree))
		return 0;
	ldns_buffer_write_u16_at(buffer, 6, ancount);

	/* insert auth section */
	if(!insert_section(rep, rep->ns_numrrsets, &nscount, buffer, 
		rep->an_numrrsets, timenow, region, 0, &tree))
		return 0;
	ldns_buffer_write_u16_at(buffer, 8, nscount);

	/* insert add section */
	if(!insert_section(rep, rep->ar_numrrsets, &arcount, buffer, 
		rep->an_numrrsets + rep->ns_numrrsets, timenow, region, 
		1, &tree))
		return 0;
	ldns_buffer_write_u16_at(buffer, 10, arcount);
	ldns_buffer_flip(buffer);

	return 1;
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
