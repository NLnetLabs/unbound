/*
 * testcode/unittsig.c - unit test for TSIG signatures.
 *
 * Copyright (c) 2025, NLnet Labs. All rights reserved.
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
 * Unit test for tsig code.
 */
#include "config.h"
#include "util/tsig.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "testcode/unitmain.h"
#include "sldns/parseutil.h"
#include "sldns/sbuffer.h"
#include "sldns/wire2str.h"
#include <ctype.h>

#define xstr(s) str(s)
#define str(s) #s
#define SRCDIRSTR xstr(SRCDIR)

/** verbosity for this file, 0 no, 1 print some, 2 print packet dumps */
static int vtest = 0;

/**
 * Content of the TSIG test files.
 *
 * The tsig test files have this syntax. It is made of lines, lines started
 * with # are a comment. empty lines are ignored.
 * file-algorithm <name>
 * 	The name is like md5, sha1, sha256 and if the algorithm is not
 * 	supported at the test run time, the file is skipped, silently.
 *
 * tsig-key:
 * 	the following lines define name:, algorithm: and secret:
 * 	and it adds a tsig-key that can be used.
 * del-key <name>
 *	The tsig key is deleted, from the in-memory key table.
 *
 * packet
 *	A packet in hex dump, on the following lines. Until 'endpacket'.
 *	It can be used to sign or verify.
 * check-packet
 *	A packet in hex dump, on the following lines. Until 'endpacket'.
 *	It is compared to the packet buffer, and the test fails if not equal.
 *
 * tsig-sign-query <key> <time> <expected result>
 *	It TSIG signs with key name, at timestamp in secs, and the
 *	result of the call is compared with the expected result, and
 *	the test fails if not equal. The result is in the packet buffer.
 * tsig-verify-query <key> <time> <rcode> <tsigerror> <tsigothertime>
 *	It tsig verifies the packet, looks up key in the key table.
 *	The verification is at timestamp, in secs. The result is checked,
 *	the key with keyname of result, the rcode function result, and
 *	if tsig data is returned, the tsigerror and tsigothertime are
 *	checked if present. If not equal the test fails.
 *	If no tsig data is returned, keyname '.', and 0 and 0 are the
 *	tsigerr and tsigothertime values that are checked.
 *
 */

/** Clean up first keyword */
static char*
get_keyword(char* line)
{
	char* s = line;
	while(isspace(*s))
		s++;
	if(strlen(s)>0 && s[strlen(s)-1] == '\n')
		s[strlen(s)-1] = 0;
	return s;
}

/** Get argument from line */
static char*
get_arg_on_line(char* line, char* keyword)
{
	char* s = line;
	s += strlen(keyword);
	while(isspace(*s))
		s++;
	return s;
}

/** Get next argument from line */
static char*
get_next_arg_on_line(char** s)
{
	char* arg;
	if(!*s)
		return *s;
	while(**s && **s == ' ')
		(*s)++;
	arg = *s;
	if(!**s)
		return arg; /* No arguments */
	*s = strchr(*s, ' ');
	if(!*s) {
		*s = arg+strlen(arg);
		return arg; /* No further arguments */
	}
	if(!**s)
		return arg;
	*(*s)++ = 0;
	while(**s && **s == ' ')
		(*s)++;
	return arg;
}

/** See if algorithm is supported for tsig test */
static int
tsig_algo_test(char* algo)
{
	if(strcmp(algo, "md5") == 0)
		return 1;
	if(strcmp(algo, "sha1") == 0)
		return 1;
	if(strcmp(algo, "sha224") == 0) {
		/* The EVP_sha256 test is also used for sha224. */
#ifdef HAVE_EVP_SHA256
		return 1;
#else
		return 0;
#endif
	}
	if(strcmp(algo, "sha256") == 0) {
#ifdef HAVE_EVP_SHA256
		return 1;
#else
		return 0;
#endif
	}
	if(strcmp(algo, "sha384") == 0) {
		/* The EVP_sha512 test is also used for sha384. */
#ifdef HAVE_EVP_SHA512
		return 1;
#else
		return 0;
#endif
	}
	if(strcmp(algo, "sha512") == 0) {
#ifdef HAVE_EVP_SHA512
		return 1;
#else
		return 0;
#endif
	}
	if(vtest)
		printf("Unknown tsig test algorithm %s\n", algo);
	return 0;
}

