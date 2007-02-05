/*
 * util/netevent.c - event notification
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
 * This file contains event notification functions.
 */

#include "util/netevent.h"
#include "util/log.h"
#include <errno.h>

/* -------- Start of local definitions -------- */

/* we use libevent */
#include <event.h>

/**
 * The internal event structure for keeping libevent info for the event.
 * Possibly other structures (list, tree) this is part of.
 */
struct internal_event {
	/** libevent event type, alloced here */
	struct event ev;
};

/**
 * Internal base structure, so that every thread has its own events.
 */
struct internal_base {
	/** libevent event_base type. */
	struct event_base* base;
};

/**
 * Internal timer structure, to store timer event in.
 */
struct internal_timer {
	/** libevent event type, alloced here */
	struct event ev;
	/** is timer enabled, yes or no */
	uint8_t enabled;
};

/**
 * Internal signal structure, to store signal event in.
 */
struct internal_signal {
	/** libevent event type, alloced here */
	struct event ev;
	/** the commpoint it is part of */
	struct comm_signal* comm;
	/** next in signal list */
	struct internal_signal* next;
};

/**
 * handle libevent callback for udp comm point.
 * @param fd: file descriptor.
 * @param event: event bits from libevent: 
 *	EV_READ, EV_WRITE, EV_SIGNAL, EV_TIMEOUT.
 * @param arg: the comm_point structure.
 */
static void comm_point_udp_callback(int fd, short event, void* arg);

/**
 * handle libevent callback for tcp accept comm point
 * @param fd: file descriptor.
 * @param event: event bits from libevent: 
 *	EV_READ, EV_WRITE, EV_SIGNAL, EV_TIMEOUT.
 * @param arg: the comm_point structure.
 */
static void comm_point_tcp_accept_callback(int fd, short event, void* arg);

/**
 * handle libevent callback for tcp data comm point
 * @param fd: file descriptor.
 * @param event: event bits from libevent: 
 *	EV_READ, EV_WRITE, EV_SIGNAL, EV_TIMEOUT.
 * @param arg: the comm_point structure.
 */
static void comm_point_tcp_handle_callback(int fd, short event, void* arg);

/**
 * handle libevent callback for timer comm.
 * @param fd: file descriptor (always -1).
 * @param event: event bits from libevent: 
 *	EV_READ, EV_WRITE, EV_SIGNAL, EV_TIMEOUT.
 * @param arg: the comm_timer structure.
 */
static void comm_timer_callback(int fd, short event, void* arg);

/**
 * handle libevent callback for signal comm.
 * @param fd: file descriptor (used signal number).
 * @param event: event bits from libevent: 
 *	EV_READ, EV_WRITE, EV_SIGNAL, EV_TIMEOUT.
 * @param arg: the internal commsignal structure.
 */
static void comm_signal_callback(int fd, short event, void* arg);

/** create a tcp handler with a parent */
static struct comm_point* comm_point_create_tcp_handler(
	struct comm_base *base, struct comm_point* parent, size_t bufsize,
        comm_point_callback_t* callback, void* callback_arg);

/* -------- End of local definitions -------- */

struct comm_base* 
comm_base_create()
{
	struct comm_base* b = (struct comm_base*)calloc(1,
		sizeof(struct comm_base));
	if(!b)
		return NULL;
	b->eb = (struct internal_base*)calloc(1, sizeof(struct internal_base));
	if(!b->eb) {
		free(b);
		return NULL;
	}
	b->eb->base = event_init();
	if(!b->eb->base) {
		free(b->eb);
		free(b);
		return NULL;
	}
	verbose(VERB_ALGO, "libevent uses %s method.", event_get_method());
	return b;
}

void 
comm_base_delete(struct comm_base* b)
{
#ifdef HAVE_EVENT_BASE_FREE
	/* only libevent 1.2+ has it */
	event_base_free(b->eb->base);
#endif /* HAVE_EVENT_BASE_FREE */
	b->eb->base = NULL;
	free(b->eb);
	free(b);
}

void 
comm_base_dispatch(struct comm_base* b)
{
	int retval;
	retval = event_base_dispatch(b->eb->base);
	if(retval != 0) {
		fatal_exit("event_dispatch returned error %d, "
			"errno is %s", retval, strerror(errno));
	}
}

