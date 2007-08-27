/*
 * daemon/worker.c - worker that handles a pending list of requests.
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
 * This file implements the worker that handles callbacks on events, for
 * pending requests.
 */
#include "config.h"
#include "util/log.h"
#include "util/net_help.h"
#include "util/random.h"
#include "daemon/worker.h"
#include "daemon/daemon.h"
#include "util/netevent.h"
#include "util/config_file.h"
#include "util/module.h"
#include "util/region-allocator.h"
#include "util/storage/slabhash.h"
#include "services/listen_dnsport.h"
#include "services/outside_network.h"
#include "services/outbound_list.h"
#include "services/cache/rrset.h"
#include "services/cache/infra.h"
#include "services/cache/dns.h"
#include "services/mesh.h"
#include "util/data/msgparse.h"
#include "util/data/msgencode.h"
#include "util/data/dname.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#include <netdb.h>
#include <signal.h>

/** Size of an UDP datagram */
#define NORMAL_UDP_SIZE	512 /* bytes */

/** measure memory leakage */
static void
debug_memleak(size_t accounted, size_t heap, 
	size_t total_alloc, size_t total_free)
{
	static int init = 0;
	static size_t base_heap, base_accounted, base_alloc, base_free;
	size_t base_af, cur_af, grow_af, grow_acc;
	if(!init) {
		init = 1;
		base_heap = heap;
		base_accounted = accounted;
		base_alloc = total_alloc;
		base_free = total_free;
	}
	base_af = base_alloc - base_free;
	cur_af = total_alloc - total_free;
	grow_af = cur_af - base_af;
	grow_acc = accounted - base_accounted;
	log_info("Leakage: %u leaked. growth: %u use, %u acc, %u heap",
		(unsigned)(grow_af - grow_acc), (unsigned)grow_af, 
		(unsigned)grow_acc, (unsigned)(heap - base_heap));
}

/** give debug heap size indication */
static void
debug_total_mem(size_t calctotal)
{
	extern void* unbound_start_brk;
	extern size_t unbound_mem_alloc, unbound_mem_freed;
	void* cur = sbrk(0);
	int total = cur-unbound_start_brk;
	log_info("Total heap memory estimate: %u  total-alloc: %u  "
		"total-free: %u", (unsigned)total, 
		(unsigned)unbound_mem_alloc, (unsigned)unbound_mem_freed);
	debug_memleak(calctotal, (size_t)total, 
		unbound_mem_alloc, unbound_mem_freed);
}

/** Report on memory usage by this thread and global */
void
worker_mem_report(struct worker* worker, struct serviced_query* cur_serv)
{
	size_t total, front, back, mesh, msg, rrset, infra, ac, superac;
	size_t me;
	if(verbosity < VERB_ALGO) 
		return;
	front = listen_get_mem(worker->front);
	back = outnet_get_mem(worker->back);
	msg = slabhash_get_mem(worker->env.msg_cache);
	rrset = slabhash_get_mem(&worker->env.rrset_cache->table);
	infra = infra_get_mem(worker->env.infra_cache);
	mesh = mesh_get_mem(worker->env.mesh);
	ac = alloc_get_mem(&worker->alloc);
	superac = alloc_get_mem(&worker->daemon->superalloc);
	me = sizeof(*worker) + sizeof(*worker->base) + sizeof(*worker->comsig)
		+ comm_point_get_mem(worker->cmd_com) + 
		sizeof(worker->rndstate) + region_get_mem(worker->scratchpad)+
		sizeof(*worker->env.scratch_buffer) + 
		ldns_buffer_capacity(worker->env.scratch_buffer);
	if(cur_serv)
		me += serviced_get_mem(cur_serv);
	total = front+back+mesh+msg+rrset+infra+ac+superac+me;
	log_info("Memory conditions: %u front=%u back=%u mesh=%u msg=%u "
		"rrset=%u infra=%u alloccache=%u globalalloccache=%u me=%u",
		(unsigned)total, (unsigned)front, (unsigned)back, 
		(unsigned)mesh, (unsigned)msg, (unsigned)rrset, 
		(unsigned)infra, (unsigned)ac, (unsigned)superac, 
		(unsigned)me);
	debug_total_mem(total);
}

void 
worker_send_cmd(struct worker* worker, ldns_buffer* buffer,
	enum worker_commands cmd)
{
	ldns_buffer_clear(buffer);
	/* like DNS message, length data */
	ldns_buffer_write_u16(buffer, sizeof(uint32_t));
	ldns_buffer_write_u32(buffer, (uint32_t)cmd);
	ldns_buffer_flip(buffer);
	if(!write_socket(worker->cmd_send_fd, ldns_buffer_begin(buffer),
		ldns_buffer_limit(buffer)))
		log_err("write socket: %s", strerror(errno));
}

