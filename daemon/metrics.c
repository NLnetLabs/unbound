/*
 * daemon/metrics.c - prometheus metrics http exporter for the unbound daemon.
 *
 * Copyright (c) 2025, NLnet Labs. All rights reserved.
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
 * Prometheus metrics exporter.  Serves the merged (stats_noreset) server
 * statistics as a plain HTTP/1.1 response in the Prometheus text exposition
 * format, on its own interface/port, configured with metrics-enable,
 * metrics-interface, metrics-port and metrics-path.  This has its own
 * plain-text listener, separate from the (TLS capable) remote-control
 * interface in daemon/remote.c.
 */
#include "config.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "daemon/metrics.h"
#include "libunbound/remote.h"
#include "daemon/daemon.h"
#include "daemon/worker.h"
#include "daemon/stats.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "util/module.h"
#include "util/netevent.h"
#include "util/timehist.h"
#include "util/timeval_func.h"
#include "util/storage/slabhash.h"
#include "services/listen_dnsport.h"
#include "services/cache/rrset.h"
#include "services/rpz.h"
#include "sldns/rrdef.h"
#include "sldns/parseutil.h"
#include "sldns/wire2str.h"

/** timeout in msec for metrics http connections */
#define METRICS_TCP_TIMEOUT 10000
/** acceptable length of an http request/header line */
#define MAX_METRICS_STRLINE 1024

/** printf style, blocking, write to a plain tcp socket */
static int
http_printf(int fd, const char* format, ...)
{
	char msg[65535];
	va_list args;
	size_t at = 0, len;
	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);
	len = strlen(msg);
	while(at < len) {
		ssize_t r = send(fd, msg+at, len-at, 0);
		if(r == -1) {
			if(errno == EAGAIN || errno == EINTR)
				continue;
			log_err("could not send: %s", sock_strerror(errno));
			return 0;
		}
		at += (size_t)r;
	}
	return 1;
}

/** read one line (up to \\n) from a plain tcp socket, blocking.
 * the trailing \\r (if any) and \\n are stripped from the result. */
static int
http_read_line(int fd, char* buf, size_t max)
{
	size_t len = 0;
	while(len < max) {
		ssize_t rr = recv(fd, buf+len, 1, 0);
		if(rr <= 0) {
			if(rr == 0) {
				buf[len] = 0;
				return 1;
			}
			if(errno == EINTR || errno == EAGAIN)
				continue;
			log_err("could not recv: %s", sock_strerror(errno));
			return 0;
		}
		if(buf[len] == '\n') {
			buf[len] = 0;
			if(len > 0 && buf[len-1] == '\r')
				buf[len-1] = 0;
			return 1;
		}
		len++;
	}
	buf[max-1] = 0;
	log_err("metrics http line too long (%d): %s", (int)max, buf);
	return 0;
}

/** print one prometheus sample line with a single label */
static int
metrics_print_label_line(int fd, const char* metric, const char* label,
	const char* value, unsigned long num)
{
	/* label values used here always come from a fixed, known-safe set
	 * (RR type/class/opcode/rcode mnemonics, or our own literal
	 * strings), so no escaping of the value is needed. */
	return http_printf(fd, "%s{%s=\"%s\"} %lu\n", metric, label, value,
		num);
}

/** emit HELP+TYPE and one sample per thread, plus the merged total, for
 * an unsigned 64-bit valued metric. "arr" has "n"+1 entries, with the
 * last (index n) holding the merged total. "expr" may use "_s" to refer
 * to the current "struct ub_stats_info*". Jumps to "done" on failure. */
#define METRICS_U64(fd_, name_, help_, type_, arr_, n_, expr_) \
	do { \
		int _mi; \
		if(!http_printf((fd_), "# HELP %s %s\n", (name_), (help_))) \
			goto done; \
		if(!http_printf((fd_), "# TYPE %s %s\n", (name_), (type_))) \
			goto done; \
		for(_mi=0; _mi <= (n_); _mi++) { \
			struct ub_stats_info* _s = &(arr_)[_mi]; \
			if(_mi == (n_)) { \
				if(!http_printf((fd_), \
					"%s{thread=\"total\"} %lu\n", (name_), \
					(unsigned long)(expr_))) goto done; \
			} else { \
				if(!http_printf((fd_), \
					"%s{thread=\"%d\"} %lu\n", (name_), \
					_mi, (unsigned long)(expr_))) goto done; \
			} \
		} \
		if(!http_printf((fd_), "\n")) goto done; \
	} while(0)

/** like METRICS_U64, but for a floating point valued metric */
#define METRICS_DOUBLE(fd_, name_, help_, type_, arr_, n_, expr_) \
	do { \
		int _mi; \
		if(!http_printf((fd_), "# HELP %s %s\n", (name_), (help_))) \
			goto done; \
		if(!http_printf((fd_), "# TYPE %s %s\n", (name_), (type_))) \
			goto done; \
		for(_mi=0; _mi <= (n_); _mi++) { \
			struct ub_stats_info* _s = &(arr_)[_mi]; \
			if(_mi == (n_)) { \
				if(!http_printf((fd_), \
					"%s{thread=\"total\"} %g\n", (name_), \
					(double)(expr_))) goto done; \
			} else { \
				if(!http_printf((fd_), \
					"%s{thread=\"%d\"} %g\n", (name_), \
					_mi, (double)(expr_))) goto done; \
			} \
		} \
		if(!http_printf((fd_), "\n")) goto done; \
	} while(0)

/** write the extended statistics (query type/class/opcode/rcode/flags,
 * caches, memory, histogram, ...) for the merged total, in prometheus
 * format. Mirrors print_ext()/print_mem()/print_hist() in daemon/remote.c */
