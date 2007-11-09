/*
 * checkconf/unbound-checkconf.c - config file checker for unbound.conf file.
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
 * The config checker checks for syntax and other errors in the unbound.conf
 * file, and can be used to check for errors before the server is started
 * or sigHUPped.
 * Exit status 1 means an error.
 */

#include "config.h"
#include "util/log.h"
#include "util/config_file.h"
#include "util/module.h"
#include "util/net_help.h"
#include "util/regional.h"
#include "iterator/iterator.h"
#include "validator/validator.h"
#include <pwd.h>

/** Give checkconf usage, and exit (1). */
static void
usage()
{
	printf("Usage:	unbound-checkconf [file]\n");
	printf("	Checks unbound configuration file for errors.\n");
	printf("file	if omitted %s is used.\n", CONFIGFILE);
	printf("-h	show this usage help.\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
	exit(1);
}

/** check if module works with config */
static void
check_mod(struct config_file* cfg, struct module_func_block* fb)
{
	struct module_env env;
	memset(&env, 0, sizeof(env));
	env.cfg = cfg;
	env.scratch = regional_create();
	env.scratch_buffer = ldns_buffer_new(BUFSIZ);
	if(!env.scratch || !env.scratch_buffer)
		fatal_exit("out of memory");
	if(!(*fb->init)(&env, 0)) {
		fatal_exit("bad config for %s module", fb->name);
	}
	(*fb->deinit)(&env, 0);
	ldns_buffer_free(env.scratch_buffer);
	regional_destroy(env.scratch);
}

/** check configuration for errors */
static void
morechecks(struct config_file* cfg)
{
	int i;
	struct sockaddr_storage a;
	socklen_t alen;
	for(i=0; i<cfg->num_ifs; i++) {
		if(!ipstrtoaddr(cfg->ifs[i], UNBOUND_DNS_PORT, &a, &alen)) {
			fatal_exit("cannot parse interface specified as '%s'",
				cfg->ifs[i]);
		}
	}
	for(i=0; i<cfg->num_out_ifs; i++) {
		if(!ipstrtoaddr(cfg->out_ifs[i], UNBOUND_DNS_PORT, 
			&a, &alen)) {
			fatal_exit("cannot parse outgoing-interface "
				"specified as '%s'", cfg->out_ifs[i]);
		}
	}

	if(cfg->verbosity < 0)
		fatal_exit("verbosity value < 0");
	if(cfg->num_threads < 0 || cfg->num_threads > 10000)
		fatal_exit("num_threads value weird");
	if(!cfg->do_ip4 && !cfg->do_ip6)
		fatal_exit("ip4 and ip6 are both disabled, pointless");
	if(!cfg->do_udp && !cfg->do_tcp)
		fatal_exit("udp and tcp are both disabled, pointless");

	if(cfg->chrootdir && strncmp(cfg->chrootdir, cfg->directory,
		strlen(cfg->chrootdir)) != 0)
		fatal_exit("working directory %s not in chrootdir %s",
			cfg->directory, cfg->chrootdir);
	if(cfg->chrootdir && cfg->pidfile && cfg->pidfile[0] &&
		strncmp(cfg->chrootdir, cfg->pidfile,
			strlen(cfg->chrootdir)) != 0)
		fatal_exit("pid file %s not in chrootdir %s",
			cfg->pidfile, cfg->chrootdir);
	if(cfg->chrootdir && cfg->logfile && cfg->logfile[0] &&
		strncmp(cfg->chrootdir, cfg->logfile,
			strlen(cfg->chrootdir)) != 0)
		fatal_exit("log file %s not in chrootdir %s",
			cfg->logfile, cfg->chrootdir);
	
	if(strcmp(cfg->module_conf, "iterator") != 0 &&
		strcmp(cfg->module_conf, "validator iterator") != 0) {
		fatal_exit("module conf %s is not known to work",
			cfg->module_conf);
	}

	if(cfg->username && cfg->username[0]) {
		struct passwd *pwd;
		if((pwd = getpwnam(cfg->username)) == NULL)
			fatal_exit("user '%s' does not exist.", cfg->username);
		endpwent();
	}
}

/** check config file */
static void
checkconf(char* cfgfile)
{
	struct config_file* cfg = config_create();
	if(!cfg)
		fatal_exit("out of memory");
	if(!config_read(cfg, cfgfile)) {
		/* config_read prints messages to stderr */
		config_delete(cfg);
		exit(1);
	}
	morechecks(cfg);
	check_mod(cfg, iter_get_funcblock());
	check_mod(cfg, val_get_funcblock());
	config_delete(cfg);
	printf("unbound-checkconf: no errors in %s\n", cfgfile);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** Main routine for checkconf */
int main(int argc, char* argv[])
{
	int c;
	char* f;
	log_ident_set("unbound-checkconf");
	log_init(NULL, 0, NULL);
	checklock_start();
	/* parse the options */
	while( (c=getopt(argc, argv, "h")) != -1) {
		switch(c) {
		case '?':
		case 'h':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if(argc != 0 && argc != 1)
		usage();
	if(argc == 1)
		f = argv[0];
	else	f = CONFIGFILE;
	checkconf(f);
	checklock_stop();
	return 0;
}
