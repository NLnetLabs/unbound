/*
 * services/mesh.c - deal with mesh of query states and handle events for that.
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
#include "config.h"
#include "services/mesh.h"
#include "services/outbound_list.h"
#include "services/cache/dns.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/module.h"
#include "util/regional.h"
#include "util/data/msgencode.h"
#include "util/timehist.h"
#include "util/fptr_wlist.h"
#include "util/alloc.h"

int
mesh_state_compare(const void* ap, const void* bp)
{
	struct mesh_state* a = (struct mesh_state*)ap;
	struct mesh_state* b = (struct mesh_state*)bp;

	if(a->s.is_priming && !b->s.is_priming)
		return -1;
	if(!a->s.is_priming && b->s.is_priming)
		return 1;

	if((a->s.query_flags&BIT_RD) && !(b->s.query_flags&BIT_RD))
		return -1;
	if(!(a->s.query_flags&BIT_RD) && (b->s.query_flags&BIT_RD))
		return 1;

	if((a->s.query_flags&BIT_CD) && !(b->s.query_flags&BIT_CD))
		return -1;
	if(!(a->s.query_flags&BIT_CD) && (b->s.query_flags&BIT_CD))
		return 1;

	return query_info_compare(&a->s.qinfo, &b->s.qinfo);
}

int
mesh_state_ref_compare(const void* ap, const void* bp)
{
	struct mesh_state_ref* a = (struct mesh_state_ref*)ap;
	struct mesh_state_ref* b = (struct mesh_state_ref*)bp;
	return mesh_state_compare(a->s, b->s);
}

struct mesh_area* 
mesh_create(struct module_stack* stack, struct module_env* env)
{
	struct mesh_area* mesh = calloc(1, sizeof(struct mesh_area));
	if(!mesh) {
		log_err("mesh area alloc: out of memory");
		return NULL;
	}
	mesh->histogram = timehist_setup();
	if(!mesh->histogram) {
		free(mesh);
		log_err("mesh area alloc: out of memory");
		return NULL;
	}
	mesh->mods = *stack;
	mesh->env = env;
	rbtree_init(&mesh->run, &mesh_state_compare);
	rbtree_init(&mesh->all, &mesh_state_compare);
	mesh->num_reply_addrs = 0;
	mesh->num_reply_states = 0;
	mesh->num_detached_states = 0;
	return mesh;
}

/** help mesh delete delete mesh states */
static void
mesh_delete_helper(rbnode_t* n, void* ATTR_UNUSED(arg))
{
	struct mesh_state* mstate = (struct mesh_state*)n->key;
	mesh_state_cleanup(mstate);
}

void 
mesh_delete(struct mesh_area* mesh)
{
	if(!mesh)
		return;
	/* free all query states */
	traverse_postorder(&mesh->all, &mesh_delete_helper, NULL);
	timehist_delete(mesh->histogram);
	free(mesh);
}

void mesh_new_client(struct mesh_area* mesh, struct query_info* qinfo,
        uint16_t qflags, struct edns_data* edns, struct comm_reply* rep,
        uint16_t qid)
{
	struct mesh_state* s = mesh_area_find(mesh, qinfo, qflags, 0);
	int was_detached = 0;
	int was_noreply = 0;
	int added = 0;
	/* see if it already exists, if not, create one */
	if(!s) {
		struct rbnode_t* n;
		s = mesh_state_create(mesh->env,qinfo, qflags, 0);
		if(!s) {
			log_err("mesh_state_create: out of memory; SERVFAIL");
			error_encode(rep->c->buffer, LDNS_RCODE_SERVFAIL,
				qinfo, qid, qflags, edns);
			comm_point_send_reply(rep);
			return;
		}
		n = rbtree_insert(&mesh->all, &s->node);
		log_assert(n != NULL);
		/* set detached (it is now) */
		mesh->num_detached_states++;
		added = 1;
	}
	if(!s->reply_list && !s->cb_list && s->super_set.count == 0)
		was_detached = 1;
	if(!s->reply_list && !s->cb_list)
		was_noreply = 1;
	/* add reply to s */
	if(!mesh_state_add_reply(s, edns, rep, qid, qflags)) {
			log_err("mesh_new_client: out of memory; SERVFAIL");
			error_encode(rep->c->buffer, LDNS_RCODE_SERVFAIL,
				qinfo, qid, qflags, edns);
			comm_point_send_reply(rep);
			if(added)
				mesh_state_delete(&s->s);
			return;
	}
	/* update statistics */
	if(was_detached) {
		log_assert(mesh->num_detached_states > 0);
		mesh->num_detached_states--;
	}
	if(was_noreply) {
		mesh->num_reply_states ++;
	}
	mesh->num_reply_addrs++;
	if(added)
		mesh_run(mesh, s, module_event_new, NULL);
}

