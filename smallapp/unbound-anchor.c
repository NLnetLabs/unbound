/*
 * unbound-anchor.c - update the root anchor if necessary.
 *
 * Copyright (c) 2010, NLnet Labs. All rights reserved.
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
 * This file checks to see that the current 5011 keys work to prime the
 * current root anchor.  If not a certificate is used to update the anchor.
 */

#include "config.h"
#include "libunbound/unbound.h"
#include <ldns/rr.h>
#include <expat.h>
#ifndef HAVE_EXPAT_H
#error "need libexpat to parse root-anchors.xml file."
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

/* TODO configure defines with prefix */
/** root key file, 5011 tracked */
#define ROOT_ANCHOR_FILE "/usr/local/etc/unbound/root.key"
/** root update cert file */
#define ROOT_CERT_FILE "/usr/local/etc/unbound/icannbundle.pem"
/** name of server in URL to fetch HTTPS from */
#define URLNAME "data.iana.org"
/** path on HTTPS server to xml file */
#define XMLNAME "/root-anchors/root-anchors.xml"
/** path on HTTPS server to p7s file */
#define P7SNAME "/root-anchors/root-anchors.p7s"
/** path on HTTPS server to pem file */
#define PEMNAME "/root-anchors/icannbundle.pem"

/** verbosity for this application */
static int verb = 0;

