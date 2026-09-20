#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sdplane], [m4_esyscmd_s([. sdplane/sdplane-version.sh])], [yasu1976@gmail.com])
AC_CONFIG_SRCDIR([sdplane/sdplane.c])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
LT_INIT([disable-shared])

AC_ARG_WITH(cflags,
[  --with-cflags              Set CFLAGS.])
if test "x${with_cflags}" != "x" ; then
  CFLAGS="${with_cflags}";
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
AC_CHECK_LIB([m], [main])
AC_CHECK_LIB([lthread], [lthread_create], ,
             [AC_MSG_ERROR([lthread library not found.])])
AC_CHECK_LIB([urcu-qsbr], [urcu_qsbr_quiescent_state], ,
             [AC_MSG_ERROR([urcu-qsbr library not found.])])
AC_CHECK_LIB([sdplane], [main])

AC_ARG_WITH([libsdplane],
            [AS_HELP_STRING([--with-libsdplane=DIR],
                            [libsdplane libdir (/usr/lib)])],
[
        libsdplanedir="$withval"
        LDFLAGS="$LDFLAGS -L$withval"
], [
        libsdplanedir="/usr/lib"
])
AC_SUBST([libsdplanedir], [$libsdplanedir])

AC_ARG_ENABLE([pktgen],
        [AS_HELP_STRING([--enable-pktgen], [enable pktgen])],
[
        case "${enableval}" in
        yes) enable_pktgen=yes ;;
        no) enable_pktgen=no ;;
        esac
], enable_pktgen=yes)
AC_SUBST([enable_pktgen], [$enable_pktgen])
AM_CONDITIONAL([ENABLE_PKTGEN], [test "$enable_pktgen" = yes])
if test "$enable_pktgen" = yes; then
AC_DEFINE([ENABLE_PKTGEN], [1], [enable support for pktgen])
fi

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stdint.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([dup2 isascii localtime_r memmove memset mkdir setenv setlocale socket strchr strdup strerror strpbrk strrchr strstr strtol strtoul strtoull uname])

AC_SUBST(LIBTOOL_DEPS)
PKG_CHECK_MODULES(DPDK, [libdpdk])

AC_SUBST([systemdsystemunitdir], [$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 sdplane/Makefile])
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
"
