## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

lib_LTLIBRARIES = libzcmdsh.la

libzcmdsh_la_SOURCES = \
	log.c termio.c vector.c debug.c debug_cmd.c \
	shell.c command.c command_shell.c file.c shell_fselect.c

include_HEADERS = \
	log.h termio.h vector.h flag.h debug.h \
	shell.h command.h command_shell.h file.h debug_cmd.h

