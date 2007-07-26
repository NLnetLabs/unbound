/*
 * util/module.h - DNS handling module interface
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
 * This file contains the interface for DNS handling modules.
 */

#ifndef UTIL_MODULE_H
#define UTIL_MODULE_H
#include "util/storage/lruhash.h"
#include "util/data/msgreply.h"
#include "util/data/msgparse.h"
struct alloc_cache;
struct rrset_cache;
struct config_file;
struct slabhash;
struct query_info;
struct edns_data;
struct region;
struct worker;
struct module_qstate;
struct ub_randstate;
struct mesh_area;
struct mesh_state;

/** Maximum number of modules in operation */
#define MAX_MODULE 2

/**
 * Module environment.
 * Services and data provided to the module.
 */
struct module_env {
	/* --- data --- */
	/** config file with config options */
	struct config_file* cfg;
	/** shared message cache */
	struct slabhash* msg_cache;
	/** shared rrset cache */
	struct rrset_cache* rrset_cache;
	/** shared infrastructure cache (edns, lameness) */
	struct infra_cache* infra_cache;

	/* --- services --- */
	/** 
	 * Direct access to the network, this packet gets sent to destination.
	 * Send DNS query to server. operate() should return with wait_reply.
	 * Later on a callback will cause operate() to be called with event
	 * timeout or reply. Replied packet is then in the query buffer.
	 * @param pkt: packet to send.
	 * @param addr: where to.
	 * @param addrlen: length of addr.
	 * @param timeout: seconds to wait until timeout.
	 * @param q: wich query state to reactivate upon return.
	 * @param use_tcp: set to true to send over TCP. 0 for UDP.
	 * @return: false on failure (memory or socket related). no query was
	 *	sent.
	 */
	int (*send_packet)(ldns_buffer* pkt, struct sockaddr_storage* addr,
		socklen_t addrlen, int timeout, struct module_qstate* q,
		int use_tcp);

	/** 
	 * Send serviced DNS query to server. UDP/TCP and EDNS is handled.
	 * operate() should return with wait_reply. Later on a callback 
	 * will cause operate() to be called with event timeout or reply.
	 * The time until a timeout is calculated from roundtrip timing,
	 * several UDP retries are attempted.
	 * @param qname: query name. (host order)
	 * @param qnamelen: length in bytes of qname, including trailing 0.
	 * @param qtype: query type. (host order)
	 * @param qclass: query class. (host order)
	 * @param flags: host order flags word, with opcode and CD bit.
	 * @param dnssec: if set, EDNS record will have DO bit set.
	 * @param addr: where to.
	 * @param addrlen: length of addr.
	 * @param q: wich query state to reactivate upon return.
	 * @return: false on failure (memory or socket related). no query was
	 *	sent. Or returns an outbound entry with qsent and qstate set.
	 *	This outbound_entry will be used on later module invocations
	 *	that involve this query (timeout, error or reply).
	 */
	struct outbound_entry* (*send_query)(uint8_t* qname, size_t qnamelen, 
		uint16_t qtype, uint16_t qclass, uint16_t flags, int dnssec, 
		struct sockaddr_storage* addr, socklen_t addrlen, 
		struct module_qstate* q);

	/**
	 * Detach-subqueries.
	 * Remove all sub-query references from this query state.
	 * Keeps super-references of those sub-queries correct.
	 * Updates stat items in mesh_area structure.
	 * @param qstate: used to find mesh state.
	 */
	void (*detach_subs)(struct module_qstate* qstate);

	/**
	 * Attach subquery.
	 * Creates it if it does not exist already.
	 * Keeps sub and super references correct.
	 * Updates stat items in mesh_area structure.
	 * Pass if it is priming query or not.
	 * return:
	 * o if error (malloc) happened.
	 * o need to initialise the new state (module init; it is a new state).
	 *   so that the next run of the query with this module is successful.
	 * o no init needed, attachment successful.
	 * 
	 * @param qstate: the state to find mesh state, and that wants to 
	 * 	receive the results from the new subquery.
	 * @param qinfo: what to query for (copied).
	 * @param qflags: what flags to use (RD flag or not).
	 * @param prime: if it is a (stub) priming query.
	 * @param newq: If the new subquery needs initialisation, it is 
	 * 	returned, otherwise NULL is returned.
	 * @return: false on error, true if success (and init may be needed).
	 */ 
	int (*attach_sub)(struct module_qstate* qstate, 
		struct query_info* qinfo, uint16_t qflags, int prime, 
		struct module_qstate** newq);

	/**
	 * Kill newly attached sub. If attach_sub returns newq for 
	 * initialisation, but that fails, then this routine will cleanup and
	 * delete the fresly created sub.
	 * @param newq: the new subquery that is no longer needed.
	 * 	It is removed.
	 */
	void (*kill_sub)(struct module_qstate* newq);

	/**
	 * Query state is done, send messages to reply entries.
	 * Encode messages using reply entry values and the querystate 
	 * (with original qinfo), using given reply_info.
	 * Pass errcode != 0 if an error reply is needed.
	 * If no reply entries, nothing is done.
	 * Must be called before a module can module_finished or return 
	 * module_error.
	 * The module must handle the super query states itself as well.
	 * 
	 * @param qstate: used for original query info. And to find mesh info.
	 * @param rcode: if not 0 (NOERROR) an error is sent back (and 
	 * 	rep ignored).
	 * @param rep: reply to encode and send back to clients.
	 */
	void (*query_done)(struct module_qstate* qstate, int rcode,
	        struct reply_info* rep);

