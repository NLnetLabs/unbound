/*
 * services/outside_network.h - listen to answers from the network
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
 * This file has functions to send queries to authoritative servers,
 * and wait for the pending answer, with timeouts.
 */

#ifndef OUTSIDE_NETWORK_H
#define OUTSIDE_NETWORK_H

#include "config.h"
#include "util/rbtree.h"
#include "util/netevent.h"
struct pending;
struct pending_timeout;
struct ub_randstate;
struct pending_tcp;
struct waiting_tcp;

/**
 * Send queries to outside servers and wait for answers from servers.
 * Contains answer-listen sockets.
 */
struct outside_network {
	/** Base for select calls */
	struct comm_base* base;

	/** buffer shared by UDP connections, since there is only one
	    datagram at any time. */
	ldns_buffer* udp_buff;

	/** 
	 * Array of udp comm point* that are used to listen to pending events.
	 * Each is on a different port. This is for ip4 ports.
	 */
	struct comm_point **udp4_ports;
	/** number of udp4 ports */
	size_t num_udp4;

	/**
	 * The opened ip6 ports.
	 */
	struct comm_point **udp6_ports;
	/** number of udp6 ports */
	size_t num_udp6;

	/** pending udp answers. sorted by id, addr */
	rbtree_t *pending;

	/**
	 * Array of tcp pending used for outgoing TCP connections.
	 * Each can be used to establish a TCP connection with a server.
	 * The file descriptors are -1 if its free, need to be opened for 
	 * the tcp connection. Can be used for ip4 and ip6.
	 */
	struct pending_tcp **tcp_conns;
	/** number of tcp communication points. */
	size_t num_tcp;
	/** list of tcp comm points that are free for use */
	struct pending_tcp* tcp_free;
	/** list of tcp queries waiting for a buffer */
	struct waiting_tcp* tcp_wait_first;
	/** last of waiting query list */
	struct waiting_tcp* tcp_wait_last;
};

/**
 * A query that has an answer pending for it.
 */
struct pending {
	/** redblacktree entry, key is the pending struct(id, addr). */
	rbnode_t node;
	/** the ID for the query */
	uint16_t id;
	/** remote address. */
	struct sockaddr_storage addr;
	/** length of addr field in use. */
	socklen_t addrlen;
	/** comm point it was sent on (and reply must come back on). */
	struct comm_point* c;
	/** timeout event */
	struct comm_timer* timer;
	/** callback for the timeout, error or reply to the message */
	comm_point_callback_t* cb;
	/** callback user argument */
	void* cb_arg;
	/** the outside network it is part of */
	struct outside_network* outnet;
};

/**
 * Pending TCP query to server.
 */
struct pending_tcp {
	/** next in list of free tcp comm points, or NULL. */
	struct pending_tcp* next_free;
	/** the ID for the query; checked in reply */
	uint16_t id;
	/** tcp comm point it was sent on (and reply must come back on). */
	struct comm_point* c;
	/** the query being serviced, NULL if the pending_tcp is unused. */
	struct waiting_tcp* query;
};

/**
 * Query waiting for TCP buffer.
 */
struct waiting_tcp {
	/** 
	 * next in waiting list.
	 * if pkt==0, this points to the pending_tcp structure.
	 */
	struct waiting_tcp* next_waiting;
	/** timeout event; timer keeps running whether the query is
	 * waiting for a buffer or the tcp reply is pending */
	struct comm_timer* timer;
	/** the outside network it is part of */
	struct outside_network* outnet;
	/** remote address. */
	struct sockaddr_storage addr;
	/** length of addr field in use. */
	socklen_t addrlen;
	/** 
	 * The query itself, the query packet to send.
	 * allocated after the waiting_tcp structure.
	 * set to NULL when the query is serviced and it part of pending_tcp.
	 * if this is NULL, the next_waiting points to the pending_tcp.
	 */
	uint8_t* pkt;
	/** length of query packet. */
	size_t pkt_len;
	/** callback for the timeout, error or reply to the message */
	comm_point_callback_t* cb;
	/** callback user argument */
	void* cb_arg;
};

/**
 * Create outside_network structure with N udp ports.
 * @param base: the communication base to use for event handling.
 * @param bufsize: size for network buffers.
 * @param num_ports: number of udp ports to open per interface.
 * @param ifs: interface names (or NULL for default interface).
 *    These interfaces must be able to access all authoritative servers.
 * @param num_ifs: number of names in array ifs.
 * @param do_ip4: service IP4.
 * @param do_ip6: service IP6.
 * @param port_base: if -1 system assigns ports, otherwise try to get
 *    the ports numbered from this starting number.
 * @param num_tcp: number of outgoing tcp buffers to preallocate.
 * @return: the new structure (with no pending answers) or NULL on error.
 */
struct outside_network* outside_network_create(struct comm_base* base,
	size_t bufsize, size_t num_ports, char** ifs, int num_ifs,
	int do_ip4, int do_ip6, int port_base, size_t num_tcp);

/**
 * Delete outside_network structure.
 * @param outnet: object to delete.
 */
void outside_network_delete(struct outside_network* outnet);

/**
 * Send UDP query, create pending answer.
 * Changes the ID for the query to be random and unique for that destination.
 * @param outnet: provides the event handling
 * @param packet: wireformat query to send to destination.
 * @param addr: address to send to.
 * @param addrlen: length of addr.
 * @param timeout: in seconds from now.
 * @param callback: function to call on error, timeout or reply.
 *    The routine does not return an error, instead it calls the callback,
 *    with an error code if an error happens.
 * @param callback_arg: user argument for callback function.
 * @param rnd: random state for generating ID and port.
 */
void pending_udp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* callback, void* callback_arg,
	struct ub_randstate* rnd);

/**
 * Send TCP query. May wait for TCP buffer. Selects ID to be random, and 
 * checks id.
 * @param outnet: provides the event handling.
 * @param packet: wireformat query to send to destination. copied from.
 * @param addr: address to send to.
 * @param addrlen: length of addr.
 * @param timeout: in seconds from now.
 *    Timer starts running now. Timer may expire if all buffers are used,
 *    without any query been sent to the server yet.
 * @param callback: function to call on error, timeout or reply.
 *    The routine does not return an error, instead it calls the callback,
 *    with an error code if an error happens.
 * @param callback_arg: user argument for callback function.
 * @param rnd: random state for generating ID.
 */
void pending_tcp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* callback, void* callback_arg,
	struct ub_randstate* rnd);

/**
 * Delete pending answer.
 * @param outnet: outside network the pending query is part of.
 *    Internal feature: if outnet is NULL, p is not unlinked from rbtree.
 * @param p: deleted
 */
void pending_delete(struct outside_network* outnet, struct pending* p);

#endif /* OUTSIDE_NETWORK_H */
