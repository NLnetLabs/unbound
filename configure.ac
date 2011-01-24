#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.56)
sinclude(acx_nlnetlabs.m4)
sinclude(acx_pthread.m4)
sinclude(acx_python.m4)
sinclude(ac_pkg_swig.m4)

AC_INIT(unbound, 1.4.9, unbound-bugs@nlnetlabs.nl, unbound)

LIBUNBOUND_CURRENT=2
LIBUNBOUND_REVISION=9
LIBUNBOUND_AGE=0
# 1.0.0 had 0:12:0
# 1.0.1 had 0:13:0
# 1.0.2 had 0:14:0
# 1.1.0 had 0:15:0
# 1.1.1 had 0:16:0
# 1.2.0 had 0:17:0
# 1.2.1 had 0:18:0
# 1.3.0 had 1:0:0   # ub_cancel and -export-symbols.
# 1.3.1 had 1:1:0
# 1.3.2 had 1:2:0
# 1.3.3 had 1:3:0
# 1.3.4 had 1:4:0
# 1.4.0-snapshots had 1:5:0
# 1.4.0 had 1:5:0 (not 2:0:0)   # ub_result.why_bogus
# 1.4.1 had 2:1:0
# 1.4.2 had 2:2:0
# 1.4.3 had 2:3:0
# 1.4.4 had 2:4:0
# 1.4.5 had 2:5:0
# 1.4.6 had 2:6:0
# 1.4.7 had 2:7:0
# 1.4.8 had 2:8:0
# 1.4.9 had 2:9:0

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

pretty_cmdline() {
	cmdline=""
	while test -n "$1"; do
		cmdline="$cmdline '"`echo $1 | sed -e 's/\\\\/\\\\\\\\/g' | sed -e 's/"/\\\\"/g' `"'"
		shift
	done
}
pretty_cmdline $@
AC_DEFINE_UNQUOTED(CONFIGURE_BUILD_WITH, ["$cmdline"], [configure flags])
AC_CANONICAL_TARGET
AC_DEFINE_UNQUOTED(CONFIGURE_TARGET, ["$target"], [configure target system])
AC_DEFINE_UNQUOTED(CONFIGURE_DATE, ["`date`"], [configure date])

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
else 
	if echo $target | grep mingw32 >/dev/null; then on_mingw="yes"
	else on_mingw="no"; fi
fi

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

AC_ARG_WITH(rootkey-file, 
    AC_HELP_STRING([--with-rootkey-file=filename], 
    [set default pathname to root key file (default run-dir/root.key). This file is read and written.]), 
    UNBOUND_ROOTKEY_FILE="$withval", 
if test $on_mingw = no; then
    UNBOUND_ROOTKEY_FILE="$UNBOUND_RUN_DIR/root.key"
else
    UNBOUND_ROOTKEY_FILE="C:\\Program Files\\Unbound\\root.key"
fi
)
AC_SUBST(UNBOUND_ROOTKEY_FILE)
ACX_ESCAPE_BACKSLASH($UNBOUND_ROOTKEY_FILE, hdr_rkey)
AC_DEFINE_UNQUOTED(ROOT_ANCHOR_FILE, ["$hdr_rkey"], [default rootkey location])

AC_ARG_WITH(rootcert-file, 
    AC_HELP_STRING([--with-rootcert-file=filename], 
    [set default pathname to root update certificate file (default run-dir/icannbundle.pem).  This file need not exist if you are content with the builtin.]), 
    UNBOUND_ROOTCERT_FILE="$withval", 
if test $on_mingw = no; then
    UNBOUND_ROOTCERT_FILE="$UNBOUND_RUN_DIR/icannbundle.pem"
else
    UNBOUND_ROOTCERT_FILE="C:\\Program Files\\Unbound\\icannbundle.pem"
fi
)
AC_SUBST(UNBOUND_ROOTCERT_FILE)
ACX_ESCAPE_BACKSLASH($UNBOUND_ROOTCERT_FILE, hdr_rpem)
AC_DEFINE_UNQUOTED(ROOT_CERT_FILE, ["$hdr_rpem"], [default rootcert location])

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

