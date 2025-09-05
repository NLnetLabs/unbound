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
#include "util/regional.h"
#include "sldns/parseutil.h"
#include "sldns/pkthdr.h"
#include "sldns/rrdef.h"
#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "util/data/msgparse.h"
#include "util/data/dname.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#ifdef HAVE_OPENSSL_CORE_NAMES_H
#include <openssl/core_names.h>
#endif

/** Fudge time to allow in signed time for TSIG records. In seconds. */
#define TSIG_FUDGE_TIME 300
/** Maximum number of unsigned TSIG packets. +3 for interoperability, off by
 * one errors. */
#define TSIG_MAX_UNSIGNED 103

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

/** Delete the tsig calc state. */
static void tsig_calc_state_delete(
	struct tsig_calc_state_crypto* calc_state_crypto);

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
	/* Canonicalize the key name. */
	query_dname_tolower(key->name);

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
	if(sldns_str2wire_dname_buf(name, buf, &len) != 0) {
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

struct tsig_key*
tsig_key_table_search_fromstr(struct tsig_key_table* key_table, char* name)
{
	uint8_t buf[LDNS_MAX_DOMAINLEN+1];
	size_t len = sizeof(buf);
	if(sldns_str2wire_dname_buf(name, buf, &len) != 0) {
		log_err("could not parse '%s'", name);
		return NULL;
	}
	return tsig_key_table_search(key_table, buf, len);
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

struct tsig_algorithm*
tsig_algo_find_wire(uint8_t* algo)
{
	size_t i;
	for(i=0; i<sizeof(tsig_algorithm_table)/sizeof(*tsig_algorithm_table);
	    i++) {
		if(query_dname_compare(tsig_algorithm_table[i].wireformat_name,
			algo) == 0)
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
	tsig->algo_name_len = key->algo->wireformat_name_len;
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
	tsig->every_nth = 1;
	return tsig;
}

struct tsig_data*
tsig_create_fromstr(struct tsig_key_table* key_table, char* name)
{
	uint8_t buf[LDNS_MAX_DOMAINLEN+1];
	size_t len = sizeof(buf);
	if(sldns_str2wire_dname_buf(name, buf, &len) != 0) {
		log_err("could not parse '%s'", name);
		return NULL;
	}
	return tsig_create(key_table, buf, len);
}

void
tsig_delete(struct tsig_data* tsig)
{
	if(!tsig) return;
	if(tsig->calc_state) {
		tsig_calc_state_delete(tsig->calc_state);
		tsig->calc_state = NULL;
	}
	free(tsig->key_name);
	free(tsig->algo_name);
	free(tsig->mac);
	free(tsig);
}

size_t tsig_get_mem(struct tsig_data* tsig)
{
	if(!tsig)
		return 0;
	return sizeof(*tsig) + tsig->key_name_len + tsig->algo_name_len
		+ tsig->mac_size;
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
		+ tsig->algo_name_len	/* Algorithm */
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
 * Called must hold locks, on key. This routine performs one buffer
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

/**
 * Calculate TSIG MAC in parts.
 * Called must hold locks, on key. The routine concatenates the argument
 * datas with digest updates.
 * @param key: the key, and algorithm.
 * @param prior: before the next buffer. or NULL.
 * @param pkt: packet with contents.
 * @param var: variables to digest.
 * @param tsig: where to store the result.
 * @return false on failure.
 */
static int
tsig_algo_calc_parts(struct tsig_key* key, struct sldns_buffer* prior,
	struct sldns_buffer* pkt, struct sldns_buffer* var,
	struct tsig_data* tsig)
{
#ifdef HAVE_EVP_MAC_CTX_NEW
	/* For EVP_MAC_CTX_new and OpenSSL since 3.0 functions. */
	EVP_MAC* mac = EVP_MAC_fetch(NULL, "hmac", NULL);
	EVP_MAC_CTX* ctx;
	OSSL_PARAM params[2];
	size_t outl = 0;
	if(!mac) {
		log_crypto_err("Could not EVP_MAC_FETCH(..hmac..)");
		return 0;
	}
	params[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
		(char*)key->algo->digest, 0);
	params[1] = OSSL_PARAM_construct_end();
	ctx = EVP_MAC_CTX_new(mac);
	if(!ctx) {
		log_crypto_err("Could not EVP_MAC_CTX_new");
		EVP_MAC_free(mac);
		return 0;
	}
	if(!EVP_MAC_init(ctx, key->data, key->data_len, params)) {
		log_crypto_err("Could not EVP_MAC_init");
		EVP_MAC_CTX_free(ctx);
		EVP_MAC_free(mac);
		return 0;
	}

	if(prior && !EVP_MAC_update(ctx, sldns_buffer_begin(prior),
		sldns_buffer_position(prior))) {
		log_crypto_err("Could not EVP_MAC_update");
		EVP_MAC_CTX_free(ctx);
		EVP_MAC_free(mac);
		return 0;
	}

	if(!EVP_MAC_update(ctx, sldns_buffer_begin(pkt),
		sldns_buffer_position(pkt))) {
		log_crypto_err("Could not EVP_MAC_update");
		EVP_MAC_CTX_free(ctx);
		EVP_MAC_free(mac);
		return 0;
	}

	if(!EVP_MAC_update(ctx, sldns_buffer_begin(var),
		sldns_buffer_position(var))) {
		log_crypto_err("Could not EVP_MAC_update");
		EVP_MAC_CTX_free(ctx);
		EVP_MAC_free(mac);
		return 0;
	}
	if(!EVP_MAC_final(ctx, tsig->mac, &outl, tsig->mac_size)) {
		log_crypto_err("Could not EVP_MAC_final");
		EVP_MAC_CTX_free(ctx);
		EVP_MAC_free(mac);
		return 0;
	}
	if(outl != tsig->mac_size) {
		log_err("Wrong output size of EVP_MAC_final");
		EVP_MAC_CTX_free(ctx);
		EVP_MAC_free(mac);
		return 0;
	}

	EVP_MAC_CTX_free(ctx);
	EVP_MAC_free(mac);
#elif defined(HAVE_HMAC_CTX_NEW)
	/* For HMAC_CTX_new and OpenSSL since 1.1 functions. */
	const EVP_MD* evp_md;
	HMAC_CTX* ctx;
	unsigned int len;
	evp_md = EVP_get_digestbyname(key->algo->digest);
	if(!evp_md) {
		char buf[256];
		snprintf(buf, sizeof(buf), "EVP_get_digestbyname %s failed",
			key->algo->digest);
		log_crypto_err(buf);
		return 0;
	}
	ctx = HMAC_CTX_new();
	if(!ctx) {
		log_crypto_err("Could not HMAC_CTX_new");
		return 0;
	}
	if(!HMAC_Init_ex(ctx, key->data, key->data_len, evp_md, NULL)) {
		log_crypto_err("Could not HMAC_Init_ex");
		HMAC_CTX_free(ctx);
		return 0;
	}
	if(prior && !HMAC_Update(ctx, sldns_buffer_begin(prior),
		sldns_buffer_position(prior))) {
		log_crypto_err("Could not HMAC_Update");
		HMAC_CTX_free(ctx);
		return 0;
	}
	if(!HMAC_Update(ctx, sldns_buffer_begin(pkt),
		sldns_buffer_position(pkt))) {
		log_crypto_err("Could not HMAC_Update");
		HMAC_CTX_free(ctx);
		return 0;
	}
	if(!HMAC_Update(ctx, sldns_buffer_begin(var),
		sldns_buffer_position(var))) {
		log_crypto_err("Could not HMAC_Update");
		HMAC_CTX_free(ctx);
		return 0;
	}
	len = (unsigned int)tsig->mac_size;
	if(!HMAC_Final(ctx, tsig->mac, &len)) {
		log_crypto_err("Could not HMAC_Final");
		HMAC_CTX_free(ctx);
		return 0;
	}
	HMAC_CTX_free(ctx);
#else
	/* For HMAC_CTX_init and OpenSSL before 1.1 functions. */
	HMAC_CTX ctx;
	const EVP_MD* evp_md;
	unsigned int len;
#ifndef HMAC_INIT_EX_RETURNS_VOID
	int r;
#endif

	memset(&ctx, 0, sizeof(ctx));
	HMAC_CTX_init(&ctx);

	evp_md = EVP_get_digestbyname(key->algo->digest);
	if(!evp_md) {
		char buf[256];
		snprintf(buf, sizeof(buf), "EVP_get_digestbyname %s failed",
			key->algo->digest);
		log_crypto_err(buf);
		return 0;
	}
#ifndef HMAC_INIT_EX_RETURNS_VOID
	r =
#endif
	HMAC_Init_ex(&ctx, key->data, key->data_len, evp_md, NULL);
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Init_ex");
		HMAC_CTX_cleanup(&ctx);
		return 0;
	}
#endif
	if(prior) {
#ifndef HMAC_INIT_EX_RETURNS_VOID
		r =
#endif
		HMAC_Update(&ctx, sldns_buffer_begin(prior),
			sldns_buffer_position(prior));
#ifndef HMAC_INIT_EX_RETURNS_VOID
		if(!r) {
			log_crypto_err("Could not HMAC_Update");
			HMAC_CTX_cleanup(&ctx);
			return 0;
		}
#endif
	}
#ifndef HMAC_INIT_EX_RETURNS_VOID
	r =
#endif
	HMAC_Update(&ctx, sldns_buffer_begin(pkt),
		sldns_buffer_position(pkt));
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Update");
		HMAC_CTX_cleanup(&ctx);
		return 0;
	}
#endif
#ifndef HMAC_INIT_EX_RETURNS_VOID
	r =
#endif
	HMAC_Update(&ctx, sldns_buffer_begin(var),
		sldns_buffer_position(var));
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Update");
		HMAC_CTX_cleanup(&ctx);
		return 0;
	}
#endif
	len = (unsigned int)tsig->mac_size;
#ifndef HMAC_INIT_EX_RETURNS_VOID
	r =
#endif
	HMAC_Final(&ctx, tsig->mac, &len);
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Final");
		HMAC_CTX_cleanup(&ctx);
		return 0;
	}
#endif
	HMAC_CTX_cleanup(&ctx);
#endif
	return 1;
}

