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
#include "util/net_help.h"
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
	cfg->stat_interval = 0;
	cfg->num_threads = 1;
	cfg->port = UNBOUND_DNS_PORT;
	cfg->do_ip4 = 1;
	cfg->do_ip6 = 1;
	cfg->do_udp = 1;
	cfg->do_tcp = 1;
	cfg->use_syslog = 1;
	cfg->outgoing_base_port = cfg->port + 2000;
	cfg->outgoing_num_ports = 16;
	cfg->outgoing_num_tcp = 10;
	cfg->incoming_num_tcp = 10;
	cfg->msg_buffer_size = 65552; /* 64 k + a small margin */
	cfg->msg_cache_size = 4 * 1024 * 1024;
	cfg->msg_cache_slabs = 4;
	cfg->num_queries_per_thread = 1024;
	cfg->rrset_cache_size = 4 * 1024 * 1024;
	cfg->rrset_cache_slabs = 4;
	cfg->host_ttl = 900;
	cfg->lame_ttl = 900;
	cfg->bogus_ttl = 900;
	cfg->max_ttl = 3600 * 24;
	cfg->infra_cache_slabs = 4;
	cfg->infra_cache_numhosts = 10000;
	cfg->infra_cache_lame_size = 10240; /* easily 40 or more entries */
	if(!(cfg->username = strdup("unbound"))) goto error_exit;
	if(!(cfg->chrootdir = strdup("/etc/unbound"))) goto error_exit;
	if(!(cfg->directory = strdup("/etc/unbound"))) goto error_exit;
	if(!(cfg->logfile = strdup(""))) goto error_exit;
	if(!(cfg->pidfile = strdup("/etc/unbound/unbound.pid"))) 
		goto error_exit;
	if(!(cfg->target_fetch_policy = strdup("3 2 1 0 0"))) goto error_exit;
	cfg->donotqueryaddrs = NULL;
	cfg->donotquery_localhost = 1;
	cfg->root_hints = NULL;
	cfg->do_daemonize = 1;
	cfg->if_automatic = 0;
	cfg->num_ifs = 0;
	cfg->ifs = NULL;
	cfg->num_out_ifs = 0;
	cfg->out_ifs = NULL;
	cfg->stubs = NULL;
	cfg->forwards = NULL;
	cfg->acls = NULL;
	cfg->harden_short_bufsize = 0;
	cfg->harden_large_queries = 0;
	cfg->harden_glue = 1;
	cfg->harden_dnssec_stripped = 1;
	cfg->hide_identity = 0;
	cfg->hide_version = 0;
	cfg->identity = NULL;
	cfg->version = NULL;
	cfg->trust_anchor_file_list = NULL;
	cfg->trust_anchor_list = NULL;
	cfg->trusted_keys_file_list = NULL;
	cfg->val_date_override = 0;
	cfg->val_clean_additional = 1;
	cfg->val_permissive_mode = 0;
	cfg->key_cache_size = 4 * 1024 * 1024;
	cfg->key_cache_slabs = 4;
	cfg->local_zones = NULL;
	cfg->local_zones_nodefault = NULL;
	cfg->local_data = NULL;
	cfg->donotquery_localhost = 0; /* allow, so that you can ask a
		forward nameserver running on localhost */
	if(!(cfg->module_conf = strdup("validator iterator"))) goto error_exit;
	if(!(cfg->val_nsec3_key_iterations = 
		strdup("1024 150 2048 500 4096 2500"))) goto error_exit;
	return cfg;
error_exit:
	config_delete(cfg); 
	return NULL;
}

struct config_file* config_create_forlib()
{
	struct config_file* cfg = config_create();
	if(!cfg) return NULL;
	/* modifications for library use, less verbose, less memory */
	free(cfg->chrootdir);
	cfg->chrootdir = NULL;
	cfg->verbosity = 0;
	cfg->outgoing_num_tcp = 2;
	cfg->msg_cache_size = 1024*1024;
	cfg->msg_cache_slabs = 1;
	cfg->rrset_cache_size = 1024*1024;
	cfg->rrset_cache_slabs = 1;
	cfg->infra_cache_slabs = 1;
	cfg->use_syslog = 0;
	cfg->key_cache_size = 1024*1024;
	cfg->key_cache_slabs = 1;
	return cfg;
}

/** check that the value passed is >= 0 */
#define IS_NUMBER_OR_ZERO \
	if(atoi(val) == 0 && strcmp(val, "0") != 0) return 0
