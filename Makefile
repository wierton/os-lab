.PHNOY: run clean boot game kernel lib submit gdb debug

CC := gcc-4.9 # this version is ok
QEMU := qemu-system-i386
IMG := disk.img
info := not found: gcc version 4.9.3 (Ubuntu 4.9.3-5ubuntu1)
LIB_COMMON_DIR := lib-common/include
LIB_APP_DIR := lib-app/include

all: $(IMG)

include lib-app/Makefile.part
include lib-common/Makefile.part

include game/Makefile.part
include boot/Makefile.part
include kernel/Makefile.part

boot	: $(boot_BIN)
game	: $(game_BIN)
kernel	: $(kernel_BIN)
lib		: $(LIB_COMMON) $(LIB_APP)

$(IMG): $(boot_BIN) $(kernel_BIN) $(game_BIN)
	@cat $(boot_BIN) $(kernel_BIN) $(game_BIN) > $(IMG)

debug: $(IMG)
	$(QEMU) -S -s -serial stdio -d int -monitor telnet:127.0.0.1:1111,server,nowait $(IMG)

gdb: $(IMG)
	gdb -ex "target remote 127.0.0.1:1234" -ex "symbol $(game_BIN)"

run: $(IMG)
	$(QEMU) -serial stdio $(IMG)

clean:
	@rm -f $(IMG) $(LIB_COMMON) $(LIB_APP)
	@rm -rf obj

submit: clean
	@cd .. && tar cvj $(shell pwd | grep -o '[^/]*$$') > 141242068.tar.bz2
