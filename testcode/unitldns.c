/*
 * testcode/unitldns.c - unit test for ldns routines.
 *
 * Copyright (c) 2014, NLnet Labs. All rights reserved.
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
 *
 */
/**
 * \file
 * Calls ldns unit tests. Exits with code 1 on a failure. 
 */

#include "config.h"
#include "util/log.h"
#include "testcode/unitmain.h"
#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "sldns/wire2str.h"
#include "sldns/parseutil.h"

/** verbose this unit test */
static int vbmp = 0;

/** print buffer to hex into string */
static void
buf_to_hex(uint8_t* b, size_t blen, char* s, size_t slen)
{
	const char* h = "0123456789ABCDEF";
	size_t i;
	if(slen < blen*2+2 && vbmp) printf("hexstring buffer too small\n");
	unit_assert(slen >= blen*2+2);
	for(i=0; i<blen; i++) {
		s[i*2] = h[(b[i]&0xf0)>>4];
		s[i*2+1] = h[b[i]&0x0f];
	}
	s[blen*2] = '\n';
	s[blen*2+1] = 0;
}

/** Transform input.
 * @param txt_in: input text format.
 * @param wire1: output wireformat in hex (txt_in converted to wire).
 * @param txt_out: output text format (converted from wire_out).
 * @param wire2: output wireformat in hex, txt_out converted back to wireformat.
 * @param bufs: size of the text buffers.
 */
static void
rr_transform(char* txt_in, char* wire1, char* txt_out, char* wire2, 
	size_t bufs)
{
	uint8_t b[65536];
	size_t len;
	int err;

	len = sizeof(b);
	err = sldns_str2wire_rr_buf(txt_in, b, &len, NULL, 3600,
		NULL, 0, NULL, 0);
	if(err != 0) {
		if(vbmp) printf("sldns_str2wire_rr_buf, pos %d: %s\n",
			LDNS_WIREPARSE_OFFSET(err),
			sldns_get_errorstr_parse(err));
	}
	unit_assert(err == 0);
	buf_to_hex(b, len, wire1, bufs);
	if(vbmp) printf("wire1: %s", wire1);

	err = sldns_wire2str_rr_buf(b, len, txt_out, bufs);
	unit_assert(err < (int)bufs && err > 0);
	if(vbmp) printf("txt: %s", txt_out);

	len = sizeof(b);
	err = sldns_str2wire_rr_buf(txt_out, b, &len, NULL, 3600,
		NULL, 0, NULL, 0);
	if(err != 0) {
		if(vbmp) printf("sldns_str2wire_rr_buf-2, pos %d: %s\n",
			LDNS_WIREPARSE_OFFSET(err),
			sldns_get_errorstr_parse(err));
	}
	unit_assert(err == 0);
	buf_to_hex(b, len, wire2, bufs);
	if(vbmp) printf("wire2: %s", wire2);
}

/** Check if results are correct */
static void
rr_checks(char* wire_chk, char* txt_chk, char* txt_out, char* wire_out,
	char* back)
{
#ifdef __APPLE__
	/* the wiretostr on ipv6 is weird on apple, we cannot check it.
	 * skip AAAA on OSX */
	if(strstr(txt_out, "IN	AAAA"))
		txt_out = txt_chk; /* skip this test, but test wirefmt */
			/* so we know that txt_out back to wire is the same */
#endif

	if(strcmp(txt_chk, txt_out) != 0 && vbmp)
		printf("txt different\n");
	if(strcmp(wire_chk, wire_out) != 0 && vbmp)
		printf("wire1 different\n");
	if(strcmp(wire_chk, back) != 0 && vbmp)
		printf("wire2 different\n");

	unit_assert(strcmp(txt_chk, txt_out) == 0);
	unit_assert(strcmp(wire_chk, wire_out) == 0);
	unit_assert(strcmp(wire_chk, back) == 0);
}

