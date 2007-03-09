/**
 * testcode/checklocks.c - wrapper on locks that checks access.
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

#include "config.h"
#include <signal.h>
#include "util/locks.h"   /* include before checklocks.h */
#include "testcode/checklocks.h"

/**
 * \file
 * Locks that are checked.
 *
 * Ugly hack: uses the fact that workers start with an int thread_num, and
 * are passed to thread_create to make the thread numbers here the same as 
 * those used for logging which is nice.
 *
 * Todo: - check global ordering of instances of locks.
 *	 - refcount statistics.
 *	 - debug status print, of thread lock stacks, and current waiting.
 */
#ifdef USE_THREAD_DEBUG

/** if key has been created */
static int key_created = 0;
/** we hide the thread debug info with this key. */
static ub_thread_key_t thr_debug_key;
/** the list of threads, so all threads can be examined. NULL if unused. */
static struct thr_check* thread_infos[THRDEBUG_MAX_THREADS];

/** print pretty lock error and exit */
static void lock_error(struct checked_lock* lock, 
	const char* func, const char* file, int line, const char* err)
{
	log_err("lock error (description follows)");
	log_err("Created at %s %s:%d", lock->create_func, 
		lock->create_file, lock->create_line);
	log_err("Previously %s %s:%d", lock->holder_func, 
		lock->holder_file, lock->holder_line);
	log_err("At %s %s:%d", func, file, line);
	log_err("Error for %s lock: %s",
		(lock->type==check_lock_mutex)?"mutex": (
		(lock->type==check_lock_spinlock)?"spinlock": "rwlock"), err);
	fatal_exit("bailing out");
}

/** 
 * Obtain lock on debug lock structure. This could be a deadlock by the caller.
 * The debug code itself does not deadlock. Anyway, check with timeouts. 
 * @param lock: on what to acquire lock.
 * @param func: user level caller identification.
 * @param file: user level caller identification.
 * @param line: user level caller identification.
 */
static void
acquire_locklock(struct checked_lock* lock, 
	const char* func, const char* file, int line)
{
	struct timespec to;
	int err;
	int contend = 0;
	/* first try; inc contention counter if not immediately */
	if((err = pthread_mutex_trylock(&lock->lock))) {
		if(err==EBUSY)
			contend++;
		else fatal_exit("error in mutex_trylock: %s", strerror(err));
	}
	if(!err)
		return; /* immediate success */
	to.tv_sec = time(NULL) + CHECK_LOCK_TIMEOUT;
	to.tv_nsec = 0;
	err = pthread_mutex_timedlock(&lock->lock, &to);
	if(err) {
		log_err("in acquiring locklock: %s", strerror(err));
		lock_error(lock, func, file, line, "acquire locklock");
	}
	/* since we hold the lock, we can edit the contention_count */
	lock->contention_count += contend;
}

/** add protected region */
void 
lock_protect(struct checked_lock* lock, void* area, size_t size)
{
	struct protected_area* e = (struct protected_area*)malloc(
		sizeof(struct protected_area));
	if(!e)
		fatal_exit("lock_protect: out of memory");
	e->region = area;
	e->size = size;
	e->hold = malloc(size);
	if(!e->hold)
		fatal_exit("lock_protect: out of memory");
	memcpy(e->hold, e->region, e->size);

	acquire_locklock(lock, __func__, __FILE__, __LINE__);
	e->next = lock->prot;
	lock->prot = e;
	LOCKRET(pthread_mutex_unlock(&lock->lock));
}

/** 
 * Check protected memory region. Memory compare. Exit on error. 
 * @param lock: which lock to check.
 * @param func: location we are now (when failure is detected).
 * @param file: location we are now (when failure is detected).
 * @param line: location we are now (when failure is detected).
 */
static void 
prot_check(struct checked_lock* lock,
	const char* func, const char* file, int line)
{
	struct protected_area* p = lock->prot;
	while(p) {
		if(memcmp(p->hold, p->region, p->size) != 0) {
			lock_error(lock, func, file, line, 
				"protected area modified");
		}
		p = p->next;
	}
}

/** Copy protected memory region. */
static void 
prot_store(struct checked_lock* lock)
{
	struct protected_area* p = lock->prot;
	while(p) {
		memcpy(p->hold, p->region, p->size);
		p = p->next;
	}
}


