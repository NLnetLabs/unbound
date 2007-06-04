/*
 * util/data/packed_rrset.c - data storage for a set of resource records.
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
 * This file contains the data storage for RRsets.
 */

#include "config.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/storage/lookup3.h"
#include "util/log.h"
#include "util/alloc.h"

void
ub_packed_rrset_parsedelete(struct ub_packed_rrset_key* pkey,
        struct alloc_cache* alloc)
{
	if(!pkey)
		return;
	if(pkey->entry.data)
		free(pkey->entry.data);
	pkey->entry.data = NULL;
	if(pkey->rk.dname)
		free(pkey->rk.dname);
	pkey->rk.dname = NULL;
	pkey->id = 0;
	alloc_special_release(alloc, pkey);
}

size_t 
ub_rrset_sizefunc(void* key, void* data)
{
	struct ub_packed_rrset_key* k = (struct ub_packed_rrset_key*)key;
	struct packed_rrset_data* d = (struct packed_rrset_data*)data;
	size_t s = sizeof(struct ub_packed_rrset_key) + k->rk.dname_len;
	s += packed_rrset_sizeof(d);
	return s;
}

size_t 
packed_rrset_sizeof(struct packed_rrset_data* d)
{
	size_t s;
	if(d->rrsig_count > 0) {
		s = ((uint8_t*)d->rr_data[d->count+d->rrsig_count-1] - 
			(uint8_t*)d) + d->rr_len[d->count+d->rrsig_count-1];
	} else {
		log_assert(d->count > 0);
		s = ((uint8_t*)d->rr_data[d->count-1] - (uint8_t*)d) + 
			d->rr_len[d->count-1];
	}
	return s;
}

int 
ub_rrset_compare(void* k1, void* k2)
{
	struct ub_packed_rrset_key* key1 = (struct ub_packed_rrset_key*)k1;
	struct ub_packed_rrset_key* key2 = (struct ub_packed_rrset_key*)k2;
	int c;
	if(key1 == key2 || key1->id == key2->id)
		return 0;
	if(key1->rk.type != key2->rk.type) {
		if(key1->rk.type < key2->rk.type)
			return -1;
		return 1;
	}
	if(key1->rk.dname_len != key2->rk.dname_len) {
		if(key1->rk.dname_len < key2->rk.dname_len)
			return -1;
		return 1;
	}
	if((c=query_dname_compare(key1->rk.dname, key2->rk.dname)) != 0)
		return c;
	if(key1->rk.rrset_class != key2->rk.rrset_class) {
		if(key1->rk.rrset_class < key2->rk.rrset_class)
			return -1;
		return 1;
	}
	if(key1->rk.flags != key2->rk.flags) {
		if(key1->rk.flags < key2->rk.flags)
			return -1;
		return 1;
	}
	return 0;
}

void 
ub_rrset_key_delete(void* key, void* userdata, int is_locked)
{
	struct ub_packed_rrset_key* k = (struct ub_packed_rrset_key*)key;
	struct alloc_cache* a = (struct alloc_cache*)userdata;
	k->id = 0;
	if(is_locked) {
		lock_rw_unlock(&k->entry.lock);
	}
	free(k->rk.dname);
	k->rk.dname = NULL;
	alloc_special_release(a, k);
}

void 
rrset_data_delete(void* data, void* ATTR_UNUSED(userdata))
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)data;
	free(d);
}

int 
rrsetdata_equal(struct packed_rrset_data* d1, struct packed_rrset_data* d2)
{
	size_t i;
	size_t total;
	if(d1->count != d2->count || d1->rrsig_count != d2->rrsig_count) 
		return 0;
	total = d1->count + d1->rrsig_count;
	for(i=0; i<total; i++) {
		if(d1->rr_len[i] != d2->rr_len[i])
			return 0;
		if(memcmp(d1->rr_data[i], d2->rr_data[i], d1->rr_len[i]) != 0)
			return 0;
	}
	return 1;
}

hashvalue_t 
rrset_key_hash(struct packed_rrset_key* key)
{
	/* type is hashed in host order */
	uint16_t t = ntohs(key->type);
	/* Note this MUST be identical to pkt_hash_rrset in msgparse.c */
	/* this routine does not have a compressed name */
	hashvalue_t h = 0xab;
        h = hashlittle(&t, sizeof(t), h);
        h = hashlittle(&key->rrset_class, sizeof(uint16_t), h);
        h = hashlittle(&key->flags, sizeof(uint32_t), h);
	h = dname_query_hash(key->dname, h);
	return h;
}

void 
packed_rrset_ptr_fixup(struct packed_rrset_data* data)
{
	size_t i;
	size_t total = data->count + data->rrsig_count;
	uint8_t* nextrdata;
	/* fixup pointers in packed rrset data */
	data->rr_len = (size_t*)((uint8_t*)data +
		sizeof(struct packed_rrset_data));
	data->rr_data = (uint8_t**)&(data->rr_len[total]);
	data->rr_ttl = (uint32_t*)&(data->rr_data[total]);
	nextrdata = (uint8_t*)&(data->rr_ttl[total]);
	for(i=0; i<total; i++) {
		data->rr_data[i] = nextrdata;
		nextrdata += data->rr_len[i];
	}
}

void 
get_cname_target(struct ub_packed_rrset_key* rrset, uint8_t** dname, 
	size_t* dname_len)
{
	struct packed_rrset_data* d;
	size_t len;
	if(ntohs(rrset->rk.type) != LDNS_RR_TYPE_CNAME)
		return;
	d = (struct packed_rrset_data*)rrset->entry.data;
	if(d->count < 1)
		return;
	if(d->rr_len[0] < 3) /* at least rdatalen + 0byte root label */
		return;
	len = ldns_read_uint16(d->rr_data[0]);
	if(len != d->rr_len[0] - sizeof(uint16_t))
		return;
	len -= sizeof(uint16_t);
	if(dname_valid(d->rr_data[0]+sizeof(uint16_t), len) != len)
		return;
	*dname = d->rr_data[0]+sizeof(uint16_t);
	*dname_len = len;
}
