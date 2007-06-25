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
#include "util/log.h"
#include "util/net_help.h"
#include "util/module.h"
#include "util/region-allocator.h"
#include "util/data/msgencode.h"

/** compare two mesh_states */
static int
mesh_state_compare(const void* ap, const void* bp)
{
	struct mesh_state* a = (struct mesh_state*)ap;
	struct mesh_state* b = (struct mesh_state*)bp;

	if(a->is_priming && !b->is_priming)
		return -1;
	if(!a->is_priming && b->is_priming)
		return 1;

	if((a->s.query_flags&BIT_RD) && !(b->s.query_flags&BIT_RD))
		return -1;
	if(!(a->s.query_flags&BIT_RD) && (b->s.query_flags&BIT_RD))
		return 1;

	return query_info_compare(&a->s.qinfo, &b->s.qinfo);
}

/** compare two mesh references */
static int
mesh_state_ref_compare(const void* ap, const void* bp)
{
	struct mesh_state_ref* a = (struct mesh_state_ref*)ap;
	struct mesh_state_ref* b = (struct mesh_state_ref*)bp;
	return mesh_state_compare(a->s, b->s);
}

struct mesh_area* 
mesh_create(int num_modules, struct module_func_block** modfunc,
	struct module_env* env)
{
	struct mesh_area* mesh = calloc(1, sizeof(struct mesh_area));
	if(!mesh) {
		log_err("mesh area alloc: out of memory");
		return NULL;
	}
	mesh->num_modules = num_modules;
	mesh->modfunc = modfunc;
	mesh->env = env;
	rbtree_init(&mesh->run, &mesh_state_compare);
	rbtree_init(&mesh->all, &mesh_state_compare);
	mesh->num_reply_addrs = 0;
	mesh->num_reply_states = 0;
	mesh->num_detached_states = 0;
	return mesh;
}

