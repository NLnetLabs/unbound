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
#include "iterator/iter_fwd.h"
#include "validator/validator.h"
#include "services/localzone.h"
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_GLOB_H
#include <glob.h>
#endif
#ifdef WITH_PYTHONMODULE
#include "pythonmod/pythonmod.h"
#endif

/** Give checkconf usage, and exit (1). */
static void
usage()
{
	printf("Usage:	unbound-checkconf [file]\n");
	printf("	Checks unbound configuration file for errors.\n");
	printf("file	if omitted %s is used.\n", CONFIGFILE);
	printf("-o option	print value of option to stdout.\n");
	printf("-h		show this usage help.\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
	exit(1);
}

/** compare and print decimal option */
#define O_DEC(opt, str, var) if(strcmp(opt, str)==0) \
	{printf("%d\n", (int)cfg->var);}
/** compare and print unsigned option */
#define O_UNS(opt, str, var) if(strcmp(opt, str)==0) \
	{printf("%u\n", (unsigned)cfg->var);}
/** compare and print yesno option */
#define O_YNO(opt, str, var) if(strcmp(opt, str)==0) \
	{printf("%s\n", cfg->var?"yes":"no");}
/** compare and print string option */
#define O_STR(opt, str, var) if(strcmp(opt, str)==0) \
	{printf("%s\n", cfg->var?cfg->var:"");}
/** compare and print array option */
#define O_IFC(opt, str, num, arr) if(strcmp(opt, str)==0) \
	{int i; for(i=0; i<cfg->num; i++) printf("%s\n", cfg->arr[i]);}
/** compare and print memorysize option */
#define O_MEM(opt, str, var) if(strcmp(opt, str)==0) { \
	if(cfg->var > 1024*1024*1024) {	\
	  size_t f=cfg->var/(size_t)1000000, b=cfg->var%(size_t)1000000; \
	  printf("%u%6.6u\n", (unsigned)f, (unsigned)b); \
	} else printf("%u\n", (unsigned)cfg->var);}
/** compare and print list option */
#define O_LST(opt, name, lst) if(strcmp(opt, name)==0) { \
	struct config_strlist* p = cfg->lst; \
	for(p = cfg->lst; p; p = p->next) \
		printf("%s\n", p->str); \
	}
/** compare and print list option */
#define O_LS2(opt, name, lst) if(strcmp(opt, name)==0) { \
	struct config_str2list* p = cfg->lst; \
	for(p = cfg->lst; p; p = p->next) \
		printf("%s %s\n", p->str, p->str2); \
	}

/** 
 * Print given option to stdout 
 * @param cfg: config
 * @param opt: option name without trailing :. 
 *	This is different from config_set_option.
 */
