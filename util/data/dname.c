/*
 * util/data/dname.h - domain name handling
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
 * This file contains domain name handling functions.
 */

#include "config.h"
#include "util/data/dname.h"
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

void 
query_dname_tolower(uint8_t* dname, size_t len)
{
	/* the dname is stored uncompressed */
	uint8_t labellen;
	log_assert(len > 0);
	labellen = *dname;
	while(labellen) {
		dname++;
		while(labellen--) {
			*dname = (uint8_t)tolower((int)*dname);
			dname++;
		}
		labellen = *dname;
	}
}


