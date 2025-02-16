
AM_CFLAGS = -g -O0

COMMON_DIR = $(top_srcdir)/module/common
SDPLANE_DIR = $(top_srcdir)/sdplane
L3FWD_MODULE_DIR = $(top_srcdir)/module/l3fwd
L2FWD_MODULE_DIR = $(top_srcdir)/module/l2fwd

sbin_PROGRAMS = sdplane
sdplane_SOURCES = \
        $(L3FWD_MODULE_DIR)/l3fwd_acl.c $(L3FWD_MODULE_DIR)/l3fwd_lpm.c \
        $(L3FWD_MODULE_DIR)/l3fwd_fib.c $(L3FWD_MODULE_DIR)/l3fwd_em.c \
        $(L3FWD_MODULE_DIR)/l3fwd_event.c \
        $(L3FWD_MODULE_DIR)/l3fwd_event_generic.c \
        $(L3FWD_MODULE_DIR)/l3fwd_event_internal_port.c \
        $(L3FWD_MODULE_DIR)/l3fwd.c \
        $(L3FWD_MODULE_DIR)/l3fwd_cmd.c $(L3FWD_MODULE_DIR)/l3fwd_cmd.h \
        $(L2FWD_MODULE_DIR)/l2fwd.c \
        $(L2FWD_MODULE_DIR)/l2fwd_cmd.c $(L2FWD_MODULE_DIR)/l2fwd_cmd.h \
        tap.c tap_handler.h tap_handler.c stat_collector.c \
        startup_config.c snprintf_flags.c dpdk_flag.h \
        dpdk_lcore_cmd.c dpdk_port_cmd.c \
        vty_server.c vty_shell.c \
        main.c lthread_main.c sdplane.h sdplane.c debug_sdplane.c \
        console_shell.c l2_repeater.c \
        sdplane_version.c thread_info.c queue_config.c rib_manager.c \
        internal_message.c rib.c nettlp.c nettlp_support.c tap_cmd.c
sdplane_CFLAGS = @DPDK_CFLAGS@ -I$(COMMON_DIR) -I$(L2FWD_MODULE_DIR) -I$(L3FWD_MODULE_DIR)
sdplane_LDADD = @DPDK_LIBS@ -llthread -L$(zcmdshdir) -lzcmdsh -lurcu-qsbr

FORCE:
.PHONY: FORCE clean-version

sdplane_version.c: FORCE
	top_srcdir=$(top_srcdir) bash $(top_srcdir)/sdplane/sdplane-version-c.sh > sdplane_version.c

clean-local: clean-version
clean-version:
	-rm -f sdplane_version.c

