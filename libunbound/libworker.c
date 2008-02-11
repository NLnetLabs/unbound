/*
 * libunbound/worker.c - worker thread or process that resolves
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
 * This file contains the worker process or thread that performs
 * the DNS resolving and validation. The worker is called by a procedure
 * and if in the background continues until exit, if in the foreground
 * returns from the procedure when done.
 */
#include "config.h"
#include "libunbound/libworker.h"
#include "libunbound/context.h"
#include "libunbound/unbound.h"
#include "services/outside_network.h"
#include "services/mesh.h"
#include "services/localzone.h"
#include "services/cache/rrset.h"
#include "services/outbound_list.h"
#include "util/module.h"
#include "util/regional.h"
#include "util/random.h"
#include "util/config_file.h"
#include "util/netevent.h"
#include "util/storage/slabhash.h"
#include "util/net_help.h"
#include "util/data/dname.h"
#include "util/data/msgreply.h"
#include "util/data/msgencode.h"

/** handle new query command for bg worker */
static void handle_newq(struct libworker* w, uint8_t* buf, uint32_t len);

/** delete libworker struct */
static void
libworker_delete(struct libworker* w)
{
	if(!w) return;
	if(w->env) {
		mesh_delete(w->env->mesh);
		context_release_alloc(w->ctx, w->env->alloc, 
			!w->is_bg || w->is_bg_thread);
		ldns_buffer_free(w->env->scratch_buffer);
		regional_destroy(w->env->scratch);
		ub_randfree(w->env->rnd);
		free(w->env);
	}
	free(w->cmd_msg);
	outside_network_delete(w->back);
	comm_point_delete(w->cmd_com);
	comm_point_delete(w->res_com);
	comm_base_delete(w->base);
	free(w);
}

/** setup fresh libworker struct */
static struct libworker*
libworker_setup(struct ub_ctx* ctx, int is_bg)
{
	unsigned int seed;
	struct libworker* w = (struct libworker*)calloc(1, sizeof(*w));
	struct config_file* cfg = ctx->env->cfg;
	if(!w) return NULL;
	w->is_bg = is_bg;
	w->ctx = ctx;
	w->env = (struct module_env*)malloc(sizeof(*w->env));
	if(!w->env) {
		free(w);
		return NULL;
	}
	*w->env = *ctx->env;
	w->env->alloc = context_obtain_alloc(ctx, !w->is_bg || w->is_bg_thread);
	if(!w->env->alloc) {
		libworker_delete(w);
		return NULL;
	}
	w->thread_num = w->env->alloc->thread_num;
	alloc_set_id_cleanup(w->env->alloc, &libworker_alloc_cleanup, w);
	if(!w->is_bg || w->is_bg_thread) {
		lock_basic_lock(&ctx->cfglock);
	}
	w->env->scratch = regional_create_custom(cfg->msg_buffer_size);
	w->env->scratch_buffer = ldns_buffer_new(cfg->msg_buffer_size);
	if(!w->is_bg || w->is_bg_thread) {
		lock_basic_unlock(&ctx->cfglock);
	}
	if(!w->env->scratch || !w->env->scratch_buffer) {
		libworker_delete(w);
		return NULL;
	}
	w->env->worker = (struct worker*)w;
	seed = (unsigned int)time(NULL) ^ (unsigned int)getpid() ^
		(((unsigned int)w->thread_num)<<17);
	seed ^= (unsigned int)w->env->alloc->next_id;
	if(!w->is_bg || w->is_bg_thread) {
		lock_basic_lock(&ctx->cfglock);
	}
	if(!(w->env->rnd = ub_initstate(seed, ctx->seed_rnd))) {
		if(!w->is_bg || w->is_bg_thread) {
			lock_basic_unlock(&ctx->cfglock);
		}
		seed = 0;
		libworker_delete(w);
		return NULL;
	}
	if(!w->is_bg || w->is_bg_thread) {
		lock_basic_unlock(&ctx->cfglock);
	}
	seed = 0;

	w->base = comm_base_create();
	if(!w->base) {
		libworker_delete(w);
		return NULL;
	}
	if(!w->is_bg || w->is_bg_thread) {
		lock_basic_lock(&ctx->cfglock);
	}
	w->back = outside_network_create(w->base, cfg->msg_buffer_size,
		(size_t)cfg->outgoing_num_ports, cfg->out_ifs,
		cfg->num_out_ifs, cfg->do_ip4, cfg->do_ip6, -1, 
		cfg->do_tcp?cfg->outgoing_num_tcp:0,
		w->env->infra_cache, w->env->rnd);
	if(!w->is_bg || w->is_bg_thread) {
		lock_basic_unlock(&ctx->cfglock);
	}
	if(!w->back) {
		libworker_delete(w);
		return NULL;
	}
	w->env->mesh = mesh_create(&ctx->mods, w->env);
	if(!w->env->mesh) {
		libworker_delete(w);
		return NULL;
	}
	w->env->send_packet = &libworker_send_packet;
	w->env->send_query = &libworker_send_query;
	w->env->detach_subs = &mesh_detach_subs;
	w->env->attach_sub = &mesh_attach_sub;
	w->env->kill_sub = &mesh_state_delete;
	w->env->detect_cycle = &mesh_detect_cycle;
	return w;
}

