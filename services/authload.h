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
#include "util/rbtree.h"
struct worker;
struct daemon;
struct auth_xfer;
struct module_env;
struct auth_load_task;
struct comm_timer;

/**
 * General information for auth load threads. The number of active threads.
 */
struct auth_load_general_info {
	/** lock on this structure. It is after the auth_zones lock, and
	 * after the auth_xfer locks. */
	lock_basic_type lock;
	/** The number of active auth load threads. */
	int num_auth_load_threads;
	/** The number of active downloads, zone transfers in progress.
	 * These are going to need an auth load thread when done. The counter
	 * is decremented when the thread and transfer have completed. */
	int num_auth_transfers;
	/** The auth zone xfer transfer wait list */
	struct auth_xfer* wait_transfer_list;
	/** The last item of the wait_transfer list. */
	struct auth_xfer* wait_transfer_last;
	/** To resume from a waiting item, the callback, at zero timeout is
	 * used. That breaks out of callbacks and handles the items there.
	 * If the timer is set, this is true. */
	int resume_timer_enabled;
	/** The resume timer for the wait_transfer list. */
	struct comm_timer* resume_timer;
	/** The worker env for the resume timer. It is set in the event base
	 * of that worker. */
	struct module_env* resume_env;
};

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
	/** if the thread is inserted in the tree at the worker. */
	int tree_inserted;
	/** the rbtree node for the worker tree of auth load threads. */
	rbnode_type node;

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
	/** The zone is an rpz zone. */
	int is_rpz;

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

	/** time taken for the task */
	struct timeval time_taken;
	/** time taken for the reload part of the task */
	struct timeval time_reload;
	/** memory used for the task */
	size_t mem_used;
};

/**
 * Add a new task to be performed by the auth load thread.
 * It starts a thread.
 * @param xfr: zone transfer to start for.
 * @param worker: worker that is connected to the task.
 * @return false on failure.
 */
int auth_load_add_task_xfr(struct auth_xfer* xfr, struct worker* worker);

/**
 * Add a new task to be performed by the auth load thread.
 * It starts a thread. This writes to zonefile.
 * @param name: name of zone, wireformat.
 * @param namelen: length of name.
 * @param dclass: class of the zone.
 * @param env: the module env.
 * @param chunk_list: for writes after http, the chunk list with data.
 * @return false on failure to create the thread.
 */
int auth_load_add_task_write(uint8_t* name, size_t namelen, uint16_t dclass,
	struct module_env* env, struct auth_chunk* chunk_list);

/** See if there is a quit signal, true if so. */
int auth_load_thread_poll_for_quit(struct auth_load_thread* thr);

/**
 * Create auth load info structure.
 * @return NULL on failure.
 */
struct auth_load_general_info* auth_load_info_create(void);

/**
 * Delete auth load info structure.
 * @param auth_load_info: to delete.
 */
void auth_load_info_delete(struct auth_load_general_info* auth_load_info);

/**
 * Grab a new thread from the auth load count.
 * @param env: with auth_load_info with the active thread count.
 *	and config file, with configured maximum.
 * @return false on failure, like too many active, true if successful.
 */
int auth_load_info_grab_thread(struct module_env* env);

/**
 * Release thread from auth load count. It is done.
 * @param env: with auth_load_info with the active thread count.
 */
void auth_load_info_release_thread(struct module_env* env);

/**
 * Grab a new transfer in progress, this is limited by the auth load count.
 * @param env: with auth_load_info with the active transfer number.
 *	and config file, with configured maximum.
 * @return false on failure, like too many active, true if successful.
 */
int auth_load_info_grab_transfer_in_progress(struct module_env* env);

/**
 * Release transfer from active count. It is done.
 * @param env: with auth_load_info with the active transfer count.
 */
void auth_load_info_release_transfer_in_progress(struct module_env* env);

/**
 * Make transfer active in the in progress transfer count.
 * @param xfr: the xfr is in progress
 * @return false if not possible.
 */
int xfr_transfer_grab_active(struct auth_xfer* xfr);

/**
 * Release active transfer from in progress transfer count.
 * @param xfr: the xfr is no longer in progress.
 */
void xfr_transfer_release_active(struct auth_xfer* xfr);

/**
 * Add transfer to wait_transfer list. It can not become active right now.
 * @param xfr: the xfr is put on the wait_transfer list.
 */
void xfr_transfer_wait_active(struct auth_xfer* xfr);

/**
 * Remove xfr from wait_transfer list
 * @param xfr: the xfr is removed from the wait_transfer list.
 */
void xfr_transfer_remove_wait_transfer_list(struct auth_xfer* xfr);

/** The timer callback for the wait_transfer resume timer. */
void auth_load_resume_timer_cb(void* arg);

/** Schedule pick up of waiting transfers. */
void auth_load_schedule_waiting_pickup(struct module_env* env);

/** Compare auth load tree entries, in the worker auth load tree. */
int auth_load_tree_cmp(const void* a, const void* b);

/** Delete tasks for the zone. The write and xfr tasks for that name, class
 * are sent a quit signal, and the thread is joined, as it exits. */
void auth_load_del_zone_tasks(struct worker* worker, uint8_t* name,
	size_t namelen, uint16_t dclass);

/** Stop all the auth load threads. */
void auth_load_stop_threads(struct daemon* daemon);

#endif /* SERVICES_AUTHLOAD_H */