/** process incoming replies from the network */
static int 
worker_handle_reply(struct comm_point* c, void* arg, int error, 
	struct comm_reply* reply_info)
{
	struct module_qstate* q = (struct module_qstate*)arg;
	struct worker* worker = q->env->worker;
	struct outbound_entry e;
	e.qstate = q;
	e.qsent = NULL;

	if(error != 0) {
		mesh_report_reply(worker->env.mesh, &e, 0, reply_info);
		worker_mem_report(worker, NULL);
		return 0;
	}
	/* sanity check. */
	if(!LDNS_QR_WIRE(ldns_buffer_begin(c->buffer))
		|| LDNS_OPCODE_WIRE(ldns_buffer_begin(c->buffer)) != 
			LDNS_PACKET_QUERY
		|| LDNS_QDCOUNT(ldns_buffer_begin(c->buffer)) > 1) {
		/* error becomes timeout for the module as if this reply
		 * never arrived. */
		mesh_report_reply(worker->env.mesh, &e, 0, reply_info);
		worker_mem_report(worker, NULL);
		return 0;
	}
	mesh_report_reply(worker->env.mesh, &e, 1, reply_info);
	worker_mem_report(worker, NULL);
	return 0;
}

/** process incoming serviced query replies from the network */
static int 
worker_handle_service_reply(struct comm_point* c, void* arg, int error, 
	struct comm_reply* reply_info)
{
	struct outbound_entry* e = (struct outbound_entry*)arg;
	struct worker* worker = e->qstate->env->worker;
	struct serviced_query *sq = e->qsent;

	verbose(VERB_ALGO, "worker svcd callback for qstate %p", e->qstate);
	if(error != 0) {
		mesh_report_reply(worker->env.mesh, e, 0, reply_info);
		worker_mem_report(worker, sq);
		return 0;
	}
	/* sanity check. */
	if(!LDNS_QR_WIRE(ldns_buffer_begin(c->buffer))
		|| LDNS_OPCODE_WIRE(ldns_buffer_begin(c->buffer)) != 
			LDNS_PACKET_QUERY
		|| LDNS_QDCOUNT(ldns_buffer_begin(c->buffer)) > 1) {
		/* error becomes timeout for the module as if this reply
		 * never arrived. */
		verbose(VERB_ALGO, "worker: bad reply handled as timeout");
		mesh_report_reply(worker->env.mesh, e, 0, reply_info);
		worker_mem_report(worker, sq);
		return 0;
	}
	mesh_report_reply(worker->env.mesh, e, 1, reply_info);
	worker_mem_report(worker, sq);
	return 0;
}

