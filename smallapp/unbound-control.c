/*
 * checkconf/unbound-control.c - remote control utility for unbound.
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
 * The remote control utility contacts the unbound server over ssl and
 * sends the command, receives the answer, and displays the result
 * from the commandline.
 */

#include "config.h"
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
#include "util/log.h"
#include "util/config_file.h"
#include "util/locks.h"
#include "util/net_help.h"
#include "util/shm_side/shm_main.h"
#include "daemon/stats.h"
#include "sldns/wire2str.h"
#include "sldns/pkthdr.h"
#include "services/rpz.h"

#ifdef HAVE_SYS_IPC_H
#include "sys/ipc.h"
#endif
#ifdef HAVE_SYS_SHM_H
#include "sys/shm.h"
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_TARGETCONDITIONALS_H
#include <TargetConditionals.h>
#endif

static void usage(void) ATTR_NORETURN;
static void ssl_err(const char* s) ATTR_NORETURN;
static void ssl_path_err(const char* s, const char *path) ATTR_NORETURN;

/** timeout to wait for connection over stream, in msec */
#define UNBOUND_CONTROL_CONNECT_TIMEOUT 5000

#include "command-help.c"

/** Give unbound-control usage, and exit (1). */
static void
usage(void)
{
	printf("Usage:	unbound-control [options] command\n"
	       "    Remote control utility for unbound server.\n"
	       "    Appending a view name to a command operates\n"
	       "    on the specified view.\n");
	printf("Options:\n"
	       "  -c file      config file, default is " CONFIGFILE "\n"
	       "  -s ip[@port] server address, if omitted config is used.\n"
	       "  -q           quiet (don't print anything if it works ok).\n"
	       "  -h           show this usage help.\n");
	printf("Commands:\n");
	helpList();
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
	exit(1);
}

#ifdef HAVE_SHMGET
/** what to put on statistics lines between var and value, ": " or "=" */
#define SQ "="
/** if true, inhibits a lot of =0 lines from the stats output */
static const int inhibit_zero = 1;
/** divide sum of timers to get average */
static void
timeval_divide(struct timeval* avg, const struct timeval* sum, long long d)
{
#ifndef S_SPLINT_S
	size_t leftover;
	if(d == 0) {
		avg->tv_sec = 0;
		avg->tv_usec = 0;
		return;
	}
	avg->tv_sec = sum->tv_sec / d;
	avg->tv_usec = sum->tv_usec / d;
	/* handle fraction from seconds divide */
	leftover = sum->tv_sec - avg->tv_sec*d;
	avg->tv_usec += (leftover*1000000)/d;
#endif
}

/** print unsigned long stats value */
#define PR_UL_NM(str, var) printf("%s."str SQ"%lu\n", nm, (unsigned long)(var));
#define PR_UL(str, var) printf(str SQ"%lu\n", (unsigned long)(var));
#define PR_UL_SUB(str, nm, var) printf(str".%s"SQ"%lu\n", nm, (unsigned long)(var));
#define PR_TIMEVAL(str, var) printf(str SQ ARG_LL "d.%6.6d\n", \
	(long long)var.tv_sec, (int)var.tv_usec);
#define PR_STATSTIME(str, var) printf(str SQ ARG_LL "d.%6.6d\n", \
	(long long)var ## _sec, (int)var ## _usec);
#define PR_LL(str, var) printf(str SQ ARG_LL"d\n", (long long)(var));

