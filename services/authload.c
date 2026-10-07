/*
 * services/authload.c - authoritative zone load thread
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
 * This file contains the auth load thread. This loads authority zone
 * and RPZ zone information in a thread, in a separate memory structure.
 * When it is done, the information is swapped over to the running server.
 */

#include "config.h"
#include "services/authload.h"
#include "daemon/worker.h"
#include "daemon/daemon.h"
#include "services/authzone.h"
#include "libunbound/authload.h"
#include "util/net_help.h"
#include "util/log.h"
#include "util/ub_event.h"
#include "util/timeval_func.h"
#include "util/data/dname.h"

/** Get memory use of buffer. */
static size_t
buffer_get_mem(struct sldns_buffer* buf)
{
	if(!buf) return 0;
	return sizeof(*buf) + (buf->_data?buf->_capacity:0);
}

/** Auth load notification to string, for descriptive purposes. */
static const char*
auth_load_notification_to_string(enum auth_load_notification_type status)
{
	switch(status) {
	case auth_load_notification_exit:
		return "auth_load_notification_exit";
	default:
		break;
	}
	return "unknown_auth_load_notification_value";
}

/** delete chunks */
static void
auth_chunk_list_delete(struct auth_chunk* first)
{
	struct auth_chunk* c = first, *cn;
	while(c) {
		cn = c->next;
		free(c->data);
		free(c);
		c = cn;
	}
}

/** Delete auth load task item */
static void
auth_load_task_delete(struct auth_load_task* task)
{
	if(!task)
		return;
	free(task->name);
	free(task->host);
	free(task->file);
	auth_chunk_list_delete(task->chunks_first);
	free(task);
}

/** Create new auth load task item */
static struct auth_load_task*
auth_load_task_create(void)
{
	struct auth_load_task* task = (struct auth_load_task*)calloc(1,
		sizeof(*task));
	return task;
}

/** Pick up the work content of task transfer of auth xfr */
static int
auth_load_task_pickup_xfr(struct auth_load_task* task, struct auth_xfer* xfr)
{
	task->name = memdup(xfr->name, xfr->namelen);
	if(!task->name)
		return 0;
	task->namelen = xfr->namelen;
	task->dclass = xfr->dclass;
	if(xfr->task_transfer->master && xfr->task_transfer->master->host) {
		task->host = strdup(xfr->task_transfer->master->host);
		if(!task->host)
			return 0;
	}
	if(xfr->task_transfer->master && xfr->task_transfer->master->file) {
		task->file = strdup(xfr->task_transfer->master->file);
		if(!task->file)
			return 0;
	}
	if(xfr->task_transfer->master)
		task->on_http = xfr->task_transfer->master->http;
	task->on_ixfr = xfr->task_transfer->on_ixfr;
	task->on_ixfr_is_axfr = xfr->task_transfer->on_ixfr_is_axfr;
	task->serial = xfr->serial;
	if(xfr->task_transfer->chunks_first) {
		task->chunks_first = xfr->task_transfer->chunks_first;
		task->chunks_last = xfr->task_transfer->chunks_last;
		task->chunks_total = xfr->task_transfer->chunks_total;
		/* The task now has the chunks. Remove them from the
		 * xfr structure. */
		xfr->task_transfer->chunks_first = 0;
		xfr->task_transfer->chunks_last = 0;
		xfr->task_transfer->chunks_total = 0;
	}
	task->is_rpz = xfr->is_rpz;

	if(task->on_http)
		task->task_type = AUTH_LOAD_TASK_HTTPCHUNKS;
	else	task->task_type = AUTH_LOAD_TASK_TRANSFER;
	return 1;
}

/** Pick up the work content of task transfer of auth zonefile write */
static int
auth_load_task_pickup_write(struct auth_load_task* task, uint8_t* name,
	size_t namelen, uint16_t dclass, struct auth_chunk* chunk_list)
{
	task->name = memdup(name, namelen);
	if(!task->name)
		return 0;
	task->namelen = namelen;
	task->dclass = dclass;
	task->chunks_first = chunk_list;

	task->task_type = AUTH_LOAD_TASK_ZONEFILE_WRITE;
	return 1;
}

/** Create xfr task */
static struct auth_load_task*
auth_load_task_create_xfr(struct auth_xfer* xfr, struct worker* worker)
{
	struct auth_load_task* task = auth_load_task_create();
	if(!task) {
		log_err("out of memory");
		return NULL;
	}
	task->worker = worker;
	if(!auth_load_task_pickup_xfr(task, xfr)) {
		log_err("out of memory");
		auth_load_task_delete(task);
		return NULL;
	}
	return task;
}

/** Create write task */
static struct auth_load_task*
auth_load_task_create_write(uint8_t* name, size_t namelen, uint16_t dclass,
	struct module_env* env, struct auth_chunk* chunk_list)
{
	struct auth_load_task* task = auth_load_task_create();
	if(!task) {
		log_err("out of memory");
		return 0;
	}
	task->worker = env->worker;
	if(!auth_load_task_pickup_write(task, name, namelen, dclass,
		chunk_list)) {
		log_err("out of memory");
		auth_load_task_delete(task);
		return 0;
	}
	return task;
}

int
auth_load_thread_poll_for_quit(struct auth_load_thread* thr)
{
	int inevent, loopexit = 0;
	uint8_t cmd;
	ssize_t ret;

	if(!thr)
		return 0;
	if(thr->need_to_quit)
		return 1;
	/* Is there data? */
	if(!sock_poll_timeout(thr->commpair[1], 0, 1, 0, &inevent)) {
		log_err("auth_load_thread_poll_for_quit: poll failed");
		return 0;
	}
	if(!inevent)
		return 0;

	/* Read the data */
	while(1) {
		if(++loopexit > 200) {
			log_err("auth_load_thread_poll_for_quit: recv loops %s",
				sock_strerror(errno));
			return 0;
		}
		ret = recv(thr->commpair[1], ((char*)&cmd), sizeof(cmd), 0);
		if(ret == -1) {
			if(
#ifndef USE_WINSOCK
				errno == EINTR || errno == EAGAIN
#  ifdef EWOULDBLOCK
				|| errno == EWOULDBLOCK
#  endif
#else
				WSAGetLastError() == WSAEINTR ||
				WSAGetLastError() == WSAEINPROGRESS ||
				WSAGetLastError() == WSAEWOULDBLOCK
#endif
				)
				continue; /* Try again. */
			log_err("auth_load_thread_poll_for_quit: recv: %s",
				sock_strerror(errno));
			return 0;
		} else if(ret == 0) {
			log_err("auth_load_thread_poll_for_quit: recv: EOF");
			return 0;
		}
		break;
	}
	if(cmd == auth_load_notification_exit) {
		thr->need_to_quit = 1;
		verbose(VERB_ALGO, "auth load: exit notification received");
		return 1;
	}
	log_err("auth_load_thread_poll_for_quit: unknown notification status "
		"received: %d %s", cmd, auth_load_notification_to_string(cmd));
	return 0;
}

/** Signal the worker connected to an auth load thread the status */
static void
auth_load_thread_signal_worker(struct auth_load_thread* thr, int status)
{
	int outevent, loopexit = 0;
	ssize_t ret;
	uint8_t to_send;
	verbose(VERB_ALGO, "auth load thread: send status %d", status);
	/* Make a blocking attempt to send. But meanwhile stay responsive,
	 * once in a while for quit commands. In case the server has to quit. */
	/* see if there is incoming quit signals */
	if(auth_load_thread_poll_for_quit(thr))
		return;
	to_send = (uint8_t)status;
	while(1) {
		if(++loopexit > 200) {
			log_err("auth load thread: could not send status");
			return;
		}
		/* wait for socket to become writable */
		if(!sock_poll_timeout(thr->commpair[1],
			200, /* msec wait before check for quit, and loop to
				wait again. */
			0, 1, &outevent)) {
			log_err("auth load thread: poll failed");
			return;
		}
		if(auth_load_thread_poll_for_quit(thr))
			return;
		if(!outevent)
			continue;
		ret = send(thr->commpair[1], &to_send, 1, 0);
		if(ret == -1) {
			if(
#ifndef USE_WINSOCK
				errno == EINTR || errno == EAGAIN
#  ifdef EWOULDBLOCK
				|| errno == EWOULDBLOCK
#  endif
#else
				WSAGetLastError() == WSAEINTR ||
				WSAGetLastError() == WSAEINPROGRESS ||
				WSAGetLastError() == WSAEWOULDBLOCK
#endif
				)
				continue; /* Try again. */
			log_err("auth load thread signal worker: send: %s",
				sock_strerror(errno));
			return;
		} else if(ret < 1) {
			continue;
		}
		break;
	}
}

/** Create proxy auth zone structure, that is used to hold the data
 * that is processed. */
static struct auth_zone*
auth_zone_create_proxy(uint8_t* nm, size_t nmlen, uint16_t dclass)
{
	struct auth_zone* z = (struct auth_zone*)calloc(1, sizeof(*z));
	if(!z) {
		return NULL;
	}
	z->node.key = z;
	z->dclass = dclass;
	z->namelen = nmlen;
	z->namelabs = dname_count_labels(nm);
	z->name = memdup(nm, nmlen);
	if(!z->name) {
		free(z);
		return NULL;
	}
	rbtree_init(&z->data, &auth_data_cmp);
	if(!(z->rpz = rpz_create_empty())) {
		free(z->name);
		free(z);
		return NULL;
	}
	return z;
}

/** Delete proxy auth zone structure */
static void
auth_zone_delete_proxy(struct auth_zone* z)
{
	if(!z)
		return;
	traverse_postorder(&z->data, auth_data_del, NULL);
	if(z->rpz)
		rpz_delete(z->rpz);
	free(z->name);
	free(z);
}

/** Calculate memory use of the authload thread for this task.
 * The size of the task struct, with the data chunks, and the proxy auth zone
 * structure that is created while the other auth zone is used for queries,
 * and other added memory.
 */
static void
auth_load_calc_mem(struct auth_load_task* task, struct auth_zone* z,
	size_t other)
{
	size_t m = 0;
	if(verbosity < 8) {
		task->mem_used = 0;
		return;
	}
	m += other;
	m += sizeof(*task);
	m += task->namelen;
	m += getmem_str(task->host);
	m += getmem_str(task->file);
	m += task->chunks_total;
	m += auth_zone_get_mem(z);
	task->mem_used = m;
}

/** Swap the data contents of the rpz structure. */
static void
auth_load_swap_rpz(struct rpz* rpz, struct rpz* proxyrpz)
{
	struct local_zones* local_zones = proxyrpz->local_zones;
	struct respip_set* respip_set = proxyrpz->respip_set;
	struct clientip_synthesized_rrset* client_set = proxyrpz->client_set;
	struct clientip_synthesized_rrset* ns_set = proxyrpz->ns_set;
	struct local_zones* nsdname_zones = proxyrpz->nsdname_zones;

	proxyrpz->local_zones = rpz->local_zones;
	proxyrpz->respip_set = rpz->respip_set;
	proxyrpz->client_set = rpz->client_set;
	proxyrpz->ns_set = rpz->ns_set;
	proxyrpz->nsdname_zones = rpz->nsdname_zones;

	rpz->local_zones = local_zones;
	rpz->respip_set = respip_set;
	rpz->client_set = client_set;
	rpz->ns_set = ns_set;
	rpz->nsdname_zones = nsdname_zones;
}

/** Swap the final zone contents with the live zone */
static void
auth_load_swap_zone(struct auth_load_thread* thr, struct auth_zone* proxyz)
{
	rbtree_type data;
	struct auth_zone* z;
	struct timeval start, end;
	if(gettimeofday(&start, NULL) < 0)
		log_err("gettimeofday: %s", strerror(errno));

	lock_rw_rdlock(&thr->task->worker->env.auth_zones->lock);
	z = auth_zone_find(thr->task->worker->env.auth_zones,
		thr->task->name, thr->task->namelen, thr->task->dclass);
	if(!z) {
		lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);
		verbose(VERB_ALGO, "auth zone missing after auth load.");
		return;
	}
	lock_rw_wrlock(&z->lock);
	lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);

	data = proxyz->data;
	proxyz->data = z->data;
	z->data = data;

	if(z->rpz && proxyz->rpz)
		auth_load_swap_rpz(z->rpz, proxyz->rpz);

	lock_rw_unlock(&z->lock);

	if(gettimeofday(&end, NULL) < 0)
		log_err("gettimeofday: %s", strerror(errno));
	timeval_subtract(&thr->task->time_reload, &end, &start);
}

/** Process zonefile write task */
static int
auth_load_process_write(struct auth_load_thread* thr)
{
	struct auth_load_task* task = thr->task;
	struct auth_chunk* chunk_list;

	chunk_list = task->chunks_first;
	task->chunks_first = NULL;

	/* Finds the zone, gets a readlock, writes chunks or zonefile,
	 * and deletes chunk_list if any. */
	zone_write_after_update_reacq(task->name, task->namelen, task->dclass,
		&task->worker->env, chunk_list, thr);

	return 1;
}

/** Process http transfer */
static int
auth_load_process_http(struct auth_load_thread* thr)
{
	struct auth_load_task* task = thr->task;
	struct sldns_buffer* scratch_buffer;
	struct auth_zone* z;
	size_t scratch_mem;

	scratch_buffer = sldns_buffer_new(sldns_buffer_capacity(
		thr->task->worker->env.scratch_buffer));
	if(!scratch_buffer) {
		log_err("out of memory");
		return 0;
	}
	scratch_mem = buffer_get_mem(scratch_buffer);
	z = auth_zone_create_proxy(task->name, task->namelen, task->dclass);
	if(!z) {
		log_err("out of memory");
		sldns_buffer_free(scratch_buffer);
		return 0;
	}
	if(auth_load_thread_poll_for_quit(thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}

	xfr_http_preview(task->file, task->chunks_first);
	if(!xfr_http_syntax_check(task->name, task->namelen, task->dclass,
		task->host, task->file, task->chunks_first, scratch_buffer)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}
	if(auth_load_thread_poll_for_quit(thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}
	if(!xfr_apply_http(task->name, task->namelen, task->host, task->file,
		task->chunks_first, z, scratch_buffer, thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}
	sldns_buffer_free(scratch_buffer);
	if(z->rpz)
		rpz_finish_config(z->rpz);
	if(auth_load_thread_poll_for_quit(thr)) {
		auth_zone_delete_proxy(z);
		return 0;
	}

	auth_load_calc_mem(task, z, scratch_mem);
	auth_load_swap_zone(thr, z);
	auth_zone_delete_proxy(z);
	return 1;
}

/** Copy RRset and append it to the domain, update last pointer. */
static int
rrset_append_copy(struct auth_data* domain, struct auth_rrset* rrset,
	struct auth_rrset** last)
{
	struct auth_rrset* s = calloc(1, sizeof(*s));
	if(!s)
		return 0;
	s->type = rrset->type;
	s->data = (struct packed_rrset_data*)memdup(rrset->data,
		packed_rrset_sizeof(rrset->data));
	if(!s->data) {
		free(s);
		return 0;
	}
	packed_rrset_ptr_fixup(s->data);
	if(!*last)
		domain->rrsets = s;
	else	(*last)->next = s;
	*last = s;
	return 1;
}

/** Copy the existing zone for modification */
static int
auth_load_copy_into_zone(struct auth_load_thread* thr, struct auth_zone* proxyz)
{
	int count = 0;
	struct auth_zone* z;
	struct auth_data* d;
	lock_rw_rdlock(&thr->task->worker->env.auth_zones->lock);
	z = auth_zone_find(thr->task->worker->env.auth_zones,
		thr->task->name, thr->task->namelen, thr->task->dclass);
	if(!z) {
		lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);
		verbose(VERB_ALGO, "auth zone missing for copy for IXFR.");
		return 0;
	}
	lock_rw_rdlock(&z->lock);
	lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);

	/* Copy from z into proxyz. */
	RBTREE_FOR(d, struct auth_data*, &z->data) {
		struct auth_rrset* rrset, *last = NULL;
		struct auth_data* proxy_d = az_domain_create(proxyz,
			d->name, d->namelen);
		if(!proxy_d) {
			log_err("out of memory");
			lock_rw_unlock(&z->lock);
			return 0;
		}
		for(rrset = d->rrsets; rrset; rrset=rrset->next) {
			if(!rrset_append_copy(proxy_d, rrset, &last)) {
				log_err("out of memory");
				lock_rw_unlock(&z->lock);
				return 0;
			}
			if((count++)%10000 == 0) {
				if(auth_load_thread_poll_for_quit(thr)) {
					lock_rw_unlock(&z->lock);
					return 0;
				}
			}
		}
		if((count++)%10000 == 0) {
			if(auth_load_thread_poll_for_quit(thr)) {
				lock_rw_unlock(&z->lock);
				return 0;
			}
		}
	}

	lock_rw_unlock(&z->lock);
	return 1;
}

/** Process ixfr transfer */
static int
auth_load_process_ixfr(struct auth_load_thread* thr)
{
	struct auth_load_task* task = thr->task;
	struct sldns_buffer* scratch_buffer;
	struct auth_zone* z;
	size_t scratch_mem;

	scratch_buffer = sldns_buffer_new(sldns_buffer_capacity(
		thr->task->worker->env.scratch_buffer));
	if(!scratch_buffer) {
		log_err("out of memory");
		return 0;
	}
	scratch_mem = buffer_get_mem(scratch_buffer);
	z = auth_zone_create_proxy(task->name, task->namelen, task->dclass);
	if(!z) {
		log_err("out of memory");
		sldns_buffer_free(scratch_buffer);
		return 0;
	}
	if(auth_load_thread_poll_for_quit(thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}

	/* Copy the existing zone for modification, that uses a read lock.
	 * That then does not interrupt the service of threads. */
	if(!auth_load_copy_into_zone(thr, z)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}
	if(!xfr_apply_ixfr(task->chunks_first, task->serial, z,
		scratch_buffer, thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}
	sldns_buffer_free(scratch_buffer);
	if(z->rpz)
		rpz_finish_config(z->rpz);
	if(auth_load_thread_poll_for_quit(thr)) {
		auth_zone_delete_proxy(z);
		return 0;
	}

	auth_load_calc_mem(task, z, scratch_mem);
	auth_load_swap_zone(thr, z);
	auth_zone_delete_proxy(z);
	return 1;
}

/** Process axfr transfer */
static int
auth_load_process_axfr(struct auth_load_thread* thr)
{
	struct auth_load_task* task = thr->task;
	struct sldns_buffer* scratch_buffer;
	struct auth_zone* z;
	size_t scratch_mem;

	scratch_buffer = sldns_buffer_new(sldns_buffer_capacity(
		thr->task->worker->env.scratch_buffer));
	if(!scratch_buffer) {
		log_err("out of memory");
		return 0;
	}
	scratch_mem = buffer_get_mem(scratch_buffer);
	z = auth_zone_create_proxy(task->name, task->namelen, task->dclass);
	if(!z) {
		log_err("out of memory");
		sldns_buffer_free(scratch_buffer);
		return 0;
	}
	if(auth_load_thread_poll_for_quit(thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}

	if(!xfr_apply_axfr(task->chunks_first, z, scratch_buffer, thr)) {
		sldns_buffer_free(scratch_buffer);
		auth_zone_delete_proxy(z);
		return 0;
	}
	sldns_buffer_free(scratch_buffer);
	if(z->rpz)
		rpz_finish_config(z->rpz);
	if(auth_load_thread_poll_for_quit(thr)) {
		auth_zone_delete_proxy(z);
		return 0;
	}

	auth_load_calc_mem(task, z, scratch_mem);
	auth_load_swap_zone(thr, z);
	auth_zone_delete_proxy(z);
	return 1;
}


/** In the auth load thread, process the task */
static int
auth_load_thread_process(struct auth_load_thread* thr)
{
	struct auth_load_task* task = thr->task;
	struct timeval start, end;
	if(gettimeofday(&start, NULL) < 0)
		log_err("gettimeofday: %s", strerror(errno));

	/* apply data */
	if(task->task_type == AUTH_LOAD_TASK_ZONEFILE_WRITE) {
		if(!auth_load_process_write(thr))
			return 0;
	} else if(task->on_http) {
		if(!auth_load_process_http(thr))
			return 0;
	} else if(task->on_ixfr && !task->on_ixfr_is_axfr) {
		if(!auth_load_process_ixfr(thr))
			return 0;
	} else {
		if(!auth_load_process_axfr(thr))
			return 0;
	}

	if(gettimeofday(&end, NULL) < 0)
		log_err("gettimeofday: %s", strerror(errno));
	timeval_subtract(&thr->task->time_taken, &end, &start);
	return 1;
}

/** The auth load thread. The thread main function. */
static void*
auth_load_thread_main(void* arg)
{
	struct auth_load_thread* thr = (struct auth_load_thread*)arg;
	int s;
	const char name[16] = "unbound/authld"; /* seems to be the safest size
                                                    between different OSes */

#if defined(HAVE_GETTID) && !defined(THREADS_DISABLED)
	thr->thread_tid = gettid();
	if(thr->thread_tid_log)
		log_thread_set(&thr->thread_tid);
	else
#endif
		log_thread_set(&thr->threadnum);

	ub_thread_setname(ub_thread_self(), name);
	(void)name; /* When setname is not defined, ignore the name variable. */

	verbose(VERB_ALGO, "start auth load thread");
	s = auth_load_thread_process(thr);
	/* The result is sent to the worker, that reaps the thread. */
	auth_load_thread_signal_worker(thr, s);
	verbose(VERB_ALGO, "stop auth load thread");
	return NULL;
}

/** Delete auth load thread structure */
static void
auth_load_thread_delete(struct auth_load_thread* thr)
{
	if(!thr)
		return;
	if(thr->tree_inserted) {
		rbtree_delete(&thr->worker->auth_load_tree, thr);
		thr->tree_inserted = 0;
	}
	if(thr->service_event && thr->service_event_is_added) {
		ub_event_del(thr->service_event);
		thr->service_event_is_added = 0;
	}
	if(thr->service_event)
		ub_event_free(thr->service_event);
	if(thr->commpair[0] != -1)
		sock_close(thr->commpair[0]);
	if(thr->commpair[1] != -1)
		sock_close(thr->commpair[1]);
	auth_load_task_delete(thr->task);
	free(thr);
}

/** Create auth load thread structure */
static struct auth_load_thread*
auth_load_thread_create(struct auth_load_task* task)
{
	int numworkers;
	struct auth_load_thread* thr = (struct auth_load_thread*)calloc(1,
		sizeof(*thr));
	if(!thr)
		return NULL;
	numworkers = task->worker->daemon->num;
	/* This number is printed into the logs */
	thr->threadnum = numworkers+3;
	thr->task = task;
	thr->commpair[0] = -1;
	thr->commpair[1] = -1;
	if(!create_socketpair(thr->commpair, task->worker->daemon->rand)) {
		auth_load_thread_delete(thr);
		return NULL;
	}
#ifdef HAVE_GETTID
        thr->thread_tid_log = task->worker->env.cfg->log_thread_id;
#endif
	return thr;
}

/** The worker routine that services the auth load connection. */
void
worker_auth_load_service_cb(int ATTR_UNUSED(fd), short ATTR_UNUSED(bits),
	void* arg)
{
	struct auth_load_thread* thr = (struct auth_load_thread*)arg;
	uint8_t recv_item;
	ssize_t ret;
	struct auth_xfer* xfr = NULL;
	struct auth_chunk* chunk_list;
	struct module_env* env = &thr->task->worker->env;
	int ixfr_fail;
	struct timeval time_taken, time_reload;
	size_t mem_used, chunks_total;
	enum auth_load_task_type task_type;

	log_assert(thr->commpair[0] >= 0);
	ret = recv(thr->commpair[0], &recv_item, 1, 0);
	if(ret == -1) {
		if(
#ifndef USE_WINSOCK
			errno == EINTR || errno == EAGAIN
#  ifdef EWOULDBLOCK
			|| errno == EWOULDBLOCK
#  endif
#else
			WSAGetLastError() == WSAEINTR ||
			WSAGetLastError() == WSAEINPROGRESS
#endif
			)
			return; /* Continue later. */
#ifdef USE_WINSOCK
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			ub_winsock_tcp_wouldblock(thr->service_event,
				UB_EV_READ);
			return; /* Continue later. */
		}
#endif
		log_err("read status from auth load thread, recv: %s",
			sock_strerror(errno));
		return;
	} else if(ret == 0) {
		verbose(VERB_ALGO, "closed connection from auth load thread");
		/* handle this like an error */
		recv_item = 0;
	/* ret<1: No short read on 1 byte, to continue later on */
	}

	/* Deal with the result of auth load thread */
	verbose(VERB_ALGO, "auth load status is %d", (int)recv_item);
	verbose(VERB_ALGO, "join with auth load thread");
	ub_thread_join(thr->tid);
	verbose(VERB_ALGO, "joined with auth load thread");

	task_type = thr->task->task_type;
	if(task_type != AUTH_LOAD_TASK_ZONEFILE_WRITE) {
		lock_rw_rdlock(&thr->task->worker->env.auth_zones->lock);
		xfr = auth_xfer_find(thr->task->worker->env.auth_zones,
			thr->task->name, thr->task->namelen, thr->task->dclass);
		if(!xfr) {
			lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);
			verbose(VERB_ALGO, "auth load: xfr is gone");
			auth_load_thread_delete(thr);
			auth_load_info_release_thread(env);
			return;
		}
		lock_basic_lock(&xfr->lock);
		lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);
	}
	ixfr_fail = thr->task->ixfr_fail;
	time_taken = thr->task->time_taken;
	time_reload = thr->task->time_reload;
	mem_used = thr->task->mem_used;
	chunks_total = thr->task->chunks_total;
	if(thr->task->on_http) {
		chunk_list = thr->task->chunks_first;
		thr->task->chunks_first = NULL;
		thr->task->chunks_last = NULL;
		thr->task->chunks_total = 0;
	} else {
		chunk_list = NULL;
	}
	auth_load_thread_delete(thr);
	auth_load_info_release_thread(env);
	if(task_type == AUTH_LOAD_TASK_ZONEFILE_WRITE)
		auth_zone_process_load_end_write(env);
	else
		xfr_process_load_end_transfer(xfr, env, recv_item, ixfr_fail,
			&time_taken, &time_reload, mem_used, chunks_total,
			chunk_list);
}

/** Attach worker to the auth load thread. */
static int
auth_load_thread_attach(struct auth_load_thread* thr, struct worker* worker)
{
	/* Setup listener in worker, that connects via a pipe to the
	 * auth load thread.
	 * The listener has to be nonblocking, so the the remote servicing
	 * thread can continue to service DNS queries.
	 * The commpair[1] element can stay blocking, it is used by the
	 * auth load thread. The thread needs to wait at these times, when
	 * it has to check briefly it can use poll. */
	verbose(VERB_ALGO, "auth_load_thread_attach");
	fd_set_nonblock(thr->commpair[0]);
	if(!comm_base_internal(worker->base)) {
		verbose(VERB_ALGO, "auth load thread: no event base");
		return 0;
	}
	thr->service_event = ub_event_new(comm_base_internal(worker->base),
		thr->commpair[0], UB_EV_READ | UB_EV_PERSIST,
		worker_auth_load_service_cb, thr);
	if(!thr->service_event) {
		log_err("out of memory");
		return 0;
	}
	if(ub_event_add(thr->service_event, NULL) != 0) {
		log_err("out of memory");
		return 0;
	}
	thr->service_event_is_added = 1;

	thr->node.key = thr;
	thr->worker = worker;
	if(!rbtree_insert(&worker->auth_load_tree, &thr->node)) {
		log_err("can not insert thread struct in tree, duplicate");
		return 0;
	}
	thr->tree_inserted = 1;
	return 1;
}

/** Create and start the auth load thread, with the task */
static int
auth_load_start_thread(struct auth_load_task* task)
{
	struct auth_load_thread* thr = auth_load_thread_create(task);
	if(!thr) {
		log_err("out of memory");
		auth_load_task_delete(task);
		return 0;
	}
	if(!auth_load_thread_attach(thr, task->worker)) {
		log_err("out of memory");
		auth_load_thread_delete(thr);
		return 0;
	}

	/* Start auth load thread */
	ub_thread_create(&thr->tid, auth_load_thread_main, thr);
	return 1;
}

int auth_load_add_task_xfr(struct auth_xfer* xfr, struct worker* worker)
{
	struct auth_load_task* task;
	verbose(VERB_ALGO, "auth load add task");

	/* Create new thread */
	task = auth_load_task_create_xfr(xfr, worker);
	if(!task)
		return 0;

	verbose(VERB_ALGO, "auth load start thread");
	if(!auth_load_start_thread(task))
		return 0;
	verbose(VERB_ALGO, "auth load thread started");
	return 1;
}

int auth_load_add_task_write(uint8_t* name, size_t namelen, uint16_t dclass,
	struct module_env* env, struct auth_chunk* chunk_list)
{
	struct auth_load_task* task;
	verbose(VERB_ALGO, "auth load add task for zonefile write");

	/* Create new thread */
	task = auth_load_task_create_write(name, namelen, dclass, env,
		chunk_list);
	if(!task)
		return 0;

	verbose(VERB_ALGO, "auth load start thread");
	if(!auth_load_start_thread(task))
		return 0;
	verbose(VERB_ALGO, "auth load thread started");
	return 1;
}

struct auth_load_general_info* auth_load_info_create(void)
{
	struct auth_load_general_info* auth_load_info =
		(struct auth_load_general_info*)calloc(1,
			sizeof(*auth_load_info));
	if(!auth_load_info) {
		log_err("malloc failure");
		return NULL;
	}
	lock_basic_init(&auth_load_info->lock);
	lock_protect(&auth_load_info->lock,
		&auth_load_info->num_auth_load_threads,
		sizeof(auth_load_info->num_auth_load_threads));
	lock_protect(&auth_load_info->lock,
		&auth_load_info->num_auth_transfers,
		sizeof(auth_load_info->num_auth_transfers));
	lock_protect(&auth_load_info->lock,
		&auth_load_info->wait_transfer_list,
		sizeof(auth_load_info->wait_transfer_list));
	lock_protect(&auth_load_info->lock,
		&auth_load_info->wait_transfer_last,
		sizeof(auth_load_info->wait_transfer_last));
	lock_protect(&auth_load_info->lock,
		&auth_load_info->resume_timer_enabled,
		sizeof(auth_load_info->resume_timer_enabled));
	lock_protect(&auth_load_info->lock,
		&auth_load_info->resume_timer,
		sizeof(auth_load_info->resume_timer));
	lock_protect(&auth_load_info->lock,
		&auth_load_info->resume_env,
		sizeof(auth_load_info->resume_env));
	return auth_load_info;
}

void auth_load_info_delete(struct auth_load_general_info* auth_load_info)
{
	if(!auth_load_info)
		return;
	lock_basic_destroy(&auth_load_info->lock);
	comm_timer_delete(auth_load_info->resume_timer);
	auth_load_info->resume_timer = NULL;
	free(auth_load_info);
}

int auth_load_info_grab_thread(struct module_env* env)
{
	struct auth_load_general_info* auth_load_info =
		env->worker->daemon->auth_load_info;
	struct config_file* cfg = env->cfg;
	int ret = 0;
	lock_basic_lock(&auth_load_info->lock);
	if(auth_load_info->num_auth_load_threads < cfg->auth_task_threads) {
		ret = 1;
		auth_load_info->num_auth_load_threads++;
	}
	lock_basic_unlock(&auth_load_info->lock);
	return ret;
}

void auth_load_info_release_thread(struct module_env* env)
{
	struct auth_load_general_info* auth_load_info =
		env->worker->daemon->auth_load_info;
	lock_basic_lock(&auth_load_info->lock);
	if(auth_load_info->num_auth_load_threads == 0) {
		verbose(VERB_ALGO, "release of auth load thread, but "
			"num_auth_load_threads not > 0.");
	} else {
		auth_load_info->num_auth_load_threads--;
	}
	lock_basic_unlock(&auth_load_info->lock);
}

int auth_load_info_grab_transfer_in_progress(struct module_env* env)
{
	struct auth_load_general_info* auth_load_info =
		env->worker->daemon->auth_load_info;
	struct config_file* cfg = env->cfg;
	int ret = 0;
	lock_basic_lock(&auth_load_info->lock);
	if(auth_load_info->num_auth_transfers < cfg->auth_task_threads) {
		ret = 1;
		auth_load_info->num_auth_transfers++;
	}
	lock_basic_unlock(&auth_load_info->lock);
	return ret;
}

void auth_load_info_release_transfer_in_progress(struct module_env* env)
{
	struct auth_load_general_info* auth_load_info =
		env->worker->daemon->auth_load_info;
	lock_basic_lock(&auth_load_info->lock);
	if(auth_load_info->num_auth_transfers == 0) {
		verbose(VERB_ALGO, "release of auth load transfer, but "
			"num_auth_transfers not > 0.");
	} else {
		auth_load_info->num_auth_transfers--;
	}
	lock_basic_unlock(&auth_load_info->lock);
}

int
xfr_transfer_grab_active(struct auth_xfer* xfr)
{
	if(xfr->task_transfer->active_transfer) {
		char zname[LDNS_MAX_DOMAINLEN];
		dname_str(xfr->name, zname);
		log_err("task transfer already active when activated transfer "
			"%s", zname);
		return 1;
	}
	xfr->task_transfer->active_transfer = 1;
	return auth_load_info_grab_transfer_in_progress(
		xfr->task_transfer->env);
}

/** Disable resume timer. */
static void
auth_load_disable_resume_timer(struct auth_load_general_info* auth_load_info)
{
	lock_basic_lock(&auth_load_info->lock);
	comm_timer_delete(auth_load_info->resume_timer);
	auth_load_info->resume_timer = NULL;
	auth_load_info->resume_env = NULL;
	auth_load_info->resume_timer_enabled = 0;
	lock_basic_unlock(&auth_load_info->lock);
}

/** Pop the first item from the wait_transfer list */
static struct auth_xfer*
auth_load_wait_transfer_pop_first(struct auth_load_general_info* auth_load_info)
{
	struct auth_xfer* xfr = auth_load_info->wait_transfer_list;
	if(xfr) {
		auth_load_info->wait_transfer_list =
			xfr->task_transfer->wait_transfer_next;
		if(xfr->task_transfer->wait_transfer_next)
			xfr->task_transfer->wait_transfer_next->task_transfer->
			wait_transfer_prev = NULL;
		else	auth_load_info->wait_transfer_last = NULL;
		xfr->task_transfer->on_wait_transfer_list = 0;
		xfr->task_transfer->wait_transfer_prev = NULL;
		xfr->task_transfer->wait_transfer_next = NULL;
	}
	return xfr;
}

void
auth_load_schedule_waiting_pickup(struct module_env* env)
{
	struct auth_load_general_info* auth_load_info =
		env->worker->daemon->auth_load_info;
	lock_basic_lock(&auth_load_info->lock);
	if(auth_load_info->wait_transfer_list &&
		!auth_load_info->resume_timer_enabled) {
		struct timeval tv;
		/* Set a timer for zero time, that makes the callback run
		 * in the event loop, outside of these callback functions.
		 * There it can handle the waiting xfr task. */
		auth_load_info->resume_timer_enabled = 1;
		auth_load_info->resume_env = env;
		auth_load_info->resume_timer = comm_timer_create(
			auth_load_info->resume_env->worker_base,
			auth_load_resume_timer_cb, auth_load_info->resume_env);
		if(!auth_load_info->resume_timer) {
			char zname[LDNS_MAX_DOMAINLEN];
			dname_str(auth_load_info->wait_transfer_list->name,
				zname);
			log_err("out of memory: schedule zone transfer %s",
				zname);
			lock_basic_unlock(&auth_load_info->lock);
			/* The server is still running, but there is no
			 * auth zone update. */
			return;
		}
		memset(&tv, 0, sizeof(tv));
		comm_timer_set(auth_load_info->resume_timer, &tv);
	}
	lock_basic_unlock(&auth_load_info->lock);
}

/** Pick up a waiting transfer */
static int
auth_load_resume_transfer(struct auth_load_general_info* auth_load_info,
	struct module_env* env)
{
	struct auth_xfer* xfr;
	/* Lock the auth zone tree, so that the xfr can not be
	 * deleted while it is picked up from the auth load info.
	 * It can then be locked. The xfr can not be locked while
	 * the auth_load_info is locked, because that lock is
	 * after the xfr lock, and it would create a lock cycle. */
	lock_rw_rdlock(&env->auth_zones->lock);
	lock_basic_lock(&auth_load_info->lock);

	/* If there is no space to start another transfer, do not
	 * pick up another one. */
	if(env->cfg->auth_task_threads != 0 &&
		auth_load_info->num_auth_transfers >=
		env->cfg->auth_task_threads) {
		lock_basic_unlock(&auth_load_info->lock);
		lock_rw_unlock(&env->auth_zones->lock);
		/* There is at least one transfer that is picked up
		 * and in progress. That, when done, is going to
		 * check for a waiting list, and schedule another
		 * resume timer, if needed. */
		return 0;
	}

	xfr = auth_load_wait_transfer_pop_first(auth_load_info);
	lock_basic_unlock(&auth_load_info->lock);
	if(xfr) {
		/* Lock the xfr, after the auth_load_info is unlocked.
		 * It has not been deleted, since the auth_zones lock
		 * is held. */
		lock_basic_lock(&xfr->lock);
	}
	lock_rw_unlock(&env->auth_zones->lock);

	if(xfr) {
		/* Pick up this transfer. */
		xfr_pick_up_transfer(xfr, env);
		/* The xfr is unlocked by the pick up call. */
	} else {
		return 0; /* no more transfers to pick up */
	}
	return 1;
}

/** The timer callback for the auth load wait_transfer resume timer. */
void auth_load_resume_timer_cb(void* arg)
{
	struct module_env* env = (struct module_env*)arg;
	struct auth_load_general_info* auth_load_info =
		env->worker->daemon->auth_load_info;

	/* Disable the timer */
	auth_load_disable_resume_timer(auth_load_info);

	/* If there are waiting xfrs on the wait_transfer list, pick one up. */
	while(1) {
		if(!auth_load_resume_transfer(auth_load_info, env))
			break;
	}
}

void
xfr_transfer_release_active(struct auth_xfer* xfr)
{
	if(!xfr->task_transfer->active_transfer)
		return;
	auth_load_info_release_transfer_in_progress(xfr->task_transfer->env);
	xfr->task_transfer->active_transfer = 0;

	/* Since a transfer is no longer in progress, see if there are
	 * waiting transfers. If so, schedule them to get picked up. */
	auth_load_schedule_waiting_pickup(xfr->task_transfer->env);
}

void
xfr_transfer_wait_active(struct auth_xfer* xfr)
{
	struct auth_load_general_info* auth_load_info =
		xfr->task_transfer->env->worker->daemon->auth_load_info;
	if(xfr->task_transfer->on_wait_transfer_list)
		return;
	lock_basic_lock(&auth_load_info->lock);
	xfr->task_transfer->wait_transfer_prev =
		auth_load_info->wait_transfer_last;
	xfr->task_transfer->wait_transfer_next = NULL;
	if(auth_load_info->wait_transfer_last)
		auth_load_info->wait_transfer_last->task_transfer->
			wait_transfer_next = xfr;
	else	auth_load_info->wait_transfer_list = xfr;
	auth_load_info->wait_transfer_last = xfr;
	lock_basic_unlock(&auth_load_info->lock);
	xfr->task_transfer->on_wait_transfer_list = 1;
}

void
xfr_transfer_remove_wait_transfer_list(struct auth_xfer* xfr)
{
	struct auth_load_general_info* auth_load_info =
		xfr->task_transfer->env->worker->daemon->auth_load_info;
	if(!xfr->task_transfer->on_wait_transfer_list)
		return;
	lock_basic_lock(&auth_load_info->lock);
	if(xfr->task_transfer->wait_transfer_prev)
		xfr->task_transfer->wait_transfer_prev->task_transfer->
			wait_transfer_next =
			xfr->task_transfer->wait_transfer_next;
	else	auth_load_info->wait_transfer_list =
			xfr->task_transfer->wait_transfer_next;
	if(xfr->task_transfer->wait_transfer_next)
		xfr->task_transfer->wait_transfer_next->task_transfer->
			wait_transfer_prev =
			xfr->task_transfer->wait_transfer_prev;
	else	auth_load_info->wait_transfer_last =
			xfr->task_transfer->wait_transfer_prev;
	lock_basic_unlock(&auth_load_info->lock);
	xfr->task_transfer->wait_transfer_prev = NULL;
	xfr->task_transfer->wait_transfer_next = NULL;
	xfr->task_transfer->on_wait_transfer_list = 0;
}

int auth_load_tree_cmp(const void* a, const void* b)
{
	struct auth_load_thread* ta = (struct auth_load_thread*)a;
	struct auth_load_thread* tb = (struct auth_load_thread*)b;
	int r;
	/* compare the task name and type by preference, so it can be
	 * searched for */
	if(!ta->task || !tb->task) {
		if(!ta->task && !tb->task) {
			if(a > b)
				return 1;
			if(a < b)
				return -1;
			return 0;
		}
		if(!ta->task)
			return -1;
		return 1;
	}
	if(ta->task->dclass != tb->task->dclass)
		return (int)tb->task->dclass - (int)ta->task->dclass;
	r = query_dname_compare(ta->task->name, tb->task->name);
	if(r != 0)
		return r;
	/* Name and class are the same, NULL worker ptr is sorted larger than
	 * the ones with tasks, so that it can find with smaller_or_equal
	 * the tasks in the tree with that name. */
	if(!ta->worker && !tb->worker)
		return 0;
	if(!ta->worker)
		return 1;
	if(!tb->worker)
		return -1;
	/* If name, class are the same, there can be more threads, such as
	 * a write thread that takes a while, and a later started xfr thread.*/
	if(a > b)
		return 1;
	if(a < b)
		return -1;
	return 0;
}

/** auth load thread, poll for and handle cmd from auth load thread. */
static int
authload_check_cmd_from_thread(struct auth_load_thread* thr)
{
	int inevent = 0;
	ssize_t ret;
	uint8_t cmd = 0;
	while(1) {
		if(!sock_poll_timeout(thr->commpair[0], 0, 1, 0, &inevent)) {
			log_err("check for cmd from auth load thread: "
				"poll failed");
#ifdef USE_WINSOCK
			ub_winsock_tcp_wouldblock(worker->daemon->
				thr->service_event, UB_EV_READ);
#endif
			return 0;
		}
		if(!inevent) {
#ifdef USE_WINSOCK
			ub_winsock_tcp_wouldblock(worker->daemon->
				thr->service_event, UB_EV_READ);
#endif
			return 0;
		}
		ret = recv(thr->commpair[0], &cmd, 1, 0);
		if(ret == -1) {
			if(
#ifndef USE_WINSOCK
				errno == EINTR || errno == EAGAIN
#  ifdef EWOULDBLOCK
				|| errno == EWOULDBLOCK
#  endif
#else
				WSAGetLastError() == WSAEINTR ||
				WSAGetLastError() == WSAEINPROGRESS
#endif
				)
				return 0; /* Continue later. */
#ifdef USE_WINSOCK
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
				ub_winsock_tcp_wouldblock(thr->service_event,
					UB_EV_READ);
				return 0; /* Continue later. */
			}
#endif
			log_err("read status from auth load thread, recv: %s",
				sock_strerror(errno));
			return 0;
		} else if(ret == 0) {
			verbose(VERB_ALGO, "closed connection from auth load thread");
			return 1;
		/* ret<1: No short read on 1 byte, to continue later on */
		}

		/* Deal with the result of auth load thread */
		verbose(VERB_ALGO, "auth load status is %d", (int)cmd);
		/* It intends to quit, we want to send it quit */
		break;
	}
	return 1;
}

/**
 * Auth load thread, send quit command to the thread. It is blocking, since used
 * on quit and change of auth zones in fastreload.
 * It handles received input from the thread, if any is received.
 */
static void
authload_send_quit_to(struct auth_load_thread* thr)
{
	int outevent, loopexit = 0;
	uint8_t cmd;
	ssize_t ret;
	verbose(VERB_ALGO, "send quit to auth load thread");
	cmd = auth_load_notification_exit;
	while(1) {
		if(++loopexit > 200) {
			log_err("send notification to auth load thread: could not send notification: loop");
			return;
		}
		if(authload_check_cmd_from_thread(thr))
			return; /* thread already exiting */
		/* wait for socket to become writable */
		if(!sock_poll_timeout(thr->commpair[0],
			-1 /* blocking */,
			0, 1, &outevent)) {
			log_err("send notification to auth load thread: poll failed");
			return;
		}
		if(!outevent)
			continue;
		/* keep static analyzer happy; send(-1,..) */
		log_assert(thr->commpair[0] >= 0);
		ret = send(thr->commpair[0], &cmd, 1, 0);
		if(ret == -1) {
			if(
#ifndef USE_WINSOCK
				errno == EINTR || errno == EAGAIN
#  ifdef EWOULDBLOCK
				|| errno == EWOULDBLOCK
#  endif
#else
				WSAGetLastError() == WSAEINTR ||
				WSAGetLastError() == WSAEINPROGRESS ||
				WSAGetLastError() == WSAEWOULDBLOCK
#endif
				)
				continue; /* Try again. */
			log_err("send notification to auth load thread: send: %s",
				sock_strerror(errno));
			return;
		}
		break;
	}
}

