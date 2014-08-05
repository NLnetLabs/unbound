# dnstap.m4

# dt_DNSTAP(default_dnstap_socket_path, [action-if-true], [action-if-false])
# --------------------------------------------------------------------------
# Check for required dnstap libraries and add dnstap configure args.
AC_DEFUN([dt_DNSTAP],
[
  AC_ARG_ENABLE([dnstap],
    AS_HELP_STRING([--enable-dnstap],
                   [Enable dnstap support (requires fstrm, protobuf-c)]),
    [opt_dnstap=$enableval], [opt_dnstap=no])

  AC_ARG_WITH([dnstap-socket-path],
    AS_HELP_STRING([--with-dnstap-socket-path=pathname],
                   [set default dnstap socket path]),
    [opt_dnstap_socket_path=$withval], [opt_dnstap_socket_path="$1"])

  if test "x$opt_dnstap" != "xno"; then
    AC_PATH_PROG([PROTOC_C], [protoc-c])
    if test -z "$PROTOC_C"; then
      AC_MSG_ERROR([The protoc-c program was not found. Please install protobuf-c!])
    fi
    PKG_CHECK_MODULES([libfstrm], [libfstrm])
    PKG_CHECK_MODULES([libprotobuf_c], [libprotobuf-c])
    DNSTAP_CFLAGS="$libfstrm_CFLAGS $libprotobuf_c_CFLAGS"
    DNSTAP_LIBS="$libfstrm_LIBS $libprotobuf_c_LIBS"
    $2
  else
    $3
  fi
])