/** print stat block */
static void pr_stats(const char* nm, struct ub_stats_info* s)
{
	struct timeval sumwait, avg;
	PR_UL_NM("num.queries", s->svr.num_queries);
	PR_UL_NM("num.queries_ip_ratelimited", 
		s->svr.num_queries_ip_ratelimited);
	PR_UL_NM("num.cachehits",
		s->svr.num_queries - s->svr.num_queries_missed_cache);
	PR_UL_NM("num.cachemiss", s->svr.num_queries_missed_cache);
	PR_UL_NM("num.prefetch", s->svr.num_queries_prefetch);
	PR_UL_NM("num.expired", s->svr.ans_expired);
	PR_UL_NM("num.recursivereplies", s->mesh_replies_sent);
#ifdef USE_DNSCRYPT
    PR_UL_NM("num.dnscrypt.crypted", s->svr.num_query_dnscrypt_crypted);
    PR_UL_NM("num.dnscrypt.cert", s->svr.num_query_dnscrypt_cert);
    PR_UL_NM("num.dnscrypt.cleartext", s->svr.num_query_dnscrypt_cleartext);
    PR_UL_NM("num.dnscrypt.malformed",
             s->svr.num_query_dnscrypt_crypted_malformed);
#endif /* USE_DNSCRYPT */
	printf("%s.requestlist.avg"SQ"%g\n", nm,
		(s->svr.num_queries_missed_cache+s->svr.num_queries_prefetch)?
			(double)s->svr.sum_query_list_size/
			(double)(s->svr.num_queries_missed_cache+
			s->svr.num_queries_prefetch) : 0.0);
	PR_UL_NM("requestlist.max", s->svr.max_query_list_size);
	PR_UL_NM("requestlist.overwritten", s->mesh_jostled);
	PR_UL_NM("requestlist.exceeded", s->mesh_dropped);
	PR_UL_NM("requestlist.current.all", s->mesh_num_states);
	PR_UL_NM("requestlist.current.user", s->mesh_num_reply_states);
#ifndef S_SPLINT_S
	sumwait.tv_sec = s->mesh_replies_sum_wait_sec;
	sumwait.tv_usec = s->mesh_replies_sum_wait_usec;
#endif
	timeval_divide(&avg, &sumwait, s->mesh_replies_sent);
	printf("%s.", nm);
	PR_TIMEVAL("recursion.time.avg", avg);
	printf("%s.recursion.time.median"SQ"%g\n", nm, s->mesh_time_median);
	PR_UL_NM("tcpusage", s->svr.tcp_accept_usage);
}

/** print uptime */
static void print_uptime(struct ub_shm_stat_info* shm_stat)
{
	PR_STATSTIME("time.now", shm_stat->time.now);
	PR_STATSTIME("time.up", shm_stat->time.up);
	PR_STATSTIME("time.elapsed", shm_stat->time.elapsed);
}

/** print memory usage */
static void print_mem(struct ub_shm_stat_info* shm_stat,
	struct ub_stats_info* s)
{
	PR_LL("mem.cache.rrset", shm_stat->mem.rrset);
	PR_LL("mem.cache.message", shm_stat->mem.msg);
	PR_LL("mem.mod.iterator", shm_stat->mem.iter);
	PR_LL("mem.mod.validator", shm_stat->mem.val);
	PR_LL("mem.mod.respip", shm_stat->mem.respip);
#ifdef CLIENT_SUBNET
	PR_LL("mem.mod.subnet", shm_stat->mem.subnet);
#endif
#ifdef USE_IPSECMOD
	PR_LL("mem.mod.ipsecmod", shm_stat->mem.ipsecmod);
#endif
#ifdef WITH_DYNLIBMODULE
	PR_LL("mem.mod.dynlib", shm_stat->mem.dynlib);
#endif
#ifdef USE_DNSCRYPT
	PR_LL("mem.cache.dnscrypt_shared_secret",
		shm_stat->mem.dnscrypt_shared_secret);
	PR_LL("mem.cache.dnscrypt_nonce",
		shm_stat->mem.dnscrypt_nonce);
#endif
	PR_LL("mem.streamwait", s->svr.mem_stream_wait);
	PR_LL("mem.http.query_buffer", s->svr.mem_http2_query_buffer);
	PR_LL("mem.http.response_buffer", s->svr.mem_http2_response_buffer);
}

/** print histogram */
static void print_hist(struct ub_stats_info* s)
{
	struct timehist* hist;
	size_t i;
	hist = timehist_setup();
	if(!hist)
		fatal_exit("out of memory");
	timehist_import(hist, s->svr.hist, NUM_BUCKETS_HIST);
	for(i=0; i<hist->num; i++) {
		printf("histogram.%6.6d.%6.6d.to.%6.6d.%6.6d=%lu\n",
			(int)hist->buckets[i].lower.tv_sec,
			(int)hist->buckets[i].lower.tv_usec,
			(int)hist->buckets[i].upper.tv_sec,
			(int)hist->buckets[i].upper.tv_usec,
			(unsigned long)hist->buckets[i].count);
	}
	timehist_delete(hist);
}

