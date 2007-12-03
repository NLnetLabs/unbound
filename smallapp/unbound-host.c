/*
 * checkconf/unbound-host.c - replacement for host that supports validation.
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
 */

/**
 * \file
 *
 * This file performs functionality like 'host', and also supports validation.
 * It uses the libunbound library.
 */

#include "config.h"
#include "libunbound/unbound.h"
#include <ldns/ldns.h>

/** Give unbound-host usage, and exit (1). */
static void
usage()
{
	printf("Usage:	unbound-host [-c class] [-t type] hostname\n");
	printf("  Queries the DNS for information.\n");
	printf("  The hostname is looked up for IP4, IP6 and mail.\n");
	printf("  If an ip-address is given a reverse lookup is done.\n");
	printf("-t type		what type to look for.\n");
	printf("-c class	what class to look for, if not class IN.\n");
	printf("-h		show this usage help.\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
	exit(1);
}

/** determine if str is ip4 */
static int
isip4(const char* nm, char** res)
{
	struct in_addr addr;
	/* ddd.ddd.ddd.ddd.in-addr.arpa. is less than 32 */
	char buf[32];
	if(inet_pton(AF_INET, nm, &addr) <= 0) {
		return 0;
	}
	snprintf(buf, sizeof(buf), "%u.%u.%u.%u.in-addr.arpa.",
		((uint8_t*)&addr)[3], ((uint8_t*)&addr)[2],
		((uint8_t*)&addr)[1], ((uint8_t*)&addr)[0]);
	*res = strdup(buf);
	return 1;
}

/** determine if str is ip6 */
static int
isip6(const char* nm, char** res)
{
	struct in6_addr addr;
	/* [nibble.]{32}.ip6.arpa. is less than 128 */
	char* hex = "0123456789abcdef";
	char buf[128];
	char *p;
	int i;
	if(inet_pton(AF_INET6, nm, &addr) <= 0) {
		return 0;
	}
	p = buf;
	for(i=15; i>=0; i--) {
		uint8_t b = ((uint8_t*)&addr)[i];
		*p++ = hex[ (b&0x0f) ];
		*p++ = '.';
		*p++ = hex[ (b&0xf0) >> 4 ];
		*p++ = '.';
	}
	snprintf(buf+16*4, sizeof(buf)-16*4, "ip6.arpa.");
	*res = strdup(buf);
	return 1;
}

/** massage input name */
static char*
massage_qname(const char* nm, int* reverse)
{
	/* recognise IP4 and IP6, create reverse addresses if needed */
	char* res;
	if(isip4(nm, &res)) {
		*reverse = 1;
	} else if(isip6(nm, &res)) {
		*reverse = 1;
	} else {
		res = strdup(nm);
	}
	if(!res) {
		fprintf(stderr, "error: out of memory\n");
		exit(1);
	}
	return res;
}

/** massage input type */
static int
massage_type(const char* t, int reverse, int* multi)
{
	if(t) {
		int r = ldns_get_rr_type_by_name(t);
		if(r == 0 && strcasecmp(t, "TYPE0") != 0 && 
			strcmp(t, "") != 0) {
			fprintf(stderr, "error unknown type %s\n", t);
			exit(1);
		}
		return r;
	}
	if(!t && reverse)
		return LDNS_RR_TYPE_PTR;
	*multi = 1;
	return LDNS_RR_TYPE_A;
}

/** massage input class */
static int
massage_class(const char* c)
{
	if(c) {
		int r = ldns_get_rr_class_by_name(c);
		if(r == 0 && strcasecmp(c, "CLASS0") != 0 && 
			strcmp(c, "") != 0) {
			fprintf(stderr, "error unknown class %s\n", c);
			exit(1);
		}
		return r;
	}
	return LDNS_RR_CLASS_IN;
}

/** nice security status string */
static const char* 
statstr(int sec, struct ub_val_result* result)
{
	if(sec) return "[secure]";
	if(result->bogus) return "[not-secure (BOGUS)]";
	return "[not-secure (insecure)]";
}

/** nice string for type */
static void
pretty_type(char* s, size_t len, int t)
{
	const ldns_rr_descriptor *d = ldns_rr_descript(t);
	if(d) {
		snprintf(s, len, "%s", d->_name);
	} else {
		snprintf(s, len, "TYPE%d", t);
	}
}

/** nice string for class */
static void
pretty_class(char* s, size_t len, int c)
{
	ldns_lookup_table *cl = ldns_lookup_by_id(ldns_rr_classes, c);
	if(cl) {
		snprintf(s, len, "%s", cl->name);
	} else {
		snprintf(s, len, "CLASS%d", c);
	}
}

/** nice string for rcode */
static void
pretty_rcode(char* s, size_t len, int r)
{
	ldns_lookup_table *rcode = ldns_lookup_by_id(ldns_rcodes, r);
	if(rcode) {
		snprintf(s, len, "%s", rcode->name);
	} else {
		snprintf(s, len, "RCODE%d", r);
	}
}

/** pretty line of output for results */
static void
pretty_output(char* q, int t, int c, int sec, int haved, 
	struct ub_val_result* result)
{
	const char *secstatus = statstr(sec, result);
	char tstr[16];
	char cstr[16];
	char rcodestr[16];
	pretty_type(tstr, 16, t);
	pretty_class(cstr, 16, c);
	pretty_rcode(rcodestr, 16, result->rcode);

	if(!haved && result->rcode) {
		printf("Host %s not found: %d(%s). %s\n",
			q, result->rcode, rcodestr, secstatus);
		return;
	}
	if(!haved) {
		printf("%s %s %s: no data. %s\n",
			q, cstr, tstr, secstatus);
		return;
	}
	printf("%s %s %s: have data. %s\n",
		q, cstr, tstr, secstatus);
	/* TODO print the data nicely */
}

/** perform a lookup and printout return if domain existed */
static int
dnslook(struct ub_val_ctx* ctx, char* q, int t, int c)
{
	int ret, sec, haved;
	struct ub_val_result* result;

	ret = ub_val_resolve(ctx, q, t, c, &sec, &haved, &result);
	if(ret != 0) {
		fprintf(stderr, "error: %s\n", ub_val_strerror(ret));
		exit(1);
	}
	pretty_output(q, t, c, sec, haved, result);
	ret = result->nxdomain;
	ub_val_result_free(result);
	return ret;
}

/** perform host lookup */
static void
lookup(const char* nm, const char* qt, const char* qc)
{
	/* massage input into a query name, type and class */
	int multi = 0;	 /* no type, so do A, AAAA, MX */
	int reverse = 0; /* we are doing a reverse lookup */
	char* realq = massage_qname(nm, &reverse);
	int t = massage_type(qt, reverse, &multi);
	int c = massage_class(qc);

	/* perform the query */
	struct ub_val_ctx* ctx = NULL;
	
	printf("lookup %s %d %d reverse=%d multi=%d\n", 
		realq, t, c, reverse, multi);
	ctx = ub_val_ctx_create();
	if(!ctx) {
		fprintf(stderr, "error: out of memory\n");
		exit(1);
	}
	if(multi) {
		if(!dnslook(ctx, realq, LDNS_RR_TYPE_A, c)) {
			/* domain exists, lookup more */
			(void)dnslook(ctx, realq, LDNS_RR_TYPE_AAAA, c);
			(void)dnslook(ctx, realq, LDNS_RR_TYPE_MX, c);
		}
	} else {
		(void)dnslook(ctx, realq, t, c);
	}
	ub_val_ctx_delete(ctx);
	free(realq);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** Main routine for checkconf */
int main(int argc, char* argv[])
{
	int c;
	char* qclass = NULL;
	char* qtype = NULL;
	/* parse the options */
	while( (c=getopt(argc, argv, "c:ht:")) != -1) {
		switch(c) {
		case 'c':
			qclass = optarg;
			break;
		case 't':
			qtype = optarg;
			break;
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

	lookup(argv[0], qtype, qclass);
	return 0;
}
