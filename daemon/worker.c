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
#include "util/net_help.h"
#include "util/random.h"
#include "daemon/worker.h"
#include "daemon/daemon.h"
#include "util/netevent.h"
#include "util/config_file.h"
#include "util/storage/slabhash.h"
#include "services/listen_dnsport.h"
#include "services/outside_network.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <signal.h>

/** timeout in seconds for UDP queries to auth servers. TODO: proper rtt */
#define UDP_QUERY_TIMEOUT 4

void 
worker_send_cmd(struct worker* worker, ldns_buffer* buffer,
	enum worker_commands cmd)
{
	ldns_buffer_clear(buffer);
	/* like DNS message, length data */
	ldns_buffer_write_u16(buffer, sizeof(uint32_t));
	ldns_buffer_write_u32(buffer, (uint32_t)cmd);
	ldns_buffer_flip(buffer);
	if(!write_socket(worker->cmd_send_fd, ldns_buffer_begin(buffer),
		ldns_buffer_limit(buffer)))
		log_err("write socket: %s", strerror(errno));
}

/** release workrequest back to the freelist,
 * note that the w->qinfo still needs to be cleared after this. 
 */
static void
req_release(struct work_query* w)
{
	if(w->worker->num_requests == w->worker->request_size)  {
		/* no longer at max, start accepting again. */
		listen_resume(w->worker->front);
	}
	w->worker->num_requests --;
	log_assert(w->worker->num_requests >= 0);
	w->next = w->worker->free_queries;
	w->worker->free_queries = w;
}

/** reply to query with given error code */
static void 
replyerror(int r, struct work_query* w)
{
	ldns_buffer* buf = w->query_reply.c->buffer;
	uint16_t flags;
	verbose(VERB_DETAIL, "reply with error");
	
	ldns_buffer_clear(buf);
	ldns_buffer_write_u16(buf, w->query_id);
	flags = (uint16_t)(0x8000 | r); /* QR and retcode*/
	flags |= (w->query_flags & 0x0100); /* copy RD bit */
	ldns_buffer_write_u16(buf, flags);
	flags = 1;
	ldns_buffer_write_u16(buf, flags);
	flags = 0;
	ldns_buffer_write(buf, &flags, sizeof(uint16_t));
	ldns_buffer_write(buf, &flags, sizeof(uint16_t));
	ldns_buffer_write(buf, &flags, sizeof(uint16_t));
	ldns_buffer_write(buf, w->qinfo.qname, w->qinfo.qnamesize);
	ldns_buffer_write_u16(buf, w->qinfo.qtype);
	ldns_buffer_write_u16(buf, w->qinfo.qclass);
	ldns_buffer_flip(buf);
	comm_point_send_reply(&w->query_reply);
	req_release(w);
	query_info_clear(&w->qinfo);
}

/** process incoming replies from the network */
static int 
worker_handle_reply(struct comm_point* c, void* arg, int error, 
	struct comm_reply* ATTR_UNUSED(reply_info))
{
	struct work_query* w = (struct work_query*)arg;
	struct reply_info* rep;
	struct msgreply_entry* e;
	verbose(VERB_DETAIL, "reply to query with stored ID %d", w->query_id);
	if(error != 0) {
		replyerror(LDNS_RCODE_SERVFAIL, w);
		return 0;
	}
	/* sanity check. */
	if(!LDNS_QR_WIRE(ldns_buffer_begin(c->buffer)))
		return 0; /* not a reply. */
	if(LDNS_OPCODE_WIRE(ldns_buffer_begin(c->buffer)) != LDNS_PACKET_QUERY)
		return 0; /* not a reply to a query. */
	if(LDNS_QDCOUNT(ldns_buffer_begin(c->buffer)) > 1)
		return 0; /* too much in the query section */
	/* woohoo a reply! */
	rep = (struct reply_info*)malloc(sizeof(struct reply_info));
	if(!rep) {
		log_err("out of memory");
		replyerror(LDNS_RCODE_SERVFAIL, w);
		return 0;
	}
	rep->replysize = ldns_buffer_limit(c->buffer) - 2; /* minus ID */
	log_info("got reply of size %d", rep->replysize);
	rep->reply = (uint8_t*)malloc(rep->replysize);
	if(!rep->reply) {
		free(rep);
		log_err("out of memory");
		replyerror(LDNS_RCODE_SERVFAIL, w);
		return 0;
	}
	memmove(rep->reply, ldns_buffer_at(c->buffer, 2), rep->replysize);
	ldns_buffer_write_u16_at(w->query_reply.c->buffer, 0, w->query_id);
	reply_info_answer(rep, w->query_flags, w->query_reply.c->buffer);
	comm_point_send_reply(&w->query_reply);
	req_release(w);
	/* store or update reply in the cache */
	if(!(e = query_info_entrysetup(&w->qinfo, rep, w->query_hash))) {
		log_err("out of memory");
		return 0;
	}
	slabhash_insert(w->worker->daemon->msg_cache, w->query_hash, 
		&e->entry, rep);
	return 0;
}

