
AM_CFLAGS = -Wno-format-truncation

COMMON_DIR = $(top_srcdir)/module/common
SDPLANE_DIR = $(top_srcdir)/sdplane
L3FWD_MODULE_DIR = $(top_srcdir)/module/l3fwd
L2FWD_MODULE_DIR = $(top_srcdir)/module/l2fwd
PKTGEN_MODULE_DIR = $(top_srcdir)/module/pktgen

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
        internal_message.c rib.c tap_cmd.c \
        dpdk_devbind.c netlink_thread.c stream_buf.c vlan_switch.c \
        linkflap_generator.c \
        enhanced_repeater.c l3_tap_handler.c neigh_manager.c
sdplane_CFLAGS = @DPDK_CFLAGS@ -I$(COMMON_DIR) -I$(L2FWD_MODULE_DIR) -I$(L3FWD_MODULE_DIR) $(AM_CFLAGS)
sdplane_LDADD = ../lib/libsdplane.la @DPDK_LIBS@ -llthread -lurcu-qsbr -lm -lrte_net_bond -lpcap

if ENABLE_PKTGEN
  sdplane_SOURCES += \
        $(PKTGEN_MODULE_DIR)/app/pktgen.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-main.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-display.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-range.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-rate.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-latency.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-pcap.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-cmds.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-dump.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-capture.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-log.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-random.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-ether.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-gre.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-gtpu.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-arp.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-vlan.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-ipv4.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-ipv6.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-udp.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-tcp.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-port-cfg.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-stats.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-cpu.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-cfg.c \
        $(PKTGEN_MODULE_DIR)/app/pktgen-seq.c \
        $(PKTGEN_MODULE_DIR)/app/cli-functions.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_env.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_map.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_help.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_search.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_scrn.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_history.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_gapbuf.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_vt100.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_cmds.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_input.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_auto_complete.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_cmap.c \
        $(PKTGEN_MODULE_DIR)/lib/cli/cli_file.c \
        $(PKTGEN_MODULE_DIR)/lib/common/copyright_info.c \
        $(PKTGEN_MODULE_DIR)/lib/common/pg_strings.c \
        $(PKTGEN_MODULE_DIR)/lib/common/pg_pcap.c \
        $(PKTGEN_MODULE_DIR)/lib/common/l2p.c \
        $(PKTGEN_MODULE_DIR)/lib/common/coremap.c \
        $(PKTGEN_MODULE_DIR)/lib/common/utils.c \
        $(PKTGEN_MODULE_DIR)/lib/common/lscpu.c \
        $(PKTGEN_MODULE_DIR)/lib/common/port_config.c \
        $(PKTGEN_MODULE_DIR)/lib/utils/portlist.c \
        $(PKTGEN_MODULE_DIR)/lib/utils/_atoip.c \
        $(PKTGEN_MODULE_DIR)/lib/plugin/plugin.c \
        pktgen_cmd.c
  sdplane_CFLAGS += -D_GNU_SOURCE -I$(PKTGEN_MODULE_DIR)/lib/common -I$(PKTGEN_MODULE_DIR)/lib/lua -I$(PKTGEN_MODULE_DIR)/lib/cli -I$(PKTGEN_MODULE_DIR)/lib/utils -I$(PKTGEN_MODULE_DIR)/lib/plugin -I$(top_srcdir)/lib
endif

FORCE:
.PHONY: FORCE clean-version

sdplane_version.c: FORCE
	top_srcdir=$(top_srcdir) bash $(top_srcdir)/sdplane/sdplane-version-c.sh > sdplane_version.c

clean-local: clean-version
clean-version:
	-rm -f sdplane_version.c

