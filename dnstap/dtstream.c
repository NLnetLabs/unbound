/*
 * dnstap/dtstream.c - Frame Streams implementation for unbound DNSTAP
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

#include "config.h"
#include "dnstap/dtstream.h"
#include "util/config_file.h"
#include "util/ub_event.h"
#include "util/net_help.h"
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#include <fcntl.h>

/** number of messages to process in one output callback */
#define DTIO_MESSAGES_PER_CALLBACK 100
/** the msec to wait for reconnect (if not immediate, the first attempt) */
#define DTIO_RECONNECT_TIMEOUT_MIN 10
/** the msec to wait for reconnect max after backoff */
#define DTIO_RECONNECT_TIMEOUT_MAX 1000

/** DTIO command channel commands */
enum {
	/** DTIO command channel stop */
	DTIO_COMMAND_STOP = 0,
	/** DTIO command channel wakeup */
	DTIO_COMMAND_WAKEUP = 1
} dtio_channel_command;

/** open the output channel */
static void dtio_open_output(struct dt_io_thread* dtio);
/** add output event for read and write */
static void dtio_add_output_event_write(struct dt_io_thread* dtio);
/** start reconnection attempts */
static void dtio_reconnect_enable(struct dt_io_thread* dtio);

void* fstrm_create_control_frame_start(char* contenttype, size_t* len)
{
	uint32_t* control;
	size_t n;
	/* start framestream message:
	 * 4byte 0: control indicator.
	 * 4byte bigendian: length of control frame
	 * 4byte bigendian: type START
	 * 4byte bigendian: frame option: content-type
	 * 4byte bigendian: length of string
	 * string of content type (dnstap)
	 */
	n = 4+4+4+4+4+strlen(contenttype);
	control = malloc(n);
	if(!control)
		return NULL;
	control[0] = 0;
	control[1] = htonl(4+4+4+strlen(contenttype));
	control[2] = htonl(FSTRM_CONTROL_FRAME_START);
	control[3] = htonl(FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE);
	control[4] = htonl(strlen(contenttype));
	memmove(&control[5], contenttype, strlen(contenttype));
	*len = n;
	return control;
}

void* fstrm_create_control_frame_stop(size_t* len)
{
	uint32_t* control;
	size_t n;
	/* stop framestream message:
	 * 4byte 0: control indicator.
	 * 4byte bigendian: length of control frame
	 * 4byte bigendian: type STOP
	 */
	n = 4+4+4;
	control = malloc(n);
	if(!control)
		return NULL;
	control[0] = 0;
	control[1] = htonl(4);
	control[2] = htonl(FSTRM_CONTROL_FRAME_STOP);
	*len = n;
	return control;
}

struct dt_msg_queue*
dt_msg_queue_create(void)
{
	struct dt_msg_queue* mq = calloc(1, sizeof(*mq));
	if(!mq) return NULL;
	mq->maxsize = 1*1024*1024; /* set max size of buffer, per worker,
		about 1 M should contain 64K messages with some overhead,
		or a whole bunch smaller ones */
	lock_basic_init(&mq->lock);
	lock_protect(&mq->lock, mq, sizeof(*mq));
	return mq;
}

/** clear the message list, caller must hold the lock */
static void
dt_msg_queue_clear(struct dt_msg_queue* mq)
{
	struct dt_msg_entry* e = mq->first, *next=NULL;
	while(e) {
		next = e->next;
		free(e->buf);
		free(e);
		e = next;
	}
	mq->first = NULL;
	mq->last = NULL;
	mq->cursize = 0;
}

void
dt_msg_queue_delete(struct dt_msg_queue* mq)
{
	if(!mq) return;
	lock_basic_destroy(&mq->lock);
	dt_msg_queue_clear(mq);
	free(mq);
}

