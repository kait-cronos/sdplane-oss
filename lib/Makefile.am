## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(top_srcdir)/lib

lib_LTLIBRARIES = libzcmdsh.la

libzcmdsh_la_SOURCES = \
	include.h zcmdsh/flag.h \
	zcmdsh/debug.c zcmdsh/log.c zcmdsh/vector.c \
	zcmdsh/termio.h zcmdsh/termio.c \
	zcmdsh/shell.c zcmdsh/command.c zcmdsh/command_shell.c \
	zcmdsh/file.c zcmdsh/shell_fselect.c \
	zcmdsh/debug_cmd.c

nobase_include_HEADERS = \
	zcmdsh/log.h zcmdsh/vector.h zcmdsh/debug.h \
	zcmdsh/shell.h zcmdsh/command.h zcmdsh/command_shell.h \
	zcmdsh/debug_cmd.h
