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

/* -------- Start of local definitions -------- */
/** The TCP reading or writing query timeout in seconds */
#define TCP_QUERY_TIMEOUT 120 

/* We define libevent structures here to hide the libevent stuff. */

#ifdef USE_MINI_EVENT
#include "util/mini_event.h"
#else
/* we use libevent */
#include <event.h>
#endif

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
	/** is timer enabled */
	uint8_t enabled;
};

/**
 * Internal signal structure, to store signal event in.
 */
struct internal_signal {
	/** libevent event type, alloced here */
	struct event ev;
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
 * @param fd: file descriptor (used for the signal number).
 * @param event: event bits from libevent: 
 *	EV_READ, EV_WRITE, EV_SIGNAL, EV_TIMEOUT.
 * @param arg: the internal commsignal structure.
 */
static void comm_signal_callback(int fd, short event, void* arg);

/** libevent callback for AF_UNIX fds. */
static void comm_point_local_handle_callback(int fd, short event, void* arg);

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
	verbose(VERB_ALGO, "libevent %s uses %s method.", 
		event_get_version(), event_get_method());
	return b;
}

void 
comm_base_delete(struct comm_base* b)
{
	if(!b)
		return;
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
			(int)sent, (int)ldns_buffer_remaining(packet));
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
	log_assert(rep.c->type == comm_udp);

	verbose(VERB_ALGO, "callback udp");
	if(!(event&EV_READ))
		return;
	log_assert(rep.c && rep.c->buffer && rep.c->fd == fd);
	ldns_buffer_clear(rep.c->buffer);
	rep.addrlen = (socklen_t)sizeof(rep.addr);
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

/** Use a new tcp handler for new query fd, set to read query. */
static void
setup_tcp_handler(struct comm_point* c, int fd) 
{
	log_assert(c->type == comm_tcp);
	log_assert(c->fd == -1);
	ldns_buffer_clear(c->buffer);
	c->tcp_is_reading = 1;
	c->tcp_byte_count = 0;
	comm_point_start_listening(c, fd, TCP_QUERY_TIMEOUT);
}

static void 
comm_point_tcp_accept_callback(int fd, short event, void* arg)
{
	struct comm_point* c = (struct comm_point*)arg, *c_hdl;
	struct comm_reply rep;
	int new_fd;
	log_assert(c->type == comm_tcp_accept);
	if(!(event & EV_READ)) {
		log_info("ignoring tcp accept event %d", (int)event);
		return;
	}
	/* accept incoming connection. */
	rep.c = NULL;
	rep.addrlen = (socklen_t)sizeof(rep.addr);
	new_fd = accept(fd, (struct sockaddr*)&rep.addr, &rep.addrlen);
	if(new_fd == -1) {
		/* EINTR is signal interrupt. others are closed connection. */
		if(	errno != EINTR 
			&& errno != EWOULDBLOCK 
			&& errno != ECONNABORTED 
#ifdef EPROTO
			&& errno != EPROTO
#endif /* EPROTO */
			)
			log_err("accept failed: %s", strerror(errno));
		return;
	}
	/* find free tcp handler. */
	if(!c->tcp_free) {
		log_err("accepted too many tcp, connections full");
		close(new_fd);
		return;
	}
	/* grab it */
	c_hdl = c->tcp_free;
	c->tcp_free = c_hdl->tcp_free;
	if(!c->tcp_free) {
		/* stop accepting incoming queries for now. */
		comm_point_stop_listening(c);
	}
	/* addr is dropped. Not needed for tcp reply. */
	setup_tcp_handler(c_hdl, new_fd);
}

/** Make tcp handler free for next assignment. */
static void
reclaim_tcp_handler(struct comm_point* c)
{
	log_assert(c->type == comm_tcp);
	comm_point_close(c);
	if(c->tcp_parent) {
		c->tcp_free = c->tcp_parent->tcp_free;
		c->tcp_parent->tcp_free = c;
		if(!c->tcp_free) {
			/* re-enable listening on accept socket */
			comm_point_start_listening(c->tcp_parent, -1, -1);
		}
	}
}

/** do the callback when writing is done */
static void
tcp_callback_writer(struct comm_point* c)
{
	log_assert(c->type == comm_tcp);
	ldns_buffer_clear(c->buffer);
	if(c->tcp_do_toggle_rw)
		c->tcp_is_reading = 1;
	c->tcp_byte_count = 0;
	comm_point_stop_listening(c);
	if(c->tcp_parent) /* for listening socket */
		reclaim_tcp_handler(c);
	else	/* its outgoing socket, start listening for reading */
		comm_point_start_listening(c, -1, -1);
}