static void
do_metrics_extended(int fd, struct worker* worker, struct daemon* daemon,
	struct ub_stats_info* s)
{
	int i, inhibit_zero = daemon->cfg->stat_inhibit_zero;
	char nm[32];
	const sldns_rr_descriptor* desc;
	const sldns_lookup_table* lt;
	size_t msg, rrset, val, iter, respip;
#ifdef CLIENT_SUBNET
	size_t subnet = 0;
#endif
#ifdef USE_IPSECMOD
	size_t ipsecmod = 0;
#endif
#ifdef USE_DNSCRYPT
	size_t dnscrypt_shared_secret = 0;
	size_t dnscrypt_nonce = 0;
#endif
#ifdef WITH_DYNLIBMODULE
	size_t dynlib = 0;
#endif
	struct timehist* hist;

	/* query type */
	if(!http_printf(fd, "# HELP unbound_query_type_total Queries by "
		"query type.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_query_type_total counter\n"))
		return;
	for(i=0; i<UB_STATS_QTYPE_NUM; i++) {
		if(inhibit_zero && s->svr.qtype[i] == 0)
			continue;
		desc = sldns_rr_descript((uint16_t)i);
		if(desc && desc->_name) {
			snprintf(nm, sizeof(nm), "%s", desc->_name);
		} else if (i == LDNS_RR_TYPE_IXFR) {
			snprintf(nm, sizeof(nm), "IXFR");
		} else if (i == LDNS_RR_TYPE_AXFR) {
			snprintf(nm, sizeof(nm), "AXFR");
		} else if (i == LDNS_RR_TYPE_MAILA) {
			snprintf(nm, sizeof(nm), "MAILA");
		} else if (i == LDNS_RR_TYPE_MAILB) {
			snprintf(nm, sizeof(nm), "MAILB");
		} else if (i == LDNS_RR_TYPE_ANY) {
			snprintf(nm, sizeof(nm), "ANY");
		} else {
			snprintf(nm, sizeof(nm), "TYPE%d", i);
		}
		if(!metrics_print_label_line(fd, "unbound_query_type_total",
			"type", nm, (unsigned long)s->svr.qtype[i])) return;
	}
	if(!inhibit_zero || s->svr.qtype_big) {
		if(!metrics_print_label_line(fd, "unbound_query_type_total",
			"type", "other", (unsigned long)s->svr.qtype_big))
			return;
	}
	if(!http_printf(fd, "\n")) return;

	/* query class */
	if(!http_printf(fd, "# HELP unbound_query_class_total Queries by "
		"query class.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_query_class_total counter\n"))
		return;
	for(i=0; i<UB_STATS_QCLASS_NUM; i++) {
		if(inhibit_zero && s->svr.qclass[i] == 0)
			continue;
		lt = sldns_lookup_by_id(sldns_rr_classes, i);
		if(lt && lt->name) {
			snprintf(nm, sizeof(nm), "%s", lt->name);
		} else {
			snprintf(nm, sizeof(nm), "CLASS%d", i);
		}
		if(!metrics_print_label_line(fd, "unbound_query_class_total",
			"class", nm, (unsigned long)s->svr.qclass[i])) return;
	}
	if(!inhibit_zero || s->svr.qclass_big) {
		if(!metrics_print_label_line(fd, "unbound_query_class_total",
			"class", "other", (unsigned long)s->svr.qclass_big))
			return;
	}
	if(!http_printf(fd, "\n")) return;

	/* query opcode */
	if(!http_printf(fd, "# HELP unbound_query_opcode_total Queries by "
		"opcode.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_query_opcode_total counter\n"))
		return;
	for(i=0; i<UB_STATS_OPCODE_NUM; i++) {
		if(inhibit_zero && s->svr.qopcode[i] == 0)
			continue;
		lt = sldns_lookup_by_id(sldns_opcodes, i);
		if(lt && lt->name) {
			snprintf(nm, sizeof(nm), "%s", lt->name);
		} else {
			snprintf(nm, sizeof(nm), "OPCODE%d", i);
		}
		if(!metrics_print_label_line(fd, "unbound_query_opcode_total",
			"opcode", nm, (unsigned long)s->svr.qopcode[i]))
			return;
	}
	if(!http_printf(fd, "\n")) return;

	/* transport */
	if(!http_printf(fd, "# HELP unbound_query_transport_total Queries "
		"by transport.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_query_transport_total counter\n"))
		return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "tcp", (unsigned long)s->svr.qtcp)) return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "tcpout", (unsigned long)s->svr.qtcp_outgoing))
		return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "udpout", (unsigned long)s->svr.qudp_outgoing))
		return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "tls", (unsigned long)s->svr.qtls)) return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "tls_resume", (unsigned long)s->svr.qtls_resume))
		return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "ipv6", (unsigned long)s->svr.qipv6)) return;
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "https", (unsigned long)s->svr.qhttps)) return;
#ifdef HAVE_NGTCP2
	if(!metrics_print_label_line(fd, "unbound_query_transport_total",
		"transport", "quic", (unsigned long)s->svr.qquic)) return;