/** make the dtio wake up by sending a wakeup command */
static void dtio_wakeup(struct dt_io_thread* dtio)
{
	uint8_t cmd = DTIO_COMMAND_WAKEUP;
	if(!dtio) return;
	if(!dtio->started) return;

	while(1) {
		ssize_t r = write(dtio->commandpipe[1], &cmd, sizeof(cmd));
		if(r == -1) {
#ifndef USE_WINSOCK
			if(errno == EINTR || errno == EAGAIN)
				continue;
			log_err("dnstap io wakeup: write: %s", strerror(errno));
#else
			if(WSAGetLastError() == WSAEINPROGRESS)
				continue;
			if(WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			log_err("dnstap io stop: write: %s",
				wsa_strerror(WSAGetLastError()));
#endif
			break;
		}
		break;
	}
}

void
dt_msg_queue_submit(struct dt_msg_queue* mq, void* buf, size_t len)
{
	int wakeup = 0;
	struct dt_msg_entry* entry;

	/* check conditions */
	if(!buf) return;
	if(len == 0) {
		/* it is not possible to log entries with zero length,
		 * because the framestream protocol does not carry it.
		 * However the protobuf serialization does not create zero
		 * length datagrams for dnstap, so this should not happen. */
		free(buf);
		return;
	}
	if(!mq) {
		free(buf);
		return;
	}

	/* allocate memory for queue entry */
	entry = malloc(sizeof(*entry));
	if(!entry) {
		log_err("out of memory logging dnstap");
		free(buf);
		return;
	}
	entry->next = NULL;
	entry->buf = buf;
	entry->len = len;

	/* aqcuire lock */
	lock_basic_lock(&mq->lock);
	/* list was empty, wakeup dtio */
	if(mq->first == NULL)
		wakeup = 1;
	/* see if it is going to fit */
	if(mq->cursize + len > mq->maxsize) {
		/* buffer full, or congested. */
		/* drop */
		lock_basic_unlock(&mq->lock);
		free(buf);
		free(entry);
		return;
	}
	mq->cursize += len;
	/* append to list */
	if(mq->last) {
		mq->last->next = entry;
	} else {
		mq->first = entry;
	}
	mq->last = entry;
	/* release lock */
	lock_basic_unlock(&mq->lock);

	if(wakeup)
		dtio_wakeup(mq->dtio);
}

struct dt_io_thread* dt_io_thread_create(void)
{
	struct dt_io_thread* dtio = calloc(1, sizeof(*dtio));
	return dtio;
}

void dt_io_thread_delete(struct dt_io_thread* dtio)
{
	struct dt_io_list_item* item, *nextitem;
	if(!dtio) return;
	item=dtio->io_list;
	while(item) {
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
	free(dtio->socket_path);
	free(dtio);
}

void dt_io_thread_apply_cfg(struct dt_io_thread* dtio, struct config_file *cfg)
{
	dtio->upstream_is_unix = 1;
	dtio->socket_path = strdup(cfg->dnstap_socket_path);
}

int dt_io_thread_register_queue(struct dt_io_thread* dtio,
        struct dt_msg_queue* mq)
{
	struct dt_io_list_item* item = malloc(sizeof(*item));
	if(!item) return 0;
	mq->dtio = dtio;
	item->queue = mq;
	item->next = dtio->io_list;
	dtio->io_list = item;
	dtio->io_list_iter = NULL;
	return 1;
}

void dt_io_thread_unregister_queue(struct dt_io_thread* dtio,
        struct dt_msg_queue* mq)
{
	struct dt_io_list_item* item, *prev=NULL;
	if(!dtio) return;
	item = dtio->io_list;
	while(item) {
		if(item->queue == mq) {
			/* found it */
			if(prev) prev->next = item->next;
			else dtio->io_list = item->next;
			/* the queue itself only registered, not deleted */
			item->queue->dtio = NULL;
			free(item);
			dtio->io_list_iter = NULL;
			return;
		}
		prev = item;
		item = item->next;
	}
}

/** pick a message from the queue, the routine locks and unlocks,
 * returns true if there is a message */
static int dt_msg_queue_pop(struct dt_msg_queue* mq, void** buf,
	size_t* len)
{
	lock_basic_lock(&mq->lock);
	if(mq->first) {
		struct dt_msg_entry* entry = mq->first;
		mq->first = entry->next;
		if(!entry->next) mq->last = NULL;
		mq->cursize -= entry->len;
		lock_basic_unlock(&mq->lock);

		*buf = entry->buf;
		*len = entry->len;
		free(entry);
		return 1;
	}
	lock_basic_unlock(&mq->lock);
	return 0;
}

/** find message in queue, false if no message, true if message to send */
static int dtio_find_in_queue(struct dt_io_thread* dtio,
	struct dt_msg_queue* mq)
{
	void* buf=NULL;
	size_t len=0;
	if(dt_msg_queue_pop(mq, &buf, &len)) {
		dtio->cur_msg = buf;
		dtio->cur_msg_len = len;
		dtio->cur_msg_done = 0;
		dtio->cur_msg_len_done = 0;
		return 1;
	}
	return 0;
}

/** find a new message to write, search message queues, false if none */
static int dtio_find_msg(struct dt_io_thread* dtio)
{
	struct dt_io_list_item *spot, *item;

	spot = dtio->io_list_iter;
	/* use the next queue for the next message lookup,
	 * if we hit the end(NULL) the NULL restarts the iter at start. */
	if(spot)
		dtio->io_list_iter = spot->next;
	else if(dtio->io_list)
		dtio->io_list_iter = dtio->io_list->next;

	/* scan from spot to end-of-io_list */
	item = spot;
	while(item) {
		if(dtio_find_in_queue(dtio, item->queue))
			return 1;
		item = item->next;
	}
	/* scan starting at the start-of-list (to wrap around the end) */
	item = dtio->io_list;
	while(item) {
		if(dtio_find_in_queue(dtio, item->queue))
			return 1;
		item = item->next;
	}
	return 0;
}

/** callback for the dnstap reconnect, to start reconnecting to output */
static void dtio_reconnect_timeout_cb(int ATTR_UNUSED(fd),
	short ATTR_UNUSED(bits), void* arg)
{
	struct dt_io_thread* dtio = (struct dt_io_thread*)arg;
	dtio->reconnect_is_added = 0;
	verbose(VERB_ALGO, "dnstap io: reconnect timer");

	dtio_open_output(dtio);
	if(dtio->event) {
		dtio_add_output_event_write(dtio);
		/* nothing wrong so far, wait on the output event */
		return;
	}
	/* exponential backoff and retry on timer */
	dtio_reconnect_enable(dtio);
}

/** attempt to reconnect to the output, after a timeout */
static void dtio_reconnect_enable(struct dt_io_thread* dtio)
{
	struct timeval tv;
	int msec;
	if(dtio->want_to_exit) return;
	if(dtio->reconnect_is_added)
		return; /* already done */

	/* exponential backoff, store the value for next timeout */
	msec = dtio->reconnect_timeout;
	if(msec == 0) {
		dtio->reconnect_timeout = DTIO_RECONNECT_TIMEOUT_MIN;
	} else {
		dtio->reconnect_timeout = msec*2;
		if(dtio->reconnect_timeout > DTIO_RECONNECT_TIMEOUT_MAX)
			dtio->reconnect_timeout = DTIO_RECONNECT_TIMEOUT_MAX;
	}
	verbose(VERB_ALGO, "dnstap io: set reconnect attempt after %d msec",
		msec);

	/* setup wait timer */
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec%1000)*1000;
	if(ub_timer_add(dtio->reconnect_timer, dtio->event_base,
		&dtio_reconnect_timeout_cb, dtio, &tv) != 0) {
		log_err("dnstap io: could not reconnect ev timer add");
		return;
	}
	dtio->reconnect_is_added = 1;
}

/** remove dtio reconnect timer */
static void dtio_reconnect_del(struct dt_io_thread* dtio)
{
	if(!dtio->reconnect_is_added)
		return;
	ub_timer_del(dtio->reconnect_timer);
	dtio->reconnect_is_added = 0;
}

/** clear the reconnect exponential backoff timer.
 * We have successfully connected so we can try again with short timeouts. */
static void dtio_reconnect_clear(struct dt_io_thread* dtio)
{
	dtio->reconnect_timeout = 0;
	dtio_reconnect_del(dtio);
}

/** delete the current message in the dtio, and reset counters */
static void dtio_cur_msg_free(struct dt_io_thread* dtio)
{
	free(dtio->cur_msg);
	dtio->cur_msg = NULL;
	dtio->cur_msg_len = 0;
	dtio->cur_msg_done = 0;
	dtio->cur_msg_len_done = 0;
}

/** del the output file descriptor event for listening */
static void dtio_del_output_event(struct dt_io_thread* dtio)
{
	if(!dtio->event_added)
		return;
	ub_event_del(dtio->event);
	dtio->event_added = 0;
	dtio->event_added_is_write = 0;
}

/** close and stop the output file descriptor event */
static void dtio_close_output(struct dt_io_thread* dtio)
{
	if(!dtio->event)
		return;
	ub_event_free(dtio->event);
	dtio->event = NULL;
#ifndef USE_WINSOCK
	close(dtio->fd);
#else
	closesocket(dtio->fd);
#endif
	dtio->fd = -1;

	/* if there is a (partial) message, discard it
	 * we cannot send (the remainder of) it, and a new
	 * connection needs to start with a control frame. */
	if(dtio->cur_msg) {
		dtio_cur_msg_free(dtio);
	}
	dtio_reconnect_enable(dtio);
}

/** check for pending nonblocking connect errors,
 * returns 1 if it is okay. -1 on error (close it), 0 to try later */
static int dtio_check_nb_connect(struct dt_io_thread* dtio)
{
	int error = 0;
	socklen_t len = (socklen_t)sizeof(error);
	if(!dtio->check_nb_connect)
		return 1; /* everything okay */
	if(getsockopt(dtio->fd, SOL_SOCKET, SO_ERROR, (void*)&error,
		&len) < 0) {
#ifndef USE_WINSOCK
		error = errno; /* on solaris errno is error */
#else
		error = WSAGetLastError();
#endif
	}
#ifndef USE_WINSOCK
#if defined(EINPROGRESS) && defined(EWOULDBLOCK)
	if(error == EINPROGRESS || error == EWOULDBLOCK)
		return 0; /* try again later */
#endif
#else
	if(error == WSAEINPROGRESS) {
		return 0; /* try again later */
	} else if(error == WSAEWOULDBLOCK) {
		ub_winsock_tcp_wouldblock((dtio->stop_flush_event?
			dtio->stop_flush_event:dtio->event), UB_EV_WRITE);
		return 0; /* try again later */
	}
#endif
	if(error != 0) {
		char* to = dtio->socket_path;
#ifndef USE_WINSOCK
		log_err("dnstap io: failed to connect to \"%s\": %s",
			to, strerror(error));
#else
		log_err("dnstap io: failed to connect to \"%s\": %s",
			to, wsa_strerror(error));
#endif
		return -1; /* error, close it */
	}

	verbose(VERB_ALGO, "dnstap io: connected to \"%s\"", dtio->socket_path);
	dtio_reconnect_clear(dtio);
	dtio->check_nb_connect = 0;
	return 1; /* everything okay */
}

/** write buffer to output.
 * returns number of bytes written, 0 if nothing happened,
 * try again later, or -1 if the channel is to be closed. */
static int dtio_write_buf(struct dt_io_thread* dtio, uint8_t* buf,
	size_t len)
{
	ssize_t ret;
	if(dtio->fd == -1)
		return -1;
	if(dtio->check_nb_connect) {
		int connect_err = dtio_check_nb_connect(dtio);
		if(connect_err == -1) {
			return -1;
		} else if(connect_err == 0) {
			return 0;
		}
	}
	ret = send(dtio->fd, (void*)buf, len, 0);
	if(ret == -1) {
#ifndef USE_WINSOCK
		if(errno == EINTR || errno == EAGAIN)
			return 0;
		log_err("dnstap io: failed send: %s", strerror(errno));
#else
		if(WSAGetLastError() == WSAEINPROGRESS)
			return 0;
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			ub_winsock_tcp_wouldblock((dtio->stop_flush_event?
				dtio->stop_flush_event:dtio->event),
				UB_EV_WRITE);
			return 0;
		}
		log_err("dnstap io: failed send: %s",
			wsa_strerror(WSAGetLastError()));
#endif
		return -1;
	}
	return ret;
}