/**
 * Calculate TSIG MAC in several calls. This has to be with several calls,
 * since it needs to update for every packet in an AXFR or IXFR, and then
 * later get a TSIG.
 * Caller must hold locks, on key, in the init step.
 */
struct tsig_calc_state_crypto {
#ifdef HAVE_EVP_MAC_CTX_NEW
	/** For EVP_MAC_CTX_new and OpenSSL since 3.0 functions. */
	EVP_MAC* mac;
	EVP_MAC_CTX* ctx;
#elif defined(HAVE_HMAC_CTX_NEW)
	/** For HMAC_CTX_new and OpenSSL since 1.1 functions. */
	HMAC_CTX* ctx;
#else
	/** For HMAC_CTX_init and OpenSSL before 1.1 functions. */
	HMAC_CTX ctx;
#endif
};

/**
 * Init the calc state for the key.
 * Caller must hold locks, on key.
 * @param key: the tsig key, with algorithm and key data.
 * @return malloced state or NULL on failure.
 */
static struct tsig_calc_state_crypto*
tsig_calc_state_init(struct tsig_key* key)
{
#ifdef HAVE_EVP_MAC_CTX_NEW
	OSSL_PARAM params[2];
#elif defined(HAVE_HMAC_CTX_NEW)
	const EVP_MD* evp_md;
#else
	const EVP_MD* evp_md;
#ifndef HMAC_INIT_EX_RETURNS_VOID
	int r;
#endif
#endif
	struct tsig_calc_state_crypto* calc_state_crypto = calloc(1,
		sizeof(*calc_state_crypto));
	if(!calc_state_crypto) {
		log_err("out of memory");
		return NULL;
	}
#ifdef HAVE_EVP_MAC_CTX_NEW
	calc_state_crypto->mac = EVP_MAC_fetch(NULL, "hmac", NULL);
	if(!calc_state_crypto->mac) {
		log_crypto_err("Could not EVP_MAC_FETCH(..hmac..)");
		free(calc_state_crypto);
		return NULL;
	}
	params[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
		(char*)key->algo->digest, 0);
	params[1] = OSSL_PARAM_construct_end();
	calc_state_crypto->ctx = EVP_MAC_CTX_new(calc_state_crypto->mac);
	if(!calc_state_crypto->ctx) {
		log_crypto_err("Could not EVP_MAC_CTX_new");
		EVP_MAC_free(calc_state_crypto->mac);
		free(calc_state_crypto);
		return NULL;
	}
	if(!EVP_MAC_init(calc_state_crypto->ctx, key->data, key->data_len,
		params)) {
		log_crypto_err("Could not EVP_MAC_init");
		EVP_MAC_CTX_free(calc_state_crypto->ctx);
		EVP_MAC_free(calc_state_crypto->mac);
		free(calc_state_crypto);
		return NULL;
	}
#elif defined(HAVE_HMAC_CTX_NEW)
	evp_md = EVP_get_digestbyname(key->algo->digest);
	if(!evp_md) {
		char buf[256];
		snprintf(buf, sizeof(buf), "EVP_get_digestbyname %s failed",
			key->algo->digest);
		log_crypto_err(buf);
		free(calc_state_crypto);
		return NULL;
	}
	calc_state_crypto->ctx = HMAC_CTX_new();
	if(!calc_state_crypto->ctx) {
		log_crypto_err("Could not HMAC_CTX_new");
		free(calc_state_crypto);
		return NULL;
	}
	if(!HMAC_Init_ex(calc_state_crypto->ctx, key->data, key->data_len,
		evp_md, NULL)) {
		log_crypto_err("Could not HMAC_Init_ex");
		HMAC_CTX_free(calc_state_crypto->ctx);
		free(calc_state_crypto);
		return NULL;
	}
#else
	memset(&calc_state_crypto->ctx, 0, sizeof(calc_state_crypto->ctx));
	HMAC_CTX_init(&calc_state_crypto->ctx);

	evp_md = EVP_get_digestbyname(key->algo->digest);
	if(!evp_md) {
		char buf[256];
		snprintf(buf, sizeof(buf), "EVP_get_digestbyname %s failed",
			key->algo->digest);
		log_crypto_err(buf);
		free(calc_state_crypto);
		return NULL;
	}
#ifndef HMAC_INIT_EX_RETURNS_VOID
	r =
#endif
	HMAC_Init_ex(&calc_state_crypto->ctx, key->data, key->data_len,
		evp_md, NULL);
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Init_ex");
		HMAC_CTX_cleanup(&calc_state_crypto->ctx);
		free(calc_state_crypto);
		return NULL;
	}
#endif
#endif
	return calc_state_crypto;
}

/**
 * Update the calc state with more information.
 * @param calc_state_crypto: the tsig calc state crypto.
 * @param buf: the buffer to add from begin to position.
 * @return 0 on failure.
 */
static int
tsig_calc_state_update(struct tsig_calc_state_crypto* calc_state_crypto,
	struct sldns_buffer* buf)
{
#ifdef HAVE_EVP_MAC_CTX_NEW
	if(!EVP_MAC_update(calc_state_crypto->ctx, sldns_buffer_begin(buf),
		sldns_buffer_position(buf))) {
		log_crypto_err("Could not EVP_MAC_update");
		return 0;
	}
#elif defined(HAVE_HMAC_CTX_NEW)
	if(!HMAC_Update(calc_state_crypto->ctx, sldns_buffer_begin(buf),
		sldns_buffer_position(buf))) {
		log_crypto_err("Could not HMAC_Update");
		return 0;
	}
#else
#ifndef HMAC_INIT_EX_RETURNS_VOID
	int r;
	r =
#endif
	HMAC_Update(&calc_state_crypto->ctx, sldns_buffer_begin(buf),
		sldns_buffer_position(buf));
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Update");
		return 0;
	}
#endif
#endif
	return 1;
}

/**
 * Final calculation for the calc state.
 * @param calc_state_crypto: the tsig calc state crypto.
 * @param tsig: the result is stored.
 * @return 0 on failure.
 */
static int
tsig_calc_state_final(struct tsig_calc_state_crypto* calc_state_crypto,
	struct tsig_data* tsig)
{
#ifdef HAVE_EVP_MAC_CTX_NEW
	size_t outl = 0;
	if(!EVP_MAC_final(calc_state_crypto->ctx, tsig->mac, &outl,
		tsig->mac_size)) {
		log_crypto_err("Could not EVP_MAC_final");
		return 0;
	}
	if(outl != tsig->mac_size) {
		log_err("Wrong output size of EVP_MAC_final");
		return 0;
	}
#elif defined(HAVE_HMAC_CTX_NEW)
	unsigned int len;
	len = (unsigned int)tsig->mac_size;
	if(!HMAC_Final(calc_state_crypto->ctx, tsig->mac, &len)) {
		log_crypto_err("Could not HMAC_Final");
		return 0;
	}
#else
	unsigned int len;
#ifndef HMAC_INIT_EX_RETURNS_VOID
	int r;
#endif
	len = (unsigned int)tsig->mac_size;
#ifndef HMAC_INIT_EX_RETURNS_VOID
	r =
#endif
	HMAC_Final(&calc_state_crypto->ctx, tsig->mac, &len);
#ifndef HMAC_INIT_EX_RETURNS_VOID
	if(!r) {
		log_crypto_err("Could not HMAC_Final");
		return 0;
	}
#endif
#endif
	return 1;
}