/** print extended */
static void print_extended(struct ub_stats_info* s)
{
	int i;
	char nm[16];

	/* TYPE */
	for(i=0; i<UB_STATS_QTYPE_NUM; i++) {
		if(inhibit_zero && s->svr.qtype[i] == 0)
			continue;
		sldns_wire2str_type_buf((uint16_t)i, nm, sizeof(nm));
		PR_UL_SUB("num.query.type", nm, s->svr.qtype[i]);
	}
	if(!inhibit_zero || s->svr.qtype_big) {
		PR_UL("num.query.type.other", s->svr.qtype_big);
	}

	/* CLASS */
	for(i=0; i<UB_STATS_QCLASS_NUM; i++) {
		if(inhibit_zero && s->svr.qclass[i] == 0)
			continue;
		sldns_wire2str_class_buf((uint16_t)i, nm, sizeof(nm));
		PR_UL_SUB("num.query.class", nm, s->svr.qclass[i]);
	}
	if(!inhibit_zero || s->svr.qclass_big) {
		PR_UL("num.query.class.other", s->svr.qclass_big);
	}

	/* OPCODE */
	for(i=0; i<UB_STATS_OPCODE_NUM; i++) {
		if(inhibit_zero && s->svr.qopcode[i] == 0)
			continue;
		sldns_wire2str_opcode_buf(i, nm, sizeof(nm));
		PR_UL_SUB("num.query.opcode", nm, s->svr.qopcode[i]);
	}

	/* transport */
	PR_UL("num.query.tcp", s->svr.qtcp);
	PR_UL("num.query.tcpout", s->svr.qtcp_outgoing);
	PR_UL("num.query.tls", s->svr.qtls);
	PR_UL("num.query.tls_resume", s->svr.qtls_resume);
	PR_UL("num.query.ipv6", s->svr.qipv6);
	PR_UL("num.query.https", s->svr.qhttps);

	/* flags */
	PR_UL("num.query.flags.QR", s->svr.qbit_QR);
	PR_UL("num.query.flags.AA", s->svr.qbit_AA);
	PR_UL("num.query.flags.TC", s->svr.qbit_TC);
	PR_UL("num.query.flags.RD", s->svr.qbit_RD);
	PR_UL("num.query.flags.RA", s->svr.qbit_RA);
	PR_UL("num.query.flags.Z", s->svr.qbit_Z);
	PR_UL("num.query.flags.AD", s->svr.qbit_AD);
	PR_UL("num.query.flags.CD", s->svr.qbit_CD);
	PR_UL("num.query.edns.present", s->svr.qEDNS);
	PR_UL("num.query.edns.DO", s->svr.qEDNS_DO);

	/* RCODE */
	for(i=0; i<UB_STATS_RCODE_NUM; i++) {
		/* Always include RCODEs 0-5 */
		if(inhibit_zero && i > LDNS_RCODE_REFUSED && s->svr.ans_rcode[i] == 0)
			continue;
		sldns_wire2str_rcode_buf(i, nm, sizeof(nm));
		PR_UL_SUB("num.answer.rcode", nm, s->svr.ans_rcode[i]);
	}
	if(!inhibit_zero || s->svr.ans_rcode_nodata) {
		PR_UL("num.answer.rcode.nodata", s->svr.ans_rcode_nodata);
	}
	/* iteration */
	PR_UL("num.query.ratelimited", s->svr.queries_ratelimited);
	/* validation */
	PR_UL("num.answer.secure", s->svr.ans_secure);
	PR_UL("num.answer.bogus", s->svr.ans_bogus);
	PR_UL("num.rrset.bogus", s->svr.rrset_bogus);
	PR_UL("num.query.aggressive.NOERROR", s->svr.num_neg_cache_noerror);
	PR_UL("num.query.aggressive.NXDOMAIN", s->svr.num_neg_cache_nxdomain);
	/* threat detection */
	PR_UL("unwanted.queries", s->svr.unwanted_queries);
	PR_UL("unwanted.replies", s->svr.unwanted_replies);
	/* cache counts */
	PR_UL("msg.cache.count", s->svr.msg_cache_count);
	PR_UL("rrset.cache.count", s->svr.rrset_cache_count);
	PR_UL("infra.cache.count", s->svr.infra_cache_count);
	PR_UL("key.cache.count", s->svr.key_cache_count);
	/* applied RPZ actions */
	for(i=0; i<UB_STATS_RPZ_ACTION_NUM; i++) {
		if(i == RPZ_NO_OVERRIDE_ACTION)
			continue;
		if(inhibit_zero && s->svr.rpz_action[i] == 0)
			continue;
		PR_UL_SUB("num.rpz.action", rpz_action_to_string(i), s->svr.rpz_action[i]);
	}
#ifdef USE_DNSCRYPT
	PR_UL("dnscrypt_shared_secret.cache.count",
			 s->svr.shared_secret_cache_count);
	PR_UL("num.query.dnscrypt.shared_secret.cachemiss",
			 s->svr.num_query_dnscrypt_secret_missed_cache);
	PR_UL("dnscrypt_nonce.cache.count", s->svr.nonce_cache_count);
	PR_UL("num.query.dnscrypt.replay",
			 s->svr.num_query_dnscrypt_replay);
#endif /* USE_DNSCRYPT */
	PR_UL("num.query.authzone.up", s->svr.num_query_authzone_up);
	PR_UL("num.query.authzone.down", s->svr.num_query_authzone_down);
#ifdef CLIENT_SUBNET
	PR_UL("num.query.subnet", s->svr.num_query_subnet);
	PR_UL("num.query.subnet_cache", s->svr.num_query_subnet_cache);
#endif
}

