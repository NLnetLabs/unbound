/*
 * util/netevent.c - event notification
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
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
	return b;
}

void 
comm_base_delete(struct comm_base* b)
{
	/* No way to delete event_base! leaks. */
	b->eb->base = NULL;
	free(b->eb);
	free(b);
}

void 
comm_base_dispatch(struct comm_base* b)
{
	int retval;
	while(1) {
		retval = event_base_dispatch(b->eb->base);
		if(retval != 0) {
			log_err("event_dispatch returned error %d, "
				"errno is %s", retval, strerror(errno));
		}
	}
}

static void 
comm_point_udp_callback(int ATTR_UNUSED(fd), short ATTR_UNUSED(event), 
	void* arg)
{
	struct comm_point* c = (struct comm_point*)arg;
	log_info("callback udp for %x", (int)c);
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
	c->tcp_handlers = (struct comm_point**)calloc(num,
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
	if(c->fd != -1)
		close(c->fd);
	c->fd = -1;
	if(event_del(&c->ev->ev) != 0) {
		log_err("could not event_del on close");
	}
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
	free(c->ev);
	free(c);
}

