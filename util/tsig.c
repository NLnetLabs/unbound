/*
 * util/tsig.c - handle TSIG signatures.
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

#include "config.h"
#include "util/tsig.h"
#include "util/config_file.h"
#include "util/log.h"
#include "util/net_help.h"
#include "sldns/parseutil.h"
#include "sldns/pkthdr.h"
#include "sldns/rrdef.h"
#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "util/data/msgparse.h"
#include "util/data/dname.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>

/** Fudge time to allow in signed time for TSIG records. In seconds. */
#define TSIG_FUDGE_TIME 300

/**
 * The list of TSIG algorithms. It has short_name, wireformat_name,
 * wireformat_name_len, digest, max_digest_size.
 */
static struct tsig_algorithm tsig_algorithm_table[] = {
	{ "hmac-md5",
	  (uint8_t*)"\x08hmac-md5\x07sig-alg\x03reg\x03int\x00", 26,
	  "md5", 16 },
	{ "hmac-sha1", (uint8_t*)"\x09hmac-sha1\x00", 11, "sha1", 20 },
	{ "hmac-sha224", (uint8_t*)"\x0Bhmac-sha224\x00", 13, "sha224", 28 },
	{ "hmac-sha256", (uint8_t*)"\x0Bhmac-sha256\x00", 13, "sha256", 32 },
	{ "hmac-sha384", (uint8_t*)"\x0Bhmac-sha384\x00", 13, "sha384", 48 },
	{ "hmac-sha512", (uint8_t*)"\x0Bhmac-sha512\x00", 13, "sha512", 64 }
};

int
tsig_key_compare(const void* v1, const void* v2)
{
	struct tsig_key* a = (struct tsig_key*)v1;
	struct tsig_key* b = (struct tsig_key*)v2;

	return query_dname_compare(a->name, b->name);
}

struct tsig_key_table*
tsig_key_table_create(void)
{
	struct tsig_key_table* key_table;
	key_table = (struct tsig_key_table*)calloc(1, sizeof(*key_table));
	if(!key_table)
		return NULL;
	key_table->tree = rbtree_create(&tsig_key_compare);
	if(!key_table->tree) {
		free(key_table);
		return NULL;
	}
	lock_rw_init(&key_table->lock);
	lock_protect(&key_table->lock, key_table->tree,
		sizeof(*key_table->tree));
	return key_table;
}

/** Delete the tsig key table key. */
static void
tsig_key_table_delete_key(rbnode_type* node, void* ATTR_UNUSED(arg))
{
	struct tsig_key* key = (struct tsig_key*)node->key;
	tsig_key_delete(key);
}

void
tsig_key_table_delete(struct tsig_key_table* key_table)
{
	if(!key_table)
		return;
	lock_rw_destroy(&key_table->lock);
	if(key_table->tree) {
		traverse_postorder(key_table->tree, &tsig_key_table_delete_key,
			NULL);
		free(key_table->tree);
	}
	free(key_table);
}

/** Create a tsig_key */
static struct tsig_key*
tsig_key_create(const char* name, const char* algorithm, const char* secret)
{
	struct tsig_key* key = (struct tsig_key*)calloc(1, sizeof(*key));
	int ret;
	if(!key) {
		log_err("out of memory");
		return NULL;
	}
	key->node.key = key;
	key->name_str = strdup(name);
	if(!key->name_str) {
		log_err("out of memory");
		tsig_key_delete(key);
		return NULL;
	}
	key->algo = tsig_algo_find_name(algorithm);
	if(!key->algo) {
		log_err("tsig key %s could not parse algorithm '%s'",
			name, algorithm);
		tsig_key_delete(key);
		return NULL;
	}
	key->name = sldns_str2wire_dname(name, &key->name_len);
	if(!key->name) {
		log_err("tsig key %s could not parse name into wireformat",
			name);
		tsig_key_delete(key);
		return NULL;
	}

	key->data_len = sldns_b64_pton_calculate_size(strlen(secret));
	if(key->data_len == 0)
		key->data = NULL;
	else	key->data = malloc(key->data_len);
	if(!key->data) {
		log_err("out of memory");
		tsig_key_delete(key);
		return NULL;
	}
	ret = sldns_b64_pton(secret, key->data, key->data_len);
	if(ret == -1 || ret > (int)key->data_len) {
		log_err("tsig key %s could not base64 decode secret blob",
			name);
		tsig_key_delete(key);
		return NULL;
	}
	key->data_len = ret;

	return key;
}

