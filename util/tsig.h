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
#include "util/locks.h"
#include "util/rbtree.h"
struct sldns_buffer;
struct config_file;
struct config_tsig_key;
struct regional;
struct tsig_calc_state_crypto;

/**
 * TSIG record, the RR that is in the packet.
 * The RR Type is TSIG and the RR class is CLASS_ANY. The TTL is 0.
 */
struct tsig_record {
	/** domain name of the RR, the key name. */
	uint8_t* key_name;
	/** length of the key_name */
	size_t key_name_len;
	/** the position of the TSIG RR in the packet, it is before the owner
	 * name. */
	size_t tsig_pos;
	/** the algorithm name, as a domain name. */
	uint8_t* algorithm_name;
	/** length of the algorithm_name */
	size_t algorithm_name_len;
	/** the signed time, 48bits on the wire */
	uint64_t signed_time;
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
	/** if the other size is 48bit, the timestamp in it. */
	uint64_t other_time;
};

/**
 * TSIG data. This keeps track of the information between packets,
 * for the TSIG signature, and state, errors, key.
 */
struct tsig_data {
	/** The key name, in wireformat */
	uint8_t* key_name;
	/** length of the key name */
	size_t key_name_len;
	/** The algo name, if the key could not be found. If NULL, it can
	 * be found in the tsig_key algo. */
	uint8_t* algo_name;
	/** length of the algo name */
	size_t algo_name_len;
	/** mac size */
	size_t mac_size;
	/** digest buffer */
	uint8_t* mac;
	/** original query ID */
	uint16_t original_query_id;
	/** the TSIG class */
	uint16_t klass;
	/** the TSIG TTL */
	uint16_t ttl;
	/** the time signed, 48bit */
	uint64_t time_signed;
	/** fudge amount of time_signed */
	uint16_t fudge;
	/** the TSIG error code */
	uint16_t error;
	/** other data length, 6 for other_time as failed time. */
	uint16_t other_len;
	/** if other len 6, this is 48bit time of error. */
	uint64_t other_time;
	/** For zone transfers, there are several packets and TSIGs,
	 * this keeps track of the tsig calculation state. It is malloced,
	 * and the tsig has to be deleted to free it. */
	struct tsig_calc_state_crypto* calc_state;
	/** For the first packet it is 0, for later packets 1. */
	int later_packet;
	/** The number of update only packets without a tsig. */
	int num_updates;
	/** The number of packets after which to sign with TSIG, 1 is every
	 * time. */
	int every_nth;
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
	 * In canonical format, that is in lowercase.
	 */
	uint8_t* wireformat_name;
	/** length of the wireformat_name */
	size_t wireformat_name_len;
	/** digest name, like "md5" */
	const char* digest;
	/** the maximum size of the digest from the algorithm, in bytes,
	 * like 16 for MD5, and 20 for SHA1. */
	size_t max_digest_size;
};

/**
 * TSIG key. This is used to sign and verify packets.
 */