void 
mesh_delete(struct mesh_area* mesh)
{
	struct mesh_state* mstate;
	if(!mesh)
		return;
	/* free all query states */
	RBTREE_FOR(mstate, struct mesh_state*, &mesh->all) {
		mesh_state_cleanup(mstate);
	}
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
	if(!s->reply_list && s->super_set.count == 0)
		was_detached = 1;
	if(!s->reply_list)
		was_noreply = 1;
	/* add reply to s */
	if(!mesh_state_add_reply(s, edns, rep, qid, qflags)) {
			log_err("mesh_new_client: out of memory; SERVFAIL");
			error_encode(rep->c->buffer, LDNS_RCODE_SERVFAIL,
				qinfo, qid, qflags, edns);
			comm_point_send_reply(rep);
			if(added)
				mesh_state_delete(s);
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
	if(added)
		mesh_run(mesh, s, module_event_new, NULL);
}

void mesh_report_reply(struct mesh_area* mesh, struct outbound_entry* e,
        int is_ok, struct comm_reply* reply)
{
	e->qstate->reply = reply;
	mesh_run(mesh, e->qstate->mesh_info,
		is_ok?module_event_reply:module_event_timeout, e);
}

struct mesh_state* 
mesh_state_create(struct module_env* env, struct query_info* qinfo, 
	uint16_t qflags, int prime)
{
	region_type* region = region_create(malloc, free);
	struct mesh_state* mstate;
	int i;
	if(!region)
		return NULL;
	mstate = (struct mesh_state*)region_alloc(region, 
		sizeof(struct mesh_state));
	if(!mstate) {
		region_destroy(region);
		return NULL;
	}
	memset(mstate, 0, sizeof(*mstate));
	mstate->node = *RBTREE_NULL;
	mstate->run_node = *RBTREE_NULL;
	mstate->node.key = mstate;
	mstate->run_node.key = mstate;
	mstate->debug_flags = 0;
	mstate->is_priming = prime;
	mstate->reply_list = NULL;
	rbtree_init(&mstate->super_set, &mesh_state_ref_compare);
	rbtree_init(&mstate->sub_set, &mesh_state_ref_compare);
	/* init module qstate */
	mstate->s.qinfo.qtype = qinfo->qtype;
	mstate->s.qinfo.qclass = qinfo->qclass;
	mstate->s.qinfo.qname_len = qinfo->qname_len;
	mstate->s.qinfo.qname = region_alloc_init(region, qinfo->qname,
		qinfo->qname_len);
	if(!mstate->s.qinfo.qname) {
		region_destroy(region);
		return NULL;
	}
	/* remove all weird bits from qflags */
	mstate->s.query_flags = (qflags & BIT_RD);
	mstate->s.reply = NULL;
	mstate->s.region = region;
	mstate->s.curmod = 0;
	mstate->s.env = env;
	mstate->s.mesh_info = mstate;
	/* init modules */
	for(i=0; i<env->mesh->num_modules; i++) {
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
	for(i=0; i<mesh->num_modules; i++) {
		(*mesh->modfunc[i]->clear)(&mstate->s, i);
		mstate->s.minfo[i] = NULL;
		mstate->s.ext_state[i] = module_finished;
	}
	region_destroy(mstate->s.region);
}

void 
mesh_state_delete(struct mesh_state* mstate)
{
	struct mesh_area* mesh;
	struct mesh_state_ref* super, ref;
	if(!mstate)
		return;
	mesh = mstate->s.env->mesh;
	mesh_detach_subs(&mstate->s);
	if(!mstate->reply_list && mstate->super_set.count == 0) {
		log_assert(mesh->num_detached_states > 0);
		mesh->num_detached_states--;
	}
	if(mstate->reply_list) {
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
		if(!ref->s->reply_list && ref->s->super_set.count == 0) {
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
	if(!sub->reply_list && sub->super_set.count == 1) {
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
	if( !(subref = region_alloc(super->s.region,
		sizeof(struct mesh_state_ref))) ||
		!(superref = region_alloc(sub->s.region,
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
		d->tv_usec += 1000000;
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
			(int)(r->edns.bits & EDNS_DO))) 
		{
			error_encode(r->query_reply.c->buffer, 
				LDNS_RCODE_SERVFAIL, &m->s.qinfo, r->qid, 
				r->qflags, &r->edns);
		}
		comm_point_send_reply(&r->query_reply);
	}
	/* account */
	if(gettimeofday(&end_time, NULL) < 0) {
		log_err("gettimeofday: %s", strerror(errno));
		return;
	} else {
		struct timeval duration;
		timeval_subtract(&duration, &end_time, &r->start_time);
		verbose(VERB_ALGO, "query took %d s %d usec",
			(int)duration.tv_sec, (int)duration.tv_usec);
		m->s.env->mesh->replies_sent++;
		timeval_add(&m->s.env->mesh->replies_sum_wait, &duration);
	}
}

void mesh_query_done(struct module_qstate* qstate, int rcode,
        struct reply_info* rep)
{
	struct mesh_state* m = qstate->mesh_info;
	struct mesh_reply* r;
	log_assert(!(m->debug_flags&1)); /* not twice! */
	m->debug_flags |= 1;
	for(r = m->reply_list; r; r = r->next) {
		mesh_send_reply(m, rcode, rep, r);
	}
}

void mesh_walk_supers(struct module_qstate* qstate, int id, int rcode,
        void (*cb)(struct module_qstate*, int, struct module_qstate*, int))
{
	struct mesh_state* m = qstate->mesh_info;
	struct mesh_area* mesh = m->s.env->mesh;
	struct mesh_state_ref* ref;
	log_assert(!(m->debug_flags&2)); /* not twice! */
	m->debug_flags |= 2;
	RBTREE_FOR(ref, struct mesh_state_ref*, &qstate->mesh_info->super_set)
	{
		/* make super runnable */
		(void)rbtree_insert(&mesh->run, &ref->s->run_node);
		/* callback */
		(*cb)(qstate, id, &ref->s->s, rcode);
	}
}

struct mesh_state* mesh_area_find(struct mesh_area* mesh,
	struct query_info* qinfo, uint16_t qflags, int prime)
{
	struct mesh_state key;
	struct mesh_state* result;

	key.node.key = &key;
	key.is_priming = prime;
	key.s.qinfo = *qinfo;
	key.s.query_flags = qflags;
	
	result = (struct mesh_state*)rbtree_search(&mesh->all, &key);
	return result;
}

int mesh_state_add_reply(struct mesh_state* s, struct edns_data* edns,
        struct comm_reply* rep, uint16_t qid, uint16_t qflags)
{
	struct mesh_reply* r = region_alloc(s->s.region, 
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

void mesh_run(struct mesh_area* mesh, struct mesh_state* mstate,
	enum module_ev ev, struct outbound_entry* e)
{
	enum module_ext_state s;
	verbose(VERB_ALGO, "mesh_run: start");
	while(mstate) {
		/* run the module */
		(*mesh->modfunc[mstate->s.curmod]->operate)
			(&mstate->s, ev, mstate->s.curmod, e);

		/* examine results */
		mstate->s.reply = NULL;
		region_free_all(mstate->s.env->scratch);
		s = mstate->s.ext_state[mstate->s.curmod];
		verbose(VERB_ALGO, "mesh_run: %s module exit state is %s", 
			mesh->modfunc[mstate->s.curmod]->name, strextstate(s));
		if(s == module_error || s == module_finished) {
			/* must have called _done and _supers */
			log_assert(mstate->debug_flags == 3);
			mesh_state_delete(mstate);
		}

		/* run more modules */
		ev = module_event_pass;
		e = NULL;
		if(mesh->run.count > 0) {
			/* pop random element off the runnable tree */
			mstate = (struct mesh_state*)mesh->run.root->key;
			(void)rbtree_delete(&mesh->run, mstate);
		} else mstate = NULL;
	}
	verbose(VERB_ALGO, "mesh_run: end");
}