/**
 * Delete the calc state.
 * @param calc_state_crypto: it is deleted.
 */
static void
tsig_calc_state_delete(struct tsig_calc_state_crypto* calc_state_crypto)
{
	if(!calc_state_crypto)
		return;
#ifdef HAVE_EVP_MAC_CTX_NEW
	EVP_MAC_CTX_free(calc_state_crypto->ctx);
	EVP_MAC_free(calc_state_crypto->mac);
#elif defined(HAVE_HMAC_CTX_NEW)
	HMAC_CTX_free(calc_state_crypto->ctx);
#else
	HMAC_CTX_cleanup(&calc_state_crypto->ctx);
#endif
	free(calc_state_crypto);
}

/** Returns true if buffer has space for vars. */
static int
tsig_vars_available(struct tsig_data* tsig, struct sldns_buffer* pkt)
{
	if(!sldns_buffer_available(pkt, tsig->key_name_len + 2 + 4
		+ tsig->algo_name_len + 6 + 2 + 2
		+ 2 + tsig->other_len))
		return 0;
	return 1;
}

/** Returns true if buffer has space for vars. */
static int
tsig_vars_available_parsed(struct tsig_record* rr, struct sldns_buffer* pkt)
{
	if(!sldns_buffer_available(pkt, rr->key_name_len + 2 + 4
		+ rr->algorithm_name_len + 6 + 2 + 2
		+ 2 + rr->other_size))
		return 0;
	return 1;
}

/** Write tsig variables to buffer, from tsig data. */
static void
tsig_write_vars(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key* key, size_t *aftername_pos)
{
	/* Write uncompressed TSIG owner, it is the key name. */
	sldns_buffer_write(pkt, tsig->key_name, tsig->key_name_len);
	*aftername_pos = sldns_buffer_position(pkt);
	sldns_buffer_write_u16(pkt, tsig->klass);
	sldns_buffer_write_u32(pkt, tsig->ttl);
	sldns_buffer_write(pkt, key->algo->wireformat_name,
		key->algo->wireformat_name_len);
	sldns_buffer_write_u48(pkt, tsig->time_signed);
	sldns_buffer_write_u16(pkt, tsig->fudge);
	sldns_buffer_write_u16(pkt, tsig->error);
	if(tsig->other_len == 6) {
		sldns_buffer_write_u16(pkt, tsig->other_len);
		sldns_buffer_write_u48(pkt, tsig->other_time);
	} else {
		sldns_buffer_write_u16(pkt, 0);
	}
}

/** Write tsig variables to buffer, that have just been parsed. */
static void
tsig_write_vars_parsed(struct tsig_record* rr, struct sldns_buffer* var,
	struct tsig_key* key)
{
	/* Write the key name uncompressed */
	sldns_buffer_write(var, key->name, key->name_len);
	sldns_buffer_write_u16(var, LDNS_RR_CLASS_ANY);
	sldns_buffer_write_u32(var, 0); /* TTL */
	sldns_buffer_write(var, key->algo->wireformat_name,
		key->algo->wireformat_name_len);
	sldns_buffer_write_u48(var, rr->signed_time);
	sldns_buffer_write_u16(var, rr->fudge_time);
	sldns_buffer_write_u16(var, rr->error_code);
	sldns_buffer_write_u16(var, rr->other_size);
	sldns_buffer_write(var, rr->other_data, rr->other_size);
}

/** Append TSIG RR to the packet. */
static void
tsig_append_rr(struct tsig_data* tsig, struct sldns_buffer* pkt,
	size_t aftername_pos, uint8_t* algo_name, size_t algo_name_len,
	uint8_t* mac, size_t mac_size)
{
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
	sldns_buffer_write_u16(pkt, algo_name_len
		+ 6 + 2 + 2 /* time,fudge,maclen */ + mac_size
		+ 2 + 2 + 2 /* id,error,otherlen */ + tsig->other_len);
	sldns_buffer_write(pkt, algo_name, algo_name_len);
	sldns_buffer_write_u48(pkt, tsig->time_signed);
	sldns_buffer_write_u16(pkt, tsig->fudge);
	sldns_buffer_write_u16(pkt, mac_size);
	if(mac_size != 0)
		sldns_buffer_write(pkt, mac, mac_size);
	sldns_buffer_write_u16(pkt, tsig->original_query_id);
	sldns_buffer_write_u16(pkt, tsig->error);
	if(tsig->other_len == 6) {
		sldns_buffer_write_u16(pkt, tsig->other_len);
		sldns_buffer_write_u48(pkt, tsig->other_time);
	} else {
		sldns_buffer_write_u16(pkt, 0);
	}

	LDNS_ARCOUNT_SET(sldns_buffer_begin(pkt),
		LDNS_ARCOUNT(sldns_buffer_begin(pkt))+1);
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
		verbose(VERB_ALGO, "tsig_sign_query: not enough buffer space");
		return 0;
	}
	lock_rw_rdlock(&key_table->lock);
	key = tsig_key_table_search(key_table, tsig->key_name,
		tsig->key_name_len);
	if(!key) {
		/* The tsig key has disappeared from the key table. */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_sign_query: key not in table");
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
	if(!tsig_vars_available(tsig, pkt)) {
		/* Buffer is too small */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_sign_query: not enough buffer space");
		return 0;
	}
	tsig_write_vars(tsig, pkt, key, &aftername_pos);

	/* Calculate the TSIG. */
	if(!tsig_algo_calc(key, pkt, tsig)) {
		/* Failure to calculate digest. */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_sign_query: failed to calculate digest");
		return 0;
	}

	/* Append TSIG record. */
	tsig_append_rr(tsig, pkt, aftername_pos, key->algo->wireformat_name,
		key->algo->wireformat_name_len, tsig->mac, tsig->mac_size);
	lock_rw_unlock(&key_table->lock);
	return 1;
}

int
tsig_verify_query(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key* key, struct tsig_record* rr, uint64_t now)
{
	uint16_t current_query_id;
	uint8_t var_buf[2048];
	struct sldns_buffer var;

	sldns_buffer_init_frm_data(&var, var_buf, sizeof(var_buf));
	tsig->error = 0;
	tsig->other_time = 0;
	tsig->other_len = 0;
	tsig->time_signed = now;
	tsig->original_query_id = rr->original_query_id;
	current_query_id = sldns_buffer_read_u16_at(pkt, 0);

	/* Create the variables to digest in the buffer. */
	/* packet with original query ID and ARCOUNT without TSIG. */
	if(!tsig_vars_available_parsed(rr, &var)) {
		verbose(VERB_ALGO, "tsig_verify_query: variable buffer too small");
		return LDNS_RCODE_SERVFAIL;
	}
	sldns_buffer_write_u16_at(pkt, 0, rr->original_query_id);
	LDNS_ARCOUNT_SET( sldns_buffer_begin(pkt)
	                , LDNS_ARCOUNT(sldns_buffer_begin(pkt)) - 1);
	sldns_buffer_set_position(pkt, rr->tsig_pos);

	tsig_write_vars_parsed(rr, &var, key);

	if(!tsig_algo_calc_parts(key, NULL, pkt, &var, tsig)) {
		/* Failure to calculate digest. */
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_verify_query: failed to calculate digest");
		return LDNS_RCODE_SERVFAIL;
	}
	if(CRYPTO_memcmp(rr->mac_data, tsig->mac, tsig->mac_size) != 0) {
		/* TSIG has wrong digest. */
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_query: TSIG %s has wrong digest",
				keynm);
		}
		tsig->error = LDNS_TSIG_ERROR_BADSIG;
		if(tsig->mac && tsig->mac_size) {
			/* The calculated digest can not be used for a reply.*/
			memset(tsig->mac, 0, tsig->mac_size);
		}
		return LDNS_RCODE_NOTAUTH;
	}
	/* The TSIG digest has verified */
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);

	/* Check the TSIG timestamp. */
	/* Time is checked after the MAC is verified, so that a bad MAC
	 * does not get a signed reply. */
	if( (now > rr->signed_time && now - rr->signed_time > rr->fudge_time) ||
	    (now < rr->signed_time && rr->signed_time - now > rr->fudge_time)) {
		/* TSIG has wrong timestamp. */
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_query: TSIG %s has wrong timestamp, now=%llu packet time=%llu fudge time=%d",
				keynm, (unsigned long long)now,
				(unsigned long long)rr->signed_time,
				(int)rr->fudge_time);
			if(rr->other_size == 6)
				verbose(VERB_ALGO, "tsig_verify_query: other time is reported at %llu",
					(unsigned long long)rr->other_time);
		}
		tsig->error = LDNS_TSIG_ERROR_BADTIME;
		tsig->other_len = 6;
		tsig->other_time = now;
		tsig->fudge = rr->fudge_time;
		return LDNS_RCODE_NOTAUTH;
	}

	return 0;
}

