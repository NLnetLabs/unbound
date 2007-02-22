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
#include "util/log.h"

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

/** helper function that logs a ldns_pkt packet to logfile. */
static void
log_pkt(const char* desc, ldns_pkt* pkt)
{
	char* str = ldns_pkt2str(pkt);
	if(!str)
		log_info("%s: (failed)", desc);
	else {
		log_info("%s%s", desc, str);
		free(str);
	}
}

/**
 * Returns a string describing the event type.
 */
static const char*
repevt_string(enum replay_event_type t)
{
	switch(t) {
	case repevt_nothing:	 return "NOTHING";
	case repevt_front_query: return "QUERY";
	case repevt_front_reply: return "CHECK_ANSWER";
	case repevt_timeout:	 return "TIMEOUT";
	case repevt_back_reply:  return "REPLY";
	case repevt_back_query:  return "CHECK_OUT_QUERY";
	case repevt_error:	 return "ERROR";
	default:		 return "UNKNOWN";
	};
}

/** delete a fake pending */
static void 
delete_fake_pending(struct fake_pending* pend)
{
	if(!pend)
		return;
	ldns_buffer_free(pend->buffer);
	ldns_pkt_free(pend->pkt);
	free(pend);
}

/** delete a replay answer */
static void
delete_replay_answer(struct replay_answer* a)
{
	if(!a)
		return;
	if(a->repinfo.c) {
		ldns_buffer_free(a->repinfo.c->buffer);
		free(a->repinfo.c);
	}
	ldns_pkt_free(a->pkt);
	free(a);
}

/**
 * return: true if pending query matches the now event.
 */
static int 
pending_matches_current(struct replay_runtime* runtime, 
	struct entry** entry, struct fake_pending **pend)
{
	struct fake_pending* p;
	struct entry* e;
	if(!runtime->now || runtime->now->evt_type != repevt_back_query
		|| !runtime->pending_list)
		return 0;
	/* see if any of the pending queries matches */
	for(p = runtime->pending_list; p; p = p->next) {
		if((e=find_match(runtime->now->match, p->pkt, p->transport))) {
			*entry = e;
			*pend = p;
			return 1;
		}
	}
	return 0;
}

/**
 * Find the range that matches this pending message.
 * @param runtime: runtime with current moment, and range list.
 * @param entry: returns the pointer to entry that matches.
 * @param pend: the pending that the entry must match.
 * @return: true if a match is found.
 */
