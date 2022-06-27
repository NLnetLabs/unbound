/*
 * testcode/doqclient.c - debug program. Perform multiple DNS queries using DoQ.
 *
 * Copyright (c) 2022, NLnet Labs. All rights reserved.
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
 */

/** 
 * \file
 *
 * Simple DNS-over-QUIC client. For testing and debugging purposes.
 * No authentication of TLS cert.
 */

#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_NGTCP2
#include <ngtcp2/ngtcp2.h>
#include "util/locks.h"
#include "util/net_help.h"
#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "sldns/wire2str.h"
#include "util/data/msgreply.h"
#include "util/data/msgencode.h"
#include "util/data/msgparse.h"

/** usage of doqclient */
static void usage(char* argv[])
{
	printf("usage: %s [options] name type class ...\n", argv[0]);
	printf("	sends the name-type-class queries over "
			"DNS-over-QUIC.\n");
	printf("-s server	IP address to send the queries to, "
			"default: 127.0.0.1\n");
	printf("-p		Port to connect to, default: %d\n",
		UNBOUND_DNS_OVER_QUIC_PORT);
	printf("-h 		This help text\n");
	exit(1);
}

/** open UDP socket to svr */
static int
open_svr_udp(const char* svr, int port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int fd = -1;
	int r;
	if(!ipstrtoaddr(svr, port, &addr, &addrlen)) {
		printf("fatal: bad server specs '%s'\n", svr);
		exit(1);
	}

	fd = socket(addr_is_ip6(&addr, addrlen)?PF_INET6:PF_INET,
		SOCK_DGRAM, 0);
	if(fd == -1) {
		perror("socket() error");
		exit(1);
	}
	r = connect(fd, (struct sockaddr*)&addr, addrlen);
	if(r < 0 && r != EINPROGRESS) {
		perror("connect() error");
		exit(1);
	}
	return fd;
}

static sldns_buffer*
make_query(char* qname, char* qtype, char* qclass)
{
	struct query_info qinfo;
	struct edns_data edns;
	sldns_buffer* buf = sldns_buffer_new(65553);
	if(!buf) fatal_exit("out of memory");
	qinfo.qname = sldns_str2wire_dname(qname, &qinfo.qname_len);
	if(!qinfo.qname) {
		printf("cannot parse query name: '%s'\n", qname);
		exit(1);
	}

	qinfo.qtype = sldns_get_rr_type_by_name(qtype);
	qinfo.qclass = sldns_get_rr_class_by_name(qclass);
	qinfo.local_alias = NULL;

	qinfo_query_encode(buf, &qinfo); /* flips buffer */
	free(qinfo.qname);
	sldns_buffer_write_u16_at(buf, 0, 0x0000);
	sldns_buffer_write_u16_at(buf, 2, BIT_RD);
	memset(&edns, 0, sizeof(edns));
	edns.edns_present = 1;
	edns.bits = EDNS_DO;
	edns.udp_size = 4096;
	if(sldns_buffer_capacity(buf) >=
		sldns_buffer_limit(buf)+calc_edns_field_size(&edns))
		attach_edns_record(buf, &edns);
	return buf;
}

/* run the dohclient queries */
static void run(const char* svr, int port, char** qs, int count)
{
	int fd, i;
	struct sldns_buffer* buf = NULL;
	char* str;
	fd = open_svr_udp(svr, port);

	/* handle query */
	for(i=0; i<count; i+=3) {
		buf = make_query(qs[i], qs[i+1], qs[i+2]);
		log_buf(0, "send query", buf);
		str = sldns_wire2str_pkt(sldns_buffer_begin(buf),
			sldns_buffer_limit(buf));
		if(!str) verbose(0, "could not sldns_wire2str_pkt");
		else verbose(0, "send query: %s", str);
		free(str);
	}

	sock_close(fd);
}
#endif /* HAVE_NGTCP2 */

#ifdef HAVE_NGTCP2
/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;
int main(int ATTR_UNUSED(argc), char** ATTR_UNUSED(argv))
{
	int c;
	int port = UNBOUND_DNS_OVER_QUIC_PORT;
	const char* svr = "127.0.0.1";
#ifdef USE_WINSOCK
	WSADATA wsa_data;
	if(WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}
#endif
	checklock_start();
	log_init(0, 0, 0);

	while((c=getopt(argc, argv, "hp:s:")) != -1) {
		switch(c) {
			case 'p':
				if(atoi(optarg)==0 && strcmp(optarg,"0")!=0) {
					printf("error parsing port, "
					    "number expected: %s\n", optarg);
					return 1;
				}
				port = atoi(optarg);
				break;
			case 's':
				svr = optarg;
				break;
			case 'h':
			case '?':
			default:
				usage(argv);
		}
	}

	argc -= optind;
	argv += optind;

	if(argc%3!=0) {
		printf("Invalid input. Specify qname, qtype, and qclass.\n");
		return 1;
	}

	run(svr, port, argv, argc/3);

	checklock_stop();
#ifdef USE_WINSOCK
	WSACleanup();
#endif
	return 0;
}
#else /* HAVE_NGTCP2 */
int main(int ATTR_UNUSED(argc), char** ATTR_UNUSED(argv))
{
	printf("Compiled without ngtcp2 for QUIC, cannot run doqclient.\n");
	return 1;
}
#endif /* HAVE_NGTCP2 */