void comm_base_exit(struct comm_base* b)
{
	if(event_base_loopexit(b->eb->base, NULL) != 0) {
		log_err("Could not loopexit");
	}
}

/** send a UDP reply */
int
comm_point_send_udp_msg(struct comm_point *c, ldns_buffer* packet,
	struct sockaddr* addr, socklen_t addrlen) 
{
	ssize_t sent;
	sent = sendto(c->fd, ldns_buffer_begin(packet), 
		ldns_buffer_remaining(packet), 0,
		addr, addrlen);
	if(sent == -1) {
		log_err("sendto failed: %s", strerror(errno));
		return 0;
	} else if((size_t)sent != ldns_buffer_remaining(packet)) {
		log_err("sent %d in place of %d bytes", 
			sent, (int)ldns_buffer_remaining(packet));
		return 0;
	}
	return 1;
}

static void 
comm_point_udp_callback(int fd, short event, void* arg)
{
	struct comm_reply rep;
	ssize_t recv;

	rep.c = (struct comm_point*)arg;

	verbose(VERB_ALGO, "callback udp");
	if(!(event&EV_READ))
		return;
	log_assert(rep.c && rep.c->buffer && rep.c->fd == fd);
	ldns_buffer_clear(rep.c->buffer);
	recv = recvfrom(fd, ldns_buffer_begin(rep.c->buffer), 
		ldns_buffer_remaining(rep.c->buffer), 0, 
		(struct sockaddr*)&rep.addr, &rep.addrlen);
	if(recv == -1) {
		if(errno != EAGAIN && errno != EINTR) {
			log_err("recvfrom failed: %s", strerror(errno));
		}
		return;
	}
	ldns_buffer_skip(rep.c->buffer, recv);
	ldns_buffer_flip(rep.c->buffer);
	if((*rep.c->callback)(rep.c, rep.c->cb_arg, NETEVENT_NOERROR, &rep)) {
		/* send back immediate reply */
		(void)comm_point_send_udp_msg(rep.c, rep.c->buffer,
			(struct sockaddr*)&rep.addr, rep.addrlen);
	}
}

static void 
comm_point_tcp_accept_callback(int ATTR_UNUSED(fd), short ATTR_UNUSED(event), 
	void* arg)
{
	struct comm_point* c = (struct comm_point*)arg;
	log_info("callback tcpaccept for %x", (int)c);
}

static void 
comm_point_tcp_handle_callback(int ATTR_UNUSED(fd), short ATTR_UNUSED(event), 
	void* arg)
{
	struct comm_point* c = (struct comm_point*)arg;
	log_info("callback tcpaccept for %x", (int)c);
}

struct comm_point* 
comm_point_create_udp(struct comm_base *base, int fd, ldns_buffer* buffer,
	comm_point_callback_t* callback, void* callback_arg)
{
	struct comm_point* c = (struct comm_point*)calloc(1,
		sizeof(struct comm_point));
	short evbits;
	if(!c)
		return NULL;
	c->ev = (struct internal_event*)calloc(1,
		sizeof(struct internal_event));
	if(!c->ev) {
		free(c);
		return NULL;
	}
	c->fd = fd;
	c->buffer = buffer;
	c->timeout = NULL;
	c->tcp_is_reading = 0;
	c->tcp_byte_count = 0;
	c->tcp_parent = NULL;
	c->cur_tcp_count = 0;
	c->max_tcp_count = 0;
	c->tcp_handlers = NULL;
	c->tcp_free = NULL;
	c->type = comm_udp;
	c->tcp_do_close = 0;
	c->tcp_do_toggle_rw = 0;
	c->callback = callback;
	c->cb_arg = callback_arg;
	evbits = EV_READ | EV_PERSIST;
	/* libevent stuff */
	event_set(&c->ev->ev, c->fd, evbits, comm_point_udp_callback, c);
	if(event_base_set(base->eb->base, &c->ev->ev) != 0 ||
		event_add(&c->ev->ev, c->timeout) != 0 ) {
		log_err("could not add udp event");
		comm_point_delete(c);
		return NULL;
	}
	return c;
}

