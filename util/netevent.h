/*
 * util/netevent.h - event notification
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
 *
 * There are three types of communication points
 *    o UDP socket - perthread buffer.
 *    o TCP-accept socket - array of TCP-sockets, socketcount.
 *    o TCP socket - own buffer, parent-TCPaccept, read/write state,
 *                   number of bytes read/written, timeout.
 *
 * There are sockets aimed towards our clients and towards the internet.
 *    o frontside - aimed towards our clients, queries come in, answers back.
 *    o behind - aimed towards internet, to the authoritative DNS servers.
 *
 */

#ifndef NET_EVENT_H
#define NET_EVENT_H

#include "config.h"
struct buffer;

/** internal event notification data storage structure. */
struct internal_event;

/** Communication point to the network */
struct comm_point {
	/** behind the scenes structure, with say libevent info. alloced. */
	struct internal_event* ev;

	/** file descriptor for communication point */
	int fd;

	/** timeout (NULL if it does not). Malloced. */
	struct timeval *timeout;

	/** buffer pointer. Either to perthread, or own buffer or NULL */
	struct buffer *buffer;

	/* -------- TCP Handler -------- */
	/** Read/Write state for TCP */
	int tcp_is_reading;
	/** The current read/write count for TCP */
	size_t tcp_byte_count;
	/** parent communication point (for TCP sockets) */
	struct comm_point tcp_parent;

	/* -------- TCP Accept -------- */
	/** current and max number of TCP connections on this socket */
	int cur_tcp_count, max_tcp_count;
	/** malloced array of tcp handlers for a tcp-accept, 
	    of size max_tcp_count. */
	struct comm_point *tcp_handlers;
	/** linked list of free tcp_handlers to use for new queries.
	    For tcp_accept the first entry, for tcp_handlers the next one. */
	struct comm_point *tcp_free;

	/** is this a UDP, TCP-accept or TCP socket. */
	enum comm_point_type {comm_udp, comm_tcp_accept, comm_tcp} type;

	/** what to do when read/write is done.

	    For a query this means it is read in and ready to be processed.
	    After that the buffer will be sent back to client.
	    tcp_accept does not get called back, is NULL then.

	    udp frontside: called after readdone. sendafter.
	    tcp frontside: called readdone, sendafter. close.
	    udp behind: called after readdone. No send after.
	    tcp behind: write done, read done, then called. No send after.

	    declare as: 
	    int my_callback(struct comm_point*, void* cb_arg, int timeout);

	    if the routine returns 0, no answer is sent back.
	    For TCP handlers after the answer is sent back the fd is closed.
	    If a timeout happens, TCP handler is closed, and callback with
	    timeout=1 is called.
	*/
	int (*)(struct comm_point*, void*) callback;
	/** argument to pass to callback. */
	void *cb_arg;
};

#endif /* NET_EVENT_H */