/** read rrs to and from string, and wireformat
 * Skips empty lines and comments.
 * @param input: input file with text format.
 * @param check: check file with hex and then textformat
 */
static void
rr_test_file(const char* input, const char* check)
{
	size_t bufs = 131072;
	FILE* inf, *chf, *of;
	int lineno = 0, chlineno = 0;
	char* txt_in = (char*)malloc(bufs);
	char* txt_out = (char*)malloc(bufs);
	char* txt_chk = (char*)malloc(bufs);
	char* wire_out = (char*)malloc(bufs);
	char* wire_chk = (char*)malloc(bufs);
	char* back = (char*)malloc(bufs);
	if(!txt_in || !txt_out || !txt_chk || !wire_out || !wire_chk || !back)
		fatal_exit("malloc failure");
	inf = fopen(input, "r");
	if(!inf) fatal_exit("cannot open %s: %s", input, strerror(errno));
	chf = fopen(check, "r");
	if(!chf) fatal_exit("cannot open %s: %s", check, strerror(errno));

	of = NULL;
	if(0) {
		/* debug: create check file */
		of = fopen("outputfile", "w");
		if(!of) fatal_exit("cannot write output: %s", strerror(errno));
	}

	while(fgets(txt_in, (int)bufs, inf)) {
		lineno++;
		if(vbmp) printf("\n%s:%d %s", input, lineno, txt_in);
		/* skip empty lines and comments */
		if(txt_in[0] == 0 || txt_in[0] == '\n' || txt_in[0] == ';')
			continue;
		/* read check lines */
		if(!fgets(wire_chk, (int)bufs, chf)) {
			printf("%s too short\n", check);
			unit_assert(0);
		}
		if(!fgets(txt_chk, (int)bufs, chf)) {
			printf("%s too short\n", check);
			unit_assert(0);
		}
		chlineno += 2;
		if(vbmp) printf("%s:%d %s", check, chlineno-1, wire_chk);
		if(vbmp) printf("%s:%d %s", check, chlineno, txt_chk);
		/* generate results */
		rr_transform(txt_in, wire_out, txt_out, back, bufs);
		/* checks */
		if(of) {
			fprintf(of, "%s%s", wire_out, txt_out);
		} else {
			rr_checks(wire_chk, txt_chk, txt_out, wire_out, back);
		}
	}
	
	if(of) fclose(of);
	fclose(inf);
	fclose(chf);
	free(txt_in);
	free(txt_out);
	free(txt_chk);
	free(wire_out);
	free(wire_chk);
	free(back);
}

#define xstr(s) str(s)
#define str(s) #s

#ifndef __QNX__
#define SRCDIRSTR xstr(SRCDIR)
#else /* !__QNX__ */
#define SRCDIRSTR "."
#endif /* __QNX__ */

/** read rrs to and from string, to and from wireformat */
static void
rr_tests(void)
{
	rr_test_file(SRCDIRSTR "/testdata/test_ldnsrr.1",
		SRCDIRSTR "/testdata/test_ldnsrr.c1");
	rr_test_file(SRCDIRSTR "/testdata/test_ldnsrr.2",
		SRCDIRSTR "/testdata/test_ldnsrr.c2");
	rr_test_file(SRCDIRSTR "/testdata/test_ldnsrr.3",
		SRCDIRSTR "/testdata/test_ldnsrr.c3");
	rr_test_file(SRCDIRSTR "/testdata/test_ldnsrr.4",
		SRCDIRSTR "/testdata/test_ldnsrr.c4");
	rr_test_file(SRCDIRSTR "/testdata/test_ldnsrr.5",
		SRCDIRSTR "/testdata/test_ldnsrr.c5");
}

