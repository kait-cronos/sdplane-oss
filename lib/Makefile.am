## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(top_srcdir)/lib

lib_LTLIBRARIES = libsdplane.la

libsdplane_la_SOURCES = \
	include.h \
	sdplane/log.c sdplane/vector.c sdplane/termio.c \
	sdplane/shell.c sdplane/command.c sdplane/command_shell.c \
	sdplane/file.c sdplane/shell_fselect.c \
	sdplane/debug.c sdplane/debug_cmd.c \
	sdplane/log_cmd.c sdplane/shell_keyfunc.c sdplane/debug_log.c \
	sdplane/debug_zcmdsh.c \
	sdplane/shell_telnet.c sdplane/exec_limit.c \
        sdplane/shell_debug.c

nobase_include_HEADERS = \
	sdplane/log.h sdplane/vector.h sdplane/termio.h sdplane/flag.h \
	sdplane/shell.h sdplane/command.h sdplane/command_shell.h \
	sdplane/debug.h sdplane/debug_cmd.h \
	sdplane/log_cmd.h sdplane/shell_keyfunc.h \
        sdplane/debug_log.h sdplane/debug_category.h sdplane/debug_zcmdsh.h \
	sdplane/debug_backtrace.h \
        sdplane/shell_telnet.h sdplane/exec_limit.h \
        sdplane/shell_debug.h
