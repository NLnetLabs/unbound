/*
 * testcode/perf.c - debug program to estimate name server performance.
 *
 * Copyright (c) 2008, NLnet Labs. All rights reserved.
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
 */

/**
 * \file
 *
 * This program estimates DNS name server performance.
 */

#include "config.h"
#include "util/log.h"
#include "util/locks.h"
#include "util/netevent.h"

/** usage information for perf */
static void usage(char* argv[])
{
	printf("usage: %s [options] server\n", argv[0]);
	printf("server: ip address of server, IP4 or IP6.\n");
	printf("	If not on port %d add @port.\n", UNBOUND_DNS_PORT);
	exit(1);
}

struct perfinfo;
struct perfio;

/** Global info for perf */
struct perfinfo { 
	/** communication base */
	struct comm_base* base;
	/** timer to send out new queries */
	struct comm_timer* sendout_timer;
	/** all purpose buffer */
	ldns_buffer* udpbuffer;

	/** destination */
	struct sockaddr_storage dest;
	/** length of dest socket addr */
	socklen_t destlen;

	/** number of seconds elapsed in total */
	struct timeval time_cumul;
	/** number of queries received in that time */
	size_t numrecv;
	/** number of queries sent out in that time */
	size_t numsent;
	/** numbers by rcode */
	size_t by_rcode[32];
	
	/** number of I/O ports */
	size_t io_num;
	/** I/O ports array */
	struct perfio* io;

	/** size of querylist */
	size_t qlist_size;
	/** list of query packets (data) */
	uint8_t** qlist_data;
	/** list of query packets (length of a packet) */
	size_t** qlist_len;
	/** index into querylist, for walking the list */
	size_t qlist_idx;
};

/** I/O port for perf */
struct perfio {
	/** id number */
	int id;
	/** file descriptor of socket */
	int fd;
	/** ptr back to perfinfo */
	struct perfinfo* info;
};

/** setup perf test environment */
static void
perfsetup(struct perfinfo* info, struct sockaddr_storage* dest, 
	socklen_t destlen)
{
	info->base = comm_base_create();
	if(!info->base) fatal_exit("out of memory");
	info->sendout_timer = comm_timer_create(...);
	if(!info->sendout_timer) fatal_exit("out of memory");
	info->udpbuffer = ldns_buffer_new(65553);
	if(!info->udpbuffer) fatal_exit("out of memory");
}

/** cleanup perf test environment */
static void
perffree(struct perfinfo* info)
{
	comm_timer_delete(info->sendout_timer);
	comm_base_free(info->base);
	ldns_buffer_free(info->udpbuffer);
}

/** perform the performance test */
static void
perfmain(struct perfinfo* info)
{
	perfsetup(info, dest, destlen);
	comm_base_dispatch(info->base);
	perffree(info);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** main program for perf */
int main(int argc, char* argv[]) 
{
	int c;
	struct perfinfo info;

	/* defaults */
	memset(&info, 0, sizeof(info));
	info.io_num = 16;

	log_init(NULL, 0, NULL);
	checklock_start();

	/* parse the options */
	while( (c=getopt(argc, argv, "h")) != -1) {
		switch(c) {
		case '?':
		case 'h':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if(argc != 1)
		usage();
	if(!extstrtoaddr(argv[0], &info->dest, &info->destlen)) {
		printf("Could not parse ip: %s\n", argv[0]);
		return 1;
	}
	
	/* do the performance test */
	perf_main(&dest, destlen);

	checklock_stop();
	return 0;
}