#ifdef HAVE_WRITEV
/** write with writev, len and message, in one write, if possible.
 * return true if message is done, false if incomplete */
static int dtio_write_with_writev(struct dt_io_thread* dtio)
{
	uint32_t sendlen = htonl(dtio->cur_msg_len);
	struct iovec iov[2];
	ssize_t r;
	if(dtio->check_nb_connect) {
		int connect_err = dtio_check_nb_connect(dtio);
		if(connect_err == -1) {
			/* close the channel */
			dtio_del_output_event(dtio);
			dtio_close_output(dtio);
			return 0;
		} else if(connect_err == 0) {
			return 0;
		}
	}
	iov[0].iov_base = ((uint8_t*)&sendlen)+dtio->cur_msg_len_done;
	iov[0].iov_len = sizeof(sendlen)-dtio->cur_msg_len_done;
	iov[1].iov_base = dtio->cur_msg;
	iov[1].iov_len = dtio->cur_msg_len;
	log_assert(iov[0].iov_len > 0);
	r = writev(dtio->fd, iov, 2);
	if(r == -1) {
#ifndef USE_WINSOCK
		if(errno == EINTR || errno == EAGAIN)
			return 0;
		log_err("dnstap io: failed writev: %s", strerror(errno));
#else
		if(WSAGetLastError() == WSAEINPROGRESS)
			return 0;
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			ub_winsock_tcp_wouldblock((dtio->stop_flush_event?
				dtio->stop_flush_event:dtio->event),
				UB_EV_WRITE);
			return 0;
		}
		log_err("dnstap io: failed writev: %s",
			wsa_strerror(WSAGetLastError()));
#endif
		/* close the channel */
		dtio_del_output_event(dtio);
		dtio_close_output(dtio);
		return 0;
	}
	/* written r bytes */
	dtio->cur_msg_len_done += r;
	if(dtio->cur_msg_len_done < 4)
		return 0;
	if(dtio->cur_msg_len_done > 4) {
		dtio->cur_msg_done = dtio->cur_msg_len_done-4;
		dtio->cur_msg_len_done = 4;
	}
	if(dtio->cur_msg_done < dtio->cur_msg_len)
		return 0;
	return 1;
}
#endif /* HAVE_WRITEV */

