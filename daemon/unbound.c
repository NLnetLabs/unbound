/*
 * daemon/unbound.c - main program for unbound DNS resolver daemon.
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
 *
 * Main program to start the DNS resolver daemon.
 */

#include "config.h"
#include "util/log.h"
#include "daemon/worker.h"
#include "util/config_file.h"

/** buffer size for network connections */
#define BUFSZ 65552

/** print usage. */
static void usage()
{
	printf("usage: unbound [options]\n");
	printf("	start unbound daemon DNS resolver.\n");
	printf("-h	this help\n");
	printf("-c file	config file to read, unbound.conf(5).\n");
	printf("-v	verbose (multiple times increase verbosity)\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
}

/**
 * Run the daemon. 
 * @param cfgfile: the config file name.
 * @param cmdline_verbose: verbosity resulting from commandline -v.
 *    These increase verbosity as specified in the config file.
 */
static void run_daemon(const char* cfgfile, int cmdline_verbose)
{
	struct worker* worker = NULL;
	struct config_file *cfg = NULL;

	if(!(cfg = config_create())) {
		fprintf(stderr, "Could not init config defaults.");
		exit(1);
	}
	if(cfgfile) {
		if(!config_read(cfg, cfgfile)) {
			config_delete(cfg);
			exit(1);
		}
		verbosity = cmdline_verbose + cfg->verbosity;
	}
	log_info("Start of %s.", PACKAGE_STRING);

	/* setup */
	worker = worker_init(cfg, BUFSZ);
	if(!worker) {
		fatal_exit("could not initialize");
	}
	
	/* drop user priviliges and chroot if needed */
	log_info("start of service (%s).", PACKAGE_STRING);
	worker_work(worker);

	/* cleanup */
	verbose(VERB_ALGO, "Exit cleanup.");
	worker_delete(worker);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/**
 * main program. Set options given commandline arguments.
 * @param argc: number of commandline arguments.
 * @param argv: array of commandline arguments.
 * @return: exit status of the program.
 */
int 
main(int argc, char* argv[])
{
	int c;
	const char* cfgfile = NULL;
	int cmdline_verbose = 0;

	log_init();
	/* parse the options */
	while( (c=getopt(argc, argv, "c:hv")) != -1) {
		switch(c) {
		case 'c':
			cfgfile = optarg;
			break;
		case 'v':
			cmdline_verbose ++;
			verbosity++;
			break;
		case '?':
		case 'h':
		default:
			usage();
			return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if(argc != 0) {
		usage();
		return 1;
	}

	run_daemon(cfgfile, cmdline_verbose);
	return 0;
}