int 
mesh_new_callback(struct mesh_area* mesh, struct query_info* qinfo,
	uint16_t qflags, struct edns_data* edns, ldns_buffer* buf, 
	uint16_t qid, mesh_cb_func_t cb, void* cb_arg)
{
	struct mesh_state* s = mesh_area_find(mesh, qinfo, qflags, 0);
	int was_detached = 0;
	int was_noreply = 0;
	int added = 0;
	/* see if it already exists, if not, create one */
	if(!s) {
		struct rbnode_t* n;
		s = mesh_state_create(mesh->env,qinfo, qflags, 0);
		if(!s) {
			return 0;
		}
		n = rbtree_insert(&mesh->all, &s->node);
		log_assert(n != NULL);
		/* set detached (it is now) */
		mesh->num_detached_states++;
		added = 1;
	}
	if(!s->reply_list && !s->cb_list && s->super_set.count == 0)
		was_detached = 1;
	if(!s->reply_list && !s->cb_list)
		was_noreply = 1;
	/* add reply to s */
	if(!mesh_state_add_cb(s, edns, buf, cb, cb_arg, qid, qflags)) {
			if(added)
				mesh_state_delete(&s->s);
			return 0;
	}
	/* update statistics */
	if(was_detached) {
		log_assert(mesh->num_detached_states > 0);
		mesh->num_detached_states--;
	}
	if(was_noreply) {
		mesh->num_reply_states ++;
	}
	mesh->num_reply_addrs++;
	if(added)
		mesh_run(mesh, s, module_event_new, NULL);
	return 1;
}

void mesh_report_reply(struct mesh_area* mesh, struct outbound_entry* e,
        int is_ok, struct comm_reply* reply)
{
	e->qstate->reply = reply;
	mesh_run(mesh, e->qstate->mesh_info,
		is_ok?module_event_reply:module_event_noreply, e);
}

struct mesh_state* 
mesh_state_create(struct module_env* env, struct query_info* qinfo, 
	uint16_t qflags, int prime)
{
	struct regional* region = alloc_reg_obtain(env->alloc);
	struct mesh_state* mstate;
	int i;
	if(!region)
		return NULL;
	mstate = (struct mesh_state*)regional_alloc(region, 
		sizeof(struct mesh_state));
	if(!mstate) {
		alloc_reg_release(env->alloc, region);
		return NULL;
	}
	memset(mstate, 0, sizeof(*mstate));
	mstate->node = *RBTREE_NULL;
	mstate->run_node = *RBTREE_NULL;
	mstate->node.key = mstate;
	mstate->run_node.key = mstate;
	mstate->reply_list = NULL;
	rbtree_init(&mstate->super_set, &mesh_state_ref_compare);
	rbtree_init(&mstate->sub_set, &mesh_state_ref_compare);
	mstate->num_activated = 0;
	/* init module qstate */
	mstate->s.qinfo.qtype = qinfo->qtype;
	mstate->s.qinfo.qclass = qinfo->qclass;
	mstate->s.qinfo.qname_len = qinfo->qname_len;
	mstate->s.qinfo.qname = regional_alloc_init(region, qinfo->qname,
		qinfo->qname_len);
	if(!mstate->s.qinfo.qname) {
		alloc_reg_release(env->alloc, region);
		return NULL;
	}
	/* remove all weird bits from qflags */
	mstate->s.query_flags = (qflags & (BIT_RD|BIT_CD));
	mstate->s.is_priming = prime;
	mstate->s.reply = NULL;
	mstate->s.region = region;
	mstate->s.curmod = 0;
	mstate->s.return_msg = 0;
	mstate->s.return_rcode = LDNS_RCODE_NOERROR;
	mstate->s.env = env;
	mstate->s.mesh_info = mstate;
	/* init modules */
	for(i=0; i<env->mesh->mods.num; i++) {
		mstate->s.minfo[i] = NULL;
		mstate->s.ext_state[i] = module_state_initial;
	}
	return mstate;
}

