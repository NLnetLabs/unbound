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