/** test various base64 decoding options */
static void
b64_test(void)
{
	/* "normal" b64 alphabet, with padding */
	char* p1 = "aGVsbG8="; /* "hello" */
	char* p2 = "aGVsbG8+"; /* "hello>" */
	char* p3 = "aGVsbG8/IQ=="; /* "hello?!" */
	char* p4 = "aGVsbG8"; /* "hel" + extra garbage */

	/* base64 url, without padding */
	char* u1 = "aGVsbG8"; /* "hello" */
	char* u2 = "aGVsbG8-"; /* "hello>" */
	char* u3 = "aGVsbG8_IQ"; /* "hello?!" */
	char* u4 = "aaaaa"; /* garbage */

	char target[128];
	size_t tarsize = 128;
	int result;

	memset(target, 0, sizeof(target));
	result = sldns_b64_pton(p1, (uint8_t*)target, tarsize);
	unit_assert(result == (int)strlen("hello") && strcmp(target, "hello") == 0);
	memset(target, 0, sizeof(target));
	result = sldns_b64_pton(p2, (uint8_t*)target, tarsize);
	unit_assert(result == (int)strlen("hello>") && strcmp(target, "hello>") == 0);
	memset(target, 0, sizeof(target));
	result = sldns_b64_pton(p3, (uint8_t*)target, tarsize);
	unit_assert(result == (int)strlen("hello?!") && strcmp(target, "hello?!") == 0);
	memset(target, 0, sizeof(target));
	result = sldns_b64_pton(p4, (uint8_t*)target, tarsize);
	/* when padding is used everything that is not a block of 4 will be
	 * ignored */
	unit_assert(result == (int)strlen("hel") && strcmp(target, "hel") == 0);

	memset(target, 0, sizeof(target));
	result = sldns_b64url_pton(u1, strlen(u1), (uint8_t*)target, tarsize);
	unit_assert(result == (int)strlen("hello") && strcmp(target, "hello") == 0);
	memset(target, 0, sizeof(target));
	result = sldns_b64url_pton(u2, strlen(u2), (uint8_t*)target, tarsize);
	unit_assert(result == (int)strlen("hello>") && strcmp(target, "hello>") == 0);
	memset(target, 0, sizeof(target));
	result = sldns_b64url_pton(u3, strlen(u3), (uint8_t*)target, tarsize);
	unit_assert(result == (int)strlen("hello+/") && strcmp(target, "hello?!") == 0);
	/* one item in block of four is not allowed */
	memset(target, 0, sizeof(target));
	result = sldns_b64url_pton(u4, strlen(u4), (uint8_t*)target, tarsize);
	unit_assert(result == -1);
}

/** test SVCB ech svcparam */
static void
svcb_ech_test(void)
{
	uint8_t rr[LDNS_RR_BUF_SIZE];
	size_t rr_len = sizeof(rr), dname_len = 0;
	int e = sldns_str2wire_rr_buf("x. 300 IN HTTPS 1 . ech=0",
		rr, &rr_len, &dname_len, 300, NULL, 0, NULL, 0);
	unit_assert(e == LDNS_WIREPARSE_ERR_OK);
	unit_assert(rr_len == dname_len + 10 /* type,class,ttl,rdatalen */ + 7 /* rdata */);
	unit_assert(sldns_read_uint16(rr + dname_len + 8 /* rdlen */) == 7);
}

/** convert hex string to wire, returns length */
static size_t
hex_to_wire(const char* hex, uint8_t* buf, size_t buflen)
{
	size_t i, len = strlen(hex)/2;
	unit_assert(strlen(hex)%2 == 0 && len <= buflen);
	for(i=0; i<len; i++)
		buf[i] = (uint8_t)(sldns_hexdigit_to_int(hex[i*2])*16 +
			sldns_hexdigit_to_int(hex[i*2+1]));
	return len;
}

/** a wire2str scan function for one rdata field */
typedef int (*wire2str_scan_func)(uint8_t** d, size_t* dl, char** s,
	size_t* sl);

/** check a scan routine: it prints the text and moves the read position
 * over the field, keeping pointer and length in step, and not further. */
