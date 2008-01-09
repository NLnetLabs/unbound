#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)

AC_INIT(unbound, 0.7.3, unbound-bugs@nlnetlabs.nl, unbound)

CFLAGS=
AC_AIX

dnl
dnl By default set $sysconfdir to /etc and $localstatedir to /var
dnl
case "$prefix" in
        NONE)
                case "$sysconfdir" in
                        '${prefix}/etc')
                                sysconfdir=/etc
                        ;;
                esac
                case "$localstatedir" in
                        '${prefix}/var')
                                localstatedir=/var
                        ;;
                esac
        ;;
esac

#
# Determine configuration file
ub_conf_file=${sysconfdir}/unbound/unbound.conf
AC_ARG_WITH([conf_file],
        AC_HELP_STRING([--with-conf-file=path], [Pathname to the Unbound configuration file]),
	[ub_conf_file=$withval])
AC_SUBST(ub_conf_file)
# the eval is to evaluate shell expansion twice, once
# for $nsd_conf_file and once for the ${prefix} within it.
AC_DEFINE_UNQUOTED(CONFIGFILE, ["`eval echo $ub_conf_file`"], [Pathname to the Unbound configuration file])

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

CHECK_COMPILER_FLAG_NEEDED(-std=c99 -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_ALL_SOURCE,
[
#include "confdefs.h"
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
], [CFLAGS="$CFLAGS -std=c99 -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_ALL_SOURCE"])

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
#include "confdefs.h"
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
#include "confdefs.h"
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
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

# skip these tests, we do not need them.
AC_DEFUN([AC_PROG_F77], [:])
AC_DEFUN([AC_PROG_FC], [:])
AC_DEFUN([AC_PROG_CXX], [:])
AC_DEFUN([AC_PROG_CXXCPP], [:])
AC_DEFUN([AC_PROG_OBJC], [:])
AC_DEFUN([AC_PROG_OBJCCPP], [:])
AC_DEFUN([AC_LIBTOOL_CXX], [:])
AC_DEFUN([AC_LIBTOOL_F77], [:])
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
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdarg.h stdbool.h netinet/in.h sys/param.h sys/socket.h sys/uio.h arpa/inet.h syslog.h],,, [AC_INCLUDES_DEFAULT])

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

# Check the type of struct msghdr msg_iovlen.
AC_MSG_CHECKING([sign of struct msghdr.msg_iovlen])
AC_RUN_IFELSE([
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
int main()
{
	struct msghdr h;
	h.msg_iovlen = -1;
	if(10/h.msg_iovlen == 0) return 0; /* unsigned */
	return 1; /* signed */
}
], [ 
# if compiled and exit status 0
cv_msgiovlen_type="unsigned" ], [ 
# not compiled or bad exit status
cv_msgiovlen_type="int" ], 
# when cross compiling
[ cv_msgiovlen_type="int" ])
AC_MSG_RESULT($cv_msgiovlen_type)
AC_DEFINE_UNQUOTED([TYPE_MSGIOVLEN], $cv_msgiovlen_type, [Define to 'int' or type of struct msghdr.msg_iovlen.])

# Check for yyunput
AC_DEFUN([CHECK_YYUNPUT],
[AC_MSG_CHECKING(whether lex can not generate yyunput)
AC_CACHE_VAL(ac_cv_c_yy_unput,
[ac_cv_c_yy_unput=no
if $LEX --nounput -h 2>&1 | grep "do not generate" 2>&1 >/dev/null; then
   ac_cv_c_yy_unput="yes"
else
   ac_cv_c_yy_unput="no"
fi
])

AC_MSG_RESULT($ac_cv_c_yy_unput)
if test $ac_cv_c_yy_unput = yes; then
	LEX="$LEX --nounput"
fi
])dnl

CHECK_YYUNPUT