struct tsig_key {
	/** the rbtree node */
	rbnode_type node;
	/** name of the key as string */
	char* name_str;
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
 * The TSIG key storage. Keys are stored by name.
 * They are read from config.
 */
struct tsig_key_table {
	/* Lock on the tsig key table and all keys.
	 * This lock is after the forwards, hints and anchor locks. */
	lock_rw_type lock;
	/* Tree of tsig keys, by wireformat name. */
	struct rbtree_type* tree;
};

/**
 * Create TSIG key table.
 * @return NULL on alloc failure.
 */
struct tsig_key_table* tsig_key_table_create(void);

/**
 * Delete TSIG key table. And the keys in it.
 * @param key_table: to delete.
 */
void tsig_key_table_delete(struct tsig_key_table* key_table);

/** Add a key to the TSIG key table. */
int tsig_key_table_add_key(struct tsig_key_table* key_table,
	struct config_tsig_key* s);

/** Delete a key from the TSIG key table. */
void tsig_key_table_del_key_fromstr(struct tsig_key_table* key_table,
	char* name);

/**
 * Apply config to the tsig key table.
 * @param key_table: the tsig key table.
 * @param cfg: the config to read.
 * @return false on failure.
 */
int tsig_key_table_apply_cfg(struct tsig_key_table* key_table,
	struct config_file* cfg);

/**
 * Find key in key table. Caller must hold lock on the table.
 * @param key_table: the tsig key table.
 * @param name: name to look for in wireformat.
 * @param namelen: length of name.
 * @return the found key or NULL if not found. The item is locked
 * by the key_table lock.
 */
struct tsig_key* tsig_key_table_search(struct tsig_key_table* key_table,
	uint8_t* name, size_t namelen);

/**
 * Find key in key table. Caller must hold lock on the table.
 * @param key_table: the tsig key table.
 * @param name: the name in string format, it is parsed to wireformat.
 * @return the found key or NULL if not found or NULL on parse error of the
 * key name as a domain name. The item is locked by the key_table lock.
 */
struct tsig_key* tsig_key_table_search_fromstr(
	struct tsig_key_table* key_table, char* name);

/**
 * Get memory usage of tsig key table.
 * @param tsig_key_table: the tsig key table.
 * @return memory use.
 */
size_t tsig_key_table_get_mem(struct tsig_key_table* tsig_key_table);

/**
 * Swap internal tree with preallocated entries. Caller should manage
 * the locks.
 * @param tsig_key_table: the tsig_key_table data structure.
 * @param data: the data structure used to take elements from. This contains
 * 	the old elements on return.
 */
void tsig_key_table_swap_tree(struct tsig_key_table* tsig_key_table,
	struct tsig_key_table* data);

/**
 * Delete TSIG key.
 * @param key: to delete
 */
void tsig_key_delete(struct tsig_key* key);

/**
 * See if an algorithm name is in the list of accepted algorithm names.
 * @param algo_name: string to check
 * @return 0 on failure.
 */
int tsig_algo_check_name(const char* algo_name);

/**
 * Get the TSIG algorithm for the algorithm name.
 * @param algo_name: string to find.
 * @return NULL on failure, tsig algorithm structure.
 */
struct tsig_algorithm* tsig_algo_find_name(const char* algo_name);

/**
 * Get the TSIG algorithm for the algorithm wireformat name.
 * @param algo: wireformat algorithm name to find.
 * @return NULL on failure, tsig algorithm structure.
 */
struct tsig_algorithm* tsig_algo_find_wire(uint8_t* algo);

/**
 * Sign pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, -1 on failure.
 * For a shared packet with contents. This signs a reply packet without
 * the prior hash, since there is no prior packet.
 */
int tsig_sign_shared(struct sldns_buffer* pkt, const uint8_t* name,
	const uint8_t* alg, const uint8_t* secret, size_t secret_len,
	uint64_t now);

/**
 * Verify pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, an error code otherwise.
 * For a shared packet with contents. This verifies a reply packet without
 * the prior hash, since there is no prior packet.
 * out 0 on success, on failure:
 * -1 for malformed, no tsig RR, or too large for buffer.
 * >0 rcode with a TSIG error code otherwise.
 */
int tsig_verify_shared(struct sldns_buffer* pkt, const uint8_t* name,
	const uint8_t* alg, const uint8_t* secret, size_t secret_len,
	uint64_t now);

/** Compare function for the key table keys. */
int tsig_key_compare(const void* v1, const void* v2);

/**
 * Find tsig key and create new tsig data.
 * @param key_table: the tsig key table.
 * @param name: key name in wireformat.
 * @param namelen: length of name.
 * @return NULL if not found, or alloc failure.
 */
struct tsig_data* tsig_create(struct tsig_key_table* key_table,
	uint8_t* name, size_t namelen);

/**
 * Find tsig key and create new tsig data.
 * @param key_table: the tsig key table.
 * @param name: key name string.
 * @return NULL if not found, or alloc failure, or could not parse string.
 */
struct tsig_data* tsig_create_fromstr(struct tsig_key_table* key_table,
	char* name);

/**
 * Delete tsig data.
 * @param tsig: the tsig data to delete.
 */
void tsig_delete(struct tsig_data* tsig);

/**
 * Get memory usage of tsig data.
 * @param rsig: the tsig data.
 * @return memory use.
 */
size_t tsig_get_mem(struct tsig_data* tsig);

/**
 * Sign a query with TSIG. Appends the TSIG record.
 * @param tsig: the tsig data, keeps state to verify reply.
 * @param pkt: query packet. position must be at end of packet.
 * @param key_table: the tsig key table is used to fetch the key details.
 * @param now: time to sign the query, the current time.
 * @return false on failure.
 */
int tsig_sign_query(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now);

/**
 * Verify a query with TSIG.
 * @param tsig: the tsig data, keep state to sign reply.
 * @param pkt: the query packet.
 * @param key: the key with algorithm, caller must hold lock.
 * @param rr: the tsig record parsed from the query.
 * @param now: time that is used, the current time.
 * @return rcode with failure for alloc failure or malformed wireformat.
 *	0 NOERROR is success, if tsig is nonNULL it has either verified
 *	or contains a TSIG error.
 */
int tsig_verify_query(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key* key, struct tsig_record* rr, uint64_t now);

/**
 * Look up key from TSIG in packet.
 * @param key_table: the tsig key table.
 * @param pkt: the packet to look at TSIG.
 * @param rr: the TSIG record parsed.
 * @param tsig_ret: the tsig key is returned here. Or it can be NULL, no TSIG.
 * @param region: if nonNULL used to allocate.
 * @param key: if the key is in the key_table the key is returned.
 *	On success the key table is locked for the key.
 * @return fail for alloc failure servfail or wireformat malformed formerr,
 *	success has 0 NOERROR, for no TSIG in packet with tsig returned NULL,
 *	and for key not found with tsig returned with a tsig error in it,
 *	and for key found with tsig returned with tsig in it.
 * After this call, the return value is the rcode for failure. Then the
 * tsig, is NULL for no TSIG, or nonNULL, with a TSIG error or content that
 * can be verified with tsig_verify_query.
 */
int tsig_lookup_key(struct tsig_key_table* key_table,
	struct sldns_buffer* pkt, struct tsig_record* rr,
	struct tsig_data** tsig_ret, struct regional* region,
	struct tsig_key** key);

/**
 * Parse a TSIG from the packet. Current position is just before it.
 * @param pkt: the packet.
 * @param rr: data filled in, with pointers to the packet buffer.
 *	The key name can be compressed.
 * @return 0 if OK, otherwise an RCODE.
 */
int tsig_parse(struct sldns_buffer* pkt, struct tsig_record* rr);

/**
 * Parse and verify the TSIG in query packet.
 * @param key_table: the tsig key table.
 * @param pkt: the packet
 * @param tsig: the tsig key is returned. Or it can be NULL.
 * @param region: if nonNULL used to allocate.
 * @param now: time that is used, the current time.
 * @return rcode with failure for alloc failure or malformed wireformat.
 *	0 NOERROR is success, if tsig is nonNULL it has either verified
 *	or contains a TSIG error.
 */
int tsig_parse_verify_query(struct tsig_key_table* key_table,
	struct sldns_buffer* pkt, struct tsig_data** tsig,
	struct regional* region, uint64_t now);

/**
 * Sign a reply with TSIG. Appends the TSIG record.
 * @param tsig: the tsig data.
 * @param pkt: the packet to sign.
 * @param key_table: the tsig key table is used to fetch the key details.
 * @param now: time to sign the query, the current time.
 * @return false on failure.
 */
int tsig_sign_reply(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now);

/**
 * Verify a reply with TSIG.
 * @param tsig: the tsig data.
 * @param pkt: the reply to verify.
 * @param key: the key with algorithm, caller must hold lock.
 * @param rr: the tsig record parsed from the reply.
 * @param now: time to sign the query, the current time.
 * @return false on failure, like
 *	alloc failure, wireformat malformed, did not verify.
 */
int tsig_verify_reply(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key* key, struct tsig_record* rr, uint64_t now);

/**
 * Verify a reply with TSIG.
 * @param tsig: the tsig data.
 * @param pkt: the reply to verify.
 * @param key_table: the tsig key table is used to fetch the key details.
 * @param now: time to sign the query, the current time.
 * @return false on failure, like
 *	alloc failure, wireformat malformed, did not verify.
 */
int tsig_parse_verify_reply(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now);

/**
 * Calculate reserved space for TSIG.
 * @param tsig: the tsig data
 * @return number of bytes to keep reserved for the TSIG added.
 */
size_t tsig_reserved_space(struct tsig_data* tsig);

/**
 * See if the packet has a TSIG record, or not.
 * @param pkt: the packet.
 * @return false if malformed or no tsig. If found, the position is
 *	just before the TSIG record. So it can be parsed.
 */
int tsig_find_rr(struct sldns_buffer* pkt);

/**
 * See if the packet as a TSIG, or not. Like tsig_find_rr, but it logs
 * no error for absence of a TSIG.
 * @param pkt: the packet
 * @return false if malformed, and false if no tsig. true if tsig,
 *	and the position is just before the TSIG record. So it can be parsed.
 */
int tsig_in_packet(struct sldns_buffer* pkt);

/**
 * Sign XFR reply with TSIG. Appends the TSIG record. Call for later
 * packets too.
 * @param tsig: the tsig data. It must be malloced for the crypto state.
 * @param pkt: the packet to sign.
 * @param key_table: the tsig key table is used to fetch the key details.
 * @param now: time to sign the query, the current time.
 * @param last_packet: set to true for the last packet, that needs to be
 *	TSIG signed.
 * @return false on failure.
 */
int tsig_sign_reply_xfr(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now, int last_packet);

/**
 * Verify XFR reply with TSIG.
 * @param tsig: the tsig data.
 * @param pkt: the reply to verify.
 * @param rr: the tsig record parsed from the reply.
 * @param now: time to sign the query, the current time.
 * @return false on failure, like
 *	alloc failure, wireformat malformed, did not verify.
 */
int tsig_verify_reply_xfr(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_record* rr, uint64_t now);

/**
 * Parse and verify XFR reply with TSIG. Position at the TSIG record, or
 * at end of packet if no TSIG record.
 * @param tsig: the tsig data.
 * @param pkt: the reply to verify.
 * @param key_table: the tsig key table is used to fetch the key details.
 * @param now: time to sign the query, the current time.
 * @param last_packet: set true for the last packet, it must have a TSIG.
 * @return false on failure, like
 *	alloc failure, wireformat malformed, did not verify.
 */
int tsig_parse_verify_reply_xfr(struct tsig_data* tsig,
	struct sldns_buffer* pkt, struct tsig_key_table* key_table,
	uint64_t now, int last_packet);

#endif /* UTIL_TSIG_H */
