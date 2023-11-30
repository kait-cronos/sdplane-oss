## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

noinst_LIBRARIES = libzcmdsh.a

libzcmdsh_a_SOURCES = \
	log.c termio.c vector.c debug.c debug_cmd.c \
	shell.c command.c command_shell.c file.c shell_fselect.c

noinst_HEADERS = \
	log.h termio.h vector.h flag.h debug.h \
	shell.h command.h command_shell.h file.h

