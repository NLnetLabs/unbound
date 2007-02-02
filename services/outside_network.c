/*
 * services/outside_network.c - implement sending of queries and wait answer.
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
 * This file has functions to send queries to authoritative servers and
 * wait for the pending answer events.
 */

#include "services/outside_network.h"
#include "services/listen_dnsport.h"
#include "util/netevent.h"
#include "util/log.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

/** number of times to retry making a random ID that is unique. */
#define MAX_ID_RETRY 1000

/** compare function of pending rbtree */
static int pending_cmp(const void* key1, const void* key2)
{
	struct pending *p1 = (struct pending*)key1;
	struct pending *p2 = (struct pending*)key2;
	if(p1->id < p2->id)
		return -1;
	if(p1->id > p2->id)
		return 1;
	log_assert(p1->id == p2->id);
	if(p1->addrlen < p2->addrlen)
		return -1;
	if(p1->addrlen > p2->addrlen)
		return 1;
	log_assert(p1->addrlen == p2->addrlen);
	return memcmp(&p1->addr, &p2->addr, p1->addrlen);
}

/** callback for incoming udp answers from the network. */
static int outnet_udp_cb(struct comm_point* c, void* arg, int error,
	struct comm_reply *reply_info)
{
	struct outside_network* outnet = (struct outside_network*)arg;
	struct pending key;
	struct pending* p;
	log_info("answer cb");

	if(error != 0) {
		log_info("outnetudp got udp error %d", error);
		return 0;
	}
	log_assert(reply_info);

	/* setup lookup key */
	key.id = LDNS_ID_WIRE(ldns_buffer_begin(c->buffer));
	memcpy(&key.addr, &reply_info->addr, reply_info->addrlen);
	key.addrlen = reply_info->addrlen;

	/* find it, see if this thing is a valid query */
	p = (struct pending*)rbtree_search(outnet->pending, &key);
	if(!p) {
		verbose(VERB_DETAIL, "received uncalled udp reply. dropped.");
		return 0;
	}

	verbose(VERB_ALGO, "received udp reply.");
	if(p->c != c) {
		verbose(VERB_DETAIL, "received answer on wrong port. dropped");
		return 0;
	}
	comm_timer_disable(p->timer);
	/* TODO handle it */

	return 0;
}

/** open another udp port to listen to, every thread has its own range
  * of open ports.
  * @param ifname: on which interface to open the port.
  * @param hints: hints on family and passiveness preset.
  * @param porthint: if not -1, it gives the port to base range on.
  * @return: file descriptor
  */
static int open_udp_port_range(const char* ifname, struct addrinfo* hints,
	int porthint)
{
	struct addrinfo *res = NULL;
	int r, s;
	char portstr[20];
	if(porthint != -1)
		snprintf(portstr, sizeof(portstr), "%d", porthint);

	if((r=getaddrinfo(ifname, ((porthint==-1)?NULL:portstr), hints, 
		&res)) != 0 || !res) {
		log_err("node %s %s getaddrinfo: %s %s",
			ifname?ifname:"default", (porthint!=-1)?portstr:"eph", 
			gai_strerror(r), r==EAI_SYSTEM?strerror(errno):"");
		return -1;
	}
	s = create_udp_sock(res);
	freeaddrinfo(res);
	return s;
}

/**
 * Create range of UDP ports on the given interface.
 * Returns number of ports bound.
 * @param coms: communication point array start position. Filled with entries.
 * @param ifname: name of interface to make port on.
 * @param num_ports: number of ports opened.
 * @param do_ip4: if true make ip4 ports.
 * @param do_ip6: if true make ip6 ports.
 * @param porthint: -1 for system chosen port, or a base of port range.
 * @param outnet: network structure with comm base, shared udp buffer.
 * @return: the number of ports successfully opened, entries filled in coms.
 */
static size_t 
make_udp_range(struct comm_point** coms, const char* ifname, 
	size_t num_ports, int do_ip4, int do_ip6, int porthint,
	struct outside_network* outnet)
{
	size_t i;
	size_t done = 0;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	if(ifname)
		hints.ai_flags |= AI_NUMERICHOST;
	hints.ai_family = AF_UNSPEC;
	if(do_ip4 && do_ip6)
		hints.ai_family = AF_UNSPEC;
	else if(do_ip4)
		hints.ai_family = AF_INET;
	else if(do_ip6)
		hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	for(i=0; i<num_ports; i++) {
		int fd = open_udp_port_range(ifname, &hints, porthint);
		if(porthint != -1) 
			porthint++;
		if(fd == -1)
			continue;
		coms[done] = comm_point_create_udp(outnet->base, fd, 
			outnet->udp_buff, outnet_udp_cb, outnet);
		if(coms[done])
			done++;
	}
	return done;
}