#endif
	if(!http_printf(fd, "\n")) return;

	/* flags */
	if(!http_printf(fd, "# HELP unbound_query_flag_total Queries by "
		"DNS header flag or EDNS feature present in the query.\n"))
		return;
	if(!http_printf(fd, "# TYPE unbound_query_flag_total counter\n"))
		return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "QR", (unsigned long)s->svr.qbit_QR)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "AA", (unsigned long)s->svr.qbit_AA)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "TC", (unsigned long)s->svr.qbit_TC)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "RD", (unsigned long)s->svr.qbit_RD)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "RA", (unsigned long)s->svr.qbit_RA)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "Z", (unsigned long)s->svr.qbit_Z)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "AD", (unsigned long)s->svr.qbit_AD)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "CD", (unsigned long)s->svr.qbit_CD)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "EDNS", (unsigned long)s->svr.qEDNS)) return;
	if(!metrics_print_label_line(fd, "unbound_query_flag_total",
		"flag", "EDNS_DO", (unsigned long)s->svr.qEDNS_DO)) return;
	if(!http_printf(fd, "\n")) return;

	/* rcode */
	if(!http_printf(fd, "# HELP unbound_answer_rcode_total Answers by "
		"rcode.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_answer_rcode_total counter\n"))
		return;
	for(i=0; i<UB_STATS_RCODE_NUM; i++) {
		if(inhibit_zero && i > LDNS_RCODE_REFUSED &&
			s->svr.ans_rcode[i] == 0)
			continue;
		lt = sldns_lookup_by_id(sldns_rcodes, i);
		if(lt && lt->name) {
			snprintf(nm, sizeof(nm), "%s", lt->name);
		} else {
			snprintf(nm, sizeof(nm), "RCODE%d", i);
		}
		if(!metrics_print_label_line(fd, "unbound_answer_rcode_total",
			"rcode", nm, (unsigned long)s->svr.ans_rcode[i]))
			return;
	}
	if(!inhibit_zero || s->svr.ans_rcode_nodata) {
		if(!metrics_print_label_line(fd, "unbound_answer_rcode_total",
			"rcode", "nodata",
			(unsigned long)s->svr.ans_rcode_nodata)) return;
	}
	if(!http_printf(fd, "\n")) return;

	/* iteration / validation / threat detection */
	if(!http_printf(fd, "# HELP unbound_query_ratelimited_total Queries "
		"ratelimited by domain recursion.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_query_ratelimited_total "
		"counter\n")) return;
	if(!http_printf(fd, "unbound_query_ratelimited_total %lu\n\n",
		(unsigned long)s->svr.queries_ratelimited)) return;

	if(!http_printf(fd, "# HELP unbound_answer_secure_total Answers "
		"that validated as secure.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_answer_secure_total counter\n"))
		return;
	if(!http_printf(fd, "unbound_answer_secure_total %lu\n\n",
		(unsigned long)s->svr.ans_secure)) return;

	if(!http_printf(fd, "# HELP unbound_answer_bogus_total Answers "
		"that failed validation (withheld as SERVFAIL).\n")) return;
	if(!http_printf(fd, "# TYPE unbound_answer_bogus_total counter\n"))
		return;
	if(!http_printf(fd, "unbound_answer_bogus_total %lu\n\n",
		(unsigned long)s->svr.ans_bogus)) return;

	if(!http_printf(fd, "# HELP unbound_rrset_bogus_total RRsets marked "
		"bogus by the validator.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_rrset_bogus_total counter\n"))
		return;
	if(!http_printf(fd, "unbound_rrset_bogus_total %lu\n\n",
		(unsigned long)s->svr.rrset_bogus)) return;

	if(!http_printf(fd, "# HELP unbound_validator_ops_total Signature "
		"validation operations performed.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_validator_ops_total counter\n"))
		return;
	if(!http_printf(fd, "unbound_validator_ops_total %lu\n\n",
		(unsigned long)s->svr.val_ops)) return;

	if(!http_printf(fd, "# HELP unbound_aggressive_nsec_total Answers "
		"synthesized from the aggressive NSEC(3) cache, by "
		"rcode.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_aggressive_nsec_total "
		"counter\n")) return;
	if(!metrics_print_label_line(fd, "unbound_aggressive_nsec_total",
		"rcode", "NOERROR",
		(unsigned long)s->svr.num_neg_cache_noerror)) return;
	if(!metrics_print_label_line(fd, "unbound_aggressive_nsec_total",
		"rcode", "NXDOMAIN",
		(unsigned long)s->svr.num_neg_cache_nxdomain)) return;
	if(!http_printf(fd, "\n")) return;

	if(!http_printf(fd, "# HELP unbound_unwanted_total Unwanted traffic "
		"received.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_unwanted_total counter\n"))
		return;
	if(!metrics_print_label_line(fd, "unbound_unwanted_total",
		"side", "queries", (unsigned long)s->svr.unwanted_queries))
		return;
	if(!metrics_print_label_line(fd, "unbound_unwanted_total",
		"side", "replies", (unsigned long)s->svr.unwanted_replies))
		return;
	if(!http_printf(fd, "\n")) return;

	/* cache counts */
	if(!http_printf(fd, "# HELP unbound_cache_count Number of entries "
		"in a cache.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_cache_count gauge\n")) return;
	if(!metrics_print_label_line(fd, "unbound_cache_count", "cache",
		"message", (unsigned long)s->svr.msg_cache_count)) return;
	if(!metrics_print_label_line(fd, "unbound_cache_count", "cache",
		"rrset", (unsigned long)s->svr.rrset_cache_count)) return;
	if(!metrics_print_label_line(fd, "unbound_cache_count", "cache",
		"infra", (unsigned long)s->svr.infra_cache_count)) return;
	if(!metrics_print_label_line(fd, "unbound_cache_count", "cache",
		"key", (unsigned long)s->svr.key_cache_count)) return;
	if(!http_printf(fd, "\n")) return;

	if(!http_printf(fd, "# HELP unbound_cache_max_collisions Maximum "
		"number of hash-table collisions seen in a cache.\n"))
		return;
	if(!http_printf(fd, "# TYPE unbound_cache_max_collisions gauge\n"))
		return;
	if(!metrics_print_label_line(fd, "unbound_cache_max_collisions",
		"cache", "message",
		(unsigned long)s->svr.msg_cache_max_collisions)) return;
	if(!metrics_print_label_line(fd, "unbound_cache_max_collisions",
		"cache", "rrset",
		(unsigned long)s->svr.rrset_cache_max_collisions)) return;
	if(!http_printf(fd, "\n")) return;

	/* RPZ actions */
	if(!http_printf(fd, "# HELP unbound_rpz_action_total RPZ actions "
		"applied.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_rpz_action_total counter\n"))
		return;
	for(i=0; i<UB_STATS_RPZ_ACTION_NUM; i++) {
		if(i == RPZ_NO_OVERRIDE_ACTION)
			continue;
		if(inhibit_zero && s->svr.rpz_action[i] == 0)
			continue;
		if(!metrics_print_label_line(fd, "unbound_rpz_action_total",
			"action", rpz_action_to_string(i),
			(unsigned long)s->svr.rpz_action[i])) return;
	}
	if(!http_printf(fd, "\n")) return;

