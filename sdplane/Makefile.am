
COMMON_DIR = ../common
L2FWD_DIR = ../l2fwd
L3FWD_DIR = ../l3fwd

sbin_PROGRAMS = sdplane
sdplane_SOURCES = \
        $(L3FWD_DIR)/l3fwd_acl.c $(L3FWD_DIR)/l3fwd_lpm.c \
        $(L3FWD_DIR)/l3fwd_fib.c $(L3FWD_DIR)/l3fwd_em.c \
        $(L3FWD_DIR)/l3fwd_event.c \
        $(L3FWD_DIR)/l3fwd_event_generic.c \
        $(L3FWD_DIR)/l3fwd_event_internal_port.c \
        l2fwd.c l2fwd_cmd.c \
        main.c lthread_main.c sdplane.h sdplane.c
sdplane_CFLAGS = @DPDK_CFLAGS@ -I$(COMMON_DIR) -I$(L2FWD_DIR) -I$(L3FWD_DIR)
sdplane_LDADD = @DPDK_LIBS@ -llthread -lzcmdsh

