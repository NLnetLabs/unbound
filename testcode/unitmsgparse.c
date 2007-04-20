/*
 * testcode/unitmsgparse.c - unit test for msg parse routines.
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
#include "util/data/msgparse.h"
#include "util/data/msgreply.h"
#include "util/alloc.h"
#include "util/region-allocator.h"
#include "util/net_help.h"

/** verbose message parse unit test */
static int vbmp = 0;

/** skip whitespace */
static void
skip_whites(const char** p)
{
	while(1) {
		while(isspace(**p))
			(*p)++;
		if(**p == ';') {
			/* comment, skip until newline */
			while(**p && **p != '\n')
				(*p)++;
			if(**p == '\n')
				(*p)++;
		} else return;
	}
}

/** takes a hex string and puts into buffer */
static void hex_to_buf(ldns_buffer* pkt, const char* hex)
{
	const char* p = hex;
	int val;
	ldns_buffer_clear(pkt);
	while(*p) {
		skip_whites(&p);
		if(ldns_buffer_position(pkt) == ldns_buffer_limit(pkt))
			fatal_exit("hex_to_buf: buffer too small");
		if(!isalnum(*p))
			break;
		val = ldns_hexdigit_to_int(*p++) << 4;
		skip_whites(&p);
		log_assert(*p && isalnum(*p));
		val |= ldns_hexdigit_to_int(*p++);
		ldns_buffer_write_u8(pkt, (uint8_t)val);
		skip_whites(&p);
	}
	ldns_buffer_flip(pkt);
	if(vbmp) {
		printf("packet size %u\n", (unsigned)ldns_buffer_limit(pkt));
	}
}

/** match two rr lists */
static int
match_list(ldns_rr_list* q, ldns_rr_list *p)
{
	size_t i;
	if(ldns_rr_list_rr_count(q) != ldns_rr_list_rr_count(p))
		return 0;
	for(i=0; i<ldns_rr_list_rr_count(q); i++)
	{
		if(ldns_rr_compare(ldns_rr_list_rr(q, i),
			ldns_rr_list_rr(p, i)) != 0) {
			verbose(3, "rr %d different", i);
			return 0;
		}
		/* and check the ttl */
		if(ldns_rr_ttl(ldns_rr_list_rr(q, i)) !=
			ldns_rr_ttl(ldns_rr_list_rr(p, i))) {
			verbose(3, "rr %d ttl different", i);
			return 0;
		}

	}
	return 1;
}

/** compare two booleans */
static int
cmp_bool(int x, int y)
{
	if(!x && !y) return 0;
	if(x && y) return 0;
	if(!x) return -1;
	return 1;
}

