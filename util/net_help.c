/*
 * util/net_help.c - implementation of the network helper code
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
 * Implementation of net_help.h.
 */

#include "config.h"
#include "util/net_help.h"
#include "util/log.h"
#include <fcntl.h>

/** returns true is string addr is an ip6 specced address. */
int
str_is_ip6(const char* str)
{
	if(strchr(str, ':'))
		return 1;
	else    return 0;
}

int 
write_socket(int s, const void *buf, size_t size)
{
        const char* data = (const char*)buf;
        size_t total_count = 0;

        while (total_count < size) {
                ssize_t count
                        = write(s, data + total_count, size - total_count);
                if (count == -1) {
                        if (errno != EAGAIN && errno != EINTR) {
                                return 0;
                        } else {
                                continue;
                        }
                }
                total_count += count;
        }
        return 1;
}

int 
fd_set_nonblock(int s) 
{
	int flag;
	if((flag = fcntl(s, F_GETFL)) == -1) {
		log_err("can't fcntl F_GETFL: %s", strerror(errno));
		flag = 0;
	}
	flag |= O_NONBLOCK;
	if(fcntl(s, F_SETFL, flag) == -1) {
		log_err("can't fcntl F_SETFL: %s", strerror(errno));
		return 0;
	}
	return 1;
}

int 
is_pow2(size_t num)
{
	if(num == 0) return 1;
	return (num & (num-1)) == 0;
}

void 
write_iov_buffer(ldns_buffer* buffer, struct iovec* iov, size_t iovlen)
{
	size_t i;
	size_t s = 0;
	ldns_buffer_clear(buffer);
	for(i=0; i<iovlen; i++) {
		log_assert(ldns_buffer_position(buffer)+iov[i].iov_len
			<= ldns_buffer_capacity(buffer));
		s += iov[i].iov_len;
		ldns_buffer_write(buffer, iov[i].iov_base, iov[i].iov_len);
	}
	ldns_buffer_flip(buffer);
}

void* 
memdup(void* data, size_t len)
{
	void* d;
	if(!data) return NULL;
	if(len == 0) return NULL;
	d = malloc(len);
	if(!d) return NULL;
	memcpy(d, data, len);
	return d;
}

void
log_addr(const char* str, struct sockaddr_storage* addr, socklen_t addrlen)
{
        uint16_t port;
        const char* family = "unknown";
        char dest[100];
        int af = (int)((struct sockaddr_in*)addr)->sin_family;
        void* sinaddr = &((struct sockaddr_in*)addr)->sin_addr;
        switch(af) {
                case AF_INET: family="ip4"; break;
                case AF_INET6: family="ip6";
                        sinaddr = &((struct sockaddr_in6*)addr)->sin6_addr;
                        break;
                case AF_UNIX: family="unix"; break;
                default: break;
        }
        if(inet_ntop(af, sinaddr, dest, (socklen_t)sizeof(dest)) == 0) {
                strncpy(dest, "(inet_ntop error)", sizeof(dest));
        }
        port = ntohs(((struct sockaddr_in*)addr)->sin_port);
        verbose(VERB_DETAIL, "%s %s %s %d (len %d)",
                str, family, dest, (int)port, (int)addrlen);
}

int 
ipstrtoaddr(const char* ip, int port, struct sockaddr_storage* addr,
	socklen_t* addrlen)
{
	uint16_t p;
	if(!ip) return 0;
	p = (uint16_t) port;
	if(str_is_ip6(ip)) {
		struct sockaddr_in6* sa = (struct sockaddr_in6*)addr;
		*addrlen = (socklen_t)sizeof(struct sockaddr_in6);
		memset(sa, 0, *addrlen);
		sa->sin6_family = AF_INET6;
		sa->sin6_port = (in_port_t)htons(p);
		if(inet_pton((int)sa->sin6_family, ip, &sa->sin6_addr) <= 0) {
			log_err("Bad ip6 address %s", ip);
			return 0;
		}
	} else { /* ip4 */
		struct sockaddr_in* sa = (struct sockaddr_in*)addr;
		*addrlen = (socklen_t)sizeof(struct sockaddr_in);
		memset(sa, 0, *addrlen);
		sa->sin_family = AF_INET;
		sa->sin_port = (in_port_t)htons(p);
		if(inet_pton((int)sa->sin_family, ip, &sa->sin_addr) <= 0) {
			log_err("Bad ip4 address %s", ip);
			return 0;
		}
	}
	return 1;
}
