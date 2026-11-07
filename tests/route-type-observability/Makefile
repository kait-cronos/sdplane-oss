ROOT ?= ../..
BUILD ?= build
TESTS := route_type show_route_type srv6_route_type main_local_packets
MODULES := rib_manager internal_message radix fib rib nexthop fdb mdb netlink_thread srv6_cmd
OBJECTS := $(addprefix $(BUILD)/,$(addsuffix .o,$(MODULES)))
REPLACED := $(addprefix $(ROOT)/sdplane/sdplane-,$(addsuffix .o,$(MODULES)))
EXISTING := $(filter-out $(ROOT)/sdplane/sdplane-main.o $(REPLACED),\
              $(wildcard $(ROOT)/sdplane/sdplane-*.o $(ROOT)/module/*/sdplane-*.o))
CPPFLAGS += -DHAVE_CONFIG_H -I$(ROOT) -I$(ROOT)/sdplane -I$(ROOT)/module/common \
            -I$(ROOT)/module/l2fwd -I$(ROOT)/module/l3fwd \
            $(shell pkg-config --cflags libdpdk)
CFLAGS = -g -O1 -UNDEBUG -fsanitize=address,undefined -fno-sanitize-recover=all \
         -fno-omit-frame-pointer -fno-pie $(EXTRA_CFLAGS)
LDFLAGS += -no-pie
LDLIBS := $(shell pkg-config --libs libdpdk) -llthread -lsdplane -lurcu-qsbr \
          -lm -lrte_net_bond -lpcap

.PHONY: check clean
check: $(addprefix $(BUILD)/,$(TESTS))
	@set -e; for test in $^; do \
	  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	  UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$$test"; \
	done

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: $(ROOT)/sdplane/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/%: %.c support.h $(OBJECTS) $(EXISTING) Makefile $(ROOT)/sdplane/sdplane
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $< $(OBJECTS) $(EXISTING) $(LDFLAGS) $(LDLIBS) -o $@

.SECONDARY: $(OBJECTS)
-include $(wildcard $(BUILD)/*.d)

clean:
	rm -rf $(BUILD)
