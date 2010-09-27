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
#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#endif
#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif
#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif
#include <openssl/x509.h>
#include <openssl/pem.h>

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
/** port number for https access */
#define HTTPS_PORT 443

/** verbosity for this application */
static int verb = 0;

/** list of IP addresses */
struct ip_list {
	/** next in list */
	struct ip_list* next;
	/** length of addr */
	socklen_t len;
	/** address ready to connect to */
	struct sockaddr_storage addr;
	/** has the address been used */
	int used;
};

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

/** print hex data */
static void
print_data(char* msg, char* data, int len)
{
	int i;
	printf("%s: ", msg);
	for(i=0; i<len; i++) {
		printf(" %2.2x", (unsigned char)data[i]);
	}
	printf("\n");
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
 * Get current time.
 * @param debugconf: with override time for tests
 */
static time_t
get_time_now(char* debugconf)
{
	if(debugconf) {
		FILE* in = fopen(debugconf, "r");
		char line[1024];
		if(!in) {
			if(verb) printf("%s: %s\n", debugconf, strerror(errno));
			return (time_t)time(NULL);
		}
		/* must be ^val-override-date: 1234567$ formatted */
		while(fgets(line, (int)sizeof(line), in)) {
			if(strncmp(line, "val-override-date: ", 19) == 0) {
				fclose(in);
				return (time_t)atoi(line+19);
			}
		}
		fclose(in);
	}
	return (time_t)time(NULL);
}

/** printout certificate in detail */
static void
verb_cert(char* msg, X509* x)
{
	if(verb == 0) return;
	if(verb == 1) {
		if(msg) printf("%s\n", msg);
		X509_print_ex_fp(stdout, x, 0, (unsigned long)-1
			^(X509_FLAG_NO_SUBJECT
			|X509_FLAG_NO_ISSUER|X509_FLAG_NO_VALIDITY));
		return;
	}
	if(msg) printf("%s\n", msg);
	X509_print_fp(stdout, x);
}

/** printout certificates in detail */
static void
verb_certs(char* msg, STACK_OF(X509)* sk)
{
	int i, num = sk_X509_num(sk);
	if(verb == 0) return;
	for(i=0; i<num; i++) {
		printf("%s (%d/%d)\n", msg, i, num);
		verb_cert(NULL, sk_X509_value(sk, i));
	}
}

/* write the certificate file */
static int
write_cert_file(char* file, STACK_OF(X509)* sk)
{
	FILE* out;
	int i, num = sk_X509_num(sk);
	if(file == NULL || strcmp(file, "") == 0)
		return 1;
	out = fopen(file, "w");
	if(!out) {
		if(verb) printf("write %s: %s\n", file, strerror(errno));
		return 0;
	}
	for(i=0; i<num; i++) {
		if(!PEM_write_X509(out, sk_X509_value(sk, i))) {
			if(verb) printf("could not write %s\n", file);
			fclose(out);
			return 0;
		}
	}
	fclose(out);
	return 1;
}

/** read certificates from a PEM bio */
static STACK_OF(X509)*
read_cert_bio(BIO* bio)
{
	STACK_OF(X509) *sk = sk_X509_new_null();
	if(!sk) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	while(!BIO_eof(bio)) {
		X509* x = PEM_read_bio_X509(bio, NULL, 0, NULL);
		if(x == NULL) {
			if(verb) printf("failed to read X509\n");
			continue;
		}
		if(!sk_X509_push(sk, x)) {
			if(verb) printf("out of memory\n");
			exit(0);
		}
	}
	return sk;
}

/* read the certificate file */
static STACK_OF(X509)*
read_cert_file(char* file)
{
	STACK_OF(X509)* sk;
	FILE* in;
	int content = 0;
	if(file == NULL || strcmp(file, "") == 0) {
		return NULL;
	}
	sk = sk_X509_new_null();
	if(!sk) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	in = fopen(file, "r");
	if(!in) {
		if(verb) printf("%s: %s\n", file, strerror(errno));
#ifndef S_SPLINT_S
		sk_X509_pop_free(sk, X509_free);
#endif
		return NULL;
	}
	while(!feof(in)) {
		X509* x = PEM_read_X509(in, NULL, 0, NULL);
		if(x == NULL) {
			if(verb) printf("failed to read X509\n");
			continue;
		}
		if(!sk_X509_push(sk, x)) {
			if(verb) printf("out of memory\n");
			fclose(in);
			exit(0);
		}
		content = 1;
	}
	fclose(in);
	if(!content) {
		if(verb) printf("%s is empty\n", file);
#ifndef S_SPLINT_S
		sk_X509_pop_free(sk, X509_free);
#endif
		return NULL;
	}
	return sk;
}

/** read certificates from the builtin certificate */
static STACK_OF(X509)*
read_builtin_cert(void)
{
/* The ICANN CA fetched at 24 Sep 2010.  Valid to 2028 */
	const char* builtin_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIDdzCCAl+gAwIBAgIBATANBgkqhkiG9w0BAQsFADBdMQ4wDAYDVQQKEwVJQ0FO\n"
"TjEmMCQGA1UECxMdSUNBTk4gQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxFjAUBgNV\n"
"BAMTDUlDQU5OIFJvb3QgQ0ExCzAJBgNVBAYTAlVTMB4XDTA5MTIyMzA0MTkxMloX\n"
"DTI5MTIxODA0MTkxMlowXTEOMAwGA1UEChMFSUNBTk4xJjAkBgNVBAsTHUlDQU5O\n"
"IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MRYwFAYDVQQDEw1JQ0FOTiBSb290IENB\n"
"MQswCQYDVQQGEwJVUzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKDb\n"
"cLhPNNqc1NB+u+oVvOnJESofYS9qub0/PXagmgr37pNublVThIzyLPGCJ8gPms9S\n"
"G1TaKNIsMI7d+5IgMy3WyPEOECGIcfqEIktdR1YWfJufXcMReZwU4v/AdKzdOdfg\n"
"ONiwc6r70duEr1IiqPbVm5T05l1e6D+HkAvHGnf1LtOPGs4CHQdpIUcy2kauAEy2\n"
"paKcOcHASvbTHK7TbbvHGPB+7faAztABLoneErruEcumetcNfPMIjXKdv1V1E3C7\n"
"MSJKy+jAqqQJqjZoQGB0necZgUMiUv7JK1IPQRM2CXJllcyJrm9WFxY0c1KjBO29\n"
"iIKK69fcglKcBuFShUECAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8B\n"
"Af8EBAMCAf4wHQYDVR0OBBYEFLpS6UmDJIZSL8eZzfyNa2kITcBQMA0GCSqGSIb3\n"
"DQEBCwUAA4IBAQAP8emCogqHny2UYFqywEuhLys7R9UKmYY4suzGO4nkbgfPFMfH\n"
"6M+Zj6owwxlwueZt1j/IaCayoKU3QsrYYoDRolpILh+FPwx7wseUEV8ZKpWsoDoD\n"
"2JFbLg2cfB8u/OlE4RYmcxxFSmXBg0yQ8/IoQt/bxOcEEhhiQ168H2yE5rxJMt9h\n"
"15nu5JBSewrCkYqYYmaxyOC3WrVGfHZxVI7MpIFcGdvSb2a1uyuua8l0BKgk3ujF\n"
"0/wsHNeP22qNyVO+XVBzrM8fk8BSUFuiT/6tZTYXRtEt5aKQZgXbKU5dUF3jT9qg\n"
"j/Br5BZw3X/zd325TvnswzMC1+ljLzHnQGGk\n"
"-----END CERTIFICATE-----\n"
		;
	STACK_OF(X509)* sk;
	BIO *bio = BIO_new_mem_buf((void*)builtin_cert,
		(int)strlen(builtin_cert));
	if(!bio) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	sk = read_cert_bio(bio);
	if(!sk) {
		if(verb) printf("internal error, out of memory\n");
		exit(0);
	}
	BIO_free(bio);
	return sk;
}

/** read update cert file or use builtin */
static STACK_OF(X509)*
read_cert_or_builtin(char* file, int* write_cert)
{
	STACK_OF(X509) *sk = read_cert_file(file);
	if(!sk) {
		*write_cert = 1;
		if(verb) printf("using builtin certificate\n");
		sk = read_builtin_cert();
	}
	if(verb) printf("have %d trusted certificates\n", sk_X509_num(sk));
	verb_certs("trusted certificates", sk);
	return sk;
}

/** printout IP address with message */
static void
verb_addr(char* msg, struct ip_list* ip)
{
	if(verb) {
		char out[100];
		void* a = &((struct sockaddr_in*)&ip->addr)->sin_addr;
		if(ip->len != (socklen_t)sizeof(struct sockaddr_in))
			a = &((struct sockaddr_in6*)&ip->addr)->sin6_addr;

		if(inet_ntop((int)((struct sockaddr_in*)&ip->addr)->sin_family,
			a, out, (socklen_t)sizeof(out))==0)
			printf("%s (inet_ntop error)\n", msg);
		else printf("%s %s\n", msg, out);
	}
}

/** free ip_list */
static void
ip_list_free(struct ip_list* p)
{
	struct ip_list* np;
	while(p) {
		np = p->next;
		free(p);
		p = np;
	}
}

/** create ip_list entry for a RR record */
static struct ip_list*
RR_to_ip(int tp, char* data, int len)
{
	struct ip_list* ip = (struct ip_list*)calloc(1, sizeof(*ip));
	uint16_t p = HTTPS_PORT;
	if(tp == LDNS_RR_TYPE_A) {
		struct sockaddr_in* sa = (struct sockaddr_in*)&ip->addr;
		ip->len = (socklen_t)sizeof(*sa);
		sa->sin_family = AF_INET;
		sa->sin_port = (in_port_t)htons(p);
		if(len != (int)sizeof(sa->sin_addr)) {
			if(verb) printf("skipped badly formatted A\n");
			free(ip);
			return NULL;
		}
		memmove(&sa->sin_addr, data, sizeof(sa->sin_addr));

	} else if(tp == LDNS_RR_TYPE_AAAA) {
		struct sockaddr_in6* sa = (struct sockaddr_in6*)&ip->addr;
		ip->len = (socklen_t)sizeof(*sa);
		sa->sin6_family = AF_INET6;
		sa->sin6_port = (in_port_t)htons(p);
		if(len != (int)sizeof(sa->sin6_addr)) {
			if(verb) printf("skipped badly formatted AAAA\n");
			free(ip);
			return NULL;
		}
		memmove(&sa->sin6_addr, data, sizeof(sa->sin6_addr));
	} else {
		if(verb) printf("internal error: bad type in RRtoip\n");
		free(ip);
		return NULL;
	}
	verb_addr("resolved server address", ip);
	return ip;
}

/** Resolve name, type, class and add addresses to iplist */
static void
resolve_host_ip(struct ub_ctx* ctx, char* host, int tp, int cl, struct ip_list** head)
{
	struct ub_result* res = NULL;
	int r;
	int i;

	r = ub_resolve(ctx, host, tp, cl, &res);
	if(r) {
		if(verb) printf("error: resolve %s %s: %s\n", host,
			(tp==LDNS_RR_TYPE_A)?"A":"AAAA", ub_strerror(r));
		return;
	}
	if(!res) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	for(i = 0; res->data[i]; i++) {
		struct ip_list* ip = RR_to_ip(tp, res->data[i], res->len[i]);
		if(!ip) continue;
		ip->next = *head;
		*head = ip;
	}
	ub_resolve_free(res);
}

/** parse a text IP address into a sockaddr */
static struct ip_list*
parse_ip_addr(char* str)
{
	socklen_t len = 0;
	struct sockaddr_storage* addr = NULL;
	struct sockaddr_in6 a6;
	struct sockaddr_in a;
	struct ip_list* ip;
	uint16_t p = HTTPS_PORT;
	memset(&a6, 0, sizeof(a6));
	memset(&a, 0, sizeof(a));

	if(inet_pton(AF_INET6, str, &a6.sin6_addr) > 0) {
		/* it is an IPv6 */
		a6.sin6_family = AF_INET6;
		a6.sin6_port = (in_port_t)htons(p);
		addr = (struct sockaddr_storage*)&a6;
		len = (socklen_t)sizeof(struct sockaddr_in6);
	}
	if(inet_pton(AF_INET, str, &a.sin_addr) > 0) {
		/* it is an IPv4 */
		a.sin_family = AF_INET;
		a.sin_port = (in_port_t)htons(p);
		addr = (struct sockaddr_storage*)&a;
		len = (socklen_t)sizeof(struct sockaddr_in);
	}
	if(!len) return NULL;
	ip = (struct ip_list*)calloc(1, sizeof(*ip));
	if(!ip) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	ip->len = len;
	memmove(&ip->addr, addr, len);
	if(verb) printf("server address is %s\n", str);
	return ip;
}

/**
 * Resolve a domain name (even though the resolver is down and there is
 * no trust anchor).  Without DNSSEC validation.
 */
static struct ip_list*
resolve_name(char* host, char* res_conf, char* root_hints, char* debugconf,
	int ip4only, int ip6only)
{
	struct ub_ctx* ctx;
	struct ip_list* list = NULL;
	/* first see if name is an IP address itself */
	if( (list=parse_ip_addr(host)) ) {
		return list;
	}
	
	/* create resolver context */
	ctx = create_unbound_context(res_conf, root_hints, debugconf,
        	ip4only, ip6only);

	/* try resolution of A */
	if(!ip6only) {
		resolve_host_ip(ctx, host, LDNS_RR_TYPE_A,
			LDNS_RR_CLASS_IN, &list);
	}

	/* try resolution of AAAA */
	if(!ip4only) {
		resolve_host_ip(ctx, host, LDNS_RR_TYPE_AAAA,
			LDNS_RR_CLASS_IN, &list);
	}

	ub_ctx_delete(ctx);
	if(!list) {
		if(verb) printf("%s has no IP addresses I can use\n", host);
		exit(0);
	}
	return list;
}

/** clear used flags */
static void
wipe_ip_usage(struct ip_list* p)
{
	while(p) {
		p->used = 0;
		p = p->next;
	}
}

/** cound unused IPs */
static int
count_unused(struct ip_list* p)
{
	int num = 0;
	while(p) {
		if(!p->used) num++;
		p = p->next;
	}
	return num;
}

/** pick random unused element from IP list */
static struct ip_list*
pick_random_ip(struct ip_list* list)
{
	struct ip_list* p = list;
	int num = count_unused(list);
	int sel;
	if(num == 0) return NULL;
	/* not perfect, but random enough */
	sel = (int)ldns_get_random() % num;
	while(sel > 0 && p) {
		if(!p->used) sel--;
		p = p->next;
	}
	while(p && p->used)
		p = p->next;
	if(!p) return NULL; /* robustness */
	return p;
}

/** close the fd */
static void
fd_close(int fd)
{
#ifndef USE_WINSOCK
	close(fd);
#else
	closesocket(fd);
#endif
}

/** connect to IP address */
static int
connect_to_ip(struct ip_list* ip)
{
	int fd;
	verb_addr("connect to", ip);
	fd = socket(ip->len==(socklen_t)sizeof(struct sockaddr_in)?
		AF_INET:AF_INET6, SOCK_STREAM, 0);
	if(fd == -1) {
		if(verb) printf("socket: %s\n", strerror(errno));
		return -1;
	}
	if(connect(fd, (struct sockaddr*)&ip->addr, ip->len) < 0) {
		if(verb) printf("connect: %s\n", strerror(errno));
		fd_close(fd);
		return -1;
	}
	return fd;
}

/** create SSL context */
static SSL_CTX*
setup_sslctx(void)
{
	SSL_CTX* sslctx = SSL_CTX_new(SSLv23_client_method());
	if(!sslctx) {
		if(verb) printf("SSL_CTX_new error\n");
		return NULL;
	}
	return sslctx;
}

/** initiate TLS on a connection */
static SSL*
TLS_initiate(SSL_CTX* sslctx, int fd)
{
	X509* x;
	int r;
	SSL* ssl = SSL_new(sslctx);
	if(!ssl) {
		if(verb) printf("SSL_new error\n");
		return NULL;
	}
	SSL_set_connect_state(ssl);
	(void)SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	if(!SSL_set_fd(ssl, fd)) {
		if(verb) printf("SSL_set_fd error\n");
		SSL_free(ssl);
		return NULL;
	}
	while(1) {
		ERR_clear_error();
		if( (r=SSL_do_handshake(ssl)) == 1)
			break;
		r = SSL_get_error(ssl, r);
		if(r != SSL_ERROR_WANT_READ && r != SSL_ERROR_WANT_WRITE) {
			if(verb) printf("SSL handshake failed\n");
			SSL_free(ssl);
			return NULL;
		}
		/* wants to be called again */
	}
	x = SSL_get_peer_certificate(ssl);
	if(!x) {
		if(verb) printf("Server presented no peer certificate\n");
		SSL_free(ssl);
		return NULL;
	}
	verb_cert("server SSL certificate", x);
	X509_free(x);
	return ssl;
}

/** perform neat TLS shutdown */
static void
TLS_shutdown(int fd, SSL* ssl, SSL_CTX* sslctx)
{
	/* shutdown the SSL connection nicely */
	if(SSL_shutdown(ssl) == 0) {
		SSL_shutdown(ssl);
	}
	SSL_free(ssl);
	SSL_CTX_free(sslctx);
	fd_close(fd);
}

/** write a line over SSL */
static int
write_ssl_line(SSL* ssl, char* str, char* sec)
{
	char buf[1024];
	size_t l;
	if(sec) {
		snprintf(buf, sizeof(buf), str, sec);
	} else {
		snprintf(buf, sizeof(buf), "%s", str);
	}
	l = strlen(buf);
	if(l+2 >= sizeof(buf)) {
		if(verb) printf("line too long\n");
		return 0;
	}
	if(verb >= 2) printf("SSL_write: %s\n", buf);
	buf[l] = '\r';
	buf[l+1] = '\n';
	buf[l+2] = 0;
	/* add \r\n */
	if(SSL_write(ssl, buf, (int)strlen(buf)) <= 0) {
		if(verb) printf("could not SSL_write %s", str);
		return 0;
	}
	return 1;
}

static int
process_one_header(char* buf, size_t* clen, int* chunked)
{
	if(verb>=2) printf("header: '%s'\n", buf);
	if(strncasecmp(buf, "HTTP/1.1 ", 9) == 0) {
		/* check returncode */
		if(buf[9] != '2') {
			if(verb) printf("bad status %s\n", buf+9);
			return 0;
		}
	} else if(strncasecmp(buf, "Content-Length: ", 16) == 0) {
		if(!*chunked)
			*clen = (size_t)atoi(buf+16);
	} else if(strncasecmp(buf, "Transfer-Encoding: chunked", 19+7) == 0) {
		*clen = 0;
		*chunked = 1;
	}
	return 1;
}

/** read one file from SSL
 * zero terminates.
 * skips \r\n (but not copied to buf).
 */
static int
read_ssl_line(SSL* ssl, char* buf, size_t len)
{
	size_t n = 0;
	int r;
	int endnl = 0;
	while(1) {
		if(n >= len) {
			if(verb) printf("line too long\n");
			return 0;
		}
		if((r = SSL_read(ssl, buf+n, 1)) <= 0) {
			if(SSL_get_error(ssl, r) == SSL_ERROR_ZERO_RETURN) {
				/* EOF */
				break;
			}
			if(verb) printf("could not SSL_read\n");
			return 0;
		}
		if(endnl && buf[n] == '\n') {
			break;
		} else if(endnl) {
			/* bad data */
			if(verb) printf("error: stray linefeeds\n");
			return 0;
		} else if(buf[n] == '\r') {
			/* skip \r, and also \n on the wire */
			endnl = 1;
			continue;
		} else if(buf[n] == '\n') {
			/* skip the \n, we are done */
			break;
		} else n++;
	}
	buf[n] = 0;
	return 1;
}

/** read http headers and process them */
static size_t
read_http_headers(SSL* ssl, size_t* clen)
{
	char buf[1024];
	int chunked = 0;
	*clen = 0;
	while(read_ssl_line(ssl, buf, sizeof(buf))) {
		if(buf[0] == 0)
			return 1;
		if(!process_one_header(buf, clen, &chunked))
			return 0;
	}
	return 0;
}

/** read a data chunk */
static char*
read_data_chunk(SSL* ssl, size_t len)
{
	size_t got = 0;
	int r;
	char* data = malloc(len+1);
	if(!data) {
		if(verb) printf("out of memory\n");
		return NULL;
	}
	while(got < len) {
		if((r = SSL_read(ssl, data+got, (int)(len-got))) <= 0) {
			if(SSL_get_error(ssl, r) == SSL_ERROR_ZERO_RETURN) {
				/* EOF */
				if(verb) printf("could not SSL_read: unexpected EOF\n");
				free(data);
				return NULL;
			}
			if(verb) printf("could not SSL_read\n");
			free(data);
			return NULL;
		}
		if(verb >= 2) printf("at %d/%d\n", (int)got, (int)len);
		got += r;
	}
	if(verb>=2) printf("read %d data\n", (int)len);
	data[len] = 0;
	return data;
}

/** parse chunk header */
static int
parse_chunk_header(char* buf, size_t* result)
{
	char* e = NULL;
	size_t v = (size_t)strtol(buf, &e, 16);
	if(e == buf)
		return 0;
	*result = v;
	return 1;
}

/** read chunked data from connection */
static BIO*
do_chunked_read(SSL* ssl)
{
	char buf[1024];
	size_t len;
	char* body;
	BIO* mem = BIO_new(BIO_s_mem());
	if(!mem) {
		if(verb) printf("out of memory\n");
		return NULL;
	}
	while(read_ssl_line(ssl, buf, sizeof(buf))) {
		/* read the chunked start line */
		if(verb>=2) printf("chunk header: %s\n", buf);
		if(!parse_chunk_header(buf, &len)) {
			BIO_free(mem);
			return NULL;
		}
		if(verb>=2) printf("chunk len: %d\n", (int)len);
		if(len == 0) {
			char z = 0;
			/* skip end-of-chunk-trailer lines,
			 * until the empty line after that */
			do {
				if(!read_ssl_line(ssl, buf, sizeof(buf))) {
					BIO_free(mem);
					return NULL;
				}
			} while (strlen(buf) > 0);
			/* end of chunks, zero terminate it */
			if(BIO_write(mem, &z, 1) <= 0) {
				if(verb) printf("out of memory\n");
				BIO_free(mem);
				return NULL;
			}
			return mem;
		}
		/* read the chunked body */
		body = read_data_chunk(ssl, len);
		if(!body) {
			BIO_free(mem);
			return NULL;
		}
		if(BIO_write(mem, body, (int)len) <= 0) {
			if(verb) printf("out of memory\n");
			free(body);
			BIO_free(mem);
			return NULL;
		}
		free(body);
		/* skip empty line after data chunk */
		if(!read_ssl_line(ssl, buf, sizeof(buf))) {
			BIO_free(mem);
			return NULL;
		}
	}
	BIO_free(mem);
	return NULL;
}

/** start HTTP1.1 transaction on SSL */
static int
write_http_get(SSL* ssl, char* pathname, char* urlname)
{
	if(write_ssl_line(ssl, "GET %s HTTP/1.1", pathname) &&
	   write_ssl_line(ssl, "Host: %s", urlname) &&
	   write_ssl_line(ssl, "User-Agent: unbound-anchor/%s",
	   	PACKAGE_VERSION) &&
	   /* We do not really do multiple queries per connection,
	    * but this header setting is also not needed.
	    * write_ssl_line(ssl, "Connection: close", NULL) &&*/
	   write_ssl_line(ssl, "", NULL)) {
		return 1;
	}
	return 0;
}

/** read HTTP result from SSL */
static BIO*
read_http_result(SSL* ssl)
{
	size_t len = 0;
	char* data;
	BIO* m;
	if(!read_http_headers(ssl, &len)) {
		return NULL;
	}
	if(len == 0) {
		/* do the chunked version */
		BIO* tmp = do_chunked_read(ssl);
		char* d = NULL;
		long l;
		l = BIO_get_mem_data(tmp, &d);
		if(verb>=2) printf("chunked data is %d\n", (int)l);
		if(l == 0 || d == NULL) {
			if(verb) printf("out of memory\n");
			return NULL;
		}
		/* the result is zero terminated for robustness, but we 
		 * do not include that in the BIO len (for binary data) */
		len = (size_t)l-1;
		data = strdup(d);
		if(data == NULL) {
			if(verb) printf("out of memory\n");
			return NULL;
		}
		BIO_free(tmp);
	} else {
		data = read_data_chunk(ssl, len);
	}
	if(verb >= 4) print_data("read data", data, (int)len);
	m = BIO_new_mem_buf(data, (int)len);
	if(!m) {
		if(verb) printf("out of memory\n");
		exit(0);
	}
	return m;
}

/** https to an IP addr, return BIO with pathname or NULL */
static BIO*
https_to_ip(struct ip_list* ip, char* pathname, char* urlname)
{
	int fd;
	SSL* ssl;
	BIO* bio;
	SSL_CTX* sslctx = setup_sslctx();
	if(!sslctx) {
		return NULL;
	}
	fd = connect_to_ip(ip);
	if(fd == -1) {
		SSL_CTX_free(sslctx);
		return NULL;
	}
	ssl = TLS_initiate(sslctx, fd);
	if(!ssl) {
		SSL_CTX_free(sslctx);
		fd_close(fd);
		return NULL;
	}
	if(!write_http_get(ssl, pathname, urlname)) {
		if(verb) printf("could not write to server\n");
		SSL_free(ssl);
		SSL_CTX_free(sslctx);
		fd_close(fd);
		return NULL;
	}
	bio = read_http_result(ssl);
	TLS_shutdown(fd, ssl, sslctx);
	return bio;
}

/**
 * Do a HTTPS, HTTP1.1 over TLS, to fetch a file
 * @param ip_list: list of IP addresses to use to fetch from.
 * @param pathname: pathname of file on server to GET.
 * @return a memory BIO with the file in it.
 */
static BIO*
https(struct ip_list* ip_list, char* pathname, char* urlname)
{
	struct ip_list* ip;
	BIO* bio = NULL;
	/* try random address first, and work through the list */
	wipe_ip_usage(ip_list);
	while( (ip = pick_random_ip(ip_list)) ) {
		ip->used = 1;
		bio = https_to_ip(ip, pathname, urlname);
		if(bio) break;
	}
	if(!bio) {
		if(verb) printf("could not fetch %s\n", pathname);
		exit(0);
	} else {
		if(verb) printf("fetched %s (%d bytes)\n",
			pathname, (int)BIO_ctrl_pending(bio));
	}
	return bio;
}

/** free up a downloaded file BIO */
static void
free_file_bio(BIO* bio)
{
	char* pp = NULL;
	BIO_reset(bio);
	(void)BIO_get_mem_data(bio, &pp);
	free(pp);
	BIO_free(bio);
}

/** verify a PKCS7 signature, false on failure */
static int
verify_p7sig(BIO* data, BIO* p7s, BIO* pem, STACK_OF(X509)* trust, time_t now)
{
	X509_VERIFY_PARAM* param = X509_VERIFY_PARAM_new();
	PKCS7* p7;
	X509_STORE *store = X509_STORE_new();
	STACK_OF(X509) *certs;
	int secure = 0;
	int i;

	BIO_reset(p7s);
	BIO_reset(data);

	if(!param || !store) {
		if(verb) printf("out of memory\n");
		X509_VERIFY_PARAM_free(param);
		X509_STORE_free(store);
		return 0;
	}

	/* convert p7s to p7 (the signature) */
	p7 = d2i_PKCS7_bio(p7s, NULL);
	if(!p7) {
		if(verb) printf("could not parse p7s signature file\n");
		X509_VERIFY_PARAM_free(param);
		X509_STORE_free(store);
		return 0;
	}
	if(verb >= 2) printf("parsed the PKCS7 signature\n");

	/* convert trust to trusted certificate store */
	/* set current time */
	X509_VERIFY_PARAM_set_time(param, now);
	/* do the selfcheck on the root certificate */
	X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_CHECK_SS_SIGNATURE);
	X509_STORE_set1_param(store, param);
	for(i=0; i<sk_X509_num(trust); i++) {
		if(!X509_STORE_add_cert(store, sk_X509_value(trust, i))) {
			if(verb) printf("failed X509_STORE_add_cert\n");
			X509_STORE_free(store);
			PKCS7_free(p7);
			return 0;
		}
	}
	if(verb >= 2) printf("setup the X509_STORE\n");

	/* convert pem to (intermediate) certs */
	certs = read_cert_bio(pem);
	if(verb >= 2) printf("read the intermediate certificates\n");

	if(PKCS7_verify(p7, certs, store, data, NULL, 0) == 1) {
		secure = 1;
		if(verb >= 2) printf("the PKCS7 signature verified\n");
	}

#ifndef S_SPLINT_S
	sk_X509_pop_free(certs, X509_free);
#endif
	X509_STORE_free(store);
	PKCS7_free(p7);
	return secure;
}