/** write more of the length, preceding the data frame.
 * return true if message is done, false if incomplete. */
static int dtio_write_more_of_len(struct dt_io_thread* dtio)
{
#ifndef HAVE_WRITEV
	uint32_t sendlen = htonl(dtio->cur_msg_len);
	int r;
#endif
	if(dtio->cur_msg_len_done >= 4)
		return 1;
#ifdef HAVE_WRITEV
	/* we try writev for everything.*/
	return dtio_write_with_writev(dtio);
#else
	r = dtio_write_buf(dtio,
		((uint8_t*)&sendlen)+dtio->cur_msg_len_done,
		sizeof(sendlen)-dtio->cur_msg_len_done);
	if(r == -1) {
		/* close the channel */
		dtio_del_output_event(dtio);
		dtio_close_output(dtio);
		return 0;
	} else if(r == 0) {
		/* try again later */
		return 0;
	}
	dtio->cur_msg_len_done += r;
	if(dtio->cur_msg_len_done < 4)
		return 0;
	return 1;
#endif /* HAVE_WRITEV */
}

/** write more of the data frame.
 * return true if message is done, false if incomplete. */
static int dtio_write_more_of_data(struct dt_io_thread* dtio)
{
	int r;
	if(dtio->cur_msg_done >= dtio->cur_msg_len)
		return 1;
	r = dtio_write_buf(dtio,
		((uint8_t*)dtio->cur_msg)+dtio->cur_msg_done,
		dtio->cur_msg_len - dtio->cur_msg_done);
	if(r == -1) {
		/* close the channel */
		dtio_del_output_event(dtio);
		dtio_close_output(dtio);
		return 0;
	} else if(r == 0) {
		/* try again later */
		return 0;
	}
	dtio->cur_msg_done += r;
	if(dtio->cur_msg_done < dtio->cur_msg_len)
		return 0;
	return 1;
}

/** write more of the current messsage. false if incomplete, true if
 * the message is done */
static int dtio_write_more(struct dt_io_thread* dtio)
{
	if(dtio->cur_msg_len_done < 4) {
		if(!dtio_write_more_of_len(dtio))
			return 0;
	}
	if(dtio->cur_msg_done < dtio->cur_msg_len) {
		if(!dtio_write_more_of_data(dtio))
			return 0;
	}
	return 1;
}

/** check if the output fd has been closed,
 * it returns false if the stream is closed. */