/** Handle the file_algorithm */
static void
handle_file_algorithm(char* line, int* break_file)
{
	char* algo = get_arg_on_line(line, "file-algorithm");
	if(!tsig_algo_test(algo)) {
		if(vtest)
			printf("algorithm not supported\n");
		*break_file = 1;
		return;
	}
	if(vtest)
		printf("algorithm supported\n");
}

/** Removes quotes if any */
static char*
quote_removal(char* line)
{
	if(line[0] == '"') {
		char* s = line+1;
		if(strlen(s)>0 && s[strlen(s)-1] == '"')
			s[strlen(s)-1] = 0;
		return s;
	}
	return line;
}

/** Handle the tsig-key */
static void
handle_tsig_key(struct tsig_key_table* key_table, FILE* in, const char* fname)
{
	char line[1024];
	char* s;
	char* name = NULL, *algorithm = NULL, *secret = NULL;
	struct config_tsig_key k;

	while(fgets(line, sizeof(line), in)) {
		line[sizeof(line)-1]=0;
		s = get_keyword(line);
		if(strncmp(s, "name:", 5) == 0) {
			name = strdup(quote_removal(
				get_arg_on_line(s, "name:")));
			if(!name)
				fatal_exit("out of memory");
		} else if(strncmp(s, "algorithm:", 10) == 0) {
			algorithm = strdup(quote_removal(
				get_arg_on_line(s, "algorithm:")));
			if(!algorithm)
				fatal_exit("out of memory");
		} else if(strncmp(s, "secret:", 7) == 0) {
			secret = strdup(quote_removal(
				get_arg_on_line(s, "secret:")));
			if(!secret)
				fatal_exit("out of memory");
		} else {
			fatal_exit("unknown tsig-key element %s: %s",
				fname, s);
		}

		if(name && algorithm && secret)
			break;
	}

	k.next = NULL;
	k.name = name;
	k.algorithm = algorithm;
	k.secret = secret;
	if(!tsig_key_table_add_key(key_table, &k))
		fatal_exit("could not tsig_key_table_add_key, out of memory");
	if(vtest)
		printf("add key %s %s\n", name, algorithm);

	free(name);
	free(algorithm);
	explicit_bzero(secret, strlen(secret));
	free(secret);
}

/** Handle the del_key */
static void
handle_del_key(char* line, struct tsig_key_table* key_table)
{
	char* name = get_arg_on_line(line, "del_key");
	tsig_key_table_del_key_fromstr(key_table, name);
	if(vtest)
		printf("deleted key %s\n", name);
}

/** skip whitespace */
static void
skip_whites(const char** p)
{
	while(1) {
		while(isspace((unsigned char)**p))
			(*p)++;
		if(**p == ';' || **p == '#') {
			/* comment, skip until newline */
			while(**p && **p != '\n')
				(*p)++;
			if(**p == '\n')
				(*p)++;
		} else return;
	}
}

/** Read hex part into buffer */
static int
read_hex_segment(char* s, struct sldns_buffer* buf)
{
	uint8_t val;
	const char* p = s;
	while(*p) {
		skip_whites(&p);
		if(!*p) break;
		if(sldns_buffer_position(buf) == sldns_buffer_limit(buf)) {
			printf("read hex: buffer too small\n");
			return 0;
		}
		if(sldns_hexdigit_to_int(*p) == -1) {
			printf("read hex: not hex: '%c'\n", *p);
			return 0;
		}
		val = sldns_hexdigit_to_int(*p++) << 4;
		skip_whites(&p);
		if(!*p) {
			printf("read hex: expected another, second, hex digit\n");
			return 0;
		}
		if(sldns_hexdigit_to_int(*p) == -1) {
			printf("read hex: not hex: '%c'\n", *p);
			return 0;
		}
		val |= sldns_hexdigit_to_int(*p++);
		sldns_buffer_write_u8(buf, (uint8_t)val);
		skip_whites(&p);
	}
	return 1;
}

