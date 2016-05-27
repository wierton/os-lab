.PHNOY: count run clean boot game kernel idle lib submit gdb debug test disk

CC := gcc-4.9 # this version is ok
LD := ld
QEMU := qemu-system-i386
IMG := disk.img
LIB_COMMON_DIR := libcommon/include
LIB_APP_DIR := libapp/include

all: $(IMG)

include config/Makefile.build
include format/Makefile.part

include libapp/Makefile.part
include libcommon/Makefile.part

include game/Makefile.part
include boot/Makefile.part
include idle/Makefile.part
include kernel/Makefile.part

# some testcase here

# TESTCASE := helloworld
# TESTCASE := fork
# TESTCASE := fork_sleep
# TESTCASE := exit
# TESTCASE := thread
TESTCASE := sem

include testcase/Makefile.part

boot	: $(boot_IMG)
game	: $(game_BIN)
kernel	: $(kernel_BIN)
test	: $(test_BIN)
idle	: $(idle_BIN)
lib		: $(LIB_COMMON) $(LIB_APP)
format  : $(format_BIN)

ENTRY := $(test_BIN)
# ENTRY := $(game_BIN)

$(IMG): $(boot_IMG) $(kernel_BIN) $(idle_BIN) $(ENTRY)
	@cat $(boot_IMG) $(kernel_BIN) $(idle_BIN) $(ENTRY) > $(IMG)

disk: $(format_BIN) $(boot_IMG) $(kernel_BIN) $(idle_BIN) $(test_BIN) $(game_BIN)
	@rm -rf $(IMG)
	@$(format_BIN) $(IMG) $(boot_IMG) $(kernel_BIN) $(idle_BIN) $(test_BIN) $(game_BIN)

debug: $(IMG)
	$(QEMU) -S -s -serial stdio -d int -monitor telnet:127.0.0.1:1111,server,nowait $(IMG)

gdb: $(IMG)
	@gdb -ex "target remote 127.0.0.1:1234" -ex "symbol $(kernel_BIN)"

run: $(IMG)
	$(QEMU) -serial stdio $(IMG)

clean:
	@rm -f $(IMG) $(LIB_COMMON) $(LIB_APP)
	@rm -rf obj

count:
	@printf "lines:"
	@find . -name "*.c" -or -name "*.h" -or -name "Makefile*" | xargs cat | wc -l

submit: clean
	@cd .. && tar cvj $(shell pwd | grep -o '[^/]*$$') > 141242068.tar.bz2
