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
#include "services/cache/infra.h"
#include "util/data/msgparse.h"
#include "util/data/msgreply.h"
#include "util/data/msgencode.h"
#include "util/netevent.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/random.h"
#include "util/fptr_wlist.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <fcntl.h>

/** number of times to retry making a random ID that is unique. */
#define MAX_ID_RETRY 1000
/** number of retries on outgoing UDP queries */
#define OUTBOUND_UDP_RETRY 1

/** initiate TCP transaction for serviced query */
static void serviced_tcp_initiate(struct outside_network* outnet, 
	struct serviced_query* sq, ldns_buffer* buff);

int 
pending_cmp(const void* key1, const void* key2)
{
	struct pending *p1 = (struct pending*)key1;
	struct pending *p2 = (struct pending*)key2;
	if(p1->id < p2->id)
		return -1;
	if(p1->id > p2->id)
		return 1;
	log_assert(p1->id == p2->id);
	return sockaddr_cmp(&p1->addr, p1->addrlen, &p2->addr, p2->addrlen);
}

int 
serviced_cmp(const void* key1, const void* key2)
{
	struct serviced_query* q1 = (struct serviced_query*)key1;
	struct serviced_query* q2 = (struct serviced_query*)key2;
	int r;
	if(q1->qbuflen < q2->qbuflen)
		return -1;
	if(q1->qbuflen > q2->qbuflen)
		return 1;
	log_assert(q1->qbuflen == q2->qbuflen);
	/* will not detect alternate casing of qname */
	if((r = memcmp(q1->qbuf, q2->qbuf, q1->qbuflen)) != 0)
		return r;
	if(q1->dnssec != q2->dnssec) {
		if(q1->dnssec < q2->dnssec)
			return -1;
		return 1;
	}
	return sockaddr_cmp(&q1->addr, q1->addrlen, &q2->addr, q2->addrlen);
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
outnet_tcp_take_into_use(struct waiting_tcp* w, uint8_t* pkt, size_t pkt_len)
{
	struct pending_tcp* pend = w->outnet->tcp_free;
	int s;
	log_assert(pend);
	log_assert(pkt);
	log_assert(w->addrlen > 0);
	/* open socket */
#ifdef INET6
	if(addr_is_ip6(&w->addr, w->addrlen))
		s = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
	else
#endif
		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s == -1) {
		log_err("outgoing tcp: socket: %s", strerror(errno));
		log_addr(0, "failed address", &w->addr, w->addrlen);
		return 0;
	}
	fd_set_nonblock(s);
	if(connect(s, (struct sockaddr*)&w->addr, w->addrlen) == -1) {
		if(errno != EINPROGRESS) {
			log_err("outgoing tcp: connect: %s", strerror(errno));
			log_addr(0, "failed address", &w->addr, w->addrlen);
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
	pend->c->repinfo.addrlen = w->addrlen;
	memcpy(&pend->c->repinfo.addr, &w->addr, w->addrlen);
	ldns_buffer_clear(pend->c->buffer);
	ldns_buffer_write(pend->c->buffer, pkt, pkt_len);
	ldns_buffer_flip(pend->c->buffer);
	pend->c->tcp_is_reading = 0;
	pend->c->tcp_byte_count = 0;
	comm_point_start_listening(pend->c, s, -1);
	return 1;
}

/** see if buffers can be used to service TCP queries */
static void
use_free_buffer(struct outside_network* outnet)
{
	struct waiting_tcp* w;
	while(outnet->tcp_free && outnet->tcp_wait_first) {
		w = outnet->tcp_wait_first;
		outnet->tcp_wait_first = w->next_waiting;
		if(outnet->tcp_wait_last == w)
			outnet->tcp_wait_last = NULL;
		if(!outnet_tcp_take_into_use(w, w->pkt, w->pkt_len)) {
			comm_point_callback_t* cb = w->cb;
			void* cb_arg = w->cb_arg;
			waiting_tcp_delete(w);
			log_assert(fptr_whitelist_pending_tcp(cb));
			(void)(*cb)(NULL, cb_arg, NETEVENT_CLOSED, NULL);
		}
	}
}

/** decomission a tcp buffer, closes commpoint and frees waiting_tcp entry */
static void
decomission_pending_tcp(struct outside_network* outnet, 
	struct pending_tcp* pend)
{
	comm_point_close(pend->c);
	pend->next_free = outnet->tcp_free;
	outnet->tcp_free = pend;
	waiting_tcp_delete(pend->query);
	pend->query = NULL;
	use_free_buffer(outnet);
}

int 
outnet_tcp_cb(struct comm_point* c, void* arg, int error,
	struct comm_reply *reply_info)
{
	struct pending_tcp* pend = (struct pending_tcp*)arg;
	struct outside_network* outnet = pend->query->outnet;
	verbose(VERB_ALGO, "outnettcp cb");
	if(error != NETEVENT_NOERROR) {
		verbose(VERB_DETAIL, "outnettcp got tcp error %d", error);
		/* pass error below and exit */
	} else {
		/* check ID */
		if(ldns_buffer_limit(c->buffer) < sizeof(uint16_t) ||
			LDNS_ID_WIRE(ldns_buffer_begin(c->buffer))!=pend->id) {
			log_addr(VERB_DETAIL, 
				"outnettcp: bad ID in reply, from:",
				&pend->query->addr, pend->query->addrlen);
			error = NETEVENT_CLOSED;
		}
	}
	log_assert(fptr_whitelist_pending_tcp(pend->query->cb));
	(void)(*pend->query->cb)(c, pend->query->cb_arg, error, reply_info);
	decomission_pending_tcp(outnet, pend);
	return 0;
}

int 
outnet_udp_cb(struct comm_point* c, void* arg, int error,
	struct comm_reply *reply_info)
{
	struct outside_network* outnet = (struct outside_network*)arg;
	struct pending key;
	struct pending* p;
	verbose(VERB_ALGO, "answer cb");

	if(error != NETEVENT_NOERROR) {
		verbose(VERB_DETAIL, "outnetudp got udp error %d", error);
		return 0;
	}
	if(ldns_buffer_limit(c->buffer) < LDNS_HEADER_SIZE) {
		verbose(VERB_DETAIL, "outnetudp udp too short");
		return 0;
	}
	log_assert(reply_info);

	/* setup lookup key */
	key.id = LDNS_ID_WIRE(ldns_buffer_begin(c->buffer));
	memcpy(&key.addr, &reply_info->addr, reply_info->addrlen);
	key.addrlen = reply_info->addrlen;
	verbose(VERB_ALGO, "Incoming reply id = %4.4x", key.id);
	log_addr(VERB_ALGO, "Incoming reply addr =", 
		&reply_info->addr, reply_info->addrlen);

	/* find it, see if this thing is a valid query response */
	verbose(VERB_ALGO, "lookup size is %d entries", (int)outnet->pending->count);
	p = (struct pending*)rbtree_search(outnet->pending, &key);
	if(!p) {
		verbose(VERB_DETAIL, "received unwanted or unsolicited udp reply dropped.");
		log_buf(VERB_ALGO, "dropped message", c->buffer);
		return 0;
	}

	verbose(VERB_ALGO, "received udp reply.");
	log_buf(VERB_ALGO, "udp message", c->buffer);
	if(p->c != c) {
		verbose(VERB_DETAIL, "received reply id,addr on wrong port. "
			"dropped.");
		return 0;
	}
	comm_timer_disable(p->timer);
	verbose(VERB_ALGO, "outnet handle udp reply");
	/* delete from tree first in case callback creates a retry */
	(void)rbtree_delete(outnet->pending, p->node.key);
	log_assert(fptr_whitelist_pending_udp(p->cb));
	(void)(*p->cb)(p->c, p->cb_arg, NETEVENT_NOERROR, reply_info);
	pending_delete(NULL, p);
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
	else if(!ifname) {
		if(hints->ai_family == AF_INET)
			ifname = "0.0.0.0";
		else	ifname="::";
	}

	if((r=getaddrinfo(ifname, ((porthint==-1)?NULL:portstr), hints, 
		&res)) != 0 || !res) {
		log_err("node %s %s getaddrinfo: %s %s",
			ifname?ifname:"default", (porthint!=-1)?portstr:"eph", 
			gai_strerror(r), 
			r==EAI_SYSTEM?(char*)strerror(errno):"");
		return -1;
	}
	s = create_udp_sock(res, 1);
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

/** calculate number of ip4 and ip6 interfaces, times multiplier */
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

void 
pending_udp_timer_cb(void *arg)
{
	struct pending* p = (struct pending*)arg;
	/* it timed out */
	verbose(VERB_ALGO, "timeout udp");
	log_assert(fptr_whitelist_pending_udp(p->cb));
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
	int do_ip6, int port_base, size_t num_tcp, struct infra_cache* infra,
	struct ub_randstate* rnd)
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
	outnet->infra = infra;
	outnet->rnd = rnd;
	outnet->svcd_overhead = 0;
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
		!(outnet->serviced = rbtree_create(serviced_cmp)) ||
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
	if(outnet->num_udp4 + outnet->num_udp6 == 0) {
		log_err("Could not open any ports on outgoing interfaces");
		outside_network_delete(outnet);
		return NULL;
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

/** helper serviced delete */
static void
serviced_node_del(rbnode_t* node, void* ATTR_UNUSED(arg))
{
	struct serviced_query* sq = (struct serviced_query*)node;
	struct service_callback* p = sq->cblist, *np;
	free(sq->qbuf);
	while(p) {
		np = p->next;
		free(p);
		p = np;
	}
	free(sq);
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
	if(outnet->serviced) {
		traverse_postorder(outnet->serviced, serviced_node_del, NULL);
		free(outnet->serviced);
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
	verbose(VERB_ALGO, "inserted new pending reply id=%4.4x", pend->id);
	return pend;
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
	if(addr_is_ip6(&pend->addr, pend->addrlen))
		nummax = (int)outnet->num_udp6;
	else 	nummax = (int)outnet->num_udp4;

	if(nummax == 0) {
		/* could try ip4to6 mapping if no ip4 ports available */
		log_err("Need to send query but have no ports of that family");
		return;
	}

	/* choose a random outgoing port and interface */
	precho = (double)ub_random(rnd) * (double)nummax / 
		((double)RAND_MAX + 1.0);
	chosen = (int)precho;

	/* don't trust in perfect double rounding */
	if(chosen < 0) chosen = 0;
	if(chosen >= nummax) chosen = nummax-1;

	if(addr_is_ip6(&pend->addr, pend->addrlen))
		pend->c = outnet->udp6_ports[chosen];
	else	pend->c = outnet->udp4_ports[chosen];
	log_assert(pend->c);

	verbose(VERB_ALGO, "query %x outbound on port %d of %d", pend->id, chosen, nummax);
}


struct pending* 
pending_udp_query(struct outside_network* outnet, ldns_buffer* packet, 
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* cb, void* cb_arg, struct ub_randstate* rnd)
{
	struct pending* pend;
	struct timeval tv;

	/* create pending struct and change ID to be unique */
	if(!(pend=new_pending(outnet, packet, addr, addrlen, cb, cb_arg, 
		rnd))) {
		return NULL;
	}
	select_port(outnet, pend, rnd);
	if(!pend->c) {
		pending_delete(outnet, pend);
		return NULL;
	}

	/* send it over the commlink */
	if(!comm_point_send_udp_msg(pend->c, packet, (struct sockaddr*)addr, 
		addrlen)) {
		pending_delete(outnet, pend);
		return NULL;
	}

	/* system calls to set timeout after sending UDP to make roundtrip
	   smaller. */
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;
	comm_timer_set(pend->timer, &tv);
	return pend;
}

void
outnet_tcptimer(void* arg)
{
	struct waiting_tcp* w = (struct waiting_tcp*)arg;
	struct outside_network* outnet = w->outnet;
	comm_point_callback_t* cb;
	void* cb_arg;
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
	cb = w->cb;
	cb_arg = w->cb_arg;
	waiting_tcp_delete(w);
	log_assert(fptr_whitelist_pending_tcp(cb));
	(void)(*cb)(NULL, cb_arg, NETEVENT_TIMEOUT, NULL);
	use_free_buffer(outnet);
}

struct waiting_tcp* 
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
		return NULL;
	}
	if(!(w->timer = comm_timer_create(outnet->base, outnet_tcptimer, w))) {
		free(w);
		return NULL;
	}
	w->pkt = NULL;
	w->pkt_len = 0;
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
		if(!outnet_tcp_take_into_use(w, ldns_buffer_begin(packet),
			ldns_buffer_limit(packet))) {
			waiting_tcp_delete(w);
			return NULL;
		}
	} else {
		/* queue up */
		w->pkt = (uint8_t*)w + sizeof(struct waiting_tcp);
		w->pkt_len = ldns_buffer_limit(packet);
		memmove(w->pkt, ldns_buffer_begin(packet), w->pkt_len);
		w->next_waiting = NULL;
		if(outnet->tcp_wait_last)
			outnet->tcp_wait_last->next_waiting = w;
		else	outnet->tcp_wait_first = w;
		outnet->tcp_wait_last = w;
	}
	return w;
}

/** create query for serviced queries */
static void
serviced_gen_query(ldns_buffer* buff, uint8_t* qname, size_t qnamelen, 
	uint16_t qtype, uint16_t qclass, uint16_t flags)
{
	ldns_buffer_clear(buff);
	/* skip id */
	ldns_buffer_write_u16(buff, flags);
	ldns_buffer_write_u16(buff, 1); /* qdcount */
	ldns_buffer_write_u16(buff, 0); /* ancount */
	ldns_buffer_write_u16(buff, 0); /* nscount */
	ldns_buffer_write_u16(buff, 0); /* arcount */
	ldns_buffer_write(buff, qname, qnamelen);
	ldns_buffer_write_u16(buff, qtype);
	ldns_buffer_write_u16(buff, qclass);
	ldns_buffer_flip(buff);
}

/** lookup serviced query in serviced query rbtree */
static struct serviced_query*
lookup_serviced(struct outside_network* outnet, ldns_buffer* buff, int dnssec,
	struct sockaddr_storage* addr, socklen_t addrlen)
{
	struct serviced_query key;
	key.node.key = &key;
	key.qbuf = ldns_buffer_begin(buff);
	key.qbuflen = ldns_buffer_limit(buff);
	key.dnssec = dnssec;
	memcpy(&key.addr, addr, addrlen);
	key.addrlen = addrlen;
	key.outnet = outnet;
	return (struct serviced_query*)rbtree_search(outnet->serviced, &key);
}

/** Create new serviced entry */
static struct serviced_query*
serviced_create(struct outside_network* outnet, ldns_buffer* buff, int dnssec,
        struct sockaddr_storage* addr, socklen_t addrlen)
{
	struct serviced_query* sq = (struct serviced_query*)malloc(sizeof(*sq));
	rbnode_t* ins;
	if(!sq) 
		return NULL;
	sq->node.key = sq;
	sq->qbuf = memdup(ldns_buffer_begin(buff), ldns_buffer_limit(buff));
	if(!sq->qbuf) {
		free(sq);
		return NULL;
	}
	sq->qbuflen = ldns_buffer_limit(buff);
	sq->dnssec = dnssec;
	memcpy(&sq->addr, addr, addrlen);
	sq->addrlen = addrlen;
	sq->outnet = outnet;
	sq->cblist = NULL;
	sq->pending = NULL;
	sq->status = serviced_initial;
	sq->retry = 0;
	sq->to_be_deleted = 0;
	ins = rbtree_insert(outnet->serviced, &sq->node);
	log_assert(ins != NULL); /* must not be already present */
	return sq;
}

/** remove waiting tcp from the outnet waiting list */
static void
waiting_list_remove(struct outside_network* outnet, struct waiting_tcp* w)
{
	struct waiting_tcp* p = outnet->tcp_wait_first, *prev = NULL;
	while(p) {
		if(p == w) {
			/* remove w */
			if(prev)
				prev->next_waiting = w->next_waiting;
			else	outnet->tcp_wait_first = w->next_waiting;
			if(outnet->tcp_wait_last == w)
				outnet->tcp_wait_last = prev;
			return;
		}
		prev = p;
		p = p->next_waiting;
	}
}

/** cleanup serviced query entry */
static void
serviced_delete(struct serviced_query* sq)
{
	if(sq->pending) {
		/* clear up the pending query */
		if(sq->status == serviced_query_UDP_EDNS ||
			sq->status == serviced_query_UDP) {
			struct pending* p = (struct pending*)sq->pending;
			pending_delete(sq->outnet, p);
		} else {
			struct waiting_tcp* p = (struct waiting_tcp*)
				sq->pending;
			if(p->pkt == NULL) {
				decomission_pending_tcp(sq->outnet, 
					(struct pending_tcp*)p->next_waiting);
			} else {
				waiting_list_remove(sq->outnet, p);
				waiting_tcp_delete(p);
			}
		}
	}
	/* does not delete from tree, caller has to do that */
	serviced_node_del(&sq->node, NULL);
}

/** put serviced query into a buffer */
static void
serviced_encode(struct serviced_query* sq, ldns_buffer* buff, int with_edns)
{
	/* generate query */
	ldns_buffer_clear(buff);
	ldns_buffer_write_u16(buff, 0); /* id placeholder */
	ldns_buffer_write(buff, sq->qbuf, sq->qbuflen);
	ldns_buffer_flip(buff);
	if(with_edns) {
		/* add edns section */
		struct edns_data edns;
		edns.edns_present = 1;
		edns.ext_rcode = 0;
		edns.edns_version = EDNS_ADVERTISED_VERSION;
		edns.udp_size = EDNS_ADVERTISED_SIZE;
		edns.bits = 0;
		if(sq->dnssec & EDNS_DO)
			edns.bits = EDNS_DO;
		if(sq->dnssec & BIT_CD)
			LDNS_CD_SET(ldns_buffer_begin(buff));
		attach_edns_record(buff, &edns);
	}
}

/**
 * Perform serviced query UDP sending operation.
 * Sends UDP with EDNS, unless infra host marked non EDNS.
 * @param sq: query to send.
 * @param buff: buffer scratch space.
 * @return 0 on error.
 */
static int
serviced_udp_send(struct serviced_query* sq, ldns_buffer* buff)
{
	int rtt, vs;
	time_t now = time(0);

	if(!infra_host(sq->outnet->infra, &sq->addr, sq->addrlen, now, &vs,
		&rtt))
		return 0;
	if(sq->status == serviced_initial) {
		if(vs != -1)
			sq->status = serviced_query_UDP_EDNS;
		else 	sq->status = serviced_query_UDP;
	}
	serviced_encode(sq, buff, sq->status == serviced_query_UDP_EDNS);
	if(gettimeofday(&sq->last_sent_time, NULL) < 0) {
		log_err("gettimeofday: %s", strerror(errno));
		return 0;
	}
	verbose(VERB_ALGO, "serviced query UDP timeout=%d msec", rtt);
	sq->pending = pending_udp_query(sq->outnet, buff, &sq->addr, 
		sq->addrlen, rtt, serviced_udp_callback, sq, sq->outnet->rnd);
	if(!sq->pending)
		return 0;
	return 1;
}

/** call the callbacks for a serviced query */
static void
serviced_callbacks(struct serviced_query* sq, int error, struct comm_point* c,
	struct comm_reply* rep)
{
	struct service_callback* p = sq->cblist, *n;
	int dobackup = (sq->cblist && sq->cblist->next); /* >1 cb*/
	uint8_t *backup_p = NULL;
	size_t backlen = 0;
	rbnode_t* rem;
	/* remove from tree, and schedule for deletion, so that callbacks
	 * can safely deregister themselves and even create new serviced
	 * queries that are identical to this one. */
	rem = rbtree_delete(sq->outnet->serviced, sq);
	log_assert(rem); /* should have been present */
	sq->to_be_deleted = 1; 
	verbose(VERB_ALGO, "svcd callbacks start");
	if(dobackup && c) {
		/* make a backup of the query, since the querystate processing
		 * may send outgoing queries that overwrite the buffer.
		 * use secondary buffer to store the query.
		 * This is a data copy, but faster than packet to server */
		backlen = ldns_buffer_limit(c->buffer);
		backup_p = memdup(ldns_buffer_begin(c->buffer), backlen);
		if(!backup_p) {
			log_err("malloc failure in serviced query callbacks");
			error = NETEVENT_CLOSED;
			c = NULL;
		}
		sq->outnet->svcd_overhead = backlen;
	}
	while(p) {
		n = p->next;
		if(dobackup && c) {
			ldns_buffer_clear(c->buffer);
			ldns_buffer_write(c->buffer, backup_p, backlen);
			ldns_buffer_flip(c->buffer);
		}
		log_assert(fptr_whitelist_serviced_query(p->cb));
		(void)(*p->cb)(c, p->cb_arg, error, rep);
		p = n;
	}
	if(backup_p) {
		free(backup_p);
		sq->outnet->svcd_overhead = 0;
	}
	verbose(VERB_ALGO, "svcd callbacks end");
	log_assert(sq->cblist == NULL);
	serviced_delete(sq);
}

int 
serviced_tcp_callback(struct comm_point* c, void* arg, int error,
        struct comm_reply* rep)
{
	struct serviced_query* sq = (struct serviced_query*)arg;
	struct comm_reply r2;
	sq->pending = NULL; /* removed after this callback */
	if(error != NETEVENT_NOERROR)
		log_addr(VERB_DETAIL, "tcp error for address", 
			&sq->addr, sq->addrlen);
	if(error==NETEVENT_NOERROR)
		infra_update_tcp_works(sq->outnet->infra, &sq->addr,
			sq->addrlen);
	if(error==NETEVENT_NOERROR && LDNS_RCODE_WIRE(ldns_buffer_begin(
		c->buffer)) == LDNS_RCODE_FORMERR && 
		sq->status == serviced_query_TCP_EDNS) {
		if(!infra_edns_update(sq->outnet->infra, &sq->addr, 
			sq->addrlen, -1, time(0)))
			log_err("Out of memory caching no edns for host");
		sq->status = serviced_query_TCP;
		serviced_tcp_initiate(sq->outnet, sq, c->buffer);
		return 0;
	}
	/* insert address into reply info */
	if(!rep) {
		/* create one if there isn't (on errors) */
		rep = &r2;
		r2.c = c;
	}
	memcpy(&rep->addr, &sq->addr, sq->addrlen);
	rep->addrlen = sq->addrlen;
	serviced_callbacks(sq, error, c, rep);
	return 0;
}

static void
serviced_tcp_initiate(struct outside_network* outnet, 
	struct serviced_query* sq, ldns_buffer* buff)
{
	serviced_encode(sq, buff, sq->status == serviced_query_TCP_EDNS);
	sq->pending = pending_tcp_query(outnet, buff, &sq->addr,
		sq->addrlen, TCP_QUERY_TIMEOUT, serviced_tcp_callback, 
		sq, outnet->rnd);
	if(!sq->pending) {
		/* delete from tree so that a retry by above layer does not
		 * clash with this entry */
		log_err("serviced_tcp_initiate: failed to send tcp query");
		serviced_callbacks(sq, NETEVENT_CLOSED, NULL, NULL);
	}
}

int 
serviced_udp_callback(struct comm_point* c, void* arg, int error,
        struct comm_reply* rep)
{
	struct serviced_query* sq = (struct serviced_query*)arg;
	struct outside_network* outnet = sq->outnet;
	struct timeval now;
	int fallback_tcp = 0;
	if(gettimeofday(&now, NULL) < 0) {
		log_err("gettimeofday: %s", strerror(errno));
		/* this option does not need current time */
		error = NETEVENT_CLOSED; 
	}
	sq->pending = NULL; /* removed after callback */
	if(error == NETEVENT_TIMEOUT) {
		int rto = 0;
		sq->retry++;
		if(!(rto=infra_rtt_update(outnet->infra, &sq->addr, sq->addrlen,
			-1, (time_t)now.tv_sec)))
			log_err("out of memory in UDP exponential backoff");
		if(sq->retry < OUTBOUND_UDP_RETRY) {
			log_name_addr(VERB_ALGO, "retry query", sq->qbuf+10,
				&sq->addr, sq->addrlen);
			if(!serviced_udp_send(sq, c->buffer)) {
				serviced_callbacks(sq, NETEVENT_CLOSED, c, rep);
			}
			return 0;
		}
		if(rto >= RTT_MAX_TIMEOUT) {
			fallback_tcp = 1;
			/* UDP does not work, fallback to TCP below */
		} else {
			serviced_callbacks(sq, NETEVENT_TIMEOUT, c, rep);
			return 0;
		}
	}
	if(error == NETEVENT_NOERROR && sq->status == serviced_query_UDP_EDNS 
		&& LDNS_RCODE_WIRE(ldns_buffer_begin(c->buffer)) 
			== LDNS_RCODE_FORMERR) {
		/* note no EDNS, fallback without EDNS */
		if(!infra_edns_update(outnet->infra, &sq->addr, sq->addrlen,
			-1, (time_t)now.tv_sec)) {
			log_err("Out of memory caching no edns for host");
		}
		sq->status = serviced_query_UDP;
		sq->retry = 0;
		if(!serviced_udp_send(sq, c->buffer)) {
			serviced_callbacks(sq, NETEVENT_CLOSED, c, rep);
		}
		return 0;
	}
	if(LDNS_TC_WIRE(ldns_buffer_begin(c->buffer)) ||
		(error != NETEVENT_NOERROR && fallback_tcp)  ) {
		/* fallback to TCP */
		/* this discards partial UDP contents */
		if(sq->status == serviced_query_UDP_EDNS)
			sq->status = serviced_query_TCP_EDNS;
		else	sq->status = serviced_query_TCP;
		serviced_tcp_initiate(outnet, sq, c->buffer);
		return 0;
	}
	/* yay! an answer */
	if(now.tv_sec > sq->last_sent_time.tv_sec ||
		(now.tv_sec == sq->last_sent_time.tv_sec &&
		now.tv_usec > sq->last_sent_time.tv_usec)) {
		/* convert from microseconds to milliseconds */
		int roundtime = (now.tv_sec - sq->last_sent_time.tv_sec)*1000
		  + ((int)now.tv_usec - (int)sq->last_sent_time.tv_usec)/1000;
		verbose(VERB_ALGO, "measured roundtrip at %d msec", roundtime);
		log_assert(roundtime >= 0);
		if(!infra_rtt_update(outnet->infra, &sq->addr, sq->addrlen, 
			roundtime, (time_t)now.tv_sec))
			log_err("out of memory noting rtt.");
	}
	serviced_callbacks(sq, error, c, rep);
	return 0;
}

/** find callback in list */
static struct service_callback*
callback_list_find(struct serviced_query* sq, void* cb_arg, 
	int (*arg_compare)(void*,void*))
{
	struct service_callback* p;
	for(p = sq->cblist; p; p = p->next) {
		if(arg_compare(p->cb_arg, cb_arg))
			return p;
	}
	return NULL;
}

struct serviced_query* 
outnet_serviced_query(struct outside_network* outnet,
	uint8_t* qname, size_t qnamelen, uint16_t qtype, uint16_t qclass,
	uint16_t flags, int dnssec, struct sockaddr_storage* addr,
	socklen_t addrlen, comm_point_callback_t* callback,
	void* callback_arg, ldns_buffer* buff, 
	int (*arg_compare)(void*,void*))
{
	struct serviced_query* sq;
	struct service_callback* cb;
	serviced_gen_query(buff, qname, qnamelen, qtype, qclass, flags);
	sq = lookup_serviced(outnet, buff, dnssec, addr, addrlen);
	if(sq) {
		/* see if it is a duplicate notification request for cb_arg */
		if(callback_list_find(sq, callback_arg, arg_compare)) {
			return sq;
		}
	}
	if(!(cb = (struct service_callback*)malloc(sizeof(*cb))))
		return NULL;
	if(!sq) {
		/* make new serviced query entry */
		sq = serviced_create(outnet, buff, dnssec, addr, addrlen);
		if(!sq) {
			free(cb);
			return NULL;
		}
		/* perform first network action */
		if(!serviced_udp_send(sq, buff)) {
			(void)rbtree_delete(outnet->serviced, sq);
			free(sq->qbuf);
			free(sq);
			free(cb);
			return NULL;
		}
	}
	/* add callback to list of callbacks */
	cb->cb = callback;
	cb->cb_arg = callback_arg;
	cb->next = sq->cblist;
	sq->cblist = cb;
	return sq;
}

/** remove callback from list */
static void
callback_list_remove(struct serviced_query* sq, void* cb_arg)
{
	struct service_callback** pp = &sq->cblist;
	while(*pp) {
		if((*pp)->cb_arg == cb_arg) {
			struct service_callback* del = *pp;
			*pp = del->next;
			free(del);
			return;
		}
		pp = &(*pp)->next;
	}
}

void outnet_serviced_query_stop(struct serviced_query* sq, void* cb_arg)
{
	if(!sq) 
		return;
	callback_list_remove(sq, cb_arg);
	/* if callbacks() routine scheduled deletion, let it do that */
	if(!sq->cblist && !sq->to_be_deleted) {
		rbnode_t* rem;
		rem = rbtree_delete(sq->outnet->serviced, sq);
		log_assert(rem); /* should be present */
		serviced_delete(sq); 
	}
}

/** get memory used by waiting tcp entry (in use or not) */
static size_t
waiting_tcp_get_mem(struct waiting_tcp* w)
{
	size_t s;
	if(!w) return 0;
	s = sizeof(*w) + w->pkt_len;
	if(w->timer)
		s += comm_timer_get_mem(w->timer);
	return s;
}

size_t outnet_get_mem(struct outside_network* outnet)
{
	size_t i;
	struct waiting_tcp* w;
	struct serviced_query* sq;
	struct service_callback* sb;
	size_t s = sizeof(*outnet) + sizeof(*outnet->base) + 
		sizeof(*outnet->udp_buff) + 
		ldns_buffer_capacity(outnet->udp_buff);
	/* second buffer is not ours */
	s += sizeof(struct comm_point*)*outnet->num_udp4;
	for(i=0; i<outnet->num_udp4; i++)
		s += comm_point_get_mem(outnet->udp4_ports[i]);
	s += sizeof(struct comm_point*)*outnet->num_udp6;
	for(i=0; i<outnet->num_udp6; i++)
		s += comm_point_get_mem(outnet->udp6_ports[i]);
	s += sizeof(struct pending_tcp*)*outnet->num_tcp;
	for(i=0; i<outnet->num_tcp; i++) {
		s += sizeof(struct pending_tcp);
		s += comm_point_get_mem(outnet->tcp_conns[i]->c);
		if(outnet->tcp_conns[i]->query)
			s += waiting_tcp_get_mem(outnet->tcp_conns[i]->query);
	}
	for(w=outnet->tcp_wait_first; w; w = w->next_waiting)
		s += waiting_tcp_get_mem(w);
	s += sizeof(*outnet->pending);
	s += (sizeof(struct pending) + comm_timer_get_mem(NULL)) * 
		outnet->pending->count;
	s += sizeof(*outnet->serviced);
	s += outnet->svcd_overhead;
	RBTREE_FOR(sq, struct serviced_query*, outnet->serviced) {
		s += sizeof(*sq) + sq->qbuflen;
		for(sb = sq->cblist; sb; sb = sb->next)
			s += sizeof(*sb);
	}
	return s;
}

size_t 
serviced_get_mem(struct serviced_query* sq)
{
	struct service_callback* sb;
	size_t s;
	s = sizeof(*sq) + sq->qbuflen;
	for(sb = sq->cblist; sb; sb = sb->next)
		s += sizeof(*sb);
	if(sq->status == serviced_query_UDP_EDNS ||
		sq->status == serviced_query_UDP) {
		s += sizeof(struct pending);
		s += comm_timer_get_mem(NULL);
	} else {
		/* does not have size of the pkt pointer */
		/* always has a timer except on malloc failures */

		/* these sizes are part of the main outside network mem */
		/*
		s += sizeof(struct waiting_tcp);
		s += comm_timer_get_mem(NULL);
		*/
	}
	return s;
}
