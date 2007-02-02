/*
 * services/listen_dnsport.c - listen on port 53 for incoming DNS queries.
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

#include "services/listen_dnsport.h"
#include "util/netevent.h"
#include "util/log.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

/** number of queued TCP connections for listen() */
#define TCP_BACKLOG 5 
/** number of simultaneous open TCP connections */
#define TCP_COUNT 10 

/**
 * Debug print of the getaddrinfo returned address.
 * @param addr: the address returned.
 */
static void
verbose_print_addr(struct addrinfo *addr)
{
	if(verbosity >= VERB_ALGO) {
		char buf[100];
		if(inet_ntop(addr->ai_family, 
			&((struct sockaddr_in*)addr->ai_addr)->sin_addr, buf, 
			(socklen_t)sizeof(buf)) == 0) {
			strncpy(buf, "(null)", sizeof(buf));
		}
		verbose(VERB_ALGO, "creating %s %s socket %s %d", 
			addr->ai_family==AF_INET?"inet":
			addr->ai_family==AF_INET6?"inet6":"otherfamily", 
			addr->ai_socktype==SOCK_DGRAM?"udp":
			addr->ai_socktype==SOCK_STREAM?"tcp":"otherprotocol",
			buf, 
			ntohs(((struct sockaddr_in*)addr->ai_addr)->sin_port));
	}
}

int
create_udp_sock(struct addrinfo *addr)
{
	int s, flag;
	int on=1;
	verbose_print_addr(addr);
	if((s = socket(addr->ai_family, addr->ai_socktype, 0)) == -1) {
		log_err("can't create socket: %s", strerror(errno));
		return -1;
	}
	if(addr->ai_family == AF_INET6) {
# if defined(IPV6_V6ONLY)
		if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, 
			&on, (socklen_t)sizeof(on)) < 0) {
			log_err("setsockopt(..., IPV6_V6ONLY, ...) failed: %s",
				strerror(errno));
			return -1;
		}
# endif
# if defined(IPV6_USE_MIN_MTU)
		/*
		 * There is no fragmentation of IPv6 datagrams
		 * during forwarding in the network. Therefore
		 * we do not send UDP datagrams larger than
		 * the minimum IPv6 MTU of 1280 octets. The
		 * EDNS0 message length can be larger if the
		 * network stack supports IPV6_USE_MIN_MTU.
		 */
		if (setsockopt(s, IPPROTO_IPV6, IPV6_USE_MIN_MTU,
			&on, (socklen_t)sizeof(on)) < 0) {
			log_err("setsockopt(..., IPV6_USE_MIN_MTU, "
				"...) failed: %s", strerror(errno));
			return -1;
		}
# endif
	}
	if(bind(s, (struct sockaddr*)addr->ai_addr, addr->ai_addrlen) != 0) {
		log_err("can't bind socket: %s", strerror(errno));
		return -1;
	}
	if((flag = fcntl(s, F_GETFL)) == -1) {
		log_err("can't fcntl F_GETFL: %s", strerror(errno));
		flag = 0;
	}
	flag |= O_NONBLOCK;
	if(fcntl(s, F_SETFL, flag) == -1) {
		log_err("can't fcntl F_SETFL: %s", strerror(errno));
		return -1;
	}
	return s;
}

/**
 * Create and bind TCP listening socket
 * @param addr: address info ready to make socket.
 * @return: the socket. -1 on error.
 */
static int
create_tcp_accept_sock(struct addrinfo *addr)
{
	int s, flag;
#if defined(SO_REUSEADDR) || defined(IPV6_V6ONLY)
	int on = 1;
#endif /* SO_REUSEADDR || IPV6_V6ONLY */
	verbose_print_addr(addr);
	if((s = socket(addr->ai_family, addr->ai_socktype, 0)) == -1) {
		log_err("can't create socket: %s", strerror(errno));
		return -1;
	}
#ifdef SO_REUSEADDR
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, 
		(socklen_t)sizeof(on)) < 0) {
		log_err("setsockopt(.. SO_REUSEADDR ..) failed: %s",
			strerror(errno));
		return -1;
	}
#endif /* SO_REUSEADDR */
#if defined(IPV6_V6ONLY)
	if(addr->ai_family == AF_INET6) {
		if(setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, 
			&on, (socklen_t)sizeof(on)) < 0) {
			log_err("setsockopt(..., IPV6_V6ONLY, ...) failed: %s",
				strerror(errno));
			return -1;
		}
	}
#endif /* IPV6_V6ONLY */
	if(bind(s, (struct sockaddr*)addr->ai_addr, addr->ai_addrlen) != 0) {
		log_err("can't bind socket: %s", strerror(errno));
		return -1;
	}
	if((flag = fcntl(s, F_GETFL)) == -1) {
		log_err("can't fcntl F_GETFL: %s", strerror(errno));
		flag = 0;
	}
	flag |= O_NONBLOCK;
	if(fcntl(s, F_SETFL, flag) == -1) {
		log_err("can't fcntl F_SETFL: %s", strerror(errno));
		return -1;
	}
	if(listen(s, TCP_BACKLOG) == -1) {
		log_err("can't listen: %s", strerror(errno));
		return -1;
	}
	return s;
}

/**
 * Create socket from getaddrinfo results
 */