/** process incoming request */
static void 
worker_process_query(struct worker* worker, struct work_query* w) 
{
	/* query the forwarding address */
	verbose(VERB_DETAIL, "process_query ID %d", w->query_id);
	pending_udp_query(worker->back, w->query_reply.c->buffer, 
		&worker->fwd_addr, worker->fwd_addrlen, UDP_QUERY_TIMEOUT,
		worker_handle_reply, w, worker->rndstate);
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

/** process control messages from the main thread. */
static int 
worker_handle_control_cmd(struct comm_point* c, void* arg, int error, 
	struct comm_reply* ATTR_UNUSED(reply_info))
{
	struct worker* worker = (struct worker*)arg;
	enum worker_commands cmd;
	if(error != NETEVENT_NOERROR) {
		if(error == NETEVENT_CLOSED)
			comm_base_exit(worker->base);
		else	log_info("control event: %d", error);
		return 0;
	}
	if(ldns_buffer_limit(c->buffer) != sizeof(uint32_t)) {
		fatal_exit("bad control msg length %d", 
			(int)ldns_buffer_limit(c->buffer));
	}
	cmd = ldns_buffer_read_u32(c->buffer);
	switch(cmd) {
	case worker_cmd_quit:
		verbose(VERB_ALGO, "got control cmd quit");
		comm_base_exit(worker->base);
		break;
	default:
		log_err("bad command %d", (int)cmd);
		break;
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
	hashvalue_t h;
	struct lruhash_entry* e;
	struct query_info qinfo;
	struct work_query* w;

	verbose(VERB_DETAIL, "worker handle request");
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
		comm_point_drop_reply(repinfo);
		return 0;
	}
	/* see if query is in the cache */
	if(!query_info_parse(&qinfo, c->buffer)) {
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), 
			LDNS_RCODE_FORMERR);
		return 1;
	}
	h = query_info_hash(&qinfo);
	if((e=slabhash_lookup(worker->daemon->msg_cache, h, &qinfo, 0))) {
		/* answer from cache */
		log_info("answer from the cache");
		/* id is still in the buffer, no need to touch it */
		reply_info_answer((struct reply_info*)e->data, 
			ldns_buffer_read_u16_at(c->buffer, 2), c->buffer);
		lock_rw_unlock(&e->lock);
		return 1;
	}
	ldns_buffer_rewind(c->buffer);
	/* perform memory allocation(s) */
	if(!query_info_allocqname(&qinfo)) {
		comm_point_drop_reply(repinfo);
		return 0;
	}

	/* grab a work request structure for this new request */
	if(!worker->free_queries) {
		/* we could get this due to a slow tcp incoming query, 
		   that started before we performed listen_pushback */
		verbose(VERB_DETAIL, "worker: too many incoming requests "
			"active. dropping incoming query.");
		comm_point_drop_reply(repinfo);
		return 0;
	}
	w = worker->free_queries;
	worker->free_queries = w->next;
	worker->num_requests ++;
	log_assert(worker->num_requests <= worker->request_size);
	if(worker->num_requests == worker->request_size)  {
		/* the max request number has been reached, stop accepting */
		listen_pushback(worker->front);
	}

	/* init request */
	w->next = NULL;
	w->query_hash = h;
	memcpy(&w->query_reply, repinfo, sizeof(struct comm_reply));
	memcpy(&w->qinfo, &qinfo, sizeof(struct query_info));
	w->query_id = LDNS_ID_WIRE(ldns_buffer_begin(c->buffer));
	w->query_flags = ldns_buffer_read_u16_at(c->buffer, 2);

	/* answer it */
	worker_process_query(worker, w);
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
			worker->need_to_restart = 1;
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
		case SIGTERM:
			log_info("caught signal SIGTERM");
			comm_base_exit(worker->base);
			break;
		default:
			log_err("unknown signal: %d, ignored", sig);
			break;
	}
}

struct worker* 
worker_create(struct daemon* daemon, int id)
{
	struct worker* worker = (struct worker*)calloc(1, 
		sizeof(struct worker));
	if(!worker) 
		return NULL;
	worker->daemon = daemon;
	worker->thread_num = id;
	worker->cmd_send_fd = -1;
	worker->cmd_recv_fd = -1;
	if(id != 0) {
		int sv[2];
		/* create socketpair to communicate with worker */
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
			free(worker);
			log_err("socketpair: %s", strerror(errno));
			return NULL;
		}
		if(!fd_set_nonblock(sv[0]) || !fd_set_nonblock(sv[1])) {
			close(sv[0]);
			close(sv[1]);
			free(worker);
			return NULL;
		}
		worker->cmd_send_fd = sv[0];
		worker->cmd_recv_fd = sv[1];
	}
	return worker;
}