/** check request sanity. Returns error code, 0 OK, or -1 discard. 
 * @param pkt: the wire packet to examine for sanity.
 * @param worker: parameters for checking.
*/
static int 
worker_check_request(ldns_buffer* pkt, struct worker* worker)
{
	if(ldns_buffer_limit(pkt) < LDNS_HEADER_SIZE) {
		verbose(VERB_DETAIL, "request too short, discarded");
		return -1;
	}
	if(ldns_buffer_limit(pkt) > NORMAL_UDP_SIZE && 
		worker->daemon->cfg->harden_large_queries) {
		verbose(VERB_DETAIL, "request too large, discarded");
		return -1;
	}
	if(LDNS_QR_WIRE(ldns_buffer_begin(pkt))) {
		verbose(VERB_DETAIL, "request has QR bit on, discarded");
		return -1;
	}
	if(LDNS_OPCODE_WIRE(ldns_buffer_begin(pkt)) != LDNS_PACKET_QUERY) {
		verbose(VERB_DETAIL, "request unknown opcode %d", 
			LDNS_OPCODE_WIRE(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_NOTIMPL;
	}
	if(LDNS_QDCOUNT(ldns_buffer_begin(pkt)) != 1) {
		verbose(VERB_DETAIL, "request wrong nr qd=%d", 
			LDNS_QDCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	if(LDNS_ANCOUNT(ldns_buffer_begin(pkt)) != 0) {
		verbose(VERB_DETAIL, "request wrong nr an=%d", 
			LDNS_ANCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	if(LDNS_NSCOUNT(ldns_buffer_begin(pkt)) != 0) {
		verbose(VERB_DETAIL, "request wrong nr ns=%d", 
			LDNS_NSCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	if(LDNS_ARCOUNT(ldns_buffer_begin(pkt)) > 1) {
		verbose(VERB_DETAIL, "request wrong nr ar=%d", 
			LDNS_ARCOUNT(ldns_buffer_begin(pkt)));
		return LDNS_RCODE_FORMERR;
	}
	return 0;
}

/** process control messages from the main thread. 
 * @param c: comm point to read from.
 * @param arg: worker.
 * @param error: error status of comm point.
 * @param reply_info: not used.
 */
static int 
worker_handle_control_cmd(struct comm_point* c, void* arg, int error, 
	struct comm_reply* ATTR_UNUSED(reply_info))
{
	struct worker* worker = (struct worker*)arg;
	enum worker_commands cmd;
	if(error != NETEVENT_NOERROR) {
		if(error == NETEVENT_CLOSED)
			comm_base_exit(worker->base);
		else	log_info("control event: %d", error);
		return 0;
	}
	if(ldns_buffer_limit(c->buffer) != sizeof(uint32_t)) {
		fatal_exit("bad control msg length %d", 
			(int)ldns_buffer_limit(c->buffer));
	}
	cmd = ldns_buffer_read_u32(c->buffer);
	switch(cmd) {
	case worker_cmd_quit:
		verbose(VERB_ALGO, "got control cmd quit");
		comm_base_exit(worker->base);
		break;
	default:
		log_err("bad command %d", (int)cmd);
		break;
	}
	return 0;
}

/** check if a delegation is secure */
static enum sec_status
check_delegation_secure(struct reply_info *rep) 
{
	/* return smallest security status */
	size_t i;
	enum sec_status sec = sec_status_secure;
	enum sec_status s;
	for(i=0; i<rep->rrset_count; i++) {
		s = ((struct packed_rrset_data*)rep->rrsets[i])->security;
		if(s < sec)
			sec = s;
	}
	return sec;
}

/** answer nonrecursive query from the cache */
static int
answer_norec_from_cache(struct worker* worker, struct query_info* qinfo,
	uint16_t id, uint16_t flags, struct comm_reply* repinfo, 
	struct edns_data* edns)
{
	/* for a nonrecursive query return either:
	 * 	o an error (servfail; we try to avoid this)
	 * 	o a delegation (closest we have; this routine tries that)
	 * 	o the answer (checked by answer_from_cache) 
	 *
	 * So, grab a delegation from the rrset cache. 
	 * Then check if it needs validation, if so, this routine fails,
	 * so that iterator can prime and validator can verify rrsets.
	 */
	uint16_t udpsize = edns->udp_size;
	int secure = 0;
	uint32_t timenow = (uint32_t)time(0);
	int must_validate = !(flags&BIT_CD) && worker->env.need_to_validate;
	struct dns_msg *msg = NULL;
	struct delegpt *dp;

	dp = dns_cache_find_delegation(&worker->env, qinfo->qname, 
		qinfo->qname_len, qinfo->qtype, qinfo->qclass,
		worker->scratchpad, &msg, timenow);
	if(!dp) { /* no delegation, need to reprime */
		region_free_all(worker->scratchpad);
		return 0;
	}
	if(must_validate) {
		switch(check_delegation_secure(msg->rep)) {
		case sec_status_unchecked:
			/* some rrsets have not been verified yet, go and 
			 * let validator do that */
			region_free_all(worker->scratchpad);
			return 0;
		case sec_status_bogus:
			/* some rrsets are bogus, reply servfail */
			edns->edns_version = EDNS_ADVERTISED_VERSION;
			edns->udp_size = EDNS_ADVERTISED_SIZE;
			edns->ext_rcode = 0;
			edns->bits &= EDNS_DO;
			error_encode(repinfo->c->buffer, LDNS_RCODE_SERVFAIL, 
				&msg->qinfo, id, flags, edns);
			region_free_all(worker->scratchpad);
			return 1;
		case sec_status_secure:
			/* all rrsets are secure */
			secure = 1;
			break;
		case sec_status_indeterminate:
		case sec_status_insecure:
		default:
			/* not secure */
			secure = 0;
			break;
		}
	}
	/* return this delegation from the cache */
	edns->edns_version = EDNS_ADVERTISED_VERSION;
	edns->udp_size = EDNS_ADVERTISED_SIZE;
	edns->ext_rcode = 0;
	edns->bits &= EDNS_DO;
	msg->rep->flags |= BIT_QR|BIT_RA;
	if(!reply_info_answer_encode(&msg->qinfo, msg->rep, id, flags, 
		repinfo->c->buffer, timenow, 1, worker->scratchpad,
		udpsize, edns, (int)(edns->bits & EDNS_DO), secure)) {
		error_encode(repinfo->c->buffer, LDNS_RCODE_SERVFAIL, 
			&msg->qinfo, id, flags, edns);
	}
	region_free_all(worker->scratchpad);
	return 1;
}

/** check cname chain in cache reply */
static int
check_cache_chain(struct reply_info* rep) {
	/* check only answer section rrs for matching cname chain.
	 * the cache may return changed rdata, but owner names are untouched.*/
	size_t i;
	uint8_t* sname = rep->rrsets[0]->rk.dname;
	size_t snamelen = rep->rrsets[0]->rk.dname_len;
	for(i=0; i<rep->an_numrrsets; i++) {
		uint16_t t = ntohs(rep->rrsets[i]->rk.type);
		if(t == LDNS_RR_TYPE_DNAME)
			continue; /* skip dnames; note TTL 0 not cached */
		/* verify that owner matches current sname */
		if(query_dname_compare(sname, rep->rrsets[i]->rk.dname) != 0){
			/* cname chain broken */
			return 0;
		}
		/* if this is a cname; move on */
		if(t == LDNS_RR_TYPE_CNAME) {
			get_cname_target(rep->rrsets[i], &sname, &snamelen);
		}
	}
	return 1;
}

/** check security status in cache reply */
static int
all_rrsets_secure(struct reply_info* rep) {
	size_t i;
	for(i=0; i<rep->rrset_count; i++) {
		if( ((struct packed_rrset_data*)rep->rrsets[i]->entry.data)
			->security != sec_status_secure )
			return 0;
	}
	return 1;
}

/** answer query from the cache */
static int
answer_from_cache(struct worker* worker, struct lruhash_entry* e, uint16_t id,
	uint16_t flags, struct comm_reply* repinfo, struct edns_data* edns)
{
	struct msgreply_entry* mrentry = (struct msgreply_entry*)e->key;
	struct reply_info* rep = (struct reply_info*)e->data;
	uint32_t timenow = time(0);
	uint16_t udpsize = edns->udp_size;
	int secure;
	int must_validate = !(flags&BIT_CD) && worker->env.need_to_validate;
	/* see if it is possible */
	if(rep->ttl <= timenow) {
		/* the rrsets may have been updated in the meantime.
		 * we will refetch the message format from the
		 * authoritative server 
		 */
		return 0;
	}
	if(!rrset_array_lock(rep->ref, rep->rrset_count, timenow))
		return 0;
	/* locked and ids and ttls are OK. */
	/* check CNAME chain (if any) */
	if(rep->an_numrrsets > 0 && (rep->rrsets[0]->rk.type == 
		htons(LDNS_RR_TYPE_CNAME) || rep->rrsets[0]->rk.type == 
		htons(LDNS_RR_TYPE_DNAME))) {
		if(!check_cache_chain(rep)) {
			/* cname chain invalid, redo iterator steps */
			verbose(VERB_ALGO, "Cache reply: cname chain broken");
		bail_out:
			rrset_array_unlock_touch(worker->env.rrset_cache, 
				worker->scratchpad, rep->ref, rep->rrset_count);
			region_free_all(worker->scratchpad);
			return 0;
		}
	}
	/* check security status of the cached answer */
	if( rep->security == sec_status_bogus && must_validate) {
		/* BAD cached */
		edns->edns_version = EDNS_ADVERTISED_VERSION;
		edns->udp_size = EDNS_ADVERTISED_SIZE;
		edns->ext_rcode = 0;
		edns->bits &= EDNS_DO;
		error_encode(repinfo->c->buffer, LDNS_RCODE_SERVFAIL, 
			&mrentry->key, id, flags, edns);
		rrset_array_unlock_touch(worker->env.rrset_cache, 
			worker->scratchpad, rep->ref, rep->rrset_count);
		region_free_all(worker->scratchpad);
		return 1;
	} else if( rep->security == sec_status_unchecked && must_validate) {
		verbose(VERB_ALGO, "Cache reply: unchecked entry needs "
			"validation");
		goto bail_out; /* need to validate cache entry first */
	} else if(rep->security == sec_status_secure) {
		if(all_rrsets_secure(rep))
			secure = 1;
		else	{
			if(must_validate) {
				verbose(VERB_ALGO, "Cache reply: secure entry"
					" changed status");
				goto bail_out; /* rrset changed, re-verify */
			}
			secure = 0;
		}
	} else	secure = 0;

	edns->edns_version = EDNS_ADVERTISED_VERSION;
	edns->udp_size = EDNS_ADVERTISED_SIZE;
	edns->ext_rcode = 0;
	edns->bits &= EDNS_DO;
	if(!reply_info_answer_encode(&mrentry->key, rep, id, flags, 
		repinfo->c->buffer, timenow, 1, worker->scratchpad,
		udpsize, edns, (int)(edns->bits & EDNS_DO), secure)) {
		error_encode(repinfo->c->buffer, LDNS_RCODE_SERVFAIL, 
			&mrentry->key, id, flags, edns);
	}
	/* cannot send the reply right now, because blocking network syscall
	 * is bad while holding locks. */
	rrset_array_unlock_touch(worker->env.rrset_cache, worker->scratchpad,
		rep->ref, rep->rrset_count);
	region_free_all(worker->scratchpad);
	/* go and return this buffer to the client */
	return 1;
}

/**
 * Fill CH class answer into buffer. Keeps query.
 * @param pkt: buffer
 * @param str: string to put into text record (<255).
 * @param edns: edns reply information.
 */
static void
chaos_replystr(ldns_buffer* pkt, const char* str, struct edns_data* edns)
{
	size_t len = strlen(str);
	unsigned int rd = LDNS_RD_WIRE(ldns_buffer_begin(pkt));
	unsigned int cd = LDNS_CD_WIRE(ldns_buffer_begin(pkt));
	if(len>255) len=255; /* cap size of TXT record */
	ldns_buffer_clear(pkt);
	ldns_buffer_skip(pkt, (ssize_t)sizeof(uint16_t)); /* skip id */
	ldns_buffer_write_u16(pkt, (uint16_t)(BIT_QR|BIT_RA));
	if(rd) LDNS_RD_SET(ldns_buffer_begin(pkt));
	if(cd) LDNS_CD_SET(ldns_buffer_begin(pkt));
	ldns_buffer_write_u16(pkt, 1); /* qdcount */
	ldns_buffer_write_u16(pkt, 1); /* ancount */
	ldns_buffer_write_u16(pkt, 0); /* nscount */
	ldns_buffer_write_u16(pkt, 0); /* arcount */
	(void)query_dname_len(pkt); /* skip qname */
	ldns_buffer_skip(pkt, (ssize_t)sizeof(uint16_t)); /* skip qtype */
	ldns_buffer_skip(pkt, (ssize_t)sizeof(uint16_t)); /* skip qclass */
	ldns_buffer_write_u16(pkt, 0xc00c); /* compr ptr to query */
	ldns_buffer_write_u16(pkt, LDNS_RR_TYPE_TXT);
	ldns_buffer_write_u16(pkt, LDNS_RR_CLASS_CH);
	ldns_buffer_write_u32(pkt, 0); /* TTL */
	ldns_buffer_write_u16(pkt, sizeof(uint8_t) + len);
	ldns_buffer_write_u8(pkt, len);
	ldns_buffer_write(pkt, str, len);
	ldns_buffer_flip(pkt);
	edns->edns_version = EDNS_ADVERTISED_VERSION;
	edns->udp_size = EDNS_ADVERTISED_SIZE;
	edns->bits &= EDNS_DO;
	attach_edns_record(pkt, edns);
}

/**
 * Answer CH class queries.
 * @param w: worker
 * @param qinfo: query info. Pointer into packet buffer.
 * @param edns: edns info from query.
 * @param pkt: packet buffer.
 * @return: true if a reply is to be sent.
 */
static int
answer_chaos(struct worker* w, struct query_info* qinfo, 
	struct edns_data* edns, ldns_buffer* pkt)
{
	struct config_file* cfg = w->env.cfg;
	if(qinfo->qtype != LDNS_RR_TYPE_ANY && qinfo->qtype != LDNS_RR_TYPE_TXT)
		return 0;
	if(query_dname_compare(qinfo->qname, 
		(uint8_t*)"\002id\006server") == 0 ||
		query_dname_compare(qinfo->qname, 
		(uint8_t*)"\010hostname\004bind") == 0)
	{
		if(cfg->hide_identity) 
			return 0;
		if(cfg->identity==NULL || cfg->identity[0]==0) {
			char buf[MAXHOSTNAMELEN];
			if (gethostname(buf, MAXHOSTNAMELEN) == 0)
				chaos_replystr(pkt, buf, edns);
			else 	{
				log_err("gethostname: %s", strerror(errno));
				chaos_replystr(pkt, "no hostname", edns);
			}
		}
		else 	chaos_replystr(pkt, cfg->identity, edns);
		return 1;
	}
	if(query_dname_compare(qinfo->qname, 
		(uint8_t*)"\007version\006server") == 0 ||
		query_dname_compare(qinfo->qname, 
		(uint8_t*)"\007version\004bind") == 0)
	{
		if(cfg->hide_version) 
			return 0;
		if(cfg->version==NULL || cfg->version[0]==0)
			chaos_replystr(pkt, PACKAGE_STRING, edns);
		else 	chaos_replystr(pkt, cfg->version, edns);
		return 1;
	}
	return 0;
}

/** handles callbacks from listening event interface */
static int 
worker_handle_request(struct comm_point* c, void* arg, int error,
	struct comm_reply* repinfo)
{
	struct worker* worker = (struct worker*)arg;
	int ret;
	hashvalue_t h;
	struct lruhash_entry* e;
	struct query_info qinfo;
	struct edns_data edns;

	if(error != NETEVENT_NOERROR) {
		log_err("handle request called with err=%d", error);
		return 0;
	}
	if((ret=worker_check_request(c->buffer, worker)) != 0) {
		verbose(VERB_ALGO, "worker check request: bad query.");
		if(ret != -1) {
			LDNS_QR_SET(ldns_buffer_begin(c->buffer));
			LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), ret);
			return 1;
		}
		comm_point_drop_reply(repinfo);
		return 0;
	}
	worker->stats.num_queries++;
	/* see if query is in the cache */
	if(!query_info_parse(&qinfo, c->buffer)) {
		verbose(VERB_ALGO, "worker parse request: formerror.");
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), 
			LDNS_RCODE_FORMERR);
		return 1;
	}
	if(qinfo.qtype == LDNS_RR_TYPE_AXFR || 
		qinfo.qtype == LDNS_RR_TYPE_IXFR) {
		verbose(VERB_ALGO, "worker request: refused zone transfer.");
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), 
			LDNS_RCODE_REFUSED);
		return 1;
	}
	if((ret=parse_edns_from_pkt(c->buffer, &edns)) != 0) {
		verbose(VERB_ALGO, "worker parse edns: formerror.");
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), ret);
		return 1;
	}
	if(edns.edns_present && edns.edns_version != 0) {
		edns.ext_rcode = (uint8_t)(EDNS_RCODE_BADVERS>>4);
		edns.edns_version = EDNS_ADVERTISED_VERSION;
		edns.udp_size = EDNS_ADVERTISED_SIZE;
		edns.bits &= EDNS_DO;
		verbose(VERB_ALGO, "query with bad edns version.");
		error_encode(c->buffer, EDNS_RCODE_BADVERS&0xf, &qinfo,
			*(uint16_t*)ldns_buffer_begin(c->buffer),
			ldns_buffer_read_u16_at(c->buffer, 2), NULL);
		attach_edns_record(c->buffer, &edns);
		return 1;
	}
	if(edns.edns_present && edns.udp_size < NORMAL_UDP_SIZE &&
		worker->daemon->cfg->harden_short_bufsize) {
		verbose(VERB_DETAIL, "worker request: EDNS bufsize %d ignored",
			(int)edns.udp_size);
		edns.udp_size = NORMAL_UDP_SIZE;
	}
	if(edns.edns_present && edns.udp_size < LDNS_HEADER_SIZE) {
		verbose(VERB_ALGO, "worker request: edns is too small.");
		LDNS_QR_SET(ldns_buffer_begin(c->buffer));
		LDNS_TC_SET(ldns_buffer_begin(c->buffer));
		LDNS_RCODE_SET(ldns_buffer_begin(c->buffer), 
			LDNS_RCODE_SERVFAIL);
		ldns_buffer_set_position(c->buffer, LDNS_HEADER_SIZE);
		ldns_buffer_flip(c->buffer);
		return 1;
	}
	if(c->type != comm_udp)
		edns.udp_size = 65535; /* max size for TCP replies */
	if(qinfo.qclass == LDNS_RR_CLASS_CH && answer_chaos(worker, &qinfo,
		&edns, c->buffer)) {
		return 1;
	}
	h = query_info_hash(&qinfo);
	if((e=slabhash_lookup(worker->env.msg_cache, h, &qinfo, 0))) {
		/* answer from cache - we have acquired a readlock on it */
		if(answer_from_cache(worker, e, 
			*(uint16_t*)ldns_buffer_begin(c->buffer), 
			ldns_buffer_read_u16_at(c->buffer, 2), repinfo, 
			&edns)) {
			lock_rw_unlock(&e->lock);
			return 1;
		}
		verbose(VERB_DETAIL, "answer from the cache -- data has timed out");
		lock_rw_unlock(&e->lock);
	}
	if(!LDNS_RD_WIRE(ldns_buffer_begin(c->buffer))) {
		if(answer_norec_from_cache(worker, &qinfo,
			*(uint16_t*)ldns_buffer_begin(c->buffer), 
			ldns_buffer_read_u16_at(c->buffer, 2), repinfo, 
			&edns)) {
			return 1;
		}
		verbose(VERB_DETAIL, "answer norec from cache -- "
			"need to validate or not primed");
	}
	ldns_buffer_rewind(c->buffer);
	server_stats_querymiss(&worker->stats, worker);

	/* grab a work request structure for this new request */
	if(worker->env.mesh->all.count > worker->request_size) {
		/* we could get this due to a slow tcp incoming query, 
		   that started before we performed listen_pushback */
		verbose(VERB_DETAIL, "worker: too many incoming requests "
			"active. dropping incoming query.");
		verbose(VERB_ALGO, "currently servicing %d of %d queries", 
			(int)worker->env.mesh->all.count, 
			(int)worker->request_size);
		worker->stats.num_query_list_exceeded++;
		comm_point_drop_reply(repinfo);
		query_info_clear(&qinfo);
		return 0;
	}
	mesh_new_client(worker->env.mesh, &qinfo, 
		ldns_buffer_read_u16_at(c->buffer, 2),
		&edns, repinfo, *(uint16_t*)ldns_buffer_begin(c->buffer));

	if(worker->env.mesh->all.count == worker->request_size)  {
		/* the max request number has been reached, stop accepting */
		listen_pushback(worker->front);
	}
	worker_mem_report(worker, NULL);
	return 0;
}

