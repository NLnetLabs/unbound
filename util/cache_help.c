/*
 * util/cache_help.c - cache help functions
 *
 * Copyright (c) 2019, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains functions to perform cache-related tasks.
 */

#include "config.h"
#include "services/cache/rrset.h"
#include "sldns/rrdef.h"
#include "util/cache_help.h"
#include "util/net_help.h"
#include "util/data/msgreply.h"
#include "util/storage/lruhash.h"
#include "util/storage/slabhash.h"

void remove_rrset_from_env_cache( struct module_env* env, uint8_t* nm,
	size_t nmlen, uint16_t t, uint16_t c) {
	hashvalue_type h;
	struct query_info k;

	rrset_cache_remove(env->rrset_cache, nm, nmlen, t, c, 0);
	if(t == LDNS_RR_TYPE_SOA)
		rrset_cache_remove(env->rrset_cache, nm, nmlen, t, c,
			PACKED_RRSET_SOA_NEG);
	k.qname = nm;
	k.qname_len = nmlen;
	k.qtype = t;
	k.qclass = c;
	k.local_alias = NULL;
	h = query_info_hash(&k, 0);
	slabhash_remove(env->msg_cache, h, &k);
	if(t == LDNS_RR_TYPE_AAAA) {
		/* for AAAA also flush dns64 bit_cd packet */
		h = query_info_hash(&k, BIT_CD);
		slabhash_remove(env->msg_cache, h, &k);
	}
}
