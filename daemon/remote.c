/*
 * daemon/remote.c - remote control for the unbound daemon.
 *
 * Copyright (c) 2008, NLnet Labs. All rights reserved.
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
 * This file contains the remote control functionality for the daemon.
 * The remote control can be performed using either the commandline
 * unbound-control tool, or a SSLv3/TLS capable web browser. 
 * The channel is secured using SSLv3 or TLSv1, and certificates.
 * Both the server and the client(control tool) have their own keys.
 */
#include "config.h"
#include "daemon/remote.h"
#include "daemon/worker.h"
#include "daemon/daemon.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "services/listen_dnsport.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

/** log ssl crypto err */
static void
log_crypto_err(const char* str)
{
	/* error:[error code]:[library name]:[function name]:[reason string] */
	char buf[128];
	unsigned long e;
	ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
	log_err("%s crypto %s", str, buf);
	while( (e=ERR_get_error()) ) {
		ERR_error_string_n(e, buf, sizeof(buf));
		log_err("and additionally crypto %s", buf);
	}
}

struct daemon_remote*
daemon_remote_create(struct worker* worker)
{
	char* s_cert;
	char* s_key;
	struct config_file* cfg = worker->daemon->cfg;
	struct daemon_remote* rc = (struct daemon_remote*)calloc(1, 
		sizeof(*rc));
	if(!rc) {
		log_err("out of memory in daemon_remote_create");
		return NULL;
	}
	rc->worker = worker;
	rc->max_active = 10;

	if(!cfg->remote_control_enable) {
		rc->ctx = NULL;
		return rc;
	}
	rc->ctx = SSL_CTX_new(SSLv23_server_method());
	if(!rc->ctx) {
		log_crypto_err("could not SSL_CTX_new");
		free(rc);
		return NULL;
	}
	/* no SSLv2 because has defects */
	if(!(SSL_CTX_set_options(rc->ctx, SSL_OP_NO_SSLv2) & SSL_OP_NO_SSLv2)){
		log_crypto_err("could not set SSL_OP_NO_SSLv2");
		daemon_remote_delete(rc);
		return NULL;
	}
	s_cert = cfg->server_cert_file;
	s_key = cfg->server_key_file;
	if(cfg->chrootdir && cfg->chrootdir[0]) {
		if(strncmp(s_cert, cfg->chrootdir, strlen(cfg->chrootdir))==0)
			s_cert += strlen(cfg->chrootdir);
		if(strncmp(s_key, cfg->chrootdir, strlen(cfg->chrootdir))==0)
			s_key += strlen(cfg->chrootdir);
	}
	verbose(VERB_ALGO, "setup SSL certificates");
	if (!SSL_CTX_use_certificate_file(rc->ctx,s_cert,SSL_FILETYPE_PEM)
		|| !SSL_CTX_use_PrivateKey_file(rc->ctx,s_key,SSL_FILETYPE_PEM)
		|| !SSL_CTX_check_private_key(rc->ctx)) {
		log_crypto_err("Error setting up SSL_CTX key and cert");
		daemon_remote_delete(rc);
		return NULL;
	}
	if(!SSL_CTX_load_verify_locations(rc->ctx, s_cert, NULL)) {
		log_crypto_err("Error setting up SSL_CTX verify locations");
		daemon_remote_delete(rc);
		return NULL;
	}
	SSL_CTX_set_client_CA_list(rc->ctx, SSL_load_client_CA_file(s_cert));
	SSL_CTX_set_verify(rc->ctx, SSL_VERIFY_PEER, NULL);

	return rc;
}

void daemon_remote_delete(struct daemon_remote* rc)
{
	struct rc_state* p, *np;
	if(!rc) return;
	/* but do not close the ports */
	listen_list_delete(rc->accept_list);
	/* do close these sockets */
	p = rc->busy_list;
	while(p) {
		np = p->next;
		if(p->ssl)
			SSL_free(p->ssl);
		comm_point_delete(p->c);
		free(p);
		p = np;
	}
	SSL_CTX_free(rc->ctx);
	free(rc);
}

/**
 * Add and open a new control port
 * @param ip: ip str
 * @param nr: port nr
 * @param list: list head
 * @param noproto_is_err: if lack of protocol support is an error.
 * @return false on failure.
 */
static int
add_open(char* ip, int nr, struct listen_port** list, int noproto_is_err)
{
	struct addrinfo hints;
	struct addrinfo* res;
	struct listen_port* n;
	int noproto;
	int fd, r;
	char port[15];
	snprintf(port, sizeof(port), "%d", nr);
	port[sizeof(port)-1]=0;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
	if((r = getaddrinfo(ip, port, &hints, &res)) != 0 || !res) {
                log_err("control interface %s:%s getaddrinfo: %s %s",
			ip?ip:"default", port, gai_strerror(r),
#ifdef EAI_SYSTEM
			r==EAI_SYSTEM?(char*)strerror(errno):""
#else
			""
#endif
			);
	}

	/* open fd */
	fd = create_tcp_accept_sock(res, 1, &noproto);
	freeaddrinfo(res);
	if(fd == -1 && noproto) {
		if(!noproto_is_err)
			return 1; /* return success, but do nothing */
		log_err("cannot open control interface %s %d : "
			"protocol not supported", ip, nr);
		return 0;
	}
	if(fd == -1) {
		log_err("cannot open control interface %s %d", ip, nr);
		return 0;
	}

	/* alloc */
	n = (struct listen_port*)calloc(1, sizeof(*n));
	if(!n) {
		close(fd);
		log_err("out of memory");
		return 0;
	}
	n->next = *list;
	*list = n;
	n->fd = fd;
	return 1;
}

struct listen_port* daemon_remote_open_ports(struct config_file* cfg)
{
	struct listen_port* l = NULL;
	log_assert(cfg->remote_control_enable && cfg->control_port);
	if(cfg->control_ifs) {
		struct config_strlist* p = cfg->control_ifs;
		for(p = cfg->control_ifs; p; p = p->next) {
			if(!add_open(p->str, cfg->control_port, &l, 1)) {
				listening_ports_free(l);
				return NULL;
			}
		}
	} else {
		/* defaults */
		if(cfg->do_ip6 &&
			!add_open("::1", cfg->control_port, &l, 0)) {
			listening_ports_free(l);
			return NULL;
		}
		if(cfg->do_ip4 &&
			!add_open("127.0.0.1", cfg->control_port, &l, 1)) {
			listening_ports_free(l);
			return NULL;
		}
	}
	return l;
}

/** open accept commpoint */
static int
accept_open(struct daemon_remote* rc, int fd)
{
	struct listen_list* n = (struct listen_list*)malloc(sizeof(*n));
	if(!n) {
		log_err("out of memory");
		return 0;
	}
	n->next = rc->accept_list;
	rc->accept_list = n;
	/* open commpt */
	n->com = comm_point_create_raw(rc->worker->base, fd, 0, 
		&remote_accept_callback, rc);
	if(!n->com)
		return 0;
	/* keep this port open, its fd is kept in the rc portlist */
	n->com->do_not_close = 1;
	return 1;
}

int daemon_remote_open_accept(struct daemon_remote* rc, 
	struct listen_port* ports)
{
	struct listen_port* p;
	for(p = ports; p; p = p->next) {
		if(!accept_open(rc, p->fd)) {
			log_err("could not create accept comm point");
			return 0;
		}
	}
	return 1;
}

int remote_accept_callback(struct comm_point* c, void* arg, int err, 
	struct comm_reply* ATTR_UNUSED(rep))
{
	struct daemon_remote* rc = (struct daemon_remote*)arg;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int newfd;
	struct rc_state* n;
	if(err != NETEVENT_NOERROR) {
		log_err("error %d on remote_accept_callback", err);
		return 0;
	}
	/* perform the accept */
	newfd = comm_point_perform_accept(c, &addr, &addrlen);
	if(newfd == -1)
		return 0;
	/* create new commpoint unless we are servicing already */
	if(rc->active >= rc->max_active) {
		log_warn("drop incoming remote control: too many connections");
		comm_point_stop_listening(c);
		close(newfd);
		return 0;
	}

	/* setup commpoint to service the remote control command */
	n = (struct rc_state*)calloc(1, sizeof(*n));
	if(!n) {
		log_err("out of memory");
		close(newfd);
		return 0;
	}
	/* start in reading state */
	n->c = comm_point_create_raw(rc->worker->base, newfd, 0, 
		&remote_control_callback, n);
	if(!n->c) {
		log_err("out of memory");
		close(newfd);
		free(n);
		return 0;
	}
	log_addr(VERB_QUERY, "new control connection from", &addr, addrlen);
	n->c->do_not_close = 0;
	comm_point_stop_listening(n->c);
	comm_point_start_listening(n->c, -1, REMOTE_CONTROL_TCP_TIMEOUT);
	memcpy(&n->c->repinfo.addr, &addr, addrlen);
	n->c->repinfo.addrlen = addrlen;
	n->shake_state = rc_hs_read;
	n->ssl = SSL_new(rc->ctx);
	if(!n->ssl) {
		log_crypto_err("could not SSL_new");
		close(newfd);
		free(n);
		return 0;
	}
	SSL_set_accept_state(n->ssl);
        (void)SSL_set_mode(n->ssl, SSL_MODE_AUTO_RETRY);
	if(!SSL_set_fd(n->ssl, newfd)) {
		log_crypto_err("could not SSL_set_fd");
		close(newfd);
		SSL_free(n->ssl);
		free(n);
		return 0;
	}

	n->rc = rc;
	n->next = rc->busy_list;
	rc->busy_list = n;
	rc->active ++;

	/* perform the first nonblocking read already, for windows, 
	 * so it can return wouldblock. could be faster too. */
	(void)remote_control_callback(n->c, n, NETEVENT_NOERROR, NULL);
	return 0;
}

/** delete from list */
static void
state_list_remove_elem(struct rc_state** list, struct comm_point* c)
{
	while(*list) {
		if( (*list)->c == c) {
			*list = (*list)->next;
			return;
		}
		list = &(*list)->next;
	}
}

/** decrease active count and remove commpoint from busy list */
static void
clean_point(struct daemon_remote* rc, struct rc_state* s)
{
	state_list_remove_elem(&rc->busy_list, s->c);
	rc->active --;
	if(s->ssl) {
		SSL_shutdown(s->ssl);
		SSL_free(s->ssl);
	}
	comm_point_delete(s->c);
	free(s);
}

/** handle remote control request */
static void
handle_req(struct daemon_remote* rc, struct rc_state* s, SSL* ssl)
{
	char* msg = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n"
		"unbound server control channel\n";
	int r;
	char buf[1024];
	fd_set_block(s->c->fd);

	ERR_clear_error();
	if((r=SSL_read(ssl, buf, (int)sizeof(buf)-1)) <= 0) {
		if(SSL_get_error(ssl, r) == SSL_ERROR_ZERO_RETURN)
			return;
		log_crypto_err("could not SSL_read");
		return;
	}
	buf[r] = 0;
	log_info("got '%s'", buf);

	ERR_clear_error();
	if((r=SSL_write(ssl, msg, (int)strlen(msg))) <= 0) {
		if(SSL_get_error(ssl, r) == SSL_ERROR_ZERO_RETURN)
			return;
		log_crypto_err("could not SSL_write");
		return;
	}
}

int remote_control_callback(struct comm_point* c, void* arg, int err, 
	struct comm_reply* ATTR_UNUSED(rep))
{
	struct rc_state* s = (struct rc_state*)arg;
	struct daemon_remote* rc = s->rc;
	int r;
	if(err != NETEVENT_NOERROR) {
		if(err==NETEVENT_TIMEOUT) 
			log_err("remote control timed out");
		clean_point(rc, s);
		return 0;
	}
	/* (continue to) setup the SSL connection */
	ERR_clear_error();
	r = SSL_do_handshake(s->ssl);
	if(r != 1) {
		r = SSL_get_error(s->ssl, r);
		if(r == SSL_ERROR_WANT_READ) {
			if(s->shake_state == rc_hs_read) {
				/* try again later */
				return 0;
			}
			s->shake_state = rc_hs_read;
			comm_point_listen_for_rw(c, 1, 0);
			return 0;
		} else if(r == SSL_ERROR_WANT_WRITE) {
			if(s->shake_state == rc_hs_write) {
				/* try again later */
				return 0;
			}
			s->shake_state = rc_hs_write;
			comm_point_listen_for_rw(c, 0, 1);
			return 0;
		} else {
			log_crypto_err("remote control failed ssl");
			clean_point(rc, s);
			return 0;
		}
	}
	s->shake_state = rc_none;

	/* once handshake has completed, check authentication */
	if(SSL_get_verify_result(s->ssl) == X509_V_OK) {
		X509* x = SSL_get_peer_certificate(s->ssl);
		if(!x) {
			verbose(VERB_DETAIL, "remote control connection "
				"provided no client certificate");
			clean_point(rc, s);
			return 0;
		}
		verbose(VERB_ALGO, "remote control connection authenticated");
		X509_free(x);
	} else {
		verbose(VERB_DETAIL, "remote control connection failed to "
			"authenticate with client certificate");
		clean_point(rc, s);
		return 0;
	}

	/* if OK start to actually handle the request */
	handle_req(rc, s, s->ssl);

	verbose(VERB_ALGO, "remote control operation completed");
	clean_point(rc, s);
	return 0;
}
