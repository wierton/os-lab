.PHNOY: run clean boot game submit gdb debug

CC := gcc-4.9 # this version is ok
QEMU := qemu-system-i386
IMG := disk.img
info := not found: gcc version 4.9.3 (Ubuntu 4.9.3-5ubuntu1)

all: $(IMG)

include game/Makefile.part
include boot/Makefile.part

boot: $(boot_BIN)
game: $(game_BIN)

$(IMG): $(boot_BIN) $(game_BIN)
	@cat $(boot_BIN) $(game_BIN) > $(IMG)

debug: $(IMG)
	$(QEMU) -S -s -serial stdio -d int -monitor telnet:127.0.0.1:1111,server,nowait $(IMG)

gdb: $(IMG)
	gdb -ex "target remote 127.0.0.1:1234" -ex "symbol $(game_BIN)"

run: $(IMG)
	$(QEMU) -serial stdio $(IMG)

clean:
	@rm -f $(IMG)
	@rm -rf obj

submit: clean
	@cd .. && tar cvj $(shell pwd | grep -o '[^/]*$$') > 141242068.tar.bz2