/** Read hex packet until 'endpacket' */
static int
read_packet_hex(char* line, struct sldns_buffer* buf, FILE* in,
	const char* fname)
{
	char l[102400];
	char* s;
	sldns_buffer_clear(buf);
	if(!read_hex_segment(line, buf)) {
		printf("Could not read hex %s: %s\n", fname, line);
		return 0;
	}
	while(fgets(l, sizeof(l), in)) {
		l[sizeof(l)-1]=0;
		s = get_keyword(l);
		if(strcmp(s, "endpacket") == 0)
			break;
		if(!read_hex_segment(s, buf)) {
			printf("Could not read hex %s: %s\n", fname, s);
			return 0;
		}
	}
	sldns_buffer_flip(buf);
	return 1;
}

/** Handle the packet */
static void
handle_packet(char* line, struct sldns_buffer* pkt, FILE* in,
	const char* fname)
{
	char* arg = get_arg_on_line(line, "packet");
	if(!read_packet_hex(arg, pkt, in, fname))
		fatal_exit("Could not read packet");
	if(vtest >= 2) {
		char* str = sldns_wire2str_pkt(sldns_buffer_begin(pkt),
			sldns_buffer_limit(pkt));
		if(str)
			printf("packet: %s\n", str);
		else
			printf("could not wire2str_pkt\n");
		free(str);
	}
}

/** Handle the check-packet */
static void
handle_check_packet(char* line, struct sldns_buffer* pkt, FILE* in,
	const char* fname)
{
	char* arg = get_arg_on_line(line, "check-packet");
	uint8_t data[65536];
	sldns_buffer check;
	sldns_buffer_init_frm_data(&check, data, sizeof(data));
	if(!read_packet_hex(arg, &check, in, fname))
		fatal_exit("Could not read check-packet");
	if(vtest >= 2) {
		char* str = sldns_wire2str_pkt(sldns_buffer_begin(&check),
			sldns_buffer_limit(&check));
		if(str)
			printf("check-packet: %s\n", str);
		else
			printf("could not wire2str_pkt\n");
		free(str);
	}

	/* Compare the packet */
	if(vtest && (sldns_buffer_limit(pkt) != sldns_buffer_limit(&check) ||
		memcmp(sldns_buffer_begin(pkt), sldns_buffer_begin(&check),
			sldns_buffer_limit(pkt)) != 0)) {
		printf("The packet and check-packet are different\n");
		if(sldns_buffer_limit(pkt) != sldns_buffer_limit(&check))
			printf("Lengths are different %d, %d (check)\n",
				(int)sldns_buffer_limit(pkt),
				(int)sldns_buffer_limit(&check));
		else printf("Lengths are the same %d\n",
				(int)sldns_buffer_limit(pkt));
		if(vtest >= 2) {
			char pkthex[64*1024*2+10], checkhex[64*1024*2+10];
			char *pktstr, *checkstr;
			hex_ntop(sldns_buffer_begin(pkt), sldns_buffer_limit(pkt),
				pkthex, sizeof(pkthex));
			hex_ntop(sldns_buffer_begin(&check), sldns_buffer_limit(&check),
				checkhex, sizeof(checkhex));
			printf("      packet: %s\n", pkthex);
			printf("check-packet: %s\n", checkhex);
			pktstr = sldns_wire2str_pkt(sldns_buffer_begin(pkt),
				sldns_buffer_limit(pkt));
			checkstr = sldns_wire2str_pkt(
				sldns_buffer_begin(&check),
				sldns_buffer_limit(&check));
			if(pktstr)
				printf("      packet: %s\n", pktstr);
			else
				printf("      packet: could not wire2str\n");
			if(checkstr)
				printf("check-packet: %s\n", checkstr);
			else
				printf("check-packet: could not wire2str\n");
			free(pktstr);
			free(checkstr);
		}
	}
	unit_assert(sldns_buffer_limit(pkt) == sldns_buffer_limit(&check) &&
		memcmp(sldns_buffer_begin(pkt), sldns_buffer_begin(&check),
		sldns_buffer_limit(pkt)) == 0);
	if(vtest)
		printf("check-packet is equal\n");
}

