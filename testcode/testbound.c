/*
 * testcode/testbound.c - test program for unbound.
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
 * Exits with code 1 on a failure. 0 if all unit tests are successfull.
 */

#include "config.h"
#include "testcode/ldns-testpkts.h"
#include "testcode/replay.h"
#include "testcode/fake_event.h"

/** 
 * include the main program from the unbound daemon.
 * rename main to daemon_main to call it
 */
#define main daemon_main
#include "daemon/unbound.c"
#undef main

/** give commandline usage for testbound. */
static void
testbound_usage()
{
	printf("usage: testbound [options]\n");
	printf("\ttest the unbound daemon.\n");
	printf("-h      this help\n");
	printf("-p file	playback text file\n");
	printf("-o str  unbound commandline options separated by spaces.\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE file in source package.\n");
	printf("Report bugs to %s.\n", PACKAGE_BUGREPORT);
}

/** Max number of arguments to pass to unbound. */
#define MAXARG 100

/** 
 * Add options from string to passed argc. splits on whitespace.
 * @param optarg: the option argument, "-v -p 12345" or so.
 * @param pass_argc: ptr to the argc for unbound. Modified.
 * @param pass_argv: the argv to pass to unbound. Modified.
 */
static void
add_opts(char* optarg, int* pass_argc, char* pass_argv[])
{
	char *p = optarg, *np;
	size_t len;
	while(p && isspace(*p)) 
		p++;
	while(p && *p) {
		/* find location of next string and length of this one */
		if((np = strchr(p, ' ')))
			len = (size_t)(np-p);
		else	len = strlen(p);
		/* allocate and copy option */
		if(*pass_argc >= MAXARG-1)
			fatal_exit("too many arguments: '%s'", p);
		pass_argv[*pass_argc] = (char*)malloc(len+1);
		if(!pass_argv[*pass_argc])
			fatal_exit("add_opts: out of memory");
		memcpy(pass_argv[*pass_argc], p, len);
		pass_argv[*pass_argc][len] = 0;
		(*pass_argc)++;
		/* go to next option */
	        p = np;
		while(p && isspace(*p)) 
			p++;
	}
}

/** pretty print commandline for unbound in this test. */
static void
echo_cmdline(int argc, char* argv[])
{
	int i;
	printf("testbound is starting:");
	for(i=0; i<argc; i++) {
		printf(" [%s]", argv[i]);
	}
	printf("\n");
}

/** read playback file */
static struct replay_scenario* 
setup_playback(const char* filename)
{
	struct replay_scenario* scen = NULL;
	if(filename) {
		FILE *in = fopen(filename, "r");
		if(!in) {
			perror(filename);
			exit(1);
		}
		scen = replay_scenario_read(in, filename);
		fclose(in);
		if(!scen)
			fatal_exit("Could not read: %s", filename);
	}
	else fatal_exit("need a playback file (-p)");
	log_info("Scenario: %s", scen->title);
	return scen;
}
	
/**
 * Main fake event test program. Setup, teardown and report errors.
 * @param argc: arg count.
 * @param argv: array of commandline arguments.
 */
int 
main(int argc, char* argv[])
{
	int c, res;
	int pass_argc = 0;
	char* pass_argv[MAXARG];
	char* playback_file = NULL;
	int init_optind = optind;
	char* init_optarg = optarg;
	struct replay_scenario* scen = NULL;

	log_info("Start of %s testbound program.", PACKAGE_STRING);
	/* determine commandline options for the daemon */
	pass_argc = 1;
	pass_argv[0] = "unbound";
	while( (c=getopt(argc, argv, "ho:p:")) != -1) {
		switch(c) {
		case 'p':
			playback_file = optarg;
			break;
		case 'o':
			add_opts(optarg, &pass_argc, pass_argv);
			break;
		case '?':
		case 'h':
		default:
			testbound_usage();
			return 1;
		}
	}
	argc -= optind;
	argv += optind;
	if(argc != 0) {
		testbound_usage();
		return 1;
	}

	/* setup test environment */
	scen = setup_playback(playback_file);
	/* init fake event backend */
	fake_event_init(scen);

	pass_argv[pass_argc] = NULL;
	echo_cmdline(pass_argc, pass_argv);

	/* reset getopt processing */
	optind = init_optind;
	optarg = init_optarg;

	/* run the normal daemon */
	res = daemon_main(pass_argc, pass_argv);

	fake_event_cleanup();
	for(c=1; c<pass_argc; c++)
		free(pass_argv[c]);
	return res;
}
