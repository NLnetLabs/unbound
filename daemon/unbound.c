/*
 * daemon/unbound.c - main program for unbound DNS resolver daemon.
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
 *
 */

/**
 * \file
 *
 * Main program to start the DNS resolver daemon.
 */

#include "config.h"
#include "util/log.h"
#include "daemon/daemon.h"
#include "util/config_file.h"
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>

/** print usage. */
static void usage()
{
	printf("usage: unbound [options]\n");
	printf("	start unbound daemon DNS resolver.\n");
	printf("-h	this help\n");
	printf("-c file	config file to read, unbound.conf(5).\n");
	printf("-d	do not fork into the background.\n");
	printf("-v	verbose (multiple times increase verbosity)\n");
	printf("Version %s\n", PACKAGE_VERSION);
	printf("BSD licensed, see LICENSE in source package for details.\n");
	printf("Report bugs to %s\n", PACKAGE_BUGREPORT);
}

/** to changedir, logfile */
static void
apply_dir(struct daemon* daemon, struct config_file* cfg, int cmdline_verbose)
{
	/* apply changes if they have changed */
	daemon->cfg = cfg;
	verbosity = cmdline_verbose + cfg->verbosity;
	if(cfg->directory && cfg->directory[0]) {
		if(!daemon->cwd || strcmp(daemon->cwd, cfg->directory) != 0) {
			if(chdir(cfg->directory)) {
				log_err("Could not chdir to %s: %s",
					cfg->directory, strerror(errno));
			}
			free(daemon->cwd);
			if(!(daemon->cwd = strdup(cfg->directory)))
				fatal_exit("malloc failed");
		}
	}
}

/** Read existing pid from pidfile. */
static pid_t
readpid (const char* file)
{
	int fd;
	pid_t pid;
	char pidbuf[32];
	char* t;
	ssize_t l;

	if ((fd = open(file, O_RDONLY)) == -1) {
		return -1;
	}

	if (((l = read(fd, pidbuf, sizeof(pidbuf)))) == -1) {
		close(fd);
		return -1;
	}

	close(fd);

	/* Empty pidfile means no pidfile... */
	if (l == 0) {
		errno = ENOENT;
		return -1;
	}

	pid = strtol(pidbuf, &t, 10);
	
	if (*t && *t != '\n') {
		return -1;
	}
	return pid;
}

/** write pid to file. */
static void
writepid (const char* pidfile, pid_t pid)
{
	FILE* f;

	if ((f = fopen(pidfile, "w")) ==  NULL ) {
		log_err("cannot open pidfile %s: %s", 
			pidfile, strerror(errno));
		return;
	}
	if(fprintf(f, "%lu\n", (unsigned long)pid) < 0) {
		log_err("cannot write to pidfile %s: %s", 
			pidfile, strerror(errno));
	}
	fclose(f);
}

/**
 * check old pid file.
 * @param cfg: the config settings
 */
static void
checkoldpid(struct config_file* cfg)
{
	pid_t old;
	if((old = readpid(cfg->pidfile)) == -1) {
		if(errno != ENOENT) {
			log_err("Could not read pidfile %s: %s",
				cfg->pidfile, strerror(errno));
		}
	} else {
		/** see if it is still alive */
		if(kill(old, 0) == 0 || errno == EPERM)
			log_warn("unbound is already running as pid %u.", old);
		else	log_warn("did not exit gracefully last time (%u)", old);
	}
}

