/*
 * util/log.h - logging service
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */

#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include "config.h"
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

/**
 * call this to initialize logging services
 */
void log_init();

/**
 * Pass printf formatted arguments. No trailing newline is needed.
 */
void log_info(const char* format, ...) ATTR_FORMAT(printf, 1, 2);

/**
 * va_list argument version of log_info.
 */
void log_vmsg(const char *format, va_list args);

#endif /* UTIL_LOG_H */