static int
pending_find_match(struct replay_runtime* runtime, struct entry** entry, 
	struct fake_pending* pend)
{
	int timenow = runtime->now->time_step;
	struct replay_range* p = runtime->scenario->range_list;
	while(p) {
		if(p->start_step <= timenow && timenow <= p->end_step &&
		  (*entry = find_match(p->match, pend->pkt, pend->transport))) {
			log_info("matched query time %d in range [%d, %d] "
				"with entry line %d", timenow, 
				p->start_step, p->end_step, (*entry)->lineno);
			return 1;
		}
		p = p->next_range;
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
	struct fake_pending* p = runtime->pending_list;
	/* slow, O(N*N), but it works as advertised with weird matching */
	while(p) {
		if(pending_find_match(runtime, entry, p)) {
			*pend = p;
			return 1;
		}
		p = p->next;
	}
	return 0;
}

/**
 * Remove the item from the pending list.
 */
static void
pending_list_delete(struct replay_runtime* runtime, struct fake_pending* pend)
{
	struct fake_pending** prev = &runtime->pending_list;
	struct fake_pending* p = runtime->pending_list;

	while(p) {
		if(p == pend) {
			*prev = p->next;
			delete_fake_pending(pend);
			return;
		}

		prev = &p->next;
		p = p->next;
	}
}

/**
 * Fill buffer with reply from the entry.
 */
static void
fill_buffer_with_reply(ldns_buffer* buffer, struct entry* entry, ldns_pkt* q)
{
	ldns_status status;
	ldns_pkt* answer_pkt = NULL;
	log_assert(entry && entry->reply_list);
	ldns_buffer_clear(buffer);
	if(entry->reply_list->reply_from_hex) {
		status = ldns_buffer2pkt_wire(&answer_pkt, 
			entry->reply_list->reply_from_hex);
		if(status != LDNS_STATUS_OK) {
			log_err("testbound: hex packet unparsable, used asis.");
			ldns_buffer_write(buffer, 
			ldns_buffer_begin(entry->reply_list->reply_from_hex), 
			ldns_buffer_limit(entry->reply_list->reply_from_hex));
		}
	} else {
		answer_pkt = ldns_pkt_clone(entry->reply_list->reply);
	}
	if(answer_pkt) {
		if(q) adjust_packet(entry, answer_pkt, q);
		status = ldns_pkt2buffer_wire(buffer, answer_pkt);
		if(status != LDNS_STATUS_OK)
			fatal_exit("ldns: cannot pkt2buffer_wire parsed pkt");
	}
	ldns_pkt_free(answer_pkt);
	ldns_buffer_flip(buffer);
}

/**
 * Perform range entry on pending message.
 * @param runtime: runtime buffer size preference.
 * @param entry: entry that codes for the reply to do.
 * @param pend: pending query that is answered, callback called.
 */
static void
answer_callback_from_entry(struct replay_runtime* runtime,
        struct entry* entry, struct fake_pending* pend)
{
	struct comm_point c;
	memset(&c, 0, sizeof(c));
	c.fd = -1;
	c.buffer = ldns_buffer_new(runtime->bufsize);
	c.type = comm_udp;
	if(pend->transport == transport_tcp)
		c.type = comm_tcp;
	fill_buffer_with_reply(c.buffer, entry, pend->pkt);
	if((*pend->callback)(&c, pend->cb_arg, NETEVENT_NOERROR, NULL)) {
		fatal_exit("testbound: unexpected: callback returned 1");
	}
	ldns_buffer_free(c.buffer);
	pending_list_delete(runtime, pend);
}

/** Check the now moment answer check event */
static void
answer_check_it(struct replay_runtime* runtime)
{
	struct replay_answer* ans = runtime->answer_list, 
		**prev = &runtime->answer_list;
	log_assert(runtime && runtime->now && 
		runtime->now->evt_type == repevt_front_reply);
	while(ans) {
		enum transport_type tr = transport_tcp;
		if(ans->repinfo.c->type == comm_udp)
			tr = transport_udp;
		if(find_match(runtime->now->match, ans->pkt, tr)) {
			struct replay_answer *n = ans->next;
			log_info("testbound matched event entry from line %d",
				runtime->now->match->lineno);
			log_info("testbound: do STEP %d %s", 
				runtime->now->time_step,
				repevt_string(runtime->now->evt_type));
			*prev = ans->next;
			delete_replay_answer(ans);
			ans = n;
			return;
		} else {
			prev = &ans->next;
			ans = ans->next;
		}
	}
	log_info("testbound: do STEP %d %s", runtime->now->time_step,
		repevt_string(runtime->now->evt_type));
	fatal_exit("testbound: not matched");
}

/**
 * Create commpoint (as return address) for a fake incoming query.
 */
static void
fake_front_query(struct replay_runtime* runtime, struct replay_moment *todo)
{
	struct comm_reply repinfo;
	memset(&repinfo, 0, sizeof(repinfo));
	repinfo.c = (struct comm_point*)calloc(1, sizeof(struct comm_point));
	repinfo.addrlen = (socklen_t)sizeof(struct sockaddr_in);
	repinfo.c->fd = -1;
	repinfo.c->ev = (struct internal_event*)runtime;
	repinfo.c->buffer = ldns_buffer_new(runtime->bufsize);
	repinfo.c->type = comm_udp;
	fill_buffer_with_reply(repinfo.c->buffer, todo->match, NULL);
	log_info("testbound: incoming QUERY");
	/* call the callback for incoming queries */
	if((*runtime->callback_query)(repinfo.c, runtime->cb_arg, 
		NETEVENT_NOERROR, &repinfo)) {
		/* send immediate reply */
		comm_point_send_reply(&repinfo);
	}
	/* clear it again, in case copy not done properly */
	memset(&repinfo, 0, sizeof(repinfo));
}

/**
 * Perform callback for fake pending message.
 */
static void
fake_pending_callback(struct replay_runtime* runtime, 
	struct replay_moment* todo, int error)
{
	struct fake_pending* p = runtime->pending_list;
	struct comm_point c;
	memset(&c, 0, sizeof(c));
	if(!p) fatal_exit("No pending queries.");
	log_assert(todo->qname == NULL); /* or find that one */
	c.buffer = ldns_buffer_new(runtime->bufsize);
	c.type = comm_udp;
	if(p->transport == transport_tcp)
		c.type = comm_tcp;
	if(todo->evt_type == repevt_back_reply && todo->match) {
		fill_buffer_with_reply(c.buffer, todo->match, p->pkt);
	}	
	if((*p->callback)(&c, p->cb_arg, error, NULL)) {
		fatal_exit("unexpected: pending callback returned 1");
	}
	/* delete the pending item. */
	ldns_buffer_free(c.buffer);
	pending_list_delete(runtime, p);
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

/**
 * Perform actions or checks determined by the moment.
 * Also advances the time by one step.
 * @param runtime: scenario runtime information.
 */
static void
do_moment_and_advance(struct replay_runtime* runtime)
{
	struct replay_moment* mom;
	if(!runtime->now) {
		advance_moment(runtime);
		return;
	}
	log_info("testbound: do STEP %d %s", runtime->now->time_step, 
		repevt_string(runtime->now->evt_type));
	switch(runtime->now->evt_type) {
	case repevt_nothing:
		advance_moment(runtime);
		break;
	case repevt_front_query:
		/* advance moment before doing the step, so that the next
		   moment which may check some result of the mom step
		   can catch those results. */
		mom = runtime->now;
		advance_moment(runtime);
		fake_front_query(runtime, mom);
		break;
	case repevt_front_reply:
		if(runtime->answer_list) 
			log_err("testbound: There are unmatched answers.");
		fatal_exit("testbound: query answer not matched");
		break;
	case repevt_timeout:
		mom = runtime->now;
		advance_moment(runtime);
		fake_pending_callback(runtime, mom, NETEVENT_TIMEOUT);
		break;
	case repevt_back_reply:
		mom = runtime->now;
		advance_moment(runtime);
		fake_pending_callback(runtime, mom, NETEVENT_NOERROR);
		break;
	case repevt_back_query:
		/* Back queries are matched when they are sent out. */
		log_err("No query matching the current moment was sent.");
		fatal_exit("testbound: back query not matched");
		break;
	case repevt_error:
		mom = runtime->now;
		advance_moment(runtime);
		fake_pending_callback(runtime, mom, NETEVENT_CLOSED);
		break;
	default:
		fatal_exit("testbound: unknown event type %d", 
			runtime->now->evt_type);
	}
}

/** run the scenario in event callbacks */
static void
run_scenario(struct replay_runtime* runtime)
{
	struct entry* entry = NULL;
	struct fake_pending* pending = NULL;
	int max_rounds = 50;
	int rounds = 0;
	runtime->now = runtime->scenario->mom_first;
	log_info("testbound: entering fake runloop");
	do {
		/* if moment matches pending query do it. */
		/* else if moment matches given answer, do it */
		/* else if precoded_range matches pending, do it */
		/* else do the current moment */
		if(pending_matches_current(runtime, &entry, &pending)) {
			log_info("testbound: do STEP %d CHECK_OUT_QUERY", 
				runtime->now->time_step);
			advance_moment(runtime);
			if(entry->copy_id)
				answer_callback_from_entry(runtime, entry, 
				pending);
		} else if(runtime->answer_list && runtime->now && 
			runtime->now->evt_type == repevt_front_reply) {
			answer_check_it(runtime);			
			advance_moment(runtime);
		} else if(pending_matches_range(runtime, &entry, &pending)) {
			answer_callback_from_entry(runtime, entry, pending);
		} else {
			do_moment_and_advance(runtime);
		}
		log_info("testbound: end of event stage");
		rounds++;
		if(rounds > max_rounds)
			fatal_exit("testbound: too many rounds, it loops.");
	} while(runtime->now);

	if(runtime->pending_list) {
		fatal_exit("testbound: there are still messages pending.");
	}
	if(runtime->answer_list) {
		fatal_exit("testbound: there are unmatched answers.");
	}
	log_info("testbound: exiting fake runloop.");
}

/*********** Dummy routines ***********/

struct listen_dnsport* 
listen_create(struct comm_base* base, int ATTR_UNUSED(num_ifs), 
	const char* ATTR_UNUSED(ifs[]), int ATTR_UNUSED(port),
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
	runtime->bufsize = bufsize;
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

struct comm_base* 
comm_base_create()
{
	/* we return the runtime structure instead. */
	struct replay_runtime* runtime = (struct replay_runtime*)
		calloc(1, sizeof(struct replay_runtime));
	runtime->scenario = saved_scenario;
	return (struct comm_base*)runtime;
}

void 
comm_base_delete(struct comm_base* b)
{
	struct replay_runtime* runtime = (struct replay_runtime*)b;
	struct fake_pending* p, *np;
	struct replay_answer* a, *na;
	if(!runtime)
		return;
	runtime->scenario= NULL;
	p = runtime->pending_list;
	while(p) {
		np = p->next;
		delete_fake_pending(p);
		p = np;
	}
	a = runtime->answer_list;
	while(a) {
		na = a->next;
		delete_replay_answer(a);
		a = na;
	}
	free(runtime);
}

void 
comm_base_dispatch(struct comm_base* b)
{
	struct replay_runtime* runtime = (struct replay_runtime*)b;
	run_scenario(runtime);
}

void 
comm_base_exit(struct comm_base* ATTR_UNUSED(b))
{
	/* some sort of failure */
	fatal_exit("testbound: comm_base_exit was called.");
}

struct comm_signal* 
comm_signal_create(struct comm_base* base,
        void (*callback)(int, void*), void* cb_arg)
{
	struct replay_runtime* runtime = (struct replay_runtime*)base;
	runtime->sig_cb = callback;
	runtime->sig_cb_arg = cb_arg;
	return calloc(1, sizeof(struct comm_signal));
}

int 
comm_signal_bind(struct comm_signal* ATTR_UNUSED(comsig), int 
	ATTR_UNUSED(sig))
{
	return 1;
}

void 
comm_signal_delete(struct comm_signal* comsig)
{
	free(comsig);
}

void 
comm_point_send_reply(struct comm_reply* repinfo)
{
	struct replay_answer* ans = (struct replay_answer*)calloc(1,
		sizeof(struct replay_answer));
	ldns_status status;
	struct replay_runtime* runtime = (struct replay_runtime*)repinfo->c->ev;
	log_info("testbound: comm_point_send_reply fake");
	/* dump it into the todo list */
	log_assert(ans);
	memcpy(&ans->repinfo, repinfo, sizeof(struct comm_reply));
	ans->next = NULL;
	if(runtime->answer_last)
		runtime->answer_last->next = ans;
	else 	runtime->answer_list = ans;
	runtime->answer_last = ans;

	/* try to parse packet */
	status = ldns_buffer2pkt_wire(&ans->pkt, ans->repinfo.c->buffer);
	if(status != LDNS_STATUS_OK) {
		log_err("ldns error parsing packet: %s",
			ldns_get_errorstr_by_id(status));
		fatal_exit("Sending unparseable DNS replies to clients!");
	}
	log_pkt("reply pkt: ", ans->pkt);
}

void 
comm_point_drop_reply(struct comm_reply* repinfo)
{
	log_info("comm_point_drop_reply fake");
	if(repinfo->c) {
		ldns_buffer_free(repinfo->c->buffer);
		free(repinfo->c);
	}
}

struct outside_network* 
outside_network_create(struct comm_base* base, size_t bufsize, 
	size_t ATTR_UNUSED(num_ports), const char** ATTR_UNUSED(ifs), 
	int ATTR_UNUSED(num_ifs), int ATTR_UNUSED(do_ip4), 
	int ATTR_UNUSED(do_ip6), int ATTR_UNUSED(port_base))
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

void 
pending_udp_query(struct outside_network* outnet, ldns_buffer* packet,
	struct sockaddr_storage* addr, socklen_t addrlen, int timeout,
	comm_point_callback_t* callback, void* callback_arg)
{
	struct replay_runtime* runtime = (struct replay_runtime*)outnet->base;
	struct fake_pending* pend = (struct fake_pending*)calloc(1,
		sizeof(struct fake_pending));
	ldns_status status;
	log_assert(pend);
	pend->buffer = ldns_buffer_new(ldns_buffer_capacity(packet));
	log_assert(pend->buffer);
	ldns_buffer_write(pend->buffer, ldns_buffer_begin(packet),
		ldns_buffer_limit(packet));
	ldns_buffer_flip(pend->buffer);
	memcpy(&pend->addr, addr, addrlen);
	pend->addrlen = addrlen;
	pend->callback = callback;
	pend->cb_arg = callback_arg;
	pend->timeout = timeout;
	pend->transport = transport_udp;
	pend->pkt = NULL;
	status = ldns_buffer2pkt_wire(&pend->pkt, packet);
	if(status != LDNS_STATUS_OK) {
		log_err("ldns error parsing udp output packet: %s",
			ldns_get_errorstr_by_id(status));
		fatal_exit("Sending unparseable DNS packets to servers!");
	}
	log_pkt("pending udp pkt: ", pend->pkt);

	/* see if it matches the current moment */
	if(runtime->now && runtime->now->evt_type == repevt_back_query &&
		find_match(runtime->now->match, pend->pkt, pend->transport)) {
		log_info("testbound: matched pending to event. "
			"advance time between events.");
		log_info("testbound: do STEP %d %s", runtime->now->time_step,
			repevt_string(runtime->now->evt_type));
		advance_moment(runtime);
		/* still create the pending, because we need it to callback */
	} 
	log_info("testbound: created fake pending");
	/* add to list */
	pend->next = runtime->pending_list;
	runtime->pending_list = pend;
}

/*********** End of Dummy routines ***********/
