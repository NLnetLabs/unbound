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
#include "util/netevent.h"
#include "services/listen_dnsport.h"

/** default port to listen for queries, passed to getaddrinfo */
#define UNBOUND_DNS_PORT "53"
/** buffer size for network connections */
#define BUFSZ 65552

/** print usage. */
static void usage()
{
	printf("usage: unbound [options]\n");
	printf("\tstart unbound daemon DNS resolver.\n");
	printf("-h	this help\n");
	printf("-p port	the port to listen on\n");
	printf("-v	verbose (multiple times increase verbosity)\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE file in source package.\n");
	printf("Report bugs to %s.\n", PACKAGE_BUGREPORT);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/**
 * main program. Set options given commandline arguments.
 * @param argc: number of commandline arguments.
 * @param argv: array of commandline arguments.
 */
int 
main(int argc, char* argv[])
{
	struct comm_base *base = 0;
	struct listen_dnsport* front = 0;
	int do_ip4=1, do_ip6=1, do_udp=1, do_tcp=1;
	const char* port = UNBOUND_DNS_PORT;
	int c;

	log_init();
	/* parse the options */
	while( (c=getopt(argc, argv, "hvp:")) != -1) {
		switch(c) {
		case 'p':
			if(!atoi(optarg))
				fatal_exit("invalid port '%s'", optarg);
			port = optarg;
			verbose(VERB_ALGO, "using port: %s", port);
			break;
		case 'v':
			verbosity ++;
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

	/* setup */
	base = comm_base_create();
	if(!base)
		fatal_exit("could not create event handling base");
	front = listen_create(base, 0, NULL, port, do_ip4, do_ip6, 
		do_udp, do_tcp, BUFSZ);
	if(!front)
		fatal_exit("could not create listening sockets");

	log_info("Start of %s.", PACKAGE_STRING);

	/* cleanup */
	verbose(VERB_ALGO, "Exit cleanup.");
	listen_delete(front);
	comm_base_delete(base);
	return 0;
}
