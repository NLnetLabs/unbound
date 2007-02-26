/*
 * util/log.c - implementation of the log code
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
 * Implementation of log.h.
 */

#include "config.h"
#include "util/log.h"
#include "util/locks.h"
#ifdef HAVE_TIME_H
#include <time.h>
#endif

enum verbosity_value verbosity = 0;
/** the file logged to. */
static FILE* logfile = 0;

void
log_init(const char* filename)
{
	FILE *f;
	if(!filename || !filename[0]) {
		if(logfile && logfile != stderr)
			fclose(logfile);
		logfile = stderr;
		return;
	}
	/* open the file for logging */
	f = fopen(filename, "a");
	if(!f) {
		log_err("Could not open logfile %s: %s", filename, 
			strerror(errno));
		return;
	}
	verbose(VERB_DETAIL, "switching to logfile %s", filename);
	if(logfile && logfile != stderr)
		fclose(logfile);
	logfile = f;
}

void
log_vmsg(const char* type, const char *format, va_list args)
{
	char message[MAXSYSLOGMSGLEN];
	const char* ident="unbound";
	vsnprintf(message, sizeof(message), format, args);
	fprintf(logfile, "[%d] %s[%d:%x] %s: %s\n",
		(int)time(NULL), ident, (int)getpid(), 
		(int)ub_thread_self(),
		type, message);
	fflush(logfile);
}

/**
 * implementation of log_info
 * @param format: format string printf-style.
 */
void
log_info(const char *format, ...)
{
        va_list args;
	va_start(args, format);
	log_vmsg("info", format, args);
	va_end(args);
}

/**
 * implementation of log_err
 * @param format: format string printf-style.
 */
void
log_err(const char *format, ...)
{
        va_list args;
	va_start(args, format);
	log_vmsg("error", format, args);
	va_end(args);
}

/**
 * implementation of log_warn
 * @param format: format string printf-style.
 */
void
log_warn(const char *format, ...)
{
        va_list args;
	va_start(args, format);
	log_vmsg("warning", format, args);
	va_end(args);
}

/**
 * implementation of fatal_exit
 * @param format: format string printf-style.
 */
void
fatal_exit(const char *format, ...)
{
        va_list args;
	va_start(args, format);
	log_vmsg("fatal error", format, args);
	va_end(args);
	exit(1);
}

/**
 * implementation of verbose
 * @param level: verbose level for the message.
 * @param format: format string printf-style.
 */
void
verbose(enum verbosity_value level, const char* format, ...)
{
        va_list args;
	va_start(args, format);
	if(verbosity >= level)
		log_vmsg("note", format, args);
	va_end(args);
}

