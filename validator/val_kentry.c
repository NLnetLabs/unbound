/*
 * validator/val_kentry.c - validator key entry definition.
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
 * This file contains functions for dealing with validator key entries.
 */
#include "config.h"
#include "validator/val_kentry.h"
#include "util/data/packed_rrset.h"
#include "util/data/dname.h"
#include "util/storage/lookup3.h"

size_t 
key_entry_sizefunc_t(void* key, void* data)
{
	struct key_entry_key* kk = (struct key_entry_key*)key;
	struct key_entry_data* kd = (struct key_entry_data*)data;
	size_t s = sizeof(*kk) + kk->namelen;
	s += sizeof(*kd);
	if(kd->rrset_data)
		s += packed_rrset_sizeof(kd->rrset_data);
	return s;
}

int 
key_entry_compfunc_t(void* k1, void* k2)
{
	struct key_entry_key* n1 = (struct key_entry_key*)k1;
	struct key_entry_key* n2 = (struct key_entry_key*)k2;
	if(n1->key_class != n2->key_class) {
		if(n1->key_class < n2->key_class)
			return -1;
		return 1;
	}
	return query_dname_compare(n1->name, n2->name);
}

void 
key_entry_delkeyfunc_t(void* key, void* ATTR_UNUSED(userarg), int islocked)
{
	struct key_entry_key* kk = (struct key_entry_key*)key;
	if(!key)
		return;
	if(islocked) {
		lock_rw_unlock(&kk->entry.lock);
	}
	free(kk->name);
	free(kk);
}

void 
key_entry_deldatafunc_t(void* data, void* ATTR_UNUSED(userarg))
{
	struct key_entry_data* kd = (struct key_entry_data*)data;
	free(kd->rrset_data);
	free(kd);
}

void 
key_entry_hash(struct key_entry_key* kk)
{
	kk->entry.hash = 0x654;
	kk->entry.hash = hashlittle(&kk->key_class, sizeof(kk->key_class), 
		kk->entry.hash);
	kk->entry.hash = dname_query_hash(kk->name, kk->entry.hash);
}
