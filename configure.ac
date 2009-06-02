#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.56)
sinclude(acx_nlnetlabs.m4)
sinclude(acx_pthread.m4)
sinclude(acx_python.m4)
sinclude(ac_pkg_swig.m4)

AC_INIT(unbound, 1.3.0, unbound-bugs@nlnetlabs.nl, unbound)

LIBUNBOUND_CURRENT=1
LIBUNBOUND_REVISION=0
LIBUNBOUND_AGE=0
# 1.0.0 had 0:12:0
# 1.0.1 had 0:13:0
# 1.0.2 had 0:14:0
# 1.1.0 had 0:15:0
# 1.1.1 had 0:16:0
# 1.2.0 had 0:17:0
# 1.2.1 had 0:18:0
# 1.3.0 had 1:0:0   # ub_cancel and -export-symbols.

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

# are we on MinGW?
if uname -s 2>&1 | grep MINGW32 >/dev/null; then on_mingw="yes"
else on_mingw="no"; fi

#
# Determine configuration file
# the eval is to evaluate shell expansion twice
if test $on_mingw = "no"; then
  ub_conf_file=`eval echo "${sysconfdir}/unbound/unbound.conf"`
else
  ub_conf_file="C:\\Program Files\\Unbound\\service.conf"
fi
AC_ARG_WITH([conf_file],
        AC_HELP_STRING([--with-conf-file=path], 
	[Pathname to the Unbound configuration file]),
	[ub_conf_file="$withval"])
AC_SUBST(ub_conf_file)
ACX_ESCAPE_BACKSLASH($ub_conf_file, hdr_config)
AC_DEFINE_UNQUOTED(CONFIGFILE, ["$hdr_config"], [Pathname to the Unbound configuration file])

# Determine run, chroot directory and pidfile locations
AC_ARG_WITH(run-dir, 
    AC_HELP_STRING([--with-run-dir=path], 
    [set default directory to chdir to (by default dir part of cfg file)]), 
    UNBOUND_RUN_DIR="$withval", 
if test $on_mingw = no; then
    UNBOUND_RUN_DIR=`dirname "$ub_conf_file"`
else
    UNBOUND_RUN_DIR=""
fi
)
AC_SUBST(UNBOUND_RUN_DIR)
ACX_ESCAPE_BACKSLASH($UNBOUND_RUN_DIR, hdr_run)
AC_DEFINE_UNQUOTED(RUN_DIR, ["$hdr_run"], [Directory to chdir to])

AC_ARG_WITH(chroot-dir, 
    AC_HELP_STRING([--with-chroot-dir=path], 
    [set default directory to chroot to (by default same as run-dir)]), 
    UNBOUND_CHROOT_DIR="$withval", 
if test $on_mingw = no; then
    UNBOUND_CHROOT_DIR="$UNBOUND_RUN_DIR"
else
    UNBOUND_CHROOT_DIR=""
fi
)
AC_SUBST(UNBOUND_CHROOT_DIR)
ACX_ESCAPE_BACKSLASH($UNBOUND_CHOOT_DIR, hdr_chroot)
AC_DEFINE_UNQUOTED(CHROOT_DIR, ["$hdr_chroot"], [Directory to chroot to])

AC_ARG_WITH(share-dir,
    AC_HELP_STRING([--with-share-dir=path],
    [set default directory with shared data (by default same as share/unbound)]),
    UNBOUND_SHARE_DIR="$withval",
    UNBOUND_SHARE_DIR="$UNBOUND_RUN_DIR")
AC_SUBST(UNBOUND_SHARE_DIR)
AC_DEFINE_UNQUOTED(SHARE_DIR, ["$UNBOUND_SHARE_DIR"], [Shared data])

