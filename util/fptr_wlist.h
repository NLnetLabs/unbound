/*
 * util/fptr_wlist.h - function pointer whitelists.
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
 * This file contains functions that check function pointers.
 * The functions contain a whitelist of known good callback values.
 * Any other values lead to an error. 
 * 
 * This prevent heap overflow based exploits, where the callback pointer
 * is overwritten by a buffer overflow (apart from this defense, buffer 
 * overflows should be fixed of course).
 *
 * Function pointers are used in
 * 	o network code callbacks.
 * 	o rbtree, lruhash, region data manipulation
 * 	o module operations.
 */

#ifndef UTIL_FPTR_WLIST_H
#define UTIL_FPTR_WLIST_H
#include "util/netevent.h"

/**
 * Check function pointer whitelist for comm_point callback values.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_comm_point(comm_point_callback_t *fptr);

/**
 * Check function pointer whitelist for comm_timer callback values.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_comm_timer(void (*fptr)(void*));

/**
 * Check function pointer whitelist for comm_signal callback values.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_comm_signal(void (*fptr)(int, void*));

/**
 * Check function pointer whitelist for event structure callback values.
 * This is not called by libevent itself, but checked by netevent.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_event(void (*fptr)(int, short, void *));

/**
 * Check function pointer whitelist for pending udp callback values.
 * This is not called by libevent itself, but checked by netevent.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_pending_udp(comm_point_callback_t *fptr);

/**
 * Check function pointer whitelist for pending tcp callback values.
 * This is not called by libevent itself, but checked by netevent.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_pending_tcp(comm_point_callback_t *fptr);

/**
 * Check function pointer whitelist for serviced query callback values.
 * This is not called by libevent itself, but checked by netevent.
 *
 * @param fptr: function pointer to check.
 * @return false if not in whitelist.
 */
int fptr_whitelist_serviced_query(comm_point_callback_t *fptr);


#endif /* UTIL_FPTR_WLIST_H */
