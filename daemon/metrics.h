/*
 * daemon/metrics.h - prometheus metrics endpoint.
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

#ifndef DAEMON_METRICS_H
#define DAEMON_METRICS_H
struct daemon_metrics;
struct config_file;
struct worker;

/* the metrics daemon needs little backlog */
#define TCP_BACKLOG_METRICS 16 /* listen() tcp backlog */

/**
 * Create new metrics endpoint for the daemon.
 * Does not open the ports, for that call the open ports routine, and
 * later the attach routine on the worker event base.
 * @return new state, or NULL on failure.
 */
struct daemon_metrics* daemon_metrics_create(void);

/**
 * Delete metrics daemon and close HTTP listeners.
 * @param m: daemon to delete.
 */
void daemon_metrics_delete(struct daemon_metrics* m);

/**
 * Close metrics HTTP listener ports.
 * Does not delete the object itself.
 * @param m: state to close.
 */
void daemon_metrics_close_ports(struct daemon_metrics* m);

/**
 * Detach the metrics listener from the event base.
 * Does not delete the object itself.
 * @param m: state to detach.
 */
void daemon_metrics_detach(struct daemon_metrics* m);

/**
 * Open and create HTTP listeners for metrics daemon.
 * @param m: metrics state that contains list of accept sockets.
 * @param cfg: config options.
 * @return false on failure.
 */
int daemon_metrics_open_ports(struct daemon_metrics* m,
	struct config_file* cfg);

/**
 * Setup HTTP listener.
 * @param m: state
 * @param worker: The worker thread that hosts the endpoint.
 *	The HTTP listener is attached to its event base.
 * @return false on failure.
 */
int daemon_metrics_attach(struct daemon_metrics* m, struct worker* worker);

#endif /* DAEMON_METRICS_H */