/** Handle the tsig-sign-query */
static void
handle_tsig_sign_query(char* line, struct tsig_key_table* key_table,
	struct sldns_buffer* pkt)
{
	char* arg = get_arg_on_line(line, "tsig-sign-query");
	char* keyname, *s, *timestr, *expectedstr;
	int expected_result, ret;
	uint64_t timepoint;
	struct tsig_data* tsig;
	size_t pos;

	s = arg;
	keyname = get_next_arg_on_line(&s);
	timestr = get_next_arg_on_line(&s);
	expectedstr = get_next_arg_on_line(&s);

	timepoint = (uint64_t)atoll(timestr);
	if(timepoint == 0 && strcmp(timestr, "0") != 0)
		fatal_exit("expected time argument for %s", timestr);
	expected_result = atoi(expectedstr);
	if(expected_result == 0 && strcmp(expectedstr, "0") != 0)
		fatal_exit("expected int argument for %s", expectedstr);

	if(vtest)
		printf("tsig-sign-query with %s %d %d\n", keyname,
			(int)timepoint, expected_result);

	tsig = tsig_create_fromstr(key_table, keyname);
	if(!tsig)
		fatal_exit("alloc fail or key not found %s", keyname);

	/* Put position at the end of the packet to sign it. */
	pos = sldns_buffer_limit(pkt);
	sldns_buffer_clear(pkt);
	sldns_buffer_set_position(pkt, pos);

	ret = tsig_sign_query(tsig, pkt, key_table, timepoint);
	sldns_buffer_flip(pkt);

	if(vtest) {
		if(ret == expected_result)
			printf("function ok, %s\n", (ret?"success":"fail"));
		else
			printf("function returned %d, expected result %d\n",
				ret, expected_result);
	}
	unit_assert(ret == expected_result);

	tsig_delete(tsig);
}

/** Convert RCODE string to number. */
static int
str2wire_rcode(const char* str)
{
	sldns_lookup_table *lt = sldns_lookup_by_name(sldns_rcodes, str);
	if(lt) {
		return (int)lt->id;
	} else if(strncmp(str, "RCODE", 5) == 0) {
		return atoi(str+5);
	}
	/* Try as-is, a number. */
	return atoi(str);
}

/** Convert TSIG error code string to number. */
static int
str2wire_tsigerror(const char* str)
{
	sldns_lookup_table *lt = sldns_lookup_by_name(sldns_tsig_errors, str);
	if(lt) {
		return (int)lt->id;
	}
	/* Try as-is, a number. */
	return atoi(str);
}

/** Print TSIG error code to string */
static void
wire2str_tsigerror_buf(int tsigerr, char* buf, size_t len)
{
	sldns_lookup_table *lt;
	lt = sldns_lookup_by_id(sldns_tsig_errors, tsigerr);
	if(lt && lt->name)
		snprintf(buf, len, "%s", lt->name);
	else	snprintf(buf, len, "%d", tsigerr);
}

/** Handle the tsig-verify-query */
static void
handle_tsig_verify_query(char* line, struct tsig_key_table* key_table,
	struct sldns_buffer* pkt)
{
	char* arg = get_arg_on_line(line, "tsig-verify-query");
	char* keyname, *s, *timestr, *expected_rcode_str,
		*expected_tsigerr_str, *expected_other_str;
	int expected_rcode, expected_tsigerr, ret;
	uint64_t timepoint, expected_other;
	struct tsig_data* tsig;
	char keyname_dname[256];