# debug mode flags warnings
AC_ARG_ENABLE(checking, AC_HELP_STRING([--enable-checking], [Enable warnings, asserts, makefile-dependencies]))
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [same as enable-checking]))
if test "$enable_debug" = "yes"; then debug_enabled="$enable_debug"; 
else debug_enabled="$enable_checking"; fi
AC_SUBST(debug_enabled)
case "$debug_enabled" in
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

AC_DEFUN([ACX_YYLEX_DESTROY], [
	AC_MSG_CHECKING([for yylex_destroy])
	if echo %% | $LEX -t 2>&1 | grep yylex_destroy >/dev/null 2>&1; then
		AC_DEFINE(LEX_HAS_YYLEX_DESTROY, 1, [if lex has yylex_destroy])
		AC_MSG_RESULT(yes)
	else AC_MSG_RESULT(no); fi
])

AC_PROG_LEX
ACX_YYLEX_DESTROY
AC_PROG_YACC
AC_CHECK_PROG(doxygen, doxygen, doxygen)
AC_CHECK_TOOL(STRIP, strip)
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
ACX_CHECK_MEMCMP_SIGNED

# add option to disable the evil rpath
ACX_ARG_RPATH
AC_SUBST(RUNTIME_PATH)

# check to see if libraries are needed for these functions.
AC_SEARCH_LIBS([inet_pton], [nsl])
AC_SEARCH_LIBS([socket], [socket])

# check some functions of the OS before linking libs (while still runnable).
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_FUNC_FSEEKO
ACX_SYS_LARGEFILE
ACX_CHECK_NONBLOCKING_BROKEN
ACX_MKDIR_ONE_ARG

# check windows threads (we use them, not pthreads, on windows).
if test "$on_mingw" = "yes"; then
# check windows threads
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

else
# not on mingw, check thread libraries.

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

fi # end of non-mingw check of thread libraries

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
   ac_save_LIBS="$LIBS" dnl otherwise AC_PYTHON_DEVEL thrashes $LIBS
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
         AC_ERROR([failed to find swig tool, install it, or do not build Python module and PyUnbound])
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
AC_CHECK_HEADERS([openssl/conf.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([openssl/engine.h],,, [AC_INCLUDES_DEFAULT])
AC_CHECK_FUNCS([OPENSSL_config EVP_sha1 EVP_sha256 EVP_sha512])
AC_CHECK_DECLS([SSL_COMP_get_compression_methods,sk_SSL_COMP_pop_free], [], [], [
AC_INCLUDES_DEFAULT
#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif

#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif

#ifdef HAVE_OPENSSL_CONF_H
#include <openssl/conf.h>
#endif

#ifdef HAVE_OPENSSL_ENGINE_H
#include <openssl/engine.h>
#endif
#include <openssl/ssl.h>
#include <openssl/evp.h>
])

AC_ARG_ENABLE(sha2, AC_HELP_STRING([--disable-sha2], [Disable SHA256 and SHA512 RRSIG support]))
case "$enable_sha2" in
	no)
	;;
	yes|*)
	AC_DEFINE([USE_SHA2], [1], [Define this to enable SHA256 and SHA512 support.])
	;;
esac

AC_ARG_ENABLE(gost, AC_HELP_STRING([--disable-gost], [Disable GOST support]))
use_gost="no"
case "$enable_gost" in
	no)
	;;
	*)
	AC_CHECK_FUNC(EVP_PKEY_set_type_str, [:],[AC_MSG_ERROR([OpenSSL 1.0.0 is needed for GOST support])])
        AC_CHECK_FUNC(EC_KEY_new, [], [AC_MSG_ERROR([OpenSSL does not support ECC, needed for GOST support])])
	use_gost="yes"
	AC_DEFINE([USE_GOST], [1], [Define this to enable GOST support.])
	;;