# add option to disable the evil rpath
dnl Check whether to use rpath or not
AC_ARG_ENABLE(rpath,
        [  --disable-rpath         disable hardcoded rpath (default=enabled)],
	enable_rpath=$enableval, enable_rpath=yes)

# Checks for libraries.
AC_ARG_WITH(ssl, AC_HELP_STRING([--with-ssl=pathname],
                                    [enable SSL (will check /usr/local/ssl
                            /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /opt/local /usr/sfw /usr)]),[
        ],[
            withval="yes"
        ])
    if test x_$withval != x_no; then
        AC_MSG_CHECKING(for SSL)
        if test x_$withval = x_ -o x_$withval = x_yes; then
            withval="/usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /opt/local /usr/sfw /usr"
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
	    if test "x$enable_rpath" = xyes; then
	        RUNTIME_PATH="$RUNTIME_PATH -R$ssldir/lib"
	    fi
            AC_CHECK_LIB(crypto, HMAC_CTX_init,, [
                    AC_MSG_ERROR([OpenSSL found in $ssldir, but version 0.9.7 or higher is required])
                ])
        fi
        AC_SUBST(HAVE_SSL)
	AC_SUBST(RUNTIME_PATH)
    fi
AC_CHECK_HEADERS([openssl/ssl.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/err.h],,, [AC_INCLUDES_DEFAULT])

# check for thread library.
AC_ARG_WITH(pthreads, AC_HELP_STRING([--with-pthreads], 
 [use pthreads library, or --without-pthreads to disable threading support.]), 
 [ ],[ withval="yes" ])
ub_have_pthreads=no
if test x_$withval != x_no; then
	sinclude(acx_pthread.m4)
	ACX_PTHREAD([
		AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.])
		LIBS="$PTHREAD_LIBS $LIBS"
		CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
		CC="$PTHREAD_CC"
		ub_have_pthreads=yes
		AC_CHECK_TYPES([pthread_spinlock_t, pthread_rwlock_t],,,[#include <pthread.h>])
		])
fi

# check solaris thread library 
AC_ARG_WITH(solaris-threads, AC_HELP_STRING([--with-solaris-threads], 
	[use solaris native thread library.]), [ ],[ withval="no" ])
if test x_$withval != x_no; then
	if test x_$ub_have_pthreads != x_no; then
	    AC_WARN([Have pthreads already, ignoring --with-solaris-threads])
	else
	AC_SEARCH_LIBS(thr_create, [thread],
	[
    		AC_DEFINE(HAVE_SOLARIS_THREADS, 1, [Using Solaris threads])

		CHECK_COMPILER_FLAG(mt, [CFLAGS="$CFLAGS -mt"],
			[CFLAGS="$CFLAGS -D_REENTRANT"])
	] , [ 
		AC_ERROR([no solaris threads found.]) 
	])
	fi
fi

# check for library used by libevent after 1.3c
AC_CHECK_LIB(rt, clock_gettime)
# check for libevent
AC_ARG_WITH(libevent, AC_HELP_STRING([--with-libevent=pathname],
    [set path to libevent  (will check /usr/local /opt/local /usr/lib /usr/pkg /usr/sfw /usr)]),
    [ ],[ withval="yes" ])
if test x_$withval != x_no; then
        AC_MSG_CHECKING(for libevent)
        if test x_$withval = x_ -o x_$withval = x_yes; then
            withval="/usr/local /opt/local /usr/lib /usr/pkg /usr/sfw /usr";
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
		else
            		AC_MSG_ERROR([Cannot find the libevent library in $withval
You can restart ./configure --with-libevent=no to use a builtin alternative.
Please note that this alternative is not as capable as libevent when using
many outgoing ports.  ])
		fi
        else
            AC_MSG_RESULT(found in $thedir)
            LDFLAGS="$LDFLAGS -L$thedir/lib";
	    if test "x$enable_rpath" = xyes; then
	        RUNTIME_PATH="$RUNTIME_PATH -R$thedir/lib"
	    fi
        fi
	AC_SUBST(RUNTIME_PATH)
	AC_SEARCH_LIBS(event_set, [event])
	AC_CHECK_HEADERS([event.h],,, [AC_INCLUDES_DEFAULT])
	AC_CHECK_FUNCS([event_base_free]) # only in libevent 1.2 and later
	AC_CHECK_FUNCS([event_base_once]) # only in libevent 1.4? and later
