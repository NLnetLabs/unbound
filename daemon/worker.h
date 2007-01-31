/*
 * daemon/worker.h - worker that handles a pending list of requests.
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
 * This file describes the worker structure that holds a list of 
 * pending requests and handles them.
 */

#ifndef DAEMON_WORKER_H
#define DAEMON_WORKER_H

#include "config.h"
#include "util/netevent.h"
struct listen_dnsport;

/**
 * Structure holding working information for unbound.
 * Holds globally visible information.
 */
struct worker {
	/** the event base this worker works with */
	struct comm_base* base;

	/** the frontside listening interface where request events come in */
	struct listen_dnsport* front;

	/** our one and only query, packet buffer and where to send. */
	struct comm_reply query_reply;

	/** number of requests currently active */
	int num_requests;
};

/**
 * Initialize worker.
 * Allocates event base, listens to ports
 * @param port: the port number to bind to.
 * @param do_ip4: listen to ip4 queries.
 * @param do_ip6: listen to ip6 queries.
 * @param do_udp: listen to udp queries.
 * @param do_tcp: listen to tcp queries.
 * @param buffer_size: size of datagram buffer.
 * @return: The worker, or NULL on error.
 */
struct worker* worker_init(const char* port, int do_ip4, int do_ip6,
	int do_udp, int do_tcp, size_t buffer_size);

/**
 * Make worker work.
 */
void worker_work(struct worker* worker);

/**
 * Delete worker.
 */
void worker_delete(struct worker* worker);

#endif /* DAEMON_WORKER_H */
