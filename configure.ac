#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.56)

AC_INIT(unbound,1.1.0, unbound-bugs@nlnetlabs.nl, unbound)

LIBUNBOUND_CURRENT=0
LIBUNBOUND_REVISION=15
LIBUNBOUND_AGE=0
# 1.0.0 had 0:12:0
# 1.0.1 had 0:13:0
# 1.0.2 had 0:14:0
# 1.1.0 had 0:15:0
# 1.1.1 had 0:16:0

#   Current  -- the number of the binary API that we're implementing
#   Revision -- which iteration of the implementation of the binary
#               API are we supplying?
#   Age      -- How many previous binary API versions do we also
#               support?
#
# If we release a new version that does not change the binary API,
# increment Revision.
#
# If we release a new version that changes the binary API, but does
# not break programs compiled against the old binary API, increment
# Current and Age.  Set Revision to 0, since this is the first
# implementation of the new API.
#
# Otherwise, we're changing the binary API and breaking bakward
# compatibility with old binaries.  Increment Current.  Set Age to 0,
# since we're backward compatible with no previous APIs.  Set Revision
# to 0 too.
AC_SUBST(LIBUNBOUND_CURRENT)
AC_SUBST(LIBUNBOUND_REVISION)
AC_SUBST(LIBUNBOUND_AGE)

CFLAGS="$CFLAGS"
AC_AIX

dnl
dnl By default set prefix to /usr/local
dnl
case "$prefix" in
        NONE)
		prefix="/usr/local"
        ;;
esac

#
# Determine configuration file
# the eval is to evaluate shell expansion twice
ub_conf_file=`eval echo "${sysconfdir}/unbound/unbound.conf"`
AC_ARG_WITH([conf_file],
        AC_HELP_STRING([--with-conf-file=path], 
	[Pathname to the Unbound configuration file]),
	[ub_conf_file="$withval"])
AC_SUBST(ub_conf_file)
AC_DEFINE_UNQUOTED(CONFIGFILE, ["$ub_conf_file"], [Pathname to the Unbound configuration file])

# Determine run, chroot directory and pidfile locations
AC_ARG_WITH(run-dir, 
    AC_HELP_STRING([--with-run-dir=path], 
    [set default directory to chdir to (by default dir part of cfg file)]), 
    UNBOUND_RUN_DIR="$withval", 
    UNBOUND_RUN_DIR=`dirname "$ub_conf_file"`)
AC_SUBST(UNBOUND_RUN_DIR)
AC_DEFINE_UNQUOTED(RUN_DIR, ["$UNBOUND_RUN_DIR"], [Directory to chdir to])

AC_ARG_WITH(chroot-dir, 
    AC_HELP_STRING([--with-chroot-dir=path], 
    [set default directory to chroot to (by default same as run-dir)]), 
    UNBOUND_CHROOT_DIR="$withval", 
    UNBOUND_CHROOT_DIR="$UNBOUND_RUN_DIR")
AC_SUBST(UNBOUND_CHROOT_DIR)
AC_DEFINE_UNQUOTED(CHROOT_DIR, ["$UNBOUND_CHROOT_DIR"], [Directory to chroot to])

AC_ARG_WITH(pidfile, 
    AC_HELP_STRING([--with-pidfile=filename], 
    [set default pathname to unbound pidfile (default run-dir/unbound.pid)]), 
    UNBOUND_PIDFILE="$withval", 
    UNBOUND_PIDFILE="$UNBOUND_RUN_DIR/unbound.pid")
AC_SUBST(UNBOUND_PIDFILE)
AC_DEFINE_UNQUOTED(PIDFILE, ["$UNBOUND_PIDFILE"], [default pidfile location])

AC_ARG_WITH(username, 
    AC_HELP_STRING([--with-username=user], 
    [set default user that unbound changes to (default user is unbound)]), 
    UNBOUND_USERNAME="$withval", 
    UNBOUND_USERNAME="unbound")
AC_SUBST(UNBOUND_USERNAME)
AC_DEFINE_UNQUOTED(UB_USERNAME, ["$UNBOUND_USERNAME"], [default username])

AC_DEFINE(WINVER, 0x0502, [the version of the windows API enabled])

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

# MinGW32 tests
AC_CHECK_HEADERS([winsock2.h ws2tcpip.h],,, [AC_INCLUDES_DEFAULT])
# end mingw32 tests

