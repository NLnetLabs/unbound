#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)

AC_INIT(unbound, 0.0, wouter@nlnetlabs.nl, unbound)

CFLAGS=
AC_AIX

dnl routine to help check for compiler flags.
AC_DEFUN([CHECK_COMPILER_FLAG],
[
AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(whether $CC supports -$1)
cache=`echo $1 | sed 'y%.=/+-%___p_%'`
AC_CACHE_VAL(cv_prog_cc_flag_$cache,
[
echo 'void f(){}' >conftest.c
if test -z "`$CC -$1 -c conftest.c 2>&1`"; then
eval "cv_prog_cc_flag_$cache=yes"
else
eval "cv_prog_cc_flag_$cache=no"
fi
rm -f conftest*
])
if eval "test \"`echo '$cv_prog_cc_flag_'$cache`\" = yes"; then
AC_MSG_RESULT(yes)
:
$2
else
AC_MSG_RESULT(no)
:
$3
fi
])

dnl routine to help check for needed compiler flags.
# if the given code compiles without the flag, execute argument 4
# if the given code only compiles with the flag, execute argument 3
# otherwise fail, execute argument 5.
AC_DEFUN([CHECK_COMPILER_FLAG_NEEDED],
[
AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(whether we need $1 as a flag for $CC)
cache=`echo $1 | sed 'y%.=/+- %___p__%'`
AC_CACHE_VAL(cv_prog_cc_flag_needed_$cache,
[
echo '$2' > conftest.c
echo 'void f(){}' >>conftest.c
if test -z "`$CC $CFLAGS -Werror -Wall -c conftest.c 2>&1`"; then
eval "cv_prog_cc_flag_needed_$cache=no"
else
[
if test -z "`$CC $CFLAGS $1 -Werror -Wall -c conftest.c 2>&1`"; then
eval "cv_prog_cc_flag_needed_$cache=yes"
else
eval "cv_prog_cc_flag_needed_$cache=fail"
#echo 'Test with flag fails too!'
#cat conftest.c
#echo "$CC $CFLAGS $1 -Werror -Wall -c conftest.c 2>&1"
#echo `$CC $CFLAGS $1 -Werror -Wall -c conftest.c`
#exit 1
fi
]
fi
rm -f conftest
])
if eval "test \"`echo '$cv_prog_cc_flag_needed_'$cache`\" = yes"; then
AC_MSG_RESULT(yes)
:
$3
else
if eval "test \"`echo '$cv_prog_cc_flag_needed_'$cache`\" = no"; then
AC_MSG_RESULT(no)
:
$4
else
AC_MSG_RESULT(failed)
:
$5
fi
fi
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_LANG_C
CHECK_COMPILER_FLAG(g, [CFLAGS="$CFLAGS -g"])
CHECK_COMPILER_FLAG(O2, [CFLAGS="$CFLAGS -O2"])

AC_CHECK_HEADERS([getopt.h time.h],,, [AC_INCLUDES_DEFAULT])

CHECK_COMPILER_FLAG_NEEDED(-std=c99 -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600,
[
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <unistd.h>
#include <netdb.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

int test() {
	int a;
	char **opts = NULL;
	struct timeval tv;
	char *t;
	time_t time = 0;
	char *buf = NULL;
	const char* str = NULL;
	t = ctime_r(&time, buf);
	tv.tv_usec = 10;
	srandom(32);
	a = getopt(2, opts, "a");
	a = isascii(32);
	str = gai_strerror(0);
	return a;
}
], [CFLAGS="$CFLAGS -std=c99 -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600"])

CHECK_COMPILER_FLAG_NEEDED(-std=c99,
[
#include <stdbool.h>
#include <ctype.h>
int test() {
        int a = 0;
        return a;
}
], [CFLAGS="$CFLAGS -std=c99"])

CHECK_COMPILER_FLAG_NEEDED(-D_BSD_SOURCE,
[
#include <ctype.h>

int test() {
        int a;
        a = isascii(32);
        return a;
}
], [CFLAGS="$CFLAGS -D_BSD_SOURCE"])

CHECK_COMPILER_FLAG_NEEDED(-D_POSIX_C_SOURCE=200112,
[
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <netdb.h>

int test() {
        int a = 0;
        char *t;
        time_t time = 0;
        char *buf = NULL;
	const char* str = NULL;
        t = ctime_r(&time, buf);
	str = gai_strerror(0);
        return a;
}
], [CFLAGS="$CFLAGS -D_POSIX_C_SOURCE=200112"])

CHECK_COMPILER_FLAG_NEEDED(-D__EXTENSIONS__,
[
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

int test() {
        int a;
        char **opts = NULL;
        struct timeval tv;
        tv.tv_usec = 10;
        srandom(32);
        a = getopt(2, opts, "a");
        a = isascii(32);
        return a;
}
], [CFLAGS="$CFLAGS -D__EXTENSIONS__"])
AC_CHECK_DECLS([isascii, getopt, srandom, ctime_r, gai_strerror],, [echo "Cannot set compile environment; pass defines in CFLAGS=-D... ./configure"; exit 1], [
#include <stdlib.h>
#include <sys/time.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <netdb.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <stdbool.h>
#include <ctype.h>
])

# for Sun studio 11.
CHECK_COMPILER_FLAG(xO4, [CFLAGS="$CFLAGS -xO4"])
CHECK_COMPILER_FLAG(xtarget=generic, [CFLAGS="$CFLAGS -xtarget=generic"])
# flag warnings.
CHECK_COMPILER_FLAG(W, [CFLAGS="$CFLAGS -W"])
CHECK_COMPILER_FLAG(Wall, [CFLAGS="$CFLAGS -Wall"])
CHECK_COMPILER_FLAG(Wextra, [CFLAGS="$CFLAGS -Wextra"])
CHECK_COMPILER_FLAG(Wdeclaration-after-statement, [CFLAGS="$CFLAGS -Wdeclaration-after-statement"])

AC_C_INLINE

AC_DEFUN([AC_CHECK_FORMAT_ATTRIBUTE],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(whether the C compiler (${CC-cc}) accepts the "format" attribute)
AC_CACHE_VAL(ac_cv_c_format_attribute,
[ac_cv_c_format_attribute=no
AC_TRY_COMPILE(
[#include <stdio.h>
void f (char *format, ...) __attribute__ ((format (printf, 1, 2)));
void (*pf) (char *format, ...) __attribute__ ((format (printf, 1, 2)));
], [
   f ("%s", "str");
],
[ac_cv_c_format_attribute="yes"],
[ac_cv_c_format_attribute="no"])
])

AC_MSG_RESULT($ac_cv_c_format_attribute)
if test $ac_cv_c_format_attribute = yes; then
  AC_DEFINE(HAVE_ATTR_FORMAT, 1, [Whether the C compiler accepts the "format" attribute])
fi
])dnl

AC_DEFUN([AC_CHECK_UNUSED_ATTRIBUTE],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(whether the C compiler (${CC-cc}) accepts the "unused" attribute)
AC_CACHE_VAL(ac_cv_c_unused_attribute,
[ac_cv_c_unused_attribute=no
AC_TRY_COMPILE(
[#include <stdio.h>
void f (char *u __attribute__((unused)));
], [
   f ("x");
],
[ac_cv_c_unused_attribute="yes"],
[ac_cv_c_unused_attribute="no"])
])

AC_MSG_RESULT($ac_cv_c_unused_attribute)
if test $ac_cv_c_unused_attribute = yes; then
  AC_DEFINE(HAVE_ATTR_UNUSED, 1, [Whether the C compiler accepts the "unused" attribute])
fi
])dnl

