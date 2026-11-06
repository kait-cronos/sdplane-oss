ROOT ?= ../..
BUILD ?= build
MODULES := rib_manager internal_message fib fdb mdb
OBJECTS := $(addprefix $(BUILD)/,$(addsuffix .o,$(MODULES)))
REPLACED := $(addprefix $(ROOT)/sdplane/sdplane-,$(addsuffix .o,$(MODULES)))
EXISTING := $(filter-out $(ROOT)/sdplane/sdplane-main.o $(REPLACED),\
              $(wildcard $(ROOT)/sdplane/sdplane-*.o $(ROOT)/module/*/sdplane-*.o))
CPPFLAGS += -DHAVE_CONFIG_H -I$(ROOT) -I$(ROOT)/sdplane -I$(ROOT)/module/common \
            -I$(ROOT)/module/l2fwd -I$(ROOT)/module/l3fwd \
            $(shell pkg-config --cflags libdpdk)
CFLAGS = -g -O1 -UNDEBUG -fsanitize=address,undefined -fno-sanitize-recover=all \
         -fno-omit-frame-pointer -fno-pie $(EXTRA_CFLAGS)
LDFLAGS += -no-pie -Wl,--wrap=malloc -Wl,--wrap=aligned_alloc -Wl,--wrap=fib_new
LDLIBS := $(shell pkg-config --libs libdpdk) -llthread -lsdplane -lurcu-qsbr \
          -lm -lrte_net_bond -lpcap

.PHONY: check clean
check: $(BUILD)/allocation_failure
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 $<

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: $(ROOT)/sdplane/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/allocation_failure: allocation_failure.c $(OBJECTS) $(EXISTING) Makefile $(ROOT)/sdplane/sdplane
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $< $(OBJECTS) $(EXISTING) $(LDFLAGS) $(LDLIBS) -o $@

-include $(wildcard $(BUILD)/*.d)

clean:
	rm -rf $(BUILD)