/** alloc struct, init lock empty */
void 
checklock_init(enum check_lock_type type, struct checked_lock** lock,
        const char* func, const char* file, int line)
{
	struct checked_lock* e = (struct checked_lock*)calloc(1, 
		sizeof(struct checked_lock));
	if(!e)
		fatal_exit("%s %s %d: out of memory", func, file, line);
	*lock = e;
	e->type = type;
	e->create_func = func;
	e->create_file = file;
	e->create_line = line;
	LOCKRET(pthread_mutex_init(&e->lock, NULL));
	switch(e->type) {
		case check_lock_mutex:
			LOCKRET(pthread_mutex_init(&e->mutex, NULL));
			break;
		case check_lock_spinlock:
			LOCKRET(pthread_spin_init(&e->spinlock, PTHREAD_PROCESS_PRIVATE));
			break;
		case check_lock_rwlock:
			LOCKRET(pthread_rwlock_init(&e->rwlock, NULL));
			break;
		default:
			log_assert(0);
	}
}

/** delete prot items */
static void 
prot_clear(struct checked_lock* lock)
{
	struct protected_area* p=lock->prot, *np;
	while(p) {
		np = p->next;
		free(p->hold);
		free(p);
		p = np;
	}
}

/** check if type is OK for the lock given */
static void 
checktype(enum check_lock_type type, struct checked_lock* lock,
        const char* func, const char* file, int line)
{
	if(type != lock->type) {
		lock_error(lock, func, file, line, "wrong lock type");
	}
}

/** check if OK, free struct */
void 
checklock_destroy(enum check_lock_type type, struct checked_lock** lock,
        const char* func, const char* file, int line)
{
	const size_t contention_interest = 10;
	struct checked_lock* e;
	if(!lock) 
		return;
	e = *lock;
	if(!e)
		return;
	checktype(type, e, func, file, line);

	/* check if delete is OK */
	acquire_locklock(e, func, file, line);
	*lock = NULL; /* use after free will fail */
	if(e->hold_count != 0)
		lock_error(e, func, file, line, "delete while locked.");
	if(e->wait_count != 0)
		lock_error(e, func, file, line, "delete while waited on.");
	prot_check(e, func, file, line);
	LOCKRET(pthread_mutex_unlock(&e->lock));

	/* contention */
	if(e->contention_count > contention_interest) {
		log_info("lock created %s %s %d has contention %u",
			e->create_func, e->create_file, e->create_line,
			(unsigned int)e->contention_count);
	}

	/* delete it */
	LOCKRET(pthread_mutex_destroy(&e->lock));
	prot_clear(e);
	/* since nobody holds the lock - see check above, no need to unlink 
	 * from the thread-held locks list. */
	switch(e->type) {
		case check_lock_mutex:
			LOCKRET(pthread_mutex_destroy(&e->mutex));
			break;
		case check_lock_spinlock:
			LOCKRET(pthread_spin_destroy(&e->spinlock));
			break;
		case check_lock_rwlock:
			LOCKRET(pthread_rwlock_destroy(&e->rwlock));
			break;
		default:
			log_assert(0);
	}
	memset(e, 0, sizeof(struct checked_lock));
	free(e);
}

/** finish acquiring lock, shared between _(rd|wr||)lock() routines. */
static void 
finish_acquire_lock(struct thr_check* thr, struct checked_lock* lock,
        const char* func, const char* file, int line)
{
	thr->waiting = NULL;
	lock->wait_count --;
	lock->holder = thr;
	lock->hold_count ++;
	lock->holder_func = func;
	lock->holder_file = file;
	lock->holder_line = line;
	
	/* insert in thread lock list, as first */
	lock->prev_held_lock[thr->num] = NULL;
	lock->next_held_lock[thr->num] = thr->holding_first;
	if(thr->holding_first)
		/* no need to lock it, since this thread already holds the
		 * lock (since it is on this list) and we only edit thr->num
		 * member in array. So it is safe.  */
		thr->holding_first->prev_held_lock[thr->num] = lock;
	else	thr->holding_last = lock;
	thr->holding_first = lock;
}

/**
 * Locking routine.
 * @param type: as passed by user.
 * @param lock: as passed by user.
 * @param func: caller location.
 * @param file: caller location.
 * @param line: caller location.
 * @param tryfunc: the pthread_mutex_trylock or similar function.
 * @param timedfunc: the pthread_mutex_timedlock or similar function.
 *	Uses absolute timeout value.
 * @param arg: what to pass to tryfunc and timedlock.
 * @param exclusive: if lock must be exlusive (only one allowed).
 * @param getwr: if attempts to get writelock (or readlock) for rwlocks.
 */