/** handle cancel command for bg worker */
static void
handle_cancel(struct libworker* w, uint8_t* buf, uint32_t len)
{
	struct ctx_query* q;
	if(w->is_bg_thread) {
		lock_basic_lock(&w->ctx->cfglock);
		q = context_deserialize_cancel(w->ctx, buf, len);
		lock_basic_unlock(&w->ctx->cfglock);
	} else {
		q = context_deserialize_cancel(w->ctx, buf, len);
	}
	if(!q) {
		/* probably simply lookup failed, i.e. the message had been
		 * processed and answered before the cancel arrived */
		return;
	}
	q->cancelled = 1;
	free(buf);
}

/** do control command coming into bg server */
static void
libworker_do_cmd(struct libworker* w, uint8_t* msg, uint32_t len)
{
	switch(context_serial_getcmd(msg, len)) {
		default:
		case UB_LIBCMD_ANSWER:
			log_err("unknown command for bg worker %d", 
				(int)context_serial_getcmd(msg, len));
			/* and fall through to quit */
		case UB_LIBCMD_QUIT:
			free(msg);
			comm_base_exit(w->base);
			break;
		case UB_LIBCMD_NEWQUERY:
			handle_newq(w, msg, len);
			break;
		case UB_LIBCMD_CANCEL:
			handle_cancel(w, msg, len);
			break;
	}
}

/** handle control command coming into server */
int 
libworker_handle_control_cmd(struct comm_point* c, void* arg, 
	int ATTR_UNUSED(err), struct comm_reply* ATTR_UNUSED(rep))
{
	struct libworker* w = (struct libworker*)arg;
	ssize_t r;

	if(w->cmd_read < sizeof(w->cmd_len)) {
		/* complete reading the length of control msg */
		r = read(c->fd, ((uint8_t*)&w->cmd_len) + w->cmd_read,
			sizeof(w->cmd_len) - w->cmd_read);
		if(r==0) {
			/* error has happened or */
			/* parent closed pipe, must have exited somehow */
			/* it is of no use to go on, exit */
			comm_base_exit(w->base);
			return 0;
		}
		if(r==-1) {
			if(errno != EAGAIN && errno != EINTR) {
				log_err("rpipe error: %s", strerror(errno));
			}
			/* nothing to read now, try later */
			return 0;
		}
		w->cmd_read += r;
		if(w->cmd_read < sizeof(w->cmd_len)) {
			/* not complete, try later */
			return 0;
		}
		w->cmd_msg = (uint8_t*)calloc(1, w->cmd_len);
		if(!w->cmd_msg) {
			log_err("malloc failure");
			w->cmd_read = 0;
			return 0;
		}
	}
	/* cmd_len has been read, read remainder */
	r = read(c->fd, w->cmd_msg + w->cmd_read - sizeof(w->cmd_len),
		w->cmd_len - (w->cmd_read - sizeof(w->cmd_len)));
	if(r==0) {
		/* error has happened or */
		/* parent closed pipe, must have exited somehow */
		/* it is of no use to go on, exit */
		comm_base_exit(w->base);
		return 0;
	}
	if(r==-1) {
		/* nothing to read now, try later */
		if(errno != EAGAIN && errno != EINTR) {
			log_err("rpipe error: %s", strerror(errno));
		}
		return 0;
	}
	w->cmd_read += r;
	if(w->cmd_read < sizeof(w->cmd_len) + w->cmd_len) {
		/* not complete, try later */
		return 0;
	}
	w->cmd_read = 0;
	libworker_do_cmd(w, w->cmd_msg, w->cmd_len); /* also frees the buf */
	w->cmd_msg = NULL;
	return 0;
}