int
tsig_parse(struct sldns_buffer* pkt, struct tsig_record* rr)
{
	size_t algopos;
	uint16_t type, klass, rdlength;
	uint32_t ttl;
	memset(rr, 0, sizeof(*rr));

	/* The buffer position is at the TSIG record. */
	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: arcount too low");
		return LDNS_RCODE_FORMERR;
	}
	if(sldns_buffer_remaining(pkt) < 1) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: packet too short");
		return LDNS_RCODE_FORMERR;
	}
	rr->tsig_pos = sldns_buffer_position(pkt);
	rr->key_name = sldns_buffer_current(pkt);
	rr->key_name_len = pkt_dname_len(pkt);
	if(rr->key_name_len == 0) {
		verbose(VERB_ALGO, "tsig_parse: tsig name malformed");
		return LDNS_RCODE_FORMERR;
	}

	if(sldns_buffer_remaining(pkt) < 2 /* type */ + 2 /* class */ +
		4 /* ttl */ + 2 /* rdlength */) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: packet too short");
		return LDNS_RCODE_FORMERR;
	}
	type = sldns_buffer_read_u16(pkt);
	klass = sldns_buffer_read_u16(pkt);
	ttl = sldns_buffer_read_u32(pkt);
	rdlength = sldns_buffer_read_u16(pkt);
	if(type != LDNS_RR_TYPE_TSIG) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG RR has wrong RR type, not TSIG");
		return LDNS_RCODE_FORMERR;
	}
	if(klass != LDNS_RR_CLASS_ANY) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG RR has wrong RR class, not ANY");
		return LDNS_RCODE_FORMERR;
	}
	if(ttl != 0) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG RR has wrong TTL, not 0");
		return LDNS_RCODE_FORMERR;
	}
	if(sldns_buffer_remaining(pkt) < rdlength) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: packet too short for rdlength");
		return LDNS_RCODE_FORMERR;
	}
	if(rdlength < 1 /* algo name first byte */
		+ 6 + 2 + 2 /* time,fudge,maclen */
		+ 2 + 2 + 2 /* id,error,otherlen */) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG record too short");
		return LDNS_RCODE_FORMERR;
	}

	algopos = sldns_buffer_position(pkt);
	rr->algorithm_name = sldns_buffer_current(pkt);
	rr->algorithm_name_len = query_dname_len(pkt);
	if(rr->algorithm_name_len == 0 ||
		rdlength < sldns_buffer_position(pkt)-algopos) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG algorithm name malformed");
		return LDNS_RCODE_FORMERR;
	}

	if(sldns_buffer_remaining(pkt) < 6 + 2 + 2 /* time,fudge,maclen */ ||
		rdlength < sldns_buffer_position(pkt)-algopos) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG record too short");
		return LDNS_RCODE_FORMERR;
	}
	rr->signed_time = sldns_buffer_read_u48(pkt);
	rr->fudge_time = sldns_buffer_read_u16(pkt);
	rr->mac_size = sldns_buffer_read_u16(pkt);
	if(sldns_buffer_remaining(pkt) < rr->mac_size
		+ 2 + 2 + 2 /* id,error,otherlen */ ||
		rdlength < sldns_buffer_position(pkt)-algopos) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG record too short");
		return LDNS_RCODE_FORMERR;
	}
	if(rr->mac_size > 16384) {
		/* the hash should not be too big, really 512/8=64 bytes */
		verbose(VERB_ALGO, "tsig_parse: TSIG mac_size too large");
		return LDNS_RCODE_FORMERR;
	}
	rr->mac_data = sldns_buffer_current(pkt);
	sldns_buffer_skip(pkt, rr->mac_size);

	rr->original_query_id = sldns_buffer_read_u16(pkt);
	rr->error_code = sldns_buffer_read_u16(pkt);
	rr->other_size = sldns_buffer_read_u16(pkt);
	rr->other_data = sldns_buffer_current(pkt);

	if(sldns_buffer_remaining(pkt) < rr->other_size ||
		rdlength < sldns_buffer_position(pkt)-algopos) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG record too short");
		return LDNS_RCODE_FORMERR;
	}
	if(rr->other_size > 16) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: other_size too large");
		return LDNS_RCODE_FORMERR;
	}
	if(rr->other_size == 6)
		rr->other_time = sldns_buffer_read_u48(pkt);
	else
		sldns_buffer_skip(pkt, rr->other_size);

	if(rdlength != sldns_buffer_position(pkt)-algopos) {
		/* The packet is malformed */
		verbose(VERB_ALGO, "tsig_parse: TSIG record has trailing data");
		return LDNS_RCODE_FORMERR;
	}
	if(sldns_buffer_remaining(pkt) > 0) {
		/* The packet is malformed */
		/* Trailing bytes after the RR, or more RRs after the TSIG. */
		verbose(VERB_ALGO, "tsig_parse: TSIG record has trailing data after it");
		return LDNS_RCODE_FORMERR;
	}

	return 0;
}

int
tsig_lookup_key(struct tsig_key_table* key_table,
	struct sldns_buffer* pkt, struct tsig_record* rr,
	struct tsig_data** tsig_ret, struct regional* region,
	struct tsig_key** key)
{
	struct tsig_data* tsig;

	if(region)
		tsig = regional_alloc_zero(region, sizeof(*tsig));
	else
		tsig = calloc(1, sizeof(*tsig));
	if(!tsig) {
		log_err("tsig_lookup_key: alloc failure");
		return LDNS_RCODE_SERVFAIL;
	}
	tsig->fudge = TSIG_FUDGE_TIME; /* seconds */
	tsig->klass = LDNS_RR_CLASS_ANY;
	tsig->every_nth = 1;
	if(region)
		tsig->key_name = regional_alloc(region, rr->key_name_len);
	else
		tsig->key_name = malloc(rr->key_name_len);
	tsig->key_name_len = rr->key_name_len;
	if(!tsig->key_name) {
		verbose(VERB_ALGO, "tsig_lookup_key: alloc failure");
		if(!region)
			free(tsig);
		return LDNS_RCODE_SERVFAIL;
	}
	dname_pkt_copy(pkt, tsig->key_name, rr->key_name);
	/* Canonicalize the key name. */
	query_dname_tolower(tsig->key_name);