else
	AC_DEFINE(USE_MINI_EVENT, 1, [Define if you want to use internal select based events])
fi

# set static linking if requested
AC_SUBST(staticexe)
staticexe=""
AC_ARG_ENABLE(staticexe, AC_HELP_STRING([--enable-static-exe],
	[ enable to compile executables statically against event, ldns libs, for debug purposes ]), 
	, )
if test x_$enable_static_exe = x_yes; then
	staticexe="-static"
fi

# set lock checking if requested
AC_ARG_ENABLE(lock_checks, AC_HELP_STRING([--enable-lock-checks],
	[ enable to check lock and unlock calls, for debug purposes ]), 
	, )
if test x_$enable_lock_checks = x_yes; then
	AC_DEFINE(ENABLE_LOCK_CHECKS, 1, [Define if you want to use debug lock checking (slow).])
fi

# set memory allocation checking if requested
AC_ARG_ENABLE(alloc-checks, AC_HELP_STRING([--enable-alloc-checks],
	[ enable to check memory allocation, for debug purposes ]), 
	, )
if test x_$enable_alloc_checks = x_yes; then
	AC_DEFINE(UNBOUND_ALLOC_STATS, 1, [use statistics for allocs and frees, for debug use])
else
	AC_FUNC_MALLOC
fi

# check to see if libraries are needed for these functions.
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, inet_pton)

AC_FUNC_CHOWN
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE

# check this after all other compilation checks, since the linking of the lib
# may break checks after this.
AC_ARG_WITH(ldns,
        AC_HELP_STRING([--with-ldns=PATH        specify prefix of path of ldns library to use])
	, [ specialldnsdir="$withval"
	CPPFLAGS="$CPPFLAGS -I$withval/include"
	LDFLAGS="$LDFLAGS -L$withval -L$withval/lib -lldns"
	ldnsdir="$withval"
	AC_SUBST(ldnsdir)
])

AC_CHECK_LIB(ldns, ldns_rr_new, [
dnl ldns was found, check compat functions
AC_REPLACE_FUNCS(inet_aton)
AC_REPLACE_FUNCS(inet_pton)
AC_REPLACE_FUNCS(inet_ntop)
AC_REPLACE_FUNCS(snprintf)
AC_REPLACE_FUNCS(strlcpy)
AC_REPLACE_FUNCS(memmove)

AC_CHECK_FUNCS([getaddrinfo])
if test $ac_cv_func_getaddrinfo = no; then
AC_LIBOBJ([fake-rfc2553])
fi

] , [
dnl use the builtin ldns-src.tar.gz file to build ldns.
if test ! -f $srcdir/ldns-src.tar.gz; then
	AC_MSG_ERROR([No ldns library found and no ldns-src.tar.gz, use --with-ldns=path.])
fi
echo "****************************************************************"
echo "***                                                          ***"
echo "*** ldns library not found (--with-ldns= to specify by hand) ***"
echo "*** Building ldns library from package included in this one. ***"
echo "***                                                          ***"
echo "****************************************************************"
if test -d ldns-src; then rm -rf ldns-src; fi
mkdir ldns-src; cp $srcdir/ldns-src.tar.gz ldns-src/ldns-src.tar.gz; (cd ldns-src; gzip -cd ldns-src.tar.gz | tar xf -); rm -f ldns-src/ldns-src.tar.gz
ldnsdir=`ls -d ldns-src/ldns*`
AC_MSG_NOTICE([Configure $ldnsdir])
echo "(cd $ldnsdir; ./configure)"
(cd $ldnsdir; ./configure)
AC_MSG_NOTICE([Finished $ldnsdir configure])
CPPFLAGS="$CPPFLAGS -I$ldnsdir/include"
LDFLAGS="$LDFLAGS $ldnsdir/lib/*.o"
AC_SUBST(ldnsdir)
dnl use ldns compat functions
AC_DEFINE(HAVE_INET_ATON)
AC_DEFINE(HAVE_INET_PTON)
AC_DEFINE(HAVE_INET_NTOP)
AC_DEFINE(HAVE_SNPRINTF)
AC_DEFINE(HAVE_STRLCPY)
AC_DEFINE(HAVE_MEMMOVE)
AC_DEFINE(HAVE_GETADDRINFO)
])