	s = arg;
	keyname = get_next_arg_on_line(&s);
	timestr = get_next_arg_on_line(&s);
	expected_rcode_str = get_next_arg_on_line(&s);
	expected_tsigerr_str = get_next_arg_on_line(&s);
	expected_other_str = get_next_arg_on_line(&s);

	timepoint = (uint64_t)atoll(timestr);
	if(timepoint == 0 && strcmp(timestr, "0") != 0)
		fatal_exit("expected time argument for %s", timestr);
	expected_rcode = str2wire_rcode(expected_rcode_str);
	if(expected_rcode == 0 && strcmp(expected_rcode_str, "0") != 0 &&
		strcmp(expected_rcode_str, "NOERROR") != 0 &&
		strcmp(expected_rcode_str, "RCODE0") != 0)
		fatal_exit("expected rcode argument for %s", expected_rcode_str);
	expected_tsigerr = str2wire_tsigerror(expected_tsigerr_str);
	if(expected_tsigerr == 0 && strcmp(expected_tsigerr_str, "0") != 0 &&
		strcmp(expected_tsigerr_str, "NOERROR") != 0)
		fatal_exit("expected tsigerrorcode argument for %s",
			expected_tsigerr_str);
	expected_other = (uint64_t)atoll(expected_other_str);
	if(expected_other == 0 && strcmp(expected_other_str, "0") != 0)
		fatal_exit("expected int argument for %s", expected_other_str);
	if(strlen(keyname) > 0 && keyname[strlen(keyname)-1] == '.')
		snprintf(keyname_dname, sizeof(keyname_dname), "%s", keyname);
	else	snprintf(keyname_dname, sizeof(keyname_dname), "%s.", keyname);

	if(vtest) {
		char bufrc[16], bufte[16];
		sldns_wire2str_rcode_buf(expected_rcode, bufrc, sizeof(bufrc));
		wire2str_tsigerror_buf(expected_tsigerr, bufte, sizeof(bufte));
		printf("tsig-verify-query with %s %d %s %s %llu\n", keyname,
			(int)timepoint, bufrc, bufte,
			(unsigned long long)expected_other);
	}

	/* Put position before TSIG */
	if(!tsig_find_rr(pkt)) {
		if(vtest)
			printf("tsig-verify-query found no TSIG RR\n");
		unit_assert(0);
		return;
	}
	ret = tsig_parse_verify_query(key_table, pkt, &tsig, NULL, timepoint);

	if(vtest) {
		char bufrc[16], bufte[16], retrc[16], rette[16];
		sldns_wire2str_rcode_buf(expected_rcode, bufrc, sizeof(bufrc));
		wire2str_tsigerror_buf(expected_tsigerr, bufte, sizeof(bufte));
		sldns_wire2str_rcode_buf(ret, retrc, sizeof(retrc));
		if(tsig)
			wire2str_tsigerror_buf(tsig->error, rette, sizeof(rette));
		else	snprintf(rette, sizeof(rette), "none");
		if(ret == expected_rcode)
			printf("function ok, rcode %s\n", retrc);
		else
			printf("function returned %s, expected result %s\n",
				retrc, bufrc);
		if(tsig) {
			char keynm[256];
			if(tsig->error == expected_tsigerr)
				printf("tsig error ok, it is %s\n", bufte);
			else	printf("tsig error %s, expected %s\n", rette,
					bufte);
			if(tsig->other_len == 6) {
				if(tsig->other_time == expected_other)
					printf("othererrortime ok, it is %llu\n",
						(unsigned long long)expected_other);
				else	printf("othererrortime %llu, expected %llu\n",
						(unsigned long long)tsig->other_time,
						(unsigned long long)expected_other);
			} else {
				if(0 == expected_other)
					printf("othererrortime ok, none\n");
				else	printf("othererrortime none, expected %llu\n",
						(unsigned long long)expected_other);
			}
			sldns_wire2str_dname_buf(tsig->key_name,
				tsig->key_name_len, keynm, sizeof(keynm));
			if(strcmp(keynm, keyname_dname) != 0)
				printf("tsig key is %s, expected %s\n",
					keynm, keyname_dname);
		} else {
			if(expected_tsigerr != 0 || expected_other != 0 ||
				strcmp(keyname_dname, ".") != 0) {
				printf("no tsig data returned, but expected it\n");
			}
		}
	}
	unit_assert(ret == expected_rcode);
	if(tsig) {
		char keynm[256];
		unit_assert(tsig->error == expected_tsigerr);
		if(tsig->other_len == 6) {
			unit_assert(tsig->other_time == (uint64_t)expected_other);
		} else {
			unit_assert(0 == expected_other);
		}
		sldns_wire2str_dname_buf(tsig->key_name, tsig->key_name_len,
			keynm, sizeof(keynm));
		unit_assert(strcmp(keynm, keyname_dname) == 0);
	} else {
		unit_assert(0 == expected_tsigerr);
		unit_assert(0 == expected_other);
		unit_assert(strcmp(keyname_dname, ".") == 0);
	}