#ifdef USE_DNSCRYPT
	if(!http_printf(fd, "# HELP unbound_dnscrypt_cache_count DNSCrypt "
		"cache entries.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_dnscrypt_cache_count gauge\n"))
		return;
	if(!metrics_print_label_line(fd, "unbound_dnscrypt_cache_count",
		"cache", "shared_secret",
		(unsigned long)s->svr.shared_secret_cache_count)) return;
	if(!metrics_print_label_line(fd, "unbound_dnscrypt_cache_count",
		"cache", "nonce",
		(unsigned long)s->svr.nonce_cache_count)) return;
	if(!http_printf(fd, "\n")) return;

	if(!http_printf(fd, "# HELP unbound_dnscrypt_cache_miss_total "
		"DNSCrypt queries without a cached shared secret.\n"))
		return;
	if(!http_printf(fd, "# TYPE unbound_dnscrypt_cache_miss_total "
		"counter\n")) return;
	if(!http_printf(fd, "unbound_dnscrypt_cache_miss_total %lu\n\n",
		(unsigned long)s->svr.num_query_dnscrypt_secret_missed_cache))
		return;

	if(!http_printf(fd, "# HELP unbound_dnscrypt_replay_total DNSCrypt "
		"queries that were replays.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_dnscrypt_replay_total "
		"counter\n")) return;
	if(!http_printf(fd, "unbound_dnscrypt_replay_total %lu\n\n",
		(unsigned long)s->svr.num_query_dnscrypt_replay)) return;
#endif

	if(!http_printf(fd, "# HELP unbound_authzone_total Queries for "
		"auth-zones.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_authzone_total counter\n"))
		return;
	if(!metrics_print_label_line(fd, "unbound_authzone_total",
		"direction", "upstream",
		(unsigned long)s->svr.num_query_authzone_up)) return;
	if(!metrics_print_label_line(fd, "unbound_authzone_total",
		"direction", "downstream",
		(unsigned long)s->svr.num_query_authzone_down)) return;
	if(!http_printf(fd, "\n")) return;

#ifdef CLIENT_SUBNET
	if(!http_printf(fd, "# HELP unbound_subnet_total Queries answered "
		"using EDNS Client Subnet specific data.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_subnet_total counter\n"))
		return;
	if(!http_printf(fd, "unbound_subnet_total %lu\n",
		(unsigned long)s->svr.num_query_subnet)) return;
	if(!http_printf(fd, "unbound_subnet_cache_total %lu\n\n",
		(unsigned long)s->svr.num_query_subnet_cache)) return;
#endif

#ifdef USE_CACHEDB
	if(!http_printf(fd, "# HELP unbound_cachedb_total Queries served "
		"from the external cachedb cache.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_cachedb_total counter\n"))
		return;
	if(!http_printf(fd, "unbound_cachedb_total %lu\n\n",
		(unsigned long)s->svr.num_query_cachedb)) return;
#endif

	/* memory usage */
	msg = slabhash_get_mem(daemon->env->msg_cache);
	rrset = slabhash_get_mem(&daemon->env->rrset_cache->table);
	val = mod_get_mem(&worker->env, "validator");
	iter = mod_get_mem(&worker->env, "iterator");
	respip = mod_get_mem(&worker->env, "respip");
#ifdef CLIENT_SUBNET
	subnet = mod_get_mem(&worker->env, "subnetcache");
#endif
#ifdef USE_IPSECMOD
	ipsecmod = mod_get_mem(&worker->env, "ipsecmod");
#endif
#ifdef USE_DNSCRYPT
	if(daemon->dnscenv) {
		dnscrypt_shared_secret = slabhash_get_mem(
			daemon->dnscenv->shared_secrets_cache);
		dnscrypt_nonce = slabhash_get_mem(daemon->dnscenv->nonces_cache);
	}
#endif
#ifdef WITH_DYNLIBMODULE
	dynlib = mod_get_mem(&worker->env, "dynlib");
#endif
	if(!http_printf(fd, "# HELP unbound_memory_bytes Memory usage in "
		"bytes.\n")) return;
	if(!http_printf(fd, "# TYPE unbound_memory_bytes gauge\n")) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"cache_rrset", (unsigned long)rrset)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"cache_message", (unsigned long)msg)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"mod_iterator", (unsigned long)iter)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"mod_validator", (unsigned long)val)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"mod_respip", (unsigned long)respip)) return;
#ifdef CLIENT_SUBNET
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"mod_subnet", (unsigned long)subnet)) return;
#endif
#ifdef USE_IPSECMOD
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"mod_ipsecmod", (unsigned long)ipsecmod)) return;
#endif
#ifdef USE_DNSCRYPT
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"cache_dnscrypt_shared_secret",
		(unsigned long)dnscrypt_shared_secret)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"cache_dnscrypt_nonce", (unsigned long)dnscrypt_nonce))
		return;
#endif
#ifdef WITH_DYNLIBMODULE
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"mod_dynlibmod", (unsigned long)dynlib)) return;
#endif
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"streamwait", (unsigned long)s->svr.mem_stream_wait)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"http_query_buffer",
		(unsigned long)s->svr.mem_http2_query_buffer)) return;
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"http_response_buffer",
		(unsigned long)s->svr.mem_http2_response_buffer)) return;
