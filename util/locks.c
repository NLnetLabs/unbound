/**
 * util/locks.c - unbound locking primitives
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
 * Implementation of locking and threading support.
 * A place for locking debug code since most locking functions are macros.
 */

#include "config.h"
#include "util/locks.h"
#include <signal.h>
#include <sys/wait.h>

/** block all signals, masks them away. */
void 
ub_thread_blocksigs()
{
	int err;
	sigset_t sigset;
	sigfillset(&sigset);
#ifdef HAVE_PTHREAD
	if((err=pthread_sigmask(SIG_SETMASK, &sigset, NULL)))
		fatal_exit("pthread_sigmask: %s", strerror(err));
#else
#  ifdef HAVE_SOLARIS_THREADS
	if((err=thr_sigsetmask(SIG_SETMASK, &sigset, NULL)))
		fatal_exit("thr_sigsetmask: %s", strerror(err));
#  else 
	/* have nothing, do single process signal mask */
	if((err=sigprocmask(SIG_SETMASK, &sigset, NULL)))
		fatal_exit("sigprocmask: %s", strerror(errno));
#  endif /* HAVE_SOLARIS_THREADS */
#endif /* HAVE_PTHREAD */
}

/** unblock one signal, so we can catch it. */
void ub_thread_sig_unblock(int sig)
{
	int err;
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, sig);
#ifdef HAVE_PTHREAD
	if((err=pthread_sigmask(SIG_UNBLOCK, &sigset, NULL)))
		fatal_exit("pthread_sigmask: %s", strerror(err));
#else
#  ifdef HAVE_SOLARIS_THREADS
	if((err=thr_sigsetmask(SIG_UNBLOCK, &sigset, NULL)))
		fatal_exit("thr_sigsetmask: %s", strerror(err));
#  else 
	/* have nothing, do nothing */
	if((err=sigprocmask(SIG_UNBLOCK, &sigset, NULL)))
		fatal_exit("sigprocmask: %s", strerror(errno));
#  endif /* HAVE_SOLARIS_THREADS */
#endif /* HAVE_PTHREAD */
}

/**
 * No threading available: fork a new process.
 * This means no shared data structure, and no locking.
 * Only the main thread ever returns. Exits on errors.
 * @param thr: the location where to store the thread-id.
 * @param func: function body of the thread. Return value of func is lost.
 * @param arg: user argument to func.
 */
void 
ub_thr_fork_create(ub_thread_t* thr, void* (*func)(void*), void* arg)
{
	pid_t pid = fork();
	switch(pid) {
	default:	/* main */
			*thr = (ub_thread_t)pid;
			return;
	case 0: 	/* child */
			*thr = (ub_thread_t)getpid();
			(void)(*func)(arg);
			exit(0);
	case -1:	/* error */
			fatal_exit("could not fork: %s", strerror(errno));
	}
}

/**
 * There is no threading. Wait for a process to terminate.
 * Note that ub_thread_t is defined as pid_t.
 * @param thread: the process id to wait for.
 */
void ub_thr_fork_wait(ub_thread_t thread)
{
	int status = 0;
	if(waitpid((pid_t)thread, &status, 0) == -1)
		log_err("waitpid(%d): %s", (int)thread, strerror(errno));
	if(status != 0)
		log_warn("process %d abnormal exit with status %d",
			(int)thread, status);
}
