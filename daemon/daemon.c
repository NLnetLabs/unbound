/*
 * daemon/daemon.c - collection of workers that handles requests.
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
 * The daemon consists of global settings and a number of workers.
 */

/** buffer size for network connections */
#define BUFSZ 65552

#include "config.h"
#include "daemon/daemon.h"
#include "daemon/worker.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/data/msgreply.h"
#include "services/listen_dnsport.h"
#include <signal.h>

/** How many quit requests happened. */
static int sig_record_quit = 0;
/** How many reload requests happened. */
static int sig_record_reload = 0;

/** used when no other sighandling happens, so we don't die
  * when multiple signals in quick succession are sent to us. */
static RETSIGTYPE record_sigh(int sig)
{
#ifdef LIBEVENT_SIGNAL_PROBLEM
	exit(0);
#endif 
	switch(sig)
	{
		case SIGTERM:
		case SIGQUIT:
		case SIGINT:
			sig_record_quit++;
			break;
		case SIGHUP:
			sig_record_reload++;
			break;
		default:
			log_err("ignoring signal %d", sig);
	}
}

/** 
 * Signal handling during the time when netevent is disabled.
 * Stores signals to replay later.
 */
static void
signal_handling_record()
{
	if( signal(SIGTERM, record_sigh) == SIG_ERR ||
		signal(SIGQUIT, record_sigh) == SIG_ERR ||
		signal(SIGINT, record_sigh) == SIG_ERR ||
		signal(SIGHUP, record_sigh) == SIG_ERR)
		log_err("install sighandler: %s", strerror(errno));
}

/**
 * Replay old signals.
 * @param wrk: worker that handles signals.
 */
static void
signal_handling_playback(struct worker* wrk)
{
	if(sig_record_quit)
		worker_sighandler(SIGTERM, wrk);
	if(sig_record_reload)
		worker_sighandler(SIGHUP, wrk);
	sig_record_quit = 0;
	sig_record_reload = 0;
}

struct daemon* 
daemon_init()
{
	struct daemon* daemon = (struct daemon*)calloc(1, 
		sizeof(struct daemon));
	if(!daemon)
		return NULL;
	signal_handling_record();
	checklock_start();
	daemon->need_to_exit = 0;
	daemon->msg_cache = lruhash_create(HASH_DEFAULT_STARTARRAY,
		HASH_DEFAULT_MAXMEM, msgreply_sizefunc, query_info_compare,
		query_info_delete, reply_info_delete, NULL);
	if(!daemon->msg_cache) {
		free(daemon);
		return NULL;
	}
	alloc_init(&daemon->superalloc, NULL);
	return daemon;	
}

int 
daemon_open_shared_ports(struct daemon* daemon)
{
	log_assert(daemon);
	if(daemon->cfg->port == daemon->listening_port)
		return 1;
	listening_ports_free(daemon->ports);
	if(!(daemon->ports=listening_ports_open(daemon->cfg)))
		return 0;
	daemon->listening_port = daemon->cfg->port;
	return 1;
}

/**
 * Allocate empty worker structures. With backptr and thread-number,
 * from 0..numthread initialised. Used as user arguments to new threads.
 * @param daemon: the daemon with (new) config settings.
 */
static void 
daemon_create_workers(struct daemon* daemon)
{
	int i;
	log_assert(daemon && daemon->cfg);
	daemon->num = daemon->cfg->num_threads;
	daemon->workers = (struct worker**)calloc((size_t)daemon->num, 
		sizeof(struct worker*));
	for(i=0; i<daemon->num; i++) {
		if(!(daemon->workers[i] = worker_create(daemon, i)))
			fatal_exit("malloc failure");
	}
}

/**
 * Close all pipes except for the numbered thread.
 * @param daemon: daemon to close pipes in.
 * @param thr: thread number 0..num-1 of thread to skip.
 */
void close_other_pipes(struct daemon* daemon, int thr)
{
	int i;
	for(i=0; i<daemon->num; i++)
		if(i!=thr) {
			if(daemon->workers[i]->cmd_send_fd != -1) {
				close(daemon->workers[i]->cmd_send_fd);
				daemon->workers[i]->cmd_send_fd = -1;
			}
			if(daemon->workers[i]->cmd_recv_fd != -1) {
				close(daemon->workers[i]->cmd_recv_fd);
				daemon->workers[i]->cmd_recv_fd = -1;
			}
		}
}