CHECK_COMPILER_FLAG_NEEDED(-std=c99 -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -D_ALL_SOURCE,
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
], [CFLAGS="$CFLAGS -std=c99 -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -D_ALL_SOURCE"])

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

CHECK_COMPILER_FLAG_NEEDED(-D_GNU_SOURCE,
[
#include <netinet/in.h>

int test() {
        struct in6_pktinfo inf;
	int a = (int)sizeof(inf);
        return a;
}
], [CFLAGS="$CFLAGS -D_GNU_SOURCE"])

# check again for GNU_SOURCE for setresgid. May fail if setresgid
# is not available at all. -D_FRSRESGID is to make this check unique.
# otherwise we would get the previous cached result.
CHECK_COMPILER_FLAG_NEEDED(-D_GNU_SOURCE -D_FRSRESGID,
[
#include <unistd.h>

int test() {
	int a = setresgid(0,0,0);
	a = setresuid(0,0,0);
        return a;
}
], [CFLAGS="$CFLAGS -D_GNU_SOURCE"])

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
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Enable debug warnings, asserts, makefile-dependencies]))
debug_enabled="$enable_debug"
AC_SUBST(debug_enabled)
case "$enable_debug" in
        yes)
		CHECK_COMPILER_FLAG(W, [CFLAGS="$CFLAGS -W"])
		CHECK_COMPILER_FLAG(Wall, [CFLAGS="$CFLAGS -Wall"])
		CHECK_COMPILER_FLAG(Wextra, [CFLAGS="$CFLAGS -Wextra"])
		CHECK_COMPILER_FLAG(Wdeclaration-after-statement, [CFLAGS="$CFLAGS -Wdeclaration-after-statement"])
		AC_DEFINE([UNBOUND_DEBUG], [], [define this to enable debug checks.])
		;;
	no|*)
		# nothing to do.
		;;
esac

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
	CPPFLAGS="$CPPFLAGS -I$srcdir"
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
# always use ./libtool unless override from commandline (libtool=mylibtool)
if test -z "$libtool"; then
	libtool="./libtool"
fi
AC_SUBST(libtool)
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
AC_CHECK_HEADERS([stdarg.h stdbool.h netinet/in.h sys/param.h sys/socket.h sys/uio.h sys/resource.h arpa/inet.h syslog.h netdb.h sys/wait.h pwd.h],,, [AC_INCLUDES_DEFAULT])

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

AC_CHECK_TYPE(rlim_t, , 
	[AC_DEFINE([rlim_t], [unsigned long], [Define to 'int' if not defined])], [
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
])
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

# add option to disable the evil rpath
dnl Check whether to use rpath or not
AC_ARG_ENABLE(rpath,
        [  --disable-rpath         disable hardcoded rpath (default=enabled)],
	enable_rpath=$enableval, enable_rpath=yes)
if test "x$enable_rpath" = xno; then
	AC_MSG_RESULT([Fixing libtool for -rpath problems.])
	sed < libtool > libtool-2 \
	's/^hardcode_libdir_flag_spec.*$'/'hardcode_libdir_flag_spec=" -D__LIBTOOL_RPATH_SED__ "/'
	mv libtool-2 libtool
	chmod 755 libtool
	libtool="./libtool"
fi

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
                found_ssl="yes"
                AC_DEFINE_UNQUOTED([HAVE_SSL], [], [Define if you have the SSL libraries installed.])
                CPPFLAGS="$CPPFLAGS -I$ssldir/include"
                break;
            fi
        done
        if test x_$found_ssl != x_yes; then
            AC_MSG_ERROR(Cannot find the SSL libraries in $withval)
        else
            AC_MSG_RESULT(found in $ssldir)
            HAVE_SSL=yes
            LDFLAGS="$LDFLAGS -L$ssldir/lib"
	    if test "x$enable_rpath" = xyes; then
	        RUNTIME_PATH="$RUNTIME_PATH -R$ssldir/lib"
	    fi
	
	    AC_MSG_CHECKING([for HMAC_CTX_init in -lcrypto])
	    LIBS="$LIBS -lcrypto"
	    AC_TRY_LINK(, [
		int HMAC_CTX_init(void);
		(void)HMAC_CTX_init();
	      ], [
		AC_MSG_RESULT(yes)
		AC_DEFINE([HAVE_HMAC_CTX_INIT], 1, 
			[If you have HMAC_CTX_init])
	      ], [
		AC_MSG_RESULT(no)
	    	# check if -lwsock32 or -lgdi32 are needed.	
		LIBS="$LIBS -lgdi32"
		AC_MSG_CHECKING([if -lcrypto needs -lgdi32])
		AC_TRY_LINK([], [
		    int HMAC_CTX_init(void);
		    (void)HMAC_CTX_init();
		  ],[
		    AC_DEFINE([HAVE_HMAC_CTX_INIT], 1, 
			[If you have HMAC_CTX_init])
		    AC_MSG_RESULT(yes) 
		  ],[
		    AC_MSG_RESULT(no)
                    AC_MSG_ERROR([OpenSSL found in $ssldir, but version 0.9.7 or higher is required])
		])
            ])
        fi
        AC_SUBST(HAVE_SSL)
	AC_SUBST(RUNTIME_PATH)
    fi