AC_ARG_WITH(pidfile, 
    AC_HELP_STRING([--with-pidfile=filename], 
    [set default pathname to unbound pidfile (default run-dir/unbound.pid)]), 
    UNBOUND_PIDFILE="$withval", 
if test $on_mingw = no; then
    UNBOUND_PIDFILE="$UNBOUND_RUN_DIR/unbound.pid"
else
    UNBOUND_PIDFILE=""
fi
)
AC_SUBST(UNBOUND_PIDFILE)
ACX_ESCAPE_BACKSLASH($UNBOUND_PIDFILE, hdr_pid)
AC_DEFINE_UNQUOTED(PIDFILE, ["$hdr_pid"], [default pidfile location])

AC_ARG_WITH(username, 
    AC_HELP_STRING([--with-username=user], 
    [set default user that unbound changes to (default user is unbound)]), 
    UNBOUND_USERNAME="$withval", 
    UNBOUND_USERNAME="unbound")
AC_SUBST(UNBOUND_USERNAME)
AC_DEFINE_UNQUOTED(UB_USERNAME, ["$UNBOUND_USERNAME"], [default username])

AC_DEFINE(WINVER, 0x0502, [the version of the windows API enabled])
ACX_RSRC_VERSION(wnvs)
AC_DEFINE_UNQUOTED(RSRC_PACKAGE_VERSION, [$wnvs], [version number for resource files])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_LANG_C
ACX_CHECK_COMPILER_FLAG(g, [CFLAGS="$CFLAGS -g"])
ACX_CHECK_COMPILER_FLAG(O2, [CFLAGS="$CFLAGS -O2"])
AC_PROG_CC
ACX_DEPFLAG
ACX_DETERMINE_EXT_FLAGS_UNBOUND

# for Sun studio 11.
ACX_CHECK_COMPILER_FLAG(xO4, [CFLAGS="$CFLAGS -xO4"])
ACX_CHECK_COMPILER_FLAG(xtarget=generic, [CFLAGS="$CFLAGS -xtarget=generic"])

# debug mode flags warnings
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Enable debug warnings, asserts, makefile-dependencies]))
debug_enabled="$enable_debug"
AC_SUBST(debug_enabled)
case "$enable_debug" in
        yes)
		ACX_CHECK_COMPILER_FLAG(W, [CFLAGS="$CFLAGS -W"])
		ACX_CHECK_COMPILER_FLAG(Wall, [CFLAGS="$CFLAGS -Wall"])
		ACX_CHECK_COMPILER_FLAG(Wextra, [CFLAGS="$CFLAGS -Wextra"])
		ACX_CHECK_COMPILER_FLAG(Wdeclaration-after-statement, [CFLAGS="$CFLAGS -Wdeclaration-after-statement"])
		AC_DEFINE([UNBOUND_DEBUG], [], [define this to enable debug checks.])
		;;
	no|*)
		# nothing to do.
		;;
esac

AC_C_INLINE
ACX_CHECK_FORMAT_ATTRIBUTE
ACX_CHECK_UNUSED_ATTRIBUTE

if test "$srcdir" != "."; then
	CPPFLAGS="$CPPFLAGS -I$srcdir"
fi

AC_PROG_LEX
AC_PROG_YACC
AC_CHECK_PROG(doxygen, doxygen, doxygen)
ACX_LIBTOOL_C_ONLY

# Checks for header files.
AC_CHECK_HEADERS([stdarg.h stdbool.h netinet/in.h sys/param.h sys/socket.h sys/uio.h sys/resource.h arpa/inet.h syslog.h netdb.h sys/wait.h pwd.h glob.h grp.h login_cap.h winsock2.h ws2tcpip.h],,, [AC_INCLUDES_DEFAULT])

# check for types.  
# Using own tests for int64* because autoconf builtin only give 32bit.
AC_CHECK_TYPE(int8_t, signed char)
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
ACX_TYPE_U_CHAR
ACX_TYPE_RLIM_T
ACX_TYPE_SOCKLEN_T
ACX_TYPE_IN_ADDR_T
ACX_TYPE_IN_PORT_T

# add option to disable the evil rpath
ACX_ARG_RPATH