AC_CHECK_FORMAT_ATTRIBUTE
AC_CHECK_UNUSED_ATTRIBUTE

if test "$srcdir" != "."; then
	CPPFLAGS="$CPPFLAGS -I$srcdir";
fi

# Use libtool 
AC_CHECK_PROGS(libtool, [glibtool libtool15 libtool], [./libtool])
AC_PATH_TOOL(AR, ar, [false])
if test $AR = false; then
	AC_MSG_ERROR([Cannot find 'ar', please extend PATH to include it])
fi
AC_CHECK_PROG(doxygen, doxygen, doxygen)
# avoid libtool max commandline length test on systems that fork slowly.
AC_CANONICAL_HOST
if echo "$host_os" | grep "sunos4" >/dev/null; then
	lt_cv_sys_max_cmd_len=32750;
fi
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([getopt.h stdarg.h stdbool.h netinet/in.h time.h sys/param.h sys/socket.h arpa/inet.h],,, [AC_INCLUDES_DEFAULT])

# check for types
AC_CHECK_TYPE(int8_t, char)
AC_CHECK_TYPE(int16_t, short)
AC_CHECK_TYPE(int32_t, int)
AC_CHECK_TYPE(int64_t, long long)
AC_CHECK_TYPE(uint8_t, unsigned char)
AC_CHECK_TYPE(uint16_t, unsigned short)
AC_CHECK_TYPE(uint32_t, unsigned int)
AC_CHECK_TYPE(uint64_t, unsigned long long)
AC_TYPE_SIZE_T
AC_CHECK_TYPE(ssize_t, int)
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_OFF_T
AC_CHECK_TYPE(u_char, unsigned char)