static int dtio_check_close(struct dt_io_thread* dtio)
{
	/* we don't want to read any packets, but if there are we can
	 * discard the input (ignore it).  Ignore of unknown (control)
	 * packets is okay for the framestream protocol.  And also, the
	 * read call can return that the stream has been closed by the
	 * other side. */
	ssize_t r;
	uint8_t buf[1024];
	if(dtio->fd == -1) return 0;
	while(1) {
		r = recv(dtio->fd, (void*)buf, sizeof(buf), 0);
		if(r == -1) {
#ifndef USE_WINSOCK
			if(errno == EINTR || errno == EAGAIN)
				return 1; /* try later */
#else
			if(WSAGetLastError() == WSAEINPROGRESS) {
				return 1; /* try later */
			} else if(WSAGetLastError() == WSAEWOULDBLOCK) {
				ub_winsock_tcp_wouldblock(
					(dtio->stop_flush_event?
					dtio->stop_flush_event:dtio->event),
					UB_EV_READ);
				return 1; /* try later */
			}
#endif
			log_err("dnstap io: output recv: %s", strerror(errno));
			/* and close below */
			break;
		}
		if(r == 0) {
			verbose(VERB_ALGO, "dnstap io: output closed by the other side");
			/* and close below */
			break;
		}
		/* something was received, ignore it */
	}
	/* the other end has been closed */
	/* close the channel */
	dtio_del_output_event(dtio);
	dtio_close_output(dtio);
	return 0;
}

/** add the output file descriptor event for listening, read only */
static void dtio_add_output_event_read(struct dt_io_thread* dtio)
{
	if(!dtio->event)
		return;
	if(dtio->event_added && !dtio->event_added_is_write)
		return;
	/* we have to (re-)register the event */
	if(dtio->event_added)
		ub_event_del(dtio->event);
	ub_event_del_bits(dtio->event, UB_EV_WRITE);
	if(ub_event_add(dtio->event, NULL) != 0) {
		log_err("dnstap io: out of memory (adding event)");
		dtio->event_added = 0;
		dtio->event_added_is_write = 0;
		/* close output and start reattempts to open it */
		dtio_close_output(dtio);
		return;
	}
	dtio->event_added = 1;
	dtio->event_added_is_write = 0;
}

/** add the output file descriptor event for listening, read and write */
static void dtio_add_output_event_write(struct dt_io_thread* dtio)
{
	if(!dtio->event)
		return;
	if(dtio->event_added && dtio->event_added_is_write)
		return;
	/* we have to (re-)register the event */
	if(dtio->event_added)
		ub_event_del(dtio->event);
	ub_event_add_bits(dtio->event, UB_EV_WRITE);
	if(ub_event_add(dtio->event, NULL) != 0) {
		log_err("dnstap io: out of memory (adding event)");
		dtio->event_added = 0;
		dtio->event_added_is_write = 0;
		/* close output and start reattempts to open it */
		dtio_close_output(dtio);
		return;
	}
	dtio->event_added = 1;
	dtio->event_added_is_write = 1;
}

/** put the dtio thread to sleep */
static void dtio_sleep(struct dt_io_thread* dtio)
{
	/* unregister the event polling for write, because there is
	 * nothing to be written */
	dtio_add_output_event_read(dtio);
}

/** callback for the dnstap events, to write to the output */
static void dtio_output_cb(int ATTR_UNUSED(fd), short bits, void* arg)
{
	struct dt_io_thread* dtio = (struct dt_io_thread*)arg;
	int i;

	if((bits&UB_EV_READ)) {
		if(!dtio_check_close(dtio))
			return;
	}

	/* loop to process a number of messages.  This improves throughput,
	 * because selecting on write-event if not needed for busy messages
	 * (dnstap log) generation and if they need to all be written back.
	 * The write event is usually not blocked up.  But not forever,
	 * because the event loop needs to stay responsive for other events.
	 * If there are no (more) messages, or if the output buffers get
	 * full, it returns out of the loop. */
	for(i=0; i<DTIO_MESSAGES_PER_CALLBACK; i++) {
		/* see if there are messages that need writing */
		if(!dtio->cur_msg) {
			if(!dtio_find_msg(dtio)) {
				if(i == 0) {
					/* no messages on the first iteration,
					 * the queues are all empty */
					dtio_sleep(dtio);
				}
				return; /* nothing to do */
			}
		}

		/* write it */
		if(dtio->cur_msg_done < dtio->cur_msg_len) {
			if(!dtio_write_more(dtio))
				return;
		}

		/* done with the current message */
		dtio_cur_msg_free(dtio);
	}
}

/** callback for the dnstap commandpipe, to stop the dnstap IO */
static void dtio_cmd_cb(int fd, short ATTR_UNUSED(bits), void* arg)
{
	struct dt_io_thread* dtio = (struct dt_io_thread*)arg;
	uint8_t cmd;
	ssize_t r;
	if(dtio->want_to_exit)
		return;
	r = read(fd, &cmd, sizeof(cmd));
	if(r == -1) {
#ifndef USE_WINSOCK
		if(errno == EINTR || errno == EAGAIN)
			return; /* ignore this */
		log_err("dnstap io: failed to read: %s", strerror(errno));
#else
		if(WSAGetLastError() == WSAEINPROGRESS)
			return;
		if(WSAGetLastError() == WSAEWOULDBLOCK)
			return;
		log_err("dnstap io: failed to read: %s",
			wsa_strerror(WSAGetLastError()));
#endif
		/* and then fall through to quit the thread */
	} else if(r == 0) {
		verbose(VERB_ALGO, "dnstap io: cmd channel closed");
	} else if(r == 1 && cmd == DTIO_COMMAND_STOP) {
		verbose(VERB_ALGO, "dnstap io: cmd channel cmd quit");
	} else if(r == 1 && cmd == DTIO_COMMAND_WAKEUP) {
		verbose(VERB_ALGO, "dnstap io: cmd channel cmd wakeup");
		/* reregister event */
		dtio_add_output_event_write(dtio);
		return;
	} else if(r == 1) {
		verbose(VERB_ALGO, "dnstap io: cmd channel unknown command");
	}
	dtio->want_to_exit = 1;
	if(ub_event_base_loopexit((struct ub_event_base*)dtio->event_base)
		!= 0) {
		log_err("dnstap io: could not loopexit");
	}
}

