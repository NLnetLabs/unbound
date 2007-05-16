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
#include "util/region-allocator.h"
#include "util/storage/slabhash.h"
#include "services/listen_dnsport.h"
#include "services/outside_network.h"
#include "services/cache/rrset.h"
#include "util/data/msgparse.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <signal.h>

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
	struct worker* worker = w->state.env->worker;
	if(worker->num_requests == worker->request_size)  {
		/* no longer at max, start accepting again. */
		listen_resume(worker->front);
	}
	log_assert(worker->num_requests >= 1);
	worker->num_requests --;
	w->next = worker->free_queries;
	worker->free_queries = w;
}

/** create error and fill into buffer */
static void
replyerror_fillbuf(int r, struct comm_reply* repinfo, uint16_t id,
	uint16_t qflags, struct query_info* qinfo)
{
	ldns_buffer* buf = repinfo->c->buffer;
	uint16_t flags;
	verbose(VERB_DETAIL, "reply with error");
	
	ldns_buffer_clear(buf);
	ldns_buffer_write(buf, &id, sizeof(uint16_t));
	flags = (uint16_t)(BIT_QR | r); /* QR and retcode*/
	flags |= (qflags & (BIT_RD|BIT_CD)); /* copy RD and CD bit */
	ldns_buffer_write_u16(buf, flags);
	flags = 1;
	ldns_buffer_write_u16(buf, flags);
	flags = 0;
	ldns_buffer_write(buf, &flags, sizeof(uint16_t));
	ldns_buffer_write(buf, &flags, sizeof(uint16_t));
	ldns_buffer_write(buf, &flags, sizeof(uint16_t));
	ldns_buffer_write(buf, qinfo->qname, qinfo->qnamesize);
	ldns_buffer_write_u16(buf, qinfo->qtype);
	ldns_buffer_write_u16(buf, qinfo->qclass);
	ldns_buffer_flip(buf);
}

/** reply to query with given error code */
static void 
replyerror(int r, struct work_query* w)
{
	w->state.edns.edns_version = EDNS_ADVERTISED_VERSION;
	w->state.edns.udp_size = EDNS_ADVERTISED_SIZE;
	w->state.edns.ext_rcode = 0;
	w->state.edns.bits &= EDNS_DO;
	replyerror_fillbuf(r, &w->query_reply, w->query_id, 
		w->state.query_flags, &w->state.qinfo);
	attach_edns_record(w->query_reply.c->buffer, &w->state.edns);
	comm_point_send_reply(&w->query_reply);
	req_release(w);
	query_info_clear(&w->state.qinfo);
}

/** process incoming request */
static void 
worker_process_query(struct worker* worker, struct work_query* w, 
	enum module_ev event) 
{
	int i;
	if(event == module_event_new) {
		w->state.curmod = 0;
		for(i=0; i<worker->daemon->num_modules; i++)
			w->state.ext_state[i] = module_state_initial;
	}
	/* allow current module to run */
	(*worker->daemon->modfunc[w->state.curmod]->operate)(&w->state, event,
		w->state.curmod);
	/* TODO examine results, start further modules, etc.
	 * assume it went to sleep
	 */
	region_free_all(worker->scratchpad);
	if(w->state.ext_state[w->state.curmod] == module_error) {
		region_free_all(w->state.region);
		replyerror(LDNS_RCODE_SERVFAIL, w);
		return;
	}
	if(w->state.ext_state[w->state.curmod] == module_finished) {
		memcpy(ldns_buffer_begin(w->query_reply.c->buffer),
			&w->query_id, sizeof(w->query_id));
		comm_point_send_reply(&w->query_reply);
		region_free_all(w->state.region);
		req_release(w);
		query_info_clear(&w->state.qinfo);
		return;
	}
	/* suspend, waits for wakeup callback */
}

/** process incoming replies from the network */
static int 
worker_handle_reply(struct comm_point* c, void* arg, int error, 
	struct comm_reply* reply_info)
{
	struct work_query* w = (struct work_query*)arg;
	struct worker* worker = w->state.env->worker;