/**
 * Function to start one thread. 
 * @param arg: user argument.
 * @return: void* user return value could be used for thread_join results.
 */
static void* 
thread_start(void* arg)
{
	struct worker* worker = (struct worker*)arg;
	int num = worker->thread_num;
	log_thread_set(&worker->thread_num);
	ub_thread_blocksigs();
#if !defined(HAVE_PTHREAD) && !defined(HAVE_SOLARIS_THREADS)
	/* close pipe ends used by main */
	close(worker->cmd_send_fd);
	worker->cmd_send_fd = -1;
	close_other_pipes(worker->daemon, worker->thread_num);
#endif /* no threads */
	if(!worker_init(worker, worker->daemon->cfg, worker->daemon->ports,
		BUFSZ, 0))
		fatal_exit("Could not initialize thread #%d", num);

	worker_work(worker);
	return NULL;
}

/**
 * Fork and init the other threads. Main thread returns for special handling.
 * @param daemon: the daemon with other threads to fork.
 */
static void
daemon_start_others(struct daemon* daemon)
{
	int i;
	log_assert(daemon);
	verbose(VERB_ALGO, "start threads");
	/* skip i=0, is this thread */
	for(i=1; i<daemon->num; i++) {
		ub_thread_create(&daemon->workers[i]->thr_id,
			thread_start, daemon->workers[i]);
#if !defined(HAVE_PTHREAD) && !defined(HAVE_SOLARIS_THREADS)
		/* close pipe end of child */
		close(daemon->workers[i]->cmd_recv_fd);
		daemon->workers[i]->cmd_recv_fd = -1;
#endif /* no threads */
	}
}

/**
 * Stop the other threads.
 * @param daemon: the daemon with other threads.
 */
static void
daemon_stop_others(struct daemon* daemon)
{
	int i;
	log_assert(daemon);
	verbose(VERB_ALGO, "stop threads");
	/* skip i=0, is this thread */
	/* use i=0 buffer for sending cmds; because we are #0 */
	for(i=1; i<daemon->num; i++) {
		worker_send_cmd(daemon->workers[i], 
			daemon->workers[0]->front->udp_buff, worker_cmd_quit);
	}
	/** wait for them to quit */
	for(i=1; i<daemon->num; i++) {
		/* join it to make sure its dead */
		verbose(VERB_ALGO, "join %d", i);
		ub_thread_join(daemon->workers[i]->thr_id);
		verbose(VERB_ALGO, "join success %d", i);
	}
}

void 
daemon_fork(struct daemon* daemon)
{
	log_assert(daemon);
	/* first create all the worker structures, so we can pass
	 * them to the newly created threads. 
	 */
	daemon_create_workers(daemon);
	
	/* Now create the threads and init the workers.
	 * By the way, this is thread #0 (the main thread).
	 */
	daemon_start_others(daemon);

	/* Special handling for the main thread. This is the thread
	 * that handles signals.
	 */
	if(!worker_init(daemon->workers[0], daemon->cfg, daemon->ports,
		BUFSZ, 1))
		fatal_exit("Could not initialize main thread");
	signal_handling_playback(daemon->workers[0]);

	/* Start resolver service on main thread. */
	log_info("start of service (%s).", PACKAGE_STRING);
	worker_work(daemon->workers[0]);

	/* we exited! a signal happened! Stop other threads */
	daemon_stop_others(daemon);

	if(daemon->workers[0]->need_to_restart)
		daemon->need_to_exit = 0;
	else	daemon->need_to_exit = 1;
}

void 
daemon_cleanup(struct daemon* daemon)
{
	int i;
	log_assert(daemon);
	/* before stopping main worker, handle signals ourselves, so we
	   don't die on multiple reload signals for example. */
	signal_handling_record();
	log_thread_set(NULL);
	for(i=0; i<daemon->num; i++)
		worker_delete(daemon->workers[i]);
	free(daemon->workers);
	daemon->workers = NULL;
	daemon->num = 0;
	daemon->cfg = NULL;
}

void 
daemon_delete(struct daemon* daemon)
{
	if(!daemon)
		return;
	listening_ports_free(daemon->ports);
	lruhash_delete(daemon->msg_cache);
	alloc_clear(&daemon->superalloc);
	free(daemon->cwd);
	free(daemon->pidfile);
	free(daemon);
	checklock_stop();
}