/** print statistics out of memory structures */
static void do_stats_shm(struct config_file* cfg, struct ub_stats_info* stats,
	struct ub_shm_stat_info* shm_stat)
{
	int i;
	char nm[32];
	for(i=0; i<cfg->num_threads; i++) {
		snprintf(nm, sizeof(nm), "thread%d", i);
		pr_stats(nm, &stats[i+1]);
	}
	pr_stats("total", &stats[0]);
	print_uptime(shm_stat);
	if(cfg->stat_extended) {
		print_mem(shm_stat, &stats[0]);
		print_hist(stats);
		print_extended(stats);
	}
}
#endif /* HAVE_SHMGET */

/** print statistics from shm memory segment */
static void print_stats_shm(const char* cfgfile)
{
#ifdef HAVE_SHMGET
	struct config_file* cfg;
	struct ub_stats_info* stats;
	struct ub_shm_stat_info* shm_stat;
	int id_ctl, id_arr;
	/* read config */
	if(!(cfg = config_create()))
		fatal_exit("out of memory");
	if(!config_read(cfg, cfgfile, NULL))
		fatal_exit("could not read config file");
	/* get shm segments */
	id_ctl = shmget(cfg->shm_key, sizeof(int), SHM_R);
	if(id_ctl == -1) {
		fatal_exit("shmget(%d): %s", cfg->shm_key, strerror(errno));
	}
	id_arr = shmget(cfg->shm_key+1, sizeof(int), SHM_R);
	if(id_arr == -1) {
		fatal_exit("shmget(%d): %s", cfg->shm_key+1, strerror(errno));
	}
	shm_stat = (struct ub_shm_stat_info*)shmat(id_ctl, NULL, SHM_RDONLY);
	if(shm_stat == (void*)-1) {
		fatal_exit("shmat(%d): %s", id_ctl, strerror(errno));
	}
	stats = (struct ub_stats_info*)shmat(id_arr, NULL, SHM_RDONLY);
	if(stats == (void*)-1) {
		fatal_exit("shmat(%d): %s", id_arr, strerror(errno));
	}

	/* print the stats */
	do_stats_shm(cfg, stats, shm_stat);

	/* shutdown */
	shmdt(shm_stat);
	shmdt(stats);
	config_delete(cfg);
#else
	(void)cfgfile;
#endif /* HAVE_SHMGET */
}

/** exit with ssl error */
static void ssl_err(const char* s)
{
	fprintf(stderr, "error: %s\n", s);
	ERR_print_errors_fp(stderr);
	exit(1);
}

/** exit with ssl error related to a file path */
static void ssl_path_err(const char* s, const char *path)
{
	unsigned long err;
	err = ERR_peek_error();
	if (ERR_GET_LIB(err) == ERR_LIB_SYS &&
		(ERR_GET_FUNC(err) == SYS_F_FOPEN ||
		 ERR_GET_FUNC(err) == SYS_F_FREAD) ) {
		fprintf(stderr, "error: %s\n%s: %s\n",
			s, path, ERR_reason_error_string(err));
		exit(1);
	} else {
		ssl_err(s);
	}
}

/** setup SSL context */
static SSL_CTX*
setup_ctx(struct config_file* cfg)
{
	char* s_cert=NULL, *c_key=NULL, *c_cert=NULL;
	SSL_CTX* ctx;

	if(!(options_remote_is_address(cfg) && cfg->control_use_cert))
		return NULL;
	s_cert = fname_after_chroot(cfg->server_cert_file, cfg, 1);
	c_key = fname_after_chroot(cfg->control_key_file, cfg, 1);
	c_cert = fname_after_chroot(cfg->control_cert_file, cfg, 1);
	if(!s_cert || !c_key || !c_cert)
		fatal_exit("out of memory");
	ctx = SSL_CTX_new(SSLv23_client_method());
	if(!ctx)
		ssl_err("could not allocate SSL_CTX pointer");
#if SSL_OP_NO_SSLv2 != 0
	if((SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2) & SSL_OP_NO_SSLv2)
		!= SSL_OP_NO_SSLv2)
		ssl_err("could not set SSL_OP_NO_SSLv2");
#endif
	if((SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3) & SSL_OP_NO_SSLv3)
		!= SSL_OP_NO_SSLv3)
		ssl_err("could not set SSL_OP_NO_SSLv3");