int
tsig_key_table_add_key(struct tsig_key_table* key_table,
	struct config_tsig_key* s)
{
	struct tsig_key* key;
	key = tsig_key_create(s->name, s->algorithm, s->secret);
	if(!key)
		return 0;
	/* Wipe the secret from config, it is in the key_table. */
	if(s->secret[0] != 0)
		explicit_bzero(s->secret, strlen(s->secret));

	lock_rw_wrlock(&key_table->lock);
	if(!rbtree_insert(key_table->tree, &key->node)) {
		log_warn("duplicate tsig-key: %s", key->name_str);
		lock_rw_unlock(&key_table->lock);
		tsig_key_delete(key);
		return 0;
	}
	lock_rw_unlock(&key_table->lock);
	return 1;
}

void
tsig_key_table_del_key_fromstr(struct tsig_key_table* key_table,
	char* name)
{
	uint8_t buf[LDNS_MAX_DOMAINLEN+1];
	size_t len = sizeof(buf);
	struct tsig_key k, *key;
	rbnode_type* node;
	if(!sldns_str2wire_dname_buf(name, buf, &len)) {
		log_err("could not parse '%s'", name);
		return;
	}

	k.node.key = &k;
	k.name = buf;
	k.name_len = len;
	node = rbtree_delete(key_table->tree, &k);
	if(!node)
		return;
	key = (struct tsig_key*)node->key;
	tsig_key_delete(key);
}

int
tsig_key_table_apply_cfg(struct tsig_key_table* key_table,
	struct config_file* cfg)
{
	struct config_tsig_key* s;
	for(s = cfg->tsig_keys; s; s = s->next) {
		if(!tsig_key_table_add_key(key_table, s))
			return 0;
	}
	return 1;
}

struct tsig_key*
tsig_key_table_search(struct tsig_key_table* key_table, uint8_t* name,
	size_t namelen)
{
	rbnode_type* node;
	struct tsig_key k;
	k.node.key = &k;
	k.name = name;
	k.name_len = namelen;
	node = rbtree_search(key_table->tree, &k);
	if(!node) return NULL;
	return (struct tsig_key*)node->key;
}

void tsig_key_delete(struct tsig_key* key)
{
	if(!key)
		return;
	free(key->name_str);
	free(key->name);
	if(key->data) {
		/* The secret data is removed. */
		if(key->data_len > 0)
			explicit_bzero(key->data, key->data_len);
		free(key->data);
	}
	free(key);
}

int
tsig_algo_check_name(const char* algo_name)
{
	/* It is either the long name for md5, "hmac-md5.sig-alg.reg.int."
	 * or a short name, "hmac-sha256", or a digest name "sha256".
	 * The name is case insensitive. */
	if(strncasecmp(algo_name, "hmac-", 5) == 0) {
		/* The name starts with 'hmac-'. */
		if(strncasecmp(algo_name+5, "sha", 3) == 0) {
			/* sha1 */
			if(strcasecmp(algo_name+8, "1") == 0 ||
				strcasecmp(algo_name+8, "1.") == 0)
				return 1;
			/* sha224 */
			if(strcasecmp(algo_name+8, "224") == 0 ||
				strcasecmp(algo_name+8, "224.") == 0)
				return 1;
			/* sha256 */
			if(strcasecmp(algo_name+8, "256") == 0 ||
				strcasecmp(algo_name+8, "256.") == 0)
				return 1;
			/* sha384 */
			if(strcasecmp(algo_name+8, "384") == 0 ||
				strcasecmp(algo_name+8, "384.") == 0)
				return 1;
			/* sha512 */
			if(strcasecmp(algo_name+8, "512") == 0 ||
				strcasecmp(algo_name+8, "512.") == 0)
				return 1;
		}
		if(strncasecmp(algo_name+5, "md5", 3) == 0) {
			/* 'hmac-md5' or 'hmac-md5.' */
			if(strcasecmp(algo_name+8, "") == 0 ||
				strcasecmp(algo_name+8, ".") == 0)
				return 1;
			if(strcasecmp(algo_name,
				"hmac-md5.sig-alg.reg.int.") == 0
			  || strcasecmp(algo_name,
			  	"hmac-md5.sig-alg.reg.int") == 0)
				return 1;
		}
	}
	if(strncasecmp(algo_name, "sha", 3) == 0) {
		if(strcasecmp(algo_name+3, "1") == 0)
			return 1;
		if(strcasecmp(algo_name+3, "224") == 0)
			return 1;
		if(strcasecmp(algo_name+3, "256") == 0)
			return 1;
		if(strcasecmp(algo_name+3, "384") == 0)
			return 1;
		if(strcasecmp(algo_name+3, "512") == 0)
			return 1;
	}
	if(strcasecmp(algo_name, "md5") == 0)
		return 1;
	return 0;
}