/** worker signal callback */
void 
worker_sighandler(int sig, void* arg)
{
	/* note that log, print, syscalls here give race conditions. */
	struct worker* worker = (struct worker*)arg;
	switch(sig) {
		case SIGHUP:
			log_info("caught signal SIGHUP");
			worker->need_to_restart = 1;
			comm_base_exit(worker->base);
			break;
		case SIGINT:
			log_info("caught signal SIGINT");
			worker->need_to_restart = 0;
			comm_base_exit(worker->base);
			break;
		case SIGQUIT:
			log_info("caught signal SIGQUIT");
			worker->need_to_restart = 0;
			comm_base_exit(worker->base);
			break;
		case SIGTERM:
			log_info("caught signal SIGTERM");
			worker->need_to_restart = 0;
			comm_base_exit(worker->base);
			break;
		default:
			log_err("unknown signal: %d, ignored", sig);
			break;
	}
}

struct worker* 
worker_create(struct daemon* daemon, int id)
{
	struct worker* worker = (struct worker*)calloc(1, 
		sizeof(struct worker));
	if(!worker) 
		return NULL;
	worker->daemon = daemon;
	worker->thread_num = id;
	worker->cmd_send_fd = -1;
	worker->cmd_recv_fd = -1;
	if(id != 0) {
		int sv[2];
		/* create socketpair to communicate with worker */
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
			free(worker);
			log_err("socketpair: %s", strerror(errno));
			return NULL;
		}
		if(!fd_set_nonblock(sv[0]) || !fd_set_nonblock(sv[1])) {
			close(sv[0]);
			close(sv[1]);
			free(worker);
			return NULL;
		}
		worker->cmd_send_fd = sv[0];
		worker->cmd_recv_fd = sv[1];
	}
	return worker;
}