#ifndef THREADS_DISABLED
/** setup the event base for the dnstap io thread */
static void dtio_setup_base(struct dt_io_thread* dtio, time_t* secs,
	struct timeval* now)
{
	memset(now, 0, sizeof(*now));
	dtio->event_base = ub_default_event_base(0, secs, now);
	if(!dtio->event_base) {
		fatal_exit("dnstap io: could not create event_base");
	}
}
#endif /* THREADS_DISABLED */

/** setup the cmd event for dnstap io */
static void dtio_setup_cmd(struct dt_io_thread* dtio)
{
	struct ub_event* cmdev;
	fd_set_nonblock(dtio->commandpipe[0]);
	cmdev = ub_event_new(dtio->event_base, dtio->commandpipe[0],
		UB_EV_READ | UB_EV_PERSIST, &dtio_cmd_cb, dtio);
	if(!cmdev) {
		fatal_exit("dnstap io: out of memory");
	}
	dtio->command_event = cmdev;
	if(ub_event_add(cmdev, NULL) != 0) {
		fatal_exit("dnstap io: out of memory (adding event)");
	}
}

/** setup the reconnect event for dnstap io */
static void dtio_setup_reconnect(struct dt_io_thread* dtio)
{
	dtio_reconnect_clear(dtio);
	dtio->reconnect_timer = ub_event_new(dtio->event_base, -1,
		UB_EV_TIMEOUT, &dtio_reconnect_timeout_cb, dtio);
	if(!dtio->reconnect_timer) {
		fatal_exit("dnstap io: out of memory");
	}
}

/**
 * structure to keep track of information during stop flush
 */
struct stop_flush_info {
	/** the event base during stop flush */
	struct ub_event_base* base;
	/** did we already want to exit this stop-flush event base */
	int want_to_exit_flush;
	/** has the timer fired */
	int timer_done;
	/** the dtio */
	struct dt_io_thread* dtio;
	/** the stop control frame */
	void* stop_frame;
	/** length of the stop frame */
	size_t stop_frame_len;
	/** how much we have done of the stop frame */
	size_t stop_frame_done;
};

/** exit the stop flush base */
static void dtio_stop_flush_exit(struct stop_flush_info* info)
{
	if(info->want_to_exit_flush)
		return;
	info->want_to_exit_flush = 1;
	if(ub_event_base_loopexit(info->base) != 0) {
		log_err("dnstap io: could not loopexit");
	}
}

/** send the stop control,
 * return true if completed the frame. */
static int dtio_control_stop_send(struct stop_flush_info* info)
{
	struct dt_io_thread* dtio = info->dtio;
	int r;
	if(info->stop_frame_done >= info->stop_frame_len)
		return 1;
	r = dtio_write_buf(dtio, ((uint8_t*)info->stop_frame) +
		info->stop_frame_done, info->stop_frame_len -
		info->stop_frame_done);
	if(r == -1) {
		verbose(VERB_ALGO, "dnstap io: stop flush: output closed");
		dtio_stop_flush_exit(info);
		return 0;
	}
	if(r == 0) {
		/* try again later, or timeout */
		return 0;
	}
	info->stop_frame_done += r;
	if(info->stop_frame_done < info->stop_frame_len)
		return 0; /* not done yet */
	return 1;
}

static void dtio_stop_timer_cb(int ATTR_UNUSED(fd), short ATTR_UNUSED(bits),
	void* arg)
{
	struct stop_flush_info* info = (struct stop_flush_info*)arg;
	if(info->want_to_exit_flush)
		return;
	verbose(VERB_ALGO, "dnstap io: stop flush timer expired, stop flush");
	info->timer_done = 1;
	dtio_stop_flush_exit(info);
}

static void dtio_stop_ev_cb(int ATTR_UNUSED(fd), short bits, void* arg)
{
	struct stop_flush_info* info = (struct stop_flush_info*)arg;
	struct dt_io_thread* dtio = info->dtio;
	if(info->want_to_exit_flush)
		return;
	if((bits&UB_EV_READ)) {
		if(!dtio_check_close(dtio)) {
			if(dtio->fd == -1) {
				verbose(VERB_ALGO, "dnstap io: "
					"stop flush: output closed");
				dtio_stop_flush_exit(info);
			}
			return;
		}
	}
	/* write remainder of last frame */
	if(dtio->cur_msg) {
		if(dtio->cur_msg_done < dtio->cur_msg_len) {
			if(!dtio_write_more(dtio)) {
				if(dtio->fd == -1) {
					verbose(VERB_ALGO, "dnstap io: "
						"stop flush: output closed");
					dtio_stop_flush_exit(info);
				}
				return;
			}
		}
		verbose(VERB_ALGO, "dnstap io: stop flush completed "
			"last frame");
		dtio_cur_msg_free(dtio);
	}
	/* write stop frame */
	if(info->stop_frame_done < info->stop_frame_len) {
		if(!dtio_control_stop_send(info))
			return;
		verbose(VERB_ALGO, "dnstap io: stop flush completed "
			"stop control frame");
	}
	/* when last frame and stop frame are sent, exit */
	dtio_stop_flush_exit(info);
}