struct tsig_algorithm*
tsig_algo_find_name(const char* algo_name)
{
	size_t i;
	char buf[40];
	const char* lookfor = algo_name;
	if(algo_name == NULL || algo_name[0] == 0 ||
		strlen(algo_name)+5 >= sizeof(buf))
		return NULL;
	if(strncasecmp(algo_name, "hmac-", 5) != 0) {
		snprintf(buf, sizeof(buf), "hmac-%s", algo_name);
		lookfor = buf;
	} else {
		if(algo_name[strlen(algo_name)-1] == '.') {
			/* Remove trailing '.' */
			snprintf(buf, sizeof(buf), "%s", algo_name);
			buf[strlen(buf)-1] = 0;
			lookfor = buf;
		}
		if(strcasecmp(lookfor, "hmac-md5.sig-alg.reg.int") == 0)
			lookfor = "hmac-md5"; /* Look for short name. */
	}

	for(i=0; i<sizeof(tsig_algorithm_table)/sizeof(*tsig_algorithm_table);
	    i++) {
		if(strcasecmp(tsig_algorithm_table[i].short_name, lookfor)
			== 0)
			return &tsig_algorithm_table[i];
	}
	return NULL;
}

/**
 * Skip packet query rr.
 * @param pkt: the packet, position before the rr, ends after the rr.
 * @return 0 on failure.
 */
static int
skip_pkt_query_rr(struct sldns_buffer* pkt)
{
	/* skip qname */
	if(sldns_buffer_remaining(pkt) < 1)
		return 0;
	if(!pkt_dname_len(pkt))
		return 0; /* malformed qname */
	if(sldns_buffer_remaining(pkt) < 4)
		return 0;
	/* skip type and class */
	sldns_buffer_skip(pkt, 2 * sizeof(uint16_t));
	return 1;
}

/**
 * Skip the packet query rrs. The position must be after the header.
 * @param pkt: the packet. The end position is after the number of query
 *	section records.
 * @param num: Limit of the number of records we want to parse.
 * @return 1 on success, 0 on failure.
 */
static int
skip_pkt_query_rrs(struct sldns_buffer* pkt, int num)
{
	int i;
	for(i=0; i<num; i++) {
		if(!skip_pkt_query_rr(pkt))
			return 0;
	}
	return 1;
}

/**
 * Verify pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, an error code otherwise.
 */
