.PHNOY: count run clean boot game kernel lib submit gdb debug

CC := gcc-4.9 # this version is ok
LD := ld
QEMU := qemu-system-i386
IMG := disk.img
info := not found: gcc version 4.9.3 (Ubuntu 4.9.3-5ubuntu1)
LIB_COMMON_DIR := libcommon/include
LIB_APP_DIR := libapp/include

all: $(IMG)

include config/Makefile.build

include libapp/Makefile.part
include libcommon/Makefile.part

include game/Makefile.part
include boot/Makefile.part
include kernel/Makefile.part

boot	: $(boot_IMG)
game	: $(game_BIN)
kernel	: $(kernel_BIN)
lib		: $(LIB_COMMON) $(LIB_APP)

$(IMG): $(boot_IMG) $(kernel_BIN) $(game_BIN)
	@cat $(boot_IMG) $(kernel_BIN) $(game_BIN) > $(IMG)

debug: $(IMG)
	$(QEMU) -S -s -serial stdio -d int -monitor telnet:127.0.0.1:1111,server,nowait $(IMG)

gdb: $(IMG)
	gdb -ex "target remote 127.0.0.1:1234" -ex "symbol $(kernel_BIN)"

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