/** match all of the packet */
static int
match_all(ldns_pkt* q, ldns_pkt* p)
{
	if(ldns_pkt_get_opcode(q) != ldns_pkt_get_opcode(p))
	{ verbose(3, "allmatch: opcode different"); return 0;}
	if(ldns_pkt_get_rcode(q) != ldns_pkt_get_rcode(p))
	{ verbose(3, "allmatch: rcode different"); return 0;}
	if(ldns_pkt_id(q) != ldns_pkt_id(p))
	{ verbose(3, "allmatch: id different"); return 0;}
	if(cmp_bool(ldns_pkt_qr(q), ldns_pkt_qr(p)) != 0)
	{ verbose(3, "allmatch: qr different"); return 0;}
	if(cmp_bool(ldns_pkt_aa(q), ldns_pkt_aa(p)) != 0)
	{ verbose(3, "allmatch: aa different"); return 0;}
	if(cmp_bool(ldns_pkt_tc(q), ldns_pkt_tc(p)) != 0)
	{ verbose(3, "allmatch: tc different"); return 0;}
	if(cmp_bool(ldns_pkt_rd(q), ldns_pkt_rd(p)) != 0)
	{ verbose(3, "allmatch: rd different"); return 0;}
	if(cmp_bool(ldns_pkt_cd(q), ldns_pkt_cd(p)) != 0)
	{ verbose(3, "allmatch: cd different"); return 0;}
	if(cmp_bool(ldns_pkt_ra(q), ldns_pkt_ra(p)) != 0)
	{ verbose(3, "allmatch: ra different"); return 0;}
	if(cmp_bool(ldns_pkt_ad(q), ldns_pkt_ad(p)) != 0)
	{ verbose(3, "allmatch: ad different"); return 0;}
	if(ldns_pkt_qdcount(q) != ldns_pkt_qdcount(p))
	{ verbose(3, "allmatch: qdcount different"); return 0;}
	if(ldns_pkt_ancount(q) != ldns_pkt_ancount(p))
	{ verbose(3, "allmatch: ancount different"); return 0;}
	if(ldns_pkt_nscount(q) != ldns_pkt_nscount(p))
	{ verbose(3, "allmatch: nscount different"); return 0;}
	if(ldns_pkt_arcount(q) != ldns_pkt_arcount(p))
	{ verbose(3, "allmatch: arcount different"); return 0;}
	if(!match_list(ldns_pkt_question(q), ldns_pkt_question(p)))
	{ verbose(3, "allmatch: qd section different"); return 0;}
	if(!match_list(ldns_pkt_answer(q), ldns_pkt_answer(p)))
	{ verbose(3, "allmatch: an section different"); return 0;}
	if(!match_list(ldns_pkt_authority(q), ldns_pkt_authority(p)))
	{ verbose(3, "allmatch: ar section different"); return 0;}
	if(!match_list(ldns_pkt_additional(q), ldns_pkt_additional(p)))
	{ verbose(3, "allmatch: ns section different"); return 0;}
	return 1;
}

/** see if buffers contain the same packet */
static int
test_buffers(ldns_buffer* pkt, ldns_buffer* out)
{
	ldns_pkt* p1=0, *p2=0;
	ldns_status s1, s2;
	/* check binary same */
	if(ldns_buffer_limit(pkt) == ldns_buffer_limit(out) &&
		memcmp(ldns_buffer_begin(pkt), ldns_buffer_begin(out),
			ldns_buffer_limit(pkt)) == 0) {
		if(vbmp) printf("binary the same (length=%u)\n",
				(unsigned)ldns_buffer_limit(pkt));
		return 1;
	}
	/* check if it 'means the same' */
	s1 = ldns_buffer2pkt_wire(&p1, pkt);
	s2 = ldns_buffer2pkt_wire(&p2, out);
	if(vbmp) {
		log_hex("orig in hex", ldns_buffer_begin(pkt),
			ldns_buffer_limit(pkt));
		log_hex("unbound out in hex", ldns_buffer_begin(out),
			ldns_buffer_limit(out));
		printf("\npacket from unbound (%d):\n", 
			(int)ldns_buffer_limit(out));
		ldns_pkt_print(stdout, p2);

		printf("\npacket original (%d):\n", 
			(int)ldns_buffer_limit(pkt));
		ldns_pkt_print(stdout, p1);
		printf("\n");
	}
	if(s1 != s2) {
		/* oops! */
		printf("input ldns parse: %s, output ldns parse: %s.\n",
			ldns_get_errorstr_by_id(s1), 
			ldns_get_errorstr_by_id(s2));
		unit_assert(0);
	}
	/* compare packets */
	unit_assert(match_all(p1, p2));
	ldns_pkt_free(p1);
	ldns_pkt_free(p2);
	return 0;
}

/** check if unbound formerr equals ldns formerr. */
static void
checkformerr(ldns_buffer* pkt)
{
	ldns_pkt* p;
	ldns_status status = ldns_buffer2pkt_wire(&p, pkt);
	if(vbmp) printf("formerr, ldns parse is: %s\n",
			ldns_get_errorstr_by_id(status));
	if(status == LDNS_STATUS_OK) {
		printf("Formerr, but ldns gives packet:\n");
		ldns_pkt_print(stdout, p);
		exit(1);
	}
	unit_assert(status != LDNS_STATUS_OK);
}

