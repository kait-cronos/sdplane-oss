#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sdplane],[m4_esyscmd_s([. sdplane/sdplane-version.sh])],[yasu1976@gmail.com])
AC_CONFIG_SRCDIR([sdplane/sdplane.c])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])

AC_ARG_WITH(cflags,
[  --with-cflags              Set CFLAGS.])
if test "x${with_cflags}" != "x" ; then
  CFLAGS="${with_cflags}";
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
AC_CHECK_LIB([lthread], [lthread_create], ,
             [AC_MSG_ERROR([lthread library not found.])])
AC_CHECK_LIB([urcu-qsbr], [urcu_qsbr_quiescent_state], ,
             [AC_MSG_ERROR([urcu-qsbr library not found.])])

AC_ARG_WITH([libsdplane],
            [AS_HELP_STRING([--with-libsdplane=DIR],
                            [libsdplane libdir (/usr/lib)])],
[
        libsdplanedir="$withval"
        LDLFAGS="$LDFLAGS -L$withval"
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
        esac],
        enable_pktgen=yes)
AC_SUBST([enable_pktgen], [$enable_pktgen])
AM_CONDITIONAL([ENABLE_PKTGEN], [test "$enable_pktgen" = yes])
if test "$enable_pktgen" = yes; then
AC_DEFINE([ENABLE_PKTGEN], [1], [enable support for pktgen])
fi

# FIXME: Replace `main' with a function in `-lzcmdsh':
AC_CHECK_LIB([sdplane], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h locale.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

AC_CHECK_HEADERS([sdplane/libsdplane_version.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset setlocale socket strchr strerror strtol strtoul strtoull])

PKG_CHECK_MODULES(DPDK, [libdpdk])

AC_SUBST([systemdsystemunitdir], [$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])

if test "x${CFLAGS}" = "x"; then
  CFLAGS="-g -O0 -Wall";
fi

tcflags=$(echo $CFLAGS | grep -- -g)
if test "x$tcflags" = x; then
  CFLAGS="-g $CFLAGS"
fi
tcflags=$(echo $CFLAGS | grep -- -O)
if test "x$tcflags" = x; then
  CFLAGS="-O0 $CFLAGS"
fi
tcflags=$(echo $CFLAGS | grep -- -W)
if test "x$tcflags" = x; then
  CFLAGS="$CFLAGS -Wall"
fi

#tcflags=$(echo $CFLAGS | grep -- march)
#if test "x$tcflags" = x; then
#  CFLAGS="$CFLAGS -march=x86-64-v2"
#fi

AC_CONFIG_FILES([Makefile sdplane/Makefile])
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