	tsig_delete(tsig);
}

/** Handle one line from the TSIG test file */
static void
handle_line(char* line, struct tsig_key_table* key_table,
	struct sldns_buffer* pkt, FILE* in, const char* fname,
	int* break_file)
{
	char* s = get_keyword(line);
	if(vtest)
		printf("line: %s\n", s);
	if(strncmp(s, "file-algorithm", 14) == 0) {
		handle_file_algorithm(s, break_file);
	} else if(strcmp(s, "tsig-key:") == 0) {
		handle_tsig_key(key_table, in, fname);
	} else if(strncmp(s, "delkey", 6) == 0) {
		handle_del_key(s, key_table);
	} else if(strncmp(s, "packet", 6) == 0) {
		handle_packet(s, pkt, in, fname);
	} else if(strncmp(s, "check-packet", 12) == 0) {
		handle_check_packet(s, pkt, in, fname);
	} else if(strncmp(s, "tsig-sign-query", 15) == 0) {
		handle_tsig_sign_query(s, key_table, pkt);
	} else if(strncmp(s, "tsig-verify-query", 17) == 0) {
		handle_tsig_verify_query(s, key_table, pkt);
	} else if(strncmp(s, "#", 1) == 0) {
		/* skip comment */
	} else if(strcmp(s, "") == 0) {
		/* skip empty lines */
	} else {
		fatal_exit("Unknown tsig line %s: %s", fname, s);
	}
	(void)pkt;
}

/** test tsig */
static void
tsig_test_one(const char* fname)
{
	struct tsig_key_table* key_table;
	sldns_buffer* pkt;
	FILE* in;
	char line[1024];
	int break_file = 0;

	unit_show_func("tsig", fname);
	key_table = tsig_key_table_create();
	if(!key_table)
		fatal_exit("out of memory");
	pkt = sldns_buffer_new(65536);
	if(!pkt)
		fatal_exit("out of memory");
	sldns_buffer_flip(pkt); /* start with empty buffer */
	in = fopen(fname, "r");
	if(!in)
		fatal_exit("could not open %s: %s", fname, strerror(errno));

	while(fgets(line, sizeof(line), in)) {
		line[sizeof(line)-1]=0;
		handle_line(line, key_table, pkt, in, fname, &break_file);
		if(break_file)
			break;
	}
	if(ferror(in))
		fatal_exit("error read %s: %s", fname, strerror(errno));

	tsig_key_table_delete(key_table);
	sldns_buffer_free(pkt);
	fclose(in);
}

/** test tsig code */
void
tsig_test(void)
{
	unit_show_feature("tsig");
	tsig_test_one(SRCDIRSTR "/testdata/tsig_test.1");
	tsig_test_one(SRCDIRSTR "/testdata/tsig_test.2");
	tsig_test_one(SRCDIRSTR "/testdata/tsig_test.3");
	tsig_test_one(SRCDIRSTR "/testdata/tsig_test.4");
	tsig_test_one(SRCDIRSTR "/testdata/tsig_test.5");
	tsig_test_one(SRCDIRSTR "/testdata/tsig_test.6");
}
