/*
 * daemon/metrics.c - prometheus metrics endpoint.
 *
 * Copyright (c) 2026, NLnet Labs. All rights reserved.
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
 * The statistics output provides metrics to prometheus.
 */
#include "config.h"
#include "daemon/metrics.h"
#include "daemon/daemon.h"
#include "daemon/worker.h"
#include "daemon/stats.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "util/ub_event.h"
#include "util/timeval_func.h"
#include "services/listen_dnsport.h"
#include "services/cache/rrset.h"

/* If there is no metrics enabled, do not add the code. */
#ifdef USE_METRICS
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>

/** The prefix for the unbound statistics. */
#define METRICS_PREFIX "unbound_"

/**
 * list of connection accepting file descriptors
 */
struct metrics_acceptlist {
	struct metrics_acceptlist* next;
	int accept_fd;
	char* ident;
	struct daemon_metrics* metrics;
};

/**
 * The metrics daemon state.
 */
struct daemon_metrics {
	/** The worker for this metrics endpoint */
	struct worker* worker;
	/** commpoints for accepting HTTP connections */
	struct metrics_acceptlist* accept_list;
	/** libevent http server */
	struct evhttp *http_server;
};

/** The callback that handles a metrics http request. */
static void metrics_http_callback(struct evhttp_request *req, void *p);

struct daemon_metrics*
daemon_metrics_create(void)
{
	struct daemon_metrics* metrics = (struct daemon_metrics*)calloc(
		sizeof(*metrics), 1);
	if(!metrics) {
		log_err("out of memory");
		return NULL;
	}
	return metrics;
}

void
daemon_metrics_delete(struct daemon_metrics* metrics)
{
	if(!metrics) return;
	daemon_metrics_detach(metrics);
	daemon_metrics_close_ports(metrics);
	free(metrics);
}

void
daemon_metrics_close_ports(struct daemon_metrics* metrics)
{
	struct metrics_acceptlist *h, *nh;
	if(!metrics) return;

	/* close listen sockets */
	h = metrics->accept_list;
	while(h) {
		nh = h->next;
		close(h->accept_fd);
		free(h->ident);
		free(h);
		h = nh;
	}
	metrics->accept_list = NULL;
}

void
daemon_metrics_detach(struct daemon_metrics* metrics)
{
	if(!metrics) return;
	if (metrics->http_server) {
		evhttp_free(metrics->http_server);
		metrics->http_server = NULL;
	}
}

/**
 * Add and open a new metrics port
 * @param metrics: metrics with result list.
 * @param cfg: config options.
 * @param ip: ip str
 * @param nr: port nr
 * @param noproto_is_err: if lack of protocol support is an error.
 * @return false on failure.
 */
