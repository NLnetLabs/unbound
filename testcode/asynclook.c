/*
 * testcode/asynclook.c - debug program perform async libunbound queries.
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
 * This program shows the results from several background lookups,
 * while printing time in the foreground.
 */

#include "config.h"
#include "libunbound/unbound.h"

/**
 * result list for the lookups
 */
struct lookinfo {
	/** name to look up */
	char* qname;
	/** tracking number that can be used to cancel the query */
	int async_id;
	/** error code from libunbound */
	int err;
	/** result from lookup */
	struct ub_val_result* result;
};

/** global variable to see how many queries we have left */
static int num_wait = 0;

/** usage information for asynclook */
void usage(char* argv[])
{
	printf("usage: %s name ...\n", argv[0]);
	printf("names are looked up at the same time, asynchronously.\n");
	printf("-d : enable debug output\n");
	printf("-t : use a resolver thread instead of forking a process\n");
	exit(1);
}

/** this is a function of type ub_val_callback_t */
void lookup_is_done(void* mydata, int err, struct ub_val_result* result)
{
	/* cast mydata back to the correct type */
	struct lookinfo* info = (struct lookinfo*)mydata;
	fprintf(stderr, "name %s resolved\n", info->qname);
	info->err = err;
	info->result = result;
	/* one less to wait for */
	num_wait--;
}

/** main program for asynclook */
int main(int argc, char** argv) 
{
	struct ub_val_ctx* ctx;
	struct lookinfo* lookups;
	int i, r;
	if(argc == 1) {
		usage(argv);
	}
	if(argc > 1 && strcmp(argv[1], "-h") == 0)
		usage(argv);
	argc--;
	argv++;

	/* create context */
	ctx = ub_val_ctx_create();
	if(!ctx) {
		printf("could not create context, %s", strerror(errno));
		return 1;
	}
	if(argc > 0 && strcmp(argv[0], "-d") == 0) {
		ub_val_ctx_debuglevel(ctx, 3);
		argc--;
		argv++;
	} 
	if(argc > 0 && strcmp(argv[0], "-t") == 0) {
		ub_val_ctx_async(ctx, 1);
		argc--;
		argv++;
	}

	/* allocate array for results. */
	lookups = (struct lookinfo*)calloc((size_t)argc, 
		sizeof(struct lookinfo));
	if(!lookups) {
		printf("out of memory\n");
		return 1;
	}

	/* perform asyncronous calls */
	num_wait = argc;
	for(i=0; i<argc; i++) {
		fprintf(stderr, "start lookup %s\n", argv[i]);
		lookups[i].qname = argv[i];
		r = ub_val_resolve_async(ctx, argv[i], LDNS_RR_TYPE_A,
			LDNS_RR_CLASS_IN, &lookups[i], &lookup_is_done, 
			&lookups[i].async_id);
	}

	/* wait while the hostnames are looked up. Do something useful here */
	for(i=0; i<1000; i++) {
		usleep(100000);
		printf("%g seconds passed\n", 0.1*(double)i);
		r = ub_val_ctx_process(ctx);
		if(r != 0) {
			printf("ub_val_ctx_process error: %s\n",
				ub_val_strerror(r));
			return 1;
		}
		if(num_wait == 0)
			break;
	}
	if(i>=999) {
		printf("timed out\n");
		return 0;
	}
	printf("lookup complete\n");

	/* print lookup results */
	for(i=0; i<argc; i++) {
		char buf[100];
		if(lookups[i].err) /* error (from libunbound) */
			printf("%s: error %s\n", lookups[i].qname,
				ub_val_strerror(lookups[i].err));
		else if(lookups[i].result->havedata)
			printf("%s: %s\n", lookups[i].qname,
				inet_ntop(AF_INET, lookups[i].result->data[0],
				buf, (socklen_t)sizeof(buf)));
		else {
			/* there is no data, why that? */
			if(lookups[i].result->rcode == 0 /*noerror*/ ||
				lookups[i].result->nxdomain)
				printf("%s: no data %s\n", lookups[i].qname,
				lookups[i].result->nxdomain?"(no such host)":
				"(no IP4 address)");
			else	/* some error (from the server) */
				printf("%s: DNS error %d\n", lookups[i].qname,
					lookups[i].result->rcode);
		}
	}

	ub_val_ctx_delete(ctx);
	for(i=0; i<argc; i++) {
		ub_val_result_free(lookups[i].result);
	}
	free(lookups);
	return 0;
}
