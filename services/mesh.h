/*
 * services/mesh.h - deal with mesh of query states and handle events for that.
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
 * This file contains functions to assist in dealing with a mesh of
 * query states. This mesh is supposed to be thread-specific.
 * It consists of query states (per qname, qtype, qclass) and connections
 * between query states and the super and subquery states, and replies to
 * send back to clients.
 */

#ifndef SERVICES_MESH_H
#define SERVICES_MESH_H

#include "util/rbtree.h"
#include "util/netevent.h"
#include "util/data/msgparse.h"
struct module_qstate;
struct mesh_state;
struct mesh_reply;
struct worker;
struct query_info;
struct reply_info;

/** 
 * Mesh of query states
 */
struct query_mesh {
	/** what worker this is a part of */
	struct worker* worker;
	/** set of runnable queries (mesh_state_ref*) */
	rbtree_t run;
	/** rbtree of all current queries */
	rbtree_t all;
	/** count of the number of mesh_reply entries */
	size_t reply_count;
	/** count of the number of mesh_states that have no mesh_replies
	 * i.e. are for internal use. */
	size_t internal_count;
};

/**
 * A mesh query state
 * Unique per qname, qtype, qclass.
 * And RD flag; in case a client turns it off.
 * And priming queries are different from ordinary queries (because of hints).
 */
struct mesh_state {
	/** node in query_mesh all tree, key is this struct */
	rbnode_t node;
	/** node in query_mesh runnable tree, key is this struct */
	rbnode_t run_node;
	/** unique identity for this mesh state: what is it for */
	/* Note that qstate qinfo is changed by iterator */

	/** if this is a priming query (with hints) */
	int is_priming;
	
	/** the query state */
	struct module_qstate* state;
	/** the list of replies to clients for the results */
	struct mesh_reply* reply_list;
	/** set of superstates (that want this state's result) 
	 * contains struct mesh_state_ref* */
	rbtree_t super_set;
	/** set of substates (that this state needs to continue)
	 * contains struct mesh_state_ref* */
	rbtree_t sub_set;
};

/**
 * Rbtree reference to a mesh_state.
 * Used in super_set and sub_set. 
 */
struct mesh_state_ref {
	/** node in rbtree for set, key is this structure */
	rbtree_t node;
	/** the mesh state */
	struct mesh_state* s;
};

/**
 * Reply to a client
 */
struct mesh_reply {
	/** next in reply list */
	struct mesh_reply* next;
	/** the query reply destination, packet buffer and where to send. */
	struct comm_reply query_reply;
	/** edns data from query */
	struct edns_data edns;
	/** id of query, in network byteorder. */
	uint16_t qid;
	/** flags of query, for reply flags */
	uint16_t qflags;
};

/**
 * Allocate mesh, to empty.
 * @param worker: what worker it is part of.
 * @return mesh: the new mesh or NULL on error.
 */
struct query_mesh* mesh_create(struct worker* worker);

/**
 * Delete mesh, and all query states and replies in it.
 * @param mesh: the mesh to delete.
 */
void mesh_delete(struct query_mesh* mesh);

/**
 * New query incoming from clients. Create new query state if needed, and
 * add mesh_reply to it. Returns error to client on malloc failures.
 * @param mesh: the mesh.
 * @param qinfo: query from client.
 * @param qflags: flags from client query.
 * @param edns: edns data from client query.
 * @param rep: where to reply to.
 * @param id: query id to reply with.
 */
void mesh_new_client(struct query_mesh* mesh, struct query_info* qinfo,
	uint16_t qflags, struct edns_data* edns, struct comm_reply* rep, 
	uint16_t id);

/**
 * Detach-subqueries.
 * Remove all sub-query references from this query state.
 * Keeps sub-query-super-references correct.
 * @param qstate: used to find mesh state.
 */
void mesh_detach_subs(struct module_qstate* qstate);

/**
 * Attach subquery.
 * Creates it if it does not exist already.
 * Keeps sub and super references correct.
 * Pass if it is priming query or not.
 * return:
 * 	o if error (malloc) happened.
 * 	o need to initialise the new state (module init; it is a new state).
 * 	  so that the next run of the query with this module is successful.
 * 	o no init needed, attachment successful.
 *
 * @param qstate: the state to find mesh state, and that wants to receive
 * 	the results from the new subquery.
 * @param qinfo: what to query for (copied).
 * @param qflags: what flags to use (RD flag or not).
 * @param prime: if it is a priming query.
 * @param newq: If the new subquery needs initialisation, it is returned,
 * 	otherwise NULL is returned.
 * @return: false on error, true if success (and init may be needed).
 */
int mesh_attach_sub(struct module_qstate* qstate, struct query_info* qinfo,
	uint16_t qflags, int prime, struct module_qstate** newq);

/**
 * Query state is done, send messages to reply entries.
 * Encode messages using reply entry values and the querystate (with original
 * qinfo), using given reply_info.
 * Pass errcode != 0 if an error reply is needed.
 * If no reply entries, nothing is done.
 * Must be called before a module can module_finished or return module_error.
 * The module must handle the super query states itself as well.
 *
 * @param qstate: used for original query info. And to find mesh info.
 * @param rcode: if not 0 (NOERROR) an error is sent back (and rep ignored).
 * @param rep: reply to encode and send back to clients.
 */
void mesh_query_done(struct module_qstate* qstate, int rcode, 
	struct reply_info* rep);

/**
 * Get a callback for the super query states that are interested in the 
 * results from this query state. These can then be changed for error 
 * or results.
 * Must be called befor a module can module_finished or return module_error.
 * After finishing or module error, the super query states become runnable
 * with event module_event_pass.
 *
 * @param qstate: the state that has results, used to find mesh state.
 * @param id: module id.
 * @param rcode: rcode to pass to callback, for easier error passing to 
 * 	parents.
 * @param cb: callback function. Called as
 * 	cb(qstate, id, super_qstate, rcode) for every super query state.
 */
void mesh_walk_supers(struct module_qstate* qstate, int id, int rcode,
	void (*cb)(struct module_qstate*, int, struct module_qstate*, int));

#endif /* SERVICES_MESH_H */