#if defined(SSL_OP_NO_RENEGOTIATION)
	/* disable client renegotiation */
	if((SSL_CTX_set_options(ctx, SSL_OP_NO_RENEGOTIATION) &
		SSL_OP_NO_RENEGOTIATION) != SSL_OP_NO_RENEGOTIATION)
		ssl_err("could not set SSL_OP_NO_RENEGOTIATION");
#endif
	if(!SSL_CTX_use_certificate_chain_file(ctx,c_cert))
		ssl_path_err("Error setting up SSL_CTX client cert", c_cert);
	if (!SSL_CTX_use_PrivateKey_file(ctx,c_key,SSL_FILETYPE_PEM))
		ssl_path_err("Error setting up SSL_CTX client key", c_key);
	if (!SSL_CTX_check_private_key(ctx))
		ssl_err("Error setting up SSL_CTX client key");
	if (SSL_CTX_load_verify_locations(ctx, s_cert, NULL) != 1)
		ssl_path_err("Error setting up SSL_CTX verify, server cert",
			     s_cert);
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	free(s_cert);
	free(c_key);
	free(c_cert);
	return ctx;
}

/** check connect error */
static void
checkconnecterr(int err, const char* svr, struct sockaddr_storage* addr,
	socklen_t addrlen, int statuscmd, int useport)
{
#ifndef USE_WINSOCK
	if(!useport) log_err("connect: %s for %s", strerror(err), svr);
	else log_err_addr("connect", strerror(err), addr, addrlen);
	if(err == ECONNREFUSED && statuscmd) {
		printf("unbound is stopped\n");
		exit(3);
	}
#else
	int wsaerr = err;
	if(!useport) log_err("connect: %s for %s", wsa_strerror(wsaerr), svr);
	else log_err_addr("connect", wsa_strerror(wsaerr), addr, addrlen);
	if(wsaerr == WSAECONNREFUSED && statuscmd) {
		printf("unbound is stopped\n");
		exit(3);
	}
#endif
	exit(1);
}

/** contact the server with TCP connect */
static int
contact_server(const char* svr, struct config_file* cfg, int statuscmd)
{
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int addrfamily = 0, proto = IPPROTO_TCP;
	int fd, useport = 1;
	/* use svr or the first config entry */
	if(!svr) {
		if(cfg->control_ifs.first) {
			svr = cfg->control_ifs.first->str;
		} else if(cfg->do_ip4) {
			svr = "127.0.0.1";
		} else {
			svr = "::1";
		}
		/* config 0 addr (everything), means ask localhost */
		if(strcmp(svr, "0.0.0.0") == 0)
			svr = "127.0.0.1";
		else if(strcmp(svr, "::0") == 0 ||
			strcmp(svr, "0::0") == 0 ||
			strcmp(svr, "0::") == 0 ||
			strcmp(svr, "::") == 0)
			svr = "::1";
	}
	if(strchr(svr, '@')) {
		if(!extstrtoaddr(svr, &addr, &addrlen))
			fatal_exit("could not parse IP@port: %s", svr);
#ifdef HAVE_SYS_UN_H
	} else if(svr[0] == '/') {
		struct sockaddr_un* usock = (struct sockaddr_un *) &addr;
		usock->sun_family = AF_LOCAL;
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
		usock->sun_len = (unsigned)sizeof(usock);
#endif
		(void)strlcpy(usock->sun_path, svr, sizeof(usock->sun_path));
		addrlen = (socklen_t)sizeof(struct sockaddr_un);
		addrfamily = AF_LOCAL;
		useport = 0;
		proto = 0;
#endif
	} else {
		if(!ipstrtoaddr(svr, cfg->control_port, &addr, &addrlen))
			fatal_exit("could not parse IP: %s", svr);
	}

	if(addrfamily == 0)
		addrfamily = addr_is_ip6(&addr, addrlen)?PF_INET6:PF_INET;
	fd = socket(addrfamily, SOCK_STREAM, proto);
	if(fd == -1) {
		fatal_exit("socket: %s", sock_strerror(errno));
	}
	fd_set_nonblock(fd);
	if(connect(fd, (struct sockaddr*)&addr, addrlen) < 0) {
#ifndef USE_WINSOCK
#ifdef EINPROGRESS
		if(errno != EINPROGRESS) {
			checkconnecterr(errno, svr, &addr,
				addrlen, statuscmd, useport);
		}
#endif
#else
		if(WSAGetLastError() != WSAEINPROGRESS &&
			WSAGetLastError() != WSAEWOULDBLOCK) {
			checkconnecterr(WSAGetLastError(), svr, &addr,
				addrlen, statuscmd, useport);
		}
#endif
	}
	while(1) {
		fd_set rset, wset, eset;
		struct timeval tv;
		FD_ZERO(&rset);
		FD_SET(FD_SET_T fd, &rset);
		FD_ZERO(&wset);
		FD_SET(FD_SET_T fd, &wset);
		FD_ZERO(&eset);
		FD_SET(FD_SET_T fd, &eset);
		tv.tv_sec = UNBOUND_CONTROL_CONNECT_TIMEOUT/1000;
		tv.tv_usec= (UNBOUND_CONTROL_CONNECT_TIMEOUT%1000)*1000;
		if(select(fd+1, &rset, &wset, &eset, &tv) == -1) {
			fatal_exit("select: %s", sock_strerror(errno));
		}
		if(!FD_ISSET(fd, &rset) && !FD_ISSET(fd, &wset) &&
			!FD_ISSET(fd, &eset)) {
			fatal_exit("timeout: could not connect to server");
		} else {
			/* check nonblocking connect error */
			int error = 0;
			socklen_t len = (socklen_t)sizeof(error);
			if(getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&error,
				&len) < 0) {
#ifndef USE_WINSOCK
				error = errno; /* on solaris errno is error */
#else
				error = WSAGetLastError();
#endif
			}
			if(error != 0) {
#ifndef USE_WINSOCK
#ifdef EINPROGRESS
				if(error == EINPROGRESS)
					continue; /* try again later */
#endif
#ifdef EWOULDBLOCK
				if(error == EWOULDBLOCK)
					continue; /* try again later */
#endif
#else
				if(error == WSAEINPROGRESS)
					continue; /* try again later */
				if(error == WSAEWOULDBLOCK)
					continue; /* try again later */
#endif
				checkconnecterr(error, svr, &addr, addrlen,
					statuscmd, useport);
			}
		}
		break;
	}
	fd_set_block(fd);
	return fd;
}