/** returns true is string addr is an ip6 specced address. */
int str_is_ip6(const char* str)
{
	if(strchr(str, ':'))
		return 1;
	else 	return 0;
}

/** calculate number of ip4 and ip6 interfaces, times multiplier. */
static void calc_num46(const char** ifs, int num_ifs, 
	int do_ip4, int do_ip6, size_t multiplier, 
	size_t* num_ip4, size_t* num_ip6)
{
	int i;
	*num_ip4 = 0;
	*num_ip6 = 0;
	if(num_ifs <= 0) {
		if(do_ip4)
			*num_ip4 = multiplier;
		if(do_ip6)
			*num_ip6 = multiplier;
		return;
	}
	for(i=0; i<num_ifs; i++)
	{
		if(str_is_ip6(ifs[i])) {
			if(do_ip6)
				*num_ip6 += multiplier;
		} else {
			if(do_ip4)
				*num_ip4 += multiplier;
		}
	}

}

/** callback for udp timeout */
static void pending_udp_timer_cb(void *arg)
{
	/* struct pending* p = (struct pending*)arg; */
	/* it timed out . TODO handle it. */
}

struct outside_network* 
outside_network_create(struct comm_base *base, size_t bufsize, 
	size_t num_ports, const char** ifs, int num_ifs, int do_ip4, 
	int do_ip6, int port_base)
{
	struct outside_network* outnet = (struct outside_network*)
		calloc(1, sizeof(struct outside_network));
	int k;
	if(!outnet) {
		log_err("malloc failed");
		return NULL;
	}
	outnet->base = base;
	calc_num46(ifs, num_ifs, do_ip4, do_ip6, num_ports, 
		&outnet->num_udp4, &outnet->num_udp6);
	if(	!(outnet->udp_buff = ldns_buffer_new(bufsize)) ||
		!(outnet->udp4_ports = (struct comm_point **)calloc(
			outnet->num_udp4+1, sizeof(struct comm_point*))) ||
		!(outnet->udp6_ports = (struct comm_point **)calloc(
			outnet->num_udp6+1, sizeof(struct comm_point*))) ||
		!(outnet->pending = rbtree_create(pending_cmp)) ) {
		log_err("malloc failed");
		outside_network_delete(outnet);
		return NULL;
	}
	/* Try to get ip6 and ip4 ports. Ip6 first, in case second fails. */
	if(num_ifs == 0) {
		if(do_ip6) {
		   	outnet->num_udp6 = make_udp_range(outnet->udp6_ports, 
				NULL, num_ports, 0, 1, port_base, outnet);
		}
		if(do_ip4) {
			outnet->num_udp4 = make_udp_range(outnet->udp4_ports, 
				NULL, num_ports, 1, 0, port_base, outnet);
		}
		if(outnet->num_udp4 != num_ports || 
			outnet->num_udp6 != num_ports) {
			log_err("Could not open all networkside ports");
			outside_network_delete(outnet);
			return NULL;
		}
	}
	else {
		size_t done_4 = 0, done_6 = 0;
		for(k=0; k<num_ifs; k++) {
			if(str_is_ip6(ifs[k]) && do_ip6) {
				done_6 += make_udp_range(
					outnet->udp6_ports+done_6, ifs[k],
					num_ports, 0, 1, port_base, outnet);
			}
			if(!str_is_ip6(ifs[k]) && do_ip4) {
				done_4 += make_udp_range(
					outnet->udp4_ports+done_4, ifs[k],
					num_ports, 1, 0, port_base, outnet);
			}
		}
		if(done_6 != outnet->num_udp6 || done_4 != outnet->num_udp4) {
			log_err("Could not open all ports on all interfaces");
			outside_network_delete(outnet);
			return NULL;
		}
		outnet->num_udp6 = done_6;
		outnet->num_udp4 = done_4;
	}
	return outnet;
}

