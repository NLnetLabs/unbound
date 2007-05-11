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
#include "util/net_help.h"
#include "util/random.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <fcntl.h>

/** number of times to retry making a random ID that is unique. */
#define MAX_ID_RETRY 1000
/** byte size of ip4 address */
#define INET_SIZE 4
/** byte size of ip6 address */
#define INET6_SIZE 16 

/** compare function of pending rbtree */
static int 
pending_cmp(const void* key1, const void* key2)
{
	struct pending *p1 = (struct pending*)key1;
	struct pending *p2 = (struct pending*)key2;
	struct sockaddr_in* p1_in = (struct sockaddr_in*)&p1->addr;
	struct sockaddr_in* p2_in = (struct sockaddr_in*)&p2->addr;
	struct sockaddr_in6* p1_in6 = (struct sockaddr_in6*)&p1->addr;
	struct sockaddr_in6* p2_in6 = (struct sockaddr_in6*)&p2->addr;
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
	if( p1_in->sin_family < p2_in->sin_family)
		return -1;
	if( p1_in->sin_family > p2_in->sin_family)
		return 1;
	log_assert( p1_in->sin_family == p2_in->sin_family );
	/* compare ip4 */
	if( p1_in->sin_family == AF_INET ) {
		/* just order it, ntohs not required */
		if(p1_in->sin_port < p2_in->sin_port)
			return -1;
		if(p1_in->sin_port > p2_in->sin_port)
			return 1;
		log_assert(p1_in->sin_port == p2_in->sin_port);
		return memcmp(&p1_in->sin_addr, &p2_in->sin_addr, INET_SIZE);
	} else if (p1_in6->sin6_family == AF_INET6) {
		/* just order it, ntohs not required */
		if(p1_in6->sin6_port < p2_in6->sin6_port)
			return -1;
		if(p1_in6->sin6_port > p2_in6->sin6_port)
			return 1;
		log_assert(p1_in6->sin6_port == p2_in6->sin6_port);
		return memcmp(&p1_in6->sin6_addr, &p2_in6->sin6_addr, 
			INET6_SIZE);
	} else {
		/* eek unknown type, perform this comparison for sanity. */
		return memcmp(&p1->addr, &p2->addr, p1->addrlen);
	}
}

/** delete waiting_tcp entry. Does not unlink from waiting list. 
 * @param w: to delete.
 */
static void
waiting_tcp_delete(struct waiting_tcp* w)
{
	if(!w) return;
	if(w->timer)
		comm_timer_delete(w->timer);
	free(w);
}

/** use next free buffer to service a tcp query */
static int
outnet_tcp_take_into_use(struct waiting_tcp* w, uint8_t* pkt)
{
	struct pending_tcp* pend = w->outnet->tcp_free;
	int s;
	log_assert(pend);
	log_assert(pkt);
	/* open socket */
#ifndef INET6
	if(addr_is_ip6(addr))
		s = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
	else
#endif
		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s == -1) {
		log_err("outgoing tcp: socket: %s", strerror(errno));
		log_addr(&w->addr, w->addrlen);
		return 0;
	}
	fd_set_nonblock(s);
	if(connect(s, (struct sockaddr*)&w->addr, w->addrlen) == -1) {
		if(errno != EINPROGRESS) {
			log_err("outgoing tcp: connect: %s", strerror(errno));
			log_addr(&w->addr, w->addrlen);
			close(s);
			return 0;
		}
	}
	w->pkt = NULL;
	w->next_waiting = (void*)pend;
	pend->id = LDNS_ID_WIRE(pkt);
	w->outnet->tcp_free = pend->next_free;
	pend->next_free = NULL;
	pend->query = w;
	ldns_buffer_clear(pend->c->buffer);
	ldns_buffer_write(pend->c->buffer, pkt, w->pkt_len);
	ldns_buffer_flip(pend->c->buffer);
	pend->c->tcp_is_reading = 0;
	pend->c->tcp_byte_count = 0;
	comm_point_start_listening(pend->c, s, -1);
	return 1;
}

/** see if buffers can be used to service TCP queries. */
static void
use_free_buffer(struct outside_network* outnet)
{
	struct waiting_tcp* w;
	while(outnet->tcp_free && outnet->tcp_wait_first) {
		w = outnet->tcp_wait_first;
		outnet->tcp_wait_first = w->next_waiting;
		if(outnet->tcp_wait_last == w)
			outnet->tcp_wait_last = NULL;
		if(!outnet_tcp_take_into_use(w, w->pkt)) {
			(void)(*w->cb)(NULL, w->cb_arg, NETEVENT_CLOSED, NULL);
			waiting_tcp_delete(w);
		}
	}
}