/** create request handling structures */
static int
reqs_init(struct worker* worker)
{
	int i;
	for(i=0; i<worker->request_size; i++) {
		struct work_query* q = (struct work_query*)calloc(1,
			sizeof(struct work_query));
		if(!q) return 0;
		q->worker = worker;
		q->next = worker->free_queries;
		worker->free_queries = q;
	}
	return 1;
}

int
worker_init(struct worker* worker, struct config_file *cfg, 
	struct listen_port* ports, size_t buffer_size, int do_sigs)
{
	unsigned int seed;
	int startport;
	worker->need_to_restart = 0;
	worker->base = comm_base_create();
	if(!worker->base) {
		log_err("could not create event handling base");
		worker_delete(worker);
		return 0;
	}
	if(do_sigs) {
		ub_thread_sig_unblock(SIGHUP);
		ub_thread_sig_unblock(SIGINT);
		ub_thread_sig_unblock(SIGQUIT);
		ub_thread_sig_unblock(SIGTERM);
#ifndef LIBEVENT_SIGNAL_PROBLEM
		worker->comsig = comm_signal_create(worker->base, 
			worker_sighandler, worker);
		if(!worker->comsig || !comm_signal_bind(worker->comsig, SIGHUP)
			|| !comm_signal_bind(worker->comsig, SIGINT)
			|| !comm_signal_bind(worker->comsig, SIGTERM)
			|| !comm_signal_bind(worker->comsig, SIGQUIT)) {
			log_err("could not create signal handlers");
			worker_delete(worker);
			return 0;
		}
#endif /* LIBEVENT_SIGNAL_PROBLEM */
	} else { /* !do_sigs */
		worker->comsig = 0;
	}
	worker->front = listen_create(worker->base, ports,
		buffer_size, worker_handle_request, worker);
	if(!worker->front) {
		log_err("could not create listening sockets");
		worker_delete(worker);
		return 0;
	}
	startport  = cfg->outgoing_base_port + 
		cfg->outgoing_num_ports * worker->thread_num;
	worker->back = outside_network_create(worker->base,
		buffer_size, (size_t)cfg->outgoing_num_ports, cfg->ifs, 
		cfg->num_ifs, cfg->do_ip4, cfg->do_ip6, startport);
	if(!worker->back) {
		log_err("could not create outgoing sockets");
		worker_delete(worker);
		return 0;
	}
	/* init random(), large table size. */
	if(!(worker->rndstate = (struct ub_randstate*)calloc(1,
		sizeof(struct ub_randstate)))) {
		log_err("malloc rndtable failed.");
		worker_delete(worker);
		return 0;
	}
	seed = (unsigned int)time(NULL) ^ (unsigned int)getpid() ^
		(unsigned int)worker->thread_num;
	if(!ub_initstate(seed, worker->rndstate, RND_STATE_SIZE)) {
		log_err("could not init random numbers.");
		worker_delete(worker);
		return 0;
	}
	if(worker->thread_num != 0) {
		/* start listening to commands */
		if(!(worker->cmd_com=comm_point_create_local(worker->base, 
			worker->cmd_recv_fd, buffer_size, 
			worker_handle_control_cmd, worker))) {
			log_err("could not create control compt.");
			worker_delete(worker);
			return 0;
		}
	}
	worker->request_size = 1;
	if(!reqs_init(worker)) {
		worker_delete(worker);
		return 0;
	}

	/* set forwarder address */
	if(cfg->fwd_address && cfg->fwd_address[0]) {
		if(!worker_set_fwd(worker, cfg->fwd_address, cfg->fwd_port)) {
			worker_delete(worker);
			fatal_exit("could not set forwarder address");
		}
	}
	alloc_init(&worker->alloc, &worker->daemon->superalloc);
	return 1;
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
	comm_point_delete(worker->cmd_com);
	comm_base_delete(worker->base);
	ub_randfree(worker->rndstate);
	free(worker->rndstate);
	/* close fds after deleting commpoints, to be sure.
	   Also epoll does not like closing fd before event_del */
	if(worker->cmd_send_fd != -1)
		close(worker->cmd_send_fd);
	worker->cmd_send_fd = -1;
	if(worker->cmd_recv_fd != -1)
		close(worker->cmd_recv_fd);
	worker->cmd_recv_fd = -1;
	alloc_clear(&worker->alloc);
	free(worker);
}

int 
worker_set_fwd(struct worker* worker, const char* ip, int port)
{
	uint16_t p;
	log_assert(worker && ip);
	p = (uint16_t) port;
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
