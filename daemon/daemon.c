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

#include "config.h"
#include "daemon/daemon.h"
#include "daemon/worker.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/data/msgreply.h"
#include "util/storage/slabhash.h"
#include "services/listen_dnsport.h"
#include "services/cache/rrset.h"
#include "services/cache/infra.h"
#include "util/module.h"
#include "iterator/iterator.h"
#include "validator/validator.h"
#include <signal.h>

/** How many quit requests happened. */
static int sig_record_quit = 0;
/** How many reload requests happened. */
static int sig_record_reload = 0;

/** used when no other sighandling happens, so we don't die
  * when multiple signals in quick succession are sent to us. 
  * @param sig: signal number.
  * @return signal handler return type (void or int).
  */
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
	ERR_load_crypto_strings();
	daemon->need_to_exit = 0;
	daemon->num_modules = 0;
	if(!(daemon->env = (struct module_env*)calloc(1, 
		sizeof(*daemon->env)))) {
		free(daemon);
		return NULL;
	}
	alloc_init(&daemon->superalloc, NULL, 0);
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

/** count number of modules (words) in the string */
static int
count_modules(const char* s)
{
	int num = 0;
	if(!s)
		return 0;
	while(*s) {
		/* skip whitespace */
		while(*s && isspace(*s))
			s++;
		if(*s && !isspace(*s)) {
			/* skip identifier */
			num++;
			while(*s && !isspace(*s))
				s++;
		}
	}
	return num;
}

/**
 * Get funcblock for module name
 * @param str: string with module name. Advanced to next value on success.
 * @return funcblock or NULL on error.
 */
static struct module_func_block*
daemon_module_factory(const char** str)
{
	/* these are the modules available */
	int num = 2;
	const char* names[] = {"iterator", "validator", NULL};
	struct module_func_block* (*fb[])(void) = 
		{&iter_get_funcblock, &val_get_funcblock, NULL};

	int i;
	const char* s = *str;
	while(*s && isspace(*s))
		s++;
	for(i=0; i<num; i++) {
		if(strncmp(names[i], s, strlen(names[i])) == 0) {
			s += strlen(names[i]);
			*str = s;
			return (*fb[i])();
		}
	}
	return NULL;
}

/**
 * Read config file module settings and set up the modfunc block
 * @param daemon: the daemon.
 * @return false on error
 */
static int
daemon_config_modules(struct daemon* daemon)
{
	const char* str = daemon->cfg->module_conf;
	int i;
	verbose(VERB_DETAIL, "module config: \"%s\"", str);
	daemon->num_modules = count_modules(str);
	if(daemon->num_modules == 0) {
		log_err("error: no modules specified");
		return 0;
	}
	if(daemon->num_modules > MAX_MODULE) {
		log_err("error: too many modules (%d max %d)",
			daemon->num_modules, MAX_MODULE);
		return 0;
	}
	daemon->modfunc = (struct module_func_block**)calloc((size_t)
		daemon->num_modules, sizeof(struct module_func_block*));
	if(!daemon->modfunc) {
		log_err("out of memory");
		return 0;
	}
	for(i=0; i<daemon->num_modules; i++) {
		daemon->modfunc[i] = daemon_module_factory(&str);
		if(!daemon->modfunc[i]) {
			log_err("Unknown value for first module in: '%s'",
				str);
			return 0;
		}
	}
	return 1;
}

/**
 * Desetup the modules, deinit, delete.
 * @param daemon: the daemon.
 */
static void
daemon_desetup_modules(struct daemon* daemon)
{
	int i;
	for(i=0; i<daemon->num_modules; i++) {
		(*daemon->modfunc[i]->deinit)(daemon->env, i);
	}
	daemon->num_modules = 0;
	free(daemon->modfunc);
	daemon->modfunc = 0;
}

/**
 * Setup modules. Assigns ids and calls module_init.
 * @param daemon: the daemon
 */
static void daemon_setup_modules(struct daemon* daemon)
{
	int i;
	if(daemon->num_modules != 0)
		daemon_desetup_modules(daemon);
	/* fixed setup of the modules */
	if(!daemon_config_modules(daemon)) {
		fatal_exit("failed to setup modules");
	}
	daemon->env->cfg = daemon->cfg;
	daemon->env->alloc = &daemon->superalloc;
	daemon->env->worker = NULL;
	daemon->env->send_packet = &worker_send_packet;
	daemon->env->send_query = &worker_send_query;
	daemon->env->need_to_validate = 0; /* set by module init below */
	for(i=0; i<daemon->num_modules; i++) {
		log_info("init module %d: %s", i, daemon->modfunc[i]->name);
		if(!(*daemon->modfunc[i]->init)(daemon->env, i)) {
			fatal_exit("module init for module %s failed",
				daemon->modfunc[i]->name);
		}
	}
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
			fatal_exit("could not create worker");
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
	log_thread_set(&worker->thread_num);
	ub_thread_blocksigs();
#if !defined(HAVE_PTHREAD) && !defined(HAVE_SOLARIS_THREADS)
	/* close pipe ends used by main */
	close(worker->cmd_send_fd);
	worker->cmd_send_fd = -1;
	close_other_pipes(worker->daemon, worker->thread_num);
#endif
	if(!worker_init(worker, worker->daemon->cfg, worker->daemon->ports, 0))
		fatal_exit("Could not initialize thread");

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

	/* setup modules */
	daemon_setup_modules(daemon);

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
	if(!worker_init(daemon->workers[0], daemon->cfg, daemon->ports, 1))
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
	daemon_desetup_modules(daemon);
	listening_ports_free(daemon->ports);
	if(daemon->env) {
		slabhash_delete(daemon->env->msg_cache);
		rrset_cache_delete(daemon->env->rrset_cache);
		infra_delete(daemon->env->infra_cache);
	}
	alloc_clear(&daemon->superalloc);
	free(daemon->cwd);
	free(daemon->pidfile);
	free(daemon->env);
	free(daemon);
	/* libcrypto cleanup */
	/* CONF_modules_unload(1); */
	EVP_cleanup();
	/* ENGINE_cleanup(); */
	CRYPTO_cleanup_all_ex_data(); /* safe, no more threads right now */
	ERR_remove_state(0);
	ERR_free_strings();
	checklock_stop();
}