/** setup SSL on the connection */
static SSL*
setup_ssl(SSL_CTX* ctx, int fd)
{
	SSL* ssl;
	X509* x;
	int r;

	if(!ctx) return NULL;
	ssl = SSL_new(ctx);
	if(!ssl)
		ssl_err("could not SSL_new");
	SSL_set_connect_state(ssl);
	(void)SSL_set_mode(ssl, (long)SSL_MODE_AUTO_RETRY);
	if(!SSL_set_fd(ssl, fd))
		ssl_err("could not SSL_set_fd");
	while(1) {
		ERR_clear_error();
		if( (r=SSL_do_handshake(ssl)) == 1)
			break;
		r = SSL_get_error(ssl, r);
		if(r != SSL_ERROR_WANT_READ && r != SSL_ERROR_WANT_WRITE)
			ssl_err("SSL handshake failed");
		/* wants to be called again */
	}

	/* check authenticity of server */
	if(SSL_get_verify_result(ssl) != X509_V_OK)
		ssl_err("SSL verification failed");
	x = SSL_get_peer_certificate(ssl);
	if(!x)
		ssl_err("Server presented no peer certificate");
	X509_free(x);

	return ssl;
}

/** read from ssl or fd, fatalexit on error, 0 EOF, 1 success */
static int
remote_read(SSL* ssl, int fd, char* buf, size_t len)
{
	if(ssl) {
		int r;
		ERR_clear_error();
		if((r = SSL_read(ssl, buf, (int)len-1)) <= 0) {
			if(SSL_get_error(ssl, r) == SSL_ERROR_ZERO_RETURN) {
				/* EOF */
				return 0;
			}
			ssl_err("could not SSL_read");
		}
		buf[r] = 0;
	} else {
		ssize_t rr = recv(fd, buf, len-1, 0);
		if(rr <= 0) {
			if(rr == 0) {
				/* EOF */
				return 0;
			}
			fatal_exit("could not recv: %s", sock_strerror(errno));
		}
		buf[rr] = 0;
	}
	return 1;
}

/** write to ssl or fd, fatalexit on error */
static void
remote_write(SSL* ssl, int fd, const char* buf, size_t len)
{
	if(ssl) {
		if(SSL_write(ssl, buf, (int)len) <= 0)
			ssl_err("could not SSL_write");
	} else {
		if(send(fd, buf, len, 0) < (ssize_t)len) {
			fatal_exit("could not send: %s", sock_strerror(errno));
		}
	}
}

// Check for the old-style view access commands; translate to the
// extended command form. Return the adjusted number of arguments.

