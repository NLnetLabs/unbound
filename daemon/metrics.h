/*
 * daemon/metrics.h - prometheus metrics http exporter for the unbound daemon.
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
 * This file contains a small plain HTTP/1.1 server that exports the
 * server statistics (as also obtained with "unbound-control stats_noreset")
 * in the Prometheus text exposition format.  It is configured with the
 * metrics-enable, metrics-interface, metrics-port and metrics-path options.
 */

#ifndef DAEMON_METRICS_H
#define DAEMON_METRICS_H
struct config_file;
struct listen_list;
struct listen_port;
struct worker;
struct comm_point;
struct comm_reply;
struct daemon_metrics;

/**
 * a busy metrics http connection
 */
struct metrics_state {
	/** the next item in list */
	struct metrics_state* next;
	/** the commpoint */
	struct comm_point* c;
	/** file descriptor */
	int fd;
	/** the metrics exporter this connection is part of */
	struct daemon_metrics* dm;
};

/**
 * The prometheus metrics http exporter state.
 * The state is only created for the first thread, other threads
 * are called from this thread.  Only the first thread listens to
 * the metrics port.
 */
struct daemon_metrics {
	/** the worker for this metrics exporter */
	struct worker* worker;
	/** commpoints for accepting metrics http connections */
	struct listen_list* accept_list;
	/** number of active commpoints that are handling a metrics request */
	int active;
	/** max active commpoints */
	int max_active;
	/** current commpoints busy; should be a short list, malloced */
	struct metrics_state* busy_list;
};

/**
 * Create new metrics http exporter state for the daemon.
 * @param cfg: config file with metrics settings.
 * @return new state, or NULL on failure.
 */
struct daemon_metrics* daemon_metrics_create(struct config_file* cfg);

/**
 * metrics exporter state to delete.
 * @param dm: state to delete.
 */
void daemon_metrics_delete(struct daemon_metrics* dm);

/**
 * metrics exporter state to clear up. Busy and accept points are closed.
 * Does not delete the dm itself.
 * @param dm: state to clear.
 */
void daemon_metrics_clear(struct daemon_metrics* dm);

/**
 * Open and create listening ports for the metrics exporter.
 * @param cfg: config options.
 * @return list of ports or NULL on failure.
 *	can be freed with listening_ports_free().
 */
struct listen_port* daemon_metrics_open_ports(struct config_file* cfg);

/**
 * Setup comm points for accepting metrics http connections.
 * @param dm: state
 * @param ports: already opened ports.
 * @param worker: worker with communication base.
 * @return false on error.
 */
int daemon_metrics_open_accept(struct daemon_metrics* dm,
	struct listen_port* ports, struct worker* worker);

/**
 * Stop accept handlers for the metrics exporter (until enabled again)
 * @param dm: state
 */
void daemon_metrics_stop_accept(struct daemon_metrics* dm);

/**
 * Start accept handlers for the metrics exporter.
 * @param dm: state
 */
void daemon_metrics_start_accept(struct daemon_metrics* dm);

#endif /* DAEMON_METRICS_H */
