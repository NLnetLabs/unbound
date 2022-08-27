AC_DEFUN([AC_PYTHON_DEVEL],[
        #
        # Allow the use of a (user set) custom python version
        #
        AC_ARG_VAR([PYTHON_VERSION],[The installed Python
                version to use, for example '2.3'. This string
                will be appended to the Python interpreter
                canonical name.])

        AC_PATH_PROG([PYTHON],[python[$PYTHON_VERSION]])
        if test -z "$PYTHON"; then
           AC_MSG_ERROR([Cannot find python$PYTHON_VERSION in your system path])
           PYTHON_VERSION=""
        fi

        if test -z "$PYTHON_VERSION"; then
		PYTHON_VERSION=`$PYTHON -c "import sys; \
			print(sys.version.split()[[0]])"`
	fi

	# Check if you have sysconfig
	AC_MSG_CHECKING([for the sysconfig Python module])
        if ac_sysconfig_result=`$PYTHON -c "import sysconfig" 2>&1`; then
                AC_MSG_RESULT([yes])
		sysconfig_module="sysconfig"
		# if yes, use sysconfig, because distutils is deprecated.
	else
                AC_MSG_RESULT([no])
		# if no, try to use distutils

		#
		# Check if you have distutils, else fail
		#
		AC_MSG_CHECKING([for the distutils Python package])
		if ac_distutils_result=`$PYTHON -c "import distutils" 2>&1`; then
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([cannot import Python module "distutils".
	Please check your Python installation. The error was:
	$ac_distutils_result])
			PYTHON_VERSION=""
		fi

		sysconfig_module="distutils.sysconfig"
	fi

        #
        # Check for Python include path
        #
        AC_MSG_CHECKING([for Python include path])
        if test -z "$PYTHON_CPPFLAGS"; then
		if test "$sysconfig_module" = "sysconfig"; then
			python_path=`$PYTHON -c 'import sysconfig; \
				print(sysconfig.get_path("include"));'`
		else
			python_path=`$PYTHON -c "import distutils.sysconfig; \
				print(distutils.sysconfig.get_python_inc());"`
		fi
                if test -n "${python_path}"; then
                        python_path="-I$python_path"
                fi
                PYTHON_CPPFLAGS=$python_path
        fi
        AC_MSG_RESULT([$PYTHON_CPPFLAGS])
        AC_SUBST([PYTHON_CPPFLAGS])

        #
        # Check for Python library path
        #
        AC_MSG_CHECKING([for Python library path])
        if test -z "$PYTHON_LDFLAGS"; then
            if test $on_mingw = "yes"; then
                PYTHON_LDFLAGS=`$PYTHON -c "from $sysconfig_module import *; \
                        print('-L'+get_config_var('LIBDIR')+' -L'+get_config_var('LIBDEST')+' '+get_config_var('LIBPYTHON')+'.dll');"`
            else
                PYTHON_LDFLAGS=`$PYTHON -c "from $sysconfig_module import *; \
                        print('-L'+get_config_var('LIBDIR')+' -L'+get_config_var('LIBDEST')+' '+get_config_var('BLDLIBRARY'));"`
            fi
        fi
        AC_MSG_RESULT([$PYTHON_LDFLAGS])
        AC_SUBST([PYTHON_LDFLAGS])

        if test -z "$PYTHON_LIBDIR"; then
                PYTHON_LIBDIR=`$PYTHON -c "from $sysconfig_module import *; \
                        print(get_config_var('LIBDIR'));"`
        fi

        #
        # Check for site packages
        #
        AC_MSG_CHECKING([for Python site-packages path])
        if test -z "$PYTHON_SITE_PKG"; then
		if test "$sysconfig_module" = "sysconfig"; then
			PYTHON_SITE_PKG=`$PYTHON -c 'import sysconfig; \
				print(sysconfig.get_path("platlib"));'`
		else
			PYTHON_SITE_PKG=`$PYTHON -c "import distutils.sysconfig; \
				print(distutils.sysconfig.get_python_lib(1,0));"`
		fi
        fi
        AC_MSG_RESULT([$PYTHON_SITE_PKG])
        AC_SUBST([PYTHON_SITE_PKG])

        #
        # final check to see if everything compiles alright
        #
        AC_MSG_CHECKING([consistency of all components of python development environment])
        AC_LANG_PUSH([C])
        # save current global flags
        ac_save_LIBS="$LIBS"
        ac_save_CPPFLAGS="$CPPFLAGS"

        LIBS="$LIBS $PYTHON_LDFLAGS"
        CPPFLAGS="$CPPFLAGS $PYTHON_CPPFLAGS"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[
                #include <Python.h>
        ]],[[
                Py_Initialize();
        ]])],[pythonexists=yes],[pythonexists=no])

        AC_MSG_RESULT([$pythonexists])

        if test ! "$pythonexists" = "yes"; then
           AC_MSG_ERROR([
  Could not link test program to Python. Maybe the main Python library has been
  installed in some non-standard library path. If so, pass it to configure,
  via the LDFLAGS environment variable.
  Example: ./configure LDFLAGS="-L/usr/non-standard-path/python/lib"
  ============================================================================
   ERROR!
   You probably have to install the development version of the Python package
   for your distribution.  The exact name of this package varies among them.
  ============================================================================
           ])
          PYTHON_VERSION=""
        fi
        AC_LANG_POP
        # turn back to default flags
        CPPFLAGS="$ac_save_CPPFLAGS"
        LIBS="$ac_save_LIBS"

        #
        # all done!
        #
])