esac

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
			ev_files_o=`ls $thedir/*.o | grep -v evdns\.o | grep -v bufferevent_openssl\.o`
			ev_files_lo=`ls $thedir/*.lo | grep -v evdns\.lo | grep -v bufferevent_openssl\.lo`
			ev_files_libso=`ls $thedir/.libs/*.o | grep -v evdns\.o | grep -v bufferevent_openssl\.o`
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
	    dnl assume /usr is in default path, do not add "".
	    if test "$thedir" != "/usr" -a "$thedir" != ""; then
                LDFLAGS="$LDFLAGS -L$thedir/lib"
		ACX_RUNTIME_PATH_ADD([$thedir/lib])
	    fi
        fi
	# check for library used by libevent after 1.3c
	AC_SEARCH_LIBS([clock_gettime], [rt])

	# is the event.h header libev or libevent?
	AC_CHECK_HEADERS([event.h],,, [AC_INCLUDES_DEFAULT])
	AC_CHECK_DECL(EV_VERSION_MAJOR, [
		AC_SEARCH_LIBS(event_set, [ev])
	],[
		AC_SEARCH_LIBS(event_set, [event])
	],[AC_INCLUDES_DEFAULT
#include <event.h>
	])
	AC_CHECK_FUNCS([event_base_free]) # only in libevent 1.2 and later
	AC_CHECK_FUNCS([event_base_once]) # only in libevent 1.4.1 and later
	AC_CHECK_FUNCS([event_base_new]) # only in libevent 1.4.1 and later
	AC_CHECK_FUNCS([event_base_get_method]) # only in libevent 1.4.3 and later
	AC_CHECK_FUNCS([ev_loop]) # only in libev. (tested on 3.51)
	AC_CHECK_FUNCS([ev_default_loop]) # only in libev. (tested on 4.00)
	if test -n "$BAK_LDFLAGS_SET"; then
		LDFLAGS="$BAK_LDFLAGS"
	fi
else
	AC_DEFINE(USE_MINI_EVENT, 1, [Define if you want to use internal select based events])
fi

# check for libexpat
AC_ARG_WITH(libexpat, AC_HELP_STRING([--with-libexpat=path],
    [specify explicit path for libexpat.]),
    [ ],[ withval="/usr/local /opt/local /usr/lib /usr/pkg /usr/sfw /usr" ])
AC_MSG_CHECKING(for libexpat)
found_libexpat="no"
for dir in $withval ; do
            if test -f "$dir/include/expat.h"; then
		found_libexpat="yes"
		dnl assume /usr is in default path.
		if test "$dir" != "/usr"; then
                    CPPFLAGS="$CPPFLAGS -I$dir/include"
		    LDFLAGS="$LDFLAGS -L$dir/lib"
		fi
            	AC_MSG_RESULT(found in $dir)
                break;
            fi
done
if test x_$found_libexpat != x_yes; then
	AC_ERROR([Could not find libexpat, expat.h])
fi
AC_CHECK_HEADERS([expat.h],,, [AC_INCLUDES_DEFAULT])

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
		# for static crosscompile, include gdi32 and zlib here.
		if test "`uname`" = "Linux"; then
			LIBS="$LIBS -lgdi32 -lz"
		fi
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
	[ enable to memory allocation statistics, for debug purposes ]), 
	, )
AC_ARG_ENABLE(alloc-lite, AC_HELP_STRING([--enable-alloc-lite],
	[ enable for lightweight alloc assertions, for debug purposes ]), 
	, )
AC_ARG_ENABLE(alloc-nonregional, AC_HELP_STRING([--enable-alloc-nonregional],
	[ enable nonregional allocs, slow but exposes regional allocations to other memory purifiers, for debug purposes ]), 
	, )
