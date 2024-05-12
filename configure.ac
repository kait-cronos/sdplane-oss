#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sdplane], [0.0.1], [yasu1976@gmail.com])
AC_CONFIG_SRCDIR([sdplane/main.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
# FIXME: Replace `main' with a function in `-llthread':
AC_CHECK_LIB([lthread], [main])
# FIXME: Replace `main' with a function in `-lzcmdsh':
AC_CHECK_LIB([zcmdsh], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h locale.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

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

AC_CONFIG_FILES([Makefile l2fwd/Makefile l3fwd/Makefile sdplane/Makefile])
AC_OUTPUT
