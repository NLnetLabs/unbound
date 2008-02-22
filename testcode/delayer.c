/*
 * testcode/delayer.c - debug program that delays queries to a server.
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
 * This program delays queries made. It performs as a proxy to another
 * server and delays queries to it.
 */

#include "config.h"
#include "util/net_help.h"
#include "util/config_file.h"
#include <signal.h>

/**
 * The ring buffer
 */
struct ringbuf {
	/** base of buffer */
	uint8_t* buf;
	/** size of buffer */
	size_t size;
	/** low mark, items start here */
	size_t low;
	/** high mark, items end here */
	size_t high;
};

/** usage information for delayer */
void usage(char* argv[])
{
	printf("usage: %s [options]\n", argv[0]);
	printf("	-f addr : use addr, forward to that server, @port.\n");
	printf("	-b addr : bind to this address to listen.\n");
	printf("	-p port : bind to this port (0 for random).\n");
	printf("	-m mem	: use this much memory for waiting queries.\n");
	printf("	-d delay: queries are delayed n milliseconds.\n");
	printf("	-h 	: this help message\n");
	exit(1);
}

/** create new ring buffer */
static struct ringbuf*
ring_create(size_t sz)
{
	struct ringbuf* r = (struct ringbuf*)calloc(1, sizeof(*r));
	if(!r) fatal_exit("out of memory");
	r->buf = (uint8_t*)malloc(sz);
	if(!r->buf) fatal_exit("out of memory");
	r->size = sz;
	r->low = 0;
	r->high = 0;
	return r;
}

/** delete ring buffer */
static void
ring_delete(struct ringbuf* r)
{
	if(!r) return;
	free(r->buf);
	free(r);
}

/** signal handler global info */
static volatile int do_quit = 0;

/** signal handler for user quit */
static RETSIGTYPE delayer_sigh(int sig)
{
	printf("exit on signal %d\n", sig);
	do_quit = 1;
}

/** delayer main service routine */
static void
service(char* bind_str, int bindport, char* serv_str, size_t memsize, 
	int delay_msec)
{
	struct sockaddr_storage bind_addr, srv_addr;
	socklen_t bind_len, srv_len;
	struct ringbuf* ring = ring_create(memsize);
	struct timeval delay;
	int s;
	delay.tv_sec = delay_msec / 1000;
	delay.tv_usec = (delay_msec % 1000)*1000;
	if(bindport == 0)
		bindport = 1024 + random()%64000;
	if(!ipstrtoaddr(bind_str, bindport, &bind_addr, &bind_len)) {
		printf("cannot parse listen address: %s\n", bind_str);
		exit(1);
	}
	if(!extstrtoaddr(serv_str, &srv_addr, &srv_len)) {
		printf("cannot parse forward address: %s\n", serv_str);
		exit(1);
	}
	if( signal(SIGINT, delayer_sigh) == SIG_ERR ||
		signal(SIGTERM, delayer_sigh) == SIG_ERR ||
		signal(SIGHUP, delayer_sigh) == SIG_ERR ||
		signal(SIGQUIT, delayer_sigh) == SIG_ERR ||
		signal(SIGALRM, delayer_sigh) == SIG_ERR)
		fatal_exit("could not bind to signal");
	/* bind UDP port */
	if((s = socket(addr_is_ip6(&bind_addr, bind_len)?AF_INET6:AF_INET,
		SOCK_DGRAM, 0)) == -1)
		fatal_exit("socket: %s", strerror(errno));
	if(bind(s, (struct sockaddr*)&bind_addr, bind_len) == -1)
		fatal_exit("bind: %s", strerror(errno));
	printf("listening on port: %d\n", bindport);

	/* process loop */
	/* wait for events */
	while(!do_quit) {
		/* get current time */
		/* sendout delayed queries to master server (frees up buffer)*/
		/* see what can be received to start waiting */
		/* see what next timeout is (if any) */
	}
	/* cleanup */
	close(s);
	ring_delete(ring);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** main program for delayer */
int main(int argc, char** argv) 
{
	int c;		/* defaults */
	char* server = "127.0.0.1@53";
	char* bindto = "0.0.0.0";
	int bindport = 0;
	size_t memsize = 10*1024*1024;
	int delay = 100;

	srandom(time(NULL) ^ getpid());
	while( (c=getopt(argc, argv, "b:d:f:hm:p:")) != -1) {
		switch(c) {
			case 'b':
				bindto = optarg;
				break;
			case 'd':
				if(atoi(optarg)==0 && strcmp(optarg,"0")!=0) {
					printf("bad delay: %s\n", optarg);
					return 1;
				}
				delay = atoi(optarg);
				break;
			case 'f':
				server = optarg;
				break;
			case 'm':
				if(!cfg_parse_memsize(optarg, &memsize)) {
					printf("bad memsize: %s\n", optarg);
					return 1;
				}
				break;
			case 'p':
				if(atoi(optarg)==0 && strcmp(optarg,"0")!=0) {
					printf("bad port nr: %s\n", optarg);
					return 1;
				}
				bindport = atoi(optarg);
				break;
			case 'h':
			case '?':
			default:
				usage(argv);
		}
	}
	argc -= optind;
	argv += optind;
	if(argc != 0)
		usage(argv);

	printf("bind to %s @ %d and forward to %s\n"
		"after %d msec (buffer %d)\n", 
		bindto, bindport, server, delay, (int)memsize);
	service(bindto, bindport, server, memsize, delay);
	return 0;
}