static int
check_args_for_viewcmd(int argc, char* argv[])
{
	char *cmd = argv[0];

	if (strncmp("view_", cmd, 5) != 0) {
		return (argc);
	}
	if (argc < 2) {
		fatal_exit("missing view name for %s\n", cmd);
	}

	// The equivalent command strips "view_"

	cmd += 5;

	// If the command has a '.', the user is specifying a zone, so we
	// can't combine the first two arguments

	if (strchr(cmd, '.') != NULL) {
		return (argc);
	}
	size_t clen = strlen(cmd);
	char *cp;

	if (argv[1] == cmd + clen + 1) {
		// This trick takes advantage of how argv is built on the stack.
		// If the start of argv[1] (the view name) is two bytes past the
		// end of the command, we can replace the intervening NUL with '.'.

		argv[0] = cmd;
		cmd[clen] = '.';
	} else if ((cp = malloc(clen + strlen(argv[1]) + 2)) != NULL) {
		sprintf(cp, "%s.%s", cmd, argv[1]);
		argv[0] = cp;
	} else {
		fatal_exit("out of memory for %s\n", argv[0]);
	}

	// Move all other arguments up one slot

	int arg;

	for (arg = 2; arg < argc; arg++) {
		argv[arg - 1] = argv[arg];
	}

	// Replace the last vacated argument with NULL for consistency

	argv[argc - 1] = NULL;

	return (argc - 1);
}

/** check args, to see if too many args. Because when a file is sent it
 * would wait for the terminal, and we can check for too many arguments,
 * eg. user put arguments on the commandline. */
static void
check_args_for_listcmd(int argc, char* argv[])
{
	if(argc >= 1 && (strcmp(argv[0], "local_zones") == 0 ||
		strcmp(argv[0], "local_zones_remove") == 0 ||
		strcmp(argv[0], "local_datas") == 0 ||
		strcmp(argv[0], "local_datas_remove") == 0) &&
		argc >= 2) {
		fatal_exit("too many arguments for command '%s', "
			"content is piped in from stdin", argv[0]);
	}
	if(argc >= 1 && (strcmp(argv[0], "view_local_datas") == 0 ||
		strcmp(argv[0], "view_local_datas_remove") == 0) &&
		argc >= 3) {
		fatal_exit("too many arguments for command '%s', "
			"content is piped in from stdin", argv[0]);
	}
}

/** send stdin to server */
static void
send_file(SSL* ssl, int fd, FILE* in, char* buf, size_t sz)
{
	while(fgets(buf, (int)sz, in)) {
		remote_write(ssl, fd, buf, strlen(buf));
	}
}

/** send end-of-file marker to server */
static void
send_eof(SSL* ssl, int fd)
{
	char e[] = {0x04, 0x0a};
	remote_write(ssl, fd, e, sizeof(e));
}

/** send command and display result */
static int
go_cmd(SSL* ssl, int fd, int quiet, int argc, char* argv[])
{
	char pre[10];
	const char* space=" ";
	const char* newline="\n";
	int was_error = 0, first_line = 1;
	int i;
	char buf[1024];
	char * pch;
	snprintf(pre, sizeof(pre), "UBCT%d ", UNBOUND_CONTROL_VERSION);
	remote_write(ssl, fd, pre, strlen(pre));
	for(i=0; i<argc; i++) {
		remote_write(ssl, fd, space, strlen(space));
		remote_write(ssl, fd, argv[i], strlen(argv[i]));
	}
	remote_write(ssl, fd, newline, strlen(newline));

	if(argc == 1 && strcmp(argv[0], "load_cache") == 0) {
		send_file(ssl, fd, stdin, buf, sizeof(buf));
	}
	else {
		pch = strtok (argv[0],".");
		if(argc >= 1 && (strcmp(argv[0], "local_zones") == 0 ||
					strcmp(argv[0], "local_zones_remove") == 0 ||
					strcmp(argv[0], "local_datas") == 0 ||
					strcmp(argv[0], "view_local_datas") == 0 ||
					strcmp(argv[0], "local_datas_remove") == 0 ||
					strcmp(argv[0], "view_local_datas_remove") == 0)) {
			send_file(ssl, fd, stdin, buf, sizeof(buf));
			send_eof(ssl, fd);
		}
	}

	while(1) {
		if(remote_read(ssl, fd, buf, sizeof(buf)) == 0) {
			break; /* EOF */
		}
		if(first_line && strncmp(buf, "error", 5) == 0) {
			printf("%s", buf);
			was_error = 1;
		} else if (!quiet)
			printf("%s", buf);

		first_line = 0;
	}
	return was_error;
}

