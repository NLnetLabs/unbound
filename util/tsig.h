/*
 * util/tsig.h - handle TSIG signatures.
 *
 * Copyright (c) 2023, NLnet Labs. All rights reserved.
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
 * This file provides functions to create and verify TSIG RRs.
 */

#ifndef UTIL_TSIG_H
#define UTIL_TSIG_H
struct sldns_buffer;

/**
 * TSIG record, the RR that is in the packet.
 * The RR Type is TSIG and the RR class is CLASS_ANY. The TTL is 0.
 */
struct tsig_record {
	/** domain name of the RR, the key name. */
	uint8_t* key_name;
	/** length of the key_name */
	size_t key_name_len;
	/** the algorithm name, as a domain name. */
	uint8_t* algorithm_name;
	/** length of the algorithm_name */
	size_t algorithm_name_len;
	/** the signed time, high part */
	uint16_t signed_time_high;
	/** the signed time, low part */
	uint32_t signed_time_low;
	/** the fudge time */
	uint16_t fudge_time;
	/** the mac size, uint16_t on the wire */
	size_t mac_size;
	/** the mac data */
	uint8_t* mac_data;
	/** the original query id */
	uint16_t original_query_id;
	/** the tsig error code */
	uint16_t error_code;
	/** length of the other data, uint16_t on the wire */
	size_t other_size;
	/** the other data */
	uint8_t* other_data;
};

/**
 * TSIG algorithm. This is the HMAC algorithm used for the TSIG mac.
 */
struct tsig_algorithm {
	/** Short name of the algorithm, like "hmac-md5" */
	char* short_name;
	/**
	 * Full wireformat name of the algorith, such as
	 * "hmac-md5.sig-alg.reg.int."
	 */
	uint8_t* wireformat_name;
	/** length of the wireformat_name */
	size_t wireformat_name_len;
};

/**
 * TSIG key. This is used to sign and verify packets.
 */
struct tsig_key {
	/** name of the key as string */
	char* name_str;
	/** algorithm string */
	char* algo_str;
	/** the algorithm structure */
	struct tsig_algorithm* algo;
	/**
	 * Name of the key, in wireformat.
	 * The key name has to be transferred as a domain name, of the TSIG
	 * RR and thus the key name has to be a wireformat domain name.
	 */
	uint8_t* name;
	/** length of name */
	size_t name_len;
	/** the data, with the secret portion of the key. decoded from the
	 * base64 string with the secret. */
	uint8_t* data;
	/** the size of the data */
	size_t data_len;
};

/**
 * Verify pkt with the name (domain name), algorithm and key.
 * out 0 on success, an error code otherwise.
 */
int tsig_verify(struct sldns_buffer* pkt, const uint8_t* name,
	const uint8_t* alg, const uint8_t* secret, size_t secret_len,
	uint64_t now);

#endif /* UTIL_TSIG_H */