/** do the callback when reading is done */
static void
tcp_callback_reader(struct comm_point* c)
{
	struct comm_reply rep;
	log_assert(c->type == comm_tcp || c->type == comm_local);
	ldns_buffer_flip(c->buffer);
	if(c->tcp_do_toggle_rw)
		c->tcp_is_reading = 0;
	c->tcp_byte_count = 0;
	if(c->type == comm_tcp)
		comm_point_stop_listening(c);
	rep.c = c;
	rep.addrlen = 0;
	if( (*c->callback)(c, c->cb_arg, NETEVENT_NOERROR, &rep) ) {
		comm_point_start_listening(c, -1, TCP_QUERY_TIMEOUT);
	}
}

/** Handle tcp reading callback. 
 * @param fd: file descriptor of socket.
 * @param c: comm point to read from into buffer.
 * @param short_ok: if true, very short packets are OK (for comm_local).
 * @return: 0 on error 
 */
static int
comm_point_tcp_handle_read(int fd, struct comm_point* c, int short_ok)
{
	ssize_t r;
	log_assert(c->type == comm_tcp || c->type == comm_local);
	if(!c->tcp_is_reading)
		return 0;

	if(c->tcp_byte_count < sizeof(uint16_t)) {
		/* read length bytes */
		r = read(fd, ldns_buffer_at(c->buffer, c->tcp_byte_count), 
			sizeof(uint16_t)-c->tcp_byte_count);
		if(r == 0)
			return 0;
		else if(r == -1) {
			if(errno == EINTR || errno == EAGAIN)
				return 1;
			log_err("read (in tcp s): %s", strerror(errno));
			return 0;
		} 
		c->tcp_byte_count += r;
		if(c->tcp_byte_count != sizeof(uint16_t))
			return 1;
		if(ldns_buffer_read_u16_at(c->buffer, 0) >
			ldns_buffer_capacity(c->buffer)) {
			verbose(VERB_DETAIL, "tcp: dropped larger than buffer");
			return 0;
		}
		ldns_buffer_set_limit(c->buffer, 
			ldns_buffer_read_u16_at(c->buffer, 0));
		if(!short_ok && 
			ldns_buffer_limit(c->buffer) < LDNS_HEADER_SIZE) {
			verbose(VERB_DETAIL, "tcp: dropped bogus too short.");
			return 0;
		}
		verbose(VERB_ALGO, "Reading tcp query of length %d", 
			(int)ldns_buffer_limit(c->buffer));
	}

	r = read(fd, ldns_buffer_current(c->buffer), 
		ldns_buffer_remaining(c->buffer));
	if(r == 0) {
		return 0;
	} else if(r == -1) {
		if(errno == EINTR || errno == EAGAIN)
			return 1;
		log_err("read (in tcp r): %s", strerror(errno));
		return 0;
	}
	ldns_buffer_skip(c->buffer, r);
	if(ldns_buffer_remaining(c->buffer) <= 0) {
		tcp_callback_reader(c);
	}
	return 1;
}

/** 
 * Handle tcp writing callback. 
 * @param fd: file descriptor of socket.
 * @param c: comm point to write buffer out of.
 * @return: 0 on error
 */
static int
comm_point_tcp_handle_write(int fd, struct comm_point* c)
{
	ssize_t r;
	log_assert(c->type == comm_tcp);
	if(c->tcp_is_reading)
		return 0;
	if(c->tcp_byte_count == 0 && c->tcp_check_nb_connect) {
		/* check for pending error from nonblocking connect */
		/* from Stevens, unix network programming, vol1, 3rd ed, p450*/
		int error = 0;
		socklen_t len = (socklen_t)sizeof(error);
		if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0){
			error = errno; /* on solaris errno is error */
		}
		if(error == EINPROGRESS || error == EWOULDBLOCK)
			return 1; /* try again later */
		if(error != 0) {
			log_err("tcp connect: %s", strerror(error));
			return 0;
		}
	}

	if(c->tcp_byte_count < sizeof(uint16_t)) {
		uint16_t len = htons(ldns_buffer_limit(c->buffer));
		struct iovec iov[2];
		iov[0].iov_base = (uint8_t*)&len + c->tcp_byte_count;
		iov[0].iov_len = sizeof(uint16_t) - c->tcp_byte_count;
		iov[1].iov_base = ldns_buffer_begin(c->buffer);
		iov[1].iov_len = ldns_buffer_limit(c->buffer);
		r = writev(fd, iov, 2);
		if(r == -1) {
			if(errno == EINTR || errno == EAGAIN)
				return 1;
			log_err("tcp writev: %s", strerror(errno));
			return 0;
		}
		c->tcp_byte_count += r;
		if(c->tcp_byte_count < sizeof(uint16_t))
			return 1;
		ldns_buffer_set_position(c->buffer, c->tcp_byte_count - 
			sizeof(uint16_t));
		if(ldns_buffer_remaining(c->buffer) == 0) {
			tcp_callback_writer(c);
		}
		return 1;
	}
	r = write(fd, ldns_buffer_current(c->buffer), 
		ldns_buffer_remaining(c->buffer));
	if(r == -1) {
		if(errno == EINTR || errno == EAGAIN)
			return 1;
		log_err("tcp write: %s", strerror(errno));
		return 0;
	}
	ldns_buffer_skip(c->buffer, r);

	if(ldns_buffer_remaining(c->buffer) == 0) {
		tcp_callback_writer(c);
	}
	
	return 1;
}