# check for thread library.
# check this first, so that the pthread lib does not get linked in via
# libssl or libpython, and thus distorts the tests, and we end up using
# the non-threadsafe C libraries.
AC_ARG_WITH(pthreads, AC_HELP_STRING([--with-pthreads], 
 [use pthreads library, or --without-pthreads to disable threading support.]), 
 [ ],[ withval="yes" ])
ub_have_pthreads=no
if test x_$withval != x_no; then
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

		ACX_CHECK_COMPILER_FLAG(mt, [CFLAGS="$CFLAGS -mt"],
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

# Check for PyUnbound
AC_ARG_WITH(pyunbound,
   AC_HELP_STRING([--with-pyunbound],
   [build PyUnbound, or --without-pyunbound to skip it. (default=no)]),
   [], [ withval="no" ])

ub_test_python=no
ub_with_pyunbound=no
if test x_$withval != x_no; then
   ub_with_pyunbound=yes
   ub_test_python=yes
fi

# Check for Python module
AC_ARG_WITH(pythonmodule,
   AC_HELP_STRING([--with-pythonmodule],
   [build Python module, or --without-pythonmodule to disable script engine. (default=no)]),
   [], [ withval="no" ])

ub_with_pythonmod=no
if test x_$withval != x_no; then
   ub_with_pythonmod=yes
   ub_test_python=yes
fi

# Check for Python & SWIG only on PyUnbound or PyModule
if test x_$ub_test_python != x_no; then

   # Check for Python
   ub_have_python=no
   AC_PYTHON_DEVEL
   if test ! -z "$PYTHON_VERSION"; then
	if test `$PYTHON -c "print '$PYTHON_VERSION' >= '2.4.0'"` = "False"; then
		AC_ERROR([Python version >= 2.4.0 is required])
	fi

      # Have Python
      AC_DEFINE(HAVE_PYTHON,1,[Define if you have Python libraries and header files.])
      LIBS="$PYTHON_LDFLAGS $LIBS"
      CPPFLAGS="$CPPFLAGS $PYTHON_CPPFLAGS"
      ub_have_python=yes

      # Check for SWIG
      ub_have_swig=no
      AC_PROG_SWIG
      AC_MSG_CHECKING(SWIG)
      if test ! -x "$SWIG"; then
         AC_MSG_RESULT(failed, won't build Python module and PyUnbound)
         AC_SUBST(swig, "")
         ub_with_pyunbound=no
         ub_with_pythonmod=no
      else
         AC_DEFINE(HAVE_SWIG, 1, [Define if you have Swig libraries and header files.])
         AC_SUBST(swig, "$SWIG")
         AC_MSG_RESULT(present)

         # If have Python & SWIG
         # Declare PythonMod
         if test x_$ub_with_pythonmod != x_no; then
            AC_DEFINE(WITH_PYTHONMODULE, 1, [Define if you want Python module.])
            WITH_PYTHONMODULE=yes
            AC_SUBST(WITH_PYTHONMODULE)
         fi

         # Declare PyUnbound
         if test x_$ub_with_pyunbound != x_no; then
            AC_DEFINE(WITH_PYUNBOUND, 1, [Define if you want PyUnbound.])
            WITH_PYUNBOUND=yes
            AC_SUBST(WITH_PYUNBOUND)
         fi
      fi
   else
      AC_MSG_RESULT([*** Python libraries not found, won't build PythonMod or PyUnbound ***])
      ub_with_pyunbound=no
      ub_with_pythonmod=no
   fi
fi

# Checks for libraries.
ACX_WITH_SSL
ACX_LIB_SSL
AC_CHECK_FUNCS([EVP_sha1 EVP_sha256 EVP_sha512 ENGINE_load_gost])

AC_ARG_ENABLE(sha2, AC_HELP_STRING([--enable-sha2], [Enable SHA256 and SHA512 RRSIG support]))
case "$enable_sha2" in
	yes)
	AC_DEFINE_UNQUOTED([USE_SHA2], [], [Define this to enable SHA256 and SHA512 support.])
	;;
	no|*)
	;;
esac

# check to see if libraries are needed for these functions.
AC_SEARCH_LIBS([inet_pton], [nsl])
AC_SEARCH_LIBS([socket], [socket])

AC_SUBST(RUNTIME_PATH)

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
		dnl assume /usr is in default path.
		if test "$thedir" != "/usr"; then
                    CPPFLAGS="$CPPFLAGS -I$thedir/include"
		fi
                break;
            fi
        done
        if test x_$found_libevent != x_yes; then
		if test -f "$dir/event.h" -a \( -f "$dir/libevent.la" -o -f "$dir/libev.la" \) ; then
			# libevent source directory
            		AC_MSG_RESULT(found in $thedir)
                	CPPFLAGS="$CPPFLAGS -I$thedir -I$thedir/include"
			BAK_LDFLAGS_SET="1"
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
	    dnl assume /usr is in default path.
	    if test "$thedir" != "/usr"; then
                LDFLAGS="$LDFLAGS -L$thedir/lib"
		ACX_RUNTIME_PATH_ADD([$thedir/lib])
	    fi
        fi
	# check for library used by libevent after 1.3c
	AC_SEARCH_LIBS([clock_gettime], [rt])

	AC_SEARCH_LIBS(event_set, [event])
	AC_CHECK_HEADERS([event.h],,, [AC_INCLUDES_DEFAULT])
	AC_CHECK_FUNCS([event_base_free]) # only in libevent 1.2 and later
	AC_CHECK_FUNCS([event_base_once]) # only in libevent 1.4.1 and later
	AC_CHECK_FUNCS([event_base_new]) # only in libevent 1.4.1 and later
	AC_CHECK_FUNCS([event_base_get_method]) # only in libevent 1.4.3 and later
	AC_CHECK_FUNCS([ev_loop]) # only in libev. (tested on 3.51)
	if test -n "$BAK_LDFLAGS_SET"; then
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
	if test "$on_mingw" = yes; then
		staticexe="-all-static"
	fi
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
ACX_SYS_LARGEFILE

ACX_CHECK_GETADDRINFO_WITH_INCLUDES
if test "$USE_WINSOCK" = 1; then
	AC_DEFINE(UB_ON_WINDOWS, 1, [Use win32 resources and API])
	UB_ON_WINDOWS=yes
	AC_SUBST(UB_ON_WINDOWS)
fi
if test $ac_cv_func_getaddrinfo = no; then
	AC_LIBOBJ([fake-rfc2553])
fi

# see if daemon(3) exists, and if it is deprecated.
AC_CHECK_FUNCS([daemon])
if test $ac_cv_func_daemon = yes; then
	ACX_FUNC_DEPRECATED([daemon], [(void)daemon(0, 0);], [
#include <stdlib.h>
])
fi

AC_SEARCH_LIBS([setusercontext], [util])
AC_CHECK_FUNCS([tzset sigprocmask fcntl getpwnam getrlimit setsid sbrk chroot kill sleep usleep random srandom recvmsg sendmsg writev setresuid setreuid setresgid setregid glob initgroups strftime localtime_r setusercontext _beginthreadex])

# check if setreuid en setregid fail, on MacOSX10.4(darwin8).
if echo $build_os | grep darwin8 > /dev/null; then
	AC_DEFINE(DARWIN_BROKEN_SETREUID, 1, [Define this if on macOSX10.4-darwin8 and setreuid and setregid do not work])
fi
ACX_CHECK_NONBLOCKING_BROKEN
ACX_MKDIR_ONE_ARG
ACX_FUNC_IOCTLSOCKET
AC_REPLACE_FUNCS(inet_aton)
AC_REPLACE_FUNCS(inet_pton)
AC_REPLACE_FUNCS(inet_ntop)
AC_REPLACE_FUNCS(snprintf)
AC_REPLACE_FUNCS(strlcpy)
AC_REPLACE_FUNCS(memmove)
AC_REPLACE_FUNCS(gmtime_r)

# check this after all other compilation checks, since the linking of the lib
# may break checks after this.
use_ldns_builtin="no"
AC_ARG_WITH(ldns, AC_HELP_STRING([--with-ldns=PATH], 
	[specify prefix of path of ldns library to use]), 
	[ specialldnsdir="$withval"
	CPPFLAGS="-I$withval/include $CPPFLAGS"
	LDFLAGS="-L$withval -L$withval/lib $LDFLAGS"
	ACX_RUNTIME_PATH_ADD([$withval/lib])
	ldnsdir="$withval"
	AC_SUBST(ldnsdir)
])

AC_ARG_WITH(ldns-builtin, AC_HELP_STRING([--with-ldns-builtin], 
	[forces use of package included with this one]), [ 
	use_ldns_builtin="yes"
])

# check if ldns is good enough
if test "$use_ldns_builtin" = "no"; then
	AC_CHECK_LIB(ldns, ldns_buffer_copy)
	if test $ac_cv_lib_ldns_ldns_buffer_copy = yes; then
	    dnl ldns was found, check compat functions
	    AC_CHECK_FUNC([ldns_b32_ntop_extended_hex],, [
		AC_MSG_ERROR([ldns version too old, need >=1.4.0])])
	else
	    use_ldns_builtin="yes"
	fi
fi

if test "$use_ldns_builtin" = "yes"; then
dnl use the builtin ldns-src.tar.gz file to build ldns.
if test ! -f $srcdir/ldns-src.tar.gz; then
	AC_MSG_ERROR([No ldns library found and no ldns-src.tar.gz, use --with-ldns=path.])
fi
echo "****************************************************************"
echo "***                                                          ***"
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
fi  dnl end of use_ldns_builtin == yes

ACX_STRIP_EXT_FLAGS
LDFLAGS="$LATE_LDFLAGS $LDFLAGS"

AC_DEFINE_UNQUOTED([MAXSYSLOGMSGLEN], [5120], [Define to the maximum message length to pass to syslog.])

AH_BOTTOM(
dnl this must be first AH_CONFIG, to define the flags before any includes.
AHX_CONFIG_EXT_FLAGS

dnl includes
[
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
]

AHX_CONFIG_FORMAT_ATTRIBUTE
AHX_CONFIG_UNUSED_ATTRIBUTE
AHX_CONFIG_FSEEKO
AHX_CONFIG_RAND_MAX
AHX_CONFIG_MAXHOSTNAMELEN
AHX_CONFIG_SNPRINTF(unbound)
AHX_CONFIG_INET_PTON(unbound)
AHX_CONFIG_INET_NTOP(unbound)
AHX_CONFIG_INET_ATON(unbound)
AHX_CONFIG_MEMMOVE(unbound)
AHX_CONFIG_STRLCPY(unbound)
AHX_CONFIG_GMTIME_R(unbound)
AHX_CONFIG_W32_SLEEP
AHX_CONFIG_W32_USLEEP
AHX_CONFIG_W32_RANDOM
AHX_CONFIG_W32_SRANDOM
AHX_CONFIG_W32_FD_SET_T
AHX_CONFIG_IPV6_MIN_MTU

[
#if defined(HAVE_EVENT_H) && !defined(HAVE_EVENT_BASE_ONCE) && (defined(HAVE_PTHREAD) || defined(HAVE_SOLARIS_THREADS))
   /* using version of libevent that is not threadsafe. */
#  define LIBEVENT_SIGNAL_PROBLEM 1
#endif

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

AC_CONFIG_FILES([Makefile doc/example.conf doc/libunbound.3 doc/unbound.8 doc/unbound-checkconf.8 doc/unbound.conf.5 doc/unbound-control.8])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT
