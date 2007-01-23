/*
 * util/log.h - logging service
 *
 * Copyright (c) 2007, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */

/**
 * \file
 *
 * This file contains logging functions.
 */

#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include "config.h"
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

/**
 * call this to initialize logging services.
 */
void log_init();

/**
 * Log informational message.
 * Pass printf formatted arguments. No trailing newline is needed.
 * @param format: printf-style format string. Arguments follow.
 */
void log_info(const char* format, ...) ATTR_FORMAT(printf, 1, 2);

/**
 * Log error message.
 * Pass printf formatted arguments. No trailing newline is needed.
 * @param format: printf-style format string. Arguments follow.
 */
void log_err(const char* format, ...) ATTR_FORMAT(printf, 1, 2);

/**
 * va_list argument version of log_info.
 * @param type: string to designate type of message (info, error).
 * @param format: the printf style format to print. no newline.
 * @param args: arguments for format string.
 */
void log_vmsg(const char* type, const char *format, va_list args);

#endif /* UTIL_LOG_H */