/** handle opportunity to write result back */
int 
libworker_handle_result_write(struct comm_point* c, void* arg, 
	int ATTR_UNUSED(err), struct comm_reply* ATTR_UNUSED(rep))
{
	struct libworker* w = (struct libworker*)arg;
	struct libworker_res_list* item = w->res_list;
	ssize_t r;
	if(!item) {
		comm_point_stop_listening(c);
		return 0;
	}
	if(w->res_write < sizeof(item->len)) {
		r = write(c->fd, ((uint8_t*)&item->len) + w->res_write,
			sizeof(item->len) - w->res_write);
		if(r == -1) {
			if(errno != EAGAIN && errno != EINTR) {
				log_err("wpipe error: %s", strerror(errno));
			}
			return 0; /* try again later */
		}
		if(r == 0) {
			/* error on pipe, must have exited somehow */
			/* it is of no use to go on, exit */
			comm_base_exit(w->base);
			return 0;
		}
		w->res_write += r;
		if(w->res_write < sizeof(item->len))
			return 0;
	}
	r = write(c->fd, item->buf + w->res_write - sizeof(item->len),
		item->len - (w->res_write - sizeof(item->len)));
	if(r == -1) {
		if(errno != EAGAIN && errno != EINTR) {
			log_err("wpipe error: %s", strerror(errno));
		}
		return 0; /* try again later */
	}
	if(r == 0) {
		/* error on pipe, must have exited somehow */
		/* it is of no use to go on, exit */
		comm_base_exit(w->base);
		return 0;
	}
	w->res_write += r;
	if(w->res_write < sizeof(item->len) + item->len)
		return 0;
	/* done this result, remove it */
	free(item->buf);
	item->buf = NULL;
	w->res_list = w->res_list->next;
	free(item);
	if(!w->res_list) {
		w->res_last = NULL;
		comm_point_stop_listening(c);
	}
	w->res_write = 0;
	return 0;
}

/** the background thread func */
static void*
libworker_dobg(void* arg)
{
	/* setup */
	uint32_t m;
	int fd;
	struct libworker* w = (struct libworker*)arg;
	struct ub_ctx* ctx = w->ctx;
	log_thread_set(&w->thread_num);
#if !defined(HAVE_PTHREAD) && !defined(HAVE_SOLARIS_THREADS)
	/* we are forked */
	w->is_bg_thread = 0;
	/* close non-used parts of the pipes */
	if(ctx->qqpipe[1] != -1) {
		close(ctx->qqpipe[1]);
		ctx->qqpipe[1] = -1;
	}
	if(ctx->rrpipe[0] != -1) {
		close(ctx->rrpipe[0]);
		ctx->rrpipe[0] = -1;
	}
#endif
	if(!w) {
		log_err("libunbound bg worker init failed, nomem");
		return NULL;
	}
	if(!(w->cmd_com=comm_point_create_raw(w->base, ctx->qqpipe[0], 0, 
		libworker_handle_control_cmd, w))) {
		log_err("libunbound bg worker init failed, no cmdcom");
		return NULL;
	}
	if(!(w->res_com=comm_point_create_raw(w->base, ctx->rrpipe[1], 1,
		libworker_handle_result_write, w))) {
		log_err("libunbound bg worker init failed, no rescom");
		return NULL;
	}

	/* do the work */
	comm_base_dispatch(w->base);

	/* cleanup */
	fd = ctx->rrpipe[1];
	ctx->rrpipe[1] = -1;
	m = UB_LIBCMD_QUIT;
	libworker_delete(w);
	close(ctx->qqpipe[0]);
	ctx->qqpipe[0] = -1;
	(void)libworker_write_msg(fd, (uint8_t*)&m, (uint32_t)sizeof(m), 0);
	close(fd);
	return NULL;
}

int libworker_bg(struct ub_ctx* ctx)
{
	struct libworker* w;
	/* fork or threadcreate */
	lock_basic_lock(&ctx->cfglock);
	if(ctx->dothread) {
		lock_basic_unlock(&ctx->cfglock);
		w = libworker_setup(ctx, 1);
		w->is_bg_thread = 1;
#ifdef ENABLE_LOCK_CHECKS
		w->thread_num = 1; /* for nicer DEBUG checklocks */
#endif
		if(!w) return UB_NOMEM;
		ub_thread_create(&ctx->bg_tid, libworker_dobg, w);
	} else {
		lock_basic_unlock(&ctx->cfglock);
		switch((ctx->bg_pid=fork())) {
			case 0:
				w = libworker_setup(ctx, 1);
				if(!w) fatal_exit("out of memory");
				/* close non-used parts of the pipes */
				close(ctx->qqpipe[1]);
				close(ctx->rrpipe[0]);
				ctx->qqpipe[1] = -1;
				ctx->rrpipe[0] = -1;
				(void)libworker_dobg(w);
				exit(0);
				break;
			case -1:
				return UB_FORKFAIL;
			default:
				break;
		}
	}
	return UB_NOERROR;
}

/** get msg reply struct (in temp region) */
static struct reply_info*
parse_reply(ldns_buffer* pkt, struct regional* region, struct query_info* qi)
{
	struct reply_info* rep;
	struct msg_parse* msg;
	if(!(msg = regional_alloc(region, sizeof(*msg)))) {
		return NULL;
	}
	memset(msg, 0, sizeof(*msg));
	ldns_buffer_set_position(pkt, 0);
	if(parse_packet(pkt, msg, region) != 0)
		return 0;
	if(!parse_create_msg(pkt, msg, NULL, qi, &rep, region)) {
		return 0;
	}
	return rep;
}

/** insert canonname */
static int
fill_canon(struct ub_result* res, uint8_t* s)
{
	char buf[255+2];
	dname_str(s, buf);
	res->canonname = strdup(buf);
	return res->canonname != 0;
}

/** fill data into result */
static int
fill_res(struct ub_result* res, struct ub_packed_rrset_key* answer,
	uint8_t* finalcname, struct query_info* rq)
{
	size_t i;
	struct packed_rrset_data* data;
	if(!answer) {
		if(finalcname) {
			if(!fill_canon(res, finalcname))
				return 0; /* out of memory */
		}
		res->data = (char**)calloc(1, sizeof(char*));
		res->len = (int*)calloc(1, sizeof(int));
		return (res->data && res->len);
	}
	data = (struct packed_rrset_data*)answer->entry.data;
	if(query_dname_compare(rq->qname, answer->rk.dname) != 0) {
		if(!fill_canon(res, answer->rk.dname))
			return 0; /* out of memory */
	} else	res->canonname = NULL;
	res->data = (char**)calloc(data->count+1, sizeof(char*));
	res->len = (int*)calloc(data->count+1, sizeof(int));
	if(!res->data || !res->len)
		return 0; /* out of memory */
	for(i=0; i<data->count; i++) {
		/* remove rdlength from rdata */
		res->len[i] = (int)(data->rr_len[i] - 2);
		res->data[i] = memdup(data->rr_data[i]+2, (size_t)res->len[i]);
		if(!res->data[i])
			return 0; /* out of memory */
	}
	res->data[data->count] = NULL;
	res->len[data->count] = 0;
	return 1;
}