	/* Search for the key. */
	lock_rw_rdlock(&key_table->lock);
	*key = tsig_key_table_search(key_table, tsig->key_name,
		tsig->key_name_len);
	if(!*key) {
		/* The tsig key is not in the key table. */
		lock_rw_unlock(&key_table->lock);
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_query: key %s not in table",
				keynm);
		}

		/* Allocate the algo name for the reply. */
		if(region)
			tsig->algo_name = regional_alloc_init(region,
				rr->algorithm_name, rr->algorithm_name_len);
		else
			tsig->algo_name = memdup(rr->algorithm_name,
				rr->algorithm_name_len);
		if(!tsig->algo_name) {
			verbose(VERB_ALGO, "tsig_lookup_key: alloc failure");
			if(!region) {
				free(tsig->key_name);
				free(tsig);
			}
			return LDNS_RCODE_SERVFAIL;
		}
		tsig->algo_name_len = rr->algorithm_name_len;

		tsig->error = LDNS_TSIG_ERROR_BADKEY;
		*tsig_ret = tsig;
		return LDNS_RCODE_NOTAUTH;
	}

	/* Verify that the algorithm name matches the key. */
	if(query_dname_compare(rr->algorithm_name,
		(*key)->algo->wireformat_name) != 0) {
		lock_rw_unlock(&key_table->lock);
		if(verbosity >= VERB_ALGO) {
			char keynm[256], algonm[256];
			dname_str(tsig->key_name, keynm);
			dname_str(rr->algorithm_name, algonm);
			verbose(VERB_ALGO, "tsig_verify_query: TSIG algorithm different for key %s, it has algo %s but the packet has %s",
				keynm, (*key)->algo->short_name, algonm);
		}

		/* Allocate the algo name for the reply. */
		if(region)
			tsig->algo_name = regional_alloc_init(region,
				rr->algorithm_name, rr->algorithm_name_len);
		else
			tsig->algo_name = memdup(rr->algorithm_name,
				rr->algorithm_name_len);
		if(!tsig->algo_name) {
			verbose(VERB_ALGO, "tsig_lookup_key: alloc failure");
			if(!region) {
				free(tsig->key_name);
				free(tsig);
			}
			return LDNS_RCODE_SERVFAIL;
		}
		tsig->algo_name_len = rr->algorithm_name_len;

		tsig->error = LDNS_TSIG_ERROR_BADKEY;
		*tsig_ret = tsig;
		return LDNS_RCODE_NOTAUTH;
	}
	tsig->algo_name = NULL;
	tsig->algo_name_len = (*key)->algo->wireformat_name_len;

	/* Check mac size. */
	if(rr->mac_size != (*key)->algo->max_digest_size ||
		rr->mac_size > 16384) {
		lock_rw_unlock(&key_table->lock);
		if(rr->mac_size < (*key)->algo->max_digest_size &&
			rr->mac_size >= (*key)->algo->max_digest_size/2) {
			/* MAC truncation is not allowed. */
			verbose(VERB_ALGO, "tsig_verify_query: TSIG with truncated mac not allowed");
			tsig->error = LDNS_TSIG_ERROR_BADTRUNC;
			*tsig_ret = tsig;
			return LDNS_RCODE_NOTAUTH;
		}
		verbose(VERB_ALGO, "tsig_verify_query: TSIG wrong maclen");
		if(!region) {
			free(tsig->key_name);
			free(tsig);
		}
		/* The length is just wrong */
		return LDNS_RCODE_FORMERR;
	}

	tsig->mac_size = (*key)->algo->max_digest_size;
	if(region)
		tsig->mac = regional_alloc_zero(region, tsig->mac_size);
	else
		tsig->mac = calloc(1, tsig->mac_size);
	if(!tsig->mac) {
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_lookup_key: alloc failure");
		if(!region) {
			free(tsig->key_name);
			free(tsig);
		}
		return LDNS_RCODE_SERVFAIL;
	}

	*tsig_ret = tsig;
	return 0;
}

int
tsig_parse_verify_query(struct tsig_key_table* key_table,
	struct sldns_buffer* pkt, struct tsig_data** tsig,
	struct regional* region, uint64_t now)
{
	int ret;
	struct tsig_record rr;
	struct tsig_key* key = NULL;
	*tsig = NULL;

	/* Parse the TSIG RR from the query. */
	ret = tsig_parse(pkt, &rr);
	if(ret != 0)
		return ret;

	/* Lookup key and create tsig data. */
	ret = tsig_lookup_key(key_table, pkt, &rr, tsig, region, &key);
	if(ret != 0)
		return ret;

	/* Verify the TSIG. */
	ret = tsig_verify_query(*tsig, pkt, key, &rr, now);
	lock_rw_unlock(&key_table->lock);
	return ret;
}

int
tsig_find_rr(struct sldns_buffer* pkt)
{
	size_t end_pos, n_rrs;
	if(sldns_buffer_limit(pkt) < LDNS_HEADER_SIZE) {
		verbose(VERB_ALGO, "No TSIG, packet too short");
		return 0;
	}
	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1) {
		verbose(VERB_ALGO, "No TSIG found, ARCOUNT == 0");
		return 0;
	}
	n_rrs = LDNS_ANCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_NSCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_ARCOUNT(sldns_buffer_begin(pkt))
	      - 1;

	sldns_buffer_rewind(pkt);
	sldns_buffer_skip(pkt, LDNS_HEADER_SIZE);

	/* Skip qnames. */
	if(!skip_pkt_query_rrs(pkt, LDNS_QDCOUNT(sldns_buffer_begin(pkt)))) {
		verbose(VERB_ALGO, "No TSIG, query section RRs malformed");
		return 0;
	}
	/* Skip all rrs. */
	if(!skip_pkt_rrs(pkt, n_rrs)) {
		verbose(VERB_ALGO, "No TSIG, packet RRs are malformed");
		return 0;
	}
	end_pos = sldns_buffer_position(pkt);

	/* The tsig owner name, the key name */
	if(sldns_buffer_remaining(pkt) < 1) {
		verbose(VERB_ALGO, "No TSIG, packet too short");
		return 0;
	}
	if(!pkt_dname_len(pkt)) {
		verbose(VERB_ALGO, "No TSIG, dname malformed");
		return 0;
	}
	if(sldns_buffer_remaining(pkt) < 2+2+4+2) {
		verbose(VERB_ALGO, "No TSIG, packet too short");
		return 0;
	}
	if(sldns_buffer_read_u16(pkt) != LDNS_RR_TYPE_TSIG) {
		verbose(VERB_ALGO, "No TSIG, last RR not type TSIG");
		return 0;
	}

	sldns_buffer_set_position(pkt, end_pos);
	return 1;
}

int tsig_in_packet(struct sldns_buffer* pkt)
{
	size_t end_pos, n_rrs;
	if(sldns_buffer_limit(pkt) < LDNS_HEADER_SIZE) {
		return 0;
	}
	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1) {
		return 0;
	}
	n_rrs = LDNS_ANCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_NSCOUNT(sldns_buffer_begin(pkt))
	      + LDNS_ARCOUNT(sldns_buffer_begin(pkt))
	      - 1;

	sldns_buffer_rewind(pkt);
	sldns_buffer_skip(pkt, LDNS_HEADER_SIZE);

	/* Skip qnames. */
	if(!skip_pkt_query_rrs(pkt, LDNS_QDCOUNT(sldns_buffer_begin(pkt)))) {
		return 0;
	}
	/* Skip all rrs. */
	if(!skip_pkt_rrs(pkt, n_rrs)) {
		return 0;
	}
	end_pos = sldns_buffer_position(pkt);

	/* The tsig owner name, the key name */
	if(sldns_buffer_remaining(pkt) < 1) {
		return 0;
	}
	if(!pkt_dname_len(pkt)) {
		return 0;
	}
	if(sldns_buffer_remaining(pkt) < 2+2+4+2) {
		return 0;
	}
	if(sldns_buffer_read_u16(pkt) != LDNS_RR_TYPE_TSIG) {
		return 0;
	}

	sldns_buffer_set_position(pkt, end_pos);
	return 1;
}

