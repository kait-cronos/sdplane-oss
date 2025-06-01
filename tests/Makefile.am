
AM_CFLAGS = -Wno-format-truncation

COMMON_DIR = $(top_srcdir)/module/common
SDPLANE_DIR = $(top_srcdir)/sdplane
L3FWD_MODULE_DIR = $(top_srcdir)/module/l3fwd
L2FWD_MODULE_DIR = $(top_srcdir)/module/l2fwd
PKTGEN_MODULE_DIR = $(top_srcdir)/module/pktgen

COMMON_SOURCES = $(L3FWD_MODULE_DIR)/l3fwd_acl.c $(L3FWD_MODULE_DIR)/l3fwd_lpm.c \
        $(L3FWD_MODULE_DIR)/l3fwd_fib.c $(L3FWD_MODULE_DIR)/l3fwd_em.c \
        $(L3FWD_MODULE_DIR)/l3fwd_event.c \
        $(L3FWD_MODULE_DIR)/l3fwd_event_generic.c \
        $(L3FWD_MODULE_DIR)/l3fwd_event_internal_port.c \
        $(L3FWD_MODULE_DIR)/l3fwd.c \
        $(L3FWD_MODULE_DIR)/l3fwd_cmd.c $(L3FWD_MODULE_DIR)/l3fwd_cmd.h \
        $(L2FWD_MODULE_DIR)/l2fwd.c \
        $(L2FWD_MODULE_DIR)/l2fwd_cmd.c $(L2FWD_MODULE_DIR)/l2fwd_cmd.h \
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
        $(SDPLANE_DIR)/tap.c $(SDPLANE_DIR)/tap_handler.h $(SDPLANE_DIR)/tap_handler.c $(SDPLANE_DIR)/stat_collector.c \
        $(SDPLANE_DIR)/startup_config.c $(SDPLANE_DIR)/snprintf_flags.c $(SDPLANE_DIR)/dpdk_flag.h \
        $(SDPLANE_DIR)/dpdk_lcore_cmd.c $(SDPLANE_DIR)/dpdk_port_cmd.c \
        $(SDPLANE_DIR)/vty_server.c $(SDPLANE_DIR)/vty_shell.c \
        $(SDPLANE_DIR)/lthread_main.c $(SDPLANE_DIR)/sdplane.h $(SDPLANE_DIR)/sdplane.c $(SDPLANE_DIR)/debug_sdplane.c \
        $(SDPLANE_DIR)/console_shell.c $(SDPLANE_DIR)/l2_repeater.c \
        $(SDPLANE_DIR)/sdplane_version.c $(SDPLANE_DIR)/thread_info.c $(SDPLANE_DIR)/queue_config.c $(SDPLANE_DIR)/rib_manager.c \
        $(SDPLANE_DIR)/internal_message.c $(SDPLANE_DIR)/rib.c $(SDPLANE_DIR)/nettlp.c $(SDPLANE_DIR)/nettlp_support.c $(SDPLANE_DIR)/tap_cmd.c \
        $(SDPLANE_DIR)/dpdk_devbind.c $(SDPLANE_DIR)/netlink_thread.c $(SDPLANE_DIR)/stream_buf.c $(SDPLANE_DIR)/vlan_switch.c \
        $(SDPLANE_DIR)/pktgen_cmd.c

bin_PROGRAMS = test_l2repeater.c
test_l2repeater_SOURCES = $(COMMON_SOURCES) test_l2repeater.c

test_l2repeater_CFLAGS = @DPDK_CFLAGS@ -I$(COMMON_DIR) -I$(L2FWD_MODULE_DIR) -I$(L3FWD_MODULE_DIR) -I$(PKTGEN_MODULE_DIR)/lib/common -I$(PKTGEN_MODULE_DIR)/lib/lua -I$(PKTGEN_MODULE_DIR)/lib/cli -I$(PKTGEN_MODULE_DIR)/lib/utils -I$(PKTGEN_MODULE_DIR)/lib/plugin $(AM_CFLAGS)
test_l2repeater_LDADD = @DPDK_LIBS@ -llthread -L$(zcmdshdir) -lsdplane -lurcu-qsbr -lm -lrte_net_bond -lpcap