static void 
checklock_lockit(enum check_lock_type type, struct checked_lock* lock,
        const char* func, const char* file, int line,
	int (*tryfunc)(void*), int (*timedfunc)(void*, struct timespec*),
	void* arg, int exclusive, int getwr)
{
	int err;
	int contend = 0;
	struct thr_check *thr = (struct thr_check*)pthread_getspecific(
		thr_debug_key);
	checktype(type, lock, func, file, line);
	if(!thr) lock_error(lock, func, file, line, "no thread info");
	
	acquire_locklock(lock, func, file, line);
	lock->wait_count ++;
	thr->waiting = lock;
	if(exclusive && lock->hold_count > 0 && lock->holder == thr) 
		lock_error(lock, func, file, line, "thread already owns lock");
	if(type==check_lock_rwlock && getwr && lock->writeholder == thr)
		lock_error(lock, func, file, line, "thread already has wrlock");
	LOCKRET(pthread_mutex_unlock(&lock->lock));

	/* first try; if busy increase contention counter */
	if((err=tryfunc(arg))) {
		struct timespec to;
		if(err != EBUSY) log_err("trylock: %s", strerror(err));
		to.tv_sec = time(NULL) + CHECK_LOCK_TIMEOUT;
		to.tv_nsec = 0;
		if((err=timedfunc(arg, &to))) {
			if(err == ETIMEDOUT)
				lock_error(lock, func, file, line, 
					"timeout possible deadlock");
			log_err("timedlock: %s", strerror(err));
		}
		contend ++;
	}
	/* got the lock */

	acquire_locklock(lock, func, file, line);
	lock->contention_count += contend;
	if(exclusive && lock->hold_count > 0)
		lock_error(lock, func, file, line, "got nonexclusive lock");
	if(type==check_lock_rwlock && getwr && lock->writeholder)
		lock_error(lock, func, file, line, "got nonexclusive wrlock");
	if(type==check_lock_rwlock && getwr)
		lock->writeholder = thr;
	/* check the memory areas for unauthorized changes,
	 * between last unlock time and current lock time.
	 * we check while holding the lock (threadsafe).
	 */
	if(getwr || exclusive)
		prot_check(lock, func, file, line);
	finish_acquire_lock(thr, lock, func, file, line);
	LOCKRET(pthread_mutex_unlock(&lock->lock));
}

/** helper for rdlock: try */
static int try_rd(void* arg)
{ return pthread_rwlock_tryrdlock((pthread_rwlock_t*)arg); }
/** helper for rdlock: timed */
static int timed_rd(void* arg, struct timespec* to)
{ return pthread_rwlock_timedrdlock((pthread_rwlock_t*)arg, to); }

/** check if OK, lock */
void 
checklock_rdlock(enum check_lock_type type, struct checked_lock* lock,
        const char* func, const char* file, int line)
{

	log_assert(type == check_lock_rwlock);
	checklock_lockit(type, lock, func, file, line,
		try_rd, timed_rd, &lock->rwlock, 0, 0);
}

/** helper for wrlock: try */
static int try_wr(void* arg)
{ return pthread_rwlock_trywrlock((pthread_rwlock_t*)arg); }
/** helper for wrlock: timed */
static int timed_wr(void* arg, struct timespec* to)
{ return pthread_rwlock_timedwrlock((pthread_rwlock_t*)arg, to); }

/** check if OK, lock */
void 
checklock_wrlock(enum check_lock_type type, struct checked_lock* lock,
        const char* func, const char* file, int line)
{
	log_assert(type == check_lock_rwlock);
	checklock_lockit(type, lock, func, file, line,
		try_wr, timed_wr, &lock->rwlock, 0, 1);
}

/** helper for lock mutex: try */
static int try_mutex(void* arg)
{ return pthread_mutex_trylock((pthread_mutex_t*)arg); }
/** helper for lock mutex: timed */
static int timed_mutex(void* arg, struct timespec* to)
{ return pthread_mutex_timedlock((pthread_mutex_t*)arg, to); }

/** helper for lock spinlock: try */
static int try_spinlock(void* arg)
{ return pthread_spin_trylock((pthread_spinlock_t*)arg); }
/** helper for lock spinlock: timed */
static int timed_spinlock(void* arg, struct timespec* to)
{
	int err;
	/* spin for 5 seconds. (ouch for the CPU, but it beats forever) */
	while( (err=try_spinlock(arg)) == EBUSY) {
#ifndef S_SPLINT_S
		if(time(NULL) >= to->tv_sec)
			return ETIMEDOUT;
#endif
	}
	return err;
}

/** check if OK, lock */
void 
checklock_lock(enum check_lock_type type, struct checked_lock* lock,
        const char* func, const char* file, int line)
{
	log_assert(type != check_lock_rwlock);
	switch(type) {
		case check_lock_mutex:
			checklock_lockit(type, lock, func, file, line,
				try_mutex, timed_mutex, &lock->mutex, 1, 0);
			break;
		case check_lock_spinlock:
			/* void* cast needed because 'volatile' on some OS */
			checklock_lockit(type, lock, func, file, line,
				try_spinlock, timed_spinlock, 
				(void*)&lock->spinlock, 1, 0);
			break;
		default:
			log_assert(0);
	}
}