static int
metrics_add_open(struct daemon_metrics* metrics, struct config_file* cfg,
	const char* ip, int nr, int noproto_is_err)
{
	struct addrinfo hints;
	struct addrinfo* res;
	struct metrics_acceptlist* hl;
	int noproto = 0;
	int fd, r;
	char port[15];
	snprintf(port, sizeof(port), "%d", nr);
	port[sizeof(port)-1]=0;
	memset(&hints, 0, sizeof(hints));
	assert(ip);

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
			if(chmod(ip, (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1) {
				verbose(VERB_QUERY, "cannot chmod metrics socket %s: %s", ip, strerror(errno));
			}
			if (cfg->username && cfg->username[0] &&
				cfg_uid != (uid_t)-1) {
				if(chown(ip, cfg_uid, cfg_gid) == -1)
					verbose(VERB_QUERY, "cannot chown metrics socket %u.%u %s: %s",
					  (unsigned)cfg_uid, (unsigned)cfg_gid,
					  ip, strerror(errno));
			}
#else
			(void)cfg;
#endif
		}
	} else {
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
		/* if we had no interface ip name, "default" is what we
		 * would do getaddrinfo for. */
		if((r = getaddrinfo(ip, port, &hints, &res)) != 0 || !res) {
#ifdef USE_WINSOCK
			if(!noproto_is_err && r == EAI_NONAME) {
				/* tried to lookup the address as name */
				return 1; /* return success, but do nothing */
			}
#endif /* USE_WINSOCK */
			log_err("metrics interface %s:%s getaddrinfo: %s %s",
				ip, port, gai_strerror(r),
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
			cfg->use_systemd, cfg->ip_dscp, "metrics");
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
	hl = (struct metrics_acceptlist*)calloc(1, sizeof(*hl));
	if(!hl) {
		sock_close(fd);
		log_err("out of memory");
		return 0;
	}
	hl->metrics = metrics;
	hl->ident = strdup(ip);
	if(!hl->ident) {
		log_err("out of memory");
		sock_close(fd);
		free(hl);
		return 0;
	}
	hl->next = metrics->accept_list;
	metrics->accept_list = hl;

	hl->accept_fd = fd;
	return 1;
}

int
daemon_metrics_open_ports(struct daemon_metrics* metrics,
	struct config_file* cfg)
{
	assert(cfg->metrics_enable);
	if(cfg->metrics_ifs.first) {
		struct config_strlist* p;
		for(p = cfg->metrics_ifs.first; p; p = p->next) {
			if(!metrics_add_open(metrics, cfg, p->str,
				cfg->metrics_port, 1)) {
				return 0;
			}
		}
	} else {
		/* defaults */
		if(cfg->do_ip6 && !metrics_add_open(metrics, cfg, "::1",
			cfg->metrics_port, 0)) {
			return 0;
		}
		if(cfg->do_ip4 &&
			!metrics_add_open(metrics, cfg, "127.0.0.1",
			cfg->metrics_port, 1)) {
			return 0;
		}
	}
	return 1;
}

int
daemon_metrics_attach(struct daemon_metrics* metrics, struct worker* worker)
{
	int fd;
	struct metrics_acceptlist* p;
	if(!metrics) return 1;
	metrics->worker = worker;
	if(!metrics->accept_list)
		return 1;

	metrics->http_server = evhttp_new(ub_libevent_get_event_base(
		comm_base_internal(worker->base)));
	if(!metrics->http_server) {
		log_err("out of memory, evhttp_new failed");
		return 0;
	}
	for(p = metrics->accept_list; p; p = p->next) {
		fd = p->accept_fd;
		if (evhttp_accept_socket(metrics->http_server, fd)) {
			log_err("metrics: cannot set http server to accept socket");
			return 0;
		}

		/* only handle requests to metrics_path, anything else returns 404 */
		evhttp_set_cb(metrics->http_server,
			worker->daemon->cfg->metrics_path,
			metrics_http_callback, p);
		/* evhttp_set_gencb(metrics->http_server, metrics_http_callback_generic, p); */
	}
	return 1;
}

/* Add help and type printout of a metric. */
static void
print_metric_help_and_type(struct evbuffer *buf, char *prefix, char *name,
	char *help, char *type)
{
	evbuffer_add_printf(buf, "# HELP %s%s %s\n# TYPE %s%s %s\n",
		prefix, name, help, prefix, name, type);
}

/* print help and type for main list of metrics */
static int
metrics_print_types(struct evbuffer *reply)
{
	char* prefix = METRICS_PREFIX;
	print_metric_help_and_type(reply, prefix, "hits_queries",
		"Unbound DNS traffic and cache hits", "gauge");
	print_metric_help_and_type(reply, prefix, "queue_queries",
		"Unbound requestlist size", "gauge");
	print_metric_help_and_type(reply, prefix, "recursion_time",
		"Unbound recursion time, in seconds", "gauge");
	print_metric_help_and_type(reply, prefix, "query_queue_time",
		"Unbound query queue time, in msec", "gauge");
	print_metric_help_and_type(reply, prefix, "socket_count",
		"Unbound socket count", "gauge");
	return 1;
}

/* metrics print of stat block */
static int
metrics_print_stats(struct evbuffer* reply, const char* nm,
	struct ub_stats_info* s)
{
	char* prefix = METRICS_PREFIX;
	struct timeval sumwait, avg;

	/* print to reply buffer the stat for prefix mt
	 * of type nm.snm and long long output svar. */
#define INFO_STATS(mt, snm, svar) \
	evbuffer_add_printf(reply, \
		"%s" mt "{type=\"%s." snm "\"} " ARG_LL "d\n", \
		prefix, nm, (long long)(svar))

	INFO_STATS("hits_queries", "num.queries", s->svr.num_queries);
	INFO_STATS("hits_queries", "num.queries_ip_ratelimited",
		s->svr.num_queries_ip_ratelimited);
	INFO_STATS("hits_queries", "num.queries_cookie_valid",
		s->svr.num_queries_cookie_valid);
	INFO_STATS("hits_queries", "num.queries_cookie_client",
		s->svr.num_queries_cookie_client);
	INFO_STATS("hits_queries", "num.queries_cookie_invalid",
		s->svr.num_queries_cookie_invalid);
	INFO_STATS("hits_queries", "num.queries_discard_timeout",
		s->svr.num_queries_discard_timeout);
	INFO_STATS("hits_queries", "num.queries_replyaddr_limit",
		s->svr.num_queries_replyaddr_limit);
	INFO_STATS("hits_queries", "num.queries_wait_limit",
		s->svr.num_queries_wait_limit);
	INFO_STATS("hits_queries", "num.cachehits",
		s->svr.num_queries - s->svr.num_queries_missed_cache);
	INFO_STATS("hits_queries", "num.cachemiss",
		s->svr.num_queries_missed_cache);
	INFO_STATS("hits_queries", "num.prefetch",
		s->svr.num_queries_prefetch);
	INFO_STATS("hits_queries", "num.queries_timed_out",
		s->svr.num_queries_timed_out);
	INFO_STATS("hits_queries", "num.expired", s->svr.ans_expired);
	INFO_STATS("hits_queries", "num.recursivereplies",
		s->mesh_replies_sent);
#ifdef USE_DNSCRYPT
	INFO_STATS("hits_queries", "num.dnscrypt.crypted",
		s->svr.num_query_dnscrypt_crypted);
	INFO_STATS("hits_queries", "num.dnscrypt.cert",
		s->svr.num_query_dnscrypt_cert);
	INFO_STATS("hits_queries", "num.dnscrypt.cleartext",
		s->svr.num_query_dnscrypt_cleartext);
	INFO_STATS("hits_queries", "num.dnscrypt.malformed",
		s->svr.num_query_dnscrypt_crypted_malformed);
#endif
	INFO_STATS("hits_queries", "num.dns_error_reports",
		s->svr.num_dns_error_reports);

	evbuffer_add_printf(reply,
		"%squeue_queries{type=\"%s.requestlist.avg\"} %g\n",
		prefix, nm,
		(s->svr.num_queries_missed_cache+s->svr.num_queries_prefetch)?
			(double)s->svr.sum_query_list_size/
			(double)(s->svr.num_queries_missed_cache+
			s->svr.num_queries_prefetch) : 0.0);
	INFO_STATS("queue_queries", "requestlist.max",
		s->svr.max_query_list_size);
	INFO_STATS("queue_queries", "requestlist.overwritten",
		s->mesh_jostled);
	INFO_STATS("queue_queries", "requestlist.exceeded",
		s->mesh_dropped);
	INFO_STATS("queue_queries", "requestlist.current.all",
		s->mesh_num_states);
	INFO_STATS("queue_queries", "requestlist.current.user",
		s->mesh_num_reply_states);
	INFO_STATS("queue_queries", "requestlist.current.replies",
		s->mesh_num_reply_addrs);

	sumwait.tv_sec = s->mesh_replies_sum_wait_sec;
	sumwait.tv_usec = s->mesh_replies_sum_wait_usec;
	timeval_divide(&avg, &sumwait, s->mesh_replies_sent);
	evbuffer_add_printf(reply,
		"%srecursion_time{type=\"%s.recursion.time.avg\"} " ARG_LL
		"d.%6.6d\n", prefix, nm,
		(long long)avg.tv_sec, (int)avg.tv_usec);
	evbuffer_add_printf(reply,
		"%srecursion_time{type=\"%s.recursion.time.median\"} %g\n",
		prefix, nm, s->mesh_time_median);

	INFO_STATS("query_queue_time", "query.queue_time_us.max",
		s->svr.max_query_time_us);

	INFO_STATS("socket_count", "tcpusage", s->svr.tcp_accept_usage);
	return 1;
}

/* metrics print of thread stats */
static int
metrics_print_thread_stats(struct evbuffer* reply, int i,
	struct ub_stats_info* s)
{
	char nm[32];
	snprintf(nm, sizeof(nm), "thread%d", i);
	nm[sizeof(nm)-1]=0;
	return metrics_print_stats(reply, nm, s);
}

/* metrics print of uptime stats */
static int
metrics_print_uptime(struct evbuffer* reply, struct worker* worker,
	struct timeval* stattime, struct timeval* time_last_stat)
{
	char* prefix = METRICS_PREFIX;
	struct timeval up, dt;
	timeval_subtract(&up, stattime, &worker->daemon->time_boot);
	timeval_subtract(&dt, stattime, time_last_stat);

	print_metric_help_and_type(reply, prefix, "time_now_seconds",
		"Time of the statistics printout, in seconds.", "untyped");
	evbuffer_add_printf(reply, "%stime_now_seconds " ARG_LL "d.%6.6u\n",
		prefix, (long long)stattime->tv_sec,
		(unsigned)stattime->tv_usec);

	print_metric_help_and_type(reply, prefix, "time_up_seconds_total",
		"Uptime since server boot in seconds.", "counter");
	evbuffer_add_printf(reply,
		"%stime_up_seconds_total " ARG_LL "d.%6.6u\n",
		prefix, (long long)up.tv_sec, (unsigned)up.tv_usec);

	print_metric_help_and_type(reply, prefix, "time_elapsed_seconds",
		"Time since last statistics printout and "
		"reset (by unbound-control stats) in seconds.",
		"untyped");
	evbuffer_add_printf(reply,
		"%stime_elapsed_seconds " ARG_LL "d.%6.6u\n",
		prefix, (long long)dt.tv_sec, (unsigned)dt.tv_usec);
	return 1;
}

/* metrics print of mem stats */
static int
metrics_print_mem(struct evbuffer* reply, struct worker* worker,
	struct daemon* daemon, struct ub_stats_info* s)
{
	char* prefix = METRICS_PREFIX;
	size_t msg, rrset, val, iter, respip;
#ifdef CLIENT_SUBNET
	size_t subnet = 0;
#endif /* CLIENT_SUBNET */
#ifdef USE_IPSECMOD
	size_t ipsecmod = 0;
#endif /* USE_IPSECMOD */
#ifdef USE_DNSCRYPT
	size_t dnscrypt_shared_secret = 0;
	size_t dnscrypt_nonce = 0;
#endif /* USE_DNSCRYPT */
#ifdef WITH_DYNLIBMODULE
    size_t dynlib = 0;
#endif /* WITH_DYNLIBMODULE */
	msg = slabhash_get_mem(daemon->env->msg_cache);
	rrset = slabhash_get_mem(&daemon->env->rrset_cache->table);
	val = mod_get_mem(&worker->env, "validator");
	iter = mod_get_mem(&worker->env, "iterator");
	respip = mod_get_mem(&worker->env, "respip");
#ifdef CLIENT_SUBNET
	subnet = mod_get_mem(&worker->env, "subnetcache");
#endif /* CLIENT_SUBNET */
#ifdef USE_IPSECMOD
	ipsecmod = mod_get_mem(&worker->env, "ipsecmod");
#endif /* USE_IPSECMOD */
#ifdef USE_DNSCRYPT
	if(daemon->dnscenv) {
		dnscrypt_shared_secret = slabhash_get_mem(
			daemon->dnscenv->shared_secrets_cache);
		dnscrypt_nonce = slabhash_get_mem(daemon->dnscenv->nonces_cache);
	}
#endif /* USE_DNSCRYPT */
#ifdef WITH_DYNLIBMODULE
    dynlib = mod_get_mem(&worker->env, "dynlib");
#endif /* WITH_DYNLIBMODULE */

	/* print to reply buffer the stat for prefix mt
	 * of type snm and long long output svar. */
#define INFO_LL_STATS(mt, snm, svar) \
	evbuffer_add_printf(reply, \
		"%s" mt "{type=\"" snm "\"} " ARG_LL "d\n", \
		prefix, (long long)(svar))

	print_metric_help_and_type(reply, prefix, "memory_bytes",
		"Unbound memory usage, in bytes", "gauge");
	INFO_LL_STATS("memory_bytes", "mem.cache.rrset", rrset);
	INFO_LL_STATS("memory_bytes", "mem.cache.message", msg);
	INFO_LL_STATS("memory_bytes", "mem.mod.iterator", iter);
	INFO_LL_STATS("memory_bytes", "mem.mod.validator", val);
	INFO_LL_STATS("memory_bytes", "mem.mod.respip", respip);
#ifdef CLIENT_SUBNET
	INFO_LL_STATS("memory_bytes", "mem.mod.subnet", subnet);
#endif /* CLIENT_SUBNET */
#ifdef USE_IPSECMOD
	INFO_LL_STATS("memory_bytes", "mem.mod.ipsecmod", ipsecmod);
#endif /* USE_IPSECMOD */
#ifdef USE_DNSCRYPT
	INFO_LL_STATS("memory_bytes", "mem.cache.dnscrypt_shared_secret",
		dnscrypt_shared_secret);
	INFO_LL_STATS("memory_bytes", "mem.cache.dnscrypt_nonce",
		dnscrypt_nonce);
#endif /* USE_DNSCRYPT */
#ifdef WITH_DYNLIBMODULE
	INFO_LL_STATS("memory_bytes", "mem.mod.dynlibmod", dynlib);
#endif /* WITH_DYNLIBMODULE */
	INFO_LL_STATS("memory_bytes", "mem.streamwait",
		s->svr.mem_stream_wait);
	INFO_LL_STATS("memory_bytes", "mem.http.query_buffer",
		s->svr.mem_http2_query_buffer);
	INFO_LL_STATS("memory_bytes", "mem.http.response_buffer",
		s->svr.mem_http2_response_buffer);
#ifdef HAVE_NGTCP2
	INFO_LL_STATS("memory_bytes", "mem.quic", s->svr.mem_quic);
#endif /* HAVE_NGTCP2 */
	return 1;
}

/* metrics print of histogram */
static int
metrics_print_hist(struct evbuffer* reply, struct ub_stats_info* s)
{
	char* prefix = METRICS_PREFIX;
	struct timehist* hist;
	size_t i;

	print_metric_help_and_type(reply, prefix, "histogram_seconds",
		"Unbound DNS histogram of reply time", "gauge");

	hist = timehist_setup();
	if(!hist) {
		log_err("out of memory");
		return 0;
	}
	timehist_import(hist, s->svr.hist, NUM_BUCKETS_HIST);
	for(i=0; i<hist->num; i++) {
		evbuffer_add_printf(reply, "%shistogram_seconds"
			"{bucket=\"%6.6d.%6.6d.to.%6.6d.%6.6d\"} %lu\n",
			prefix,
			(int)hist->buckets[i].lower.tv_sec,
			(int)hist->buckets[i].lower.tv_usec,
			(int)hist->buckets[i].upper.tv_sec,
			(int)hist->buckets[i].upper.tv_usec,
			(unsigned long)hist->buckets[i].count);
	}
	timehist_delete(hist);
	return 1;
}

/* metrics print of extended stats */
static int
metrics_print_ext(struct evbuffer* reply, struct ub_stats_info* s,
	int inhibit_zero)
{
	(void)reply;
	(void)s;
	(void)inhibit_zero;
	return 1;
}

/* process statistics */
static void
do_metrics_stats(struct evbuffer* reply, struct worker* worker, int reset)
{
	struct daemon* daemon = worker->daemon;
	struct ub_stats_info total;
	struct ub_stats_info s;
	int i;
	struct timeval stattime, time_last_stat;

	memset(&total, 0, sizeof(total));
	log_assert(daemon->num > 0);

	if(!metrics_print_types(reply))
		return;

	/* gather all thread statistics in one place */
	for(i=0; i<daemon->num; i++) {
		server_stats_obtain(worker, daemon->workers[i], &s, reset);
		if(!metrics_print_thread_stats(reply, i, &s))
			return;
		if(i == 0)
			total = s;
		else	server_stats_add(&total, &s);
	}
	total.mesh_time_median /= (double)daemon->num;
	if(gettimeofday(&stattime, NULL) < 0)
		log_err("gettimeofday: %s", strerror(errno));
	time_last_stat = worker->daemon->time_last_stat;
	if(reset) {
		worker->daemon->time_last_stat = stattime;
	}

	/* print the statistics */
	if(!metrics_print_stats(reply, "total", &total))
		return;
	if(!metrics_print_uptime(reply, worker, &stattime, &time_last_stat))
		return;
	if(daemon->cfg->stat_extended) {
		if(!metrics_print_mem(reply, worker, daemon, &total))
			return;
		if(!metrics_print_hist(reply, &total))
			return;
		if(!metrics_print_ext(reply, &total,
			daemon->cfg->stat_inhibit_zero))
			return;
	}
}

/* Callback for handling the active http request to the specific URI */
static void
metrics_http_callback(struct evhttp_request *req, void *p)
{
	struct evbuffer *reply = NULL;
	struct daemon_metrics *metrics = ((struct metrics_acceptlist *)p)->metrics;

	/* currently only GET requests are supported/allowed */
	enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
	if (cmd != EVHTTP_REQ_GET /* && cmd != EVHTTP_REQ_HEAD */) {
		evhttp_send_error(req, HTTP_BADMETHOD, 0);
		return;
	}

	reply = evbuffer_new();

	if (!reply) {
		evhttp_send_error(req, HTTP_INTERNAL, 0);
		log_err("metrics: failed to allocate reply buffer\n");
		return;
	}

	evhttp_add_header(evhttp_request_get_output_headers(req),
			  "Content-Type", "text/plain; version=0.0.4");
	do_metrics_stats(reply, metrics->worker,
		!metrics->worker->daemon->cfg->stat_cumulative);
	evhttp_send_reply(req, HTTP_OK, NULL, reply);
	verbose(VERB_DETAIL, "metrics operation completed, response sent");
	evbuffer_free(reply);
}
#endif /* USE_METRICS */