static void
print_option(struct config_file* cfg, const char* opt)
{
	O_DEC(opt, "verbosity", verbosity)
	else O_DEC(opt, "statistics-interval", stat_interval)
	else O_YNO(opt, "statistics-cumulative", stat_interval)
	else O_YNO(opt, "extended-statistics", stat_extended)
	else O_DEC(opt, "num-threads", num_threads)
	else O_IFC(opt, "interface", num_ifs, ifs)
	else O_IFC(opt, "outgoing-interface", num_out_ifs, out_ifs)
	else O_YNO(opt, "interface-automatic", if_automatic)
	else O_DEC(opt, "port", port)
	else O_DEC(opt, "outgoing-range", outgoing_num_ports)
	else O_DEC(opt, "outgoing-num-tcp", outgoing_num_tcp)
	else O_DEC(opt, "incoming-num-tcp", incoming_num_tcp)
	else O_DEC(opt, "edns-buffer-size", edns_buffer_size)
	else O_DEC(opt, "msg-buffer-size", msg_buffer_size)
	else O_MEM(opt, "msg-cache-size", msg_cache_size)
	else O_DEC(opt, "msg-cache-slabs", msg_cache_slabs)
	else O_DEC(opt, "num-queries-per-thread", num_queries_per_thread)
	else O_UNS(opt, "jostle-timeout", jostle_time)
	else O_MEM(opt, "so-rcvbuf", socket_rcvbuf)
	else O_MEM(opt, "rrset-cache-size", rrset_cache_size)
	else O_DEC(opt, "rrset-cache-slabs", rrset_cache_slabs)
	else O_YNO(opt, "prefetch", prefetch)
	else O_DEC(opt, "cache-max-ttl", max_ttl)
	else O_DEC(opt, "infra-host-ttl", host_ttl)
	else O_DEC(opt, "infra-lame-ttl", lame_ttl)
	else O_DEC(opt, "infra-cache-slabs", infra_cache_slabs)
	else O_MEM(opt, "infra-cache-numhosts", infra_cache_numhosts)
	else O_MEM(opt, "infra-cache-lame-size", infra_cache_lame_size)
	else O_YNO(opt, "do-ip4", do_ip4)
	else O_YNO(opt, "do-ip6", do_ip6)
	else O_YNO(opt, "do-udp", do_udp)
	else O_YNO(opt, "do-tcp", do_tcp)
	else O_YNO(opt, "do-daemonize", do_daemonize)
	else O_STR(opt, "chroot", chrootdir)
	else O_STR(opt, "username", username)
	else O_STR(opt, "directory", directory)
	else O_STR(opt, "logfile", logfile)
	else O_STR(opt, "pidfile", pidfile)
	else O_YNO(opt, "hide-identity", hide_identity)
	else O_YNO(opt, "hide-version", hide_version)
	else O_STR(opt, "identity", identity)
	else O_STR(opt, "version", version)
	else O_STR(opt, "target-fetch-policy", target_fetch_policy)
	else O_YNO(opt, "harden-short-bufsize", harden_short_bufsize)
	else O_YNO(opt, "harden-large-queries", harden_large_queries)
	else O_YNO(opt, "harden-glue", harden_glue)
	else O_YNO(opt, "harden-dnssec-stripped", harden_dnssec_stripped)
	else O_YNO(opt, "harden-referral-path", harden_referral_path)
	else O_YNO(opt, "use-caps-for-id", use_caps_bits_for_id)
	else O_DEC(opt, "unwanted-reply-threshold", unwanted_threshold)
	else O_YNO(opt, "do-not-query-localhost", donotquery_localhost)
	else O_STR(opt, "module-config", module_conf)
	else O_STR(opt, "dlv-anchor-file", dlv_anchor_file)
	else O_DEC(opt, "val-bogus-ttl", bogus_ttl)
	else O_YNO(opt, "val-clean-additional", val_clean_additional)
	else O_DEC(opt, "val-log-level", val_log_level)
	else O_YNO(opt, "val-permissive-mode", val_permissive_mode)
	else O_STR(opt, "val-nsec3-keysize-iterations",val_nsec3_key_iterations)
	else O_UNS(opt, "add-holddown", add_holddown)
	else O_UNS(opt, "del-holddown", del_holddown)
	else O_UNS(opt, "keep-missing", keep_missing)
	else O_MEM(opt, "key-cache-size", key_cache_size)
	else O_DEC(opt, "key-cache-slabs", key_cache_slabs)
	else O_MEM(opt, "neg-cache-size", neg_cache_size)
	else O_YNO(opt, "control-enable", remote_control_enable)
	else O_DEC(opt, "control-port", control_port)
	else O_STR(opt, "server-key-file", server_key_file)
	else O_STR(opt, "server-cert-file", server_cert_file)
	else O_STR(opt, "control-key-file", control_key_file)
	else O_STR(opt, "control-cert-file", control_cert_file)
	else O_LST(opt, "root-hints", root_hints)
	else O_LS2(opt, "access-control", acls)
	else O_LST(opt, "do-not-query-address", donotqueryaddrs)
	else O_LST(opt, "private-address", private_address)
	else O_LST(opt, "private-domain", private_domain)
	else O_LST(opt, "auto-trust-anchor-file", auto_trust_anchor_file_list)
	else O_LST(opt, "trust-anchor-file", trust_anchor_file_list)
	else O_LST(opt, "trust-anchor", trust_anchor_list)
	else O_LST(opt, "trusted-keys-file", trusted_keys_file_list)
	else O_LST(opt, "dlv-anchor", dlv_anchor_list)
	else O_LST(opt, "control-interface", control_ifs)
	else O_UNS(opt, "val-override-date", val_date_override)
	/* not here:
	 * outgoing-permit, outgoing-avoid - have list of ports
	 * local-zone - zones and nodefault variables
	 * local-data - see below
	 * local-data-ptr - converted to local-data entries
	 * stub-zone, name, stub-addr, stub-host, stub-prime
	 * forward-zone, name, forward-addr, forward-host
	 */
	else fatal_exit("cannot print option '%s'", opt);
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

/** get base dir of a fname */
static char*
basedir(char* fname)
{
	char* rev;
	if(!fname) fatal_exit("out of memory");
	rev = strrchr(fname, '/');
	if(!rev) return NULL;
	if(fname == rev) return NULL;
	rev[0] = 0;
	return fname;
}

/** check chroot for a file string */
static void
check_chroot_string(const char* desc, char** ss,
	const char* chrootdir, struct config_file* cfg)
{
	char* str = *ss;
	if(str && str[0]) {
		*ss = fname_after_chroot(str, cfg, 1);
		if(!*ss) fatal_exit("out of memory");
		if(!is_file(*ss)) {
			if(chrootdir && chrootdir[0])
				fatal_exit("%s: \"%s\" does not exist in "
					"chrootdir %s", desc, str, chrootdir);
			else
				fatal_exit("%s: \"%s\" does not exist", 
					desc, str);
		}
		/* put in a new full path for continued checking */
		free(str);
	}
}

/** check file list, every file must be inside the chroot location */
static void
check_chroot_filelist(const char* desc, struct config_strlist* list,
	const char* chrootdir, struct config_file* cfg)
{
	struct config_strlist* p;
	for(p=list; p; p=p->next) {
		check_chroot_string(desc, &p->str, chrootdir, cfg);
	}
}

/** check file list, with wildcard processing */
static void
check_chroot_filelist_wild(const char* desc, struct config_strlist* list,
	const char* chrootdir, struct config_file* cfg)
{
	struct config_strlist* p;
	for(p=list; p; p=p->next) {
#ifdef HAVE_GLOB
		if(strchr(p->str, '*') || strchr(p->str, '[') || 
			strchr(p->str, '?') || strchr(p->str, '{') || 
			strchr(p->str, '~')) {
			char* s = p->str;
			/* adjust whole pattern for chroot and check later */
			p->str = fname_after_chroot(p->str, cfg, 1);
			free(s);
		} else
#endif /* HAVE_GLOB */
			check_chroot_string(desc, &p->str, chrootdir, cfg);
	}
}

/** check configuration for errors */
static void
morechecks(struct config_file* cfg, const char* fname)
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
	if(cfg->edns_buffer_size > cfg->msg_buffer_size)
		fatal_exit("edns-buffer-size larger than msg-buffer-size, "
			"answers will not fit in processing buffer");

	if(cfg->chrootdir && cfg->chrootdir[0] && 
		cfg->chrootdir[strlen(cfg->chrootdir)-1] == '/')
		fatal_exit("chootdir %s has trailing slash '/' please remove.",
			cfg->chrootdir);
	if(cfg->chrootdir && cfg->chrootdir[0] && 
		!is_dir(cfg->chrootdir)) {
		fatal_exit("bad chroot directory");
	}
	if(cfg->chrootdir && cfg->chrootdir[0]) {
		char buf[10240];
		buf[0] = 0;
		if(fname[0] != '/') {
			if(getcwd(buf, sizeof(buf)) == NULL)
				fatal_exit("getcwd: %s", strerror(errno));
			strncat(buf, "/", sizeof(buf)-strlen(buf)-1);
		}
		strncat(buf, fname, sizeof(buf)-strlen(buf)-1);
		if(strncmp(buf, cfg->chrootdir, strlen(cfg->chrootdir)) != 0)
			fatal_exit("config file %s is not inside chroot %s",
				buf, cfg->chrootdir);
	}
	if(cfg->directory && cfg->directory[0]) {
		char* ad = fname_after_chroot(cfg->directory, cfg, 0);
		if(!ad) fatal_exit("out of memory");
		if(!is_dir(ad)) fatal_exit("bad chdir directory");
		free(ad);
	}
	if( (cfg->chrootdir && cfg->chrootdir[0]) ||
	    (cfg->directory && cfg->directory[0])) {
		if(cfg->pidfile && cfg->pidfile[0]) {
			char* ad = (cfg->pidfile[0]=='/')?strdup(cfg->pidfile):
				fname_after_chroot(cfg->pidfile, cfg, 1);
			char* bd = basedir(ad);
			if(bd && !is_dir(bd))
				fatal_exit("pidfile directory does not exist");
			free(ad);
		}
		if(cfg->logfile && cfg->logfile[0]) {
			char* ad = fname_after_chroot(cfg->logfile, cfg, 1);
			char* bd = basedir(ad);
			if(bd && !is_dir(bd))
				fatal_exit("logfile directory does not exist");
			free(ad);
		}
	}

	check_chroot_filelist("file with root-hints", 
		cfg->root_hints, cfg->chrootdir, cfg);
	check_chroot_filelist("trust-anchor-file", 
		cfg->trust_anchor_file_list, cfg->chrootdir, cfg);
	check_chroot_filelist("auto-trust-anchor-file", 
		cfg->auto_trust_anchor_file_list, cfg->chrootdir, cfg);
	check_chroot_filelist_wild("trusted-keys-file", 
		cfg->trusted_keys_file_list, cfg->chrootdir, cfg);
	check_chroot_string("dlv-anchor-file", &cfg->dlv_anchor_file, 
		cfg->chrootdir, cfg);
	/* remove chroot setting so that modules are not stripping pathnames*/
	free(cfg->chrootdir);
	cfg->chrootdir = NULL;
	
	if(strcmp(cfg->module_conf, "iterator") != 0 
		&& strcmp(cfg->module_conf, "validator iterator") != 0
#ifdef WITH_PYTHONMODULE
		&& strcmp(cfg->module_conf, "python iterator") != 0 
		&& strcmp(cfg->module_conf, "python validator iterator") != 0 
		&& strcmp(cfg->module_conf, "validator python iterator") != 0
#endif
		) {
		fatal_exit("module conf '%s' is not known to work",
			cfg->module_conf);
	}

#ifdef HAVE_GETPWNAM
	if(cfg->username && cfg->username[0]) {
		if(getpwnam(cfg->username) == NULL)
			fatal_exit("user '%s' does not exist.", cfg->username);
		endpwent();
	}
#endif
	if(cfg->remote_control_enable) {
		check_chroot_string("server-key-file", &cfg->server_key_file,
			cfg->chrootdir, cfg);
		check_chroot_string("server-cert-file", &cfg->server_cert_file,
			cfg->chrootdir, cfg);
		if(!is_file(cfg->control_key_file))
			fatal_exit("control-key-file: \"%s\" does not exist",
				cfg->control_key_file);
		if(!is_file(cfg->control_cert_file))
			fatal_exit("control-cert-file: \"%s\" does not exist",
				cfg->control_cert_file);
	}

	localzonechecks(cfg);
}