if test x_$enable_alloc_nonregional = x_yes; then
	AC_DEFINE(UNBOUND_ALLOC_NONREGIONAL, 1, [use malloc not regions, for debug use])
fi
if test x_$enable_alloc_checks = x_yes; then
	AC_DEFINE(UNBOUND_ALLOC_STATS, 1, [use statistics for allocs and frees, for debug use])
else
	if test x_$enable_alloc_lite = x_yes; then
		AC_DEFINE(UNBOUND_ALLOC_LITE, 1, [use to enable lightweight alloc assertions, for debug use])
	else
		ACX_FUNC_MALLOC([unbound])
	fi
fi

ACX_CHECK_GETADDRINFO_WITH_INCLUDES
if test "$USE_WINSOCK" = 1; then
	AC_DEFINE(UB_ON_WINDOWS, 1, [Use win32 resources and API])
	UB_ON_WINDOWS=yes
	AC_SUBST(UB_ON_WINDOWS)
	AC_CHECK_HEADERS([iphlpapi.h],,, [AC_INCLUDES_DEFAULT
#include <windows.h>
	])
	AC_CHECK_TOOL(WINDRES, windres)
	LIBS="$LIBS -liphlpapi"
fi
if test $ac_cv_func_getaddrinfo = no; then
	AC_LIBOBJ([fake-rfc2553])
fi
# check after getaddrinfo for its libraries
ACX_FUNC_IOCTLSOCKET

# see if daemon(3) exists, and if it is deprecated.
AC_CHECK_FUNCS([daemon])
if test $ac_cv_func_daemon = yes; then
	ACX_FUNC_DEPRECATED([daemon], [(void)daemon(0, 0);], [
#include <stdlib.h>
])
fi

# check wether strptime also works
AC_DEFUN([AC_CHECK_STRPTIME_WORKS],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(whether strptime works)
if test c${cross_compiling} = cno; then
AC_TRY_RUN([
#define _XOPEN_SOURCE
#include <time.h>
int main(void) { struct tm tm; char *res;
res = strptime("20070207111842", "%Y%m%d%H%M%S", &tm);
if (!res) return 1; return 0; }
] , [eval "ac_cv_c_strptime_works=yes"], [eval "ac_cv_c_strptime_works=no"])
else
eval "ac_cv_c_strptime_works=maybe"
fi
AC_MSG_RESULT($ac_cv_c_strptime_works)
if test $ac_cv_c_strptime_works = no; then
AC_LIBOBJ(strptime)
else
AC_DEFINE_UNQUOTED([STRPTIME_WORKS], 1, [use default strptime.])
fi
])dnl
AC_CHECK_FUNCS([strptime],[AC_CHECK_STRPTIME_WORKS],[AC_LIBOBJ([strptime])])
AC_SEARCH_LIBS([setusercontext], [util])
AC_CHECK_FUNCS([tzset sigprocmask fcntl getpwnam getrlimit setrlimit setsid sbrk chroot kill sleep usleep random srandom recvmsg sendmsg writev socketpair glob initgroups strftime localtime_r setusercontext _beginthreadex])
AC_CHECK_FUNCS([setresuid],,[AC_CHECK_FUNCS([setreuid])])
AC_CHECK_FUNCS([setresgid],,[AC_CHECK_FUNCS([setregid])])

# check if setreuid en setregid fail, on MacOSX10.4(darwin8).
if echo $build_os | grep darwin8 > /dev/null; then
	AC_DEFINE(DARWIN_BROKEN_SETREUID, 1, [Define this if on macOSX10.4-darwin8 and setreuid and setregid do not work])
fi
AC_REPLACE_FUNCS(inet_aton)
AC_REPLACE_FUNCS(inet_pton)
AC_REPLACE_FUNCS(inet_ntop)
AC_REPLACE_FUNCS(snprintf)
AC_REPLACE_FUNCS(strlcpy)
AC_REPLACE_FUNCS(memmove)
AC_REPLACE_FUNCS(gmtime_r)
AC_REPLACE_FUNCS(ctime_r)

