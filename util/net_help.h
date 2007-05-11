/*
 * util/net_help.h - network help functions 
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
 * This file contains functions to perform network related tasks.
 */

#ifndef NET_HELP_H
#define NET_HELP_H

/** DNS constants for uint16_t style flag manipulation. host byteorder. */
/** AA flag */
#define BIT_AA 0x0400
/** RD flag */
#define BIT_RD 0x0100
/** CD flag */
#define BIT_CD 0x0010
/** QR flag */
#define BIT_QR 0x8000

/** timeout in seconds for UDP queries to auth servers. TODO: proper rtt */
#define UDP_QUERY_TIMEOUT 4
/** timeout in seconds for TCP queries to auth servers. TODO: proper rtt */
#define TCP_QUERY_TIMEOUT 30
/** Advertised version of EDNS capabilities */
#define EDNS_ADVERTISED_VERSION         0
/** Advertised size of EDNS capabilities */
#define EDNS_ADVERTISED_SIZE    4096

/**
 * See if string is ip4 or ip6.
 * @param str: IP specification.
 * @return: true if string addr is an ip6 specced address.
 */
int str_is_ip6(const char* str);

/**
 * Write (blocking) to a nonblocking socket.
 * @param s: fd.
 * @param buf: data buffer.
 * @param size: length of data to send.
 * @return: 0 on error. errno is set.
 */
int
write_socket(int s, const void *buf, size_t size);

/**
 * Set fd nonblocking.
 * @param s: file descriptor.
 * @return: 0 on error (error is printed to log).
 */
int fd_set_nonblock(int s); 

/**
 * See if number is a power of 2.
 * @param num: the value.
 * @return: true if the number is a power of 2.
 */
int is_pow2(size_t num);

/**
 * Write iov vector into a buffer.
 * @param buffer: to write data into. 
 * @param iov: iov vector
 * @param iovlen: length of iov vector
 */
void write_iov_buffer(ldns_buffer* buffer, struct iovec* iov, size_t iovlen);

#endif /* NET_HELP_H */
