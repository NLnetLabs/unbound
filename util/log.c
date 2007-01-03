/*
 * util/log.c - implementation of the log code
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
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
log_vmsg(const char *format, va_list args)
{
	char message[MAXSYSLOGMSGLEN];
	const char* ident="unbound";
	vsnprintf(message, sizeof(message), format, args);
	fprintf(stderr, "[%d] %s[%d]: %s\n",
		(int)time(NULL), ident, (int)getpid(), message);
}

void
log_info(const char *format, ...)
{
        va_list args;
	va_start(args, format);
	log_vmsg(format, args);
	va_end(args);
}