static struct comm_point* 
comm_point_create_tcp_handler(struct comm_base *base, 
	struct comm_point* parent, size_t bufsize,
        comm_point_callback_t* callback, void* callback_arg)
{
	struct comm_point* c = (struct comm_point*)calloc(1,
		sizeof(struct comm_point));
	short evbits;
	if(!c)
		return NULL;
	c->ev = (struct internal_event*)calloc(1,
		sizeof(struct internal_event));
	if(!c->ev) {
		free(c);
		return NULL;
	}
	c->fd = -1;
	c->buffer = ldns_buffer_new(bufsize);
	c->timeout = NULL;
	c->tcp_is_reading = 0;
	c->tcp_byte_count = 0;
	c->tcp_parent = parent;
	c->cur_tcp_count = 0;
	c->max_tcp_count = 0;
	c->tcp_handlers = NULL;
	c->tcp_free = NULL;
	c->type = comm_tcp;
	c->tcp_do_close = 0;
	c->tcp_do_toggle_rw = 0;
	c->callback = callback;
	c->cb_arg = callback_arg;
	/* add to parent free list */
	c->tcp_free = parent->tcp_free;
	parent->tcp_free = c;
	/* libevent stuff */
	evbits = EV_PERSIST | EV_READ;
	event_set(&c->ev->ev, c->fd, evbits, comm_point_tcp_handle_callback, c);
	if(event_base_set(base->eb->base, &c->ev->ev) != 0)
	{
		log_err("could not basetset tcphdl event");
		parent->tcp_free = c->tcp_free;
		free(c->ev);
		free(c);
		return NULL;
	}
	return c;
}

struct comm_point* 
comm_point_create_tcp(struct comm_base *base, int fd, int num, size_t bufsize,
        comm_point_callback_t* callback, void* callback_arg)
{
	struct comm_point* c = (struct comm_point*)calloc(1,
		sizeof(struct comm_point));
	short evbits;
	int i;
	/* first allocate the TCP accept listener */
	if(!c)
		return NULL;
	c->ev = (struct internal_event*)calloc(1,
		sizeof(struct internal_event));
	if(!c->ev) {
		free(c);
		return NULL;
	}
	c->fd = fd;
	c->buffer = NULL;
	c->timeout = NULL;
	c->tcp_is_reading = 0;
	c->tcp_byte_count = 0;
	c->tcp_parent = NULL;
	c->cur_tcp_count = 0;
	c->max_tcp_count = num;
	c->tcp_handlers = (struct comm_point**)calloc((size_t)num,
		sizeof(struct comm_point*));
	c->tcp_free = NULL;
	c->type = comm_tcp_accept;
	c->tcp_do_close = 0;
	c->tcp_do_toggle_rw = 0;
	c->callback = NULL;
	c->cb_arg = NULL;
	evbits = EV_READ | EV_PERSIST;
	/* libevent stuff */
	event_set(&c->ev->ev, c->fd, evbits, comm_point_tcp_accept_callback, c);
	if( event_base_set(base->eb->base, &c->ev->ev) != 0 ||
		event_add(&c->ev->ev, c->timeout) != 0 )
	{
		log_err("could not add tcpacc event");
		if(!event_del(&c->ev->ev)) {
			log_err("could not event_del tcpacc event");
		}
		free(c->tcp_handlers);
		free(c->ev);
		free(c);
		return NULL;
	}

	/* now prealloc the tcp handlers */
	for(i=0; i<num; i++) {
		c->tcp_handlers[i] = comm_point_create_tcp_handler(base,
			c, bufsize, callback, callback_arg);
		if(!c->tcp_handlers[i]) {
			comm_point_delete(c);
			return NULL;
		}
	}
	
	return c;
}

void 
comm_point_close(struct comm_point* c)
{
	if(event_del(&c->ev->ev) != 0) {
		log_err("could not event_del on close");
	}
	/* close fd after removing from event lists, or epoll/etc messes up */
	if(c->fd != -1)
		close(c->fd);
	c->fd = -1;
}

void 
comm_point_delete(struct comm_point* c)
{
	if(!c) 
		return;
	comm_point_close(c);
	if(c->tcp_handlers) {
		int i;
		for(i=0; i<c->max_tcp_count; i++)
			comm_point_delete(c->tcp_handlers[i]);
		free(c->tcp_handlers);
	}
	if(c->type == comm_tcp)
		ldns_buffer_free(c->buffer);
	free(c->ev);
	free(c);
}

void 
comm_point_set_cb_arg(struct comm_point* c, void *arg)
{
	log_assert(c);
	c->cb_arg = arg;
}