/** Give unbound-anchor usage, and exit (1). */
static void
usage()
{
	printf("Usage:	unbound-anchor [opts]\n");
	printf("	Setup or update root anchor. "
		"Most options have defaults.\n");
	printf("	Run this program before you start the validator.\n");
	printf("\n");
	printf("	The anchor and cert are filled with default builtin\n");
	printf("	values if the file does not exist or is empty.\n");
	printf("\n");
	printf("-a file		root key file, default %s\n", ROOT_ANCHOR_FILE);
	printf("-c file		cert file, default %s\n", ROOT_CERT_FILE);
	/* TODO
	printf("-o file		output key file, if enabled new key written"
		"		there and exit code 4 for manual change\n");
	*/
	printf("-u name		server in https url, default %s\n", URLNAME);
	printf("-x path		pathname to xml, default %s\n", XMLNAME);
	printf("-s path		pathname to p7s, default %s\n", P7SNAME);
	printf("-p path		pathname to pem, default %s\n", PEMNAME);
	printf("-4		work using IPv4 only\n");
	printf("-6		work using IPv6 only\n");
	printf("-f resolv.conf	use given resolv.conf to resolve -u name\n");
	printf("-r root.hints	use given root.hints to resolve -u name\n"
		"		builtin root hints are used by default\n");
	printf("-v		more verbose\n");
	printf("-C conf		debug, read config\n");
	printf("-F 		debug, force update with cert\n");
	printf("-h		show this usage help\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
	exit(1);
}

/** perform actual certupdate work */
static int
do_certupdate(char* root_anchor_file, char* root_cert_file,
	char* urlname, char* xmlname, char* p7sname, char* pemname,
	char* res_conf, char* root_hints, char* debugconf,
	int ip4only, int ip6only, struct ub_result* dnskey)
{
	/* read pem file or provide builtin */

	/* lookup A, AAAA for the urlname (or parse urlname if IP address) */

	/* fetch the necessary files over HTTPS */

	/* update the pem file (optional) */

	/* verify and update the root anchor */
		/* verify xml file */
		/* see if xml file verifies the dnskey that was probed */
		/* reinstate 5011 tracking */
	if(verb) printf("success: the anchor has been updated "
			"using the cert\n");
	ub_resolve_free(dnskey);
	return 0;
}

/**
 * Try to read the root RFC5011 autotrust anchor file,
 * @param file: filename.
 * @return:
 * 	0 if does not exist or empty
 * 	1 if trust-point-revoked-5011
 * 	2 if it is OK.
 */
static int
try_read_anchor(char* file)
{
	int empty = 1;
	char line[10240];
	char* p;
	FILE* in = fopen(file, "r");
	if(!in) {
		/* only if the file does not exist, can we fix it */
		if(errno != ENOENT) {
			if(verb) printf("%s: %s\n", file, strerror(errno));
			if(verb) printf("error: cannot access the file\n");
			exit(0);
		}
		if(verb) printf("%s does not exist\n", file);
		return 0;
	}
	while(fgets(line, (int)sizeof(line), in)) {
		line[sizeof(line)-1] = 0;
		if(strncmp(line, ";;REVOKED", 9) == 0) {
			fclose(in);
			if(verb) printf("%s : the trust point is revoked\n"
				"and the zone is considered unsigned.\n"
				"if you wish to re-enable, delete the file\n",
				file);
			return 1;
		}
		p=line;
		while(*p == ' ' || *p == '\t')
			p++;
		if(p[0]==0 || p[0]=='\n' || p[0]==';') continue;
		/* this line is a line of content */
		empty = 0;
	}
	fclose(in);
	if(empty) {
		if(verb) printf("%s is empty\n", file);
		return 0;
	}
	if(verb) printf("%s has content\n", file);
	return 2;
}

/** Write the builtin root anchor to a file */
static void
write_builtin_anchor(char* file)
{
	const char* builtin_root_anchor = ". IN DS 19036 8 2 49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5\n";
	FILE* out = fopen(file, "w");
	if(!out) {
		if(verb) printf("%s: %s\n", file, strerror(errno));
		if(verb) printf("  could not write builtin anchor\n");
		return;
	}
	if(!fwrite(builtin_root_anchor, strlen(builtin_root_anchor), 1, out)) {
		if(verb) printf("%s: %s\n", file, strerror(errno));
		if(verb) printf("  could not complete write builtin anchor\n");
	}
	fclose(out);
}

/** 
 * Check the root anchor file.
 * If does not exist, provide builtin and write file.
 * If empty, provide builtin and write file.
 * If trust-point-revoked-5011 file: make the program exit.
 */
static int
provide_builtin(char* root_anchor_file)
{
	/* try to read it */
	switch(try_read_anchor(root_anchor_file))
	{
		case 0: /* no exist or empty */
			write_builtin_anchor(root_anchor_file);
			break;
		case 1: /* revoked tp */
			return 0;	
		case 2: /* it is fine */
		default:
			break;
	}
	return 1;
}

/** print ub context creation error and exit */
static void
ub_ctx_error_exit(struct ub_ctx* ctx, const char* str, const char* str2)
{
	ub_ctx_delete(ctx);
	if(str && str2 && verb) printf("%s: %s\n", str, str2);
	if(verb) printf("error: could not create unbound resolver context\n");
	exit(0);
}

/**
 * Create a new unbound context with the commandline settings applied
 */
static struct ub_ctx* 
create_unbound_context(char* res_conf, char* root_hints, char* debugconf,
        int ip4only, int ip6only)
{
	int r;
	struct ub_ctx* ctx = ub_ctx_create();
	if(!ctx) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	/* do not waste time and network traffic to fetch extra nameservers */
	r = ub_ctx_set_option(ctx, "target-fetch-policy:", "0 0 0 0 0");
	if(r && verb) printf("ctx targetfetchpolicy: %s\n", ub_strerror(r));
	/* read config file first, so its settings can be overridden */
	if(debugconf) {
		r = ub_ctx_config(ctx, debugconf);
		if(r) ub_ctx_error_exit(ctx, debugconf, ub_strerror(r));
	}
	if(res_conf) {
		r = ub_ctx_resolvconf(ctx, res_conf);
		if(r) ub_ctx_error_exit(ctx, res_conf, ub_strerror(r));
	}
	if(root_hints) {
		r = ub_ctx_set_option(ctx, "root-hints:", root_hints);
		if(r) ub_ctx_error_exit(ctx, root_hints, ub_strerror(r));
	}
	if(ip4only) {
		r = ub_ctx_set_option(ctx, "do-ip6:", "no");
		if(r) ub_ctx_error_exit(ctx, "ip4only", ub_strerror(r));
	}
	if(ip6only) {
		r = ub_ctx_set_option(ctx, "do-ip4:", "no");
		if(r) ub_ctx_error_exit(ctx, "ip6only", ub_strerror(r));
	}
	return ctx;
}

/**
 * add an autotrust anchor for the root to the context
 */
static void
add_5011_probe_root(struct ub_ctx* ctx, char* root_anchor_file)
{
	int r;
	r = ub_ctx_set_option(ctx, "auto-trust-anchor-file:", root_anchor_file);
	if(r) {
		if(verb) printf("add 5011 probe to ctx: %s\n", ub_strerror(r));
		ub_ctx_delete(ctx);
		exit(0);
	}
}

/**
 * Prime the root key and return the result.  Exit on error.
 */
static struct ub_result*
prime_root_key(struct ub_ctx* ctx)
{
	struct ub_result* res = NULL;
	int r;
	r = ub_resolve(ctx, ".", LDNS_RR_TYPE_DNSKEY, LDNS_RR_CLASS_IN, &res);
	if(r) {
		if(verb) printf("resolve DNSKEY: %s\n", ub_strerror(r));
		ub_ctx_delete(ctx);
		exit(0);
	}
	if(!res) {
		if(verb) printf("out of memory\n");
		ub_ctx_delete(ctx);
		exit(0);
	}
	return res;
}

/** read last successful probe time from autotrust file (if possible) */
static int32_t
read_last_success_time(char* file)
{
	FILE* in = fopen(file, "r");
	char line[1024];
	if(!in) {
		if(verb) printf("%s: %s\n", file, strerror(errno));
		return 0;
	}
	while(fgets(line, (int)sizeof(line), in)) {
		if(strncmp(line, ";;last_success: ", 16) == 0) {
			char* e;
			time_t x = (unsigned int)strtol(line+16, &e, 10);
			fclose(in);
			if(line+16 == e) {
				if(verb) printf("failed to parse "
					"last_success probe time\n");
				return 0;
			}
			if(verb) printf("last successful probe: %s", ctime(&x));
			return (int32_t)x;
		}
	}
	fclose(in);
	if(verb) printf("no last_success probe time in anchor file\n");
	return 0;
}

/**
 * Get current time.
 * @param debugconf: with override time for tests
 */
static int32_t
get_time_now(char* debugconf)
{
	if(debugconf) {
		FILE* in = fopen(debugconf, "r");
		char line[1024];
		if(!in) {
			if(verb) printf("%s: %s\n", debugconf, strerror(errno));
			return (int32_t)time(NULL);
		}
		/* must be ^val-override-date: 1234567$ formatted */
		while(fgets(line, (int)sizeof(line), in)) {
			if(strncmp(line, "val-override-date: ", 19) == 0) {
				fclose(in);
				return (int32_t)atoi(line+19);
			}
		}
		fclose(in);
	}
	return (int32_t)time(NULL);
}

/**
 * Read autotrust 5011 probe file and see if the date
 * compared to the current date allows a certupdate.
 * If the last successful probe was recent then 5011 cannot be behind,
 * and the failure cannot be solved with a certupdate.
 * The debugconf is to validation-override the date for testing.
 * @return true if certupdate is ok.
 */
static int
probe_date_allows_certupdate(char* root_anchor_file, char* debugconf)
{
	int32_t last_success = read_last_success_time(root_anchor_file);
	int32_t now = get_time_now(debugconf);
	int32_t leeway = 30 * 24 * 3600; /* 30 days leeway */
	/* if the date is before 2010-07-15:00.00.00 then the root has not
	 * been signed yet, and thus we refuse to take action. */
	if(now - 1279144800 < 0) {
		if(verb) printf("the date is before the root was first signed,"
			" please correct the clock\n");
		return 0;
	}
	if(now - last_success < 0) {
		if(verb) printf("the last successful probe is in the future,"
			" clock was modified\n");
		return 0;
	}
	if(now - last_success >= leeway) {
		if(verb) printf("the last successful probe was more than 30 "
			"days ago\n");
		return 1;
	}
	if(verb) printf("the last successful probe is recent\n");
	return 0;
}

/** perform the unbound-anchor work */
static int
do_root_update_work(char* root_anchor_file, char* root_cert_file,
	char* urlname, char* xmlname, char* p7sname, char* pemname,
	char* res_conf, char* root_hints, char* debugconf,
	int ip4only, int ip6only, int force)
{
	struct ub_ctx* ctx;
	struct ub_result* dnskey;

	/* see if builtin rootanchor needs to be provided, or if
	 * rootanchor is 'revoked-trust-point' */
	if(!provide_builtin(root_anchor_file))
		return 0;

	/* make unbound context with 5011-probe for root anchor,
	 * and probe . DNSKEY */
	ctx = create_unbound_context(res_conf, root_hints, debugconf,
		ip4only, ip6only);
	add_5011_probe_root(ctx, root_anchor_file);
	dnskey = prime_root_key(ctx);
	ub_ctx_delete(ctx);
	
	/* if secure: exit */
	if(dnskey->secure && !force) {
		if(verb) printf("success: the anchor is ok\n");
		ub_resolve_free(dnskey);
		return 0;
	}
	if(force && verb) printf("debug cert update forced\n");

	/* if not (and NOERROR): check date and do certupdate */
	if((dnskey->rcode == 0 && probe_date_allows_certupdate(root_anchor_file,
		debugconf)) || force)
		return do_certupdate(root_anchor_file, root_cert_file,
			urlname, xmlname, p7sname, pemname,
			res_conf, root_hints, debugconf, ip4only, ip6only,
			dnskey);
	if(verb) printf("fail: the anchor is NOT ok and could not be fixed\n");
	ub_resolve_free(dnskey);
	return 0;
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** Main routine for checkconf */
int main(int argc, char* argv[])
{
	int c;
	char* root_anchor_file = ROOT_ANCHOR_FILE;
	char* root_cert_file = ROOT_CERT_FILE;
	char* urlname = URLNAME;
	char* xmlname = XMLNAME;
	char* p7sname = P7SNAME;
	char* pemname = PEMNAME;
	char* res_conf = NULL;
	char* root_hints = NULL;
	char* debugconf = NULL;
	int ip4only=0, ip6only=0, force=0;
	/* parse the options */
	while( (c=getopt(argc, argv, "46C:Fa:c:f:hp:r:s:u:vx:")) != -1) {
		switch(c) {
		case '4':
			ip4only = 1;
			break;
		case '6':
			ip6only = 1;
			break;
		case 'a':
			root_anchor_file = optarg;
			break;
		case 'c':
			root_cert_file = optarg;
			break;
		case 'u':
			urlname = optarg;
			break;
		case 'x':
			xmlname = optarg;
			break;
		case 's':
			p7sname = optarg;
			break;
		case 'p':
			pemname = optarg;
			break;
		case 'f':
			res_conf = optarg;
			break;
		case 'r':
			root_hints = optarg;
			break;
		case 'C':
			debugconf = optarg;
			break;
		case 'F':
			force = 1;
			break;
		case 'v':
			verb++;
			break;
		case '?':
		case 'h':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if(argc != 0)
		usage();
	return do_root_update_work(root_anchor_file, root_cert_file,
		urlname, xmlname, p7sname, pemname,
		res_conf, root_hints, debugconf, ip4only, ip6only, force);
}
