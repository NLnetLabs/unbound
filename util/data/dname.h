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
#include "util/storage/lruhash.h"

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
 * Compare query dnames (uncompressed storage). The Dnames passed do not
 * have to be lowercased, comparison routine does this.
 * Dnames have to be valid format.
 * @param d1: dname to compare
 * @param d2: dname to compare
 * @return: -1, 0, or +1 depending on comparison results.
 * 	Sort order is first difference found. not the canonical ordering.
 */
int query_dname_compare(uint8_t* d1, uint8_t* d2);

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

/**
 * Compare dnames in packet (compressed). Dnames must be valid.
 * routine performs lowercasing, so the packet casing is preserved.
 * @param pkt: packet, used to resolve compression pointers.
 * @param d1: dname to compare
 * @param d2: dname to compare
 * @return: -1, 0, or +1 depending on comparison results.
 * 	Sort order is first difference found. not the canonical ordering.
 */
int dname_pkt_compare(ldns_buffer* pkt, uint8_t* d1, uint8_t* d2);

/**
 * Hash dname, label by label, lowercasing, into hashvalue.
 * Dname in query format (not compressed).
 * @param dname: dname to hash.
 * @param h: initial hash value.
 * @return: result hash value.
 */
hashvalue_t dname_query_hash(uint8_t* dname, hashvalue_t h);

/**
 * Hash dname, label by label, lowercasing, into hashvalue.
 * Dname in pkt format (compressed).
 * @param pkt: packet, for resolving compression pointers.
 * @param dname: dname to hash, pointer to the pkt buffer.
 * 	Must be valid format. No loops, etc.
 * @param h: initial hash value.
 * @return: result hash value.
 * 	Result is the same as dname_query_hash, even if compression is used.
 */
hashvalue_t dname_pkt_hash(ldns_buffer* pkt, uint8_t* dname, hashvalue_t h);

/**
 * Copy over a valid dname and decompress it.
 * @param pkt: packet to resolve compression pointers.
 * @param to: buffer of size from pkt_len function to hold result.
 * @param dname: pointer into packet where dname starts.
 */
void dname_pkt_copy(ldns_buffer* pkt, uint8_t* to, uint8_t* dname);

/**
 * Copy over a valid dname to a packet.
 * @param pkt: packet to copy to.
 * @param dname: dname to copy.
 * @return: 0 if not enough space in buffer.
 */
int dname_buffer_write(ldns_buffer* pkt, uint8_t* dname);

/**
 * Count the number of labels in an uncompressed dname in memory.
 * @param dname: pointer to uncompressed dname.
 * @return: count of labels, including root label, "com." has 2 labels.
 */
int dname_count_labels(uint8_t* dname);

/**
 * Count labels and dname length both, for uncompressed dname in memory.
 * @param dname: pointer to uncompressed dname.
 * @param size: length of dname, including root label.
 * @return: count of labels, including root label, "com." has 2 labels.
 */
int dname_count_size_labels(uint8_t* dname, size_t* size);

/**
 * Compare dnames, sorted not canonical, but by label.
 * Such that zone contents follows zone apex.
 * @param d1: first dname. pointer to uncompressed wireformat.
 * @param labs1: number of labels in first dname.
 * @param d2: second dname. pointer to uncompressed wireformat.
 * @param labs2: number of labels in second dname.
 * @param mlabs: number of labels that matched exactly.
 * @return: 0 for equal, -1 smaller, or +1 d1 larger than d2.
 */
int dname_lab_cmp(uint8_t* d1, int labs1, uint8_t* d2, int labs2, int* mlabs);

/** 
 * Debug helper. Print wireformat dname to output. 
 * @param out: like stdout or a file.
 * @param pkt: if not NULL, the packet for resolving compression ptrs.
 * @param dname: pointer to (start of) dname.
 */
void dname_print(FILE* out, ldns_buffer* pkt, uint8_t* dname);

/** 
 * Debug helper. Print dname to given string buffer (string buffer must
 * be at least 255 chars + 1 for the 0, in printable form.
 * This may lose information (? for nonprintable characters, or & if
 * the name is too long, # for a bad label length).
 * @param dname: uncompressed wireformat.
 * @param str: buffer of 255+1 length.
 */
void dname_str(uint8_t* dname, char* str);

#endif /* UTIL_DATA_DNAME_H */
