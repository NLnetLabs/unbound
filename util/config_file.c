/*
 * util/config_file.c - reads and stores the config file for unbound.
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
 * This file contains functions for the config file.
 */

#include "config.h"
#include "util/log.h"

#include "util/configyyrename.h"
#include "util/config_file.h"
#include "util/configparser.h"
/** global config during parsing */
struct config_parser_state* cfg_parser = 0;
/** lex in file */
extern FILE* ub_c_in;
/** lex out file */
extern FILE* ub_c_out;
/** the yacc lex generated parse function */
int ub_c_parse(void);
/** the lexer function */
int ub_c_lex(void);
/** wrap function */
int ub_c_wrap(void);
/** print error with file and line number */
void ub_c_error(const char *message);

struct config_file* 
config_create()
{
	struct config_file* cfg;
	cfg = (struct config_file*)calloc(1, sizeof(struct config_file));
	if(!cfg)
		return NULL;
	/* the defaults if no config is present */
	cfg->verbosity = 1;
	cfg->num_threads = 1;
	cfg->port = UNBOUND_DNS_PORT;
	cfg->do_ip4 = 1;
	cfg->do_ip6 = 1;
	cfg->do_udp = 1;
	cfg->do_tcp = 1;
	cfg->outgoing_base_port = cfg->port + 1000;
	cfg->outgoing_num_ports = 16;
	cfg->outgoing_num_tcp = 10;
	cfg->msg_cache_size = 4 * 1024 * 1024;
	cfg->msg_cache_slabs = 4;
	cfg->num_queries_per_thread = 1024;
	cfg->rrset_cache_size = 4 * 1024 * 1024;
	cfg->rrset_cache_slabs = 4;
	cfg->host_ttl = 900;
	cfg->lame_ttl = 900;
	cfg->infra_cache_slabs = 4;
	cfg->infra_cache_numhosts = 1000;
	cfg->infra_cache_numlame = 1000;
	if(!(cfg->fwd_address = strdup(""))) goto error_exit;
	if(!(cfg->username = strdup(""))) goto error_exit;
	if(!(cfg->chrootdir = strdup(""))) goto error_exit;
	if(!(cfg->directory = strdup("/etc/unbound"))) goto error_exit;
	if(!(cfg->logfile = strdup(""))) goto error_exit;
	if(!(cfg->pidfile = strdup("unbound.pid"))) goto error_exit;
	if(!(cfg->target_fetch_policy = strdup("3 2 1 0 0"))) goto error_exit;
	cfg->fwd_port = UNBOUND_DNS_PORT;
	cfg->do_daemonize = 1;
	cfg->num_ifs = 0;
	cfg->ifs = NULL;
	cfg->stubs = NULL;
	return cfg;
error_exit:
	config_delete(cfg); 
	return NULL;
}

/** initialize the global cfg_parser object */
static void
create_cfg_parser(struct config_file* cfg, char* filename)
{
	static struct config_parser_state st;
	cfg_parser = &st;
	cfg_parser->filename = filename;
	cfg_parser->line = 1;
	cfg_parser->errors = 0;
	cfg_parser->cfg = cfg;
	cfg_parser->server_settings_seen = 0;
}

int 
config_read(struct config_file* cfg, const char* filename)
{
	FILE *in;
	if(!filename)
		return 1;
	in = fopen(filename, "r");
	if(!in) {
		log_err("Could not open %s: %s", filename, strerror(errno));
		return 0;
	}
	create_cfg_parser(cfg, (char*)filename);
	ub_c_in = in;
	ub_c_parse();
	fclose(in);

	if(cfg_parser->errors != 0) {
		fprintf(stderr, "read %s failed: %d errors in configuration file\n",
			cfg_parser->filename, cfg_parser->errors);
		return 0;
	}
	return 1;
}

/** delete config strlist */
static void
config_delstrlist(struct config_strlist* p)
{
	struct config_strlist *np;
	while(p) {
		np = p->next;
		free(p->str);
		free(p);
		p = np;
	}
}

void 
config_delete(struct config_file* cfg)
{
	if(!cfg) return;
	free(cfg->fwd_address);
	free(cfg->username);
	free(cfg->chrootdir);
	free(cfg->directory);
	free(cfg->logfile);
	free(cfg->pidfile);
	free(cfg->target_fetch_policy);
	if(cfg->ifs) {
		int i;
		for(i=0; i<cfg->num_ifs; i++)
			free(cfg->ifs[i]);
		free(cfg->ifs);
	}
	if(cfg->stubs) {
		struct config_stub* p = cfg->stubs, *np;
		while(p) {
			np = p->next;
			free(p->name);
			config_delstrlist(p->hosts);
			config_delstrlist(p->addrs);
			free(p);
			p = np;
		}
	}
	free(cfg);
}

/** print error with file and line number */
void ub_c_error_va_list(const char *fmt, va_list args)
{
	cfg_parser->errors++;
	fprintf(stderr, "%s:%d: error: ", cfg_parser->filename,
	cfg_parser->line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

/** print error with file and line number */
void ub_c_error_msg(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ub_c_error_va_list(fmt, args);
	va_end(args);
}

void ub_c_error(const char *str)
{
	cfg_parser->errors++;
	fprintf(stderr, "%s:%d: error: %s\n", cfg_parser->filename,
		cfg_parser->line, str);
}

int ub_c_wrap()
{
	return 1;
}