AC_CHECK_HEADERS([openssl/ssl.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/err.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/rand.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_FUNCS([EVP_sha1 EVP_sha256 EVP_sha512])

# check if libssl needs libdl
BAKLIBS="$LIBS"
LIBS="-lssl $LIBS"
AC_MSG_CHECKING([if libssl needs libdl])
AC_TRY_LINK_FUNC([SSL_CTX_new], [
	AC_MSG_RESULT([no])
	LIBS="$BAKLIBS"
] , [
	AC_MSG_RESULT([yes])
	LIBS="$BAKLIBS"
	AC_SEARCH_LIBS([dlopen], [dl])
])

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
ub_have_sol_threads=no
if test x_$withval != x_no; then
	if test x_$ub_have_pthreads != x_no; then
	    AC_WARN([Have pthreads already, ignoring --with-solaris-threads])
	else
	AC_SEARCH_LIBS(thr_create, [thread],
	[
    		AC_DEFINE(HAVE_SOLARIS_THREADS, 1, [Using Solaris threads])

		CHECK_COMPILER_FLAG(mt, [CFLAGS="$CFLAGS -mt"],
			[CFLAGS="$CFLAGS -D_REENTRANT"])
		ub_have_sol_threads=yes
	] , [ 
		AC_ERROR([no solaris threads found.]) 
	])
	fi
fi

# check windows threads
if test x_$ub_have_pthreads = x_no -a x_$ub_have_sol_threads = x_no; then
	AC_CHECK_HEADERS([windows.h],,, [AC_INCLUDES_DEFAULT])
	AC_MSG_CHECKING([for CreateThread])
	AC_TRY_COMPILE([
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
], [
	HANDLE t = CreateThread(NULL, 0, NULL, NULL, 0, NULL);
],
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_WINDOWS_THREADS, 1, [Using Windows threads])
,	
	AC_MSG_RESULT(no)
)
fi

# check to see if libraries are needed for these functions.
AC_SEARCH_LIBS([inet_pton], [nsl])
AC_SEARCH_LIBS([socket], [socket])

# check for libevent
AC_ARG_WITH(libevent, AC_HELP_STRING([--with-libevent=pathname],
    [use libevent (will check /usr/local /opt/local /usr/lib /usr/pkg /usr/sfw /usr  or you can specify an explicit path). Slower, but allows use of large outgoing port ranges.]),
    [ ],[ withval="no" ])
if test x_$withval = x_yes -o x_$withval != x_no; then
        AC_MSG_CHECKING(for libevent)
        if test x_$withval = x_ -o x_$withval = x_yes; then
            withval="/usr/local /opt/local /usr/lib /usr/pkg /usr/sfw /usr"
        fi
        for dir in $withval; do
            thedir="$dir"
            if test -f "$dir/include/event.h"; then
                found_libevent="yes"
                CPPFLAGS="$CPPFLAGS -I$thedir/include"
                break;
            fi
        done
        if test x_$found_libevent != x_yes; then
		if test -f "$dir/event.h" -a \( -f "$dir/libevent.la" -o -f "$dir/libev.la" \) ; then
			# libevent source directory
            		AC_MSG_RESULT(found in $thedir)
                	CPPFLAGS="$CPPFLAGS -I$thedir -I$thedir/include"
			BAK_LDFLAGS="$LDFLAGS"
			# remove evdns from linking
			mkdir build >/dev/null 2>&1
			mkdir build/libevent >/dev/null 2>&1
			mkdir build/libevent/.libs >/dev/null 2>&1
			ev_files_o=`ls $thedir/*.o | grep -v evdns\.o`
			ev_files_lo=`ls $thedir/*.lo | grep -v evdns\.lo`
			ev_files_libso=`ls $thedir/.libs/*.o | grep -v evdns\.o`
			cp $ev_files_o build/libevent
			cp $ev_files_lo build/libevent
			cp $ev_files_libso build/libevent/.libs
            		LATE_LDFLAGS="build/libevent/*.lo -lm"
			LDFLAGS="build/libevent/*.o $LDFLAGS -lm"
		else
            		AC_MSG_ERROR([Cannot find the libevent library in $withval
You can restart ./configure --with-libevent=no to use a builtin alternative.
Please note that this alternative is not as capable as libevent when using
large outgoing port ranges.  ])
		fi
        else
            AC_MSG_RESULT(found in $thedir)
            LDFLAGS="$LDFLAGS -L$thedir/lib"
	    if test "x$enable_rpath" = xyes; then
	        RUNTIME_PATH="$RUNTIME_PATH -R$thedir/lib"
	    fi
        fi
	AC_SUBST(RUNTIME_PATH)
	# check for library used by libevent after 1.3c
	AC_SEARCH_LIBS([clock_gettime], [rt])

	AC_SEARCH_LIBS(event_set, [event])
	AC_CHECK_HEADERS([event.h],,, [AC_INCLUDES_DEFAULT])
	AC_CHECK_FUNCS([event_base_free]) # only in libevent 1.2 and later
	AC_CHECK_FUNCS([event_base_once]) # only in libevent 1.4? and later
	AC_CHECK_FUNCS([event_base_get_method]) # only in libevent 1.4.3 and later
	if test -n "$BAK_LDFLAGS"; then
		LDFLAGS="$BAK_LDFLAGS"
	fi
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
	CHECKLOCK_SRC="testcode/checklocks.c"
	AC_SUBST(CHECKLOCK_SRC)
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

AC_FUNC_CHOWN
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE
dnl try to see if an additional _LARGEFILE_SOURCE 1 is needed to get fseeko
CHECK_COMPILER_FLAG_NEEDED(-D_LARGEFILE_SOURCE=1,
[
#include <stdio.h>
int test() {
        int a = fseeko(stdin, 0, 0);
        return a;
}
], [CFLAGS="$CFLAGS -D_LARGEFILE_SOURCE=1"])

AC_DEFUN([AC_CHECK_GETADDRINFO_WITH_INCLUDES],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(for getaddrinfo)
ac_cv_func_getaddrinfo=no
AC_LINK_IFELSE(
[
#ifdef __cplusplus
extern "C"
{
#endif
char* getaddrinfo();
char* (*f) () = getaddrinfo;
#ifdef __cplusplus
}
#endif
int main() {
        ;
        return 0;
}
],
dnl this case on linux, solaris, bsd
[ac_cv_func_getaddrinfo="yes"],
dnl no quick getaddrinfo, try mingw32 and winsock2 library.
ORIGLIBS="$LIBS"
LIBS="$LIBS -lws2_32"
AC_LINK_IFELSE(
AC_LANG_PROGRAM(
[
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
],
[
        (void)getaddrinfo(NULL, NULL, NULL, NULL);
]
),
[
ac_cv_func_getaddrinfo="yes"
dnl already: LIBS="$LIBS -lws2_32"
AC_DEFINE(USE_WINSOCK, 1, [Whether the windows socket API is used])
],
[
ac_cv_func_getaddrinfo="no"
LIBS="$ORIGLIBS"
])
)

AC_MSG_RESULT($ac_cv_func_getaddrinfo)
if test $ac_cv_func_getaddrinfo = yes; then
  AC_DEFINE(HAVE_GETADDRINFO, 1, [Whether getaddrinfo is available])
fi
])dnl