# check this after all other compilation checks, since the linking of the lib
# may break checks after this.
use_ldns_builtin="no"
AC_ARG_WITH(ldns, AC_HELP_STRING([--with-ldns=PATH], 
	[specify prefix of path of ldns library to use]), 
	[ specialldnsdir="$withval"
	if test "$withval" != "yes"; then
		if test "$withval" != "/usr" -a "$withval" != ""; then
			CPPFLAGS="-I$withval/include $CPPFLAGS"
			LDFLAGS="-L$withval/lib $LDFLAGS"
			ACX_RUNTIME_PATH_ADD([$withval/lib])
		fi
		ldnsdir="$withval"
		AC_SUBST(ldnsdir)
	fi
])

AC_ARG_WITH(ldns-builtin, AC_HELP_STRING([--with-ldns-builtin], 
	[forces use of package included with this one]), [ 
	use_ldns_builtin="yes"
])

# check if ldns is good enough
if test "$use_ldns_builtin" = "no"; then
	AC_CHECK_LIB(ldns, ldns_buffer_copy)
	AC_CHECK_FUNC(ldns_key_buf2rsa_raw)
	AC_CHECK_FUNC(ldns_b32_ntop_extended_hex)
	if test x$use_gost = xyes; then
	    AC_CHECK_FUNC(ldns_key_EVP_load_gost_id)
	    AC_CHECK_FUNCS([ldns_key_EVP_unload_gost])
	else
	    ac_cv_func_ldns_key_EVP_load_gost_id="yes"
	fi
	AC_CHECK_HEADERS([ldns/ldns.h],,, [AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
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
])
	if test $ac_cv_lib_ldns_ldns_buffer_copy = yes \
	    -a $ac_cv_func_ldns_key_buf2rsa_raw = yes \
	    -a $ac_cv_header_ldns_ldns_h = yes \
	    -a $ac_cv_func_ldns_b32_ntop_extended_hex = yes \
	    -a $ac_cv_func_ldns_key_EVP_load_gost_id = yes; then
	    dnl ldns was found
	    :
	else
	    AC_MSG_ERROR([No ldns library found (or not recent); install or update ldns library, use --with-ldns=path or --with-ldns-builtin])
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

AC_DEFINE_UNQUOTED([MAXSYSLOGMSGLEN], [10240], [Define to the maximum message length to pass to syslog.])

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
]

AHX_CONFIG_FORMAT_ATTRIBUTE
AHX_CONFIG_UNUSED_ATTRIBUTE
AHX_CONFIG_FSEEKO
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
AHX_MEMCMP_BROKEN(unbound)

[
#ifndef HAVE_CTIME_R
#define ctime_r unbound_ctime_r
char *ctime_r(const time_t *timep, char *buf);
#endif

#if !defined(HAVE_STRPTIME) || !defined(STRPTIME_WORKS)
#define strptime unbound_strptime
struct tm;
char *strptime(const char *s, const char *format, struct tm *tm);
#endif

#if defined(HAVE_EVENT_H) && !defined(HAVE_EVENT_BASE_ONCE) && !(defined(HAVE_EV_LOOP) || defined(HAVE_EV_DEFAULT_LOOP)) && (defined(HAVE_PTHREAD) || defined(HAVE_SOLARIS_THREADS))
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
#elif defined(UNBOUND_ALLOC_LITE)
#  include "util/alloc.h"
#endif /* UNBOUND_ALLOC_LITE and UNBOUND_ALLOC_STATS */

/** default port for DNS traffic. */
#define UNBOUND_DNS_PORT 53
])

AC_CONFIG_FILES([Makefile doc/example.conf doc/libunbound.3 doc/unbound.8 doc/unbound-anchor.8 doc/unbound-checkconf.8 doc/unbound.conf.5 doc/unbound-control.8])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT
