/*
 * configparser.y -- yacc grammar for unbound configuration files
 *
 * Copyright (c) 2001-2006, NLnet Labs. All rights reserved.
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

%{
#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "util/configyyrename.h"
#include "util/config_file.h"
#include "util/net_help.h"

int ub_c_lex(void);
void ub_c_error(const char *message);

/* these need to be global, otherwise they cannot be used inside yacc */
extern struct config_parser_state* cfg_parser;

#if 0
#define OUTYY(s)  printf s /* used ONLY when debugging */
#else
#define OUTYY(s)
#endif

%}
%union {
	char*	str;
}

%token SPACE LETTER NEWLINE COMMENT COLON ANY ZONESTR
%token <str> STRING
%token VAR_SERVER VAR_VERBOSITY VAR_NUM_THREADS VAR_PORT
%token VAR_OUTGOING_PORT VAR_OUTGOING_RANGE VAR_INTERFACE
%token VAR_DO_IP4 VAR_DO_IP6 VAR_DO_UDP VAR_DO_TCP 
%token VAR_CHROOT VAR_USERNAME VAR_DIRECTORY VAR_LOGFILE VAR_PIDFILE
%token VAR_MSG_CACHE_SIZE VAR_MSG_CACHE_SLABS VAR_NUM_QUERIES_PER_THREAD
%token VAR_RRSET_CACHE_SIZE VAR_RRSET_CACHE_SLABS VAR_OUTGOING_NUM_TCP
%token VAR_INFRA_HOST_TTL VAR_INFRA_LAME_TTL VAR_INFRA_CACHE_SLABS
%token VAR_INFRA_CACHE_NUMHOSTS VAR_INFRA_CACHE_NUMLAME VAR_NAME
%token VAR_STUB_ZONE VAR_STUB_HOST VAR_STUB_ADDR VAR_TARGET_FETCH_POLICY
%token VAR_HARDEN_SHORT_BUFSIZE VAR_HARDEN_LARGE_QUERIES
%token VAR_FORWARD_ZONE VAR_FORWARD_HOST VAR_FORWARD_ADDR
%token VAR_DO_NOT_QUERY_ADDRESS VAR_HIDE_IDENTITY VAR_HIDE_VERSION
%token VAR_IDENTITY VAR_VERSION VAR_HARDEN_GLUE VAR_MODULE_CONF
%token VAR_TRUST_ANCHOR_FILE

%%
toplevelvars: /* empty */ | toplevelvars toplevelvar ;
toplevelvar: serverstart contents_server | stubstart contents_stub |
	forwardstart contents_forward
	;

/* server: declaration */
serverstart: VAR_SERVER
	{ OUTYY(("\nP(server:)\n")); 
		if(cfg_parser->server_settings_seen) {
			yyerror("duplicate server: element.");
		}
		cfg_parser->server_settings_seen = 1;
	}
	;
contents_server: contents_server content_server 
	| ;
content_server: server_num_threads | server_verbosity | server_port |
	server_outgoing_port | server_outgoing_range | server_do_ip4 |
	server_do_ip6 | server_do_udp | server_do_tcp | 
	server_interface | server_chroot | server_username | 
	server_directory | server_logfile | server_pidfile |
	server_msg_cache_size | server_msg_cache_slabs |
	server_num_queries_per_thread | server_rrset_cache_size | 
	server_rrset_cache_slabs | server_outgoing_num_tcp | 
	server_infra_host_ttl | server_infra_lame_ttl | 
	server_infra_cache_slabs | server_infra_cache_numhosts |
	server_infra_cache_numlame | server_target_fetch_policy | 
	server_harden_short_bufsize | server_harden_large_queries |
	server_do_not_query_address | server_hide_identity |
	server_hide_version | server_identity | server_version |
	server_harden_glue | server_module_conf | server_trust_anchor_file
	;
stubstart: VAR_STUB_ZONE
	{
		struct config_stub* s;
		OUTYY(("\nP(stub_zone:)\n")); 
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->stubs;
			cfg_parser->cfg->stubs = s;
		} else 
			yyerror("out of memory");
	}
	;
contents_stub: contents_stub content_stub 
	| ;
content_stub: stub_name | stub_host | stub_addr 
	;
forwardstart: VAR_FORWARD_ZONE
	{
		struct config_stub* s;
		OUTYY(("\nP(forward_zone:)\n")); 
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->forwards;
			cfg_parser->cfg->forwards = s;
		} else 
			yyerror("out of memory");
	}
	;
contents_forward: contents_forward content_forward 
	| ;
content_forward: forward_name | forward_host | forward_addr 
	;
server_num_threads: VAR_NUM_THREADS STRING 
	{ 
		OUTYY(("P(server_num_threads:%s)\n", $2)); 
		if(atoi($2) == 0 && strcmp($2, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi($2);
		free($2);
	}
	;
server_verbosity: VAR_VERBOSITY STRING 
	{ 
		OUTYY(("P(server_verbosity:%s)\n", $2)); 
		if(atoi($2) == 0 && strcmp($2, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi($2);
		free($2);
	}
	;
server_port: VAR_PORT STRING
	{
		OUTYY(("P(server_port:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi($2);
		free($2);
	}
	;
server_interface: VAR_INTERFACE STRING
	{
		OUTYY(("P(server_interface:%s)\n", $2));
		if(cfg_parser->cfg->num_ifs == 0)
			cfg_parser->cfg->ifs = calloc(1, sizeof(char*));
		else 	cfg_parser->cfg->ifs = realloc(cfg_parser->cfg->ifs,
				(cfg_parser->cfg->num_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->ifs[cfg_parser->cfg->num_ifs++] = $2;
	}
	;
server_outgoing_port: VAR_OUTGOING_PORT STRING
	{
		OUTYY(("P(server_outgoing_port:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->outgoing_base_port = atoi($2);
		free($2);
	}
	;
server_outgoing_range: VAR_OUTGOING_RANGE STRING
	{
		OUTYY(("P(server_outgoing_range:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi($2);
		free($2);
	}
	;
server_outgoing_num_tcp: VAR_OUTGOING_NUM_TCP STRING
	{
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", $2));
		if(atoi($2) == 0 && strcmp($2, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi($2);
		free($2);
	}
	;
server_do_ip4: VAR_DO_IP4 STRING
	{
		OUTYY(("P(server_do_ip4:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp($2, "yes")==0);
		free($2);
	}
	;
server_do_ip6: VAR_DO_IP6 STRING
	{
		OUTYY(("P(server_do_ip6:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp($2, "yes")==0);
		free($2);
	}
	;
server_do_udp: VAR_DO_UDP STRING
	{
		OUTYY(("P(server_do_udp:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp($2, "yes")==0);
		free($2);
	}
	;
server_do_tcp: VAR_DO_TCP STRING
	{
		OUTYY(("P(server_do_tcp:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp($2, "yes")==0);
		free($2);
	}
	;
server_chroot: VAR_CHROOT STRING
	{
		OUTYY(("P(server_chroot:%s)\n", $2));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = $2;
	}
	;
server_username: VAR_USERNAME STRING
	{
		OUTYY(("P(server_username:%s)\n", $2));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = $2;
	}
	;
server_directory: VAR_DIRECTORY STRING
	{
		OUTYY(("P(server_directory:%s)\n", $2));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = $2;
	}
	;
server_logfile: VAR_LOGFILE STRING
	{
		OUTYY(("P(server_logfile:%s)\n", $2));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = $2;
	}
	;
server_pidfile: VAR_PIDFILE STRING
	{
		OUTYY(("P(server_pidfile:%s)\n", $2));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = $2;
	}
	;
server_trust_anchor_file: VAR_TRUST_ANCHOR_FILE STRING
	{
		OUTYY(("P(server_trust_anchor_file:%s)\n", $2));
		free(cfg_parser->cfg->trust_anchor_file);
		cfg_parser->cfg->trust_anchor_file = $2;
	}
	;
server_hide_identity: VAR_HIDE_IDENTITY STRING
	{
		OUTYY(("P(server_hide_identity:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp($2, "yes")==0);
		free($2);
	}
	;
server_hide_version: VAR_HIDE_VERSION STRING
	{
		OUTYY(("P(server_hide_version:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp($2, "yes")==0);
		free($2);
	}
	;
server_identity: VAR_IDENTITY STRING
	{
		OUTYY(("P(server_identity:%s)\n", $2));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = $2;
	}
	;
server_version: VAR_VERSION STRING
	{
		OUTYY(("P(server_version:%s)\n", $2));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = $2;
	}
	;
server_msg_cache_size: VAR_MSG_CACHE_SIZE STRING
	{
		OUTYY(("P(server_msg_cache_size:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->msg_cache_size = atoi($2);
		free($2);
	}
	;
server_msg_cache_slabs: VAR_MSG_CACHE_SLABS STRING
	{
		OUTYY(("P(server_msg_cache_slabs:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->msg_cache_slabs = atoi($2);
			if(!is_pow2(cfg_parser->cfg->msg_cache_slabs))
				yyerror("must be a power of 2");
		}
		free($2);
	}
	;
server_num_queries_per_thread: VAR_NUM_QUERIES_PER_THREAD STRING
	{
		OUTYY(("P(server_num_queries_per_thread:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi($2);
		free($2);
	}
	;
server_rrset_cache_size: VAR_RRSET_CACHE_SIZE STRING
	{
		OUTYY(("P(server_rrset_cache_size:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->rrset_cache_size = atoi($2);
		free($2);
	}
	;
server_rrset_cache_slabs: VAR_RRSET_CACHE_SLABS STRING
	{
		OUTYY(("P(server_rrset_cache_slabs:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->rrset_cache_slabs = atoi($2);
			if(!is_pow2(cfg_parser->cfg->rrset_cache_slabs))
				yyerror("must be a power of 2");
		}
		free($2);
	}
	;
server_infra_host_ttl: VAR_INFRA_HOST_TTL STRING
	{
		OUTYY(("P(server_infra_host_ttl:%s)\n", $2));
		if(atoi($2) == 0 && strcmp($2, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi($2);
		free($2);
	}
	;
server_infra_lame_ttl: VAR_INFRA_LAME_TTL STRING
	{
		OUTYY(("P(server_infra_lame_ttl:%s)\n", $2));
		if(atoi($2) == 0 && strcmp($2, "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->lame_ttl = atoi($2);
		free($2);
	}
	;
server_infra_cache_numhosts: VAR_INFRA_CACHE_NUMHOSTS STRING
	{
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi($2);
		free($2);
	}
	;
server_infra_cache_numlame: VAR_INFRA_CACHE_NUMLAME STRING
	{
		OUTYY(("P(server_infra_cache_numlame:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numlame = atoi($2);
		free($2);
	}
	;
server_infra_cache_slabs: VAR_INFRA_CACHE_SLABS STRING
	{
		OUTYY(("P(server_infra_cache_slabs:%s)\n", $2));
		if(atoi($2) == 0)
			yyerror("number expected");
		else {
			cfg_parser->cfg->infra_cache_slabs = atoi($2);
			if(!is_pow2(cfg_parser->cfg->infra_cache_slabs))
				yyerror("must be a power of 2");
		}
		free($2);
	}
	;
server_target_fetch_policy: VAR_TARGET_FETCH_POLICY STRING
	{
		OUTYY(("P(server_target_fetch_policy:%s)\n", $2));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = $2;
	}
	;
server_harden_short_bufsize: VAR_HARDEN_SHORT_BUFSIZE STRING
	{
		OUTYY(("P(server_harden_short_bufsize:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize = 
			(strcmp($2, "yes")==0);
		free($2);
	}
	;
server_harden_large_queries: VAR_HARDEN_LARGE_QUERIES STRING
	{
		OUTYY(("P(server_harden_large_queries:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries = 
			(strcmp($2, "yes")==0);
		free($2);
	}
	;
server_harden_glue: VAR_HARDEN_GLUE STRING
	{
		OUTYY(("P(server_harden_glue:%s)\n", $2));
		if(strcmp($2, "yes") != 0 && strcmp($2, "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue = 
			(strcmp($2, "yes")==0);
		free($2);
	}
	;
server_do_not_query_address: VAR_DO_NOT_QUERY_ADDRESS STRING
	{
		OUTYY(("P(server_do_not_query_address:%s)\n", $2));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, $2))
			yyerror("out of memory");
	}
	;
server_module_conf: VAR_MODULE_CONF STRING
	{
		OUTYY(("P(server_module_conf:%s)\n", $2));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = $2;
	}
	;
stub_name: VAR_NAME STRING
	{
		OUTYY(("P(name:%s)\n", $2));
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = $2;
	}
	;
stub_host: VAR_STUB_HOST STRING
	{
		OUTYY(("P(stub-host:%s)\n", $2));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, $2))
			yyerror("out of memory");
	}
	;
stub_addr: VAR_STUB_ADDR STRING
	{
		OUTYY(("P(stub-addr:%s)\n", $2));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, $2))
			yyerror("out of memory");
	}
	;
forward_name: VAR_NAME STRING
	{
		OUTYY(("P(name:%s)\n", $2));
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = $2;
	}
	;
forward_host: VAR_FORWARD_HOST STRING
	{
		OUTYY(("P(forward-host:%s)\n", $2));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, $2))
			yyerror("out of memory");
	}
	;
forward_addr: VAR_FORWARD_ADDR STRING
	{
		OUTYY(("P(forward-addr:%s)\n", $2));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, $2))
			yyerror("out of memory");
	}
	;
%%

/* parse helper routines could be here */
