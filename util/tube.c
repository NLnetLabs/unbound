/*
 * util/tube.c - pipe service
 *
 * Copyright (c) 2008, NLnet Labs. All rights reserved.
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
 * This file contains pipe service functions.
 */
#include "config.h"
#include "util/tube.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/netevent.h"
#include "util/fptr_wlist.h"

struct tube* tube_create(void)
{
	struct tube* tube = (struct tube*)calloc(1, sizeof(*tube));
	int sv[2];
	if(!tube) return 0;
	tube->sr = -1;
	tube->sw = -1;
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
		log_err("socketpair: %s", strerror(errno));
		free(tube);
		return NULL;
	}
	tube->sr = sv[0];
	tube->sw = sv[1];
	if(!fd_set_nonblock(tube->sr) || !fd_set_nonblock(tube->sw)) {
		log_err("tube: cannot set nonblocking");
		tube_delete(tube);
		return NULL;
	}
	return tube;
}

void tube_delete(struct tube* tube)
{
	if(!tube) return;
	if(tube->listen_com) {
		comm_point_delete(tube->listen_com);
	}
	/* close fds after deleting commpoints, to be sure.
	 *            Also epoll does not like closing fd before event_del */
	if(tube->sr != -1) close(tube->sr);
	if(tube->sw != -1) close(tube->sw);
	tube->sr = -1;
	tube->sw = -1;
	free(tube);
}

int
tube_handle_listen(struct comm_point* c, void* arg, int error,
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	struct tube* tube = (struct tube*)arg;
	if(error != NETEVENT_NOERROR) {
		fptr_ok(fptr_whitelist_tube_listen(tube->listen_cb));
		(*tube->listen_cb)(tube, NULL, error, tube->listen_arg);
		return 0;
	}
	fptr_ok(fptr_whitelist_tube_listen(tube->listen_cb));
	(*tube->listen_cb)(tube, c->buffer, error, tube->listen_arg);
	return 0;
}

int tube_listen_cmd(struct tube* tube, struct comm_base* base,
        size_t msg_buffer_sz, tube_callback_t* cb, void* arg)
{
	tube->listen_cb = cb;
	tube->listen_arg = arg;
	if(!(tube->listen_com = comm_point_create_local(base, tube->sr, 
		msg_buffer_sz, tube_handle_listen, tube))) {
		log_err("tube_listen_cmd: commpoint creation failed");
		return 0;
	}
	return 1;
}

int tube_send_cmd(struct tube* tube, ldns_buffer* buffer)
{
	if(!write_socket(tube->sw, ldns_buffer_begin(buffer),
		ldns_buffer_limit(buffer))) {
		log_err("write socket: %s", strerror(errno));
		return 0;
	}
	return 1;
}