static void
scan_check(const char* name, wire2str_scan_func scan, const char* hex,
	size_t fieldlen, const char* text)
{
	uint8_t buf[256], *d = buf;
	char out[1024], *s = out;
	size_t len = hex_to_wire(hex, buf, sizeof(buf)), dl = len,
		sl = sizeof(out);
	int w;
	if(vbmp) printf("scan_check %s %s\n", name, hex);
	memset(out, 0, sizeof(out));
	w = (*scan)(&d, &dl, &s, &sl);
	if(vbmp) printf("got %d '%s' consumed %d\n", w, out, (int)(d-buf));
	unit_assert(w == (int)strlen(text));
	unit_assert(strcmp(out, text) == 0);
	unit_assert(d == buf + fieldlen);
	unit_assert(dl == len - fieldlen);
	unit_assert(s == out + strlen(text));
	unit_assert(sl == sizeof(out) - strlen(text));
}

/** check the dname scan routine, it can follow compression pointers
 * in pkt, but the read position only moves over the name in the data */
static void
dname_scan_check(const char* hex, size_t start, size_t fieldlen,
	const char* text)
{
	uint8_t buf[256], *d;
	char out[1024], *s = out;
	size_t len = hex_to_wire(hex, buf, sizeof(buf)), dl, sl = sizeof(out);
	int w, comprloop = 0;
	if(vbmp) printf("dname_scan_check %s at %d\n", hex, (int)start);
	unit_assert(start <= len);
	d = buf + start;
	dl = len - start;
	memset(out, 0, sizeof(out));
	w = sldns_wire2str_dname_scan(&d, &dl, &s, &sl, buf, len, &comprloop);
	unit_assert(w == (int)strlen(text));
	unit_assert(strcmp(out, text) == 0);
	unit_assert(d == buf + start + fieldlen);
	unit_assert(dl == len - start - fieldlen);
}

/** check the RR scan routine moves over the RR and not further */
static void
rr_scan_check(const char* hex, size_t fieldlen, const char* text)
{
	uint8_t buf[256], *d = buf;
	char out[1024], *s = out;
	size_t len = hex_to_wire(hex, buf, sizeof(buf)), dl = len,
		sl = sizeof(out);
	int w, comprloop = 0;
	if(vbmp) printf("rr_scan_check %s\n", hex);
	memset(out, 0, sizeof(out));
	w = sldns_wire2str_rr_scan(&d, &dl, &s, &sl, buf, len, &comprloop);
	if(vbmp) printf("got %d '%s' consumed %d\n", w, out, (int)(d-buf));
	unit_assert(w == (int)strlen(text));
	unit_assert(strcmp(out, text) == 0);
	unit_assert(d == buf + fieldlen);
	unit_assert(dl == len - fieldlen);
}