/** check forwards */
static void
check_fwd(struct config_file* cfg)
{
	struct iter_forwards* fwd = forwards_create();
	if(!fwd || !forwards_apply_cfg(fwd, cfg)) {
		fatal_exit("Could not set forward zones");
	}
	forwards_delete(fwd);
}

/** check config file */
static void
checkconf(const char* cfgfile, const char* opt)
{
	struct config_file* cfg = config_create();
	if(!cfg)
		fatal_exit("out of memory");
	if(!config_read(cfg, cfgfile, NULL)) {
		/* config_read prints messages to stderr */
		config_delete(cfg);
		exit(1);
	}
	morechecks(cfg, cfgfile);
	check_mod(cfg, iter_get_funcblock());
	check_mod(cfg, val_get_funcblock());
#ifdef WITH_PYTHONMODULE
	if(strstr(cfg->module_conf, "python"))
		check_mod(cfg, pythonmod_get_funcblock());
#endif
	check_fwd(cfg);
	if(opt) print_option(cfg, opt);
	else	printf("unbound-checkconf: no errors in %s\n", cfgfile);
	config_delete(cfg);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/** Main routine for checkconf */
int main(int argc, char* argv[])
{
	int c;
	const char* f;
	const char* opt = NULL;
	log_ident_set("unbound-checkconf");
	log_init(NULL, 0, NULL);
	checklock_start();
	/* parse the options */
	while( (c=getopt(argc, argv, "ho:")) != -1) {
		switch(c) {
		case 'o':
			opt = optarg;
			break;
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
	checkconf(f, opt);
	checklock_stop();
	return 0;
}
