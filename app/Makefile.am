## Process this file with automake to produce Makefile.in

all: $(bin_PROGRAMS)

# .PHONY: opensh_ver.c
# CLEANFILES = opensh_ver.c
FORCE:
.PHONY: FORCE

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = opensh
sbin_PROGRAMS =
if WANT_VTYD
  sbin_PROGRAMS += vtyd
endif

noinst_HEADERS = \
	opensh_ver.h opensh.h \
	vty_server.h vty_shell.h

# opensh_ver.c: opensh-version-c.sh opensh-version.txt
opensh_ver.c: FORCE
	top_srcdir=$(top_srcdir) bash $(top_srcdir)/app/opensh-version-c.sh > opensh_ver.c

opensh_SOURCES = \
	opensh_ver.c opensh.c

opensh_LDFLAGS = \
	-static
#	-Wl,-rpath-link=$(top_srcdir)/lib

opensh_LDADD = \
	../lib/libsdplane.la

vtyd_SOURCES = \
	vtyd.c vty_server.c vty_shell.c

vtyd_LDFLAGS = \
	-static
#	-Wl,-rpath-link=$(top_srcdir)/lib

vtyd_LDADD = \
	../lib/libsdplane.la -llthread