/** flush at end, last packet and stop control */
static void dtio_control_stop_flush(struct dt_io_thread* dtio)
{
	/* briefly attempt to flush the previous packet to the output,
	 * this could be a partial packet, or even the start control frame */
	time_t secs = 0;
	struct timeval now;
	struct stop_flush_info info;
	struct timeval tv;
	struct ub_event* timer, *stopev;

	if(dtio->fd == -1 || dtio->check_nb_connect) {
		/* no connection or we have just connected, so nothing is
		 * sent yet, so nothing to stop or flush */
		return;
	}

	memset(&info, 0, sizeof(info));
	memset(&now, 0, sizeof(now));
	info.dtio = dtio;
	info.base = ub_default_event_base(0, &secs, &now);
	if(!info.base) {
		log_err("dnstap io: malloc failure");
		return;
	}
	timer = ub_event_new(info.base, -1, UB_EV_TIMEOUT,
		&dtio_stop_timer_cb, &info);
	if(!timer) {
		log_err("dnstap io: malloc failure");
		ub_event_base_free(info.base);
		return;
	}
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 2;
	if(ub_timer_add(timer, info.base, &dtio_stop_timer_cb, &info,
		&tv) != 0) {
		log_err("dnstap io: cannot event_timer_add");
		ub_event_free(timer);
		ub_event_base_free(info.base);
		return;
	}
	stopev = ub_event_new(info.base, dtio->fd, UB_EV_READ |
		UB_EV_WRITE | UB_EV_PERSIST, &dtio_stop_ev_cb, &info);
	if(!stopev) {
		log_err("dnstap io: malloc failure");
		ub_timer_del(timer);
		ub_event_free(timer);
		ub_event_base_free(info.base);
		return;
	}
	if(ub_event_add(stopev, NULL) != 0) {
		log_err("dnstap io: cannot event_add");
		ub_event_free(stopev);
		ub_timer_del(timer);
		ub_event_free(timer);
		ub_event_base_free(info.base);
		return;
	}
	info.stop_frame = fstrm_create_control_frame_stop(
		&info.stop_frame_len);
	if(!info.stop_frame) {
		log_err("dnstap io: malloc failure");
		ub_event_del(stopev);
		ub_event_free(stopev);
		ub_timer_del(timer);
		ub_event_free(timer);
		ub_event_base_free(info.base);
		return;
	}
	dtio->stop_flush_event = stopev;

	/* wait briefly, or until finished */
	verbose(VERB_ALGO, "dnstap io: stop flush started");
	if(ub_event_base_dispatch(info.base) < 0) {
		log_err("dnstap io: dispatch flush failed, errno is %s",
			strerror(errno));
	}
	verbose(VERB_ALGO, "dnstap io: stop flush ended");
	free(info.stop_frame);
	dtio->stop_flush_event = NULL;
	ub_event_del(stopev);
	ub_event_free(stopev);
	ub_timer_del(timer);
	ub_event_free(timer);
	ub_event_base_free(info.base);
}

/** perform desetup and free stuff when the dnstap io thread exits */
static void dtio_desetup(struct dt_io_thread* dtio)
{
	dtio_control_stop_flush(dtio);
	dtio_del_output_event(dtio);
	dtio_close_output(dtio);
	ub_event_del(dtio->command_event);
	ub_event_free(dtio->command_event);
#ifndef USE_WINSOCK
	close(dtio->commandpipe[0]);
#else
	_close(dtio->commandpipe[0]);
#endif
	dtio->commandpipe[0] = -1;
	dtio_reconnect_del(dtio);
	ub_event_free(dtio->reconnect_timer);
	dtio_cur_msg_free(dtio);
#ifndef THREADS_DISABLED
	ub_event_base_free(dtio->event_base);
#endif
}

/** setup a start control message */
static int dtio_control_start_send(struct dt_io_thread* dtio)
{
	log_assert(dtio->cur_msg == NULL && dtio->cur_msg_len == 0);
	dtio->cur_msg = fstrm_create_control_frame_start(DNSTAP_CONTENT_TYPE,
		&dtio->cur_msg_len);
	if(!dtio->cur_msg) {
		return 0;
	}
	/* setup to send the control message */
	/* set that the buffer needs to be sent, but the length
	 * of that buffer is already written, that way the buffer can
	 * start with 0 length and then the length of the control frame
	 * in it */
	dtio->cur_msg_done = 0;
	dtio->cur_msg_len_done = 4;
	return 1;
}

