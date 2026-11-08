ROOT ?= ../..
BUILD ?= build
CPPFLAGS += -DHAVE_CONFIG_H -I$(ROOT) -I$(ROOT)/sdplane \
            $(shell pkg-config --cflags libdpdk)
CFLAGS = -g -O1 -UNDEBUG -ffunction-sections -fdata-sections \
         -fsanitize=address,undefined -fno-sanitize-recover=all \
         -fno-omit-frame-pointer -fno-pie
LDFLAGS += -no-pie -Wl,--gc-sections -Wl,--wrap=setsockopt -Wl,--wrap=getsockopt
LDLIBS := $(shell pkg-config --libs libdpdk) -llthread -lsdplane -lurcu-qsbr -pthread

.PHONY: check clean
check: $(BUILD)/receive_buffer
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 $<

$(BUILD)/receive_buffer: receive_buffer.c $(ROOT)/sdplane/netlink_thread.c Makefile
	mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP $< $(LDFLAGS) $(LDLIBS) -o $@

-include $(BUILD)/receive_buffer.d

clean:
	rm -rf $(BUILD)