AC_CHECK_GETADDRINFO_WITH_INCLUDES
if test $ac_cv_func_getaddrinfo = no; then
AC_LIBOBJ([fake-rfc2553])
fi
AC_CHECK_FUNCS([tzset sigprocmask fcntl getpwnam getrlimit setsid sbrk chroot kill sleep usleep random srandom recvmsg sendmsg writev setresuid setreuid setresgid setregid])

# check if setreuid en setregid fail, on MacOSX10.4(darwin8).
if echo $build_os | grep darwin8 > /dev/null; then
	AC_DEFINE(DARWIN_BROKEN_SETREUID, 1, [Define this if on macOSX10.4-darwin8 and setreuid and setregid do not work])
fi

# check if select and nonblocking sockets actually work.
AC_MSG_CHECKING([if nonblocking sockets work])
AC_RUN_IFELSE(AC_LANG_PROGRAM([
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
],[[
	int port;
	int sfd, cfd;
	int num = 10;
	int i, p;
	struct sockaddr_in a;
	/* test if select and nonblocking reads work well together */
	/* open port.
	   fork child to send 10 messages.
	   select to read.
	   then try to nonblocking read the 10 messages
	   then, nonblocking read must give EAGAIN
	*/

	port = 12345 + (time(0)%32);
	sfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sfd == -1) {
		perror("socket");
		return 1;
	}
	memset(&a, 0, sizeof(a));
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	a.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(sfd, (struct sockaddr*)&a, sizeof(a)) < 0) {
		perror("bind");
		return 1;
	}
	if(fcntl(sfd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl");
		return 1;
	}

	cfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(cfd == -1) {
		perror("client socket");
		return 1;
	}
	a.sin_port = 0;
	if(bind(cfd, (struct sockaddr*)&a, sizeof(a)) < 0) {
		perror("client bind");
		return 1;
	}
	a.sin_port = htons(port);

	/* no handler, causes exit in 10 seconds */
	alarm(10);

	/* send and receive on the socket */
	if((p=fork()) == 0) {
		for(i=0; i<num; i++) {
			if(sendto(cfd, &i, sizeof(i), 0, 
				(struct sockaddr*)&a, sizeof(a)) < 0) {
				perror("sendto");
				return 1;
			}
		}
	} else {
		/* parent */
		fd_set rset;
		int x;
		if(p == -1) {
			perror("fork");
			return 1;
		}
		FD_ZERO(&rset);
		FD_SET(sfd, &rset);
		if(select(sfd+1, &rset, NULL, NULL, NULL) < 1) {
			perror("select");
			return 1;
		}
		i = 0;
		while(i < num) {
			if(recv(sfd, &x, sizeof(x), 0) != sizeof(x)) {
				if(errno == EAGAIN)
					continue;
				perror("recv");
				return 1;
			}
			i++;
		}
		/* now we want to get EAGAIN: nonblocking goodness */
		errno = 0;
		recv(sfd, &x, sizeof(x), 0);
		if(errno != EAGAIN) {
			perror("trying to recv again");
			return 1;
		}
		/* EAGAIN encountered */
	}

	close(sfd);
	close(cfd);
]]), [
	AC_MSG_RESULT([yes])
], [
	AC_MSG_RESULT([no])
	AC_DEFINE([NONBLOCKING_IS_BROKEN], 1, [Define if the network stack does not fully support nonblocking io (causes lower performance).])
], [
	AC_MSG_RESULT([crosscompile(yes)])
])

# check mkdir
AC_MSG_CHECKING([whether mkdir has one arg])
AC_TRY_COMPILE([
#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
], [
	(void)mkdir("directory");
],
AC_MSG_RESULT(yes)
AC_DEFINE(MKDIR_HAS_ONE_ARG, 1, [Define if mkdir has one argument.])
,
AC_MSG_RESULT(no)
)

# check ioctlsocket
AC_MSG_CHECKING(for ioctlsocket)
AC_LINK_IFELSE(AC_LANG_PROGRAM([
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
], [
	(void)ioctlsocket(0, 0, NULL);
]), [
AC_MSG_RESULT(yes)
AC_DEFINE(HAVE_IOCTLSOCKET, 1, [if the function 'ioctlsocket' is available])
],[AC_MSG_RESULT(no)])