int
tsig_sign_reply(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now)
{
	size_t aftername_pos;
	struct tsig_key* key;
	uint8_t var_buf[2048];
	struct sldns_buffer var;
	uint16_t current_query_id;

	if(!tsig) {
		/* For some rcodes, like FORMERR, no tsig data is returned,
		 * and also no TSIG is needed on the reply. */
		verbose(VERB_ALGO, "tsig_sign_reply: no TSIG on error reply");
		return 1;
	}

	if(sldns_buffer_remaining(pkt) < tsig_reserved_space(tsig)) {
		/* Not enough space in buffer for packet and TSIG. */
		verbose(VERB_ALGO, "tsig_sign_reply: not enough buffer space");
		return 0;
	}

	if(LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_SERVFAIL ||
	   LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_FORMERR ||
	   LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_NOTIMPL ||
	   (LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_NOTAUTH &&
	    tsig->error != LDNS_TSIG_ERROR_BADTIME)) {
		uint8_t* algo_name;
		size_t algo_name_len;
		/* No TSIG calculation on error reply. */
		if(tsig->error == 0)
			return 1; /* No TSIG needed for the error. Also,
			copying in possible formerr contents is not desired. */
		if(tsig->algo_name) {
			/* For errors, the tsig->algo_name is allocated. */
			algo_name = tsig->algo_name;
			algo_name_len = tsig->algo_name_len;
		} else {
			/* Robust code in case there is algo name. */
			algo_name = (uint8_t*)"\000";
			algo_name_len = 1;
		}

		/* The TSIG can be written straight away */
		sldns_buffer_write(pkt, tsig->key_name, tsig->key_name_len);
		aftername_pos = sldns_buffer_position(pkt);
		tsig_append_rr(tsig, pkt, aftername_pos, algo_name,
			algo_name_len, NULL, 0);
		return 1;
	}

	lock_rw_rdlock(&key_table->lock);
	key = tsig_key_table_search(key_table, tsig->key_name,
		tsig->key_name_len);
	if(!key) {
		/* The tsig key has disappeared from the key table. */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_sign_reply: key not in table");
		return 0;
	}

	tsig->time_signed = now;
	/* The variables for the TSIG calculation fit in the current buffer,
	 * since the reserved space for the TSIG record is larger. */
	if(!tsig_vars_available(tsig, pkt)) {
		/* Buffer is too small */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_sign_reply: not enough buffer space");
		return 0;
	}

	current_query_id = sldns_buffer_read_u16_at(pkt, 0);
	sldns_buffer_write_u16_at(pkt, 0, tsig->original_query_id);
	tsig_write_vars(tsig, pkt, key, &aftername_pos);

	/* The prior digest. It is prepended to the other variables. */
	sldns_buffer_init_frm_data(&var, var_buf, sizeof(var_buf));
	if(!sldns_buffer_available(&var, 2 /* mac_size */ + tsig->mac_size)) {
		/* Buffer too small. */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_sign_reply: not enough buffer space");
		return 0;
	}
	sldns_buffer_write_u16(&var, tsig->mac_size);
	sldns_buffer_write(&var, tsig->mac, tsig->mac_size);

	/* Calculate the TSIG. */
	if(!tsig_algo_calc_parts(key, NULL, &var, pkt, tsig)) {
		/* Failure to calculate digest. */
		lock_rw_unlock(&key_table->lock);
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_sign_reply: failed to calculate digest");
		return 0;
	}

	/* Append TSIG record. */
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);
	tsig_append_rr(tsig, pkt, aftername_pos, key->algo->wireformat_name,
		key->algo->wireformat_name_len, tsig->mac, tsig->mac_size);
	lock_rw_unlock(&key_table->lock);
	return 1;
}

int
tsig_verify_reply(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key* key, struct tsig_record* rr, uint64_t now)
{
	uint8_t var_buf[2048], prior_buf[1024];
	struct sldns_buffer var, prior;
	uint16_t current_query_id;

	sldns_buffer_init_frm_data(&var, var_buf, sizeof(var_buf));
	sldns_buffer_init_frm_data(&prior, prior_buf, sizeof(prior_buf));

	/* Create the variables to digest in the buffer. */
	/* packet with original query ID and ARCOUNT without TSIG. */
	/* It is prepended with the prior mac. */
	if(!tsig_vars_available_parsed(rr, &var)) {
		verbose(VERB_ALGO, "tsig_verify_reply: variable buffer too small");
		return 0;
	}
	if(sldns_buffer_remaining(&prior) <
		2 /* mac_size */ + tsig->mac_size) {
		verbose(VERB_ALGO, "tsig_verify_reply: prior buffer too small");
		return 0;
	}

	sldns_buffer_write_u16(&prior, tsig->mac_size);
	sldns_buffer_write(&prior, tsig->mac, tsig->mac_size);

	current_query_id = sldns_buffer_read_u16_at(pkt, 0);
	sldns_buffer_write_u16_at(pkt, 0, rr->original_query_id);
	LDNS_ARCOUNT_SET( sldns_buffer_begin(pkt)
			, LDNS_ARCOUNT(sldns_buffer_begin(pkt)) - 1);
	sldns_buffer_set_position(pkt, rr->tsig_pos);

	tsig_write_vars_parsed(rr, &var, key);

	if(!tsig_algo_calc_parts(key, &prior, pkt, &var, tsig)) {
		/* Failure to calculate digest. */
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_verify_reply: failed to calculate digest");
		return 0;
	}
	if(CRYPTO_memcmp(rr->mac_data, tsig->mac, tsig->mac_size) != 0) {
		/* TSIG has wrong digest. */
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_reply: TSIG %s has wrong digest",
				keynm);
		}
		return 0;
	}
	/* The TSIG digest has verified */
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);

	/* Check the TSIG timestamp. */
	/* Time is checked after the MAC is verified */
	if( (now > rr->signed_time && now - rr->signed_time > rr->fudge_time) ||
	    (now < rr->signed_time && rr->signed_time - now > rr->fudge_time)) {
		/* TSIG has wrong timestamp. */
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_reply: TSIG %s has wrong timestamp, now=%llu packet time=%llu fudge time=%d",
				keynm, (unsigned long long)now,
				(unsigned long long)rr->signed_time,
				(int)rr->fudge_time);
			if(rr->other_size == 6)
				verbose(VERB_ALGO, "tsig_verify_reply: other time is reported at %llu",
					(unsigned long long)rr->other_time);
		}
		return 0;
	}

	return 1;
}

int
tsig_parse_verify_reply(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now)
{
	struct tsig_key* key;
	struct tsig_record rr;
	int ret;

	/* Parse the TSIG RR from the query. */
	ret = tsig_parse(pkt, &rr);
	if(ret != 0)
		return 0;

	/* Check it is the same key, same algo, same digest_size */
	if(dname_pkt_compare(pkt, tsig->key_name, rr.key_name) != 0) {
		verbose(VERB_ALGO, "tsig_verify_reply: key name wrong");
		return 0;
	}

	lock_rw_rdlock(&key_table->lock);
	key = tsig_key_table_search(key_table, tsig->key_name,
		tsig->key_name_len);
	if(!key) {
		/* The tsig key has disappeared from the key table. */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_verify_reply: key not in table");
		return 0;
	}

	if(query_dname_compare(key->algo->wireformat_name,
		rr.algorithm_name) != 0) {
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_verify_reply: algorithm wrong");
		return 0;
	}
	if(tsig->mac_size != rr.mac_size) {
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_verify_reply: mac size wrong");
		return 0;
	}

	/* Verify the TSIG. */
	ret = tsig_verify_reply(tsig, pkt, key, &rr, now);
	lock_rw_unlock(&key_table->lock);
	return ret;
}

/**
 * Verify pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, an error code otherwise.
 * For a shared packet with contents.
 */
int
tsig_verify_shared(sldns_buffer* pkt, const uint8_t* name, const uint8_t* alg,
		const uint8_t* secret, size_t secret_len, uint64_t now)
{
	int ret;
	struct tsig_record rr;
	struct tsig_key key;
	struct tsig_data tsig;
	uint8_t bufname[256];
	size_t bufname_len;
	uint8_t macbuf[1024];

	if(!tsig_find_rr(pkt)) {
		verbose(VERB_ALGO, "tsig_verify_shared: No TSIG found");
		return -1;
	}

	/* Parse the TSIG RR from the query. */
	ret = tsig_parse(pkt, &rr);
	if(ret != 0) {
		if(ret == LDNS_RCODE_SERVFAIL ||
			ret == LDNS_RCODE_FORMERR)
			return -1;
		return ret;
	}

	if(rr.key_name_len > sizeof(bufname)) {
		verbose(VERB_ALGO, "tsig_verify_shared: key name too long");
		return -1;
	}
	dname_pkt_copy(pkt, bufname, rr.key_name);
	query_dname_tolower(bufname);
	bufname_len = rr.key_name_len;

	if(query_dname_compare(bufname, name) != 0) {
		verbose(VERB_ALGO, "tsig_verify_shared: wrong key");
		return LDNS_TSIG_ERROR_BADKEY;
	}
	if(query_dname_compare(rr.algorithm_name, alg) != 0) {
		verbose(VERB_ALGO, "tsig_verify_shared: wrong algorithm");
		return LDNS_TSIG_ERROR_BADKEY;
	}

	memset(&tsig, 0, sizeof(tsig));
	tsig.fudge = TSIG_FUDGE_TIME; /* seconds */
	tsig.klass = LDNS_RR_CLASS_ANY;
	tsig.key_name = bufname;
	tsig.key_name_len = bufname_len;
	tsig.algo_name = NULL;
	tsig.algo_name_len = rr.algorithm_name_len;

	memset(&key, 0, sizeof(key));
	key.name = bufname;
	key.name_len = bufname_len;
	key.data = (uint8_t*)secret;
	key.data_len = secret_len;
	key.algo = tsig_algo_find_wire((uint8_t*)alg);
	if(!key.algo) {
		verbose(VERB_ALGO, "tsig_verify_shared: unknown algorithm");
		return LDNS_TSIG_ERROR_BADKEY;
	}
	if(key.algo->max_digest_size != rr.mac_size) {
		verbose(VERB_ALGO, "tsig_verify_shared: wrong mac size");
		return -1;
	}
	if(key.algo->max_digest_size > sizeof(macbuf)) {
		verbose(VERB_ALGO, "tsig_verify_shared: mac size too large");
		return -1;
	}
	tsig.mac_size = key.algo->max_digest_size;
	tsig.mac = macbuf;

	/* Verify the TSIG. */
	ret = tsig_verify_query(&tsig, pkt, &key, &rr, now);

	if(ret != 0) {
		if(ret == LDNS_RCODE_SERVFAIL ||
			ret == LDNS_RCODE_FORMERR)
			return -1;
		return ret;
	}
	return 0;
}

