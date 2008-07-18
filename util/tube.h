/*
 * util/tube.h - pipe service
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

#ifndef UTIL_TUBE_H
#define UTIL_TUBE_H
struct comm_reply;
struct comm_base;
struct tube;

/**
 * Callback from pipe listen function
 * void mycallback(tube, buffer, error, argument);
 * if error is true (NETEVENT_*), buffer is probably NULL.
 */
typedef void (tube_callback_t)(struct tube* tube, ldns_buffer* buffer,
	int error, void* arg);

/**
 * A pipe
 */
struct tube {
	/** pipe end to read from */
	int sr;
	/** pipe end to write on */
	int sw;

	/** listen commpoint */
	struct comm_point* listen_com;
	/** listen callback */
	tube_callback_t* listen_cb;
	/** listen callback user arg */
	void* listen_arg;
};

/**
 * Create a pipe
 * @return: new tube struct or NULL on error.
 */
struct tube* tube_create(void);

/**
 * Delete and destroy a pipe
 * @param tube: to delete
 */
void tube_delete(struct tube* tube);

/**
 * Start listening for information over the pipe
 * @param tube: tube to listen on
 * @param base: what base to register event callback.
 * @param msg_buffer_sz: what message buffer size to use.
 * @param cb: callback routine.
 * @param arg: user argument for callback routine.
 * @return true if successful, false on error.
 */
int tube_listen_cmd(struct tube* tube, struct comm_base* base, 
	size_t msg_buffer_sz, tube_callback_t* cb, void* arg);

/**
 * Send a command over a pipe, blocking operation.
 * @param tube: tube to send the info on.
 * @param buffer: buffer to send. starts with network order uint16 with 
 *	length of remainder of buffer.
 *	The receiver does not receive the length uint16 in the buffer
 *	(the buffer is sized appropriately).
 * @return 0 on error, true on success.
 */
int tube_send_cmd(struct tube* tube, ldns_buffer* buffer);

/** decl for fptr_wlist of tube pipe listen handler */
int tube_handle_listen(struct comm_point* c, void* arg, int error, 
	struct comm_reply* reply_info);

#endif /* UTIL_TUBE_H */
