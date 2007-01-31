/*
 * services/listen_dnsport.h - listen on port 53 for incoming DNS queries.
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
 * This file has functions to get queries from clients.
 */

#ifndef LISTEN_DNSPORT_H
#define LISTEN_DNSPORT_H

#include "config.h"
#include "util/netevent.h"
struct listen_list;

/**
 * Listening for queries structure.
 * Contains list of query-listen sockets.
 */
struct listen_dnsport {
	/** Base for select calls */
	struct comm_base* base;

	/** buffer shared by UDP connections, since there is only one
	    datagram at any time. */
	ldns_buffer* udp_buff;

	/** list of comm points used to get incoming events */
	struct listen_list *cps;
};

/**
 * Single linked list to store event points.
 */
struct listen_list {
	/** next in list */
	struct listen_list *next;
	/** event info */
	struct comm_point *com;
};

/**
 * Getaddrinfo, create socket, bind and listen to zero or more 
 * interfaces for IP4 and/or IP6, for UDP and/or TCP.
 * On the given port number. It creates the listening sockets.
 * @param base: the comm_base that provides event functionality.
 * @param num_ifs: number of interfaces to listen on. Can be 0,
 *	for default all ifs.
 * @param ifs: array of strings with interface specs, IP addresses.
 * @param port: the port number to bind to.
 * @param do_ip4: listen to ip4 queries.
 * @param do_ip6: listen to ip6 queries.
 * @param do_udp: listen to udp queries.
 * @param do_tcp: listen to tcp queries.
 * @param bufsize: size of datagram buffer.
 * @param cb: callback function when a request arrives. It is passed
 *	  the packet and user argument. Return true to send a reply.
 * @param cb_arg: user data argument for callback function.
 * @return: the malloced listening structure, ready for use. NULL on error.
 */
struct listen_dnsport* listen_create(struct comm_base* base,
	int num_ifs, const char* ifs[], const char* port,
	int do_ip4, int do_ip6, int do_udp, int do_tcp,
	size_t bufsize, comm_point_callback_t* cb, void *cb_arg);

/**
 * delete the listening structure
 */
void listen_delete(struct listen_dnsport* listen);

#endif /* LISTEN_DNSPORT_H */