/**
 * Sign pkt with the name (domain name), algorithm and key in Base64.
 * out 0 on success, -1 on failure.
 * For a shared packet with contents.
 */
int
tsig_sign_shared(sldns_buffer* pkt, const uint8_t* name, const uint8_t* alg,
		const uint8_t* secret, size_t secret_len, uint64_t now)
{
	struct tsig_key key;
	struct tsig_data tsig;
	size_t aftername_pos;
	uint8_t macbuf[1024];
	uint8_t bufname[256];

	memset(&key, 0, sizeof(key));
	(void)dname_count_size_labels((uint8_t*)name, &key.name_len);
	if(key.name_len == 0 || key.name_len > sizeof(bufname)) {
		verbose(VERB_ALGO, "tsig_sign_shared: key name too long");
		return -1;
	}
	memcpy(bufname, name, key.name_len);
	query_dname_tolower(bufname);
	key.name = bufname;
	key.data = (uint8_t*)secret;
	key.data_len = secret_len;
	key.algo = tsig_algo_find_wire((uint8_t*)alg);
	if(!key.algo) {
		verbose(VERB_ALGO, "tsig_sign_shared: unknown algorithm");
		return -1;
	}
	if(key.algo->max_digest_size > sizeof(macbuf)) {
		verbose(VERB_ALGO, "tsig_sign_shared: mac size too large");
		return -1;
	}

	memset(&tsig, 0, sizeof(tsig));
	tsig.time_signed = now;
	tsig.fudge = TSIG_FUDGE_TIME; /* seconds */
	tsig.klass = LDNS_RR_CLASS_ANY;
	tsig.key_name = key.name;
	tsig.key_name_len = key.name_len;
	tsig.algo_name = NULL;
	tsig.algo_name_len = key.algo->wireformat_name_len;
	tsig.mac_size = key.algo->max_digest_size;
	tsig.mac = macbuf;

	if(sldns_buffer_remaining(pkt) < tsig_reserved_space(&tsig)) {
		/* Not enough space in buffer for packet and TSIG. */
		verbose(VERB_ALGO, "tsig_sign_shared: not enough buffer space");
		return -1;
	}
	tsig.original_query_id = sldns_buffer_read_u16_at(pkt, 0);

	/* That fits in the current buffer, since the reserved space for
	 * the TSIG record is larger. */
	if(!tsig_vars_available(&tsig, pkt)) {
		/* Buffer is too small */
		verbose(VERB_ALGO, "tsig_sign_shared: not enough buffer space");
		return -1;
	}
	tsig_write_vars(&tsig, pkt, &key, &aftername_pos);

	/* Calculate the TSIG. */
	if(!tsig_algo_calc(&key, pkt, &tsig)) {
		/* Failure to calculate digest. */
		verbose(VERB_ALGO, "tsig_sign_shared: failed to calculate digest");
		return -1;
	}

	/* Append TSIG record. */
	tsig_append_rr(&tsig, pkt, aftername_pos, key.algo->wireformat_name,
		key.algo->wireformat_name_len, tsig.mac, tsig.mac_size);
	return 0;
}

int
tsig_sign_reply_xfr(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now, int last_packet)
{
	size_t aftername_pos;
	uint16_t current_query_id;
	uint8_t timers_var_buf[64];
	struct sldns_buffer timers_var;

	sldns_buffer_init_frm_data(&timers_var, timers_var_buf,
		sizeof(timers_var_buf));

	if(!tsig) {
		/* For some rcodes, like FORMERR, no tsig data is returned,
		 * and also no TSIG is needed on the reply. */
		verbose(VERB_ALGO, "tsig_sign_reply: no TSIG on error reply");
		return 1;
	}
	if(LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_SERVFAIL ||
	   LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_FORMERR ||
	   LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_NOTIMPL ||
	   (LDNS_RCODE_WIRE(sldns_buffer_begin(pkt)) == LDNS_RCODE_NOTAUTH &&
	    tsig->error != LDNS_TSIG_ERROR_BADTIME)) {
		uint8_t* algo_name;
		size_t algo_name_len;
		/* No TSIG calculation on error reply. */
		if(tsig->error == 0)
			return 1; /* No TSIG needed for the error. Also,
			copying in possible formerr contents is not desired. */
		if(tsig->algo_name) {
			/* For errors, the tsig->algo_name is allocated. */
			algo_name = tsig->algo_name;
			algo_name_len = tsig->algo_name_len;
		} else {
			/* Robust code in case there is algo name. */
			algo_name = (uint8_t*)"\000";
			algo_name_len = 1;
		}

		/* The TSIG can be written straight away */
		sldns_buffer_write(pkt, tsig->key_name, tsig->key_name_len);
		aftername_pos = sldns_buffer_position(pkt);
		tsig_append_rr(tsig, pkt, aftername_pos, algo_name,
			algo_name_len, NULL, 0);
		return 1;
	}

	if(!tsig->later_packet) {
		/* First packet is signed as usual */
		int ret;
		ret = tsig_sign_reply(tsig, pkt, key_table, now);
		tsig->later_packet = 1;
		return ret;
	}

	if(tsig->num_updates == 0) {
		/* Init the calc state for the new packet, or for the new
		 * packet sequence. */
		struct tsig_key* key;
		if(tsig->calc_state) {
			tsig_calc_state_delete(tsig->calc_state);
			tsig->calc_state = NULL;
		}
		lock_rw_rdlock(&key_table->lock);
		key = tsig_key_table_search(key_table, tsig->key_name,
			tsig->key_name_len);
		if(!key) {
			/* The tsig key has disappeared from the key table. */
			lock_rw_unlock(&key_table->lock);
			verbose(VERB_ALGO, "tsig_sign_reply_xfr: key not in table");
			return 0;
		}

		tsig->calc_state = tsig_calc_state_init(key);
		lock_rw_unlock(&key_table->lock);
		if(!tsig->calc_state) {
			verbose(VERB_ALGO, "tsig_sign_reply_xfr: out of memory");
			return 0;
		}

		/* Update with the prior mac. */
		if(tsig->mac_size != 0 && tsig->mac) {
			uint8_t prior_buf[1024];
			struct sldns_buffer prior;
			sldns_buffer_init_frm_data(&prior, prior_buf,
				sizeof(prior_buf));
			if(sldns_buffer_remaining(&prior) <
				2 /* mac_size */ + tsig->mac_size) {
				verbose(VERB_ALGO, "tsig_sign_reply_xfr: prior buffer too small");
				return 0;
			}
			sldns_buffer_write_u16(&prior, tsig->mac_size);
			sldns_buffer_write(&prior, tsig->mac, tsig->mac_size);
			if(!tsig_calc_state_update(tsig->calc_state, &prior)) {
				verbose(VERB_ALGO, "tsig_sign_reply_xfr: failed to update tsig crypto");
				return 0;
			}
		}
	}

	if(tsig->every_nth != 1 && tsig->num_updates+1 < tsig->every_nth &&
		!last_packet) {
		/* Update with the packet contents. */
		current_query_id = sldns_buffer_read_u16_at(pkt, 0);
		sldns_buffer_write_u16_at(pkt, 0, tsig->original_query_id);
		if(!tsig_calc_state_update(tsig->calc_state, pkt)) {
			sldns_buffer_write_u16_at(pkt, 0, current_query_id);
			verbose(VERB_ALGO, "tsig_sign_reply_xfr: failed to update tsig crypto");
			return 0;
		}
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		tsig->num_updates++;
		return 1;
	}

	/* Update with the packet contents. */
	current_query_id = sldns_buffer_read_u16_at(pkt, 0);
	sldns_buffer_write_u16_at(pkt, 0, tsig->original_query_id);
	if(!tsig_calc_state_update(tsig->calc_state, pkt)) {
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_sign_reply_xfr: failed to update tsig crypto");
		return 0;
	}
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);

	/* Update with the timers part of the variables. */
	tsig->time_signed = now;
	sldns_buffer_write_u48(&timers_var, tsig->time_signed);
	sldns_buffer_write_u16(&timers_var, tsig->fudge);
	if(!tsig_calc_state_update(tsig->calc_state, &timers_var)) {
		verbose(VERB_ALGO, "tsig_sign_reply_xfr: failed to update tsig crypto");
		return 0;
	}

	/* Finalize the calculation. */
	if(!tsig_calc_state_final(tsig->calc_state, tsig)) {
		verbose(VERB_ALGO, "tsig_sign_reply_xfr: failed to make final tsig crypto");
		return 0;
	}

	/* Append TSIG record. */
	if(sldns_buffer_remaining(pkt) < tsig_reserved_space(tsig)) {
		/* Not enough space in buffer for packet and TSIG. */
		verbose(VERB_ALGO, "tsig_sign_reply: not enough buffer space");
		return 0;
	}
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);
	sldns_buffer_write(pkt, tsig->key_name, tsig->key_name_len);
	aftername_pos = sldns_buffer_position(pkt);
	tsig_append_rr(tsig, pkt, aftername_pos, tsig->algo_name,
		tsig->algo_name_len, tsig->mac, tsig->mac_size);
	tsig->num_updates = 0;
	return 1;
}

