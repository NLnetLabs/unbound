/*
 * daemon/worker.c - worker that handles a pending list of requests.
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
 * This file implements the worker that handles callbacks on events, for
 * pending requests.
 */
#include "config.h"
#include "util/log.h"
#include "daemon/worker.h"
#include "util/netevent.h"
#include "services/listen_dnsport.h"

/** handles callbacks from listening event interface */
static int worker_handle_request(ldns_buffer* packet, void* arg)
{
	log_info("worker handle request");
	return 0;
}

struct worker* worker_init(const char* port, int do_ip4, int do_ip6,
	int do_udp, int do_tcp, size_t buffer_size)
{
	struct worker* worker = (struct worker*)malloc(sizeof(struct worker));
	if(!worker) 
		return NULL;
	worker->base = comm_base_create();
	if(!worker->base) {
		log_err("could not create event handling base");
		return NULL;
	}
	worker->front = listen_create(worker->base, 0, NULL, port, 
		do_ip4, do_ip6, do_udp, do_tcp, buffer_size, 
		worker_handle_request, worker);
	if(!worker->front) {
		comm_base_delete(worker->base);
		log_err("could not create listening sockets");
		return NULL;
	}
	return worker;
}


void worker_work(struct worker* worker)
{
	comm_base_dispatch(worker->base);
}

void worker_delete(struct worker* worker)
{
	if(!worker) 
		return;
	listen_delete(worker->front);
	comm_base_delete(worker->base);
	free(worker);
}
