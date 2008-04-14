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
#include "services/localzone.h"
#include <pwd.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

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

/** check localzones */
static void
localzonechecks(struct config_file* cfg)
{
	struct local_zones* zs;
	if(!(zs = local_zones_create()))
		fatal_exit("out of memory");
	if(!local_zones_apply_cfg(zs, cfg))
		fatal_exit("failed local-zone, local-data configuration");
	local_zones_delete(zs);
}

/** emit warnings for IP in hosts */
static void
warn_hosts(const char* typ, struct config_stub* list)
{
	struct sockaddr_storage a;
	socklen_t alen;
	struct config_stub* s;
	struct config_strlist* h;
	for(s=list; s; s=s->next) {
		for(h=s->hosts; h; h=h->next) {
			if(extstrtoaddr(h->str, &a, &alen)) {
				fprintf(stderr, "unbound-checkconf: warning:"
				  " %s %s: \"%s\" is an IP%s address, "
				  "and when looked up as a host name "
				  "during use may not resolve.\n", 
				  s->name, typ, h->str,
				  addr_is_ip6(&a, alen)?"6":"4");
			}
		}
	}
}

/** check interface strings */
static void
interfacechecks(struct config_file* cfg)
{
	struct sockaddr_storage a;
	socklen_t alen;
	int i, j;
	for(i=0; i<cfg->num_ifs; i++) {
		if(!ipstrtoaddr(cfg->ifs[i], UNBOUND_DNS_PORT, &a, &alen)) {
			fatal_exit("cannot parse interface specified as '%s'",
				cfg->ifs[i]);
		}
		for(j=0; j<cfg->num_ifs; j++) {
			if(i!=j && strcmp(cfg->ifs[i], cfg->ifs[j])==0)
				fatal_exit("interface: %s present twice, "
					"cannot bind same ports twice.",
					cfg->ifs[i]);
		}
	}
	for(i=0; i<cfg->num_out_ifs; i++) {
		if(!ipstrtoaddr(cfg->out_ifs[i], UNBOUND_DNS_PORT, 
			&a, &alen)) {
			fatal_exit("cannot parse outgoing-interface "
				"specified as '%s'", cfg->out_ifs[i]);
		}
		for(j=0; j<cfg->num_out_ifs; j++) {
			if(i!=j && strcmp(cfg->out_ifs[i], cfg->out_ifs[j])==0)
				fatal_exit("outgoing-interface: %s present "
					"twice, cannot bind same ports twice.",
					cfg->out_ifs[i]);
		}
	}
}

/** check acl ips */
static void
aclchecks(struct config_file* cfg)
{
	int d;
	struct sockaddr_storage a;
	socklen_t alen;
	struct config_str2list* acl;
	for(acl=cfg->acls; acl; acl = acl->next) {
		if(!netblockstrtoaddr(acl->str, UNBOUND_DNS_PORT, &a, &alen, 
			&d)) {
			fatal_exit("cannot parse access control address %s %s",
				acl->str, acl->str2);
		}
	}
}

/** true if fname is a file */
static int
is_file(const char* fname) 
{
	struct stat buf;
	if(stat(fname, &buf) < 0) {
		if(errno==EACCES) {
			printf("warning: no search permission for one of the directories in path: %s\n", fname);
			return 1;
		}
		perror(fname);
		return 0;
	}
	if(S_ISDIR(buf.st_mode)) {
		printf("%s is not a file\n", fname);
		return 0;
	}
	return 1;
}

/** true if fname is a directory */
static int
is_dir(const char* fname) 
{
	struct stat buf;
	if(stat(fname, &buf) < 0) {
		if(errno==EACCES) {
			printf("warning: no search permission for one of the directories in path: %s\n", fname);
			return 1;
		}
		perror(fname);
		return 0;
	}
	if(!(S_ISDIR(buf.st_mode))) {
		printf("%s is not a directory\n", fname);
		return 0;
	}
	return 1;
}

/** convert a filename to full pathname in original filesys
 * @param fname: the path name to convert.
 * 	Must not be null or empty.
 * @param cfg: config struct for chroot and chdir (if set).
 * @param use_chdir: if false, only chroot is applied.
 * @return pointer to static buffer which is: [chroot][chdir]fname
 */