int
worker_init(struct worker* worker, struct config_file *cfg, 
	struct listen_port* ports, size_t buffer_size, int do_sigs)
{
	unsigned int seed;
	int startport;
	worker->need_to_restart = 0;
	worker->base = comm_base_create();
	if(!worker->base) {
		log_err("could not create event handling base");
		worker_delete(worker);
		return 0;
	}
	if(do_sigs) {
		ub_thread_sig_unblock(SIGHUP);
		ub_thread_sig_unblock(SIGINT);
		ub_thread_sig_unblock(SIGQUIT);
		ub_thread_sig_unblock(SIGTERM);
#ifndef LIBEVENT_SIGNAL_PROBLEM
		worker->comsig = comm_signal_create(worker->base, 
			worker_sighandler, worker);
		if(!worker->comsig || !comm_signal_bind(worker->comsig, SIGHUP)
			|| !comm_signal_bind(worker->comsig, SIGINT)
			|| !comm_signal_bind(worker->comsig, SIGTERM)
			|| !comm_signal_bind(worker->comsig, SIGQUIT)) {
			log_err("could not create signal handlers");
			worker_delete(worker);
			return 0;
		}
#endif /* LIBEVENT_SIGNAL_PROBLEM */
	} else { /* !do_sigs */
		worker->comsig = 0;
	}
	/* init random(), large table size. */
	if(!(worker->rndstate = (struct ub_randstate*)calloc(1,
		sizeof(struct ub_randstate)))) {
		log_err("malloc rndtable failed.");
		worker_delete(worker);
		return 0;
	}
	seed = (unsigned int)time(NULL) ^ (unsigned int)getpid() ^
		(((unsigned int)worker->thread_num)<<17);
		/* shift thread_num so it does not match out pid bits */
	if(!ub_initstate(seed, worker->rndstate, RND_STATE_SIZE)) {
		log_err("could not init random numbers.");
		worker_delete(worker);
		return 0;
	}
	worker->front = listen_create(worker->base, ports,
		buffer_size, worker_handle_request, worker);
	if(!worker->front) {
		log_err("could not create listening sockets");
		worker_delete(worker);
		return 0;
	}
	startport  = cfg->outgoing_base_port + 
		cfg->outgoing_num_ports * worker->thread_num;
	worker->back = outside_network_create(worker->base,
		buffer_size, (size_t)cfg->outgoing_num_ports, cfg->ifs, 
		cfg->num_ifs, cfg->do_ip4, cfg->do_ip6, startport, 
		cfg->do_tcp?cfg->outgoing_num_tcp:0, 
		worker->daemon->env->infra_cache, worker->rndstate);
	if(!worker->back) {
		log_err("could not create outgoing sockets");
		worker_delete(worker);
		return 0;
	}
	outside_network_set_secondary_buffer(worker->back, 
		worker->front->udp_buff);
	if(worker->thread_num != 0) {
		/* start listening to commands */
		if(!(worker->cmd_com=comm_point_create_local(worker->base, 
			worker->cmd_recv_fd, buffer_size, 
			worker_handle_control_cmd, worker))) {
			log_err("could not create control compt.");
			worker_delete(worker);
			return 0;
		}
	}
	worker->scratchpad = region_create_custom(malloc, free, 
		65536, 8192, 32, 1);
	if(!worker->scratchpad) {
		log_err("malloc failure");
		worker_delete(worker);
		return 0;
	}
	worker->request_size = cfg->num_queries_per_thread;

