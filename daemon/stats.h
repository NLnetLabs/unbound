/*
 * daemon/stats.h - collect runtime performance indicators.
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

#ifndef DAEMON_STATS_H
#define DAEMON_STATS_H
struct worker;

/** per worker statistics */
struct server_stats {
	/** number of queries from clients received. */
	size_t num_queries;
	/** number of queries that had a cache-miss. */
	size_t num_queries_missed_cache;

	/**
	 * Sum of the querylistsize of the worker for 
	 * every query that missed cache. To calculate average.
	 */
	size_t sum_query_list_size;
	/** max value of query list size reached. */
	size_t max_query_list_size;
	/** number of times that the query_list_size was insufficient */
	size_t num_query_list_exceeded;
};

/** 
 * Initialize server stats to 0.
 * @param stats: what to init (this is alloced by the caller).
 */
void server_stats_init(struct server_stats* stats);

/** add query if it missed the cache */
void server_stats_querymiss(struct server_stats* stats, struct worker* worker);

/** display the stats to the log */
void server_stats_log(struct server_stats* stats, int threadnum);

#endif /* DAEMON_STATS_H */