	w->state.reply = reply_info;
	if(error != 0) {
		worker_process_query(worker, w, module_event_timeout);
		w->state.reply = NULL;
		return 0;
	}
	/* sanity check. */
	if(!LDNS_QR_WIRE(ldns_buffer_begin(c->buffer))
		|| LDNS_OPCODE_WIRE(ldns_buffer_begin(c->buffer)) != 
			LDNS_PACKET_QUERY
		|| LDNS_QDCOUNT(ldns_buffer_begin(c->buffer)) > 1) {
		/* error becomes timeout for the module as if this reply
		 * never arrived. */
		worker_process_query(worker, w, module_event_timeout);
		w->state.reply = NULL;
		return 0;
	}
	worker_process_query(worker, w, module_event_reply);
	w->state.reply = NULL;
	return 0;
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
	if(LDNS_ARCOUNT(ldns_buffer_begin(pkt)) > 1) {
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

/** answer query from the cache */
static int
answer_from_cache(struct worker* worker, struct lruhash_entry* e, uint16_t id,
	uint16_t flags, struct comm_reply* repinfo, struct edns_data* edns)
{
	struct msgreply_entry* mrentry = (struct msgreply_entry*)e->key;
	struct reply_info* rep = (struct reply_info*)e->data;
	uint32_t timenow = time(0);
	uint16_t udpsize = edns->udp_size;
	size_t i;
	hashvalue_t* h;
	/* see if it is possible */
	if(rep->ttl <= timenow) {
		/* the rrsets may have been updated in the meantime.
		 * we will refetch the message format from the
		 * authoritative server 
		 */
		return 0;
	}
	edns->edns_version = EDNS_ADVERTISED_VERSION;
	edns->udp_size = EDNS_ADVERTISED_SIZE;
	edns->ext_rcode = 0;
	edns->bits &= EDNS_DO;
	if(!(h = (hashvalue_t*)region_alloc(worker->scratchpad, 
		sizeof(hashvalue_t)*rep->rrset_count)))
		return 0;
	/* check rrsets */
	for(i=0; i<rep->rrset_count; i++) {
		if(i>0 && rep->ref[i].key == rep->ref[i-1].key)
			continue; /* only lock items once */
		lock_rw_rdlock(&rep->ref[i].key->entry.lock);
		if(rep->ref[i].id != rep->ref[i].key->id ||
			rep->ttl <= timenow) {
			/* failure! rollback our readlocks */
			size_t j;
			for(j=0; j<=i; j++)
				lock_rw_unlock(&rep->ref[j].key->entry.lock);
			return 0;
		}
	}
	/* locked and ids and ttls are OK. */
	if(!reply_info_answer_encode(&mrentry->key, rep, id, flags, 
		repinfo->c->buffer, timenow, 1, worker->scratchpad,
		udpsize, edns)) {
		replyerror_fillbuf(LDNS_RCODE_SERVFAIL, repinfo, id,
			flags, &mrentry->key);
	}
	/* unlock */
	for(i=0; i<rep->rrset_count; i++) {
		if(i>0 && rep->ref[i].key == rep->ref[i-1].key)
			continue; /* only unlock items once */
		h[i] = rep->ref[i].key->entry.hash;
		lock_rw_unlock(&rep->ref[i].key->entry.lock);
	}
	/* LRU touch, with no rrset locks held */
	for(i=0; i<rep->rrset_count; i++) {
		if(i>0 && rep->ref[i].key == rep->ref[i-1].key)
			continue; /* only touch items once */
		rrset_cache_touch(worker->env.rrset_cache, rep->ref[i].key,
			h[i], rep->ref[i].id);
	}
	region_free_all(worker->scratchpad);
	/* go and return this buffer to the client */
	return 1;
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
	struct edns_data edns;

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
	worker->stats.num_queries++;
	/* see if query is in the cache */
	if(!query_info_parse(&qinfo, c->buffer)) {
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), 
			LDNS_RCODE_FORMERR);
		return 1;
	}
	h = query_info_hash(&qinfo);
	if((ret=parse_edns_from_pkt(c->buffer, &edns)) != 0) {
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), ret);
		return 1;
	}
	if(edns.edns_present && edns.edns_version != 0) {
		edns.ext_rcode = (uint8_t)(EDNS_RCODE_BADVERS>>4);
		edns.edns_version = EDNS_ADVERTISED_VERSION;
		edns.udp_size = EDNS_ADVERTISED_SIZE;
		edns.bits &= EDNS_DO;
		replyerror_fillbuf(EDNS_RCODE_BADVERS&0xf, repinfo, 
			*(uint16_t*)ldns_buffer_begin(c->buffer),
			ldns_buffer_read_u16_at(c->buffer, 2), &qinfo);
		attach_edns_record(c->buffer, &edns);
		return 1;
	}
	if(c->type != comm_udp)
		edns.udp_size = 65535; /* max size for TCP replies */
	if((e=slabhash_lookup(worker->env.msg_cache, h, &qinfo, 0))) {
		/* answer from cache - we have acquired a readlock on it */
		log_info("answer from the cache");
		if(answer_from_cache(worker, e, 
			*(uint16_t*)ldns_buffer_begin(c->buffer), 
			ldns_buffer_read_u16_at(c->buffer, 2), repinfo, 
			&edns)) {
			lock_rw_unlock(&e->lock);
			return 1;
		}
		log_info("answer from the cache -- data has timed out");
		lock_rw_unlock(&e->lock);
	}
	ldns_buffer_rewind(c->buffer);
	server_stats_querymiss(&worker->stats, worker);
	/* perform memory allocation(s) */
	if(!query_info_allocqname(&qinfo)) {
		comm_point_drop_reply(repinfo);
		return 0;
	}

	/* grab a work request structure for this new request */
	if(!(w = worker->free_queries)) {
		/* we could get this due to a slow tcp incoming query, 
		   that started before we performed listen_pushback */
		verbose(VERB_DETAIL, "worker: too many incoming requests "
			"active. dropping incoming query.");
		worker->stats.num_query_list_exceeded++;
		comm_point_drop_reply(repinfo);
		query_info_clear(&qinfo);
		return 0;
	}
	w->state.edns = edns;
	worker->free_queries = w->next;
	worker->num_requests ++;
	log_assert(worker->num_requests <= worker->request_size);
	if(worker->num_requests == worker->request_size)  {
		/* the max request number has been reached, stop accepting */
		listen_pushback(worker->front);
	}

	/* init request */
	w->next = NULL;
	w->state.query_hash = h;
	memcpy(&w->query_reply, repinfo, sizeof(struct comm_reply));
	memcpy(&w->state.qinfo, &qinfo, sizeof(struct query_info));
	memcpy(&w->query_id, ldns_buffer_begin(c->buffer), sizeof(uint16_t));
	w->state.query_flags = ldns_buffer_read_u16_at(c->buffer, 2);

	/* answer it */
	w->state.buf = c->buffer;
	worker_process_query(worker, w, module_event_new);
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
			worker->need_to_restart = 0;
			comm_base_exit(worker->base);
			break;
		case SIGQUIT:
			log_info("caught signal SIGQUIT");
			worker->need_to_restart = 0;
			comm_base_exit(worker->base);
			break;
		case SIGTERM:
			log_info("caught signal SIGTERM");
			worker->need_to_restart = 0;
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
	size_t i;
	for(i=0; i<worker->request_size; i++) {
		struct work_query* q = (struct work_query*)calloc(1,
			sizeof(struct work_query));
		if(!q) return 0;
		q->state.buf = worker->front->udp_buff;
		q->state.scratch = worker->scratchpad;
		q->state.region = region_create_custom(malloc, free, 1024, 
			64, 16, 0);
		if(!q->state.region) {
			free(q);
			return 0;
		}
		q->state.env = &worker->env;
		q->state.work_info = q;
		q->next = worker->free_queries;
		worker->free_queries = q;
		q->all_next = worker->all_queries;
		worker->all_queries = q;
	}
	return 1;
}