#ifdef HAVE_NGTCP2
	if(!metrics_print_label_line(fd, "unbound_memory_bytes", "type",
		"quic", (unsigned long)s->svr.mem_quic)) return;
#endif
	if(!http_printf(fd, "\n")) return;

	/* histogram of reply times */
	hist = timehist_setup();
	if(!hist) {
		log_err("out of memory");
		return;
	}
	timehist_import(hist, s->svr.hist, NUM_BUCKETS_HIST);
	if(!http_printf(fd, "# HELP unbound_reply_time_seconds Histogram of "
		"the time it took to answer queries that needed recursive "
		"processing.\n")) { timehist_delete(hist); return; }
	if(!http_printf(fd, "# TYPE unbound_reply_time_seconds gauge\n")) {
		timehist_delete(hist); return; }
	for(i=0; i<(int)hist->num; i++) {
		if(!http_printf(fd,
			"unbound_reply_time_seconds{le_lower=\"%d.%6.6d\","
			"le_upper=\"%d.%6.6d\"} %lu\n",
			(int)hist->buckets[i].lower.tv_sec,
			(int)hist->buckets[i].lower.tv_usec,
			(int)hist->buckets[i].upper.tv_sec,
			(int)hist->buckets[i].upper.tv_usec,
			(unsigned long)hist->buckets[i].count)) {
			timehist_delete(hist);
			return;
		}
	}
	timehist_delete(hist);
	(void)http_printf(fd, "\n");
}

/** write the per-thread and merged total server statistics (as obtained
 * via stats_noreset) in prometheus exposition format */