AC_DEFINE_UNQUOTED([MAXSYSLOGMSGLEN], [5120], [Define to the maximum message length to pass to syslog.])

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

#include <errno.h>

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
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

#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
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
#if defined(HAVE_EVENT_H) && !defined(HAVE_EVENT_BASE_ONCE) && (defined(HAVE_PTHREAD) || defined(HAVE_SOLARIS_THREADS))
   /* using version of libevent that is not threadsafe. */
#  define LIBEVENT_SIGNAL_PROBLEM 1
#endif

#ifndef RAND_MAX
#define RAND_MAX	2147483647
#endif

#ifndef IPV6_MIN_MTU
#define IPV6_MIN_MTU 1280
#endif /* IPV6_MIN_MTU */

#ifndef CHECKED_INET6
#  define CHECKED_INET6
#  ifdef AF_INET6
#    define INET6
#  else
#    define AF_INET6        28
#  endif
#endif /* CHECKED_INET6 */

/* maximum nesting of included files */
#define MAXINCLUDES 10

#ifndef HAVE_SNPRINTF
#include <stdarg.h>
int snprintf (char *str, size_t count, const char *fmt, ...);
int vsnprintf (char *str, size_t count, const char *fmt, va_list arg);
#endif /* HAVE_SNPRINTF */
#ifndef HAVE_INET_PTON
int inet_pton(int af, const char* src, void* dst);
#endif /* HAVE_INET_PTON */
#ifndef HAVE_INET_NTOP
const char *inet_ntop(int af, const void *src, char *dst, size_t size);
#endif
#ifndef HAVE_INET_ATON
int inet_aton(const char *cp, struct in_addr *addr);
#endif
#ifndef HAVE_MEMMOVE
void *memmove(void *dest, const void *src, size_t n);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif
#ifndef HAVE_GETADDRINFO
struct sockaddr_storage;
#include "compat/fake-rfc2553.h"
#endif

#include "ldns/ldns.h"

#ifdef UNBOUND_ALLOC_STATS
#  define malloc(s) unbound_stat_malloc_log(s, __FILE__, __LINE__, __func__)
#  define calloc(n,s) unbound_stat_calloc_log(n, s, __FILE__, __LINE__, __func__)
#  define free(p) unbound_stat_free_log(p, __FILE__, __LINE__, __func__)
#  define realloc(p,s) unbound_stat_realloc_log(p, s, __FILE__, __LINE__, __func__)
void *unbound_stat_malloc(size_t size);
void *unbound_stat_calloc(size_t nmemb, size_t size);
void unbound_stat_free(void *ptr);
void *unbound_stat_realloc(void *ptr, size_t size);
void *unbound_stat_malloc_log(size_t size, const char* file, int line,
	const char* func);
void *unbound_stat_calloc_log(size_t nmemb, size_t size, const char* file,
	int line, const char* func);
void unbound_stat_free_log(void *ptr, const char* file, int line,
	const char* func);
void *unbound_stat_realloc_log(void *ptr, size_t size, const char* file,
	int line, const char* func);
#endif /* UNBOUND_ALLOC_STATS */

/** default port for DNS traffic. */
#define UNBOUND_DNS_PORT 53
])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT
