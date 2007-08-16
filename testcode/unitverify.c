/*
 * testcode/unitverify.c - unit test for signature verification routines.
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
 *
 */
/**
 * \file
 * Calls verification unit tests. Exits with code 1 on a failure. 
 */

#include "config.h"
#include "util/log.h"
#include "testcode/unitmain.h"
#include "validator/val_sigcrypt.h"
#include "validator/validator.h"
#include "testcode/ldns-testpkts.h"
#include "util/data/msgreply.h"
#include "util/data/msgparse.h"
#include "util/region-allocator.h"
#include "util/alloc.h"
#include "util/net_help.h"
#include "util/module.h"
#include "util/config_file.h"

/** verbose signature test */
static int vsig = 0;

/** entry to packet buffer with wireformat */
static void
entry_to_buf(struct entry* e, ldns_buffer* pkt)
{
	unit_assert(e->reply_list);
	if(e->reply_list->reply_from_hex) {
		ldns_buffer_copy(pkt, e->reply_list->reply_from_hex);
	} else {
		ldns_status status;
		size_t answer_size;
		uint8_t* ans = NULL;
		status = ldns_pkt2wire(&ans, e->reply_list->reply, 
			&answer_size);
		if(status != LDNS_STATUS_OK) {
			log_err("could not create reply: %s",
				ldns_get_errorstr_by_id(status));
			fatal_exit("error in test");
		}
		ldns_buffer_clear(pkt);
		ldns_buffer_write(pkt, ans, answer_size);
		ldns_buffer_flip(pkt);
		free(ans);
	}
}

/** entry to reply info conversion */
static void
entry_to_repinfo(struct entry* e, struct alloc_cache* alloc, struct region* 
	region, ldns_buffer* pkt, struct query_info* qi, 
	struct reply_info** rep)
{
	int ret;
	struct edns_data edns;
	entry_to_buf(e, pkt);
	ret = reply_info_parse(pkt, alloc, qi, rep, region, &edns);
	region_free_all(region);
	if(ret != 0) {
		printf("parse code %d: %s\n", ret,
			ldns_lookup_by_id(ldns_rcodes, ret)->name);
		unit_assert(ret != 0);
	}
}

/** extract DNSKEY rrset from answer and convert it */
static struct ub_packed_rrset_key* 
extract_keys(struct entry* e, struct alloc_cache* alloc, struct region*
        region, ldns_buffer* pkt)
{
	struct ub_packed_rrset_key* dnskey = NULL;
	struct query_info qinfo;
	struct reply_info* rep = NULL;
	size_t i;

	entry_to_repinfo(e, alloc, region, pkt, &qinfo, &rep);
	for(i=0; i<rep->an_numrrsets; i++) {
		if(ntohs(rep->rrsets[i]->rk.type) == LDNS_RR_TYPE_DNSKEY) {
			dnskey = rep->rrsets[i];
			rep->rrsets[i] = NULL;
			break;
		}
	}
	unit_assert(dnskey);

	reply_info_parsedelete(rep, alloc);
	query_info_clear(&qinfo);
	return dnskey;
}

/** return true if answer should be bogus */
static int
should_be_bogus(struct ub_packed_rrset_key* rrset)
{
	struct packed_rrset_data* d = (struct packed_rrset_data*)rrset->
		entry.data;
	if(d->rrsig_count == 0)
		return 1;
	return 0;
}

/** verify and test one rrset against the key rrset */
static void
verifytest_rrset(struct module_env* env, struct val_env* ve, 
	struct ub_packed_rrset_key* rrset, struct ub_packed_rrset_key* dnskey)
{
	enum sec_status sec;
	if(vsig) {
		log_nametypeclass(VERB_DETAIL, "verify of rrset",
			rrset->rk.dname, ntohs(rrset->rk.type),
			ntohs(rrset->rk.rrset_class));
	}
	sec = dnskeyset_verify_rrset(env, ve, rrset, dnskey);
	if(vsig) {
		printf("verify outcome is: %s\n", sec_status_to_string(sec));
	}
	if(should_be_bogus(rrset)) {
		unit_assert(sec == sec_status_bogus);
	} else {
		unit_assert(sec == sec_status_secure);
	}
}

/** verify and test an entry - every rr in the message */
static void
verifytest_entry(struct entry* e, struct alloc_cache* alloc, struct region*
        region, ldns_buffer* pkt, struct ub_packed_rrset_key* dnskey,
	struct module_env* env, struct val_env* ve)
{
	struct query_info qinfo;
	struct reply_info* rep = NULL;
	size_t i;

	region_free_all(region);
	if(vsig) {
		printf("verifying pkt:\n");
		ldns_pkt_print(stdout, e->reply_list->reply);
		printf("\n");
	}
	entry_to_repinfo(e, alloc, region, pkt, &qinfo, &rep);

