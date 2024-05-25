
sbin_PROGRAMS = l3fwd

l3fwd_SOURCES = \
	main.c l3fwd_acl.c l3fwd_lpm.c l3fwd_fib.c l3fwd_em.c l3fwd_event.c \
	l3fwd_event_generic.c l3fwd_event_internal_port.c

l3fwd_CFLAGS = -O3 @DPDK_CFLAGS@ -I../common -DALLOW_EXPERIMENTAL_API
l3fwd_LDADD = @DPDK_LIBS@

