AC_INIT(woodycxx, 1.0, cj.nasa@gmail.com)
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-portability])
AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_CHECK_LIB([pthread], [pthread_mutex_init])
AC_PROG_RANLIB
AC_PROG_CXX

MY_CFLAGS="-Wall -Wno-sign-compare -Wno-non-virtual-dtor -include config.h -I \$(top_srcdir)/"

AM_CPPFLAGS="$MY_CFLAGS"

AC_SUBST(AM_CPPFLAGS)

AC_CONFIG_FILES([
    Makefile
    sysipc/Makefile
    smart_ptr/Makefile
    io/Makefile
    net/Makefile
])

AC_OUTPUT

