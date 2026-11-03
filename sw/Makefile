# Bare-metal RV32IM programs for Spike.
#   make            build every test in tests/
#   make run        run them all on Spike (non-zero exit on any failure)
#   make run-hello  run one test
# Override the toolchain prefix if yours differs, e.g. make CROSS=riscv32-unknown-elf-

CROSS   ?= riscv64-unknown-elf-
CC      := $(CROSS)gcc
OBJDUMP := $(CROSS)objdump
SPIKE   ?= spike
ARCH    := rv32im_zicsr
ABI     := ilp32

BUILD   := build
CFLAGS  := -march=$(ARCH) -mabi=$(ABI) -O2 -g -Wall -Wextra \
           -ffreestanding -fno-builtin -nostdlib -nostartfiles -mcmodel=medany \
           -Icommon
LDFLAGS := -T common/link.ld -Wl,--gc-sections -Wl,--no-warn-rwx-segments -lgcc

COMMON  := common/crt0.S common/bm_spike.c
TESTS   := $(basename $(notdir $(wildcard tests/*.c)))
ELFS    := $(addprefix $(BUILD)/,$(addsuffix .elf,$(TESTS)))

.PHONY: all run clean $(addprefix run-,$(TESTS))
all: $(ELFS)

$(BUILD)/%.elf: tests/%.c $(COMMON) common/bm.h common/link.ld | $(BUILD)
	$(CC) $(CFLAGS) $(COMMON) $< -o $@ $(LDFLAGS)
	$(OBJDUMP) -d $@ > $(BUILD)/$*.dis

$(BUILD):
	mkdir -p $@

run-%: $(BUILD)/%.elf
	$(SPIKE) --isa=$(ARCH) $<

run: $(ELFS)
	@set -e; for t in $(TESTS); do echo "== $$t"; $(SPIKE) --isa=$(ARCH) $(BUILD)/$$t.elf; done

clean:
	rm -rf $(BUILD)