void 
mesh_state_cleanup(struct mesh_state* mstate)
{
	struct mesh_area* mesh;
	int i;
	if(!mstate)
		return;
	/* de-init modules */
	mesh = mstate->s.env->mesh;
	for(i=0; i<mesh->mods.num; i++) {
		log_assert(fptr_whitelist_mod_clear(mesh->mods.mod[i]->clear));
		(*mesh->mods.mod[i]->clear)(&mstate->s, i);
		mstate->s.minfo[i] = NULL;
		mstate->s.ext_state[i] = module_finished;
	}
	alloc_reg_release(mstate->s.env->alloc, mstate->s.region);
}

void 
mesh_state_delete(struct module_qstate* qstate)
{
	struct mesh_area* mesh;
	struct mesh_state_ref* super, ref;
	struct mesh_state* mstate;
	if(!qstate)
		return;
	mstate = qstate->mesh_info;
	mesh = mstate->s.env->mesh;
	mesh_detach_subs(&mstate->s);
	if(!mstate->reply_list && !mstate->cb_list
		&& mstate->super_set.count == 0) {
		log_assert(mesh->num_detached_states > 0);
		mesh->num_detached_states--;
	}
	if(mstate->reply_list || mstate->cb_list) {
		log_assert(mesh->num_reply_states > 0);
		mesh->num_reply_states--;
	}
	ref.node.key = &ref;
	ref.s = mstate;
	RBTREE_FOR(super, struct mesh_state_ref*, &mstate->super_set) {
		(void)rbtree_delete(&super->s->sub_set, &ref);
	}
	(void)rbtree_delete(&mesh->run, mstate);
	(void)rbtree_delete(&mesh->all, mstate);
	mesh_state_cleanup(mstate);
}

void mesh_detach_subs(struct module_qstate* qstate)
{
	struct mesh_area* mesh = qstate->env->mesh;
	struct mesh_state_ref* ref, lookup;
	struct rbnode_t* n;
	lookup.node.key = &lookup;
	lookup.s = qstate->mesh_info;
	RBTREE_FOR(ref, struct mesh_state_ref*, &qstate->mesh_info->sub_set) {
		n = rbtree_delete(&ref->s->super_set, &lookup);
		log_assert(n != NULL); /* must have been present */
		if(!ref->s->reply_list && !ref->s->cb_list
			&& ref->s->super_set.count == 0) {
			mesh->num_detached_states++;
			log_assert(mesh->num_detached_states + 
				mesh->num_reply_states <= mesh->all.count);
		}
	}
	rbtree_init(&qstate->mesh_info->sub_set, &mesh_state_ref_compare);
}

int mesh_attach_sub(struct module_qstate* qstate, struct query_info* qinfo,
        uint16_t qflags, int prime, struct module_qstate** newq)
{
	/* find it, if not, create it */
	struct mesh_area* mesh = qstate->env->mesh;
	struct mesh_state* sub = mesh_area_find(mesh, qinfo, qflags, prime);
	if(!sub) {
		struct rbnode_t* n;
		/* create a new one */
		sub = mesh_state_create(qstate->env, qinfo, qflags, prime);
		if(!sub) {
			log_err("mesh_attach_sub: out of memory");
			return 0;
		}
		n = rbtree_insert(&mesh->all, &sub->node);
		log_assert(n != NULL);
		/* set detached (it is now) */
		mesh->num_detached_states++;
		/* set new query state to run */
		n = rbtree_insert(&mesh->run, &sub->run_node);
		log_assert(n != NULL);
		*newq = &sub->s;
	} else
		*newq = NULL;
	if(!mesh_state_attachment(qstate->mesh_info, sub))
		return 0;
	if(!sub->reply_list && !sub->cb_list && sub->super_set.count == 1) {
		/* it used to be detached, before this one got added */
		log_assert(mesh->num_detached_states > 0);
		mesh->num_detached_states--;
	}
	/* *newq will be run when inited after the current module stops */
	return 1;
}

