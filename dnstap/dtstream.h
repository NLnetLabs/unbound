/*
 * dnstap/dtstream.h - Frame Streams implementation for unbound DNSTAP
 *
 * Copyright (c) 2020, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file
 *
 * An implementation of the Frame Streams data transport protocol for
 * the Unbound DNSTAP message logging facility.
 */

#ifndef DTSTREAM_H
#define DTSTREAM_H

#include "util/locks.h"
struct dt_msg_entry;
struct dt_io_list_item;
struct config_file;

/**
 * A message buffer with dnstap messages queued up.  It is per-worker.
 * It has locks to synchronize.  If the buffer is full, a new message
 * cannot be added and is discarded.  A thread reads the messages and sends
 * them.
 */
struct dt_msg_queue {
	/** lock of the buffer structure.  Hold this lock to add or remove
	 * entries to the buffer.  Release it so that other threads can also
	 * put messages to log, or a message can be taken out to send away
	 * by the writer thread.
	 */
	lock_basic_type lock;
	/** the maximum size of the buffer, in bytes */
	size_t maxsize;
	/** current size of the buffer, in bytes.  data bytes of messages.
	 * If a new message make it more than maxsize, the buffer is full */
	size_t cursize;
	/** list of messages.  The messages are added to the back and taken
	 * out from the front. */
	struct dt_msg_entry* first, *last;
};

/**
 * An entry in the dt_msg_queue. contains one DNSTAP message.
 * It is malloced.
 */
struct dt_msg_entry {
	/** next in the list. */
	struct dt_msg_entry* next;
	/** the buffer with the data to send, an encoded DNSTAP message */
	void* buf;
	/** the length to send. */
	size_t len;
};

/**
 * IO thread that reads from the queues and writes them.
 */
struct dt_io_thread {
	/** event base, for event handling */
	void* event_base;
	/** list of queues that is registered to get written */
	struct dt_io_list_item* io_list;
	/** iterator point in the io_list, to pick from them in a
	 * round-robin fashion, instead of only from the first when busy.
	 * if NULL it means start at the start of the list. */
	struct dt_io_list_item* io_list_iter;
	/** thread id, of the io thread */
	ub_thread_type tid;
	/** file descriptor that the thread writes to */
	int fd;
	/** event structure that the thread uses */
	void* event;
	/** the event is added */
	int event_added;
	/** check for nonblocking connect errors on fd */
	int check_nb_connect;
	/** the buffer that currently getting written, or NULL if no
	 * (partial) message written now */
	void* cur_msg;
	/** length of the current message */
	size_t cur_msg_len;
	/** number of bytes written for the current message */
	size_t cur_msg_done;
	/** number of bytes of the length that have been written,
	 * for the current message length that precedes the frame */
	size_t cur_msg_len_done;

	/** command pipe that stops the pipe if closed.  Used to quit
	 * the program. [0] is read, [1] is written to. */
	int commandpipe[2];
	/** the event to listen to the commandpipe */
	void* command_event;
	/** the io thread wants to exit */
	int want_to_exit;

	/** If the log server is connected to over unix domain sockets,
	 * eg. a file is named that is created to log onto. */
	int upstream_is_unix;
	/** if the log server is connected to over TCP.  The ip address and
	 * port are used */
	int upstream_is_tcp;
	/** if the log server is connected to over TLS.  ip address, port,
	 * and client certificates can be used for authentication. */
	int upstream_is_tls;

	/** the file path for unix socket (or NULL) */
	char* socket_path;
	/** the ip address and port number (or NULL) */
	char* ip_str;
	/** is the TLS upstream authenticated by name, if nonNULL,
	 * we use the same cert bundle as used by other TLS streams. */
	char* tls_server_name;
	/** are client certificates in use */
	int use_client_certs;
	/** client cert files: the .key file */
	char* client_key_file;
	/** client cert files: the .pem file */
	char* client_cert_file;
};

/**
 * IO thread list of queues list item
 * lists a worker queue that should be looked at and sent to the log server.
 */
struct dt_io_list_item {
	/** next in the list of buffers to inspect */
	struct dt_io_list_item* next;
	/** buffer of this worker */
	struct dt_msg_queue* queue;
};

/* Frame Streams data transfer protocol encode for DNSTAP messages.
 * The protocol looks to be specified in the libfstrm library.
 *
 * Quick writeup for DNSTAP usage, from reading fstrm/control.h eloquent
 * comments and fstrm/control.c for some bytesize details (the content type
 * length).
 *
 * The Frame Streams can be unidirectional or bi-directional.
 * bi-directional streams use control frame types READY, ACCEPT and FINISH.
 * uni-directional streams use control frame types START and STOP.
 * unknown control frame types should be ignored by the receiver, they
 * do not change the data frame encoding.
 *
 * bi-directional control frames implement a simple handshake protocol
 * between sender and receiver.
 *
 * The uni-directional control frames have one start and one stop frame,
 * before and after the data.  The start frame can have a content type.
 * The start and stop frames are not optional.
 *
 * data frames are preceded by 4byte length, bigendian.
 * zero length data frames are not possible, they are an escape that
 * signals the presence of a control frame.
 *
 * a control frame consists of 0 value in 4byte bigendian, this is really
 * the data frame length, with 0 the escape sequence that indicates one
 * control frame follows.
 * Then, 4byte bigendian, length of the control frame message.
 * Then, the control frame payload (of that length). with in it:
 *   4byte bigendian, control type (eg. START, STOP, READY, ACCEPT, FINISH).
 *   perhaps nothing more (STOP, FINISH), but for other types maybe
 *   control fields
 *      4byte bigendian, the control-field-type, currently only content-type.
 *      4byte bigendian, length of the string for this option.
 *      .. bytes of that string.
 *
 * The START type can have only one field.  Field max len 256.
 * control frame max frame length 512 (excludes the 0-escape and control
 * frame length bytes).
 *
 * the bidirectional type of transmission is like this:
 * client sends READY (with content type included),
 * client waits for ACCEPT (with content type included),
 * client sends START (with matched content type from ACCEPT)
 * .. data frames
 * client sends STOP.
 * client waits for FINISH frame.
 *
 */

