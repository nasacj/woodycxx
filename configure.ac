AC_INIT(woodycxx, 1.0, cj.nasa@gmail.com)
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-portability])
AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

# the upper level dir
project_dir="\$(top_srcdir)/.."
AC_SUBST(project_dir)

# exported files destination
EXPORT_DIR="\$(project_dir)/export"
AC_SUBST(EXPORT_DIR)

# initialize name of library directory
EXPORT_LIBDIR=$EXPORT_DIR/lib
AC_SUBST(EXPORT_LIBDIR)

# initialize name of include directory
EXPORT_INCDIR=$EXPORT_DIR/include
AC_SUBST(EXPORT_INCDIR)

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
    smart_ptr/Makefile
    sysipc/Makefile
    io/Makefile
    net/Makefile
])

AC_OUTPUT