void outside_network_delete(struct outside_network* outnet)
{
	if(!outnet)
		return;
	/* check every element, since we can be called on malloc error */
	if(outnet->pending) {
		struct pending *p, *np;
		p = (struct pending*)rbtree_first(outnet->pending);
		while(p && (rbnode_t*)p!=RBTREE_NULL) {
			np = (struct pending*)rbtree_next((rbnode_t*)p);
			pending_delete(NULL, p);
			p = np;
		}
		free(outnet->pending);
	}
	if(outnet->udp_buff)
		ldns_buffer_free(outnet->udp_buff);
	if(outnet->udp4_ports) {
		size_t i;
		for(i=0; i<outnet->num_udp4; i++)
			comm_point_delete(outnet->udp4_ports[i]);
		free(outnet->udp4_ports);
	}
	if(outnet->udp6_ports) {
		size_t i;
		for(i=0; i<outnet->num_udp6; i++)
			comm_point_delete(outnet->udp6_ports[i]);
		free(outnet->udp6_ports);
	}
	free(outnet);
}

void pending_delete(struct outside_network* outnet, struct pending* p)
{
	if(!p)
		return;
	if(outnet) {
		(void)rbtree_delete(outnet->pending, p->node.key);
	}
	if(p->timer)
		comm_timer_delete(p->timer);
	free(p);
}

/** create a new pending item with given characteristics, false on failure */
static struct pending*
new_pending(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen)
{
	/* alloc */
	int id_tries = 0;
	struct pending* pend = (struct pending*)calloc(1, 
		sizeof(struct pending));
	if(!pend) {
		log_err("malloc failure");
		return NULL;
	}
	pend->timer = comm_timer_create(outnet->base, pending_udp_timer_cb, 
		pend);
	if(!pend->timer) {
		free(pend);
		return NULL;
	}
	/* set */
	pend->id = LDNS_ID_WIRE(ldns_buffer_begin(packet));
	memcpy(&pend->addr, addr, addrlen);
	pend->addrlen = addrlen;

	/* insert in tree */
	pend->node.key = pend;
	while(!rbtree_insert(outnet->pending, &pend->node)) {
		/* change ID to avoid collision */
		pend->id = (random()>>8) & 0xffff;
		LDNS_ID_SET(ldns_buffer_begin(packet), pend->id);
		id_tries++;
		if(id_tries == MAX_ID_RETRY) {
			log_err("failed to generate unique ID, drop msg");
			pending_delete(NULL, pend);
			return NULL;
		}
	}
	return pend;
}

/** 
 * Checkout address family.
 * @param addr: the sockaddr to examine.
 * return: true if sockaddr is ip6.
 */
static int addr_is_ip6(struct sockaddr_storage* addr)
{
	short family = *(short*)addr;
	if(family == AF_INET6)
		return 1;
	else	return 0;
}

/** 
 * Select outgoing comm point for a query. Fills in c. 
 * @param outnet: network structure that has arrays of ports to choose from.
 * @param pend: the message to send. c is filled in, randomly chosen.
 */
static void select_port(struct outside_network* outnet, struct pending* pend)
{
	double precho;
	int chosen, nummax;

	log_assert(outnet && pend);
	/* first select ip4 or ip6. */
	if(addr_is_ip6(&pend->addr))
		nummax = (int)outnet->num_udp6;
	else 	nummax = (int)outnet->num_udp4;

	if(nummax == 0) {
		/* could try ip4to6 mapping if no ip4 ports available */
		log_err("Need to send query but have no ports of that family");
		return;
	}

	/* choose a random outgoing port and interface */
	/* uses lousy random() function. TODO: entropy source. */
	precho = (double)random() * (double)nummax / 
		((double)RAND_MAX + 1.0);
	chosen = (int)precho;

	/* don't trust in perfect double rounding */
	if(chosen < 0) chosen = 0;
	if(chosen >= nummax) chosen = nummax-1;

	if(addr_is_ip6(&pend->addr))
		pend->c = outnet->udp6_ports[chosen];
	else	pend->c = outnet->udp4_ports[chosen];
	log_assert(pend->c);

	log_info("chose query %x outbound %d of %d", 
		pend->id, chosen, nummax);
}


void pending_udp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout)
{
	struct pending* pend;
	struct timeval tv;

	/* create pending struct (and possibly change ID to be unique) */
	if(!(pend=new_pending(outnet, packet, addr, addrlen))) {
		/* callback user for the error */
		return;
	}
	select_port(outnet, pend);

	/* send it over the commlink */
	if(!comm_point_send_udp_msg(pend->c, packet, (struct sockaddr*)addr, 
		addrlen)) {
		/* error, call error callback function */
		pending_delete(outnet, pend);
		/* callback user for the error */
		return;
	}

	/* system calls to set timeout after sending UDP to make roundtrip
	   smaller. */
	tv.tv_sec = (int)time(NULL) + timeout;
	tv.tv_usec = 0;
	comm_timer_set(pend->timer, &tv);
}