static int
make_sock(int stype, const char* ifname, const char* port, 
	struct addrinfo *hints)
{
	struct addrinfo *res = NULL;
	int r, s;
	hints->ai_socktype = stype;
	if((r=getaddrinfo(ifname, port, hints, &res)) != 0 || !res) {
		log_err("node %s:%s getaddrinfo: %s %s", 
			ifname?ifname:"default", port, gai_strerror(r),
			r==EAI_SYSTEM?strerror(errno):"");
		return -1;
	}
	if(stype == SOCK_DGRAM)
		s = create_udp_sock(res);
	else	s = create_tcp_accept_sock(res);
	freeaddrinfo(res);
	return s;
}

/**
 * Helper for listen_create. Creates one interface (or NULL for default).
 * @param ifname: The interface ip address.
 * @param front: The the listening info.
 * @param base: Event base.
 * @param port: Port number to use (as string).
 * @param do_udp: if udp should be used.
 * @param do_tcp: if udp should be used.
 * @param hints: for getaddrinfo. family and flags have to be set by caller.
 * @param bufsize: TCP buffer size.
 * @param cb: callback function
 * @param cb_arg: user parameter for callback function.
 * @return: returns false on error.
 */
static int
listen_create_if(const char* ifname, struct listen_dnsport* front, 
	struct comm_base* base, const char* port, int do_udp, int do_tcp, 
	struct addrinfo *hints, size_t bufsize, comm_point_callback_t* cb, 
	void *cb_arg)
{
	struct comm_point *cp_udp = NULL, *cp_tcp = NULL;
	struct listen_list *el_udp, *el_tcp;
	int s;
	if(!do_udp && !do_tcp)
		return 0;
	if(do_udp) {
		if((s = make_sock(SOCK_DGRAM, ifname, port, hints)) == -1)
			return 0;
		cp_udp = comm_point_create_udp(base, s, front->udp_buff, 
			cb, cb_arg);
		if(!cp_udp) {
			log_err("can't create commpoint");	
			close(s);
			return 0;
		}
	}
	if(do_tcp) {
		if((s = make_sock(SOCK_STREAM, ifname, port, hints)) == -1) {
			comm_point_delete(cp_udp);
			return 0;
		}
		cp_tcp = comm_point_create_tcp(base, s, TCP_COUNT, bufsize, 
			cb, cb_arg);
		if(!cp_tcp) {
			log_err("can't create commpoint");	
			comm_point_delete(cp_udp);
			return 0;
		}
	}
	/* add commpoints to the listen structure */
	el_udp = (struct listen_list*)malloc(sizeof(struct listen_list));
	if(!el_udp) {
		log_err("out of memory");
		comm_point_delete(cp_udp);
		comm_point_delete(cp_tcp);
		return 0;
	}
	el_tcp = (struct listen_list*)malloc(sizeof(struct listen_list));
	if(!el_tcp) {
		log_err("out of memory");
		free(el_udp);
		comm_point_delete(cp_udp);
		comm_point_delete(cp_tcp);
		return 0;
	}
	el_udp->com = cp_udp;
	el_udp->next = front->cps;
	front->cps = el_udp;
	el_tcp->com = cp_tcp;
	el_tcp->next = front->cps;
	front->cps = el_tcp;
	return 1;
}

struct listen_dnsport* 
listen_create(struct comm_base* base, int num_ifs, const char* ifs[], 
	const char* port, int do_ip4, int do_ip6, int do_udp, int do_tcp,
	size_t bufsize, comm_point_callback_t* cb, void *cb_arg)
{
	struct addrinfo hints;
	int i;
	struct listen_dnsport* front = (struct listen_dnsport*)
		malloc(sizeof(struct listen_dnsport));
	if(!front)
		return NULL;
	front->cps = NULL;
	front->udp_buff = ldns_buffer_new(bufsize);
	if(!front->udp_buff) {
		free(front);
		return NULL;
	}
	
	/* getaddrinfo */
	memset(&hints, 0, sizeof(hints));

	hints.ai_flags = AI_PASSIVE;
	/* no name lookups on our listening ports */
	if(num_ifs > 0)
		hints.ai_flags |= AI_NUMERICHOST;

	hints.ai_family = AF_UNSPEC;
	if(!do_ip4 && !do_ip6) {
		listen_delete(front);
		return NULL;
	} else if(do_ip4 && do_ip6)
		hints.ai_family = AF_UNSPEC;
	else if(do_ip4)
		hints.ai_family = AF_INET;
	else if(do_ip6) {
		hints.ai_family = AF_INET6;
	}

	if(num_ifs == 0) {
		if(!listen_create_if(NULL, front, base, port, 
			do_udp, do_tcp, &hints, bufsize, cb, cb_arg)) {
			listen_delete(front);
			return NULL;
		}
	} else for(i = 0; i<num_ifs; i++) {
		if(!listen_create_if(ifs[i], front, base, port, 
			do_udp, do_tcp, &hints, bufsize, cb, cb_arg)) {
			listen_delete(front);
			return NULL;
		}
	}

	return front;
}

void 
listen_delete(struct listen_dnsport* front)
{
	struct listen_list *p, *pn;
	if(!front) 
		return;
	p = front->cps;
	while(p) {
		pn = p->next;
		comm_point_delete(p->com);
		free(p);
		p = pn;
	}
	ldns_buffer_free(front->udp_buff);
	free(front);
}

