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
#include "daemon/remote.h"

/** 
 * include the main program from the unbound daemon.
 * rename main to daemon_main to call it
 */
#define main daemon_main
#include "daemon/unbound.c"
#undef main

/** maximum line length for lines in the replay file. */
#define MAX_LINE_LEN 1024
/** the config file (removed at exit) */
static char cfgfile[MAX_LINE_LEN];

/** give commandline usage for testbound. */
static void
testbound_usage()
{
	printf("usage: testbound [options]\n");
	printf("\ttest the unbound daemon.\n");
	printf("-h      this help\n");
	printf("-p file	playback text file\n");
	printf("-2 	detect SHA256 support (exit code 0 or 1)\n");
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
add_opts(const char* optarg, int* pass_argc, char* pass_argv[])
{
	const char *p = optarg, *np;
	size_t len;
	while(p && isspace((int)*p)) 
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
		while(p && isspace((int)*p)) 
			p++;
	}
}

/** pretty print commandline for unbound in this test */
static void
echo_cmdline(int argc, char* argv[])
{
	int i;
	fprintf(stderr, "testbound is starting:");
	for(i=0; i<argc; i++) {
		fprintf(stderr, " [%s]", argv[i]);
	}
	fprintf(stderr, "\n");
}

/** process config elements */
static void
setup_config(FILE* in, char* configfile, int* lineno,
	int* pass_argc, char* pass_argv[])
{
	char line[MAX_LINE_LEN];
	char* parse;
	FILE* cfg;
#ifdef USE_WINSOCK
	snprintf(configfile, MAX_LINE_LEN, "testbound_cfg_%u.tmp", 
		(unsigned)getpid());
#else
	snprintf(configfile, MAX_LINE_LEN, "/tmp/testbound_cfg_%u.tmp", 
		(unsigned)getpid());
#endif
	add_opts("-c", pass_argc, pass_argv);
	add_opts(configfile, pass_argc, pass_argv);
	cfg = fopen(configfile, "w");
	if(!cfg) fatal_exit("could not open %s: %s", 
			configfile, strerror(errno));
	line[MAX_LINE_LEN-1] = 0;
	/* some basic settings to not pollute the host system */
	fprintf(cfg, "server:	use-syslog: no\n");
	fprintf(cfg, "		directory: \"\"\n");
	fprintf(cfg, "		chroot: \"\"\n");
	fprintf(cfg, "		username: \"\"\n");
	fprintf(cfg, "		pidfile: \"\"\n");
	while(fgets(line, MAX_LINE_LEN-1, in)) {
		parse = line;
		(*lineno)++;
		while(isspace((int)*parse))
			parse++;
		if(!*parse || parse[0] == ';')
			continue;
		if(strncmp(parse, "COMMANDLINE", 11) == 0) {
			parse[strlen(parse)-1] = 0; /* strip off \n */
			add_opts(parse+11, pass_argc, pass_argv);
			continue;
		}
		if(strncmp(parse, "CONFIG_END", 10) == 0) {
			fclose(cfg);
			return;
		}
		fputs(line, cfg);
	}
	fatal_exit("No CONFIG_END in input file");

}

/** read playback file */
static struct replay_scenario* 
setup_playback(const char* filename, char* configfile,
	int* pass_argc, char* pass_argv[])
{
	struct replay_scenario* scen = NULL;
	int lineno = 0;

	if(filename) {
		FILE *in = fopen(filename, "rb");
		if(!in) {
			perror(filename);
			exit(1);
		}
		setup_config(in, configfile, &lineno, pass_argc, pass_argv);
		scen = replay_scenario_read(in, filename, &lineno);
		fclose(in);
		if(!scen)
			fatal_exit("Could not read: %s", filename);
	}
	else fatal_exit("need a playback file (-p)");
	log_info("Scenario: %s", scen->title);
	return scen;
}

/** remove config file at exit */
void remove_configfile(void)
{
	unlink(cfgfile);
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

	log_init(NULL, 0, NULL);
	log_info("Start of %s testbound program.", PACKAGE_STRING);
	/* determine commandline options for the daemon */
	cfgfile[0] = 0;
	pass_argc = 1;
	pass_argv[0] = "unbound";
	add_opts("-d", &pass_argc, pass_argv);
	while( (c=getopt(argc, argv, "2ho:p:")) != -1) {
		switch(c) {
		case '2':
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
			printf("SHA256 supported\n");
			exit(0);
#else
			printf("SHA256 not supported\n");
			exit(1);
#endif
			break;
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
	if(atexit(&remove_configfile) != 0)
		fatal_exit("atexit() failed: %s", strerror(errno));

	/* setup test environment */
	scen = setup_playback(playback_file, cfgfile, &pass_argc, pass_argv);
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
	if(res == 0)
		log_info("Testbound Exit Success");
	return res;
}

/* fake remote control */
struct listen_port* daemon_remote_open_ports(struct config_file* 
	ATTR_UNUSED(cfg))
{
	return NULL;
}

struct daemon_remote* daemon_remote_create(struct worker* ATTR_UNUSED(worker))
{
	return (struct daemon_remote*)calloc(1,1);
}

void daemon_remote_delete(struct daemon_remote* rc)
{
	free(rc);
}

int daemon_remote_open_accept(struct daemon_remote* ATTR_UNUSED(rc),
        struct listen_port* ATTR_UNUSED(ports))
{
	return 1;
}

int remote_accept_callback(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(repinfo))
{
	log_assert(0);
	return 0;
}

int remote_control_callback(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(repinfo))
{
	log_assert(0);
	return 0;
}

void wsvc_command_option(const char* ATTR_UNUSED(wopt), 
	const char* ATTR_UNUSED(cfgfile), int ATTR_UNUSED(v), 
	int ATTR_UNUSED(c))
{
	log_assert(0);
}

void wsvc_setup_worker(struct worker* ATTR_UNUSED(worker))
{
	/* do nothing */
}

void wsvc_desetup_worker(struct worker* ATTR_UNUSED(worker))
{
	/* do nothing */
}

#ifdef UB_ON_WINDOWS
void worker_win_stop_cb(int ATTR_UNUSED(fd), short ATTR_UNUSED(ev),
	void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

void wsvc_cron_cb(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}
#endif /* UB_ON_WINDOWS */