	server_stats_init(&worker->stats);
	alloc_init(&worker->alloc, &worker->daemon->superalloc, 
		worker->thread_num);
	worker->env = *worker->daemon->env;
	worker->env.worker = worker;
	worker->env.alloc = &worker->alloc;
	worker->env.rnd = worker->rndstate;
	worker->env.scratch = worker->scratchpad;
	worker->env.mesh = mesh_create(worker->daemon->num_modules,
		worker->daemon->modfunc, &worker->env);
	worker->env.detach_subs = &mesh_detach_subs;
	worker->env.attach_sub = &mesh_attach_sub;
	worker->env.kill_sub = &mesh_state_delete;
	worker->env.detect_cycle = &mesh_detect_cycle;
	worker->env.scratch_buffer = ldns_buffer_new(65536);
	if(!worker->env.mesh || !worker->env.scratch_buffer) {
		worker_delete(worker);
		return 0;
	}
	worker_mem_report(worker, NULL);
	return 1;
}

void 
worker_work(struct worker* worker)
{
	comm_base_dispatch(worker->base);
}

void 
worker_delete(struct worker* worker)
{
	if(!worker) 
		return;
	mesh_stats(worker->env.mesh, "mesh has");
	server_stats_log(&worker->stats, worker->thread_num);
	worker_mem_report(worker, NULL);
	mesh_delete(worker->env.mesh);
	ldns_buffer_free(worker->env.scratch_buffer);
	listen_delete(worker->front);
	outside_network_delete(worker->back);
	comm_signal_delete(worker->comsig);
	comm_point_delete(worker->cmd_com);
	comm_base_delete(worker->base);
	ub_randfree(worker->rndstate);
	free(worker->rndstate);
	/* close fds after deleting commpoints, to be sure.
	   Also epoll does not like closing fd before event_del */
	if(worker->cmd_send_fd != -1)
		close(worker->cmd_send_fd);
	worker->cmd_send_fd = -1;
	if(worker->cmd_recv_fd != -1)
		close(worker->cmd_recv_fd);
	worker->cmd_recv_fd = -1;
	alloc_clear(&worker->alloc);
	region_destroy(worker->scratchpad);
	free(worker);
}

int 
worker_send_packet(ldns_buffer* pkt, struct sockaddr_storage* addr,
        socklen_t addrlen, int timeout, struct module_qstate* q, int use_tcp)
{
	struct worker* worker = q->env->worker;
	if(use_tcp) {
		return pending_tcp_query(worker->back, pkt, addr, addrlen, 
			timeout, worker_handle_reply, q, 
			worker->rndstate) != 0;
	}
	return pending_udp_query(worker->back, pkt, addr, addrlen, 
		timeout*1000, worker_handle_reply, q, 
		worker->rndstate) != 0;
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

struct outbound_entry*
worker_send_query(uint8_t* qname, size_t qnamelen, uint16_t qtype,
	uint16_t qclass, uint16_t flags, int dnssec,
	struct sockaddr_storage* addr, socklen_t addrlen,
	struct module_qstate* q)
{
	struct worker* worker = q->env->worker;
	struct outbound_entry* e = (struct outbound_entry*)region_alloc(
		q->region, sizeof(*e));
	if(!e) 
		return NULL;
	e->qstate = q;
	e->qsent = outnet_serviced_query(worker->back, qname,
		qnamelen, qtype, qclass, flags, dnssec, addr, addrlen, 
		worker_handle_service_reply, e, worker->back->udp_buff,
		&outbound_entry_compare);
	if(!e->qsent) {
		free(e);
		return NULL;
	}
	return e;
}