int
tsig_verify(sldns_buffer* pkt, const uint8_t* name, const uint8_t* alg,
		const uint8_t* secret, size_t secret_len, uint64_t now)
{
	size_t   n_rrs;
	size_t   end_of_message;
	uint8_t  mac[1024];
	uint16_t mac_sz;
	uint8_t  hmac_result[1024];
	unsigned int hmac_result_len;
	size_t   pos;
	uint16_t other_len;
	size_t   algname_size;
	uint64_t time_signed;
	uint16_t fudge;
	const EVP_MD* digester;
	uint8_t* tsig_name, *tsig_algo;
	uint16_t rdlength;
	uint16_t current_query_id;

	if(sldns_buffer_limit(pkt) < LDNS_HEADER_SIZE) {
		verbose(VERB_ALGO, "No TSIG, packet too short");
		return -1;
	}
	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1) {
		verbose(VERB_ALGO, "No TSIG found, ARCOUNT == 0");
		return -1;
	}
	LDNS_ARCOUNT_SET( sldns_buffer_begin(pkt)
	                , LDNS_ARCOUNT(sldns_buffer_begin(pkt)) - 1);
	n_rrs = LDNS_ANCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_NSCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_ARCOUNT(sldns_buffer_begin(pkt));

	sldns_buffer_rewind(pkt);
	sldns_buffer_skip(pkt, LDNS_HEADER_SIZE);

	/* Skip qnames. */
	if(!skip_pkt_query_rrs(pkt, LDNS_QDCOUNT(sldns_buffer_begin(pkt)))) {
		verbose(VERB_ALGO, "No TSIG, query section RRs malformed");
		return -1;
	}
	/* Skip all rrs. */
	if(!skip_pkt_rrs(pkt, n_rrs)) {
		verbose(VERB_ALGO, "No TSIG, packet RRs are malformed");
		return -1;
	}
	end_of_message = sldns_buffer_position(pkt);

	/* Skip TSIG name. */
	if(sldns_buffer_remaining(pkt) < 1) {
		verbose(VERB_ALGO, "No TSIG, no tsig name");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	tsig_name = sldns_buffer_current(pkt);
	if(!pkt_dname_len(pkt)) {
		verbose(VERB_ALGO, "No TSIG, tsig name malformed");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(dname_pkt_compare(pkt, tsig_name, (uint8_t*)name) != 0) {
		sldns_buffer_set_position(pkt, end_of_message);
		return LDNS_TSIG_ERROR_BADKEY;
	}
	pos = sldns_buffer_position(pkt);             /* Append pos */

	/* Skip type, class, TTL and rdlength */
	if(sldns_buffer_remaining(pkt) < 2 /* type */ + 2 /* class */
		+ 4 /* TTL */ + 2 /* rdlength */) {
		verbose(VERB_ALGO, "No TSIG, TSIG RR malformed, packet too short");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(sldns_buffer_read_u16(pkt) != LDNS_RR_TYPE_TSIG) {
		verbose(VERB_ALGO, "No TSIG found, wrong RR type");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	sldns_buffer_skip(pkt, sizeof(uint16_t)       /* skip class */
	                     + sizeof(uint32_t));     /* skip TTL */
	rdlength = sldns_buffer_read_u16(pkt);        /* read rdlength */
	if(sldns_buffer_remaining(pkt) < rdlength) {
		verbose(VERB_ALGO, "TSIG rdlength wrong, packet too short");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}

	/* Read the TSIG rdata. */
	if(rdlength < 18 /* Rdata length with '.' names, zero lengths. */) {
		verbose(VERB_ALGO, "TSIG rdata too short");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	tsig_algo = sldns_buffer_current(pkt);
	/* The algorithm name MUST be uncompressed wireformat (RFC8945),
	 * check that it is. Fails on malformed and skips the name. */
	if(!(algname_size = query_dname_len(pkt))) {
		verbose(VERB_ALGO, "TSIG algorithm name malformed");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(query_dname_compare(alg, tsig_algo) != 0) {
		sldns_buffer_set_position(pkt, end_of_message);
		return LDNS_TSIG_ERROR_BADKEY;
	}
	if(rdlength < algname_size + 6 /* time */ + 2 /* fudge */
		+ 2 /* mac size */) {
		verbose(VERB_ALGO, "TSIG rdata too short for timestamp");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	time_signed = sldns_buffer_read_u48(pkt);     /* time signed */
	fudge = sldns_buffer_read_u16(pkt);           /* fudge */
	mac_sz = sldns_buffer_read_u16(pkt);          /* mac size */

	if(rdlength < algname_size + 6 /* time */ + 2 /* fudge */
		+ 2 /* mac size */ + mac_sz + 2 /* origid */
		+ 2 /* error_code */ + 2 /* otherlen */) {
		verbose(VERB_ALGO, "TSIG rdata too short for mac");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(mac_sz > 16384) {
		/* the hash should not be too big, really 512/8=64 bytes */
		verbose(VERB_ALGO, "TSIG mac too large");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(mac_sz > sizeof(mac)) {
		verbose(VERB_ALGO, "TSIG mac too large for buffer");
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	memcpy(mac,sldns_buffer_current(pkt),mac_sz); /* copy mac */
	sldns_buffer_skip(pkt, mac_sz);               /* skip mac */

	/* Set ID to original ID */
	current_query_id = sldns_buffer_read_u16_at(pkt, 0);
	sldns_buffer_write_u16_at(pkt, 0, sldns_buffer_read_u16(pkt));
	sldns_buffer_skip(pkt, sizeof(uint16_t));     /* skip error */
	other_len = sldns_buffer_read_u16(pkt);       /* other len */
	if(rdlength < algname_size + 6 /* time */ + 2 /* fudge */
		+ 2 /* mac size */ + mac_sz + 2 /* origid */
		+ 2 /* error_code */ + 2 /* otherlen */ + other_len) {
		verbose(VERB_ALGO, "TSIG rdata too short for Other Data");
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(rdlength > algname_size + 6 /* time */ + 2 /* fudge */
		+ 2 /* mac size */ + mac_sz + 2 /* origid */
		+ 2 /* error_code */ + 2 /* otherlen */ + other_len) {
		/* Trailing data in TSIG RR */
		verbose(VERB_ALGO, "TSIG rdata wrong, it has trailing data");
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}
	if(other_len > 16) {
		verbose(VERB_ALGO, "TSIG Other Len too large");
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		sldns_buffer_set_position(pkt, end_of_message);
		return -1;
	}

	/* The buffer for the packet is used for the digest buffer
	 * for this message. It fits in the buffer. */
	/* move CLASS (uint16_t) and TTL (uint32_t) -> TYPE position */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, pos + sizeof(uint16_t) /* type */)
	       , sizeof(uint16_t) + sizeof(uint32_t));
	pos +=   sizeof(uint16_t) + sizeof(uint32_t);

	/* Append Algorithm Name, Time signed (uint48_t), Fudge (uint16_t) */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, pos + sizeof(uint16_t) /* type */
	                                  + sizeof(uint16_t) /* rdlength */)
	       , algname_size + 6 /* sizeof(uint48_t) */ + sizeof(uint16_t));
	pos +=   algname_size + 6 /* sizeof(uint48_t) */ + sizeof(uint16_t);

	/* Append Error (uint16_t), Other Len (uint16_t), Other Data */
	memmove( sldns_buffer_at(pkt, pos)
	       , sldns_buffer_at(pkt, sldns_buffer_position(pkt)
	                            - 2 * sizeof(uint16_t))
	       , 2 * sizeof(uint16_t) + other_len);
	pos +=   2 * sizeof(uint16_t) + other_len;

	digester = EVP_sha256();
	hmac_result_len = sizeof(hmac_result);
	HMAC( digester, secret, secret_len, sldns_buffer_begin(pkt), pos
	    , hmac_result, &hmac_result_len);
	if(CRYPTO_memcmp(mac, hmac_result, hmac_result_len) == 0) {
		/* Restore the query id of the packet. */
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		sldns_buffer_set_position(pkt, end_of_message);
		/* The TSIG has verified. */
		return now > time_signed ?
		     ( time_signed - now > fudge ? LDNS_TSIG_ERROR_BADTIME : 0 )
		     : now - time_signed > fudge ? LDNS_TSIG_ERROR_BADTIME : 0 ;
	}
	/* Restore the query id of the packet. */
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);
	sldns_buffer_set_position(pkt, end_of_message);
	return LDNS_TSIG_ERROR_BADSIG;
}

struct tsig_data*
tsig_create(struct tsig_key_table* key_table, uint8_t* name, size_t namelen)
{
	struct tsig_key* key;
	struct tsig_data* tsig;
	lock_rw_rdlock(&key_table->lock);
	key = tsig_key_table_search(key_table, name, namelen);
	if(!key) {
		lock_rw_unlock(&key_table->lock);
		return NULL;
	}
	/* the key table lock is also on the returned key */

	tsig = calloc(1, sizeof(*tsig));
	if(!tsig) {
		lock_rw_unlock(&key_table->lock);
		log_err("out of memory");
		return NULL;
	}
	tsig->key_name_len = key->name_len;
	tsig->key_name = memdup(key->name, key->name_len);
	if(!tsig->key_name) {
		lock_rw_unlock(&key_table->lock);
		tsig_delete(tsig);
		log_err("out of memory");
		return NULL;
	}
	tsig->algorithm_name_len = key->algo->wireformat_name_len;
	tsig->mac_size = key->algo->max_digest_size;
	tsig->mac = calloc(1, tsig->mac_size);
	if(!tsig->mac) {
		lock_rw_unlock(&key_table->lock);
		tsig_delete(tsig);
		log_err("out of memory");
		return NULL;
	}
	lock_rw_unlock(&key_table->lock);

	tsig->original_query_id = 0;
	tsig->klass = LDNS_RR_CLASS_ANY;
	tsig->ttl = 0;
	tsig->time_signed = 0;
	tsig->fudge = TSIG_FUDGE_TIME; /* seconds */
	tsig->error = 0;
	tsig->other_len = 0;
	tsig->other_time = 0;
	return tsig;
}

struct tsig_data*
tsig_create_fromstr(struct tsig_key_table* key_table, char* name)
{
	uint8_t buf[LDNS_MAX_DOMAINLEN+1];
	size_t len = sizeof(buf);
	if(!sldns_str2wire_dname_buf(name, buf, &len)) {
		log_err("could not parse '%s'", name);
		return NULL;
	}
	return tsig_create(key_table, buf, len);
}

void
tsig_delete(struct tsig_data* tsig)
{
	if(!tsig) return;
	free(tsig->key_name);
	free(tsig->mac);
	free(tsig);
}

size_t
tsig_reserved_space(struct tsig_data* tsig)
{
	if(!tsig)
		return 0;
	return
		tsig->key_name_len	/* Owner */
		+ sizeof(uint16_t)	/* Type */
		+ sizeof(uint16_t)	/* Class */
		+ sizeof(uint32_t)	/* TTL */
		+ sizeof(uint16_t)	/* RDATA length */
		+ tsig->algorithm_name_len /* Algorithm */
		+ 6 /* 48bit */		/* Signed time */
		+ sizeof(uint16_t)	/* Fudge */
		+ sizeof(uint16_t)	/* Mac size */
		+ tsig->mac_size	/* Mac data */
		+ sizeof(uint16_t)	/* Original query ID */
		+ sizeof(uint16_t)	/* Error code */
		+ sizeof(uint16_t)	/* Other size */
		+ 6;			/* 48bit in case of Other data */
}

/**
 * Calculate the digest for the TSIG algorithm over the packet.
 * Called must hold locks, on key. This routine performs one-host
 * calculation.
 * @param key: the key, and algorithm.
 * @param pkt: packet with contents.
 * @param tsig: where to store the result.
 * @return false on failure.
 */
static int
tsig_algo_calc(struct tsig_key* key, struct sldns_buffer* pkt,
	struct tsig_data* tsig)
{
	const EVP_MD* evp_md;
	unsigned int hmac_result_len;
	unsigned char* hmac_result;

	/* Setup digester algorithm */
	evp_md = EVP_get_digestbyname(key->algo->digest);
	if(!evp_md) {
		/* Could not fetch algorithm. */
		char name[256], buf[1024];
		dname_str(key->name, name);
		snprintf(buf, sizeof(buf), "EVP_get_digestbyname failed for %s %s",
			name, key->algo->digest);
		log_crypto_err(buf);
		return 0;
	}

	/* Perform calculation */
	hmac_result_len = tsig->mac_size;
	hmac_result = HMAC(evp_md, key->data, key->data_len,
		sldns_buffer_begin(pkt), sldns_buffer_position(pkt),
		tsig->mac, &hmac_result_len);
	if(!hmac_result) {
		/* The HMAC calculation failed. */
		char name[256], buf[1024];
		dname_str(key->name, name);
		snprintf(buf, sizeof(buf), "HMAC failed for %s %s",
			name, key->algo->digest);
		log_crypto_err(buf);
		return 0;
	}
	return 1;
}

int
tsig_sign_query(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now)
{
	size_t aftername_pos;
	struct tsig_key* key;
	if(!tsig)
		return 0;
	tsig->time_signed = now;
	tsig->fudge = TSIG_FUDGE_TIME; /* seconds */
	if(sldns_buffer_remaining(pkt) < tsig_reserved_space(tsig)) {
		/* Not enough space in buffer for packet and TSIG. */
		return 0;
	}
	lock_rw_rdlock(&key_table->lock);
	key = tsig_key_table_search(key_table, tsig->key_name,
		tsig->key_name_len);
	if(!key) {
		/* The tsig key has disappeared from the key table. */
		lock_rw_unlock(&key_table->lock);
		return 0;
	}

	tsig->original_query_id = sldns_buffer_read_u16_at(pkt, 0);

	/* What is signed is this buffer:
	 * <packet with original query id and ARCOUNT without TSIG>
	 * <name> TSIG owner is key name
	 * u16 class, u32 TTL, <name> algorithm_name, u48 time_signed,
	 * u16 fudge, u16 error, u16 other_len, <data> other_data. */
	/* That fits in the current buffer, since the reserved space for
	 * the TSIG record is larger. */

	/* Write uncompressed TSIG owner, it is the key name. */
	sldns_buffer_write(pkt, tsig->key_name, tsig->key_name_len);
	aftername_pos = sldns_buffer_position(pkt);
	sldns_buffer_write_u16(pkt, tsig->klass);
	sldns_buffer_write_u32(pkt, tsig->ttl);
	sldns_buffer_write(pkt, key->algo->wireformat_name,
		key->algo->wireformat_name_len);
	sldns_buffer_write_u48(pkt, tsig->time_signed);
	sldns_buffer_write_u16(pkt, tsig->fudge);
	sldns_buffer_write_u16(pkt, tsig->error);
	sldns_buffer_write_u16(pkt, tsig->other_len);
	if(tsig->other_len != 0)
		sldns_buffer_write_u48(pkt, tsig->other_time);

	/* Sign it */
	if(!tsig_algo_calc(key, pkt, tsig)) {
		/* Failure to calculate digest. */
		lock_rw_unlock(&key_table->lock);
		return 0;
	}

	/* Append TSIG record */
	/* The record appended consists of:
	 * owner name, u16 type, u16 class, u32 TTL, u16 rdlength,
	 * algo name, u48 signed_time, u16 fudge, u16 mac_len, mac data,
	 * u16 original_query_id, u16 error, u16 other_len, other data.
	 */
	sldns_buffer_set_position(pkt, aftername_pos);
	sldns_buffer_write_u16(pkt, LDNS_RR_TYPE_TSIG);
	sldns_buffer_write_u16(pkt, tsig->klass);
	sldns_buffer_write_u32(pkt, tsig->ttl);

	/* rdlength */
	sldns_buffer_write_u16(pkt, key->algo->wireformat_name_len
		+ 6 + 2 + 2 /* time,fudge,maclen */ + tsig->mac_size
		+ 2 + 2 + 2 /* id,error,otherlen */ + tsig->other_len);
	sldns_buffer_write(pkt, key->algo->wireformat_name,
		key->algo->wireformat_name_len);
	sldns_buffer_write_u48(pkt, tsig->time_signed);
	sldns_buffer_write_u16(pkt, tsig->fudge);
	sldns_buffer_write_u16(pkt, tsig->mac_size);
	sldns_buffer_write(pkt, tsig->mac, tsig->mac_size);
	sldns_buffer_write_u16(pkt, tsig->original_query_id);
	sldns_buffer_write_u16(pkt, tsig->error);
	sldns_buffer_write_u16(pkt, tsig->other_len);
	if(tsig->other_len == 6)
		sldns_buffer_write_u48(pkt, tsig->other_time);

	LDNS_ARCOUNT_SET(sldns_buffer_begin(pkt),
		LDNS_ARCOUNT(sldns_buffer_begin(pkt))+1);

	lock_rw_unlock(&key_table->lock);
	return 1;
}
