
sbin_PROGRAMS = sdplane
sdplane_SOURCES = \
        main.c ../l3fwd/l3fwd_acl.c ../l3fwd/l3fwd_lpm.c ../l3fwd/l3fwd_fib.c ../l3fwd/l3fwd_em.c ../l3fwd/l3fwd_event.c \
        ../l3fwd/l3fwd_event_generic.c ../l3fwd/l3fwd_event_internal_port.c \
        lthread_main.c l2fwd_main.c l2fwd_init.c l2fwd_cmd.c soft_dplane.c
sdplane_CFLAGS = @DPDK_CFLAGS@ -I../common -I../l2fwd -I../l3fwd -I/home/yasu/local/include
sdplane_LDADD = @DPDK_LIBS@ -llthread -L/home/yasu/local/lib/x86_64-linux-gnu -lzcmdsh

