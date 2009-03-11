/*
 * winrc/win_svc.c - windows services API implementation for unbound
 *
 * Copyright (c) 2009, NLnet Labs. All rights reserved.
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
 * This file contains functions to integrate with the windows services API.
 * This means it handles the commandline switches to install and remove
 * the service (via CreateService and DeleteService), it handles
 * the ServiceMain() main service entry point when started as a service,
 * and it handles the Handler[_ex]() to process requests to the service
 * (such as start and stop and status).
 */
#include "config.h"
#include "winrc/win_svc.h"
#include "winrc/w_inst.h"
#include "daemon/daemon.h"
#include "daemon/worker.h"
#include "util/config_file.h"
#include "util/netevent.h"
#include "util/winsock_event.h"

/** from gen_msg.h - success message record for windows message log */
#define MSG_GENERIC_SUCCESS              ((DWORD)0x20010001L)
/** from gen_msg.h - informational message record for windows message log */
#define MSG_GENERIC_INFO                 ((DWORD)0x60010002L)
/** from gen_msg.h - warning message record for windows message log */
#define MSG_GENERIC_WARN                 ((DWORD)0xA0010003L)
/** from gen_msg.h - error message record for windows message log */
#define MSG_GENERIC_ERR                  ((DWORD)0xE0010004L)

/** global service status */
SERVICE_STATUS	service_status;
/** global service status handle */
SERVICE_STATUS_HANDLE service_status_handle;
/** global service stop event */
WSAEVENT service_stop_event = NULL;
/** event struct for stop callbacks */
struct event service_stop_ev;
/** config file to open. global communication to service_main() */
const char* service_cfgfile = CONFIGFILE;
/** commandline verbosity. global communication to service_main() */
int service_cmdline_verbose = 0;

/**
 * Report current service status to service control manager
 * @param state: current state
 * @param exitcode: error code (when stopped)
 * @param wait: pending operation estimated time in milliseconds.
 */
void report_status(DWORD state, DWORD exitcode, DWORD wait)
{
	static DWORD checkpoint = 1;
	service_status.dwCurrentState = state;
	service_status.dwWin32ExitCode = exitcode;
	service_status.dwWaitHint = wait;
	if(state == SERVICE_START_PENDING)
		service_status.dwControlsAccepted = 0;
	else 	service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	if(state == SERVICE_RUNNING || state == SERVICE_STOPPED)
		service_status.dwCheckPoint = 0;
	else	service_status.dwCheckPoint = checkpoint++;
	SetServiceStatus(service_status_handle, &service_status);
}

/**
 * Service control handler. Called by serviceControlManager when a control
 * code is sent to the service (with ControlService).
 * @param ctrl: control code
 */
static void 
hdlr(DWORD ctrl)
{
	if(ctrl == SERVICE_CONTROL_STOP) {
		report_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
		/* send signal to stop */
		if(!WSASetEvent(service_stop_event))
			log_err("Could not WSASetEvent: %s",
				wsa_strerror(WSAGetLastError()));
		return;
	} else {
		/* ctrl == SERVICE_CONTROL_INTERROGATE or whatever */
		/* update status */
		report_status(service_status.dwCurrentState, NO_ERROR, 0);
	}
}

/**
 * report event to system event log
 * For use during startup and shutdown.
 * @param str: the error
 */
static void
reportev(const char* str)
{
	char b[256];
	HANDLE* s;
	LPCTSTR msg = b;
	/* print quickly to keep GetLastError value */
	snprintf(b, sizeof(b), "%s: %s (%d)", SERVICE_NAME, 
		str, (int)GetLastError());
	s = RegisterEventSource(NULL, SERVICE_NAME);
	if(!s) return;
	ReportEvent(s, /* event log */
		EVENTLOG_ERROR_TYPE, /* event type */
		0, /* event category */
		MSG_GENERIC_ERR, /* event ID (from gen_msg.mc) */
		NULL, /* user security context */
		1, /* numstrings */
		0, /* binary size */
		&msg, /* strings */
		NULL); /* binary data */
	DeregisterEventSource(s);
}

/**
 * Init service. Keeps calling status pending to tell service control
 * manager that this process is not hanging.
 * @param d: daemon returned here.
 * @param c: config file returned here.
 * @return false if failed.
 */