AC_REPLACE_FUNCS(inet_aton)
AC_REPLACE_FUNCS(inet_pton)
AC_REPLACE_FUNCS(inet_ntop)
AC_REPLACE_FUNCS(snprintf)
AC_REPLACE_FUNCS(strlcpy)
AC_REPLACE_FUNCS(memmove)
AC_REPLACE_FUNCS(gmtime_r)

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

AC_CHECK_LIB(ldns, ldns_buffer_copy, [
dnl ldns was found, check compat functions
AC_CHECK_LIB(ldns, [ldns_rr_free])
AC_CHECK_FUNC([ldns_b32_ntop_extended_hex],, [
	AC_MSG_ERROR([ldns version too old, need >=1.4.0])])
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
if test -d $srcdir/ldns-src; then rm -rf $srcdir/ldns-src; fi
mkdir $srcdir/ldns-src; cp $srcdir/ldns-src.tar.gz $srcdir/ldns-src/ldns-src.tar.gz; (cd $srcdir/ldns-src; gzip -cd ldns-src.tar.gz | tar xf -); rm -f $srcdir/ldns-src/ldns-src.tar.gz; mv $srcdir/ldns-src $srcdir/ldns-srcb; mv $srcdir/ldns-srcb/ldns* $srcdir/ldns-src; rmdir $srcdir/ldns-srcb
ldnsdir="ldns-src"
AC_MSG_NOTICE([Configure $ldnsdir scheduled after config.status])
AC_CONFIG_SUBDIRS([ldns-src])
CPPFLAGS="-I$ldnsdir/include $CPPFLAGS"
LDFLAGS="$LDFLAGS $ldnsdir/*.lo"
AC_SUBST(ldnsdir)
])
LDFLAGS="$LATE_LDFLAGS $LDFLAGS"

AC_DEFINE_UNQUOTED([MAXSYSLOGMSGLEN], [5120], [Define to the maximum message length to pass to syslog.])

AH_BOTTOM([
#ifndef UNBOUND_DEBUG
#  define NDEBUG
#endif

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

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif

#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
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

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
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
#define snprintf snprintf_unbound
#define vsnprintf vsnprintf_unbound
#include <stdarg.h>
int snprintf (char *str, size_t count, const char *fmt, ...);
int vsnprintf (char *str, size_t count, const char *fmt, va_list arg);
#endif /* HAVE_SNPRINTF */
#ifndef HAVE_INET_PTON
#define inet_pton inet_pton_unbound
int inet_pton(int af, const char* src, void* dst);
#endif /* HAVE_INET_PTON */
#ifndef HAVE_INET_NTOP
#define inet_ntop inet_ntop_unbound
const char *inet_ntop(int af, const void *src, char *dst, size_t size);
#endif
#ifndef HAVE_INET_ATON
#define inet_aton inet_aton_unbound
int inet_aton(const char *cp, struct in_addr *addr);
#endif
#ifndef HAVE_MEMMOVE
#define memmove memmove_unbound
void *memmove(void *dest, const void *src, size_t n);
#endif
#ifndef HAVE_STRLCPY
#define strlcpy strlcpy_unbound
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif
#ifndef HAVE_GMTIME_R
#define gmtime_r gmtime_r_unbound
struct tm *gmtime_r(const time_t *timep, struct tm *result);
#endif
#ifndef HAVE_GETADDRINFO
struct sockaddr_storage;
#include "compat/fake-rfc2553.h"
#endif
#ifndef HAVE_SLEEP
#define sleep(x) Sleep((x)*1000) /* on win32 */
#endif /* HAVE_SLEEP */
#ifndef HAVE_USLEEP
#define usleep(x) Sleep((x)/1000 + 1) /* on win32 */
#endif /* HAVE_USLEEP */
#ifndef HAVE_RANDOM
#define random rand /* on win32, for tests only (bad random) */
#endif /* HAVE_RANDOM */
#ifndef HAVE_SRANDOM
#define srandom(x) srand(x) /* on win32, for tests only (bad random) */
#endif /* HAVE_SRANDOM */

/* detect if we need to cast to unsigned int for FD_SET to avoid warnings */
#ifdef HAVE_WINSOCK2_H
#define FD_SET_T (u_int)
#else
#define FD_SET_T 
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

AC_CONFIG_FILES([Makefile doc/example.conf doc/libunbound.3 doc/unbound.8 doc/unbound-checkconf.8 doc/unbound.conf.5 doc/unbound-control.8])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT
