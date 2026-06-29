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

	if(task->on_http)
		task->task_type = AUTH_LOAD_TASK_HTTPCHUNKS;
	else	task->task_type = AUTH_LOAD_TASK_TRANSFER;
	return 1;
}

/** Create xfr task */
static struct auth_load_task*
auth_load_task_create_xfr(struct auth_xfer* xfr, struct worker* worker)
{
	struct auth_load_task* task = auth_load_task_create();
	if(!task) {
		log_err("out of memory");
		return 0;
	}
	task->worker = worker;
	if(!auth_load_task_pickup_xfr(task, xfr)) {
		log_err("out of memory");
		auth_load_task_delete(task);
		return 0;
	}
	return task;
}

/** See if there is a quit signal, true if so. */
static int
auth_load_thread_poll_for_quit(struct auth_load_thread* thr)
{
	int inevent, loopexit = 0;
	uint8_t cmd;
	ssize_t ret;

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

/** In the auth load thread, process the task */
static int
auth_load_thread_process(struct auth_load_thread* thr)
{
	(void)thr;
	return 0;
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
	struct auth_xfer* xfr;
	struct module_env* env;
	int ixfr_fail;

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
	lock_rw_rdlock(&thr->task->worker->env.auth_zones->lock);
	xfr = auth_xfer_find(thr->task->worker->env.auth_zones,
		thr->task->name, thr->task->namelen, thr->task->dclass);
	if(!xfr) {
		lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);
		verbose(VERB_ALGO, "auth load: xfr is gone");
		auth_load_thread_delete(thr);
		return;
	}
	lock_basic_lock(&xfr->lock);
	lock_rw_unlock(&thr->task->worker->env.auth_zones->lock);
	env = &thr->task->worker->env;
	ixfr_fail = thr->task->ixfr_fail;
	auth_load_thread_delete(thr);
	xfr_process_load_end_transfer(xfr, env, recv_item, ixfr_fail);
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
	fd_set_nonblock(thr->commpair[0]);
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
	int can_run = 0;

	/* Check auth load count */
	can_run = 1;

	/* Create new thread */
	task = auth_load_task_create_xfr(xfr, worker);
	if(!task)
		return 0;
	if(can_run) {
		if(!auth_load_start_thread(task))
			return 0;
		return 1;
	}

	/* Make wait item */
	return 0;
}
