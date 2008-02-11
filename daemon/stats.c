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
#include "services/mesh.h"

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

void server_stats_log(struct server_stats* stats, int threadnum)
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
		(unsigned)stats->num_query_list_exceeded);
}
