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
#include "services/outside_network.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <errno.h>
#include <signal.h>

/** timeout in seconds for UDP queries to auth servers. TODO: proper rtt */
#define UDP_QUERY_TIMEOUT 5
/** the size of ID and flags, opcode, rcode in dns packet */
#define ID_AND_FLAGS 4

/** reply to query with given error code */
static void 
replyerror(int r, struct worker* worker)
{
	LDNS_QR_SET(ldns_buffer_begin(worker->query_reply.c->buffer));
	LDNS_RCODE_SET(ldns_buffer_begin(worker->query_reply.c->buffer), r);
	comm_point_send_reply(&worker->query_reply);
	worker->num_requests --;
}

/** process incoming replies from the network */
static int 
worker_handle_reply(struct comm_point* c, void* arg, int error, 
	struct comm_reply* ATTR_UNUSED(reply_info))
{
	struct worker* worker = (struct worker*)arg;
	if(error != 0) {
		replyerror(LDNS_RCODE_SERVFAIL, worker);
		return 0;
	}
	/* woohoo a reply! */
	ldns_buffer_clear(worker->query_reply.c->buffer);
	ldns_buffer_skip(worker->query_reply.c->buffer, ID_AND_FLAGS);
	ldns_buffer_write(worker->query_reply.c->buffer, 
		ldns_buffer_at(c->buffer, ID_AND_FLAGS), 
		ldns_buffer_limit(c->buffer) - ID_AND_FLAGS);
	LDNS_QR_SET(ldns_buffer_begin(worker->query_reply.c->buffer));
	ldns_buffer_flip(worker->query_reply.c->buffer);
	comm_point_send_reply(&worker->query_reply);
	worker->num_requests --;
	return 0;
}

/** process incoming request */
static void 
worker_process_query(struct worker* worker) 
{
	/* query the forwarding address */
	pending_udp_query(worker->back, worker->query_reply.c->buffer, 
		&worker->fwd_addr, worker->fwd_addrlen, UDP_QUERY_TIMEOUT,
		worker_handle_reply, worker);
}

