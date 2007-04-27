/*
 * testcode/unitmain.c - unit test main program for unbound.
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
 * Unit test main program. Calls all the other unit tests.
 * Exits with code 1 on a failure. 0 if all unit tests are successfull.
 */

#include "config.h"
#include "util/log.h"
#include "testcode/unitmain.h"

/** number of tests done */
int testcount = 0;

#include "util/alloc.h"
/** test alloc code */
static void
alloc_test() {
	alloc_special_t *t1, *t2;
	struct alloc_cache major, minor1, minor2;
	int i;

	alloc_init(&major, NULL, 0);
	alloc_init(&minor1, &major, 0);
	alloc_init(&minor2, &major, 1);

	t1 = alloc_special_obtain(&minor1);
	alloc_clear(&minor1);

	alloc_special_release(&minor2, t1);
	t2 = alloc_special_obtain(&minor2);
	unit_assert( t1 == t2 ); /* reused */
	alloc_special_release(&minor2, t1);

	for(i=0; i<100; i++) {
		t1 = alloc_special_obtain(&minor1);
		alloc_special_release(&minor2, t1);
	}
	if(0) {
		alloc_stats(&minor1);
		alloc_stats(&minor2);
		alloc_stats(&major);
	}
	/* reuse happened */
	unit_assert(minor1.num_quar + minor2.num_quar + major.num_quar == 11);

	alloc_clear(&minor1);
	alloc_clear(&minor2);
	unit_assert(major.num_quar == 11);
	alloc_clear(&major);
}

#include "util/net_help.h"
/** test net code */
static void 
net_test()
{
	unit_assert( str_is_ip6("::") );
	unit_assert( str_is_ip6("::1") );
	unit_assert( str_is_ip6("2001:7b8:206:1:240:f4ff:fe37:8810") );
	unit_assert( str_is_ip6("fe80::240:f4ff:fe37:8810") );
	unit_assert( !str_is_ip6("0.0.0.0") );
	unit_assert( !str_is_ip6("213.154.224.12") );
	unit_assert( !str_is_ip6("213.154.224.255") );
	unit_assert( !str_is_ip6("255.255.255.0") );
	unit_assert( is_pow2(0) );
	unit_assert( is_pow2(1) );
	unit_assert( is_pow2(2) );
	unit_assert( is_pow2(4) );
	unit_assert( is_pow2(8) );
	unit_assert( is_pow2(16) );
	unit_assert( is_pow2(1024) );
	unit_assert( is_pow2(1024*1024) );
	unit_assert( is_pow2(1024*1024*1024) );
	unit_assert( !is_pow2(3) );
	unit_assert( !is_pow2(5) );
	unit_assert( !is_pow2(6) );
	unit_assert( !is_pow2(7) );
	unit_assert( !is_pow2(9) );
	unit_assert( !is_pow2(10) );
	unit_assert( !is_pow2(11) );
	unit_assert( !is_pow2(17) );
	unit_assert( !is_pow2(23) );
	unit_assert( !is_pow2(257) );
	unit_assert( !is_pow2(259) );
}

/** put dname into buffer */
static ldns_buffer*
dname_to_buf(ldns_buffer* b, const char* str)
{
	ldns_rdf* rdf;
	ldns_status status;
	ldns_buffer_clear(b);
	rdf = ldns_dname_new_frm_str(str);	
	status = ldns_dname2buffer_wire(b, rdf);
	if(status != LDNS_STATUS_OK)
		fatal_exit("%s ldns: %s", __func__, 
			ldns_get_errorstr_by_id(status));
	ldns_rdf_deep_free(rdf);
	ldns_buffer_flip(b);
	return b;
}