/** check if OK, unlock */
void 
checklock_unlock(enum check_lock_type type, struct checked_lock* lock,
        const char* func, const char* file, int line)
{
	struct thr_check *thr = (struct thr_check*)pthread_getspecific(
		thr_debug_key);
	checktype(type, lock, func, file, line);
	if(!thr) lock_error(lock, func, file, line, "no thread info");

	acquire_locklock(lock, func, file, line);
	/* was this thread even holding this lock? */
	if(thr->holding_first != lock &&
		lock->prev_held_lock[thr->num] == NULL) {
		lock_error(lock, func, file, line, "unlock nonlocked lock");
	}
	if(lock->hold_count <= 0)
		lock_error(lock, func, file, line, "too many unlocks");

	/* store this point as last touched by */
	lock->holder = thr;
	lock->hold_count --;
	lock->holder_func = func;
	lock->holder_file = file;
	lock->holder_line = line;

	/* delete from thread holder list */
	/* no need to lock other lockstructs, because they are all on the
	 * held-locks list, and this thread holds their locks.
	 * we only touch the thr->num members, so it is safe.  */
	if(thr->holding_first == lock)
		thr->holding_first = lock->next_held_lock[thr->num];
	if(thr->holding_last == lock)
		thr->holding_last = lock->prev_held_lock[thr->num];
	if(lock->next_held_lock[thr->num])
		lock->next_held_lock[thr->num]->prev_held_lock[thr->num] =
			lock->prev_held_lock[thr->num];
	if(lock->prev_held_lock[thr->num])
		lock->prev_held_lock[thr->num]->next_held_lock[thr->num] =
			lock->next_held_lock[thr->num];
	lock->next_held_lock[thr->num] = NULL;
	lock->prev_held_lock[thr->num] = NULL;

	if(type==check_lock_rwlock && lock->writeholder == thr) {
		lock->writeholder = NULL;
		prot_store(lock);
	} else if(type != check_lock_rwlock) {
		/* store memory areas that are protected, for later checks */
		prot_store(lock);
	}
	LOCKRET(pthread_mutex_unlock(&lock->lock));

	/* unlock it */
	switch(type) {
		case check_lock_mutex:
			LOCKRET(pthread_mutex_unlock(&lock->mutex));
			break;
		case check_lock_spinlock:
			LOCKRET(pthread_spin_unlock(&lock->spinlock));
			break;
		case check_lock_rwlock:
			LOCKRET(pthread_rwlock_unlock(&lock->rwlock));
			break;
		default:
			log_assert(0);
	}
}

/** checklock thread main, Inits thread structure. */
static void* checklock_main(void* arg)
{
	struct thr_check* thr = (struct thr_check*)arg; 
	void* ret;
	thr->id = pthread_self();
	/* Hack to get same numbers as in log file */
	thr->num = *(int*)(thr->arg);
	log_assert(thread_infos[thr->num] == NULL);
	thread_infos[thr->num] = thr;
	LOCKRET(pthread_setspecific(thr_debug_key, thr));
	ret = thr->func(thr->arg);
	thread_infos[thr->num] = NULL;
	free(thr);
	return ret;
}

/** allocate debug info and create thread */
void 
checklock_thrcreate(pthread_t* id, void* (*func)(void*), void* arg)
{
	struct thr_check* thr = (struct thr_check*)calloc(1, 
		sizeof(struct thr_check));
	if(!thr)
		fatal_exit("thrcreate: out of memory");
	if(!key_created) {
		struct thr_check* thisthr = (struct thr_check*)calloc(1, 
			sizeof(struct thr_check));
		if(!thisthr)
			fatal_exit("thrcreate: out of memory");
		key_created = 1;
		LOCKRET(pthread_key_create(&thr_debug_key, NULL));
		LOCKRET(pthread_setspecific(thr_debug_key, thisthr));
		thread_infos[0] = thisthr;
	}
	thr->func = func;
	thr->arg = arg;
	LOCKRET(pthread_create(id, NULL, checklock_main, thr));
}

/** signal handler for join timeout, Exits. */
static RETSIGTYPE joinalarm(int ATTR_UNUSED(sig))
{
	fatal_exit("join thread timeout. hangup or deadlock.");
}

/** wait for thread with a timeout. */
void 
checklock_thrjoin(pthread_t thread)
{
	/* wait with a timeout */
	if(signal(SIGALRM, joinalarm) == SIG_ERR)
		fatal_exit("signal(): %s", strerror(errno));
	(void)alarm(CHECK_JOIN_TIMEOUT);
	LOCKRET(pthread_join(thread, NULL));
	(void)alarm(0);
}

#endif /* USE_THREAD_DEBUG */
