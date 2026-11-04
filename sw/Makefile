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

.PHONY: all run clean mamba-float mamba-fixed mamba-x-mac mamba-x-macnl mamba-x compare FORCE $(addprefix run-,$(TESTS))
all: $(ELFS) $(BUILD)/mamba_float.elf $(BUILD)/mamba_fixed.elf

# ------------------------------------------------------------ Mamba engine
# Sizes for random weights (until the real model exists), or WEIGHTS=<dir of .npy>.
#   make mamba-float                          small, fast check (d=64, 2 layers)
#   make mamba-float D_MODEL=256 N_LAYER=4    the planned model size
D_MODEL ?= 64
N_LAYER ?= 2
VOCAB   ?= 256
NTOK    ?= 8
WEIGHTS ?=
PYTHON  ?= python3
GEN     := $(BUILD)/gen
EXPORT_ARGS := $(if $(WEIGHTS),--weights $(WEIGHTS),--random --d-model $(D_MODEL) --n-layer $(N_LAYER) --vocab $(VOCAB)) --tokens $(NTOK)
ENGINE  := engine/mamba_f32.c engine/fmath.c

# Re-export only when the arguments or the generators change.
$(GEN)/args: FORCE
	@mkdir -p $(GEN)
	@echo '$(EXPORT_ARGS)' | cmp -s - $@ || echo '$(EXPORT_ARGS)' > $@

$(GEN)/stamp: $(GEN)/args tools/export_weights.py ../model/mamba_step.py
	$(PYTHON) tools/export_weights.py $(EXPORT_ARGS) --out $(GEN)
	@touch $@

$(BUILD)/mamba_float.elf: apps/mamba_float.c $(ENGINE) $(wildcard engine/*.h) $(COMMON) common/bm.h common/link.ld $(GEN)/stamp
	$(CC) $(CFLAGS) -Iengine -I$(GEN) $(COMMON) $(ENGINE) apps/mamba_float.c $(GEN)/blobs.S -o $@ $(LDFLAGS)
	$(OBJDUMP) -d $@ > $(BUILD)/mamba_float.dis

mamba-float: $(BUILD)/mamba_float.elf
	$(SPIKE) --isa=$(ARCH) $<

# Fixed-point engine: bit-exact with model/mamba_fixed.py, plus kernel profile.
# Four builds from one source (the ablation table):
#   make mamba-fixed      RV32IM software baseline
#   make mamba-x-mac      + MAC unit             (needs the Spike extension)
#   make mamba-x-macnl    + MAC + nonlinear unit
#   make mamba-x          full Xmamba
#   make compare          all four, then the instruction-count table
# Add D_MODEL=256 N_LAYER=4 for the planned model size.
GENQ    := $(BUILD)/genq
ENGINEQ := engine/mamba_q.c
XMDIR   := ../sim/spike
XM_SRC  := $(XMDIR)/xmamba_ref.c
LUTS    := $(BUILD)/xm/xmamba_luts.h
XLIB    := $(XMDIR)/build/libxmamba.so
SPIKE_X := $(SPIKE) --isa=$(ARCH) --extlib=$(XLIB) --extension=xmamba

$(GENQ)/args: FORCE
	@mkdir -p $(GENQ)
	@echo '$(EXPORT_ARGS)' | cmp -s - $@ || echo '$(EXPORT_ARGS)' > $@

$(GENQ)/stamp: $(GENQ)/args tools/export_weights.py ../model/mamba_step.py ../model/mamba_fixed.py
	$(PYTHON) tools/export_weights.py --fixed $(EXPORT_ARGS) --out $(GENQ)
	@touch $@

$(LUTS): $(XMDIR)/gen_luts_h.py ../model/mamba_fixed.py
	$(PYTHON) $(XMDIR)/gen_luts_h.py $@

$(XLIB): FORCE
	@$(MAKE) -s -C $(XMDIR)

QDEPS := apps/mamba_fixed.c $(ENGINEQ) engine/mamba_q.h engine/xmamba.h $(XM_SRC) $(XMDIR)/xmamba_ref.h \
         $(LUTS) $(COMMON) common/bm.h common/link.ld $(GENQ)/stamp

define QBUILD
$(BUILD)/mamba_$(1).elf: $(QDEPS)
	$(CC) $(CFLAGS) $(2) -Iengine -I$(GENQ) -I$(XMDIR) -I$(BUILD)/xm $(COMMON) $(ENGINEQ) \
	    apps/mamba_fixed.c $(GENQ)/blobs.S -o $$@ $(LDFLAGS)
	$(OBJDUMP) -d $$@ > $$(@:.elf=.dis)
endef
$(eval $(call QBUILD,fixed,))
$(eval $(call QBUILD,x_mac,-DXM_MAC))
$(eval $(call QBUILD,x_macnl,-DXM_MAC -DXM_NL))
$(eval $(call QBUILD,xmamba,-DXM_MAC -DXM_NL -DXM_SCAN))

mamba-fixed: $(BUILD)/mamba_fixed.elf
	$(SPIKE) --isa=$(ARCH) $<
mamba-x-mac: $(BUILD)/mamba_x_mac.elf $(XLIB)
	$(SPIKE_X) $<
mamba-x-macnl: $(BUILD)/mamba_x_macnl.elf $(XLIB)
	$(SPIKE_X) $<
mamba-x: $(BUILD)/mamba_xmamba.elf $(XLIB)
	$(SPIKE_X) $<

# Run all four builds, keep the logs, and print the comparison table.
compare: $(BUILD)/mamba_fixed.elf $(BUILD)/mamba_x_mac.elf $(BUILD)/mamba_x_macnl.elf $(BUILD)/mamba_xmamba.elf $(XLIB)
	$(SPIKE) --isa=$(ARCH) $(BUILD)/mamba_fixed.elf > $(BUILD)/log_baseline.txt
	$(SPIKE_X) $(BUILD)/mamba_x_mac.elf   > $(BUILD)/log_mac.txt
	$(SPIKE_X) $(BUILD)/mamba_x_macnl.elf > $(BUILD)/log_macnl.txt
	$(SPIKE_X) $(BUILD)/mamba_xmamba.elf  > $(BUILD)/log_xmamba.txt
	$(PYTHON) tools/compare_counts.py $(BUILD)/log_baseline.txt $(BUILD)/log_mac.txt \
	    $(BUILD)/log_macnl.txt $(BUILD)/log_xmamba.txt

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