/** delete request list */
static void
reqs_delete(struct worker* worker)
{
	struct work_query* q = worker->all_queries;
	struct work_query* n;
	while(q) {
		n = q->all_next;
		log_assert(q->state.env->worker == worker);
		/* comm_reply closed in outside_network_delete */
		query_info_clear(&q->state.qinfo);
		region_destroy(q->state.region);
		free(q);
		q = n;
	}
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
		cfg->num_ifs, cfg->do_ip4, cfg->do_ip6, startport, 
		cfg->do_tcp?cfg->outgoing_num_tcp:0);
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
	worker->scratchpad = region_create_custom(malloc, free, 
		65536, 8192, 32, 1);
	if(!worker->scratchpad) {
		log_err("malloc failure");
		worker_delete(worker);
		return 0;
	}
	worker->request_size = cfg->num_queries_per_thread;
	if(!reqs_init(worker)) {
		worker_delete(worker);
		return 0;
	}

	server_stats_init(&worker->stats);
	alloc_init(&worker->alloc, &worker->daemon->superalloc, 
		worker->thread_num);
	worker->env = *worker->daemon->env;
	worker->env.worker = worker;
	worker->env.alloc = &worker->alloc;
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
	server_stats_log(&worker->stats, worker->thread_num);
	reqs_delete(worker);
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
	region_destroy(worker->scratchpad);
	free(worker);
}

int 
worker_send_query(ldns_buffer* pkt, struct sockaddr_storage* addr,
        socklen_t addrlen, int timeout, struct module_qstate* q, int use_tcp)
{
	struct worker* worker = q->env->worker;
	if(use_tcp) {
		return pending_tcp_query(worker->back, pkt, addr, addrlen, 
			timeout, worker_handle_reply, q->work_info, 
			worker->rndstate);
	}
	return pending_udp_query(worker->back, pkt, addr, addrlen, timeout,
		worker_handle_reply, q->work_info, worker->rndstate);
}
