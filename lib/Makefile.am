## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(top_srcdir)/lib

lib_LTLIBRARIES = libzcmdsh.la

libzcmdsh_la_SOURCES = \
	include.h \
	zcmdsh/log.c zcmdsh/vector.c zcmdsh/termio.c \
	zcmdsh/shell.c zcmdsh/command.c zcmdsh/command_shell.c \
	zcmdsh/file.c zcmdsh/shell_fselect.c \
	zcmdsh/debug.c zcmdsh/debug_cmd.c \
	zcmdsh/debug_module.c zcmdsh/debug_module_cmd.c \
	zcmdsh/log_cmd.c

nobase_include_HEADERS = \
	zcmdsh/log.h zcmdsh/vector.h zcmdsh/flag.h \
	zcmdsh/shell.h zcmdsh/command.h zcmdsh/command_shell.h \
	zcmdsh/debug.h zcmdsh/debug_cmd.h \
	zcmdsh/debug_module.h zcmdsh/debug_module_cmd.h zcmdsh/termio.h \
	zcmdsh/log_cmd.h