/** open the output file descriptor */
static void dtio_open_output(struct dt_io_thread* dtio)
{
#ifdef HAVE_SYS_UN_H
	struct ub_event* ev;
	struct sockaddr_un s;
	dtio->fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(dtio->fd == -1) {
#ifndef USE_WINSOCK
		log_err("dnstap io: failed to create socket: %s",
			strerror(errno));
#else
		log_err("dnstap io: failed to create socket: %s",
			wsa_strerror(WSAGetLastError()));
#endif
		return;
	}
	memset(&s, 0, sizeof(s));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
        /* this member exists on BSDs, not Linux */
        s.sun_len = (unsigned)sizeof(usock);
#endif
	s.sun_family = AF_LOCAL;
	/* length is 92-108, 104 on FreeBSD */
        (void)strlcpy(s.sun_path, dtio->socket_path, sizeof(s.sun_path));
	fd_set_nonblock(dtio->fd);
	if(connect(dtio->fd, (struct sockaddr*)&s, (socklen_t)sizeof(s))
		== -1) {
		char* to = dtio->socket_path;
#ifndef USE_WINSOCK
		log_err("dnstap io: failed to connect to \"%s\": %s",
			to, strerror(errno));
#else
		log_err("dnstap io: failed to connect to \"%s\": %s",
			to, wsa_strerror(WSAGetLastError()));
#endif
#ifndef USE_WINSOCK
		close(dtio->fd);
#else
		closesocket(dtio->fd);
#endif
		dtio->fd = -1;
		dtio_reconnect_enable(dtio);
		return;
	}
	dtio->check_nb_connect = 1;

	/* the EV_READ is to catch channel close, write to write packets */
	ev = ub_event_new(dtio->event_base, dtio->fd,
		UB_EV_READ | UB_EV_WRITE | UB_EV_PERSIST, &dtio_output_cb,
		dtio);
	if(!ev) {
		log_err("dnstap io: out of memory");
#ifndef USE_WINSOCK
		close(dtio->fd);
#else
		closesocket(dtio->fd);
#endif
		dtio->fd = -1;
		dtio_reconnect_enable(dtio);
		return;
	}
	dtio->event = ev;

	/* setup protocol control message to start */
	if(!dtio_control_start_send(dtio)) {
		log_err("dnstap io: out of memory");
		ub_event_free(dtio->event);
		dtio->event = NULL;
#ifndef USE_WINSOCK
		close(dtio->fd);
#else
		closesocket(dtio->fd);
#endif
		dtio->fd = -1;
		dtio_reconnect_enable(dtio);
		return;
	}
#endif /* HAVE_SYS_UN_H */
}

/** perform the setup of the writer thread on the established event_base */
static void dtio_setup_on_base(struct dt_io_thread* dtio)
{
	dtio_setup_cmd(dtio);
	dtio_setup_reconnect(dtio);
	dtio_open_output(dtio);
	dtio_add_output_event_write(dtio);
}

#ifndef THREADS_DISABLED
/** the IO thread function for the DNSTAP IO */
static void* dnstap_io(void* arg)
{
	struct dt_io_thread* dtio = (struct dt_io_thread*)arg;
	time_t secs = 0;
	struct timeval now;

	/* setup */
	verbose(VERB_ALGO, "start dnstap io thread");
	dtio_setup_base(dtio, &secs, &now);
	dtio_setup_on_base(dtio);

	/* run */
	if(ub_event_base_dispatch(dtio->event_base) < 0) {
		log_err("dnstap io: dispatch failed, errno is %s",
			strerror(errno));
	}

	/* cleanup */
	verbose(VERB_ALGO, "stop dnstap io thread");
	dtio_desetup(dtio);
	return NULL;
}
#endif /* THREADS_DISABLED */

int dt_io_thread_start(struct dt_io_thread* dtio, void* event_base_nothr)
{
	/* set up the thread, can fail */
#ifndef USE_WINSOCK
	if(pipe(dtio->commandpipe) == -1) {
		log_err("failed to create pipe: %s", strerror(errno));
		return 0;
	}
#else
	if(_pipe(dtio->commandpipe, 4096, _O_BINARY) == -1) {
		log_err("failed to create _pipe: %s",
			wsa_strerror(WSAGetLastError()));
		return 0;
	}
#endif

	/* start the thread */
	dtio->started = 1;
#ifndef THREADS_DISABLED
	ub_thread_create(&dtio->tid, dnstap_io, dtio);
	(void)event_base_nothr;
#else
	dtio->event_base = event_base_nothr;
	dtio_setup_on_base(dtio);
#endif
	return 1;
}

void dt_io_thread_stop(struct dt_io_thread* dtio)
{
#ifndef THREADS_DISABLED
	uint8_t cmd = DTIO_COMMAND_STOP;
#endif
	if(!dtio) return;
	if(!dtio->started) return;
	verbose(VERB_ALGO, "dnstap io: send stop cmd");

#ifndef THREADS_DISABLED
	while(1) {
		ssize_t r = write(dtio->commandpipe[1], &cmd, sizeof(cmd));
		if(r == -1) {
#ifndef USE_WINSOCK
			if(errno == EINTR || errno == EAGAIN)
				continue;
			log_err("dnstap io stop: write: %s", strerror(errno));
#else
			if(WSAGetLastError() == WSAEINPROGRESS)
				continue;
			if(WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			log_err("dnstap io stop: write: %s",
				wsa_strerror(WSAGetLastError()));
#endif
			break;
		}
		break;
	}
	dtio->started = 0;
#endif /* THREADS_DISABLED */

#ifndef USE_WINSOCK
	close(dtio->commandpipe[1]);
#else
	_close(dtio->commandpipe[1]);
#endif
	dtio->commandpipe[1] = -1;
#ifndef THREADS_DISABLED
	ub_thread_join(dtio->tid);
#else
	dtio->want_to_exit = 1;
	dtio_desetup(dtio);
#endif
}