/** check request sanity. Returns error code, 0 OK, or -1 discard. 
 * @param pkt: the wire packet to examine for sanity.
*/
static int 
worker_check_request(ldns_buffer* pkt)
{
	if(ldns_buffer_limit(pkt) < LDNS_HEADER_SIZE) {
		verbose(VERB_DETAIL, "request too short, discarded");
		return -1;
	}
	if(LDNS_QR_WIRE(ldns_buffer_begin(pkt))) {
		verbose(VERB_DETAIL, "request has QR bit on, discarded");
		return -1;
	}
	if(LDNS_OPCODE_WIRE(ldns_buffer_begin(pkt)) != LDNS_PACKET_QUERY) {
		verbose(VERB_DETAIL, "request unknown opcode %d", 
			LDNS_OPCODE_WIRE(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_NOTIMPL;
	}
	if(LDNS_QDCOUNT(ldns_buffer_begin(pkt)) != 1) {
		verbose(VERB_DETAIL, "request wrong nr qd=%d", 
			LDNS_QDCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	if(LDNS_ANCOUNT(ldns_buffer_begin(pkt)) != 0) {
		verbose(VERB_DETAIL, "request wrong nr an=%d", 
			LDNS_ANCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	if(LDNS_NSCOUNT(ldns_buffer_begin(pkt)) != 0) {
		verbose(VERB_DETAIL, "request wrong nr ns=%d", 
			LDNS_NSCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	if(LDNS_ARCOUNT(ldns_buffer_begin(pkt)) != 0) {
		verbose(VERB_DETAIL, "request wrong nr ar=%d", 
			LDNS_ARCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	return 0;
}

/** handles callbacks from listening event interface */
static int 
worker_handle_request(struct comm_point* c, void* arg, int error,
	struct comm_reply* repinfo)
{
	struct worker* worker = (struct worker*)arg;
	int ret;
	log_info("worker handle request");
	if(error != NETEVENT_NOERROR) {
		log_err("called with err=%d", error);
		return 0;
	}
	if((ret=worker_check_request(c->buffer)) != 0) {
		if(ret != -1) {
			LDNS_QR_SET(ldns_buffer_begin(c->buffer));
			LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), ret);
			return 1;
		}
		return 0;
	}
	if(worker->num_requests > 0) {
		verbose(VERB_DETAIL, "worker: too many requests active. "
			"dropping incoming query.");
		return 0;
	}
	/* answer it */
	worker->num_requests ++;
	memcpy(&worker->query_reply, repinfo, sizeof(struct comm_reply));
	worker_process_query(worker);
	return 0;
}

/** worker signal callback */
void 
worker_sighandler(int sig, void* arg)
{
	/* note that log, print, syscalls here give race conditions. */
	struct worker* worker = (struct worker*)arg;
	switch(sig) {
		case SIGHUP:
			log_info("caught signal SIGHUP");
			comm_base_exit(worker->base);
			break;
		case SIGINT:
			log_info("caught signal SIGINT");
			comm_base_exit(worker->base);
			break;
		case SIGQUIT:
			log_info("caught signal SIGQUIT");
			comm_base_exit(worker->base);
			break;
		default:
			log_err("unknown signal: %d, ignored", sig);
			break;
	}
}

struct worker* 
worker_init(const char* port, int do_ip4, int do_ip6, int do_udp, int do_tcp, 
	size_t buffer_size, size_t numports, int base_port)
{
	struct worker* worker = (struct worker*)calloc(1, 
		sizeof(struct worker));
	if(!worker) 
		return NULL;
	worker->base = comm_base_create();
	if(!worker->base) {
		log_err("could not create event handling base");
		worker_delete(worker);
		return NULL;
	}
	worker->comsig = comm_signal_create(worker->base, worker_sighandler, 
		worker);
	if(!worker->comsig || !comm_signal_bind(worker->comsig, SIGHUP)
		|| !comm_signal_bind(worker->comsig, SIGINT)
		|| !comm_signal_bind(worker->comsig, SIGQUIT)) {
		log_err("could not create signal handlers");
		worker_delete(worker);
		return NULL;
	}
	worker->front = listen_create(worker->base, 0, NULL, port, 
		do_ip4, do_ip6, do_udp, do_tcp, buffer_size, 
		worker_handle_request, worker);
	if(!worker->front) {
		log_err("could not create listening sockets");
		worker_delete(worker);
		return NULL;
	}
	worker->back = outside_network_create(worker->base,
		buffer_size, numports, NULL, 0, do_ip4, do_ip6, base_port);
	if(!worker->back) {
		log_err("could not create outgoing sockets");
		worker_delete(worker);
		return NULL;
	}
	/* init random(), large table size. */
	if(!initstate(time(NULL)^getpid(), worker->rndstate, RND_STATE_SIZE)) {
		log_err("could not init random numbers.");
		worker_delete(worker);
		return NULL;
	}
	return worker;
}

void 
worker_work(struct worker* worker)
{
	comm_base_dispatch(worker->base);
}

void 
worker_delete(struct worker* worker)
{
	if(!worker) 
		return;
	listen_delete(worker->front);
	outside_network_delete(worker->back);
	comm_signal_delete(worker->comsig);
	comm_base_delete(worker->base);
	free(worker);
}

int 
worker_set_fwd(struct worker* worker, const char* ip, const char* port)
{
	uint16_t p;
	log_assert(worker && ip);
	if(port)
		p = (uint16_t)atoi(port);
	else 	p = (uint16_t)atoi(UNBOUND_DNS_PORT);
	if(!p) {
		log_err("Bad port number %s", port?port:"default_port");
		return 0;
	}
	if(str_is_ip6(ip)) {
		struct sockaddr_in6* sa = 
			(struct sockaddr_in6*)&worker->fwd_addr;
		worker->fwd_addrlen = (socklen_t)sizeof(struct sockaddr_in6);
		memset(sa, 0, worker->fwd_addrlen);
		sa->sin6_family = AF_INET6;
		sa->sin6_port = (in_port_t)htons(p);
		if(inet_pton((int)sa->sin6_family, ip, &sa->sin6_addr) <= 0) {
			log_err("Bad ip6 address %s", ip);
			return 0;
		}
	} else { /* ip4 */
		struct sockaddr_in* sa = 
			(struct sockaddr_in*)&worker->fwd_addr;
		worker->fwd_addrlen = (socklen_t)sizeof(struct sockaddr_in);
		memset(sa, 0, worker->fwd_addrlen);
		sa->sin_family = AF_INET;
		sa->sin_port = (in_port_t)htons(p);
		if(inet_pton((int)sa->sin_family, ip, &sa->sin_addr) <= 0) {
			log_err("Bad ip4 address %s", ip);
			return 0;
		}
	}
	verbose(VERB_ALGO, "fwd queries to: %s %d", ip, p);
	return 1;
}