/** fill result from parsed message, on error fills servfail */
void
libworker_enter_result(struct ub_result* res, ldns_buffer* buf,
	struct regional* temp, enum sec_status msg_security)
{
	struct query_info rq;
	struct reply_info* rep;
	res->rcode = LDNS_RCODE_SERVFAIL;
	rep = parse_reply(buf, temp, &rq);
	if(!rep) {
		log_err("cannot parse buf");
		return; /* error parsing buf, or out of memory */
	}
	if(!fill_res(res, reply_find_answer_rrset(&rq, rep), 
		reply_find_final_cname_target(&rq, rep), &rq))
		return; /* out of memory */
	/* rcode, havedata, nxdomain, secure, bogus */
	res->rcode = (int)FLAGS_GET_RCODE(rep->flags);
	if(res->data && res->data[0])
		res->havedata = 1;
	if(res->rcode == LDNS_RCODE_NXDOMAIN)
		res->nxdomain = 1;
	if(msg_security == sec_status_secure)
		res->secure = 1;
	if(msg_security == sec_status_bogus)
		res->bogus = 1;
}

/** fillup fg results */
static void
libworker_fillup_fg(struct ctx_query* q, int rcode, ldns_buffer* buf, 
	enum sec_status s)
{
	if(rcode != 0) {
		q->res->rcode = rcode;
		q->msg_security = s;
		return;
	}

	q->res->rcode = LDNS_RCODE_SERVFAIL;
	q->msg_security = 0;
	q->msg = memdup(ldns_buffer_begin(buf), ldns_buffer_limit(buf));
	q->msg_len = ldns_buffer_limit(buf);
	if(!q->msg) {
		return; /* the error is in the rcode */
	}

	/* canonname and results */
	q->msg_security = s;
	libworker_enter_result(q->res, buf, q->w->env->scratch, s);
}

/** callback with fg results */
static void
libworker_fg_done_cb(void* arg, int rcode, ldns_buffer* buf, enum sec_status s)
{
	struct ctx_query* q = (struct ctx_query*)arg;
	/* fg query is done; exit comm base */
	comm_base_exit(q->w->base);

	libworker_fillup_fg(q, rcode, buf, s);
}

/** setup qinfo and edns */
static int
setup_qinfo_edns(struct libworker* w, struct ctx_query* q, 
	struct query_info* qinfo, struct edns_data* edns)
{
	ldns_rdf* rdf;
	qinfo->qtype = (uint16_t)q->res->qtype;
	qinfo->qclass = (uint16_t)q->res->qclass;
	rdf = ldns_dname_new_frm_str(q->res->qname);
	if(!rdf) {
		return 0;
	}
	qinfo->qname = ldns_rdf_data(rdf);
	qinfo->qname_len = ldns_rdf_size(rdf);
	edns->edns_present = 1;
	edns->ext_rcode = 0;
	edns->edns_version = 0;
	edns->bits = EDNS_DO;
	if(ldns_buffer_capacity(w->back->udp_buff) < 65535)
		edns->udp_size = (uint16_t)ldns_buffer_capacity(
			w->back->udp_buff);
	else	edns->udp_size = 65535;
	ldns_rdf_free(rdf);
	return 1;
}

int libworker_fg(struct ub_ctx* ctx, struct ctx_query* q)
{
	struct libworker* w = libworker_setup(ctx, 0);
	uint16_t qflags, qid;
	struct query_info qinfo;
	struct edns_data edns;
	if(!w)
		return UB_INITFAIL;
	if(!setup_qinfo_edns(w, q, &qinfo, &edns)) {
		libworker_delete(w);
		return UB_SYNTAX;
	}
	qid = 0;
	qflags = BIT_RD;
	q->w = w;
	/* see if there is a fixed answer */
	ldns_buffer_write_u16_at(w->back->udp_buff, 0, qid);
	ldns_buffer_write_u16_at(w->back->udp_buff, 2, qflags);
	if(local_zones_answer(ctx->local_zones, &qinfo, &edns, 
		w->back->udp_buff, w->env->scratch)) {
		libworker_fillup_fg(q, LDNS_RCODE_NOERROR, 
			w->back->udp_buff, sec_status_insecure);
		libworker_delete(w);
		free(qinfo.qname);
		return UB_NOERROR;
	}
	/* process new query */
	if(!mesh_new_callback(w->env->mesh, &qinfo, qflags, &edns, 
		w->back->udp_buff, qid, libworker_fg_done_cb, q)) {
		free(qinfo.qname);
		return UB_NOMEM;
	}
	free(qinfo.qname);

	/* wait for reply */
	comm_base_dispatch(w->base);

	libworker_delete(w);
	return UB_NOERROR;
}

