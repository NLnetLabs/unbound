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
struct pending;
struct pending_timeout;
struct comm_point;
struct comm_base;

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

	/** pending answers. sorted by id, addr */
	rbtree_t *pending;
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
};

/**
 * Create outside_network structure with N udp ports.
 * @param base: the communication base to use for event handling.
 * @param bufsize: size for network buffers.
 * @param num_ports: number of udp ports to open per interface.
 * @param ifs: interface names (or NULL for default interface).
 * @param num_ifs: number of names in array ifs.
 * @param do_ip4: service IP4.
 * @param do_ip6: service IP6.
 * @param port_base: if -1 system assigns ports, otherwise try to get
 *    the ports numbered from this starting number.
 * @return: the new empty structure or NULL on error.
 */
struct outside_network* outside_network_create(struct comm_base* base,
	size_t bufsize, size_t num_ports, const char** ifs, int num_ifs,
	int do_ip4, int do_ip6, int port_base);

/**
 * Delete outside_network structure.
 * @param outnet: object to delete.
 */
void outside_network_delete(struct outside_network* outnet);

/**
 * Send UDP query, create pending answer.
 * @param outnet: provides the event handling
 * @param packet: wireformat query to send to destination.
 * @param addr: address to send to.
 * @param addrlen: length of addr.
 * @param timeout: in seconds from now.
 */
void pending_udp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout);

/**
 * Delete pending answer.
 * @param outnet: outside network the pending query is part of.
 *    Used internal, if NULL, p is not unlinked from rbtree.
 * @param p: deleted
 */
void pending_delete(struct outside_network* outnet, struct pending* p);

/** 
 * See if string is ip4 or ip6.
 * @param str: IP specification.
 * @return: true is string addr is an ip6 specced address. 
 */
int str_is_ip6(const char* str);

#endif /* OUTSIDE_NETWORK_H */