/** go ahead and read config, contact server and perform command and display */
static int
go(const char* cfgfile, char* svr, int quiet, int argc, char* argv[])
{
	struct config_file* cfg;
	int fd, ret;
	SSL_CTX* ctx;
	SSL* ssl;

	/* read config */
	if(!(cfg = config_create()))
		fatal_exit("out of memory");
	if(!config_read(cfg, cfgfile, NULL))
		fatal_exit("could not read config file");
	if(!cfg->remote_control_enable)
		log_warn("control-enable is 'no' in the config file.");
#ifdef UB_ON_WINDOWS
	w_config_adjust_directory(cfg);
#endif
	ctx = setup_ctx(cfg);

	/* contact server */
	fd = contact_server(svr, cfg, argc>0&&strcmp(argv[0],"status")==0);
	ssl = setup_ssl(ctx, fd);

	/* send command */
	ret = go_cmd(ssl, fd, quiet, argc, argv);

	if(ssl) SSL_free(ssl);
	sock_close(fd);
	if(ctx) SSL_CTX_free(ctx);
	config_delete(cfg);
	return ret;
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** Main routine for unbound-control */
int
main(int argc, char* argv[])
{
	int c, ret;
	int quiet = 0;
	const char* cfgfile = CONFIGFILE;
	char* svr = NULL;
#ifdef USE_WINSOCK
	int r;
	WSADATA wsa_data;
#endif
#ifdef USE_THREAD_DEBUG
	/* stop the file output from unbound-control, overwrites the servers */
	extern int check_locking_order;
	check_locking_order = 0;
#endif /* USE_THREAD_DEBUG */
	log_ident_set("unbound-control");
	log_init(NULL, 0, NULL);
	checklock_start();
#ifdef USE_WINSOCK
	/* use registry config file in preference to compiletime location */
	if(!(cfgfile=w_lookup_reg_str("Software\\Unbound", "ConfigFile")))
		cfgfile = CONFIGFILE;
#endif
	/* parse the options */
	while( (c=getopt(argc, argv, "c:s:qh")) != -1) {
		switch(c) {
		case 'c':
			cfgfile = optarg;
			break;
		case 's':
			svr = optarg;
			break;
		case 'q':
			quiet = 1;
			break;
		case '?':
		case 'h':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if(argc == 0)
		usage();
	if(argc >= 1 && strcmp(argv[0], "start")==0) {
#if (defined(TARGET_OS_TV) && TARGET_OS_TV) || (defined(TARGET_OS_WATCH) && TARGET_OS_WATCH)
		fatal_exit("could not exec unbound: %s",
			strerror(ENOSYS));
#else
		if(execlp("unbound", "unbound", "-c", cfgfile, 
			(char*)NULL) < 0) {
			fatal_exit("could not exec unbound: %s",
				strerror(errno));
		}
#endif
	}
	if(argc >= 1 && strcmp(argv[0], "stats_shm")==0) {
		print_stats_shm(cfgfile);
		return 0;
	}
	if ((argc = check_args_for_viewcmd(argc, argv)) == 0) {
		return 1;
	}
	check_args_for_listcmd(argc, argv);

#ifdef USE_WINSOCK
	if((r = WSAStartup(MAKEWORD(2,2), &wsa_data)) != 0)
		fatal_exit("WSAStartup failed: %s", wsa_strerror(r));
#endif

#ifdef HAVE_ERR_LOAD_CRYPTO_STRINGS
	ERR_load_crypto_strings();
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000 || !defined(HAVE_OPENSSL_INIT_SSL)
	ERR_load_SSL_strings();
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000 || !defined(HAVE_OPENSSL_INIT_CRYPTO)
#  ifndef S_SPLINT_S
	OpenSSL_add_all_algorithms();
#  endif
#else
	OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS
		| OPENSSL_INIT_ADD_ALL_DIGESTS
		| OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000 || !defined(HAVE_OPENSSL_INIT_SSL)
	(void)SSL_library_init();
#else
	(void)OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
#endif

	if(!RAND_status()) {
		/* try to seed it */
		unsigned char buf[256];
		unsigned int seed=(unsigned)time(NULL) ^ (unsigned)getpid();
		unsigned int v = seed;
		size_t i;
		for(i=0; i<256/sizeof(v); i++) {
			memmove(buf+i*sizeof(v), &v, sizeof(v));
			v = v*seed + (unsigned int)i;
		}
		RAND_seed(buf, 256);
		log_warn("no entropy, seeding openssl PRNG with time\n");
	}

	ret = go(cfgfile, svr, quiet, argc, argv);

#ifdef USE_WINSOCK
	WSACleanup();
#endif
	checklock_stop();
	return ret;
}