/** Perform the verification and update of the trustanchor file */
static void
verify_and_update_anchor(char* root_anchor_file, char* debugconf,
	struct ub_result* dnskey, BIO* xml, BIO* p7s, BIO* pem,
	STACK_OF(X509)* cert)
{
	time_t now = get_time_now(debugconf);

	/* verify xml file */
	if(!verify_p7sig(xml, p7s, pem, cert, now)) {
		printf("the PKCS7 signature failed\n");
		exit(0);
	}

	/* see if xml file verifies the dnskey that was probed */

	/* reinstate 5011 tracking */

}

/** perform actual certupdate work */
static int
do_certupdate(char* root_anchor_file, char* root_cert_file,
	char* urlname, char* xmlname, char* p7sname, char* pemname,
	char* res_conf, char* root_hints, char* debugconf,
	int ip4only, int ip6only, struct ub_result* dnskey)
{
	int write_cert = 0;
	STACK_OF(X509)* cert;
	BIO *pem, *xml, *p7s;
	struct ip_list* ip_list = NULL;
	/* read pem file or provide builtin */
	cert = read_cert_or_builtin(root_cert_file, &write_cert);

	/* lookup A, AAAA for the urlname (or parse urlname if IP address) */
	ip_list = resolve_name(urlname, res_conf, root_hints, debugconf,
		ip4only, ip6only);

	/* fetch the necessary files over HTTPS */
	xml = https(ip_list, xmlname, urlname);
	p7s = https(ip_list, p7sname, urlname);
	pem = https(ip_list, pemname, urlname);

	/* update the pem file (optional) */
	/*do_pem_update(cert, pem, &write_cert);*/
	if(write_cert) {
		(void)write_cert_file(root_cert_file, cert);
		if(verb) printf("wrote cert to %s\n", root_cert_file);
	}

	/* verify and update the root anchor */
	verify_and_update_anchor(root_anchor_file, debugconf, dnskey,
		xml, p7s, pem, cert);
	if(verb) printf("success: the anchor has been updated "
			"using the cert\n");

	free_file_bio(xml);
	free_file_bio(p7s);
	free_file_bio(pem);
#ifndef S_SPLINT_S
	sk_X509_pop_free(cert, X509_free);
#endif
	ub_resolve_free(dnskey);
	ip_list_free(ip_list);
	return 1;
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
provide_builtin(char* root_anchor_file, int* used_builtin)
{
	/* try to read it */
	switch(try_read_anchor(root_anchor_file))
	{
		case 0: /* no exist or empty */
			write_builtin_anchor(root_anchor_file);
			*used_builtin = 1;
			break;
		case 1: /* revoked tp */
			return 0;	
		case 2: /* it is fine */
		default:
			break;
	}
	return 1;
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
	int32_t now = (int32_t)get_time_now(debugconf);
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
	int used_builtin = 0;

	/* see if builtin rootanchor needs to be provided, or if
	 * rootanchor is 'revoked-trust-point' */
	if(!provide_builtin(root_anchor_file, &used_builtin))
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
		return used_builtin;
	}
	if(force && verb) printf("debug cert update forced\n");

	/* if not (and NOERROR): check date and do certupdate */
	if((dnskey->rcode == 0 && probe_date_allows_certupdate(root_anchor_file,
		debugconf)) || force) {
		if(do_certupdate(root_anchor_file, root_cert_file,
			urlname, xmlname, p7sname, pemname,
			res_conf, root_hints, debugconf, ip4only, ip6only,
			dnskey))
			return 1;
		return used_builtin;
	}
	if(verb) printf("fail: the anchor is NOT ok and could not be fixed\n");
	ub_resolve_free(dnskey);
	return used_builtin;
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
	ERR_load_crypto_strings();
	ERR_load_SSL_strings();
	OpenSSL_add_all_algorithms();
	(void)SSL_library_init();
	return do_root_update_work(root_anchor_file, root_cert_file,
		urlname, xmlname, p7sname, pemname,
		res_conf, root_hints, debugconf, ip4only, ip6only, force);
}