/** daemonize, drop user priviliges and chroot if needed */
static void
do_chroot(struct daemon* daemon, struct config_file* cfg, int debug_mode)
{
	log_assert(cfg);

	/* daemonize last to be able to print error to user */
	if(cfg->chrootdir && cfg->chrootdir[0])
		if(chroot(cfg->chrootdir))
			fatal_exit("unable to chroot: %s", strerror(errno));
	if(cfg->username && cfg->username[0]) {
		struct passwd *pwd;
		if((pwd = getpwnam(cfg->username)) == NULL)
			fatal_exit("user '%s' does not exist.", cfg->username);
		if(setgid(pwd->pw_gid) != 0)
			fatal_exit("unable to set group id: %s", strerror(errno));
		if(setuid(pwd->pw_uid) != 0)
			fatal_exit("unable to set user id: %s", strerror(errno));
		endpwent();
	}
	/* check old pid file before forking */
	if(cfg->pidfile && cfg->pidfile[0]) {
		checkoldpid(cfg);
	}

	/* init logfile just before fork */
	log_init(cfg->logfile);
	if(!debug_mode && cfg->do_daemonize) {
		int fd;
		/* Take off... */
		switch (fork()) {
			case 0:
				break;
			case -1:
				unlink(cfg->pidfile);
				fatal_exit("fork failed: %s", strerror(errno));
			default:
				/* exit interactive session */
				exit(0);
		}
		/* detach */
		if(setsid() == -1)
			fatal_exit("setsid() failed: %s", strerror(errno));
		if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
			(void)dup2(fd, STDIN_FILENO);
			(void)dup2(fd, STDOUT_FILENO);
			(void)dup2(fd, STDERR_FILENO);
			if (fd > 2)
				(void)close(fd);
		}
	}
	if(cfg->pidfile && cfg->pidfile[0]) {
		writepid(cfg->pidfile, getpid());
		daemon->pidfile = strdup(cfg->pidfile);
	}
}

/**
 * Run the daemon. 
 * @param cfgfile: the config file name.
 * @param cmdline_verbose: verbosity resulting from commandline -v.
 *    These increase verbosity as specified in the config file.
 * @param debug_mode: if set, do not daemonize.
 */
static void run_daemon(const char* cfgfile, int cmdline_verbose, int debug_mode)
{
	struct config_file* cfg = NULL;
	struct daemon* daemon = NULL;
	int done_chroot = 0;

	if(!(daemon = daemon_init()))
		fatal_exit("alloc failure");
	while(!daemon->need_to_exit) {
		if(done_chroot)
			log_info("Restart of %s.", PACKAGE_STRING);
		else	log_info("Start of %s.", PACKAGE_STRING);

		/* config stuff */
		if(!(cfg = config_create()))
			fatal_exit("Could not alloc config defaults");
		if(!config_read(cfg, cfgfile))
			fatal_exit("Could not read config file: %s", cfgfile);
		apply_dir(daemon, cfg, cmdline_verbose);
	
		/* prepare */
		if(!daemon_open_shared_ports(daemon))
			fatal_exit("could not open ports");
		if(!done_chroot) { 
			do_chroot(daemon, cfg, debug_mode); 
			done_chroot = 1; 
		}
		/* work */
		daemon_fork(daemon);

		/* clean up for restart */
		verbose(VERB_ALGO, "cleanup.");
		daemon_cleanup(daemon);
		config_delete(cfg);
	}
	verbose(VERB_ALGO, "Exit cleanup.");
	if(daemon->pidfile)
		unlink(daemon->pidfile);
	daemon_delete(daemon);
}

/** getopt global, in case header files fail to declare it. */
extern int optind;
/** getopt global, in case header files fail to declare it. */
extern char* optarg;

/**
 * main program. Set options given commandline arguments.
 * @param argc: number of commandline arguments.
 * @param argv: array of commandline arguments.
 * @return: exit status of the program.
 */
int 
main(int argc, char* argv[])
{
	int c;
	const char* cfgfile = NULL;
	int cmdline_verbose = 0;
	int debug_mode = 0;

	log_init(NULL);
	/* parse the options */
	while( (c=getopt(argc, argv, "c:dhv")) != -1) {
		switch(c) {
		case 'c':
			cfgfile = optarg;
			break;
		case 'v':
			cmdline_verbose ++;
			verbosity++;
			break;
		case 'd':
			debug_mode = 1;
			break;
		case '?':
		case 'h':
		default:
			usage();
			return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if(argc != 0) {
		usage();
		return 1;
	}

	run_daemon(cfgfile, cmdline_verbose, debug_mode);
	return 0;
}