int mesh_state_attachment(struct mesh_state* super, struct mesh_state* sub)
{
	struct rbnode_t* n;
	struct mesh_state_ref* subref; /* points to sub, inserted in super */
	struct mesh_state_ref* superref; /* points to super, inserted in sub */
	if( !(subref = regional_alloc(super->s.region,
		sizeof(struct mesh_state_ref))) ||
		!(superref = regional_alloc(sub->s.region,
		sizeof(struct mesh_state_ref))) ) {
		log_err("mesh_state_attachment: out of memory");
		return 0;
	}
	superref->node.key = superref;
	superref->s = super;
	subref->node.key = subref;
	subref->s = sub;
	n = rbtree_insert(&sub->super_set, &superref->node);
	log_assert(n != NULL);
	n = rbtree_insert(&super->sub_set, &subref->node);
	log_assert(n != NULL);
	return 1;
}

/** subtract timers and the values do not overflow or become negative */
static void
timeval_subtract(struct timeval* d, struct timeval* end, struct timeval* start)
{
#ifndef S_SPLINT_S
	d->tv_sec = end->tv_sec - start->tv_sec;
	while(end->tv_usec < start->tv_usec) {
		end->tv_usec += 1000000;
		d->tv_sec--;
	}
	d->tv_usec = end->tv_usec - start->tv_usec;
#endif
}

/** add timers and the values do not overflow or become negative */
static void
timeval_add(struct timeval* d, struct timeval* add)
{
#ifndef S_SPLINT_S
	d->tv_sec += add->tv_sec;
	d->tv_usec += add->tv_usec;
	while(d->tv_usec > 1000000 ) {
		d->tv_usec -= 1000000;
		d->tv_sec++;
	}
#endif
}

/** divide sum of timers to get average */
static void
timeval_divide(struct timeval* avg, struct timeval* sum, size_t d)
{
#ifndef S_SPLINT_S
	size_t leftover;
	if(d == 0) {
		avg->tv_sec = 0;
		avg->tv_usec = 0;
		return;
	}
	avg->tv_sec = sum->tv_sec / d;
	avg->tv_usec = sum->tv_usec / d;
	/* handle fraction from seconds divide */
	leftover = sum->tv_sec - avg->tv_sec*d;
	avg->tv_usec += (leftover*1000000)/d;
#endif
}

/**
 * callback results to mesh cb entry
 * @param m: mesh state to send it for.
 * @param rcode: if not 0, error code.
 * @param rep: reply to send (or NULL if rcode is set).
 * @param r: callback entry
 */
static void
mesh_do_callback(struct mesh_state* m, int rcode, struct reply_info* rep,
	struct mesh_cb* r)
{
	int secure;
	/* bogus messages are not made into servfail, sec_status passed 
	 * to the callback function */
	if(rep && rep->security == sec_status_secure)
		secure = 1;
	else	secure = 0;
	if(!rep && rcode == LDNS_RCODE_NOERROR)
		rcode = LDNS_RCODE_SERVFAIL;
	/* send the reply */
	if(rcode) {
		(*r->cb)(r->cb_arg, rcode, r->buf, sec_status_unchecked);
	} else {
		size_t udp_size = r->edns.udp_size;
		ldns_buffer_clear(r->buf);
		r->edns.edns_version = EDNS_ADVERTISED_VERSION;
		r->edns.udp_size = EDNS_ADVERTISED_SIZE;
		r->edns.ext_rcode = 0;
		r->edns.bits &= EDNS_DO;
		if(!reply_info_answer_encode(&m->s.qinfo, rep, r->qid, 
			r->qflags, r->buf, 0, 1, 
			m->s.env->scratch, udp_size, &r->edns, 
			(int)(r->edns.bits & EDNS_DO), secure)) 
		{
			(*r->cb)(r->cb_arg, LDNS_RCODE_SERVFAIL, r->buf,
				sec_status_unchecked);
		}
		else	(*r->cb)(r->cb_arg, LDNS_RCODE_NOERROR, r->buf,
				rep->security);
	}
	m->s.env->mesh->num_reply_addrs--;
}