	for(i=0; i<rep->rrset_count; i++) {
		verifytest_rrset(env, ve, rep->rrsets[i], dnskey);
	}

	reply_info_parsedelete(rep, alloc);
	query_info_clear(&qinfo);
}

/** find RRset in reply by type */
static struct ub_packed_rrset_key*
find_rrset_type(struct reply_info* rep, uint16_t type)
{
	size_t i;
	for(i=0; i<rep->rrset_count; i++) {
		if(ntohs(rep->rrsets[i]->rk.type) == type)
			return rep->rrsets[i];
	}
	return NULL;
}

/** DS sig test an entry - get DNSKEY and DS in entry and verify */
static void
dstest_entry(struct entry* e, struct alloc_cache* alloc, struct region*
        region, ldns_buffer* pkt, struct module_env* env)
{
	struct query_info qinfo;
	struct reply_info* rep = NULL;
	struct ub_packed_rrset_key* ds, *dnskey;
	int ret;

	region_free_all(region);
	if(vsig) {
		printf("verifying DS-DNSKEY match:\n");
		ldns_pkt_print(stdout, e->reply_list->reply);
		printf("\n");
	}
	entry_to_repinfo(e, alloc, region, pkt, &qinfo, &rep);
	ds = find_rrset_type(rep, LDNS_RR_TYPE_DS);
	dnskey = find_rrset_type(rep, LDNS_RR_TYPE_DNSKEY);
	/* check test is OK */
	unit_assert(ds && dnskey);

	ret = ds_digest_match_dnskey(env, dnskey, 0, ds, 0);
	if(strncmp((char*)qinfo.qname, "\003yes", 4) == 0) {
		if(vsig) {
			printf("result(yes)= %s\n", ret?"yes":"no");
		}
		unit_assert(ret);
	} else if (strncmp((char*)qinfo.qname, "\002no", 3) == 0) {
		if(vsig) {
			printf("result(no)= %s\n", ret?"yes":"no");
		}
		unit_assert(!ret);
	} else {
		fatal_exit("Bad qname in DS unit test, yes or no");
	}

	reply_info_parsedelete(rep, alloc);
	query_info_clear(&qinfo);
}

/** verify from a file */
static void
verifytest_file(const char* fname, const char* at_date)
{
	/* 
	 * The file contains a list of ldns-testpkts entries.
	 * The first entry must be a query for DNSKEY.
	 * The answer rrset is the keyset that will be used for verification
	 */
	struct ub_packed_rrset_key* dnskey;
	struct region* region = region_create(malloc, free);
	struct alloc_cache alloc;
	ldns_buffer* buf = ldns_buffer_new(65535);
	struct entry* e;
	struct entry* list = read_datafile(fname);
	struct module_env env;
	struct val_env ve;

	if(!list)
		fatal_exit("could not read %s: %s", fname, strerror(errno));
	alloc_init(&alloc, NULL, 1);
	memset(&env, 0, sizeof(env));
	memset(&ve, 0, sizeof(ve));
	env.scratch = region;
	env.scratch_buffer = buf;
	ve.date_override = cfg_convert_timeval(at_date);
	unit_assert(region && buf);
	dnskey = extract_keys(list, &alloc, region, buf);
	if(vsig) log_nametypeclass(VERB_DETAIL, "test dnskey",
			dnskey->rk.dname, ntohs(dnskey->rk.type), 
			ntohs(dnskey->rk.rrset_class));
	/* ready to go! */
	for(e = list->next; e; e = e->next) {
		verifytest_entry(e, &alloc, region, buf, dnskey, &env, &ve);
	}

	delete_entry(list);
	region_destroy(region);
	alloc_clear(&alloc);
	ldns_buffer_free(buf);
}

/** verify DS matches DNSKEY from a file */
static void
dstest_file(const char* fname)
{
	/* 
	 * The file contains a list of ldns-testpkts entries.
	 * The first entry must be a query for DNSKEY.
	 * The answer rrset is the keyset that will be used for verification
	 */
	struct region* region = region_create(malloc, free);
	struct alloc_cache alloc;
	ldns_buffer* buf = ldns_buffer_new(65535);
	struct entry* e;
	struct entry* list = read_datafile(fname);
	struct module_env env;

	if(!list)
		fatal_exit("could not read %s: %s", fname, strerror(errno));
	alloc_init(&alloc, NULL, 1);
	memset(&env, 0, sizeof(env));
	env.scratch = region;
	env.scratch_buffer = buf;
	unit_assert(region && buf);

	/* ready to go! */
	for(e = list; e; e = e->next) {
		dstest_entry(e, &alloc, region, buf, &env);
	}

	delete_entry(list);
	region_destroy(region);
	alloc_clear(&alloc);
	ldns_buffer_free(buf);
}

void 
verify_test()
{
	printf("verify test\n");
	verifytest_file("testdata/test_signatures.1", "20070818005004");
	dstest_file("testdata/test_ds_sig.1");
}