static void
do_metrics(int fd, struct worker* worker)
{
	struct daemon* daemon = worker->daemon;
	struct ub_stats_info* stats;
	int i, n = daemon->num;

	stats = (struct ub_stats_info*)calloc((size_t)n+1, sizeof(*stats));
	if(!stats) {
		log_err("out of memory in do_metrics");
		return;
	}
	for(i=0; i<n; i++)
		server_stats_obtain(worker, daemon->workers[i], &stats[i], 0);
	if(n > 0) {
		stats[n] = stats[0];
		for(i=1; i<n; i++)
			server_stats_add(&stats[n], &stats[i]);
		stats[n].mesh_time_median /= (double)n;
	}

	METRICS_U64(fd, "unbound_queries_total",
		"Total number of queries received.", "counter",
		stats, n, _s->svr.num_queries);
	METRICS_U64(fd, "unbound_queries_ip_ratelimited_total",
		"Queries dropped because of the IP rate limit.", "counter",
		stats, n, _s->svr.num_queries_ip_ratelimited);
	METRICS_U64(fd, "unbound_queries_cookie_valid_total",
		"Queries with a valid DNS Cookie.", "counter",
		stats, n, _s->svr.num_queries_cookie_valid);
	METRICS_U64(fd, "unbound_queries_cookie_client_total",
		"Queries with only the client part of the DNS Cookie.",
		"counter", stats, n, _s->svr.num_queries_cookie_client);
	METRICS_U64(fd, "unbound_queries_cookie_invalid_total",
		"Queries with an invalid DNS Cookie.", "counter",
		stats, n, _s->svr.num_queries_cookie_invalid);
	METRICS_U64(fd, "unbound_queries_discard_timeout_total",
		"Queries removed because of the discard-timeout.",
		"counter", stats, n, _s->svr.num_queries_discard_timeout);
	METRICS_U64(fd, "unbound_queries_replyaddr_limit_total",
		"Queries removed because of the reply address limit.",
		"counter", stats, n, _s->svr.num_queries_replyaddr_limit);
	METRICS_U64(fd, "unbound_queries_wait_limit_total",
		"Queries removed because of the wait limit.", "counter",
		stats, n, _s->svr.num_queries_wait_limit);
	METRICS_U64(fd, "unbound_cache_hits_total",
		"Queries that were answered using a cached result.",
		"counter", stats, n,
		_s->svr.num_queries - _s->svr.num_queries_missed_cache);
	METRICS_U64(fd, "unbound_cache_miss_total",
		"Queries that needed recursive processing.", "counter",
		stats, n, _s->svr.num_queries_missed_cache);
	METRICS_U64(fd, "unbound_prefetch_total",
		"Cache prefetches performed.", "counter",
		stats, n, _s->svr.num_queries_prefetch);
	METRICS_U64(fd, "unbound_queries_timed_out_total",
		"Queries that were too late to process.", "counter",
		stats, n, _s->svr.num_queries_timed_out);
	METRICS_U64(fd, "unbound_query_queue_time_us_max",
		"Longest time a query had to wait in the queue, in "
		"microseconds.", "gauge", stats, n,
		_s->svr.max_query_time_us);
	METRICS_U64(fd, "unbound_expired_total",
		"Replies served from expired cache entries.", "counter",
		stats, n, _s->svr.ans_expired);
	METRICS_U64(fd, "unbound_recursive_replies_total",
		"Replies that needed recursive processing.", "counter",
		stats, n, _s->mesh_replies_sent);
	METRICS_U64(fd, "unbound_dns_error_reports_total",
		"DNS Error Reports generated.", "counter",
		stats, n, _s->svr.num_dns_error_reports);
	METRICS_DOUBLE(fd, "unbound_requestlist_avg",
		"Average number of requests in the internal request list.",
		"gauge", stats, n,
		(_s->svr.num_queries_missed_cache+_s->svr.num_queries_prefetch)?
			(double)_s->svr.sum_query_list_size/
			(double)(_s->svr.num_queries_missed_cache+
			_s->svr.num_queries_prefetch) : 0.0);
	METRICS_U64(fd, "unbound_requestlist_max",
		"Largest size of the internal request list.", "gauge",
		stats, n, _s->svr.max_query_list_size);
	METRICS_U64(fd, "unbound_requestlist_overwritten_total",
		"Request list entries overwritten by a newer one.",
		"counter", stats, n, _s->mesh_jostled);
	METRICS_U64(fd, "unbound_requestlist_exceeded_total",
		"Queries dropped because the request list was full.",
		"counter", stats, n, _s->mesh_dropped);
	METRICS_U64(fd, "unbound_requestlist_current_all",
		"Current number of request list entries, including "
		"internal states.", "gauge", stats, n, _s->mesh_num_states);
	METRICS_U64(fd, "unbound_requestlist_current_user",
		"Current number of request list entries from client "
		"queries.", "gauge", stats, n, _s->mesh_num_reply_states);
	METRICS_U64(fd, "unbound_requestlist_current_replies",
		"Current number of outstanding replies.", "gauge",
		stats, n, _s->mesh_num_reply_addrs);
	METRICS_DOUBLE(fd, "unbound_recursion_time_median_seconds",
		"Median time it took to answer queries that needed "
		"recursive processing.", "gauge", stats, n,
		_s->mesh_time_median);
	METRICS_U64(fd, "unbound_tcpusage",
		"Currently held tcp handler buffers.", "gauge",
		stats, n, _s->svr.tcp_accept_usage);
#ifdef USE_DNSCRYPT
	METRICS_U64(fd, "unbound_dnscrypt_crypted_total",
		"Queries that used DNSCrypt encryption.", "counter",
		stats, n, _s->svr.num_query_dnscrypt_crypted);
	METRICS_U64(fd, "unbound_dnscrypt_cert_total",
		"Queries for the DNSCrypt certificate.", "counter",
		stats, n, _s->svr.num_query_dnscrypt_cert);
	METRICS_U64(fd, "unbound_dnscrypt_cleartext_total",
		"Cleartext queries not asking for the certificate.",
		"counter", stats, n, _s->svr.num_query_dnscrypt_cleartext);
	METRICS_U64(fd, "unbound_dnscrypt_malformed_total",
		"Malformed DNSCrypt encrypted queries.", "counter",
		stats, n, _s->svr.num_query_dnscrypt_crypted_malformed);
#endif

	/* average recursion time, needs timeval_divide per entry */
	{
		int _mi;
		if(!http_printf(fd, "# HELP unbound_recursion_time_avg_seconds "
			"Average time it took to answer queries that needed "
			"recursive processing.\n")) goto done;
		if(!http_printf(fd, "# TYPE unbound_recursion_time_avg_seconds "
			"gauge\n")) goto done;
		for(_mi=0; _mi <= n; _mi++) {
			struct timeval sumwait, avg;
			struct ub_stats_info* _s = &stats[_mi];
#ifndef S_SPLINT_S
			sumwait.tv_sec = _s->mesh_replies_sum_wait_sec;
			sumwait.tv_usec = _s->mesh_replies_sum_wait_usec;
#endif
			timeval_divide(&avg, &sumwait, _s->mesh_replies_sent);
			if(_mi == n) {
				if(!http_printf(fd,
					"unbound_recursion_time_avg_seconds"
					"{thread=\"total\"} %d.%6.6d\n",
					(int)avg.tv_sec, (int)avg.tv_usec))
					goto done;
			} else {
				if(!http_printf(fd,
					"unbound_recursion_time_avg_seconds"
					"{thread=\"%d\"} %d.%6.6d\n", _mi,
					(int)avg.tv_sec, (int)avg.tv_usec))
					goto done;
			}
		}
		if(!http_printf(fd, "\n")) goto done;
	}

	/* uptime, this is global, not per thread */
	{
		struct timeval now = *worker->env.now_tv;
		struct timeval up;
		timeval_subtract(&up, &now, &daemon->time_boot);
		if(!http_printf(fd, "# HELP unbound_uptime_seconds Time "
			"since Unbound was started.\n")) goto done;
		if(!http_printf(fd, "# TYPE unbound_uptime_seconds gauge\n"))
			goto done;
		if(!http_printf(fd, "unbound_uptime_seconds %d.%6.6d\n\n",
			(int)up.tv_sec, (int)up.tv_usec)) goto done;
	}

	if(daemon->cfg->stat_extended)
		do_metrics_extended(fd, worker, daemon, &stats[n]);
done:
	free(stats);
}

/** write the http response headers for a 200 OK metrics reply */
static int
metrics_send_header(int fd)
{
	if(!http_printf(fd, "HTTP/1.1 200 OK\r\n")) return 0;
	if(!http_printf(fd, "Content-Type: text/plain; "
		"version=0.0.4; charset=utf-8\r\n")) return 0;
	if(!http_printf(fd, "Connection: close\r\n\r\n")) return 0;
	return 1;
}

/** write a short http error response */
static int
metrics_send_error(int fd, const char* status)
{
	if(!http_printf(fd, "HTTP/1.1 %s\r\n", status)) return 0;
	if(!http_printf(fd, "Content-Type: text/plain; charset=utf-8\r\n"))
		return 0;
	if(!http_printf(fd, "Connection: close\r\n\r\n")) return 0;
	return http_printf(fd, "%s\n", status);
}

/** handle a metrics http request: parse the request line and headers,
 * and if the path matches metrics-path, write the metrics response */
