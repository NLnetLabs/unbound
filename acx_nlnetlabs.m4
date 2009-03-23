# acx_nlnetlabs.m4 - common macros for configure checks
# Copyright 2009, Wouter Wijngaards, NLnet Labs.   
# BSD licensed.
#
# Automates some of the checking constructs.  Aims at portability for POSIX.
# Documentation for functions is below.
#

dnl Escape backslashes as \\, for C:\ paths, for the C preprocessor defines.
dnl for example, NLX_ESCAPE_BACKSLASH($from_var, to_var)
dnl $1: the text to change. 
dnl $2: the result.
AC_DEFUN(ACX_ESCAPE_BACKSLASH, $2="`echo $1 | sed -e 's/\\\\/\\\\\\\\/g'`" )

dnl Calculate comma separated windows-resource numbers from package version.
dnl Picks the first three(,0) or four numbers out of the name.
dnl $1: variable for the result
AC_DEFUN(ACX_RSRC_VERSION,
$1=[[`echo $PACKAGE_VERSION | sed -e 's/^[^0-9]*\([0-9]\)[^0-9]*\([0-9]\)[^0-9]*\([0-9]\)[^0-9]*\([0-9]\).*$/\1,\2,\3,\4/' -e 's/^[^0-9]*\([0-9]\)[^0-9]*\([0-9]\)[^0-9]*\([0-9]\)[^0-9]*$/\1,\2,\3,0/' `]]
)

dnl Routine to help check for compiler flags.
dnl Checks if the compiler will accept the flag.
dnl $1: the flag without a - in front, so g to check -g.
dnl $2: executed if yes
dnl $3: executed if no
AC_DEFUN(ACX_CHECK_COMPILER_FLAG, 
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
rm -f conftest conftest.o conftest.c
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

dnl setup flags for ACX_CHECK_COMPILER_FLAG_NEEDED
dnl ERRFLAG: result, compiler flag to turn warnings into errors
AC_DEFUN([ACX_CHECK_ERROR_FLAGS],
[
ACX_CHECK_COMPILER_FLAG(Werror, [ERRFLAG="-Werror"], [ERRFLAG="-errwarn"])
ACX_CHECK_COMPILER_FLAG(Wall, [ERRFLAG="$ERRFLAG -Wall"],
                        	[ERRFLAG="$ERRFLAG -errfmt"])
])

dnl Routine to help check for needed compiler flags.
dnl $1: flags for CC
dnl $2: the includes and code
dnl $3: if the given code only compiles with the flag, execute argument 3
dnl $4: if the given code compiles without the flag, execute argument 4
dnl $5: with and without flag the compile fails, execute argument 5.
AC_DEFUN([ACX_CHECK_COMPILER_FLAG_NEEDED],
[
AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([ACX_CHECK_ERROR_FLAGS])
AC_MSG_CHECKING(whether we need $1 as a flag for $CC)
cache=AS_TR_SH($1)
dnl cache=`echo $1 | sed 'y%.=/+- %___p__%'`
AC_CACHE_VAL(cv_prog_cc_flag_needed_$cache,
[
echo '$2' > conftest.c
echo 'void f(){}' >>conftest.c
if test -z "`$CC $CFLAGS $ERRFLAG -c conftest.c 2>&1`"; then
eval "cv_prog_cc_flag_needed_$cache=no"
else
[
if test -z "`$CC $CFLAGS $1 $ERRFLAG -c conftest.c 2>&1`"; then
eval "cv_prog_cc_flag_needed_$cache=yes"
else
eval "cv_prog_cc_flag_needed_$cache=fail"
#echo 'Test with flag fails too!'
#cat conftest.c
#echo "$CC $CFLAGS $1 $ERRFLAG -c conftest.c 2>&1"
#echo `$CC $CFLAGS $1 $ERRFLAG -c conftest.c 2>&1`
#exit 1
fi
]
fi
rm -f conftest conftest.c conftest.o
])
if eval "test \"`echo '$cv_prog_cc_flag_needed_'$cache`\" = yes"; then
AC_MSG_RESULT(yes)
:
$3
else
if eval "test \"`echo '$cv_prog_cc_flag_needed_'$cache`\" = no"; then
AC_MSG_RESULT(no)
#echo 'Test with flag is no!'
#cat conftest.c
#echo "$CC $CFLAGS $1 $ERRFLAG -c conftest.c 2>&1"
#echo `$CC $CFLAGS $1 $ERRFLAG -c conftest.c 2>&1`
#exit 1
:
$4
else
AC_MSG_RESULT(failed)
:
$5
fi
fi
])

dnl Check for CC dependency flag
dnl DEPFLAG: set to flag that generates dependencies.
AC_DEFUN(ACX_DEPFLAG,
[
AC_MSG_CHECKING([$CC dependency flag])
echo 'void f(){}' >conftest.c
if test "`$CC -MM conftest.c 2>&1`" = "conftest.o: conftest.c"; then
	DEPFLAG="-MM"
else 
  if test "`$CC -xM1 conftest.c 2>&1`" = "conftest.o: conftest.c"; then
	DEPFLAG="-xM1"
  else
	DEPFLAG="-MM"  # dunno do something
  fi 
fi
AC_MSG_RESULT($DEPFLAG)
rm -f conftest.c
AC_SUBST(DEPFLAG)
])

dnl Determine flags that gives POSIX and BSD functionality.
dnl CFLAGS is modified for the result.
AC_DEFUN(ACX_DETERMINE_EXT_FLAGS_UNBOUND,
[
ACX_CHECK_COMPILER_FLAG(std=c99, [C99FLAG="-std=c99"])
ACX_CHECK_COMPILER_FLAG(xc99, [C99FLAG="-xc99"])

AC_CHECK_HEADERS([getopt.h time.h],,, [AC_INCLUDES_DEFAULT])

ACX_CHECK_COMPILER_FLAG_NEEDED($C99FLAG -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -D_ALL_SOURCE,
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
	struct msghdr msg;
	msg.msg_control = 0;
	t = ctime_r(&time, buf);
	tv.tv_usec = 10;
	srandom(32);
	a = getopt(2, opts, "a");
	a = isascii(32);
	str = gai_strerror(0);
	return a;
}
], [CFLAGS="$CFLAGS $C99FLAG -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -D_ALL_SOURCE"])

ACX_CHECK_COMPILER_FLAG_NEEDED($C99FLAG -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_ALL_SOURCE,
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
	struct msghdr msg;
	msg.msg_control = 0;
	t = ctime_r(&time, buf);
	tv.tv_usec = 10;
	srandom(32);
	a = getopt(2, opts, "a");
	a = isascii(32);
	str = gai_strerror(0);
	return a;
}
], [CFLAGS="$CFLAGS $C99FLAG -D__EXTENSIONS__ -D_BSD_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_ALL_SOURCE"])