/** add result to the bg worker result queue */
static void
add_bg_result(struct libworker* w, struct ctx_query* q, ldns_buffer* pkt, 
	int err)
{
	uint8_t* msg = NULL;
	uint32_t len = 0;
	struct libworker_res_list* item;

	/* serialize and delete unneeded q */
	if(w->is_bg_thread) {
		lock_basic_lock(&w->ctx->cfglock);
		q->msg_len = ldns_buffer_remaining(pkt);
		q->msg = memdup(ldns_buffer_begin(pkt), q->msg_len);
		if(!q->msg)
			msg = context_serialize_answer(q, UB_NOMEM, NULL, &len);
		else	msg = context_serialize_answer(q, err, NULL, &len);
		lock_basic_unlock(&w->ctx->cfglock);
	} else {
		msg = context_serialize_answer(q, err, pkt, &len);
		(void)rbtree_delete(&w->ctx->queries, q->node.key);
		w->ctx->num_async--;
		context_query_delete(q);
	}

	if(!msg) {
		log_err("out of memory for async answer");
		return;
	}
	item = (struct libworker_res_list*)malloc(sizeof(*item));
	if(!item) {
		free(msg);
		log_err("out of memory for async answer");
		return;
	}
	item->buf = msg;
	item->len = len;
	item->next = NULL;
	/* add at back of list, since the first one may be partially written */
	if(w->res_last)
		w->res_last->next = item;
	else	w->res_list = item;
	w->res_last = item;
	if(w->res_list == w->res_last) {
		/* first added item, start the write process */
		comm_point_start_listening(w->res_com, -1, -1);
	}
}

/** callback with bg results */
static void
libworker_bg_done_cb(void* arg, int rcode, ldns_buffer* buf, enum sec_status s)
{
	struct ctx_query* q = (struct ctx_query*)arg;

	if(q->cancelled) {
		if(q->w->is_bg_thread) {
			/* delete it now */
			struct ub_ctx* ctx = q->w->ctx;
			lock_basic_lock(&ctx->cfglock);
			(void)rbtree_delete(&ctx->queries, q->node.key);
			ctx->num_async--;
			context_query_delete(q);
			lock_basic_unlock(&ctx->cfglock);
		}
		/* cancelled, do not give answer */
		return;
	}
	q->msg_security = s;
	if(rcode != 0) {
		error_encode(buf, rcode, NULL, 0, BIT_RD, NULL);
	}
	add_bg_result(q->w, q, buf, UB_NOERROR);
}


/** handle new query command for bg worker */
static void
handle_newq(struct libworker* w, uint8_t* buf, uint32_t len)
{
	uint16_t qflags, qid;
	struct query_info qinfo;
	struct edns_data edns;
	struct ctx_query* q;
	if(w->is_bg_thread) {
		lock_basic_lock(&w->ctx->cfglock);
		q = context_lookup_new_query(w->ctx, buf, len);
		lock_basic_unlock(&w->ctx->cfglock);
	} else {
		q = context_deserialize_new_query(w->ctx, buf, len);
	}
	free(buf);
	if(!q) {
		log_err("failed to deserialize newq");
		return;
	}
	if(!setup_qinfo_edns(w, q, &qinfo, &edns)) {
		add_bg_result(w, q, NULL, UB_SYNTAX);
		return;
	}
	qid = 0;
	qflags = BIT_RD;
	/* see if there is a fixed answer */
	ldns_buffer_write_u16_at(w->back->udp_buff, 0, qid);
	ldns_buffer_write_u16_at(w->back->udp_buff, 2, qflags);
	if(local_zones_answer(w->ctx->local_zones, &qinfo, &edns, 
		w->back->udp_buff, w->env->scratch)) {
		q->msg_security = sec_status_insecure;
		add_bg_result(w, q, w->back->udp_buff, UB_NOERROR);
		free(qinfo.qname);
		return;
	}
	q->w = w;
	/* process new query */
	if(!mesh_new_callback(w->env->mesh, &qinfo, qflags, &edns, 
		w->back->udp_buff, qid, libworker_bg_done_cb, q)) {
		add_bg_result(w, q, NULL, UB_NOMEM);
	}
	free(qinfo.qname);
}