/** callback for pending tcp connections */
static int 
outnet_tcp_cb(struct comm_point* c, void* arg, int error,
	struct comm_reply *reply_info)
{
	struct pending_tcp* pend = (struct pending_tcp*)arg;
	struct outside_network* outnet = pend->query->outnet;
	verbose(VERB_ALGO, "outnettcp cb");
	if(error != NETEVENT_NOERROR) {
		log_info("outnettcp got tcp error %d", error);
		/* pass error below and exit */
	} else {
		/* check ID */
		if(ldns_buffer_limit(c->buffer) < sizeof(uint16_t) ||
			LDNS_ID_WIRE(ldns_buffer_begin(c->buffer))!=pend->id) {
			log_info("outnettcp: bad ID in reply, from:");
			log_addr(&pend->query->addr, pend->query->addrlen);
			error = NETEVENT_CLOSED;
		}
	}
	(void)(*pend->query->cb)(c, pend->query->cb_arg, error, reply_info);
	comm_point_close(c);
	pend->next_free = outnet->tcp_free;
	outnet->tcp_free = pend;
	waiting_tcp_delete(pend->query);
	pend->query = NULL;
	use_free_buffer(outnet);
	return 0;
}

/** callback for incoming udp answers from the network. */
static int 
outnet_udp_cb(struct comm_point* c, void* arg, int error,
	struct comm_reply *reply_info)
{
	struct outside_network* outnet = (struct outside_network*)arg;
	struct pending key;
	struct pending* p;
	verbose(VERB_ALGO, "answer cb");

	if(error != NETEVENT_NOERROR) {
		log_info("outnetudp got udp error %d", error);
		return 0;
	}
	if(ldns_buffer_limit(c->buffer) < LDNS_HEADER_SIZE) {
		log_info("outnetudp udp too short");
		return 0;
	}
	log_assert(reply_info);

	/* setup lookup key */
	key.id = LDNS_ID_WIRE(ldns_buffer_begin(c->buffer));
	memcpy(&key.addr, &reply_info->addr, reply_info->addrlen);
	key.addrlen = reply_info->addrlen;
	verbose(VERB_ALGO, "Incoming reply id=%4.4x addr=", key.id);
	log_addr(&key.addr, key.addrlen);

	/* find it, see if this thing is a valid query response */
	verbose(VERB_ALGO, "lookup size is %d entries", (int)outnet->pending->count);
	p = (struct pending*)rbtree_search(outnet->pending, &key);
	if(!p) {
		verbose(VERB_DETAIL, "received unsolicited udp reply. dropped.");
		return 0;
	}

	verbose(VERB_ALGO, "received udp reply.");
	if(p->c != c) {
		verbose(VERB_DETAIL, "received reply id,addr on wrong port. "
			"dropped.");
		return 0;
	}
	comm_timer_disable(p->timer);
	verbose(VERB_ALGO, "outnet handle udp reply");
	(void)(*p->cb)(p->c, p->cb_arg, NETEVENT_NOERROR, NULL);
	pending_delete(outnet, p);
	return 0;
}

/** open another udp port to listen to, every thread has its own range
  * of open ports.
  * @param ifname: on which interface to open the port.
  * @param hints: hints on family and passiveness preset.
  * @param porthint: if not -1, it gives the port to base range on.
  * @return: file descriptor
  */