	/**
	 * Get a callback for the super query states that are interested in the 
	 * results from this query state. These can then be changed for error 
	 * or results.
	 * Must be called befor a module can module_finished or return 
	 * module_error.  After finishing or module error, the super 
	 * query states become runnable with event module_event_pass.
	 * 
	 * @param qstate: the state that has results, used to find mesh state.
	 * @param id: module id.
	 * @param cb: callback function. Called as
	 * 	cb(qstate, id, super_qstate) for every super qstate.
	 */
	void (*walk_supers)(struct module_qstate* qstate, int id, 
		void (*cb)(struct module_qstate*, int, struct module_qstate*));

	/**
	 * Detect if adding a dependency for qstate on name,type,class will
	 * create a dependency cycle.
	 * @param qstate: given mesh querystate.
	 * @param qinfo: query info for dependency. Assumed RDflag and not
	 * 	priming.
	 * @return true if the name,type,class exists and the given 
	 * 	qstate mesh exists as a dependency of that name. Thus 
	 * 	if qstate becomes dependent on name,type,class then a 
	 * 	cycle is created.
	 */
	int (*detect_cycle)(struct module_qstate* qstate, 
		struct query_info* qinfo);

	/** region for temporary usage. May be cleared after operate() call. */
	struct region* scratch;
	/** internal data for daemon - worker thread. */
	struct worker* worker;
	/** mesh area with query state dependencies */
	struct mesh_area* mesh;
	/** allocation service */
	struct alloc_cache* alloc;
	/** random table to generate random numbers */
	struct ub_randstate* rnd;
	/** module specific data. indexed by module id. */
	void* modinfo[MAX_MODULE];
};

/**
 * External visible states of the module state machine 
 * Modules may also have an internal state.
 * Modules are supposed to run to completion or until blocked.
 */
enum module_ext_state {
	/** initial state - new query */
	module_state_initial = 0,
	/** waiting for reply to outgoing network query */
	module_wait_reply,
	/** module is waiting for another module */
	module_wait_module,
	/** module is waiting for sub-query */
	module_wait_subquery,
	/** module could not finish the query */
	module_error,
	/** module is finished with query */
	module_finished
};

/**
 * Events that happen to modules, that start or wakeup modules.
 */
enum module_ev {
	/** new query */
	module_event_new = 0,
	/** query passed by other module */
	module_event_pass,
	/** reply inbound from server */
	module_event_reply,
	/** no reply, timeout or other error */
	module_event_noreply,
	/** error */
	module_event_error
};

/**
 * Module state, per query.
 */
struct module_qstate {
	/** which query is being answered: name, type, class */
	struct query_info qinfo;
	/** flags uint16 from query */
	uint16_t query_flags;

	/** comm_reply contains server replies */
	struct comm_reply* reply;
	/** region for this query. Cleared when query process finishes. */
	struct region* region;

	/** which module is executing */
	int curmod;
	/** module states */
	enum module_ext_state ext_state[MAX_MODULE];
	/** module specific data for query. indexed by module id. */
	void* minfo[MAX_MODULE];
	/** environment for this query */
	struct module_env* env;
	/** mesh related information for this query */
	struct mesh_state* mesh_info;
};

/** 
 * Module functionality block
 */
struct module_func_block {
	/** text string name of module */
	char* name;

	/** 
	 * init the module. Called once for the global state.
	 * This is the place to apply settings from the config file.
	 * @param env: module environment.
	 * @param id: module id number.
	 * return: 0 on error
	 */
	int (*init)(struct module_env* env, int id);
	/**
	 * de-init, delete, the module. Called once for the global state.
	 * @param env: module environment.
	 * @param id: module id number.
	 */
	void (*deinit)(struct module_env* env, int id);

	/**
	 * accept a new query, or work further on existing query.
	 * Changes the qstate->ext_state to be correct on exit.
	 * @param ev: event that causes the module state machine to 
	 *	(re-)activate.
	 * @param qstate: the query state. 
	 * @param id: module id number that operate() is called on. 
	 * @param outbound: if not NULL this event is due to the reply/timeout
	 *	or error on this outbound query.
	 * @return: if at exit the ext_state is:
	 *	o wait_module: next module is started. (with pass event).
	 *	o error or finished: previous module is resumed.
	 *	o otherwise it waits until that event happens (assumes
	 *	  the service routine to make subrequest or send message
	 *	  have been called.
	 */
	void (*operate)(struct module_qstate* qstate, enum module_ev event, 
		int id, struct outbound_entry* outbound);
	/**
	 * clear module specific data
	 */
	void (*clear)(struct module_qstate* qstate, int id);
};

/** 
 * Debug utility: module external qstate to string 
 * @param s: the state value.
 * @return descriptive string.
 */
const char* strextstate(enum module_ext_state s);

/** 
 * Debug utility: module event to string 
 * @param e: the module event value.
 * @return descriptive string.
 */
const char* strmodulevent(enum module_ev e);

#endif /* UTIL_MODULE_H */