AC_CHECK_TYPE(socklen_t, , 
	[AC_DEFINE([socklen_t], [int], [Define to 'int' if not defined])], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
])
AC_CHECK_TYPE(in_addr_t, [], [AC_DEFINE([in_addr_t], [uint32_t], [in_addr_t])], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])
AC_CHECK_TYPE(in_port_t, [], [AC_DEFINE([in_port_t], [uint16_t], [in_port_t])], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])

# Checks for libraries.
AC_ARG_WITH(ssl, AC_HELP_STRING([--with-ssl=pathname],
                                    [enable SSL (will check /usr/local/ssl
                            /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr/sfw /usr)]),[
        ],[
            withval="yes"
        ])
    if test x_$withval != x_no; then
        AC_MSG_CHECKING(for SSL)
        if test x_$withval = x_ -o x_$withval = x_yes; then
            withval="/usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr/sfw /usr"
        fi
        for dir in $withval; do
            ssldir="$dir"
            if test -f "$dir/include/openssl/ssl.h"; then
                found_ssl="yes";
                AC_DEFINE_UNQUOTED([HAVE_SSL], [], [Define if you have the SSL libraries installed.])
                CPPFLAGS="$CPPFLAGS -I$ssldir/include";
                break;
            fi
        done
        if test x_$found_ssl != x_yes; then
            AC_MSG_ERROR(Cannot find the SSL libraries in $withval)
        else
            AC_MSG_RESULT(found in $ssldir)
            HAVE_SSL=yes
            LDFLAGS="$LDFLAGS -L$ssldir/lib";
	    RUNTIME_PATH="$RUNTIME_PATH -R$ssldir/lib"
            AC_CHECK_LIB(crypto, HMAC_CTX_init,, [
                    AC_MSG_ERROR([OpenSSL found in $ssldir, but version 0.9.7 or higher is required])
                ])
        fi
        AC_SUBST(HAVE_SSL)
	AC_SUBST(RUNTIME_PATH)
    fi
AC_CHECK_HEADERS([openssl/ssl.h],,, [AC_INCLUDES_DEFAULT])