/**
 * Send reply to mesh reply entry
 * @param m: mesh state to send it for.
 * @param rcode: if not 0, error code.
 * @param rep: reply to send (or NULL if rcode is set).
 * @param r: reply entry
 */
static void
mesh_send_reply(struct mesh_state* m, int rcode, struct reply_info* rep,
	struct mesh_reply* r)
{
	struct timeval end_time;
	int secure;
	/* examine security status */
	if(m->s.env->need_to_validate && !(r->qflags&BIT_CD) && rep && 
		rep->security <= sec_status_bogus) {
		rcode = LDNS_RCODE_SERVFAIL;
	}
	if(rep && rep->security == sec_status_secure)
		secure = 1;
	else	secure = 0;
	if(!rep && rcode == LDNS_RCODE_NOERROR)
		rcode = LDNS_RCODE_SERVFAIL;
	/* send the reply */
	if(rcode) {
		error_encode(r->query_reply.c->buffer, rcode, &m->s.qinfo,
			r->qid, r->qflags, &r->edns);
		comm_point_send_reply(&r->query_reply);
	} else {
		size_t udp_size = r->edns.udp_size;
		r->edns.edns_version = EDNS_ADVERTISED_VERSION;
		r->edns.udp_size = EDNS_ADVERTISED_SIZE;
		r->edns.ext_rcode = 0;
		r->edns.bits &= EDNS_DO;
		if(!reply_info_answer_encode(&m->s.qinfo, rep, r->qid, 
			r->qflags, r->query_reply.c->buffer, 0, 1, 
			m->s.env->scratch, udp_size, &r->edns, 
			(int)(r->edns.bits & EDNS_DO), secure)) 
		{
			error_encode(r->query_reply.c->buffer, 
				LDNS_RCODE_SERVFAIL, &m->s.qinfo, r->qid, 
				r->qflags, &r->edns);
		}
		comm_point_send_reply(&r->query_reply);
	}
	/* account */
	m->s.env->mesh->num_reply_addrs--;
	if(gettimeofday(&end_time, NULL) < 0) {
		log_err("gettimeofday: %s", strerror(errno));
		return;
	} else {
		struct timeval duration;
		timeval_subtract(&duration, &end_time, &r->start_time);
		verbose(VERB_ALGO, "query took %d.%6.6d sec",
			(int)duration.tv_sec, (int)duration.tv_usec);
		m->s.env->mesh->replies_sent++;
		timeval_add(&m->s.env->mesh->replies_sum_wait, &duration);
		timehist_insert(m->s.env->mesh->histogram, &duration);
	}
}

void mesh_query_done(struct mesh_state* mstate)
{
	struct mesh_reply* r;
	struct mesh_cb* c;
	struct reply_info* rep = (mstate->s.return_msg?
		mstate->s.return_msg->rep:NULL);
	for(r = mstate->reply_list; r; r = r->next) {
		mesh_send_reply(mstate, mstate->s.return_rcode, rep, r);
	}
	for(c = mstate->cb_list; c; c = c->next) {
		mesh_do_callback(mstate, mstate->s.return_rcode, rep, c);
	}
}

void mesh_walk_supers(struct mesh_area* mesh, struct mesh_state* mstate)
{
	struct mesh_state_ref* ref;
	RBTREE_FOR(ref, struct mesh_state_ref*, &mstate->super_set)
	{
		/* make super runnable */
		(void)rbtree_insert(&mesh->run, &ref->s->run_node);
		/* callback the function to inform super of result */
		log_assert(fptr_whitelist_mod_inform_super(
			mesh->mods.mod[ref->s->s.curmod]->inform_super));
		(*mesh->mods.mod[ref->s->s.curmod]->inform_super)(&mstate->s, 
			ref->s->s.curmod, &ref->s->s);
	}
}

struct mesh_state* mesh_area_find(struct mesh_area* mesh,
	struct query_info* qinfo, uint16_t qflags, int prime)
{
	struct mesh_state key;
	struct mesh_state* result;

	key.node.key = &key;
	key.s.is_priming = prime;
	key.s.qinfo = *qinfo;
	key.s.query_flags = qflags;
	
