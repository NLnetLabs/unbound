/*
 * testcode/replay.h - store and use a replay of events for the DNS resolver.
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
 * Store and use a replay of events for the DNS resolver.
 * Used to test known scenarios to get known outcomes.
 *
 * <pre>
 * File format for replay files.
 *
 * ; comment line.
 * SCENARIO_BEGIN name_of_scenario
 * RANGE_BEGIN start_time end_time
 *    ; give ip of the virtual server, it matches any ip if not present.
 *    ADDRESS ip_address 
 *    match_entries
 * RANGE_END
 * ; more RANGE items.
 * ; go to the next moment
 * STEP time_step event_type [ADDRESS ip_address]
 * ; event_type can be:
 *	o NOTHING - nothing
 *	o QUERY - followed by entry
 *	o CHECK_ANSWER - followed by entry
 *	o CHECK_OUT_QUERY - followed by entry (if copy-id it is also reply).
 *	o REPLY - followed by entry
 *      o TIMEOUT
 *      o ERROR
 * ; following entry starts on the next line, ENTRY_BEGIN.
 * ; more STEP items
 * SCENARIO_END
 *
 *
 * ; Example file
 * SCENARIO_BEGIN Example scenario
 * RANGE_BEGIN 0 100
 *   ENTRY_BEGIN
 *   ; precoded answers to queries.
 *   ENTRY_END
 * END_RANGE
 * STEP 0 QUERY
 *   ENTRY_BEGIN
 *   ; query
 *   ENTRY_END
 * ; a query is sent out to the network by resolver.
 * ; precoded answer from range is returned.
 * ; algorithm will do precoded answers from RANGE immediately, except if
 * ; the next step specifically checks for that OUT_QUERY.
 * ; or if none of the precoded answers match.
 * STEP 1 CHECK_ANSWER
 *   ENTRY_BEGIN
 *   ; what the reply should look like
 *   ENTRY_END
 * ; successful termination. (if the answer was OK).
 * ; also, all answers must have been checked with CHECK_ANSWER.
 * ; and, no more pending out_queries (that have not been checked).
 * SCENARIO_END
 * 
 * </pre>
 */

#ifndef TESTCODE_REPLAY_H
#define TESTCODE_REPLAY_H
#include "config.h"
#include "util/netevent.h"
#include "testcode/ldns-testpkts.h"
struct replay_answer;
struct replay_moment;
struct replay_range;
struct fake_pending;

/**
 * A replay scenario.
 */
struct replay_scenario {
	/** name of replay scenario. malloced string. */
	char* title;

	/** The list of replay moments. Linked list. Time increases in list. */
	struct replay_moment* mom_first;
	/** The last element in list of replay moments. */
	struct replay_moment* mom_last;

	/** 
	 * List of matching answers. This is to ease replay scenario
	 * creation. It lists queries (to the network) and what answer
	 * should be returned. The matching answers are valid for a range
	 * of time steps. 
	 * So: timestep, parts of query, destination --> answer.
	 */
	struct replay_range* range_list;
};

/**
 * A replay moment.
 * Basically, it consists of events to a fake select() call.
 * This is a recording of an event that happens.
 * And if output is presented, what is done with that.
 */
struct replay_moment {
	/** 
	 * The replay time step number. Starts at 0, time is incremented 
	 * every time the fake select() is run. 
	 */
	int time_step;
	/** Next replay moment in list of replay moments. */
	struct replay_moment* mom_next;

	/** what happens this moment? */
	enum replay_event_type {
		/** nothing happens, as if this event is not there. */
		repevt_nothing,
		/** incoming query */
		repevt_front_query,
		/** test fails if reply to query does not match */
		repevt_front_reply,
		/** timeout */
		repevt_timeout,
		/** reply arrives from the network */
		repevt_back_reply,
		/** test fails if query to the network does not match */
		repevt_back_query,
		/** an error happens to outbound query */
		repevt_error
	} 
		/** variable with what is to happen this moment */
		evt_type;

	/** The sent packet must match this. Incoming events, the data. */
	struct entry* match;

	/** address that must be matched, or packet remote host address. */
	struct sockaddr_storage addr;
	/** length of addr, if 0, then any address will do */
	socklen_t addrlen;

	/** what pending query should timeout or is answered. or 
	 * NULL for last sent query. 
	 * Unused at this time.
	 */
	ldns_rr* qname;
};

/**
 * Range of timesteps, and canned replies to matching queries.
 */
struct replay_range {
	/** time range when this is valid. Including start and end step. */
	int start_step;
	/** end step of time range. */
	int end_step;
	/** address of where this range is served. */
	struct sockaddr_storage addr;
	/** length of addr, if 0, then any address will do */
	socklen_t addrlen;

	/** Matching list */
	struct entry* match;

	/** next in list of time ranges. */
	struct replay_range* next_range;
};

/**
 * Replay storage of runtime information.
 */
struct replay_runtime {
	/**
	 * The scenario
	 */
	struct replay_scenario* scenario;
	/**
	 * Current moment.
	 */
	struct replay_moment* now;

	/** 
	 * List of pending queries in order they were sent out. First
	 * one has been sent out most recently. Last one in list is oldest. 
	 */
	struct fake_pending* pending_list;

	/**
	 * List of answers to queries from clients. These need to be checked.
	 */
	struct replay_answer* answer_list;
	
	/** last element in answer list. */
	struct replay_answer* answer_last;

	/** callback to call for incoming queries */
	comm_point_callback_t* callback_query;
	/** user argument for incoming query callback */
	void *cb_arg;

	/** signal handler callback */
	void (*sig_cb)(int, void*);
	/** signal handler user arg */
	void *sig_cb_arg;
	/** time to exit cleanly */
	int exit_cleanly;

	/** size of buffers */
	size_t bufsize;
};

/**
 * Pending queries to network, fake replay version.
 */
struct fake_pending {
	/** what is important only that we remember the query, copied here. */
	ldns_buffer* buffer;
	/** and to what address this is sent to. */
	struct sockaddr_storage addr;
	/** len of addr */
	socklen_t addrlen;
	/** The callback function to call when answer arrives (or timeout) */
	comm_point_callback_t* callback;
	/** callback user argument */
	void* cb_arg;
	/** original timeout in seconds from 'then' */
	int timeout;

	/** next in pending list */
	struct fake_pending* next;
	/** the buffer parsed into a ldns_pkt */
	ldns_pkt* pkt;
	/** by what transport was the query sent out */
	enum transport_type transport;
	/** if this is a serviced query */
	int serviced;
	/** the runtime structure this is part of */
	struct replay_runtime* runtime;
};

/**
 * An answer that is pending to happen.
 */
struct replay_answer {
	/** Next in list */
	struct replay_answer* next;
	/** reply information */
	struct comm_reply repinfo;
	/** the answer preparsed as ldns pkt */
	ldns_pkt* pkt;
};

/**
 * Read a replay scenario from the file.
 * @param in: file to read from.
 * @param name: name to print in errors.
 * @param lineno: incremented for every line read.
 * @return: Scenario. NULL if no scenario read.
 */
struct replay_scenario* replay_scenario_read(FILE* in, const char* name, 
	int* lineno);

/**
 * Delete scenario.
 * @param scen: to delete.
 */
void replay_scenario_delete(struct replay_scenario* scen);

#endif /* TESTCODE_REPLAY_H */
