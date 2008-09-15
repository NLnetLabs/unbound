/*
 * daemon/stats.c - collect runtime performance indicators.
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
 * This file describes the data structure used to collect runtime performance
 * numbers. These 'statistics' may be of interest to the operator.
 */
#include "config.h"
#include "daemon/stats.h"
#include "daemon/worker.h"
#include "daemon/daemon.h"
#include "services/mesh.h"
#include "util/config_file.h"
#include "util/tube.h"
#include "util/timehist.h"

/** add timers and the values do not overflow or become negative */
static void
timeval_add(struct timeval* d, struct timeval* add)
{
#ifndef S_SPLINT_S
	d->tv_sec += add->tv_sec;
	d->tv_usec += add->tv_usec;
	while(d->tv_usec > 1000000 ) {
		d->tv_usec -= 1000000;
		d->tv_sec++;
	}
#endif
}

void server_stats_init(struct server_stats* stats)
{
	memset(stats, 0, sizeof(*stats));
}

void server_stats_querymiss(struct server_stats* stats, struct worker* worker)
{
	stats->num_queries_missed_cache++;
	stats->sum_query_list_size += worker->env.mesh->all.count;
	if(worker->env.mesh->all.count > stats->max_query_list_size)
		stats->max_query_list_size = worker->env.mesh->all.count;
}

void server_stats_log(struct server_stats* stats, struct worker* worker,
	int threadnum)
{
	log_info("server stats for thread %d: %u queries, "
		"%u answers from cache, %u recursions", 
		threadnum, (unsigned)stats->num_queries, 
		(unsigned)(stats->num_queries - 
			stats->num_queries_missed_cache),
		(unsigned)stats->num_queries_missed_cache);
	log_info("server stats for thread %d: requestlist max %u avg %g "
		"exceeded %u", threadnum, (unsigned)stats->max_query_list_size,
		stats->num_queries_missed_cache?
			(double)stats->sum_query_list_size/
			stats->num_queries_missed_cache : 0.0,
		(unsigned)worker->env.mesh->stats_dropped);
}

void
server_stats_compile(struct worker* worker, struct stats_info* s)
{
	s->svr = worker->stats;
	s->mesh_num_states = worker->env.mesh->all.count;
	s->mesh_num_reply_states = worker->env.mesh->num_reply_states;
	s->mesh_jostled = worker->env.mesh->stats_jostled;
	s->mesh_dropped = worker->env.mesh->stats_dropped;
	s->mesh_replies_sent = worker->env.mesh->replies_sent;
	s->mesh_replies_sum_wait = worker->env.mesh->replies_sum_wait;
	s->mesh_time_median = timehist_quartile(worker->env.mesh->histogram,
		0.50);
	if(!worker->env.cfg->stat_cumulative) {
		server_stats_init(&worker->stats);
		mesh_stats_clear(worker->env.mesh);
	}
}

void server_stats_obtain(struct worker* worker, struct worker* who,
	struct stats_info* s)
{
	uint8_t *reply = NULL;
	size_t len = 0;
	if(worker == who) {
		/* just fill it in */
		server_stats_compile(worker, s);
		return;
	}
	/* communicate over tube */
	verbose(VERB_ALGO, "write stats cmd");
	worker_send_cmd(who, worker_cmd_stats);
	verbose(VERB_ALGO, "wait for stats reply");
	if(!tube_read_msg(worker->cmd, &reply, &len, 0))
		fatal_exit("failed to read stats over cmd channel");
	if(len != sizeof(*s))
		fatal_exit("stats on cmd channel wrong length %d %d",
			(int)len, (int)sizeof(*s));
	memcpy(s, reply, len);
	free(reply);
}

void server_stats_reply(struct worker* worker)
{
	struct stats_info s;
	server_stats_compile(worker, &s);
	verbose(VERB_ALGO, "write stats replymsg");
	if(!tube_write_msg(worker->daemon->workers[0]->cmd, 
		(uint8_t*)&s, sizeof(s), 0))
		fatal_exit("could not write stat values over cmd channel");
}

void server_stats_add(struct stats_info* total, struct stats_info* a)
{
	total->svr.num_queries += a->svr.num_queries;
	total->svr.num_queries_missed_cache += a->svr.num_queries_missed_cache;
	total->svr.sum_query_list_size += a->svr.sum_query_list_size;
	/* the max size reached is upped to higher of both */
	if(a->svr.max_query_list_size > total->svr.max_query_list_size)
		total->svr.max_query_list_size = a->svr.max_query_list_size;

	total->mesh_num_states += a->mesh_num_states;
	total->mesh_num_reply_states += a->mesh_num_reply_states;
	total->mesh_jostled += a->mesh_jostled;
	total->mesh_dropped += a->mesh_dropped;
	total->mesh_replies_sent += a->mesh_replies_sent;
	timeval_add(&total->mesh_replies_sum_wait, &a->mesh_replies_sum_wait);
	/* the medians are averaged together, this is not as accurate as
	 * taking the median over all of the data, but is good and fast
	 * added up here, division later*/
	total->mesh_time_median += a->mesh_time_median;
}
