.PHNOY: run clean boot game

CC := gcc
QEMU := qemu-system-i386
IMG := disk.img

all: $(IMG)

include game/Makefile.part
include boot/Makefile.part

boot: $(boot_BIN)
#game: $(game_BIN)

$(IMG): $(boot_BIN)
	@cat $(boot_BIN) > $(IMG)
	$(call git_cmd)

run: $(IMG)
	$(QEMU) -serial stdio $(IMG)

clean:
	@rm $(IMG)
	@rm -rf obj