/** test a packet */
static void
testpkt(ldns_buffer* pkt, struct alloc_cache* alloc, ldns_buffer* out, 
	const char* hex)
{
	struct query_info qi;
	struct reply_info* rep = 0;
	size_t sz;
	int ret;
	uint16_t id;
	uint16_t flags;
	uint32_t timenow = 0;
	region_type *region = region_create(malloc, free);
	struct iovec iov[1024];
	size_t maxiov = 1024;

	hex_to_buf(pkt, hex);
	memmove(&id, ldns_buffer_begin(pkt), sizeof(id));
	memmove(&flags, ldns_buffer_at(pkt, 2), sizeof(flags));
	flags = ntohs(flags);
	ret = reply_info_parse(pkt, alloc, &qi, &rep);
	if(ret != 0) {
		if(vbmp) printf("parse code %d: %s\n", ret, 
			ldns_lookup_by_id(ldns_rcodes, ret)->name);
		if(ret == LDNS_RCODE_FORMERR)
			checkformerr(pkt);
		unit_assert(ret != LDNS_RCODE_SERVFAIL);
	} else {
		sz = reply_info_iov_regen(&qi, rep, id, flags, iov, maxiov,
			timenow, region);
		unit_assert(sz != 0); /* udp packets should fit in 1024 iov */
		write_iov_buffer(out, iov, sz);
		if(vbmp) printf("iov len outlen %u %u\n", (unsigned)sz, 
			(unsigned)ldns_buffer_limit(out));
		test_buffers(pkt, out);
	} 

	query_info_clear(&qi);
	reply_info_parsedelete(rep, alloc);
	region_destroy(region);
}

/** simple test of parsing. */
static void
simpletest(ldns_buffer* pkt, struct alloc_cache* alloc, ldns_buffer* out)
{
	/* a root query  drill -q - */
	testpkt(pkt, alloc, out, 
		" c5 40 01 00 00 01 00 00 00 00 00 00 00 00 02 00 01 ");

	/* very small packet */
	testpkt(pkt, alloc, out, 
"; 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19\n"
";-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
"74 0c 85 83 00 01 00 00 00 01 00 00 03 62 6c 61 09 6e 6c 6e    ;          1-  20\n"
"65 74 6c 61 62 73 02 6e 6c 00 00 0f 00 01 09 6e 6c 6e 65 74    ;         21-  40\n"
"6c 61 62 73 02 6e 6c 00 00 06 00 01 00 00 46 50 00 40 04 6f    ;         41-  60\n"
"70 65 6e 09 6e 6c 6e 65 74 6c 61 62 73 02 6e 6c 00 0a 68 6f    ;         61-  80\n"
"73 74 6d 61 73 74 65 72 09 6e 6c 6e 65 74 6c 61 62 73 02 6e    ;         81- 100\n"
"6c 00 77 a1 02 58 00 00 70 80 00 00 1c 20 00 09 3a 80 00 00    ;        101- 120\n"
"46 50\n");
	
	/* a root reply  drill -w - */
	testpkt(pkt, alloc, out, 
	" ; 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19\n"
	" ;-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
	" 97 3f 81 80 00 01 00 0d 00 00 00 02 00 00 02 00 01 00 00 02    ;          1-  20\n"
	" 00 01 00 06 6d 38 00 14 01 49 0c 52 4f 4f 54 2d 53 45 52 56    ;         21-  40\n"
	" 45 52 53 03 4e 45 54 00 00 00 02 00 01 00 06 6d 38 00 14 01    ;         41-  60\n"
	" 4a 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00 00    ;         61-  80\n"
	" 00 02 00 01 00 06 6d 38 00 14 01 4b 0c 52 4f 4f 54 2d 53 45    ;         81- 100\n"
	" 52 56 45 52 53 03 4e 45 54 00 00 00 02 00 01 00 06 6d 38 00    ;        101- 120\n"
	" 14 01 4c 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e 45 54    ;        121- 140\n"
	" 00 00 00 02 00 01 00 06 6d 38 00 14 01 4d 0c 52 4f 4f 54 2d    ;        141- 160\n"
	" 53 45 52 56 45 52 53 03 4e 45 54 00 00 00 02 00 01 00 06 6d    ;        161- 180\n"
	" 38 00 14 01 41 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e    ;        181- 200\n"
	" 45 54 00 00 00 02 00 01 00 06 6d 38 00 14 01 42 0c 52 4f 4f    ;        201- 220\n"
	" 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00 00 00 02 00 01 00    ;        221- 240\n"
	" 06 6d 38 00 14 01 43 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53    ;        241- 260\n"
	" 03 4e 45 54 00 00 00 02 00 01 00 06 6d 38 00 14 01 44 0c 52    ;        261- 280\n"
	" 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00 00 00 02 00    ;        281- 300\n"
	" 01 00 06 6d 38 00 14 01 45 0c 52 4f 4f 54 2d 53 45 52 56 45    ;        301- 320\n"
	" 52 53 03 4e 45 54 00 00 00 02 00 01 00 06 6d 38 00 14 01 46    ;        321- 340\n"
	" 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00 00 00    ;        341- 360\n"
	" 02 00 01 00 06 6d 38 00 14 01 47 0c 52 4f 4f 54 2d 53 45 52    ;        361- 380\n"
	" 56 45 52 53 03 4e 45 54 00 00 00 02 00 01 00 06 6d 38 00 14    ;        381- 400\n"
	" 01 48 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00    ;        401- 420\n"
	" 01 41 0c 52 4f 4f 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00    ;        421- 440\n"
	" 00 01 00 01 00 02 64 b9 00 04 c6 29 00 04 01 4a 0c 52 4f 4f    ;        441- 460\n"
	" 54 2d 53 45 52 56 45 52 53 03 4e 45 54 00 00 01 00 01 00 02    ;        461- 480\n"
	" 64 b9 00 04 c0 3a 80 1e  ");
}