ACX_CHECK_COMPILER_FLAG_NEEDED($C99FLAG,
[
#include <stdbool.h>
#include <ctype.h>
int test() {
        int a = 0;
        return a;
}
], [CFLAGS="$CFLAGS $C99FLAG"])

ACX_CHECK_COMPILER_FLAG_NEEDED(-D_BSD_SOURCE,
[
#include <ctype.h>

int test() {
        int a;
        a = isascii(32);
        return a;
}
], [CFLAGS="$CFLAGS -D_BSD_SOURCE"])

ACX_CHECK_COMPILER_FLAG_NEEDED(-D_GNU_SOURCE,
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
ACX_CHECK_COMPILER_FLAG_NEEDED(-D_GNU_SOURCE -D_FRSRESGID,
[
#include <unistd.h>

int test() {
	int a = setresgid(0,0,0);
	a = setresuid(0,0,0);
        return a;
}
], [CFLAGS="$CFLAGS -D_GNU_SOURCE"])

ACX_CHECK_COMPILER_FLAG_NEEDED(-D_POSIX_C_SOURCE=200112,
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

ACX_CHECK_COMPILER_FLAG_NEEDED(-D__EXTENSIONS__,
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

])

dnl Check the printf-format attribute (if any)
dnl result in HAVE_ATTR_FORMAT
AC_DEFUN([ACX_CHECK_FORMAT_ATTRIBUTE],
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

dnl Check how to mark function arguments as unused.
dnl result in HAVE_ATTR_UNUSED
AC_DEFUN([ACX_CHECK_UNUSED_ATTRIBUTE],
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

dnl Perform libtool check, portably, only for C
AC_DEFUN(ACX_LIBTOOL_C_ONLY, [
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
# avoid libtool max commandline length test on systems that fork slowly.
AC_CANONICAL_HOST
if echo "$host_os" | grep "sunos4" >/dev/null; then
	lt_cv_sys_max_cmd_len=32750;
fi
AC_PROG_LIBTOOL
])

dnl End of file