/** check that the value passed is > 0 */
#define IS_NONZERO_NUMBER \
	if(atoi(val) == 0) return 0
/** check that the value passed is not 0 and a power of 2 */
#define IS_POW2_NUMBER \
	if(atoi(val) == 0 || !is_pow2((size_t)atoi(val))) return 0
/** check that the value passed is yes or no */
#define IS_YES_OR_NO \
	if(strcmp(val, "yes") != 0 && strcmp(val, "no") != 0) return 0

int config_set_option(struct config_file* cfg, const char* opt,
        const char* val)
{
	if(strcmp(opt, "verbosity:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->verbosity = atoi(val);
	} else if(strcmp(opt, "statistics-interval:") == 0) {
		if(strcmp(val, "0") == 0 || strcmp(val, "") == 0)
			cfg->stat_interval = 0;
		else if(atoi(val) == 0)
			return 0;
		else cfg->stat_interval = atoi(val);
	} else if(strcmp(opt, "num_threads:") == 0) {
		/* not supported, library must have 1 thread in bgworker */
		return 0;
	} else if(strcmp(opt, "do-ip4:") == 0) {
		IS_YES_OR_NO;
		cfg->do_ip4 = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "do-ip6:") == 0) {
		IS_YES_OR_NO;
		cfg->do_ip6 = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "do-udp:") == 0) {
		IS_YES_OR_NO;
		cfg->do_udp = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "do-tcp:") == 0) {
		IS_YES_OR_NO;
		cfg->do_tcp = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "outgoing-port:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->outgoing_base_port = atoi(val);
	} else if(strcmp(opt, "outgoing-range:") == 0) {
		IS_NONZERO_NUMBER;
		cfg->outgoing_num_ports = atoi(val);
	} else if(strcmp(opt, "outgoing-num-tcp:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->outgoing_num_tcp = (size_t)atoi(val);
	} else if(strcmp(opt, "incoming-num-tcp:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->incoming_num_tcp = (size_t)atoi(val);
	} else if(strcmp(opt, "msg-buffer-size:") == 0) {
		IS_NONZERO_NUMBER;
		cfg->msg_buffer_size = (size_t)atoi(val);
	} else if(strcmp(opt, "msg-cache-size:") == 0) {
		return cfg_parse_memsize(val, &cfg->msg_cache_size);
	} else if(strcmp(opt, "msg-cache-slabs:") == 0) {
		IS_POW2_NUMBER;
		cfg->msg_cache_slabs = (size_t)atoi(val);
	} else if(strcmp(opt, "num-queries-per-thread:") == 0) {
		IS_NONZERO_NUMBER;
		cfg->num_queries_per_thread = (size_t)atoi(val);
	} else if(strcmp(opt, "rrset-cache-size:") == 0) {
		return cfg_parse_memsize(val, &cfg->rrset_cache_size);
	} else if(strcmp(opt, "rrset-cache-slabs:") == 0) {
		IS_POW2_NUMBER;
		cfg->rrset_cache_slabs = (size_t)atoi(val);
	} else if(strcmp(opt, "cache-max-ttl:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->max_ttl = atoi(val);
	} else if(strcmp(opt, "infra-host-ttl:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->host_ttl = atoi(val);
	} else if(strcmp(opt, "infra-lame-ttl:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->lame_ttl = atoi(val);
	} else if(strcmp(opt, "infra-cache-slabs:") == 0) {
		IS_POW2_NUMBER;
		cfg->infra_cache_slabs = (size_t)atoi(val);
	} else if(strcmp(opt, "infra-cache-numhosts:") == 0) {
		IS_NONZERO_NUMBER;
		cfg->infra_cache_numhosts = (size_t)atoi(val);
	} else if(strcmp(opt, "infra-cache-lame-size:") == 0) {
		return cfg_parse_memsize(val, &cfg->infra_cache_lame_size);
	} else if(strcmp(opt, "logfile:") == 0) {
		cfg->use_syslog = 0;
		free(cfg->logfile);
		return (cfg->logfile = strdup(val)) != NULL;
	} else if(strcmp(opt, "use-syslog:") == 0) {
		IS_YES_OR_NO;
		cfg->use_syslog = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "root-hints:") == 0) {
		return cfg_strlist_insert(&cfg->root_hints, strdup(val));
	} else if(strcmp(opt, "target-fetch-policy:") == 0) {
		free(cfg->target_fetch_policy);
		return (cfg->target_fetch_policy = strdup(val)) != NULL;
	} else if(strcmp(opt, "harden-glue:") == 0) {
		IS_YES_OR_NO;
		cfg->harden_glue = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "harden-short-bufsize:") == 0) {
		IS_YES_OR_NO;
		cfg->harden_short_bufsize = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "harden-large-queries:") == 0) {
		IS_YES_OR_NO;
		cfg->harden_large_queries = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "harden-dnssec-stripped:") == 0) {
		IS_YES_OR_NO;
		cfg->harden_dnssec_stripped = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "do-not-query-localhost:") == 0) {
		IS_YES_OR_NO;
		cfg->donotquery_localhost = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "do-not-query-address:") == 0) {
		return cfg_strlist_insert(&cfg->donotqueryaddrs, strdup(val));
	} else if(strcmp(opt, "trust-anchor-file:") == 0) {
		return cfg_strlist_insert(&cfg->trust_anchor_file_list, 
			strdup(val));
	} else if(strcmp(opt, "trust-anchor:") == 0) {
		return cfg_strlist_insert(&cfg->trust_anchor_list, 
			strdup(val));
	} else if(strcmp(opt, "trusted-keys-file:") == 0) {
		return cfg_strlist_insert(&cfg->trusted_keys_file_list, 
			strdup(val));
	} else if(strcmp(opt, "val-override-date:") == 0) {
		if(strcmp(val, "") == 0 || strcmp(val, "0") == 0) {
			cfg->val_date_override = 0;
		} else if(strlen(val) == 14) {
			cfg->val_date_override = cfg_convert_timeval(val);
			return cfg->val_date_override != 0;
		} else {
			if(atoi(val) == 0) return 0;
			cfg->val_date_override = (uint32_t)atoi(val);
		}
	} else if(strcmp(opt, "val-bogus-ttl:") == 0) {
		IS_NUMBER_OR_ZERO;
		cfg->bogus_ttl = atoi(val);
	} else if(strcmp(opt, "val-clean-additional:") == 0) {
		IS_YES_OR_NO;
		cfg->val_clean_additional = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "val-permissive-mode:") == 0) {
		IS_YES_OR_NO;
		cfg->val_permissive_mode = (strcmp(val, "yes") == 0);
	} else if(strcmp(opt, "val-nsec3-keysize-iterations:") == 0) {
		free(cfg->val_nsec3_key_iterations);
		return (cfg->val_nsec3_key_iterations = strdup(val)) != NULL;
	} else if(strcmp(opt, "key-cache-size:") == 0) {
		return cfg_parse_memsize(val, &cfg->key_cache_size);
	} else if(strcmp(opt, "key-cache-slabs:") == 0) {
		IS_POW2_NUMBER;
		cfg->key_cache_slabs = (size_t)atoi(val);
	} else if(strcmp(opt, "local-data:") == 0) {
		return cfg_strlist_insert(&cfg->local_data, strdup(val));
	} else if(strcmp(opt, "module-config:") == 0) {
		free(cfg->module_conf);
		return (cfg->module_conf = strdup(val)) != NULL;
	} else {
		/* unknown or unsupported (from the library interface) */
		return 0;
	}
	return 1;
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
}

int 
config_read(struct config_file* cfg, char* filename)
{
	FILE *in;
	if(!filename)
		return 1;
	in = fopen(filename, "r");
	if(!in) {
		log_err("Could not open %s: %s", filename, strerror(errno));
		return 0;
	}
	create_cfg_parser(cfg, filename);
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

void
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
config_deldblstrlist(struct config_str2list* p)
{
	struct config_str2list *np;
	while(p) {
		np = p->next;
		free(p->str);
		free(p->str2);
		free(p);
		p = np;
	}
}

void
config_delstubs(struct config_stub* p)
{
	struct config_stub* np;
	while(p) {
		np = p->next;
		free(p->name);
		config_delstrlist(p->hosts);
		config_delstrlist(p->addrs);
		free(p);
		p = np;
	}
}

void 
config_delete(struct config_file* cfg)
{
	if(!cfg) return;
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
	if(cfg->out_ifs) {
		int i;
		for(i=0; i<cfg->num_out_ifs; i++)
			free(cfg->out_ifs[i]);
		free(cfg->out_ifs);
	}
	config_delstubs(cfg->stubs);
	config_delstubs(cfg->forwards);
	config_delstrlist(cfg->donotqueryaddrs);
	config_delstrlist(cfg->root_hints);
	free(cfg->identity);
	free(cfg->version);
	free(cfg->module_conf);
	config_delstrlist(cfg->trust_anchor_file_list);
	config_delstrlist(cfg->trusted_keys_file_list);
	config_delstrlist(cfg->trust_anchor_list);
	config_deldblstrlist(cfg->acls);
	free(cfg->val_nsec3_key_iterations);
	config_deldblstrlist(cfg->local_zones);
	config_delstrlist(cfg->local_zones_nodefault);
	config_delstrlist(cfg->local_data);
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

int 
cfg_strlist_insert(struct config_strlist** head, char* item)
{
	struct config_strlist *s;
	if(!item || !head)
		return 0;
	s = (struct config_strlist*)calloc(1, sizeof(struct config_strlist));
	if(!s)
		return 0;
	s->str = item;
	s->next = *head;
	*head = s;
	return 1;
}

int 
cfg_str2list_insert(struct config_str2list** head, char* item, char* i2)
{
	struct config_str2list *s;
	if(!item || !i2 || !head)
		return 0;
	s = (struct config_str2list*)calloc(1, sizeof(struct config_str2list));
	if(!s)
		return 0;
	s->str = item;
	s->str2 = i2;
	s->next = *head;
	*head = s;
	return 1;
}

uint32_t 
cfg_convert_timeval(const char* str)
{
	uint32_t t;
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	if(strlen(str) < 14)
		return 0;
	if(sscanf(str, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, 
		&tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6)
		return 0;
	tm.tm_year -= 1900;
	tm.tm_mon--;
	/* Check values */
	if (tm.tm_year < 70)	return 0;
	if (tm.tm_mon < 0 || tm.tm_mon > 11)	return 0;
	if (tm.tm_mday < 1 || tm.tm_mday > 31) 	return 0;
	if (tm.tm_hour < 0 || tm.tm_hour > 23)	return 0;
	if (tm.tm_min < 0 || tm.tm_min > 59)	return 0;
	if (tm.tm_sec < 0 || tm.tm_sec > 59)	return 0;
	/* call ldns conversion function */
	t = mktime_from_utc(&tm);
	return t;
}

int 
cfg_count_numbers(const char* s)
{
        /* format ::= (sp num)+ sp      */
        /* num ::= [-](0-9)+            */
        /* sp ::= (space|tab)*          */
        int num = 0;
        while(*s) {
                while(*s && isspace((int)*s))
                        s++;
                if(!*s) /* end of string */
                        break;
                if(*s == '-')
                        s++;
                if(!*s) /* only - not allowed */
                        return 0;
                if(!isdigit((int)*s)) /* bad character */
                        return 0;
                while(*s && isdigit((int)*s))
                        s++;
                num++;
        }
        return num;
}

/** all digit number */
static int isalldigit(const char* str, size_t l)
{
	size_t i;
	for(i=0; i<l; i++)
		if(!isdigit(str[i]))
			return 0;
	return 1;
}

int 
cfg_parse_memsize(const char* str, size_t* res)
{
	size_t len = (size_t)strlen(str);
	size_t mult = 1;
	if(!str || len == 0) {
		log_err("not a size: '%s'", str);
		return 0;
	}
	if(isalldigit(str, len)) {
		*res = (size_t)atol(str);
		return 1;
	}
	/* check appended num */
	while(len>0 && str[len-1]==' ')
		len--;
	if(len > 1 && str[len-1] == 'b') 
		len--;
	else if(len > 1 && str[len-1] == 'B') 
		len--;
	
	if(len > 1 && tolower(str[len-1]) == 'g')
		mult = 1024*1024*1024;
	else if(len > 1 && tolower(str[len-1]) == 'm')
		mult = 1024*1024;
	else if(len > 1 && tolower(str[len-1]) == 'k')
		mult = 1024;
	else if(len > 0 && isdigit(str[len-1]))
		mult = 1;
	else {
		log_err("unknown size specifier: '%s'", str);
		return 0;
	}
	while(len>1 && str[len-2]==' ')
		len--;

	if(!isalldigit(str, len-1)) {
		log_err("unknown size specifier: '%s'", str);
		return 0;
	}
	*res = ((size_t)atol(str)) * mult;
	return 1;
}

/** the MAX_TTL global */
extern uint32_t MAX_TTL;

void 
config_apply(struct config_file* config)
{
	MAX_TTL = (uint32_t)config->max_ttl;
}