	result = (struct mesh_state*)rbtree_search(&mesh->all, &key);
	return result;
}

int mesh_state_add_cb(struct mesh_state* s, struct edns_data* edns,
        ldns_buffer* buf, mesh_cb_func_t cb, void* cb_arg,
	uint16_t qid, uint16_t qflags)
{
	struct mesh_cb* r = regional_alloc(s->s.region, 
		sizeof(struct mesh_cb));
	if(!r)
		return 0;
	r->buf = buf;
	r->cb = cb;
	r->cb_arg = cb_arg;
	r->edns = *edns;
	r->qid = qid;
	r->qflags = qflags;
	r->next = s->cb_list;
	s->cb_list = r;
	return 1;

}

int mesh_state_add_reply(struct mesh_state* s, struct edns_data* edns,
        struct comm_reply* rep, uint16_t qid, uint16_t qflags)
{
	struct mesh_reply* r = regional_alloc(s->s.region, 
		sizeof(struct mesh_reply));
	if(!r)
		return 0;
	r->query_reply = *rep;
	r->edns = *edns;
	r->qid = qid;
	r->qflags = qflags;
	if(gettimeofday(&r->start_time, NULL) < 0) {
		log_err("addrep: gettimeofday: %s", strerror(errno));
		memset(&r->start_time, 0, sizeof(r->start_time));
	}
	r->next = s->reply_list;
	s->reply_list = r;
	return 1;

}

/**
 * Continue processing the mesh state at another module.
 * Handles module to modules tranfer of control.
 * Handles module finished.
 * @param mesh: the mesh area.
 * @param mstate: currently active mesh state.
 * 	Deleted if finished, calls _done and _supers to 
 * 	send replies to clients and inform other mesh states.
 * 	This in turn may create additional runnable mesh states.
 * @param s: state at which the current module exited.
 * @param ev: the event sent to the module.
 * 	returned is the event to send to the next module.
 * @return true if continue processing at the new module.
 * 	false if not continued processing is needed.
 */
static int
mesh_continue(struct mesh_area* mesh, struct mesh_state* mstate,
	enum module_ext_state s, enum module_ev* ev)
{
	mstate->num_activated++;
	if(mstate->num_activated > MESH_MAX_ACTIVATION) {
		/* module is looping. Stop it. */
		log_err("internal error: looping module stopped");
		log_query_info(VERB_QUERY, "pass error for qstate",
			&mstate->s.qinfo);
		s = module_error;
	}
	if(s == module_wait_module) {
		/* start next module */
		mstate->s.curmod++;
		if(mesh->mods.num == mstate->s.curmod) {
			log_err("Cannot pass to next module; at last module");
			log_query_info(VERB_QUERY, "pass error for qstate",
				&mstate->s.qinfo);
			mstate->s.curmod--;
			return mesh_continue(mesh, mstate, module_error, ev);
		}
		*ev = module_event_pass;
		return 1;
	}
	if(s == module_error && mstate->s.return_rcode == LDNS_RCODE_NOERROR) {
		/* error is bad, handle pass back up below */
		mstate->s.return_rcode = LDNS_RCODE_SERVFAIL;
	}
	if(s == module_error || s == module_finished) {
		if(mstate->s.curmod == 0) {
			mesh_query_done(mstate);
			mesh_walk_supers(mesh, mstate);
			mesh_state_delete(&mstate->s);
			return 0;
		}
		/* pass along the locus of control */
		mstate->s.curmod --;
		*ev = module_event_moddone;
		return 1;
	}
	return 0;
}

