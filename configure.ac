#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libsdplane],[m4_esyscmd_s([git describe --abbrev=7 --dirty=-modified --always --tags])],[yasu1976@gmail.com])
AC_CONFIG_SRCDIR([lib/sdplane/command_shell.c])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(/usr/local)
LT_INIT
AC_ENABLE_SHARED
AC_ENABLE_STATIC

dnl ------------
dnl Check CFLAGS
dnl ------------
AC_ARG_WITH(cflags,
[  --with-cflags              Set CFLAGS.])

dnl -----------------------------------------
dnl If linkbase specified expand it
dnl -----------------------------------------
if test "x${with_cflags}" != "x" ; then
  CFLAGS="${with_cflags}";
fi

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(m, main)

# AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h execinfo.h pty.h util.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_CHECK_HEADERS_ONCE([sys/time.h])
AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_MALLOC
AC_FUNC_MEMCMP
#AC_FUNC_REALLOC
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memmove memset strdup strerror strstr ppoll])

AC_SUBST(LIBTOOL_DEPS)

opensh_version=`bash app/opensh-version.sh`
AC_DEFINE_UNQUOTED([OPENSH_VERSION],["$opensh_version"],[opensh version.])
AC_SUBST(OPENSH_VERSION)


case $host in
x86_64-*-linux-gnu*)
    test "x${CFLAGS}" = "x" && CFLAGS="-O0 -g -Wall";
    linux=true;
    ;;
*-apple-darwin*)
    prefix=$HOME/macos
    test "x${CFLAGS}" = "x" && CFLAGS="-O0 -g -Wall";
    macos=true;
    ;;
*)
    ;;
esac
AM_CONDITIONAL(WANT_VTYD, test x$linux = xtrue)

AC_CONFIG_FILES([Makefile lib/Makefile app/Makefile])
AC_OUTPUT

echo "
building configuration
-------------------
version                 : ${VERSION}
host                    : ${host}
compiler                : ${CC}
compiler flags          : ${CFLAGS}
linker flags            : ${LDFLAGS}
link libraries          : ${LIBS}
prefix                  : ${prefix}
opensh-version          : $opensh_version
"

