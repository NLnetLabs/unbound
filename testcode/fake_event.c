/*
 * testcode/fake_event.c - fake event handling that replays existing scenario.
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
 * Event service that replays a scenario.
 * This implements the same exported symbols as the files:
 * util/netevent.c
 * services/listen_dnsport.c
 * services/outside_network.c
 * But these do not actually access the network or events, instead
 * the scenario is played.
 */

#include "config.h"
#include "testcode/fake_event.h"
#include "util/netevent.h"
#include "services/listen_dnsport.h"
#include "services/outside_network.h"
#include "testcode/replay.h"
#include "testcode/ldns-testpkts.h"

/** Global variable: the scenario. Saved here for when event_init is done. */
static struct replay_scenario* saved_scenario = NULL;

void 
fake_event_init(struct replay_scenario* scen)
{
	saved_scenario = scen;
}

void 
fake_event_cleanup()
{
	replay_scenario_delete(saved_scenario);
	saved_scenario = NULL;
}

/**
 * return: true if pending query matches the now event.
 */
static int 
pending_matches_current(struct replay_runtime* runtime)
{
	struct fake_pending* p;
	if(!runtime->now || runtime->now->evt_type != repevt_back_query
		|| !runtime->pending_list)
		return 0;
	/* see if any of the pending queries matches */
	for(p = runtime->pending_list; p; p = p->next) {
		if(find_match(runtime->now->match, p->pkt, p->transport))
			return 1;
	}
	return 0;
}

/**
 * See if outgoing pending query matches an entry.
 * @param runtime: runtime.
 * @param entry: if true, the entry that matches is returned.
 * @param pend: if true, the outgoing message that matches is returned.
 * return: true if pending query matches the now event.
 */
static int 
pending_matches_range(struct replay_runtime* runtime, 
	struct entry** entry, struct fake_pending** pend)
{
}

/**
 * Perform range entry on pending message.
 * @param runtime: runtime, needed?.
 * @param entry: entry that codes for the reply to do.
 * @param pend: pending query that is answered, callback called.
 */
static void
answer_callback_from_entry(struct replay_runtime* runtime,
        struct entry* entry, struct fake_pending* pend)
{
}

/**
 * Perform actions or checks determined by the moment.
 */
static void
do_moment(struct replay_runtime* runtime)
{
	if(!runtime->now)
		return;
}

/**
 * Advance to the next moment.
 */
static void
advance_moment(struct replay_runtime* runtime)
{
	if(!runtime->now)
		runtime->now = runtime->scenario->mom_first;
	else 	runtime->now = runtime->now->mom_next;
}

/** run the scenario in event callbacks */
static void
run_scenario(struct replay_runtime* runtime)
{
	struct entry* entry = NULL;
	struct fake_pending* pending = NULL;
	runtime->now = NULL;
	do {
		/* if moment matches pending query do it. */
		/* else if precoded_range matches pending, do it */
		/* else do the current moment */
		if(pending_matches_current(runtime)) {
			advance_moment(runtime);
		} else if(pending_matches_range(runtime, &entry, &pending)) {
			answer_callback_from_entry(runtime, entry, pending);
		} else {
			do_moment(runtime);
			advance_moment(runtime);
		}
	} while(runtime->now);
}

/*********** Dummy routines ***********/

struct listen_dnsport* 
listen_create(struct comm_base* base,
	int ATTR_UNUSED(num_ifs), const char* ATTR_UNUSED(ifs[]), 
	const char* ATTR_UNUSED(port),
	int ATTR_UNUSED(do_ip4), int ATTR_UNUSED(do_ip6), 
	int ATTR_UNUSED(do_udp), int ATTR_UNUSED(do_tcp),
	size_t bufsize, comm_point_callback_t* cb, void* cb_arg)
{
	struct replay_runtime* runtime = (struct replay_runtime*)base;
	struct listen_dnsport* l= calloc(1, sizeof(struct listen_dnsport));
	if(!l)
		return NULL;
	l->base = base;
	l->udp_buff = ldns_buffer_new(bufsize);
	if(!l->udp_buff) {
		free(l);
		return NULL;
	}
	runtime->callback_query = cb;
	runtime->cb_arg = cb_arg;
	return l;
}

void 
listen_delete(struct listen_dnsport* listen)
{
	if(!listen)
		return;
	ldns_buffer_free(listen->udp_buff);
	free(listen);
}

struct comm_base* comm_base_create()
{
	/* we return the runtime structure instead. */
	struct replay_runtime* runtime = (struct replay_runtime*)
		calloc(1, sizeof(struct replay_runtime));
	runtime->scenario = saved_scenario;
	return (struct comm_base*)runtime;
}

void comm_base_delete(struct comm_base* b)
{
	struct replay_runtime* runtime = (struct replay_runtime*)b;
	free(runtime);
}

void comm_base_dispatch(struct comm_base* b)
{
	struct replay_runtime* runtime = (struct replay_runtime*)b;
	run_scenario(runtime);
}

void comm_base_exit(struct comm_base* ATTR_UNUSED(b))
{
	/* some sort of failure */
	exit(1);
}

struct comm_signal* comm_signal_create(struct comm_base* base,
        void (*callback)(int, void*), void* cb_arg)
{
	struct replay_runtime* runtime = (struct replay_runtime*)base;
	runtime->sig_cb = callback;
	runtime->sig_cb_arg = cb_arg;
	return calloc(1, sizeof(struct comm_signal));
}

int comm_signal_bind(struct comm_signal* ATTR_UNUSED(comsig), int 
	ATTR_UNUSED(sig))
{
	return 1;
}

void comm_signal_delete(struct comm_signal* comsig)
{
	free(comsig);
}

void 
comm_point_send_reply(struct comm_reply* repinfo)
{
	/* TODO see if this is checked */
}

void 
comm_point_drop_reply(struct comm_reply* repinfo)
{
	/* TODO */
}

struct outside_network* 
outside_network_create(struct comm_base* base, size_t bufsize, 
	size_t ATTR_UNUSED(num_ports), 
	const char** ATTR_UNUSED(ifs), int ATTR_UNUSED(num_ifs),
	int ATTR_UNUSED(do_ip4), int ATTR_UNUSED(do_ip6), 
	int ATTR_UNUSED(port_base))
{
	struct outside_network* outnet =  calloc(1, 
		sizeof(struct outside_network));
	if(!outnet)
		return NULL;
	outnet->base = base;
	outnet->udp_buff = ldns_buffer_new(bufsize);
	if(!outnet->udp_buff)
		return NULL;
	return outnet;
}

void 
outside_network_delete(struct outside_network* outnet)
{
	if(!outnet)
		return;
	ldns_buffer_free(outnet->udp_buff);
	free(outnet);
}

void pending_udp_query(struct outside_network* outnet, ldns_buffer* packet,
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* callback, void* callback_arg)
{
	/* TODO create it */
}

/*********** End of Dummy routines ***********/