/** test that the wire2str scan routines consume exactly their field */
static void
wire2str_scan_test(void)
{
	/* fixed size fields, followed by trailing octets "AABB" */
	scan_check("int8", sldns_wire2str_int8_scan, "07AABB", 1, "7");
	scan_check("int16", sldns_wire2str_int16_scan, "0102AABB", 2, "258");
	scan_check("int32", sldns_wire2str_int32_scan, "00010000AABB", 4,
		"65536");
	scan_check("period", sldns_wire2str_period_scan, "00000E10AABB", 4,
		"3600");
	scan_check("tsigtime", sldns_wire2str_tsigtime_scan, "00005F5E1000AABB",
		6, "1600000000");
	scan_check("a", sldns_wire2str_a_scan, "C0000201AABB", 4,
		"192.0.2.1");
	scan_check("aaaa", sldns_wire2str_aaaa_scan,
		"20010DB8000000000000000000000001AABB", 16, "2001:db8::1");
	scan_check("class", sldns_wire2str_class_scan, "0001AABB", 2, "IN");
	scan_check("type", sldns_wire2str_type_scan, "0010AABB", 2, "TXT");
	scan_check("ttl", sldns_wire2str_ttl_scan, "00000E10AABB", 4, "3600");
	scan_check("cert_alg", sldns_wire2str_cert_alg_scan, "0001AABB", 2,
		"PKIX");
	scan_check("tsigerror", sldns_wire2str_tsigerror_scan, "0012AABB", 2,
		"BADTIME");
	scan_check("eui48", sldns_wire2str_eui48_scan, "00005E0053FFAABB", 6,
		"00-00-5e-00-53-ff");
	scan_check("eui64", sldns_wire2str_eui64_scan, "00005E0000000001AABB",
		8, "00-00-5e-00-00-00-00-01");
	scan_check("ilnp64", sldns_wire2str_ilnp64_scan, "20010DB800000001AABB",
		8, "2001:0db8:0000:0001");

	/* length prefixed fields, followed by trailing octets "AABB" */
	scan_check("str", sldns_wire2str_str_scan, "03616263AABB", 4,
		"\"abc\"");
	scan_check("str empty", sldns_wire2str_str_scan, "00AABB", 1, "\"\"");
	scan_check("unquoted", sldns_wire2str_unquoted_scan, "03616263AABB", 4,
		"abc");
	scan_check("tag", sldns_wire2str_tag_scan, "056973737565AABB", 6,
		"issue");
	scan_check("nsec3_salt", sldns_wire2str_nsec3_salt_scan, "02ABCDAABB",
		3, "ABCD");
	scan_check("nsec3_salt empty", sldns_wire2str_nsec3_salt_scan,
		"00AABB", 1, "-");
	scan_check("b32_ext", sldns_wire2str_b32_ext_scan, "0501020304AAAABB",
		6, "0410615a");
	scan_check("int16_data", sldns_wire2str_int16_data_scan,
		"0003010203AABB", 5, "3 AQID");
	scan_check("apl", sldns_wire2str_apl_scan, "00011803C00002AABB", 7,
		"1:192.0.2.0/24");
	scan_check("hip", sldns_wire2str_hip_scan, "02020003ABCD010203AABB", 9,
		"2 ABCD AQID");

	/* fields that take the remainder of the rdata */
	scan_check("hex", sldns_wire2str_hex_scan, "ABCDEF", 3, "ABCDEF");
	scan_check("b64", sldns_wire2str_b64_scan, "010203", 3, "AQID");
	scan_check("long_str", sldns_wire2str_long_str_scan, "616263", 3,
		"\"abc\"");
	scan_check("nsec", sldns_wire2str_nsec_scan, "00026001", 4,
		"A NS MX");
	scan_check("unknown", sldns_wire2str_unknown_scan, "ABCD", 2,
		"\\# 2 ABCD");

	/* domain names, followed by trailing octets "AABB" */
	dname_scan_check("00AABB", 0, 1, ".");
	dname_scan_check("076578616D706C6503636F6D00AABB", 0, 13,
		"example.com.");
	/* compression pointer to offset 0, read position moves over the
	 * pointer only */
	dname_scan_check("076578616D706C6503636F6D00C000AABB", 13, 2,
		"example.com.");
	dname_scan_check("076578616D706C6503636F6D0003777777C000AABB", 13, 6,
		"www.example.com.");

	/* resource records, followed by trailing octets "AABB" */
	rr_scan_check("076578616D706C6500" "0001" "0001" "00000E10" "0004"
		"C0000201" "AABB", 23, "example.\t3600\tIN\tA\t192.0.2.1\n");
	rr_scan_check("00" "0029" "1000" "00" "00" "8000" "0000" "AABB", 11,
		"; EDNS: version: 0; flags: do ; udp: 4096\n");
}

void
ldns_test(void)
{
	unit_show_feature("sldns");
	rr_tests();
	b64_test();
	svcb_ech_test();
	wire2str_scan_test();
}