static int
service_init(struct daemon** d, struct config_file** c)
{
	struct config_file* cfg = NULL;
	struct daemon* daemon = NULL;
	const char* logfile= "C:\\unbound.log";
	verbosity=4; service_cmdline_verbose=4;
	log_init(logfile, 0, NULL); /* DEBUG logfile */

	/* create daemon */
	daemon = daemon_init();
	if(!daemon) return 0;
	report_status(SERVICE_START_PENDING, NO_ERROR, 2800);

	/* read config */
	cfg = config_create();
	if(!cfg) return 0;
	if(!config_read(cfg, service_cfgfile, daemon->chroot)) {
		if(errno != ENOENT) {
			log_err("error in config file");
			return 0;
		}
		log_warn("could not open config file, using defaults");
	}
	report_status(SERVICE_START_PENDING, NO_ERROR, 2600);

	verbose(VERB_QUERY, "winservice - apply settings");
	/* apply settings and init */
	verbosity = cfg->verbosity + service_cmdline_verbose;
	if(cfg->directory && cfg->directory[0]) {
		if(chdir(cfg->directory)) {
			log_err("could not chdir to %s: %s", 
				cfg->directory, strerror(errno));
			if(errno != ENOENT)
				return 0;
			log_warn("could not change directory - continuing");
		} else
			verbose(VERB_QUERY, "chdir to %s", cfg->directory);
	}
	/* log_init(cfg->logfile, cfg->use_syslog, cfg->chrootdir); DEBUG*/
	report_status(SERVICE_START_PENDING, NO_ERROR, 2400);
	verbose(VERB_QUERY, "winservice - apply cfg");
	daemon_apply_cfg(daemon, cfg);
	
	/* open ports */
	/* keep reporting that we are busy starting */
	report_status(SERVICE_START_PENDING, NO_ERROR, 2200);
	verbose(VERB_QUERY, "winservice - open ports");
	if(!daemon_open_shared_ports(daemon)) return 0;
	verbose(VERB_QUERY, "winservice - ports opened");
	report_status(SERVICE_START_PENDING, NO_ERROR, 2000);

	*d = daemon;
	*c = cfg;
	return 1;
}

/**
 * The main function for the service.
 * Called by the services API when starting unbound on windows in background.
 * Arguments could have been present in the string 'path'.
 * @param argc: nr args
 * @param argv: arg text.
 */
static void 
service_main(DWORD ATTR_UNUSED(argc), LPTSTR* ATTR_UNUSED(argv))
{
	struct config_file* cfg = NULL;
	struct daemon* daemon = NULL;

	reportev("Trying to report event");

	service_status_handle = RegisterServiceCtrlHandler(SERVICE_NAME, 
		(LPHANDLER_FUNCTION)hdlr);
	if(!service_status_handle) {
		reportev("Could not RegisterServiceCtrlHandler");
		return;
	}
	
	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwServiceSpecificExitCode = 0;

	/* we are now starting up */
	report_status(SERVICE_START_PENDING, NO_ERROR, 3000);
	if(!service_init(&daemon, &cfg)) {
		reportev("Could not service_init");
		report_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	/* event that gets signalled when we want to quit; it
	 * should get registered in the worker-0 waiting loop. */
	service_stop_event = WSACreateEvent();
	if(service_stop_event == WSA_INVALID_EVENT) {
		log_err("WSACreateEvent: %s", wsa_strerror(WSAGetLastError()));
		reportev("Could not WSACreateEvent");
		report_status(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
	if(!WSAResetEvent(service_stop_event)) {
		log_err("WSAResetEvent: %s", wsa_strerror(WSAGetLastError()));
	}

	/* SetServiceStatus SERVICE_RUNNING;*/
	report_status(SERVICE_RUNNING, NO_ERROR, 0);
	verbose(VERB_QUERY, "winservice - init complete");

	/* daemon performs work */
	daemon_fork(daemon);

	/* exit */
	verbose(VERB_ALGO, "winservice - cleanup.");
	report_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
	daemon_cleanup(daemon);
	config_delete(cfg);
	daemon_delete(daemon);
	(void)WSACloseEvent(service_stop_event);
	verbose(VERB_QUERY, "winservice - full stop");
	report_status(SERVICE_STOPPED, NO_ERROR, 0);
}

/** start the service */
static void 
service_start(const char* cfgfile, int v)
{
	SERVICE_TABLE_ENTRY myservices[2] = {
		{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)service_main},
		{NULL, NULL} };
	verbosity=v;
	if(1) {
		/* DEBUG log to file */
		fclose(fopen("C:\\unbound.log", "w"));
		log_init("C:\\unbound.log", 0, 0);
		verbose(VERB_QUERY, "open logfile");
	}
	service_cfgfile = cfgfile;
	service_cmdline_verbose = v;
	/* this call returns when service has stopped. */
	if(!StartServiceCtrlDispatcher(myservices)) {
		reportev("Could not StartServiceCtrlDispatcher");
	}
}

void
wsvc_command_option(const char* wopt, const char* cfgfile, int v)
{
	if(strcmp(wopt, "install") == 0)
		wsvc_install(stdout, NULL);
	else if(strcmp(wopt, "remove") == 0)
		wsvc_remove(stdout);
	else if(strcmp(wopt, "service") == 0)
		service_start(cfgfile, v);
	else fatal_exit("unknown option: %s", wopt);
	exit(0);
}

void
worker_win_stop_cb(int ATTR_UNUSED(fd), short ATTR_UNUSED(ev), void* arg)
{
        struct worker* worker = (struct worker*)arg;
        verbose(VERB_QUERY, "caught stop signal (wsaevent)");
        worker->need_to_exit = 1;
        comm_base_exit(worker->base);
}

void wsvc_setup_worker(struct worker* worker)
{
	/* if not started with -w service, do nothing */
	if(!service_stop_event)
		return;
	if(!winsock_register_wsaevent(comm_base_internal(worker->base),
		&service_stop_ev, service_stop_event,
		&worker_win_stop_cb, worker)) {
		fatal_exit("could not register wsaevent");
		return;
	}
}

