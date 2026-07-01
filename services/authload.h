/*
 * services/authload.h - authoritative zone load thread
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

#ifndef SERVICES_AUTHLOAD_H
#define SERVICES_AUTHLOAD_H
#include "util/locks.h"
struct worker;
struct auth_xfer;
struct module_env;
struct auth_load_task;

/**
 * The types of notifications that the auth load thread sends around.
 */
enum auth_load_notification_type {
	/** This is sent to make the auth load thread perform exit */
	auth_load_notification_exit
};

/**
 * The auth load thread. The thread runs to load authority zone information
 * and RPZ information into memory. It loads into a copy. Then that is swapped
 * over to the running server. This keeps the server responsive while the
 * information is loaded.
 */
struct auth_load_thread {
	/** the thread number for the thread,
	 * must be first to cast thread arg to int* in checklock code. */
	int threadnum;
	/** thread id, of the io thread */
	ub_thread_type tid;
#ifdef HAVE_GETTID
	/** thread tid, the LWP id */
	pid_t thread_tid;
	/** if logging should include the LWP id */
	int thread_tid_log;
#endif

	/** communication socket pair, that sends commands */
	int commpair[2];
	/** if the thread has to quit */
	int need_to_quit;
	/** the event that listens on the worker to commpair,
	 * it receives content from the auth load thread. */
	void* service_event;
	/** if the event that listens on the worker has
	 * been added to the comm base. */
	int service_event_is_added;

	/** the worker that the auth load is connected to */
	struct worker* worker;

	/** The task that the thread is working on */
	struct auth_load_task* task;
};

/**
 * The types of tasks that the auth load can perform.
 */
enum auth_load_task_type {
	AUTH_LOAD_TASK_TRANSFER,
	AUTH_LOAD_TASK_ZONEFILE_READ,
	AUTH_LOAD_TASK_ZONEFILE_WRITE,
	AUTH_LOAD_TASK_HTTPCHUNKS
};

/**
 * The task for the auth load. The task can be to load a zone transfer, AXFR,
 * IXFR, from zonefile, and from a http read, from chunks.
 */
struct auth_load_task {
	/** The type of the task */
	enum auth_load_task_type task_type;
	/** The task is connected with this worker */
	struct worker* worker;

	/** The zone name */
	uint8_t* name;
	/** The zone namelen */
	size_t namelen;
	/** The zone class */
	uint16_t dclass;

	/** name of the host that the transfer comes from. */
	char* host;
	/** file part of the url that the transfer comes from, or NULL. */
	char* file;
	/** Set if the host is http transfer, if false it is AXFR or IXFR. */
	int on_http;
	/** Set if the transfer is doing IXFR */
	int on_ixfr;
	/** Set if the transfer is an IXFR but we detected an AXFR contents */
	int on_ixfr_is_axfr;

	/** Set if the ixfr failed. (So that there can be backoff to AXFR). */
	int ixfr_fail;

	/** current serial (from SOA), if we have no zone, 0
	 * This is for checking the IXFR result. */
	uint32_t serial;

	/** the data chunks, or NULL, to process. */
	struct auth_chunk* chunks_first;
	/** last data chunk */
	struct auth_chunk* chunks_last;
	/** size of data in data chunks. */
	size_t chunks_total;
};

/**
 * Add a new task to be performed by the auth load thread.
 * It starts a thread, or makes a wait list item.
 * @param xfr: zone transfer to start for.
 * @param worker: worker that is connected to the task.
 * @return false on failure.
 */
int auth_load_add_task_xfr(struct auth_xfer* xfr, struct worker* worker);

/** See if there is a quit signal, true if so. */
int auth_load_thread_poll_for_quit(struct auth_load_thread* thr);

#endif /* SERVICES_AUTHLOAD_H */