int
tsig_verify_reply_xfr(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_record* rr, uint64_t now)
{
	uint8_t timers_var_buf[64];
	struct sldns_buffer timers_var;
	uint16_t current_query_id;

	sldns_buffer_init_frm_data(&timers_var, timers_var_buf,
		sizeof(timers_var_buf));

	/* packet with original query ID and ARCOUNT without TSIG. */
	current_query_id = sldns_buffer_read_u16_at(pkt, 0);
	sldns_buffer_write_u16_at(pkt, 0, rr->original_query_id);
	LDNS_ARCOUNT_SET( sldns_buffer_begin(pkt)
			, LDNS_ARCOUNT(sldns_buffer_begin(pkt)) - 1);
	sldns_buffer_set_position(pkt, rr->tsig_pos);

	/* Update with packet */
	if(!tsig_calc_state_update(tsig->calc_state, pkt)) {
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_verify_reply_xfr: failed to update tsig crypto");
		return 0;
	}

	/* Update with the timers part of the variables. */
	sldns_buffer_write_u48(&timers_var, rr->signed_time);
	sldns_buffer_write_u16(&timers_var, rr->fudge_time);
	if(!tsig_calc_state_update(tsig->calc_state, &timers_var)) {
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_verify_reply_xfr: failed to update tsig crypto");
		return 0;
	}

	/* Finalize the calculation. */
	if(!tsig_calc_state_final(tsig->calc_state, tsig)) {
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		verbose(VERB_ALGO, "tsig_verify_reply_xfr: failed to make final tsig crypto");
		return 0;
	}

	if(CRYPTO_memcmp(rr->mac_data, tsig->mac, tsig->mac_size) != 0) {
		/* TSIG has wrong digest. */
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_reply_xfr: TSIG %s has wrong digest",
				keynm);
		}
		return 0;
	}
	/* The TSIG digest has verified */
	sldns_buffer_write_u16_at(pkt, 0, current_query_id);

	/* Check the TSIG timestamp. */
	/* Time is checked after the MAC is verified */
	if( (now > rr->signed_time && now - rr->signed_time > rr->fudge_time) ||
	    (now < rr->signed_time && rr->signed_time - now > rr->fudge_time)) {
		/* TSIG has wrong timestamp. */
		if(verbosity >= VERB_ALGO) {
			char keynm[256];
			dname_str(tsig->key_name, keynm);
			verbose(VERB_ALGO, "tsig_verify_reply_xfr: TSIG %s has wrong timestamp, now=%llu packet time=%llu fudge time=%d",
				keynm, (unsigned long long)now,
				(unsigned long long)rr->signed_time,
				(int)rr->fudge_time);
			if(rr->other_size == 6)
				verbose(VERB_ALGO, "tsig_verify_reply_xfr: other time is reported at %llu",
					(unsigned long long)rr->other_time);
		}
		return 0;
	}

	return 1;
}

int
tsig_parse_verify_reply_xfr(struct tsig_data* tsig, struct sldns_buffer* pkt,
	struct tsig_key_table* key_table, uint64_t now, int last_packet)
{
	struct tsig_key* key;
	struct tsig_record rr;
	int ret;

	if(!tsig->later_packet) {
		/* First packet verifies like normal. */
		int ret;
		ret = tsig_parse_verify_reply(tsig, pkt, key_table, now);
		tsig->later_packet = 1;
		return ret;
	}

	/* Init if needed. */
	if(tsig->num_updates == 0) {
		/* Init the calc state for the new packet, or for the new
		 * packet sequence. */
		struct tsig_key* key;
		if(tsig->calc_state) {
			tsig_calc_state_delete(tsig->calc_state);
			tsig->calc_state = NULL;
		}
		lock_rw_rdlock(&key_table->lock);
		key = tsig_key_table_search(key_table, tsig->key_name,
			tsig->key_name_len);
		if(!key) {
			/* The tsig key has disappeared from the key table. */
			lock_rw_unlock(&key_table->lock);
			verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: key not in table");
			return 0;
		}

		tsig->calc_state = tsig_calc_state_init(key);
		lock_rw_unlock(&key_table->lock);
		if(!tsig->calc_state) {
			verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: out of memory");
			return 0;
		}

		/* Update with the prior mac. */
		if(tsig->mac_size != 0 && tsig->mac) {
			uint8_t prior_buf[1024];
			struct sldns_buffer prior;
			sldns_buffer_init_frm_data(&prior, prior_buf,
				sizeof(prior_buf));
			if(sldns_buffer_remaining(&prior) <
				2 /* mac_size */ + tsig->mac_size) {
				verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: prior buffer too small");
				return 0;
			}
			sldns_buffer_write_u16(&prior, tsig->mac_size);
			sldns_buffer_write(&prior, tsig->mac, tsig->mac_size);
			if(!tsig_calc_state_update(tsig->calc_state, &prior)) {
				verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: failed to update tsig crypto");
				return 0;
			}
		}
	}

	/* If there is no TSIG, that is okay, for every_nth. Update
	 * the counters. */
	/* The ARCOUNT is 0 or we are at end of packet without TSIG. */
	if(LDNS_ARCOUNT(sldns_buffer_begin(pkt)) < 1 ||
		sldns_buffer_remaining(pkt) < 1) {
		uint16_t current_query_id;
		tsig->num_updates++;
		if(last_packet) {
			verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: last packet missing TSIG.");
			return 0;
		}
		if(tsig->num_updates > TSIG_MAX_UNSIGNED) {
			verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: too many packets without TSIG.");
			return 0;
		}
		/* Update with the packet contents. */
		current_query_id = sldns_buffer_read_u16_at(pkt, 0);
		sldns_buffer_write_u16_at(pkt, 0, tsig->original_query_id);
		if(!tsig_calc_state_update(tsig->calc_state, pkt)) {
			sldns_buffer_write_u16_at(pkt, 0, current_query_id);
			verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: failed to update tsig crypto");
			return 0;
		}
		sldns_buffer_write_u16_at(pkt, 0, current_query_id);
		return 1;
	}

	/* Parse the TSIG RR from the query. */
	ret = tsig_parse(pkt, &rr);
	if(ret != 0)
		return 0;

	/* Check it is the same key, same algo, same digest_size */
	if(dname_pkt_compare(pkt, tsig->key_name, rr.key_name) != 0) {
		verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: key name wrong");
		return 0;
	}

	lock_rw_rdlock(&key_table->lock);
	key = tsig_key_table_search(key_table, tsig->key_name,
		tsig->key_name_len);
	if(!key) {
		/* The tsig key has disappeared from the key table. */
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: key not in table");
		return 0;
	}

	if(query_dname_compare(key->algo->wireformat_name,
		rr.algorithm_name) != 0) {
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: algorithm wrong");
		return 0;
	}
	if(tsig->mac_size != rr.mac_size) {
		lock_rw_unlock(&key_table->lock);
		verbose(VERB_ALGO, "tsig_parse_verify_reply_xfr: mac size wrong");
		return 0;
	}
	lock_rw_unlock(&key_table->lock);
	tsig->num_updates = 0;

	/* Verify the TSIG. */
	ret = tsig_verify_reply_xfr(tsig, pkt, &rr, now);
	return ret;
}
