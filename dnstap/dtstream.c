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

void
dt_msg_queue_submit(struct dt_msg_queue* mq, void* buf, size_t len)
{
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
	/* see if it is going to fit */
	if(mq->cursize + len > mq->maxsize) {
		/* buffer full, or congested. */
		/* drop */
		lock_basic_unlock(&mq->lock);
		free(buf);
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
	item->queue = mq;
	item->next = dtio->io_list;
	dtio->io_list = item;
	return 1;
}

void dt_io_thread_unregister_queue(struct dt_io_thread* dtio,
        struct dt_msg_queue* mq)
{
	struct dt_io_list_item* item=dtio->io_list, *prev=NULL;
	while(item) {
		if(item->queue == mq) {
			/* found it */
			if(prev) prev->next = item->next;
			else dtio->io_list = item->next;
			/* the queue itself only registered, not deleted */
			free(item);
			return;
		}
		prev = item;
		item = item->next;
	}
}

/** pick a message from the queue, lock and unlock, true if a message */
static int pick_msg_from_queue(struct dt_msg_queue* mq, void** buf,
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
	if(pick_msg_from_queue(mq, &buf, &len)) {
		dtio->cur_msg = buf;
		dtio->cur_msg_len = len;
		dtio->cur_msg_done = 0;
		return 1;
	}
	return 0;
}

/** find a new message to write, search message queues, false if none */
static int dtio_find_msg(struct dt_io_thread* dtio)
{
	struct dt_io_list_item* item = dtio->io_list;
	while(item) {
		if(dtio_find_in_queue(dtio, item->queue))
			return 1;
		item = item->next;
	}
	return 0;
}

/** write more of the current messsage. false if incomplete, true if
 * the message is done */
static int dtio_write_more(int fd, struct dt_io_thread* dtio)
{
	return 0;
}

/** callback for the dnstap events, to write to the output */
static void dtio_output_cb(int fd, short ATTR_UNUSED(bits), void* arg)
{
	struct dt_io_thread* dtio = (struct dt_io_thread*)arg;
	
	/* see if there are messages that need writing */
	if(!dtio->cur_msg) {
		if(!dtio_find_msg(dtio))
			return; /* nothing to do */
	}

	/* write it */
	if(dtio->cur_msg_done < dtio->cur_msg_len) {
		if(!dtio_write_more(fd, dtio))
			return;
	}

	/* done with the current message */
	free(dtio->cur_msg);
	dtio->cur_msg = NULL;
	dtio->cur_msg_len = 0;
	dtio->cur_msg_done = 0;
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
		if(errno == EINTR || errno == EAGAIN)
			return; /* ignore this */
		log_err("dnstap io: failed to read: %s", strerror(errno));
		/* and then fall through to quit the thread */
	}
	if(r == 0) {
		verbose(VERB_ALGO, "dnstap io: cmd channel closed");
	} else if(r == 1 && cmd == 0) {
		verbose(VERB_ALGO, "dnstap io: cmd channel cmd quit");
	}
	dtio->want_to_exit = 1;
	if(ub_event_base_loopexit((struct ub_event_base*)dtio->event_base)
		!= 0) {
		log_err("dnstap io: could not loopexit");
	}
}

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

/** del the output file descriptor event for listening */
static void dtio_del_output_event(struct dt_io_thread* dtio)
{
	if(!dtio->event_added)
		return;
	ub_event_del(dtio->event);
	dtio->event_added = 0;
}

/** close and stop the output file descriptor event */
static void dtio_close_output(struct dt_io_thread* dtio)
{
	if(!dtio->event)
		return;
	ub_event_free(dtio->event);
	dtio->event = NULL;
	close(dtio->fd);
	dtio->fd = -1;
}

/** perform desetup and free stuff when the dnstap io thread exits */
static void dtio_desetup(struct dt_io_thread* dtio)
{
	dtio_del_output_event(dtio);
	dtio_close_output(dtio);
	ub_event_del(dtio->command_event);
	ub_event_free(dtio->command_event);
	close(dtio->commandpipe[0]);
	dtio->commandpipe[0] = -1;
	ub_event_base_free(dtio->event_base);
}

/** open the output file descriptor */
static void dtio_open_output(struct dt_io_thread* dtio)
{
	struct ub_event* ev;
	struct sockaddr_un s;
	dtio->fd = socket(AF_LOCAL, SOCK_STREAM, SOCK_CLOEXEC);
	if(dtio->fd == -1) {
		log_err("dnstap io: failed to create socket: %s",
			strerror(errno));
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
	if(connect(dtio->fd, (struct sockaddr*)&s, (socklen_t)sizeof(s))
		== -1) {
		log_err("dnstap io: failed to connect: %s", strerror(errno));
		return;
	}
	fd_set_nonblock(dtio->fd);

	/* the EV_READ is to catch channel close, write to write packets */
	ev = ub_event_new(dtio->event_base, dtio->fd,
		UB_EV_READ | UB_EV_WRITE | UB_EV_PERSIST, &dtio_output_cb,
		dtio);
	if(!ev) {
		fatal_exit("dnstap io: out of memory");
	}
	dtio->event = ev;

}

/** add the output file descriptor event for listening */
static void dtio_add_output_event(struct dt_io_thread* dtio)
{
	if(ub_event_add(dtio->event, NULL) != 0) {
		fatal_exit("dnstap io: out of memory (adding event)");
	}
	dtio->event_added = 1;
}

/** the IO thread function for the DNSTAP IO */
static void* dnstap_io(void* arg)
{
	struct dt_io_thread* dtio = (struct dt_io_thread*)arg;
	time_t secs = 0;
	struct timeval now;

	/* setup */
	dtio_setup_base(dtio, &secs, &now);
	dtio_setup_cmd(dtio);
	dtio_open_output(dtio);
	dtio_add_output_event(dtio);
	verbose(VERB_ALGO, "start dnstap io thread");

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

int dt_io_thread_start(struct dt_io_thread* dtio)
{
	/* set up the thread, can fail */
	if(pipe(dtio->commandpipe) == -1) {
		log_err("failed to create pipe: %s", strerror(errno));
		return 0;
	}

	/* start the thread */
	ub_thread_create(&dtio->tid, dnstap_io, dtio);
	return 1;
}

void dt_io_thread_stop(struct dt_io_thread* dtio)
{
	uint8_t cmd = 0;
	if(!dtio) return;
	if(!dtio->event_base) return; /* not started */

	while(1) {
		ssize_t r = write(dtio->commandpipe[1], &cmd, sizeof(cmd));
		if(r == -1) {
			if(errno == EINTR || errno == EAGAIN)
				continue;
			log_err("dnstap io stop: write: %s", strerror(errno));
			break;
		}
		break;
	}

	close(dtio->commandpipe[1]);
	dtio->commandpipe[1] = -1;
	ub_thread_join(dtio->tid);
}