static void
metrics_handle_req(struct daemon_metrics* dm, struct metrics_state* s)
{
	int fd = s->c->fd;
	char buf[MAX_METRICS_STRLINE];
	char path[MAX_METRICS_STRLINE];
	char* sp, *qs;
	size_t plen;
#ifdef USE_WINSOCK
	/* makes it possible to set the socket blocking again. */
	WSAEventSelect(s->c->fd, NULL, 0);
#endif
	fd_set_block(fd);

	if(!http_read_line(fd, buf, sizeof(buf)))
		return;
	if(strncmp(buf, "GET ", 4) != 0) {
		verbose(VERB_ALGO, "metrics request: unsupported method");
		(void)metrics_send_error(fd, "405 Method Not Allowed");
		return;
	}
	sp = buf+4;
	qs = strchr(sp, ' ');
	if(!qs) {
		(void)metrics_send_error(fd, "400 Bad Request");
		return;
	}
	plen = (size_t)(qs-sp);
	if(plen == 0 || plen >= sizeof(path)) {
		(void)metrics_send_error(fd, "400 Bad Request");
		return;
	}
	memmove(path, sp, plen);
	path[plen] = 0;
	/* strip a query string, if present */
	qs = strchr(path, '?');
	if(qs) *qs = 0;

	/* read and ignore header lines, until the blank line */
	while(1) {
		if(!http_read_line(fd, buf, sizeof(buf)))
			return;
		if(buf[0] == 0)
			break;
	}

	if(strcmp(path, dm->worker->daemon->cfg->metrics_path) != 0) {
		verbose(VERB_ALGO, "metrics request for unknown path: %s",
			path);
		(void)metrics_send_error(fd, "404 Not Found");
		return;
	}
	if(!metrics_send_header(fd))
		return;
	do_metrics(fd, dm->worker);
}

/** delete metrics_state from busy list and close its comm point */
static void
metrics_clean_point(struct daemon_metrics* dm, struct metrics_state* s)
{
	struct metrics_state** prev = &dm->busy_list;
	while(*prev) {
		if(*prev == s) {
			*prev = s->next;
			break;
		}
		prev = &(*prev)->next;
	}
	dm->active --;
	comm_point_delete(s->c);
	free(s);
}

struct daemon_metrics*
daemon_metrics_create(struct config_file* ATTR_UNUSED(cfg))
{
	struct daemon_metrics* dm = (struct daemon_metrics*)calloc(1,
		sizeof(*dm));
	if(!dm) {
		log_err("out of memory in daemon_metrics_create");
		return NULL;
	}
	dm->max_active = 10;
	return dm;
}

void daemon_metrics_clear(struct daemon_metrics* dm)
{
	struct metrics_state* p, *np;
	if(!dm) return;
	/* but do not close the ports */
	listen_list_delete(dm->accept_list);
	dm->accept_list = NULL;
	/* do close these sockets */
	p = dm->busy_list;
	while(p) {
		np = p->next;
		comm_point_delete(p->c);
		free(p);
		p = np;
	}
	dm->busy_list = NULL;
	dm->active = 0;
	dm->worker = NULL;
}

void daemon_metrics_delete(struct daemon_metrics* dm)
{
	if(!dm) return;
	daemon_metrics_clear(dm);
	free(dm);
}

/**
 * Add and open a new metrics listening port.
 * @param ip: ip str, or local socket path (starting with a '/').
 * @param nr: port nr
 * @param list: list head
 * @param noproto_is_err: if lack of protocol support is an error.
 * @param cfg: config with username for chown of unix-sockets.
 * @return false on failure.
 */