/** simple test of parsing, pcat file. */
static void
testfromfile(ldns_buffer* pkt, struct alloc_cache* alloc, ldns_buffer* out,
	const char* fname)
{
	FILE* in = fopen(fname, "r");
	char buf[102400];
	int no=0;
	if(!in) {
		perror("fname");
		return;
	}
	while(fgets(buf, (int)sizeof(buf), in)) {
		if(buf[0] == ';') /* comment */
			continue;
		if(strlen(buf) < 10) /* skip pcat line numbers. */
			continue;
		if(vbmp) {
			printf("test no %d: %s", no, buf);
			fflush(stdout);
		}
		testpkt(pkt, alloc, out, buf);
		no++;
	}
	fclose(in);
}

/** simple test of parsing, drill file. */
static void
testfromdrillfile(ldns_buffer* pkt, struct alloc_cache* alloc, 
	ldns_buffer* out, const char* fname)
{
	FILE* in = fopen(fname, "r");
	char buf[102400];
	char *np = buf;
	if(!in) {
		perror("fname");
		return;
	}
	while(fgets(np, (int)sizeof(buf) - (np-buf), in)) {
		if(np[0] == ';') /* comment */
			continue;
		np = &np[strlen(np)];
	}
	if(vbmp) {
		printf("test %s", buf);
		fflush(stdout);
	}
	testpkt(pkt, alloc, out, buf);
	fclose(in);
}

void msgparse_test()
{
	ldns_buffer* pkt = ldns_buffer_new(65553);
	ldns_buffer* out = ldns_buffer_new(65553);
	struct alloc_cache super_a, alloc;
	/* init */
	alloc_init(&super_a, NULL, 0);
	alloc_init(&alloc, &super_a, 2);

	printf("testmsgparse\n");
	simpletest(pkt, &alloc, out);
	testfromfile(pkt, &alloc, out, "testdata/test_packets.1");
	testfromfile(pkt, &alloc, out, "testdata/test_packets.2");
	testfromfile(pkt, &alloc, out, "testdata/test_packets.3");
	if(0) testfromdrillfile(pkt, &alloc, out, "blabla");

	/* cleanup */
	alloc_clear(&alloc);
	alloc_clear(&super_a);
	ldns_buffer_free(pkt);
	ldns_buffer_free(out);
}
