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
#ifdef HAVE_SYSLOG_H
#  include <syslog.h>
#else
/**define LOG_ constants */
#  define LOG_CRIT 2
#  define LOG_ERR 3
#  define LOG_WARNING 4
#  define LOG_NOTICE 5
#  define LOG_INFO 6
#  define LOG_DEBUG 7
#endif

/* default verbosity */
enum verbosity_value verbosity = 0;
/** the file logged to. */
static FILE* logfile = 0;
/** if key has been created */
static int key_created = 0;
/** pthread key for thread ids in logfile */
static ub_thread_key_t logkey;
/** the identity of this executable/process */
static const char* ident="unbound";
#ifdef HAVE_SYSLOG_H
/** are we using syslog(3) to log to */
static int log_to_syslog = 0;
#endif /* HAVE_SYSLOG_H */

void
log_init(const char* filename, int use_syslog, const char* chrootdir)
{
	FILE *f;
	if(!key_created) {
		key_created = 1;
		ub_thread_key_create(&logkey, NULL);
	}
	if(logfile 
#ifdef HAVE_SYSLOG_H
	|| log_to_syslog
#endif
	)
	verbose(VERB_QUERY, "switching log to %s", 
		use_syslog?"syslog":(filename&&filename[0]?filename:"stderr"));
	if(logfile && logfile != stderr)
		fclose(logfile);
#ifdef HAVE_SYSLOG_H
	if(log_to_syslog) {
		closelog();
		log_to_syslog = 0;
	}
	if(use_syslog) {
		openlog(ident, 0, LOG_DAEMON);
		log_to_syslog = 1;
		return;
	}
#endif /* HAVE_SYSLOG_H */
	if(!filename || !filename[0]) {
		logfile = stderr;
		return;
	}
	/* open the file for logging */
	if(chrootdir && chrootdir[0] && strncmp(filename, chrootdir,
		strlen(chrootdir)) == 0) 
		filename += strlen(chrootdir);
	f = fopen(filename, "a");
	if(!f) {
		log_err("Could not open logfile %s: %s", filename, 
			strerror(errno));
		return;
	}
	logfile = f;
}

void log_file(FILE *f)
{
	logfile = f;
}

void log_thread_set(int* num)
{
	ub_thread_key_set(logkey, num);
}

void log_ident_set(const char* id)
{
	ident = id;
}

void
log_vmsg(int pri, const char* type,
	const char *format, va_list args)
{
	char message[MAXSYSLOGMSGLEN];
	unsigned int* tid = (unsigned int*)ub_thread_key_get(logkey);
	(void)pri;
	vsnprintf(message, sizeof(message), format, args);
#ifdef HAVE_SYSLOG_H
	if(log_to_syslog) {
		syslog(pri, "[%d:%x] %s: %s", 
			(int)getpid(), tid?*tid:0, type, message);
		return;
	}
#endif /* HAVE_SYSLOG_H */
	if(!logfile) return;
	fprintf(logfile, "[%d] %s[%d:%x] %s: %s\n", (int)time(NULL), 
		ident, (int)getpid(), tid?*tid:0, type, message);
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
	log_vmsg(LOG_INFO, "info", format, args);
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
	log_vmsg(LOG_ERR, "error", format, args);
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
	log_vmsg(LOG_WARNING, "warning", format, args);
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
	log_vmsg(LOG_CRIT, "fatal error", format, args);
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
	if(verbosity >= level) {
		if(level == VERB_OPS)
			log_vmsg(LOG_NOTICE, "notice", format, args);
		else if(level == VERB_DETAIL)
			log_vmsg(LOG_INFO, "info", format, args);
		else	log_vmsg(LOG_DEBUG, "debug", format, args);
	}
	va_end(args);
}

void 
log_hex(const char* msg, void* data, size_t length)
{
	size_t i, j;
	uint8_t* data8 = (uint8_t*)data;
	const char* hexchar = "0123456789ABCDEF";
	char buf[1024+1]; /* alloc blocksize hex chars + \0 */
	const size_t blocksize = 1024;
	size_t len;

	if(length == 0) {
		log_info("%s[%u]", msg, (unsigned)length);
		return;
	}

	for(i=0; i<length; i+=blocksize/2) {
		len = blocksize/2;
		if(length - i < blocksize/2)
			len = length - i;
		for(j=0; j<len; j++) {
			buf[j*2] = hexchar[ data8[i+j] >> 4 ];
			buf[j*2 + 1] = hexchar[ data8[i+j] & 0xF ];
		}
		buf[len*2] = 0;
		log_info("%s[%u:%u] %.*s", msg, (unsigned)length, 
			(unsigned)i, (int)len*2, buf);
	}
}

void log_buf(enum verbosity_value level, const char* msg, ldns_buffer* buf)
{
	if(verbosity < level)
		return;
	log_hex(msg, ldns_buffer_begin(buf), ldns_buffer_limit(buf));
}