#include "util/data/msgreply.h"
#include "util/data/dname.h"
/** test query parse code */
static void
msgreply_test()
{
	ldns_buffer* buff = ldns_buffer_new(65800);
	ldns_buffer_flip(buff);
	unit_assert( query_dname_len(buff) == 0);
	unit_assert( query_dname_len(dname_to_buf(buff, ".")) == 1 );
	unit_assert( query_dname_len(dname_to_buf(buff, "bla.foo.")) == 9 );
	unit_assert( query_dname_len(dname_to_buf(buff, "x.y.z.example.com.")) == 19 );

	ldns_buffer_write_at(buff, 0, "\012abCDeaBCde\003cOm\000", 16);
	query_dname_tolower(ldns_buffer_begin(buff), 16);
	unit_assert( memcmp(ldns_buffer_begin(buff), 
		"\012abcdeabcde\003com\000", 16) == 0);

	ldns_buffer_write_at(buff, 0, "\001+\012abC{e-ZYXe\003NET\000", 18);
	query_dname_tolower(ldns_buffer_begin(buff), 18);
	unit_assert( memcmp(ldns_buffer_begin(buff), 
		"\001+\012abc{e-zyxe\003net\000", 18) == 0);

	ldns_buffer_write_at(buff, 0, "\000", 1);
	query_dname_tolower(ldns_buffer_begin(buff), 1);
	unit_assert( memcmp(ldns_buffer_begin(buff), "\000", 1) == 0);

	ldns_buffer_write_at(buff, 0, "\002NL\000", 4);
	query_dname_tolower(ldns_buffer_begin(buff), 4);
	unit_assert( memcmp(ldns_buffer_begin(buff), "\002nl\000", 4) == 0);

	/* test query_dname_compare */
	unit_assert(query_dname_compare((uint8_t*)"", (uint8_t*)"") == 0);
	unit_assert(query_dname_compare((uint8_t*)"\001a", 
					(uint8_t*)"\001a") == 0);
	unit_assert(query_dname_compare((uint8_t*)"\003abc\001a", 
					(uint8_t*)"\003abc\001a") == 0);
	unit_assert(query_dname_compare((uint8_t*)"\003aBc\001a", 
					(uint8_t*)"\003AbC\001A") == 0);
	unit_assert(query_dname_compare((uint8_t*)"\003abc", 
					(uint8_t*)"\003abc\001a") == -1);
	unit_assert(query_dname_compare((uint8_t*)"\003abc\001a", 
					(uint8_t*)"\003abc") == +1);
	unit_assert(query_dname_compare((uint8_t*)"\003abc\001a", 
					(uint8_t*)"") == +1);
	unit_assert(query_dname_compare((uint8_t*)"", 
					(uint8_t*)"\003abc\001a") == -1);
	unit_assert(query_dname_compare((uint8_t*)"\003abc\001a", 
					(uint8_t*)"\003xxx\001a") == -1);
	unit_assert(query_dname_compare((uint8_t*)"\003axx\001a", 
					(uint8_t*)"\003abc\001a") == 1);
	unit_assert(query_dname_compare((uint8_t*)"\003abc\001a", 
					(uint8_t*)"\003abc\001Z") == -1);
	unit_assert(query_dname_compare((uint8_t*)"\003abc\001Z", 
					(uint8_t*)"\003abc\001a") == 1);

	unit_assert(dname_count_labels((uint8_t*)"") == 1);
	unit_assert(dname_count_labels((uint8_t*)"\003com") == 2);
	unit_assert(dname_count_labels((uint8_t*)"\003org") == 2);
	unit_assert(dname_count_labels((uint8_t*)"\007example\003com") == 3);
	unit_assert(dname_count_labels((uint8_t*)"\003bla\007example\003com") 
		== 4);

	ldns_buffer_free(buff);
}

/**
 * Main unit test program. Setup, teardown and report errors.
 * @param argc: arg count.
 * @param argv: array of commandline arguments.
 */
int 
main(int argc, char* argv[])
{
	log_init(NULL);
	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		printf("\tperforms unit tests.\n");
		return 1;
	}
	printf("Start of %s unit test.\n", PACKAGE_STRING);
	checklock_start();
	net_test();
	alloc_test();
	msgreply_test();
	lruhash_test();
	slabhash_test();
	msgparse_test();
	checklock_stop();
	printf("%d checks ok.\n", testcount);
	return 0;
}
