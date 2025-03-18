/*
 * testcode/shell-parse.c - debug program. Creates a DNS query message from input parameters and
 * parses messages from stdin with wire2str.
 *
 * Copyright (c) 2020, NLnet Labs. All rights reserved.
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
 * Creates a DNS query message from input parameter. This can be piped to another program.
 * For testing and debugging purposes.
 */

#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "sldns/sbuffer.h"
#include "sldns/str2wire.h"
#include "sldns/wire2str.h"
#include "util/data/msgencode.h"
#include "util/data/msgparse.h"
#include "util/data/msgreply.h"
#include "util/net_help.h"

#define READBUF_SIZE	(512U)	/* 512 should be plenty for our purposes */

static void usage(char* argv[])
{
	printf("usage: %s name type class\n", argv[0]);
	printf("       %s\n", argv[0]);
	exit(1);
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
	if(qinfo.qtype == 0 && strcmp(qtype, "TYPE0") != 0) {
		printf("cannot parse query type: '%s'\n", qtype);
		exit(1);
	}
	qinfo.qclass = sldns_get_rr_class_by_name(qclass);
	if(qinfo.qclass == 0 && strcmp(qclass, "CLASS0") != 0) {
		printf("cannot parse query class: '%s'\n", qclass);
		exit(1);
	}
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

int main(int argc, char** argv)
{
	struct sldns_buffer* buf = NULL;

	if(((argc != 1) && (argc != 4)) || ((argc == 2) && (strcmp(argv[1], "-h") == 0))) {
		usage(argv);
		return 1;
	}
	if (argc == 4) {
		buf = make_query(argv[1], argv[2], argv[3]);
		if (write(STDOUT_FILENO, sldns_buffer_begin(buf), sldns_buffer_remaining(buf)) <= 0) {
			log_err("Unable to write query to stdout");
			return 1;
		};
		sldns_buffer_free(buf);
	}
	else {
		uint8_t *buf = malloc(READBUF_SIZE);
		ssize_t bytes = read(STDIN_FILENO, buf, READBUF_SIZE);
		char *pktstr;

		if (bytes <= 0) {
			log_err("Unable to read bytes from stdin %s", strerror(errno));
			return 1;
		}

		pktstr = sldns_wire2str_pkt(buf, bytes);
		if (pktstr) {
			printf("%s\n", pktstr);
		}
		else {
			/* write back output */
			if (write(STDOUT_FILENO, buf, bytes) <= 0) {
				log_err("Unable to write query to stdout");
				return 1;
			};
		}
		free(pktstr);
	}
	
	return 0;
}