static int
metrics_add_open(const char* ip, int nr, struct listen_port** list,
	int noproto_is_err, struct config_file* cfg)
{
	struct addrinfo hints;
	struct addrinfo* res;
	struct listen_port* n;
	int noproto = 0;
	int fd, r;
	char port[15];
	snprintf(port, sizeof(port), "%d", nr);
	port[sizeof(port)-1]=0;
	memset(&hints, 0, sizeof(hints));
	log_assert(ip);

	if(ip[0] == '/') {
		/* This looks like a local socket */
		fd = create_local_accept_sock(ip, &noproto, cfg->use_systemd);
		/*
		 * Change socket ownership and permissions so users other
		 * than root can access it provided they are in the same
		 * group as the user we run as.
		 */
		if(fd != -1) {
#ifdef HAVE_CHOWN
			chmod(ip, (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
			if (cfg->username && cfg->username[0] &&
				cfg_uid != (uid_t)-1) {
				if(chown(ip, cfg_uid, cfg_gid) == -1)
					verbose(VERB_QUERY, "cannot chown %u.%u %s: %s",
					  (unsigned)cfg_uid, (unsigned)cfg_gid,
					  ip, strerror(errno));
			}
#else
			(void)cfg;
#endif
		}
	} else {
		const char* s = strchr(ip, '@');
		char newif[128];
		if(s) {
			/* override port with ifspec@port */
			int portnr;
			if((size_t)(s-ip) >= sizeof(newif)) {
				log_err("ifname too long: %s", ip);
				return 0;
			}
			portnr = atoi(s+1);
			if(portnr <= 0 || portnr > 65535) {
				log_err("invalid portnumber in metrics-interface: %s",
					ip);
				return 0;
			}
			(void)strlcpy(newif, ip, sizeof(newif));
			newif[s-ip] = 0;
			ip = newif;
			snprintf(port, sizeof(port), "%d", portnr);
			port[sizeof(port)-1]=0;
		}
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
		if((r = getaddrinfo(ip, port, &hints, &res)) != 0 || !res) {
#ifdef USE_WINSOCK
			if(!noproto_is_err && r == EAI_NONAME) {
				/* tried to lookup the address as name */
				return 1; /* return success, but do nothing */
			}
#endif /* USE_WINSOCK */
			log_err("metrics interface %s:%s getaddrinfo: %s %s",
				ip?ip:"default", port, gai_strerror(r),
#ifdef EAI_SYSTEM
				r==EAI_SYSTEM?(char*)strerror(errno):""
#else
				""
#endif
			);
			return 0;
		}

		/* open fd */
		fd = create_tcp_accept_sock(res, 1, &noproto, 0,
			cfg->ip_transparent, 0, 0, cfg->ip_freebind,
			cfg->use_systemd, cfg->ip_dscp, "unbound-metrics");
		freeaddrinfo(res);
	}

	if(fd == -1 && noproto) {
		if(!noproto_is_err)
			return 1; /* return success, but do nothing */
		log_err("cannot open metrics interface %s %d : "
			"protocol not supported", ip, nr);
		return 0;
	}
	if(fd == -1) {
		log_err("cannot open metrics interface %s %d", ip, nr);
		return 0;
	}

	/* alloc */
	n = (struct listen_port*)calloc(1, sizeof(*n));
	if(!n) {
		sock_close(fd);
		log_err("out of memory");
		return 0;
	}
	n->next = *list;
	*list = n;
	n->fd = fd;
	return 1;
}

struct listen_port* daemon_metrics_open_ports(struct config_file* cfg)
{
	struct listen_port* l = NULL;
	log_assert(cfg->metrics_enable && cfg->metrics_port);
	if(cfg->metrics_ifs.first) {
		char** rcif = NULL;
		int i, num_rcif = 0;
		if(!resolve_interface_names(NULL, 0, cfg->metrics_ifs.first,
			&rcif, &num_rcif)) {
			return NULL;
		}
		for(i=0; i<num_rcif; i++) {
			if(!metrics_add_open(rcif[i], cfg->metrics_port, &l,
				1, cfg)) {
				listening_ports_free(l);
				config_del_strarray(rcif, num_rcif);
				return NULL;
			}
		}
		config_del_strarray(rcif, num_rcif);
	} else {
		/* defaults, same as for the remote control interface */
		if(cfg->do_ip6 &&
			!metrics_add_open("::1", cfg->metrics_port, &l, 0, cfg)) {
			listening_ports_free(l);
			return NULL;
		}
		if(cfg->do_ip4 &&
			!metrics_add_open("127.0.0.1", cfg->metrics_port, &l,
				1, cfg)) {
			listening_ports_free(l);
			return NULL;
		}
	}
	return l;
}

/** open accept commpoint for metrics */
static int
metrics_accept_open(struct daemon_metrics* dm, int fd)
{
	struct listen_list* n = (struct listen_list*)malloc(sizeof(*n));
	if(!n) {
		log_err("out of memory");
		return 0;
	}
	n->next = dm->accept_list;
	dm->accept_list = n;
	/* open commpt */
	n->com = comm_point_create_raw(dm->worker->base, fd, 0,
		&metrics_accept_callback, dm);
	if(!n->com)
		return 0;
	/* keep this port open, its fd is kept in the dm portlist */
	n->com->do_not_close = 1;
	return 1;
}

int daemon_metrics_open_accept(struct daemon_metrics* dm,
	struct listen_port* ports, struct worker* worker)
{
	struct listen_port* p;
	dm->worker = worker;
	for(p = ports; p; p = p->next) {
		if(!metrics_accept_open(dm, p->fd)) {
			log_err("could not create metrics accept comm point");
			return 0;
		}
	}
	return 1;
}

void daemon_metrics_stop_accept(struct daemon_metrics* dm)
{
	struct listen_list* p;
	for(p=dm->accept_list; p; p=p->next) {
		comm_point_stop_listening(p->com);
	}
}

void daemon_metrics_start_accept(struct daemon_metrics* dm)
{
	struct listen_list* p;
	for(p=dm->accept_list; p; p=p->next) {
		comm_point_start_listening(p->com, -1, -1);
	}
}

int metrics_control_callback(struct comm_point* ATTR_UNUSED(c), void* arg,
	int err, struct comm_reply* ATTR_UNUSED(rep))
{
	struct metrics_state* s = (struct metrics_state*)arg;
	struct daemon_metrics* dm = s->dm;
	if(err != NETEVENT_NOERROR) {
		if(err==NETEVENT_TIMEOUT)
			log_err("metrics connection timed out");
		metrics_clean_point(dm, s);
		return 0;
	}
	metrics_handle_req(dm, s);

	verbose(VERB_ALGO, "metrics http request completed");
	metrics_clean_point(dm, s);
	return 0;
}

int metrics_accept_callback(struct comm_point* c, void* arg,
	int err, struct comm_reply* ATTR_UNUSED(rep))
{
	struct daemon_metrics* dm = (struct daemon_metrics*)arg;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int newfd;
	struct metrics_state* n;
	if(err != NETEVENT_NOERROR) {
		log_err("error %d on metrics_accept_callback", err);
		return 0;
	}
	/* perform the accept */
	newfd = comm_point_perform_accept(c, &addr, &addrlen);
	if(newfd == -1)
		return 0;
	/* create new commpoint unless we are servicing already */
	if(dm->active >= dm->max_active) {
		log_warn("drop incoming metrics connection: too many "
			"connections");
		sock_close(newfd);
		return 0;
	}

	n = (struct metrics_state*)calloc(1, sizeof(*n));
	if(!n) {
		log_err("out of memory");
		sock_close(newfd);
		return 0;
	}
	n->fd = newfd;
	n->dm = dm;
	n->c = comm_point_create_raw(dm->worker->base, newfd, 0,
		&metrics_control_callback, n);
	if(!n->c) {
		log_err("out of memory");
		free(n);
		sock_close(newfd);
		return 0;
	}
	log_addr(VERB_QUERY, "new metrics connection from", &addr, addrlen);
	n->c->do_not_close = 0;
	comm_point_stop_listening(n->c);
	comm_point_start_listening(n->c, -1, METRICS_TCP_TIMEOUT);
	memcpy(&n->c->repinfo.remote_addr, &addr, addrlen);
	n->c->repinfo.remote_addrlen = addrlen;

	n->next = dm->busy_list;
	dm->busy_list = n;
	dm->active ++;

	/* perform the first nonblocking read already, for windows,
	 * so it can return wouldblock. could be faster too. */
	(void)metrics_control_callback(n->c, n, NETEVENT_NOERROR, NULL);
	return 0;
}
