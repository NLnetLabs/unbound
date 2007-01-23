/*
 * util/log.c - implementation of the log code
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */
/**
 * \file
 * Implementation of log.h.
 */

#include "config.h"
#include "util/log.h"
#ifdef HAVE_TIME_H
#include <time.h>
#endif

void
log_init()
{
}

void
log_vmsg(const char* type, const char *format, va_list args)
{
	char message[MAXSYSLOGMSGLEN];
	const char* ident="unbound";
	vsnprintf(message, sizeof(message), format, args);
	fprintf(stderr, "[%d] %s[%d] %s: %s\n",
		(int)time(NULL), ident, (int)getpid(), type, message);
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