void comm_point_send_reply(struct comm_reply *repinfo)
{
	log_assert(repinfo && repinfo->c);
	if(repinfo->c->type == comm_udp) {
		comm_point_send_udp_msg(repinfo->c, repinfo->c->buffer,
			(struct sockaddr*)&repinfo->addr, repinfo->addrlen);
	} else {
		log_info("tcp reply");
	}
}

struct comm_timer* comm_timer_create(struct comm_base* base,
        void (*cb)(void*), void* cb_arg)
{
	struct comm_timer *tm = (struct comm_timer*)calloc(1,
		sizeof(struct comm_timer));
	if(!tm)
		return NULL;
	tm->ev_timer = (struct internal_timer*)calloc(1,
		sizeof(struct internal_timer));
	if(!tm->ev_timer) {
		log_err("malloc failed");
		free(tm);
		return NULL;
	}
	tm->callback = cb;
	tm->cb_arg = cb_arg;
	/*evtimer_set(&tm->ev_timer->ev, comm_timer_callback, tm);*/
	event_set(&tm->ev_timer->ev, -1, EV_PERSIST|EV_TIMEOUT, 
		comm_timer_callback, tm);
	if(event_base_set(base->eb->base, &tm->ev_timer->ev) != 0) {
		log_err("timer_create: event_base_set failed.");
		free(tm->ev_timer);
		free(tm);
		return NULL;
	}
	return tm;
}

void comm_timer_disable(struct comm_timer* timer)
{
	if(!timer)
		return;
	evtimer_del(&timer->ev_timer->ev);
	timer->ev_timer->enabled = 0;
}

void comm_timer_set(struct comm_timer* timer, struct timeval* tv)
{
	if(timer->ev_timer->enabled)
		comm_timer_disable(timer);
	evtimer_add(&timer->ev_timer->ev, tv);
	timer->ev_timer->enabled = 1;
}

void comm_timer_delete(struct comm_timer* timer)
{
	if(!timer)
		return;
	comm_timer_disable(timer);
	free(timer->ev_timer);
	free(timer);
}

static void 
comm_timer_callback(int ATTR_UNUSED(fd), short event, void* arg)
{
	struct comm_timer* tm = (struct comm_timer*)arg;
	if(!(event&EV_TIMEOUT))
		return;
	tm->ev_timer->enabled = 0;
	(*tm->callback)(tm->cb_arg);
}

int 
comm_timer_is_set(struct comm_timer* timer)
{
	return (int)timer->ev_timer->enabled;
}

struct comm_signal* comm_signal_create(struct comm_base* base,
        void (*callback)(int, void*), void* cb_arg)
{
	struct comm_signal* com = (struct comm_signal*)malloc(
		sizeof(struct comm_signal));
	if(!com) {
		log_err("malloc failed");
		return NULL;
	}
	com->base = base;
	com->callback = callback;
	com->cb_arg = cb_arg;
	com->ev_signal = NULL;
	return com;
}

static void comm_signal_callback(int sig, short event, void* arg)
{
	struct internal_signal* entry = (struct internal_signal*)arg;
	if(!(event & EV_SIGNAL))
		return;
	(*entry->comm->callback)(sig, entry->comm->cb_arg);
}

int comm_signal_bind(struct comm_signal* comsig, int sig)
{
	struct internal_signal* entry = (struct internal_signal*)calloc(1, 
		sizeof(struct internal_signal));
	if(!entry) {
		log_err("malloc failed");
		return 0;
	}
	log_assert(comsig);
	entry->comm = comsig;
	/* add signal event */
	signal_set(&entry->ev, sig, comm_signal_callback, entry);
	if(event_base_set(comsig->base->eb->base, &entry->ev) != 0) {
		log_err("Could not set signal base");
		free(entry);
		return 0;
	}
	if(signal_add(&entry->ev, NULL) != 0) {
		log_err("Could not add signal handler");
		free(entry);
		return 0;
	}
	/* link into list */
	entry->next = comsig->ev_signal;
	comsig->ev_signal = entry;
	return 1;
}

void comm_signal_delete(struct comm_signal* comsig)
{
	struct internal_signal* p, *np;
	if(!comsig)
		return;
	p=comsig->ev_signal;
	while(p) {
		np = p->next;
		signal_del(&p->ev);
		free(p);
		p = np;
	}
	free(comsig);
}