static int 
open_udp_port_range(const char* ifname, struct addrinfo* hints, int porthint)
{
	struct addrinfo *res = NULL;
	int r, s;
	char portstr[32];
	if(porthint != -1)
		snprintf(portstr, sizeof(portstr), "%d", porthint);

	if((r=getaddrinfo(ifname, ((porthint==-1)?NULL:portstr), hints, 
		&res)) != 0 || !res) {
		log_err("node %s %s getaddrinfo: %s %s",
			ifname?ifname:"default", (porthint!=-1)?portstr:"eph", 
			gai_strerror(r), 
			r==EAI_SYSTEM?(char*)strerror(errno):"");
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

/** calculate number of ip4 and ip6 interfaces, times multiplier. */
static void 
calc_num46(char** ifs, int num_ifs, int do_ip4, int do_ip6, 
	size_t multiplier, size_t* num_ip4, size_t* num_ip6)
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
static void 
pending_udp_timer_cb(void *arg)
{
	struct pending* p = (struct pending*)arg;
	/* it timed out */
	verbose(VERB_ALGO, "timeout udp");
	(void)(*p->cb)(p->c, p->cb_arg, NETEVENT_TIMEOUT, NULL);
	pending_delete(p->outnet, p);
}

/** create pending_tcp buffers */
static int
create_pending_tcp(struct outside_network* outnet, size_t bufsize)
{
	size_t i;
	if(outnet->num_tcp == 0)
		return 1; /* no tcp needed, nothing to do */
	if(!(outnet->tcp_conns = (struct pending_tcp **)calloc(
			outnet->num_tcp, sizeof(struct pending_tcp*))))
		return 0;
	for(i=0; i<outnet->num_tcp; i++) {
		if(!(outnet->tcp_conns[i] = (struct pending_tcp*)calloc(1, 
			sizeof(struct pending_tcp))))
			return 0;
		outnet->tcp_conns[i]->next_free = outnet->tcp_free;
		outnet->tcp_free = outnet->tcp_conns[i];
		outnet->tcp_conns[i]->c = comm_point_create_tcp_out(
			outnet->base, bufsize, outnet_tcp_cb, 
			outnet->tcp_conns[i]);
		if(!outnet->tcp_conns[i]->c)
			return 0;
	}
	return 1;
}

struct outside_network* 
outside_network_create(struct comm_base *base, size_t bufsize, 
	size_t num_ports, char** ifs, int num_ifs, int do_ip4, 
	int do_ip6, int port_base, size_t num_tcp)
{
	struct outside_network* outnet = (struct outside_network*)
		calloc(1, sizeof(struct outside_network));
	int k;
	if(!outnet) {
		log_err("malloc failed");
		return NULL;
	}
	outnet->base = base;
	outnet->num_tcp = num_tcp;
#ifndef INET6
	do_ip6 = 0;
#endif
	calc_num46(ifs, num_ifs, do_ip4, do_ip6, num_ports, 
		&outnet->num_udp4, &outnet->num_udp6);
	/* adds +1 to portnums so we do not allocate zero bytes. */
	if(	!(outnet->udp_buff = ldns_buffer_new(bufsize)) ||
		!(outnet->udp4_ports = (struct comm_point **)calloc(
			outnet->num_udp4+1, sizeof(struct comm_point*))) ||
		!(outnet->udp6_ports = (struct comm_point **)calloc(
			outnet->num_udp6+1, sizeof(struct comm_point*))) ||
		!(outnet->pending = rbtree_create(pending_cmp)) ||
		!create_pending_tcp(outnet, bufsize)) {
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
		if( (do_ip4 && outnet->num_udp4 != num_ports) || 
			(do_ip6 && outnet->num_udp6 != num_ports)) {
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

/** helper pending delete */
static void
pending_node_del(rbnode_t* node, void* arg)
{
	struct pending* pend = (struct pending*)node;
	struct outside_network* outnet = (struct outside_network*)arg;
	pending_delete(outnet, pend);
}

void 
outside_network_delete(struct outside_network* outnet)
{
	if(!outnet)
		return;
	/* check every element, since we can be called on malloc error */
	if(outnet->pending) {
		/* free pending elements, but do no unlink from tree. */
		traverse_postorder(outnet->pending, pending_node_del, NULL);
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
	if(outnet->tcp_conns) {
		size_t i;
		for(i=0; i<outnet->num_tcp; i++)
			if(outnet->tcp_conns[i]) {
				comm_point_delete(outnet->tcp_conns[i]->c);
				waiting_tcp_delete(outnet->tcp_conns[i]->query);
				free(outnet->tcp_conns[i]);
			}
		free(outnet->tcp_conns);
	}
	if(outnet->tcp_wait_first) {
		struct waiting_tcp* p = outnet->tcp_wait_first, *np;
		while(p) {
			np = p->next_waiting;
			waiting_tcp_delete(p);
			p = np;
		}
	}

	free(outnet);
}

void 
pending_delete(struct outside_network* outnet, struct pending* p)
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
	struct sockaddr_storage* addr, socklen_t addrlen,
	comm_point_callback_t* callback, void* callback_arg, 
	struct ub_randstate* rnd)
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
	/* id uses lousy random() TODO use better and entropy */
	pend->id = ((unsigned)ub_random(rnd)>>8) & 0xffff;
	LDNS_ID_SET(ldns_buffer_begin(packet), pend->id);
	memcpy(&pend->addr, addr, addrlen);
	pend->addrlen = addrlen;
	pend->cb = callback;
	pend->cb_arg = callback_arg;
	pend->outnet = outnet;

	/* insert in tree */
	pend->node.key = pend;
	while(!rbtree_insert(outnet->pending, &pend->node)) {
		/* change ID to avoid collision */
		pend->id = ((unsigned)ub_random(rnd)>>8) & 0xffff;
		LDNS_ID_SET(ldns_buffer_begin(packet), pend->id);
		id_tries++;
		if(id_tries == MAX_ID_RETRY) {
			log_err("failed to generate unique ID, drop msg");
			pending_delete(NULL, pend);
			return NULL;
		}
	}
	verbose(VERB_ALGO, "inserted new pending reply id=%4.4x addr=", pend->id);
	log_addr(&pend->addr, pend->addrlen);
	return pend;
}

/** 
 * Checkout address family.
 * @param addr: the sockaddr to examine.
 * return: true if sockaddr is ip6.
 */
static int 
addr_is_ip6(struct sockaddr_storage* addr)
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
 * @param rnd: random state for generating ID and port.
 */
static void 
select_port(struct outside_network* outnet, struct pending* pend,
	struct ub_randstate* rnd)
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
	/* TODO: entropy source. */
	precho = (double)ub_random(rnd) * (double)nummax / 
		((double)RAND_MAX + 1.0);
	chosen = (int)precho;

	/* don't trust in perfect double rounding */
	if(chosen < 0) chosen = 0;
	if(chosen >= nummax) chosen = nummax-1;

	if(addr_is_ip6(&pend->addr))
		pend->c = outnet->udp6_ports[chosen];
	else	pend->c = outnet->udp4_ports[chosen];
	log_assert(pend->c);

	verbose(VERB_ALGO, "query %x outbound %d of %d", pend->id, chosen, nummax);
}


int 
pending_udp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* cb, void* cb_arg, struct ub_randstate* rnd)
{
	struct pending* pend;
	struct timeval tv;

	/* create pending struct and change ID to be unique */
	if(!(pend=new_pending(outnet, packet, addr, addrlen, cb, cb_arg, 
		rnd))) {
		return 0;
	}
	select_port(outnet, pend, rnd);

	/* send it over the commlink */
	if(!comm_point_send_udp_msg(pend->c, packet, (struct sockaddr*)addr, 
		addrlen)) {
		pending_delete(outnet, pend);
		return 0;
	}

	/* system calls to set timeout after sending UDP to make roundtrip
	   smaller. */
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	comm_timer_set(pend->timer, &tv);
	return 1;
}

/** callback for outgoing TCP timer event */
static void
outnet_tcptimer(void* arg)
{
	struct waiting_tcp* w = (struct waiting_tcp*)arg;
	struct outside_network* outnet = w->outnet;
	if(w->pkt) {
		/* it is on the waiting list */
		struct waiting_tcp* p=outnet->tcp_wait_first, *prev=NULL;
		while(p) {
			if(p == w) {
				if(prev) prev->next_waiting = w->next_waiting;
				else	outnet->tcp_wait_first=w->next_waiting;
				outnet->tcp_wait_last = prev;
				break;
			}
			prev = p;
			p=p->next_waiting;
		}
	} else {
		/* it was in use */
		struct pending_tcp* pend=(struct pending_tcp*)w->next_waiting;
		comm_point_close(pend->c);
		pend->query = NULL;
		pend->next_free = outnet->tcp_free;
		outnet->tcp_free = pend;
	}
	(void)(*w->cb)(NULL, w->cb_arg, NETEVENT_TIMEOUT, NULL);
	waiting_tcp_delete(w);
	use_free_buffer(outnet);
}

int 
pending_tcp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* callback, void* callback_arg,
	struct ub_randstate* rnd)
{
	struct pending_tcp* pend = outnet->tcp_free;
	struct waiting_tcp* w;
	struct timeval tv;
	uint16_t id;
	/* if no buffer is free allocate space to store query */
	w = (struct waiting_tcp*)malloc(sizeof(struct waiting_tcp) 
		+ (pend?0:ldns_buffer_limit(packet)));
	if(!w) {
		return 0;
	}
	if(!(w->timer = comm_timer_create(outnet->base, outnet_tcptimer, w))) {
		free(w);
		return 0;
	}
	w->pkt = NULL;
	w->pkt_len = ldns_buffer_limit(packet);
	/* id uses lousy random() TODO use better and entropy */
	id = ((unsigned)ub_random(rnd)>>8) & 0xffff;
	LDNS_ID_SET(ldns_buffer_begin(packet), id);
	memcpy(&w->addr, addr, addrlen);
	w->addrlen = addrlen;
	w->outnet = outnet;
	w->cb = callback;
	w->cb_arg = callback_arg;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	comm_timer_set(w->timer, &tv);
	if(pend) {
		/* we have a buffer available right now */
		if(!outnet_tcp_take_into_use(w, ldns_buffer_begin(packet))) {
			waiting_tcp_delete(w);
			return 0;
		}
	} else {
		/* queue up */
		w->pkt = (uint8_t*)w + sizeof(struct waiting_tcp);
		memmove(w->pkt, ldns_buffer_begin(packet), w->pkt_len);
		w->next_waiting = NULL;
		if(outnet->tcp_wait_last)
			outnet->tcp_wait_last->next_waiting = w;
		else	outnet->tcp_wait_first = w;
		outnet->tcp_wait_last = w;
	}
	return 1;
}