static char*
fname_after_chroot(const char* fname, struct config_file* cfg, int use_chdir)
{
	static char buf[1024];
	int slashit = 0;
	buf[0] = 0;
	if(cfg->chrootdir && cfg->chrootdir[0] && 
		strncmp(cfg->chrootdir, fname, strlen(cfg->chrootdir)) == 0) {
		/* already full pathname, return it */
		strncpy(buf, fname, sizeof(buf)-1);
		buf[sizeof(buf)-1] = 0;
		return buf;
	}
	/* chroot */
	if(cfg->chrootdir && cfg->chrootdir[0]) {
		/* start with chrootdir */
		strncpy(buf, cfg->chrootdir, sizeof(buf)-1);
		slashit = 1;
	}
	/* chdir */
	if(fname[0] == '/' || !use_chdir) {
		/* full path, no chdir */
	} else if(cfg->directory && cfg->directory[0]) {
		/* prepend chdir */
		if(slashit && cfg->directory[0] != '/')
			strncat(buf, "/", sizeof(buf)-1);
		if(strncmp(cfg->chrootdir, cfg->directory, 
			strlen(cfg->chrootdir)) == 0)
			strncat(buf, cfg->directory+strlen(cfg->chrootdir), 
				sizeof(buf)-1);
		else strncat(buf, cfg->directory, sizeof(buf)-1);
		slashit = 1;
	}
	/* fname */
	if(slashit && fname[0] != '/')
		strncat(buf, "/", sizeof(buf)-1);
	strncat(buf, fname, sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0;
	return buf;
}

/** get base dir of a fname */
static char*
basedir(const char* fname, struct config_file* cfg)
{
	char* d = fname_after_chroot(fname, cfg, 1);
	char* rev = strrchr(d, '/');
	if(!rev) return NULL;
	if(d == rev) return NULL;
	rev[0] = 0;
	return d;
}

/** check file list, every file must be inside the chroot location */
static void
check_chroot_filelist(const char* desc, struct config_strlist* list,
	const char* chrootdir, struct config_file* cfg)
{
	struct config_strlist* p;
	char* old;
	for(p=list; p; p=p->next) {
		if(p->str && p->str[0]) {
			if(!is_file(fname_after_chroot(p->str, cfg, 1))) {
				fatal_exit("%s: \"%s\" does not exist in chrootdir %s", 
					desc, p->str, chrootdir);
			}
			old = p->str;
			/* put in a new full path for continued checking */
			p->str = strdup(fname_after_chroot(p->str, cfg, 1));
			free(old);
		}
	}
}

/** check configuration for errors */
static void
morechecks(struct config_file* cfg)
{
	warn_hosts("stub-host", cfg->stubs);
	warn_hosts("forward-host", cfg->forwards);
	interfacechecks(cfg);
	aclchecks(cfg);

	if(cfg->verbosity < 0)
		fatal_exit("verbosity value < 0");
	if(cfg->num_threads < 0 || cfg->num_threads > 10000)
		fatal_exit("num_threads value weird");
	if(!cfg->do_ip4 && !cfg->do_ip6)
		fatal_exit("ip4 and ip6 are both disabled, pointless");
	if(!cfg->do_udp && !cfg->do_tcp)
		fatal_exit("udp and tcp are both disabled, pointless");

	if(cfg->chrootdir && cfg->chrootdir[0] && 
		cfg->chrootdir[strlen(cfg->chrootdir)-1] == '/')
		fatal_exit("chootdir %s has trailing slash '/' please remove.",
			cfg->chrootdir);
	if(cfg->chrootdir && cfg->chrootdir[0] && 
		!is_dir(cfg->chrootdir)) {
		fatal_exit("bad chroot directory");
	}
	if(cfg->directory && cfg->directory[0] && 
		!is_dir(fname_after_chroot(cfg->directory, cfg, 0))) {
		fatal_exit("bad chdir directory");
	}
	if( (cfg->chrootdir && cfg->chrootdir[0]) ||
	    (cfg->directory && cfg->directory[0])) {
		if(cfg->pidfile && cfg->pidfile[0] &&
		   basedir(cfg->pidfile, cfg) &&
		   !is_dir(basedir(cfg->pidfile, cfg))) {
			fatal_exit("pidfile directory does not exist");
		}
		if(cfg->logfile && cfg->logfile[0] &&
		   basedir(cfg->logfile, cfg) &&
		   !is_dir(basedir(cfg->logfile, cfg))) {
			fatal_exit("pidfile directory does not exist");
		}
	}

	check_chroot_filelist("file with root-hints", 
		cfg->root_hints, cfg->chrootdir, cfg);
	check_chroot_filelist("trust-anchor-file", 
		cfg->trust_anchor_file_list, cfg->chrootdir, cfg);
	check_chroot_filelist("trusted-keys-file", 
		cfg->trusted_keys_file_list, cfg->chrootdir, cfg);
	/* remove chroot setting so that modules are not stripping pathnames*/
	free(cfg->chrootdir);
	cfg->chrootdir = NULL;
	
	if(strcmp(cfg->module_conf, "iterator") != 0 &&
		strcmp(cfg->module_conf, "validator iterator") != 0) {
		fatal_exit("module conf '%s' is not known to work",
			cfg->module_conf);
	}

	if(cfg->username && cfg->username[0]) {
		if(getpwnam(cfg->username) == NULL)
			fatal_exit("user '%s' does not exist.", cfg->username);
		endpwent();
	}

	localzonechecks(cfg);
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
