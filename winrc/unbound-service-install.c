/*
 * winrc/unbound-service-install.c - windows services installation util
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

/** service name for unbound (internal to ServiceManager) */
#define SERVICE_NAME "unbound"

/** output for diagnostics */
static FILE *out;

/** exit with windows error */
static void
fatal_win(const char* str)
{
	fprintf(out, "%s (%d)\n", str, (int)GetLastError());
	exit(1);
}

/** put quotes around string. Needs one space in front 
 * @param str: to be quoted.
 * @param maxlen: max length of the string buffer.
 */
static void
quote_it(char* str, size_t maxlen)
{
	if(strlen(str) == maxlen) {
		fprintf(out, "string too long %s", str);
		exit(1);
	}
	str[0]='"';
	str[strlen(str)+1]=0;
	str[strlen(str)]='"';
}

/** change suffix */
static void
change(char* path, size_t max, const char* from, const char* to)
{
	size_t fromlen = strlen(from);
	size_t tolen = strlen(to);
	size_t pathlen = strlen(path);
	if(pathlen - fromlen + tolen >= max) {
		fprintf(out, "string too long %s", path);
		exit(1);
	}
	snprintf(path+pathlen-fromlen, max-(pathlen-fromlen), "%s", to);
}

/** Install service in servicecontrolmanager */
static void 
wsvc_install(void)
{
	SC_HANDLE scm;
	SC_HANDLE sv;
	TCHAR path[MAX_PATH+2+256];
	fprintf(out, "installing unbound service\n");
	if(!GetModuleFileName(NULL, path+1, MAX_PATH))
		fatal_win("could not GetModuleFileName");
	/* change 'unbound-service-install' to 'unbound' */
	change(path, sizeof(path), "-service-install.exe", ".exe");

	/* have to quote it because of spaces in directory names */
	/* could append arguments to be sent to ServiceMain */
	quote_it(path, sizeof(path));
	strcat(path, " -w service");
	scm = OpenSCManager(NULL, NULL, (int)SC_MANAGER_CREATE_SERVICE);
	if(!scm) fatal_win("could not OpenSCManager");
	sv = CreateService(
		scm,
		SERVICE_NAME, /* name of service */
		"Unbound DNS validator", /* display name */
		SERVICE_ALL_ACCESS, /* desired access */
		SERVICE_WIN32_OWN_PROCESS, /* service type */
		SERVICE_AUTO_START, /* start type */
		SERVICE_ERROR_NORMAL, /* error control type */
		path, /* path to service's binary */
		NULL, /* no load ordering group */
		NULL, /* no tag identifier */
		NULL, /* no deps */
		NULL, /* on LocalSystem */
		NULL /* no password */
		);
	if(!sv) {
		CloseServiceHandle(scm);
		fatal_win("could not CreateService");
	}
	CloseServiceHandle(sv);
	CloseServiceHandle(scm);
	fprintf(out, "unbound service installed\n");
}

/** Install service main */
int main(int ATTR_UNUSED(argc), char** ATTR_UNUSED(argv))
{
	out = fopen("unbound-service-install.log", "w");
	wsvc_install();
	return 0;
}