/** max length of Frame Streams content type field string */
#define FSTRM_CONTENT_TYPE_LENGTH_MAX 256
/** control frame value to denote the control frame ACCEPT */
#define FSTRM_CONTROL_FRAME_ACCEPT 0x01
/** control frame value to denote the control frame START */
#define FSTRM_CONTROL_FRAME_START 0x02
/** control frame value to denote the control frame STOP */
#define FSTRM_CONTROL_FRAME_STOP 0x03
/** control frame value to denote the control frame READY */
#define FSTRM_CONTROL_FRAME_READY 0x04
/** control frame value to denote the control frame FINISH */
#define FSTRM_CONTROL_FRAME_FINISH 0x05
/** the constant that denotes the control field type that is the
 * string for the content type of the stream. */
#define FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE 0x01
/** the content type for DNSTAP frame streams */
#define DNSTAP_CONTENT_TYPE             "protobuf:dnstap.Dnstap"

/**
 * This creates an FSTRM control frame of type START.
 * @param contenttype: a zero delimited string with the content type.
 * 	eg. use the constant DNSTAP_CONTENT_TYPE, which is defined as
 * 	"protobuf:dnstap.Dnstap", for a dnstap frame stream.
 * @param len: if a buffer is returned this is the length of that buffer.
 * @return NULL on malloc failure.  Returns a malloced buffer with the
 * protocol message.  The buffer starts with the 4 bytes of 0 that indicate
 * a control frame.  The buffer should be sent without preceding it with
 * the 'len' variable (like data frames are), but straight the content of the
 * buffer, because the lengths are included in the buffer.  This is so that
 * the zero control indicator can be included before the control frame length.
 */
void* fstrm_create_control_frame_start(char* contenttype, size_t* len);
/**
 * This creates an FSTRM control frame of type STOP.
 * @param len: if a buffer is returned this is the length of that buffer.
 * @return NULL on malloc failure.  Returns a malloced buffer with the
 * protocol message.  The buffer starts with the 4 bytes of 0 that indicate
 * a control frame.  The buffer should be sent without preceding it with
 * the 'len' variable (like data frames are), but straight the content of the
 * buffer, because the lengths are included in the buffer.  This is so that
 * the zero control indicator can be included before the control frame length.
 */
void* fstrm_create_control_frame_stop(size_t* len);

/**
 * Create new (empty) worker message queue. Limit set to default on max.
 * @return NULL on malloc failure or a new queue (not locked).
 */
struct dt_msg_queue* dt_msg_queue_create(void);

/**
 * Delete a worker message queue.  It has to be unlinked from access,
 * so it can be deleted without lock worries.  The queue is emptied (deleted).
 * @param mq: message queue.
 */
void dt_msg_queue_delete(struct dt_msg_queue* mq);

/**
 * Submit a message to the queue.  The queue is locked by the routine,
 * the message is inserted, and then the queue is unlocked so the
 * message can be picked up by the writer thread.
 * @param mq: message queue.
 * @param buf: buffer with message (dnstap contents).
 * 	The buffer must have been malloced by caller.  It is linked in
 * 	the queue, and is free()d after use.  If the routine fails
 * 	the buffer is freed as well (and nothing happens, the item
 * 	could not be logged).
 * @param len: length of buffer.
 */
void dt_msg_queue_submit(struct dt_msg_queue* mq, void* buf, size_t len);

/**
 * Create IO thread.
 * @return new io thread object. not yet started. or NULL malloc failure.
 */
struct dt_io_thread* dt_io_thread_create(void);

/**
 * Delete the IO thread structure.
 * @param dtio: the io thread that is deleted.  It must not be running.
 */
void dt_io_thread_delete(struct dt_io_thread* dtio);

/**
 * Apply config to the dtio thread
 * @param dtio: io thread, not yet started.
 * @param cfg: config file struct.
 */
void dt_io_thread_apply_cfg(struct dt_io_thread* dtio,
	struct config_file *cfg);

/**
 * Register a msg queue to the io thread.  It will be polled to see if
 * there are messages and those then get removed and sent, when the thread
 * is running.
 * @param dtio: the io thread.
 * @param mq: message queue to register.
 * @return false on failure (malloc failure).
 */
int dt_io_thread_register_queue(struct dt_io_thread* dtio,
	struct dt_msg_queue* mq);

/**
 * Unregister queue from io thread.
 * @param dtio: the io thread.
 * @param mq: message queue.
 */
void dt_io_thread_unregister_queue(struct dt_io_thread* dtio,
        struct dt_msg_queue* mq);

/**
 * Start the io thread
 * @param dtio: the io thread.
 * @return false on failure.
 */
int dt_io_thread_start(struct dt_io_thread* dtio);

/** 
 * Stop the io thread
 * @param dtio: the io thread.
 */
void dt_io_thread_stop(struct dt_io_thread* dtio);

#endif /* DTSTREAM_H */