void libworker_alloc_cleanup(void* arg)
{
	struct libworker* w = (struct libworker*)arg;
	slabhash_clear(&w->env->rrset_cache->table);
        slabhash_clear(w->env->msg_cache);
}

int libworker_send_packet(ldns_buffer* pkt, struct sockaddr_storage* addr,
        socklen_t addrlen, int timeout, struct module_qstate* q, int use_tcp)
{
	struct libworker* w = (struct libworker*)q->env->worker;
	if(use_tcp) {
		return pending_tcp_query(w->back, pkt, addr, addrlen,
			timeout, libworker_handle_reply, q, 
			q->env->rnd) != 0;
	}
	return pending_udp_query(w->back, pkt, addr, addrlen,
		timeout*1000, libworker_handle_reply, q, 
		q->env->rnd) != 0;
}

/** compare outbound entry qstates */
static int
outbound_entry_compare(void* a, void* b)
{
        struct outbound_entry* e1 = (struct outbound_entry*)a;
        struct outbound_entry* e2 = (struct outbound_entry*)b;
        if(e1->qstate == e2->qstate)
                return 1;
        return 0;
}

struct outbound_entry* libworker_send_query(uint8_t* qname, size_t qnamelen,
        uint16_t qtype, uint16_t qclass, uint16_t flags, int dnssec,
        struct sockaddr_storage* addr, socklen_t addrlen,
        struct module_qstate* q)
{
	struct libworker* w = (struct libworker*)q->env->worker;
	struct outbound_entry* e = (struct outbound_entry*)regional_alloc(
		q->region, sizeof(*e));
	if(!e)
		return NULL;
	e->qstate = q;
	e->qsent = outnet_serviced_query(w->back, qname,
		qnamelen, qtype, qclass, flags, dnssec, addr, addrlen,
		libworker_handle_service_reply, e, w->back->udp_buff,
		&outbound_entry_compare);
	if(!e->qsent) {
		return NULL;
	}
	return e;
}

int 
libworker_handle_reply(struct comm_point* c, void* arg, int error,
        struct comm_reply* reply_info)
{
	struct module_qstate* q = (struct module_qstate*)arg;
	struct libworker* lw = (struct libworker*)q->env->worker;
	struct outbound_entry e;
	e.qstate = q;
	e.qsent = NULL;

	if(error != 0) {
		mesh_report_reply(lw->env->mesh, &e, 0, reply_info);
		return 0;
	}
	/* sanity check. */
	if(!LDNS_QR_WIRE(ldns_buffer_begin(c->buffer))
		|| LDNS_OPCODE_WIRE(ldns_buffer_begin(c->buffer)) !=
			LDNS_PACKET_QUERY
		|| LDNS_QDCOUNT(ldns_buffer_begin(c->buffer)) > 1) {
		/* error becomes timeout for the module as if this reply
		 * never arrived. */
		mesh_report_reply(lw->env->mesh, &e, 0, reply_info);
		return 0;
	}
	mesh_report_reply(lw->env->mesh, &e, 1, reply_info);
	return 0;
}

int 
libworker_handle_service_reply(struct comm_point* c, void* arg, int error,
        struct comm_reply* reply_info)
{
	struct outbound_entry* e = (struct outbound_entry*)arg;
	struct libworker* lw = (struct libworker*)e->qstate->env->worker;

	if(error != 0) {
		mesh_report_reply(lw->env->mesh, e, 0, reply_info);
		return 0;
	}
	/* sanity check. */
	if(!LDNS_QR_WIRE(ldns_buffer_begin(c->buffer))
		|| LDNS_OPCODE_WIRE(ldns_buffer_begin(c->buffer)) !=
			LDNS_PACKET_QUERY
		|| LDNS_QDCOUNT(ldns_buffer_begin(c->buffer)) > 1) {
		/* error becomes timeout for the module as if this reply
		 * never arrived. */
		mesh_report_reply(lw->env->mesh, e, 0, reply_info);
		return 0;
	}
	mesh_report_reply(lw->env->mesh,  e, 1, reply_info);
	return 0;
}