static void 
comm_point_tcp_handle_callback(int fd, short event, void* arg)
{
	struct comm_point* c = (struct comm_point*)arg;
	log_assert(c->type == comm_tcp);

	if(event&EV_READ) {
		if(!comm_point_tcp_handle_read(fd, c, 0)) {
			reclaim_tcp_handler(c);
			if(!c->tcp_do_close)
				(void)(*c->callback)(c, c->cb_arg, 
					NETEVENT_CLOSED, NULL);
		}
		return;
	}
	if(event&EV_WRITE) {
		if(!comm_point_tcp_handle_write(fd, c)) {
			reclaim_tcp_handler(c);
			if(!c->tcp_do_close)
				(void)(*c->callback)(c, c->cb_arg, 
					NETEVENT_CLOSED, NULL);
		}
		return;
	}
	if(event&EV_TIMEOUT) {
		verbose(VERB_DETAIL, "tcp took too long, dropped");
		reclaim_tcp_handler(c);
		if(!c->tcp_do_close)
			(void)(*c->callback)(c, c->cb_arg,
				NETEVENT_TIMEOUT, NULL);
		return;
	}
	log_err("Ignored event %d for tcphdl.", event);
}

static void comm_point_local_handle_callback(int fd, short event, void* arg)
{
	struct comm_point* c = (struct comm_point*)arg;
	log_assert(c->type == comm_local);

	if(event&EV_READ) {
		if(!comm_point_tcp_handle_read(fd, c, 1)) {
			(void)(*c->callback)(c, c->cb_arg, NETEVENT_CLOSED, 
				NULL);
		}
		return;
	}
	log_err("Ignored event %d for localhdl.", event);
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
	c->max_tcp_count = 0;
	c->tcp_handlers = NULL;
	c->tcp_free = NULL;
	c->type = comm_udp;
	c->tcp_do_close = 0;
	c->do_not_close = 0;
	c->tcp_do_toggle_rw = 0;
	c->tcp_check_nb_connect = 0;
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
	if(!c->buffer) {
		free(c->ev);
		free(c);
		return NULL;
	}
	c->timeout = (struct timeval*)malloc(sizeof(struct timeval));
	if(!c->timeout) {
		ldns_buffer_free(c->buffer);
		free(c->ev);
		free(c);
		return NULL;
	}
	c->tcp_is_reading = 0;
	c->tcp_byte_count = 0;
	c->tcp_parent = parent;
	c->max_tcp_count = 0;
	c->tcp_handlers = NULL;
	c->tcp_free = NULL;
	c->type = comm_tcp;
	c->tcp_do_close = 0;
	c->do_not_close = 0;
	c->tcp_do_toggle_rw = 1;
	c->tcp_check_nb_connect = 0;
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
	c->max_tcp_count = num;
	c->tcp_handlers = (struct comm_point**)calloc((size_t)num,
		sizeof(struct comm_point*));
	if(!c->tcp_handlers) {
		free(c->ev);
		free(c);
		return NULL;
	}
	c->tcp_free = NULL;
	c->type = comm_tcp_accept;
	c->tcp_do_close = 0;
	c->do_not_close = 0;
	c->tcp_do_toggle_rw = 0;
	c->tcp_check_nb_connect = 0;
	c->callback = NULL;
	c->cb_arg = NULL;
	evbits = EV_READ | EV_PERSIST;
	/* libevent stuff */
	event_set(&c->ev->ev, c->fd, evbits, comm_point_tcp_accept_callback, c);
	if(event_base_set(base->eb->base, &c->ev->ev) != 0 ||
		event_add(&c->ev->ev, c->timeout) != 0 )
	{
		log_err("could not add tcpacc event");
		comm_point_delete(c);
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

struct comm_point* 
comm_point_create_tcp_out(struct comm_base *base, size_t bufsize,
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
	if(!c->buffer) {
		free(c->ev);
		free(c);
		return NULL;
	}
	c->timeout = NULL;
	c->tcp_is_reading = 0;
	c->tcp_byte_count = 0;
	c->tcp_parent = NULL;
	c->max_tcp_count = 0;
	c->tcp_handlers = NULL;
	c->tcp_free = NULL;
	c->type = comm_tcp;
	c->tcp_do_close = 0;
	c->do_not_close = 0;
	c->tcp_do_toggle_rw = 1;
	c->tcp_check_nb_connect = 1;
	c->callback = callback;
	c->cb_arg = callback_arg;
	evbits = EV_PERSIST | EV_WRITE;
	event_set(&c->ev->ev, c->fd, evbits, comm_point_tcp_handle_callback, c);
	if(event_base_set(base->eb->base, &c->ev->ev) != 0)
	{
		log_err("could not basetset tcpout event");
		ldns_buffer_free(c->buffer);
		free(c->ev);
		free(c);
		return NULL;
	}

	return c;
}

struct comm_point* 
comm_point_create_local(struct comm_base *base, int fd, size_t bufsize,
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
	c->buffer = ldns_buffer_new(bufsize);
	if(!c->buffer) {
		free(c->ev);
		free(c);
		return NULL;
	}
	c->timeout = NULL;
	c->tcp_is_reading = 1;
	c->tcp_byte_count = 0;
	c->tcp_parent = NULL;
	c->max_tcp_count = 0;
	c->tcp_handlers = NULL;
	c->tcp_free = NULL;
	c->type = comm_local;
	c->tcp_do_close = 0;
	c->do_not_close = 1;
	c->tcp_do_toggle_rw = 0;
	c->tcp_check_nb_connect = 0;
	c->callback = callback;
	c->cb_arg = callback_arg;
	/* libevent stuff */
	evbits = EV_PERSIST | EV_READ;
	event_set(&c->ev->ev, c->fd, evbits, comm_point_local_handle_callback, 
		c);
	if(event_base_set(base->eb->base, &c->ev->ev) != 0 ||
		event_add(&c->ev->ev, c->timeout) != 0 )
	{
		log_err("could not add tcphdl event");
		free(c->ev);
		free(c);
		return NULL;
	}
	return c;
}

void 
comm_point_close(struct comm_point* c)
{
	if(!c)
		return;
	if(c->fd != -1)
		if(event_del(&c->ev->ev) != 0) {
			log_err("could not event_del on close");
		}
	/* close fd after removing from event lists, or epoll.. is messed up */
	if(c->fd != -1 && !c->do_not_close)
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
	free(c->timeout);
	if(c->type == comm_tcp || c->type == comm_local)
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

void 
comm_point_send_reply(struct comm_reply *repinfo)
{
	log_assert(repinfo && repinfo->c);
	if(repinfo->c->type == comm_udp) {
		comm_point_send_udp_msg(repinfo->c, repinfo->c->buffer,
			(struct sockaddr*)&repinfo->addr, repinfo->addrlen);
	} else {
		comm_point_start_listening(repinfo->c, -1, TCP_QUERY_TIMEOUT);
	}
}

void 
comm_point_send_reply_iov(struct comm_reply* repinfo, struct iovec* iov,
        size_t iovlen)
{
	log_assert(repinfo && repinfo->c);
	if(repinfo->c->type == comm_udp) {
		struct msghdr hdr;
		memset(&hdr, 0, sizeof(hdr));
		hdr.msg_name = &repinfo->addr;
		hdr.msg_namelen = repinfo->addrlen;
		hdr.msg_iov = iov + 1;
		hdr.msg_iovlen = (TYPE_MSGIOVLEN)(iovlen - 1);
		/* note that number of characters sent is not checked. */
		if(sendmsg(repinfo->c->fd, &hdr, 0) == -1)
			log_err("sendmsg: %s", strerror(errno));
	} else {
		/* try if it can be sent in writev right now */
		size_t i;
		uint16_t len = 0;
		ssize_t done;
		for(i=1; i<iovlen; i++)
			len += iov[i].iov_len;
		len = htons(len);
		iov[0].iov_base = (void*)&len;
		iov[0].iov_len = sizeof(uint16_t);
		if((done=writev(repinfo->c->fd, iov, (int)iovlen)) == -1) {
#ifdef S_SPLINT_S
			/* don't complain about returning stack references */
			iov[0].iov_base = NULL;
#endif
			if(errno != EINTR && errno != EAGAIN) {
				log_err("writev: %s", strerror(errno));
				comm_point_drop_reply(repinfo);
				return;
			}
			done = 0;
		}
#ifdef S_SPLINT_S
		/* don't complain about returning stack references */
		iov[0].iov_base = NULL;
#endif
		if((size_t)done == ntohs(len) + sizeof(uint16_t)) {
			/* done in one call */
			comm_point_drop_reply(repinfo);
		} else {
			/* sending remaining bytes */
			ldns_buffer_clear(repinfo->c->buffer);
			repinfo->c->tcp_byte_count = (size_t)done;
			for(i=1; i<iovlen; i++)
				ldns_buffer_write(repinfo->c->buffer,
					iov[i].iov_base, iov[i].iov_len);
			ldns_buffer_flip(repinfo->c->buffer);
			if((size_t)done >= sizeof(uint16_t))
				ldns_buffer_set_position(repinfo->c->buffer,
					(size_t)done - sizeof(uint16_t));
			comm_point_start_listening(repinfo->c, -1, 
				TCP_QUERY_TIMEOUT);
		}
	}
}

void 
comm_point_drop_reply(struct comm_reply* repinfo)
{
	if(!repinfo)
		return;
	log_assert(repinfo && repinfo->c);
	log_assert(repinfo->c->type != comm_tcp_accept);
	if(repinfo->c->type == comm_udp)
		return;
	reclaim_tcp_handler(repinfo->c);
}

void 
comm_point_stop_listening(struct comm_point* c)
{
	verbose(VERB_ALGO, "comm point stop listening %d", c->fd);
	if(event_del(&c->ev->ev) != 0) {
		log_err("event_del error to stoplisten");
	}
}

void 
comm_point_start_listening(struct comm_point* c, int newfd, int sec)
{
	verbose(VERB_ALGO, "comm point start listening %d", c->fd);
	if(c->type == comm_tcp_accept && !c->tcp_free) {
		/* no use to start listening no free slots. */
		return;
	}
	if(sec != -1 && sec != 0) {
		if(!c->timeout) {
			c->timeout = (struct timeval*)malloc(sizeof(
				struct timeval));
			if(!c->timeout) {
				log_err("cpsl: malloc failed. No net read.");
				return;
			}
		}
#ifndef S_SPLINT_S /* splint fails on struct timeval. */
		c->timeout->tv_sec = sec;
		c->timeout->tv_usec = 0;
#endif /* S_SPLINT_S */
	}
	if(c->type == comm_tcp) {
		c->ev->ev.ev_events &= ~(EV_READ|EV_WRITE);
		if(c->tcp_is_reading)
			c->ev->ev.ev_events |= EV_READ;
		else	c->ev->ev.ev_events |= EV_WRITE;
	}
	if(newfd != -1) {
		if(c->fd != -1)
			close(c->fd);
		c->fd = newfd;
		c->ev->ev.ev_fd = c->fd;
	}
	if(event_add(&c->ev->ev, sec==0?NULL:c->timeout) != 0) {
		log_err("event_add failed. in cpsl.");
	}
}

struct comm_timer* 
comm_timer_create(struct comm_base* base, void (*cb)(void*), void* cb_arg)
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

void 
comm_timer_disable(struct comm_timer* timer)
{
	if(!timer)
		return;
	evtimer_del(&timer->ev_timer->ev);
	timer->ev_timer->enabled = 0;
}

void 
comm_timer_set(struct comm_timer* timer, struct timeval* tv)
{
	if(timer->ev_timer->enabled)
		comm_timer_disable(timer);
	evtimer_add(&timer->ev_timer->ev, tv);
	timer->ev_timer->enabled = 1;
}

void 
comm_timer_delete(struct comm_timer* timer)
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

struct comm_signal* 
comm_signal_create(struct comm_base* base,
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

static void 
comm_signal_callback(int sig, short event, void* arg)
{
	struct comm_signal* comsig = (struct comm_signal*)arg;
	if(!(event & EV_SIGNAL))
		return;
	(*comsig->callback)(sig, comsig->cb_arg);
}

int 
comm_signal_bind(struct comm_signal* comsig, int sig)
{
	struct internal_signal* entry = (struct internal_signal*)calloc(1, 
		sizeof(struct internal_signal));
	if(!entry) {
		log_err("malloc failed");
		return 0;
	}
	log_assert(comsig);
	/* add signal event */
	signal_set(&entry->ev, sig, comm_signal_callback, comsig);
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

void 
comm_signal_delete(struct comm_signal* comsig)
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
