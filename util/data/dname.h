/*
 * util/data/dname.h - domain name routines
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
 * This file contains functions to deal with domain names (dnames).
 *
 * Some of the functions deal with domain names as a wireformat buffer,
 * with a length.
 */

#ifndef UTIL_DATA_DNAME_H
#define UTIL_DATA_DNAME_H

/** 
 * Determine length of dname in buffer, no compression ptrs allowed, 
 * @param query: the ldns buffer, current position at start of dname.
 *	at end, position is at end of the dname.
 * @return: 0 on parse failure, or length including ending 0 of dname. 
 */
size_t query_dname_len(ldns_buffer* query);

/** lowercase query dname */
void query_dname_tolower(uint8_t* dname, size_t len);

/**
 * Determine correct, compressed, dname present in packet.
 * Checks for parse errors.
 * @param pkt: packet to read from (from current start position).
 * @return: 0 on parse error.
 *	At exit the position is right after the (compressed) dname.
 *	Compression pointers are followed and checked for loops.
 *	The uncompressed wireformat length is returned.
 */
size_t pkt_dname_len(ldns_buffer* pkt);

#endif /* UTIL_DATA_DNAME_H */