int 
libworker_write_msg(int fd, uint8_t* buf, uint32_t len, int nonblock)
{
	ssize_t r;
	/* test */
	if(nonblock) {
		r = write(fd, &len, sizeof(len));
		if(r == -1) {
			if(errno==EINTR || errno==EAGAIN)
				return -1;
			log_err("msg write failed: %s", strerror(errno));
			return -1; /* can still continue, perhaps */
		}
	} else r = 0;
	if(!fd_set_block(fd))
		return 0;
	/* write remainder */
	if(r != (ssize_t)sizeof(len)) {
		if(write(fd, (char*)(&len)+r, sizeof(len)-r) == -1) {
			log_err("msg write failed: %s", strerror(errno));
			(void)fd_set_nonblock(fd);
			return 0;
		}
	}
	if(write(fd, buf, len) == -1) {
		log_err("msg write failed: %s", strerror(errno));
		(void)fd_set_nonblock(fd);
		return 0;
	}
	if(!fd_set_nonblock(fd))
		return 0;
	return 1;
}

int 
libworker_read_msg(int fd, uint8_t** buf, uint32_t* len, int nonblock)
{
	ssize_t r;

	/* test */
	*len = 0;
	if(nonblock) {
		r = read(fd, len, sizeof(*len));
		if(r == -1) {
			if(errno==EINTR || errno==EAGAIN)
				return -1;
			log_err("msg read failed: %s", strerror(errno));
			return -1; /* we can still continue, perhaps */
		}
		if(r == 0) /* EOF */
			return 0;
	} else r = 0;
	if(!fd_set_block(fd))
		return 0;
	/* read remainder */
	if(r != (ssize_t)sizeof(*len)) {
		if((r=read(fd, (char*)(len)+r, sizeof(*len)-r)) == -1) {
			log_err("msg read failed: %s", strerror(errno));
			(void)fd_set_nonblock(fd);
			return 0;
		}
		if(r == 0) /* EOF */ {
			(void)fd_set_nonblock(fd);
			return 0;
		}
	}
	*buf = (uint8_t*)malloc(*len);
	if(!*buf) {
		log_err("out of memory");
		(void)fd_set_nonblock(fd);
		return 0;
	}
	if((r=read(fd, *buf, *len)) == -1) {
		log_err("msg read failed: %s", strerror(errno));
		(void)fd_set_nonblock(fd);
		return 0;
	}
	if(r == 0) { /* EOF */
		(void)fd_set_nonblock(fd);
		return 0;
	}
	if(!fd_set_nonblock(fd))
		return 0;
	return 1;
}

/* --- fake callbacks for fptr_wlist to work --- */
int worker_handle_control_cmd(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

int worker_handle_request(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(repinfo))
{
	log_assert(0);
	return 0;
}

int worker_handle_reply(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

int worker_handle_service_reply(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

void worker_sighandler(int ATTR_UNUSED(sig), void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

int worker_send_packet(ldns_buffer* ATTR_UNUSED(pkt), 
	struct sockaddr_storage* ATTR_UNUSED(addr), 
	socklen_t ATTR_UNUSED(addrlen), int ATTR_UNUSED(timeout), 
	struct module_qstate* ATTR_UNUSED(q), int ATTR_UNUSED(use_tcp))
{
	log_assert(0);
	return 0;
}

struct outbound_entry* worker_send_query(uint8_t* ATTR_UNUSED(qname), 
	size_t ATTR_UNUSED(qnamelen), uint16_t ATTR_UNUSED(qtype), 
	uint16_t ATTR_UNUSED(qclass), uint16_t ATTR_UNUSED(flags), 
	int ATTR_UNUSED(dnssec), struct sockaddr_storage* ATTR_UNUSED(addr), 
	socklen_t ATTR_UNUSED(addrlen), struct module_qstate* ATTR_UNUSED(q))
{
	log_assert(0);
	return 0;
}

void 
worker_alloc_cleanup(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

int
acl_list_cmp(const void* ATTR_UNUSED(k1), const void* ATTR_UNUSED(k2))
{
	log_assert(0);
	return 0;
}

void worker_stat_timer_cb(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}
