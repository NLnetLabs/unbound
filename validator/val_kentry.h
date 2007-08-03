/*
 * validator/val_kentry.h - validator key entry definition.
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

#ifndef VALIDATOR_VAL_KENTRY_H
#define VALIDATOR_VAL_KENTRY_H
struct packed_rrset_data;
#include "util/storage/lruhash.h"

/**
 * A key entry for the validator.
 * This may or may not be a trusted key.
 * This is what is stored in the key cache.
 * This is the key part for the cache; the key entry key.
 */
struct key_entry_key {
	/** lru hash entry */
	struct lruhash_entry entry;
	/** name of the key */
	uint8_t* name;
	/** length of name */
	size_t namelen;
	/** class of the key, host byteorder */
	uint16_t key_class;
};

/**
 * Key entry for the validator.
 * Contains key status.
 * This is the data part for the cache, the key entry data.
 *
 * Can be in three basic states:
 * 	isbad=0:		good key
 * 	isbad=1:		bad key
 * 	isbad=0 && rrset=0:	insecure space.
 */
struct key_entry_data {
	/** the TTL of this entry (absolute time) */
	uint32_t ttl;
	/** the key rrdata. can be NULL to signal keyless name. */
	struct packed_rrset_data* rrset_data;
	/** DNS RR type of the rrset data */
	uint16_t rrset_type;
	/** if the key is bad: Bogus or malformed */
	uint8_t isbad;
};

/** function for lruhash operation */
size_t key_entry_sizefunc_t(void* key, void* data);

/** function for lruhash operation */
int key_entry_compfunc_t(void* k1, void* k2);

/** function for lruhash operation */
void key_entry_delkeyfunc_t(void* key, void* userarg, int islocked);

/** function for lruhash operation */
void key_entry_deldatafunc_t(void* data, void* userarg);

/** calculate hash for key entry 
 * @param kk: key entry. The lruhash entry.hash value is filled in.
 */
void key_entry_hash(struct key_entry_key* kk);

#endif /* VALIDATOR_VAL_KENTRY_H */