# check for libevent
AC_ARG_WITH(libevent, AC_HELP_STRING([--with-libevent=pathname],
                                    [set path to libevent  (will check /usr/local /usr/lib /usr/pkg /usr/sfw /usr)]),[
        ],[
            withval="yes"
        ])
    if test x_$withval != x_no; then
        AC_MSG_CHECKING(for libevent)
        if test x_$withval = x_ -o x_$withval = x_yes; then
            withval="/usr/local /usr/lib /usr/pkg /usr/sfw /usr"
        fi
        for dir in $withval; do
            thedir="$dir"
            if test -f "$dir/include/event.h"; then
                found_libevent="yes";
                CPPFLAGS="$CPPFLAGS -I$thedir/include";
                break;
            fi
        done
        if test x_$found_libevent != x_yes; then
		if test -f "$dir/event.h" -a -f "$dir/.libs/libevent.a"; then
			# libevent source directory
            		AC_MSG_RESULT(found in $thedir)
                	CPPFLAGS="$CPPFLAGS -I$thedir";
            		LDFLAGS="$thedir/.libs/*.o $LDFLAGS";
	    		AC_SEARCH_LIBS(event_set, [event])
		else
            		AC_MSG_ERROR(Cannot find the libevent library in $withval)
		fi
        else
            AC_MSG_RESULT(found in $thedir)
            LDFLAGS="$LDFLAGS -L$thedir/lib";
	    RUNTIME_PATH="$RUNTIME_PATH -R$thedir/lib"
	    AC_SEARCH_LIBS(event_set, [event])
        fi
	AC_SUBST(RUNTIME_PATH)
    fi
AC_CHECK_HEADERS([event.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_FUNCS([event_base_free]) # only in libevent 1.2 and later

# check to see if libraries are needed for these functions.
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, inet_pton)

AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE

#AC_REPLACE_FUNCS(snprintf)
#AC_REPLACE_FUNCS(strlcpy)
#AC_REPLACE_FUNCS(memmove)

#AC_CHECK_FUNCS([getaddrinfo])
#if test $ac_cv_func_getaddrinfo = no; then
#AC_LIBOBJ([fake-rfc2553])
#fi

AC_ARG_WITH(ldns,
        AC_HELP_STRING([--with-ldns=PATH        specify prefix of path of ldns library to use])
	, [ specialldnsdir="$withval"
	CPPFLAGS="$CPPFLAGS -I$withval/include"
	LDFLAGS="$LDFLAGS -L$withval -L$withval/lib -lldns"
	ldnsdir="$withval"
	AC_SUBST(ldnsdir)
])

AC_CHECK_LIB(ldns, ldns_rr_new,, [AC_MSG_ERROR([Can't find ldns library])])

AC_DEFINE_UNQUOTED([MAXSYSLOGMSGLEN], [512], [Define to the maximum message length to pass to syslog.])

AH_BOTTOM([
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_ATTR_FORMAT
#  define ATTR_FORMAT(archetype, string_index, first_to_check) \
    __attribute__ ((format (archetype, string_index, first_to_check)))
#else /* !HAVE_ATTR_FORMAT */
#  define ATTR_FORMAT(archetype, string_index, first_to_check) /* empty */
#endif /* !HAVE_ATTR_FORMAT */
#if defined(DOXYGEN)
#  define ATTR_UNUSED(x)  x
#elif defined(__cplusplus)
#  define ATTR_UNUSED(x)
#elif defined(HAVE_ATTR_UNUSED)
#  define ATTR_UNUSED(x)  x __attribute__((unused))
#else /* !HAVE_ATTR_UNUSED */
#  define ATTR_UNUSED(x)  x
#endif /* !HAVE_ATTR_UNUSED */
#ifndef HAVE_FSEEKO
#define fseeko fseek
#define ftello ftell
#endif /* HAVE_FSEEKO */

#include "ldns/ldns.h"

/** default port to listen for queries, passed to getaddrinfo */
#define UNBOUND_DNS_PORT "53"
])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT
