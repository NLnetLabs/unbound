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
#include "testcode/unitmain.h"
#include "sldns/sbuffer.h"
#include <ctype.h>

#define xstr(s) str(s)
#define str(s) #s
#define SRCDIRSTR xstr(SRCDIR)

/** verbosity for this file */
static int vtest = 0;

/**
 * Content of the TSIG test files.
 *
 * The tsig test files have this syntax. It is made of lines, lines started
 * with # are a comment. empty lines are ignored.
 * file_algorithm <name>
 * 	The name is like md5, sha1, sha256 and if the algorithm is not
 * 	supported at the test run time, the file is skipped, silently.
 * tsig-key:
 * 	the following lines define name:, algorithm: and secret:
 * 	and it adds a tsig-key that can be used.
 * del-key <name>
 *	The tsig key is deleted, from the in-memory key table.
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
	char* algo = get_arg_on_line(line, "file_algorithm");
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

/** Handle one line from the TSIG test file */
static void
handle_line(char* line, struct tsig_key_table* key_table,
	struct sldns_buffer* pkt, FILE* in, const char* fname,
	int* break_file)
{
	char* s = get_keyword(line);
	if(vtest)
		printf("line: %s\n", s);
	if(strncmp(s, "file_algorithm", 14) == 0) {
		handle_file_algorithm(s, break_file);
	} else if(strcmp(s, "tsig-key:") == 0) {
		handle_tsig_key(key_table, in, fname);
	} else if(strncmp(s, "delkey", 6) == 0) {
		handle_del_key(s, key_table);
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
}