void auth_load_del_zone_tasks(struct worker* worker, uint8_t* name,
	size_t namelen, uint16_t dclass)
{
	struct auth_load_thread* thr;
	struct auth_load_thread key;
	struct auth_load_task kt;
	memset(&key, 0, sizeof(key));
	memset(&kt, 0, sizeof(kt));
	key.task = &kt;
	key.worker = NULL; /* this finds entries smallerorequal that are the
		same name and class */
	key.node.key = &key;
	kt.name = name;
	kt.namelen = namelen;
	kt.dclass = dclass;

	while(1) {
		rbnode_type* result = NULL;
		(void)rbtree_find_less_equal(&worker->auth_load_tree, &key,
			&result);
		if(!result)
			break;
		/* Check if the same name and class. If not, there are no
		 * elements of that name and class. */
		thr = (struct auth_load_thread*)result->key;
		if(!thr->task ||
			query_dname_compare(thr->task->name, name) != 0 ||
			thr->task->dclass != dclass)
			break;
		authload_send_quit_to(thr);
		ub_thread_join(thr->tid);
		auth_load_thread_delete(thr);
	}
}

/** Stop the auth load threads for a worker */
static void
auth_load_stop_worker_threads(struct worker* worker)
{
	struct auth_load_thread* thr;
	RBTREE_FOR(thr, struct auth_load_thread*, &worker->auth_load_tree) {
		authload_send_quit_to(thr);
		ub_thread_join(thr->tid);
		thr->tree_inserted = 0; /* no need to delete from the tree */
		auth_load_thread_delete(thr);
	}
}

void auth_load_stop_threads(struct daemon* daemon)
{
	int i;
	for(i=0; i<daemon->num; i++) {
		auth_load_stop_worker_threads(daemon->workers[i]);
	}
}