void mesh_run(struct mesh_area* mesh, struct mesh_state* mstate,
	enum module_ev ev, struct outbound_entry* e)
{
	enum module_ext_state s;
	verbose(VERB_ALGO, "mesh_run: start");
	while(mstate) {
		/* run the module */
		log_assert(fptr_whitelist_mod_operate(
			mesh->mods.mod[mstate->s.curmod]->operate));
		(*mesh->mods.mod[mstate->s.curmod]->operate)
			(&mstate->s, ev, mstate->s.curmod, e);

		/* examine results */
		mstate->s.reply = NULL;
		regional_free_all(mstate->s.env->scratch);
		s = mstate->s.ext_state[mstate->s.curmod];
		verbose(VERB_ALGO, "mesh_run: %s module exit state is %s", 
			mesh->mods.mod[mstate->s.curmod]->name, strextstate(s));
		e = NULL;
		if(mesh_continue(mesh, mstate, s, &ev))
			continue;

		/* run more modules */
		ev = module_event_pass;
		if(mesh->run.count > 0) {
			/* pop random element off the runnable tree */
			mstate = (struct mesh_state*)mesh->run.root->key;
			(void)rbtree_delete(&mesh->run, mstate);
		} else mstate = NULL;
	}
	if(verbosity >= VERB_ALGO) {
		mesh_stats(mesh, "mesh_run: end");
		mesh_log_list(mesh);
	}
}

void 
mesh_log_list(struct mesh_area* mesh)
{
	char buf[30];
	struct mesh_state* m;
	int num = 0;
	RBTREE_FOR(m, struct mesh_state*, &mesh->all) {
		snprintf(buf, sizeof(buf), "%d%s%s%s%s%s mod%d %s%s", 
			num++, (m->s.is_priming)?"p":"",  /* prime */
			(m->s.query_flags&BIT_RD)?"RD":"",
			(m->s.query_flags&BIT_CD)?"CD":"",
			(m->super_set.count==0)?"d":"", /* detached */
			(m->sub_set.count!=0)?"c":"",  /* children */
			m->s.curmod, (m->reply_list)?"rep":"", /*hasreply*/
			(m->cb_list)?"cb":"" /* callbacks */
			); 
		log_query_info(VERB_ALGO, buf, &m->s.qinfo);
	}
}

void 
mesh_stats(struct mesh_area* mesh, const char* str)
{
	verbose(VERB_DETAIL, "%s %u recursion states (%u with reply, "
		"%u detached), %u waiting replies, %u recursion replies "
		"sent", str, (unsigned)mesh->all.count, 
		(unsigned)mesh->num_reply_states,
		(unsigned)mesh->num_detached_states,
		(unsigned)mesh->num_reply_addrs,
		(unsigned)mesh->replies_sent);
	if(mesh->replies_sent > 0) {
		struct timeval avg;
		timeval_divide(&avg, &mesh->replies_sum_wait, 
			mesh->replies_sent);
		log_info("average recursion processing time "
			"%d.%6.6d sec", (int)avg.tv_sec, (int)avg.tv_usec);
		log_info("histogram of recursion processing times");
		timehist_log(mesh->histogram, "recursions");
	}
}

void 
mesh_stats_clear(struct mesh_area* mesh)
{
	if(!mesh)
		return;
	mesh->replies_sent = 0;
	mesh->replies_sum_wait.tv_sec = 0;
	mesh->replies_sum_wait.tv_usec = 0;
	timehist_clear(mesh->histogram);
}

size_t 
mesh_get_mem(struct mesh_area* mesh)
{
	struct mesh_state* m;
	size_t s = sizeof(*mesh) + sizeof(struct timehist) +
		sizeof(struct th_buck)*mesh->histogram->num;
	RBTREE_FOR(m, struct mesh_state*, &mesh->all) {
		/* all, including m itself allocated in qstate region */
		s += regional_get_mem(m->s.region);
	}
	return s;
}

/** helper recursive rbtree find routine */
static int
find_in_subsub(struct mesh_state* m, struct mesh_state* tofind)
{
	struct mesh_state_ref* r;
	RBTREE_FOR(r, struct mesh_state_ref*, &m->sub_set) {
		if(r->s == tofind || find_in_subsub(r->s, tofind))
			return 1;
	}
	return 0;
}

int 
mesh_detect_cycle(struct module_qstate* qstate, struct query_info* qinfo,
	uint16_t flags, int prime)
{
	struct mesh_area* mesh = qstate->env->mesh;
	struct mesh_state* cyc_m = qstate->mesh_info;
	struct mesh_state* dep_m = mesh_area_find(mesh, qinfo, flags, prime);
	if(!dep_m)
		return 0;
	if(dep_m == cyc_m || find_in_subsub(dep_m, cyc_m))
		return 1;
	return 0;
}
