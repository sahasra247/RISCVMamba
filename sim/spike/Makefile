# Build the Xmamba Spike extension and host-side checks.
#   make            build/libxmamba.so (load with --extlib=... --extension=xmamba)
#   make check      compare xmamba_ref.c with model/mamba_fixed.py on many inputs
SPIKE_PREFIX ?= /usr/local
PYTHON       ?= python3
BUILD        := build
CFLAGS       := -O2 -fPIC -Wall -Wextra -I. -I$(BUILD)
CXXFLAGS     := -std=c++17 -O2 -fPIC -Wall -I. -I$(BUILD) -I$(SPIKE_PREFIX)/include

.PHONY: all check clean
all: $(BUILD)/libxmamba.so

$(BUILD)/xmamba_luts.h: gen_luts_h.py ../../model/mamba_fixed.py
	$(PYTHON) gen_luts_h.py $@

$(BUILD)/xmamba_ref.o: xmamba_ref.c xmamba_ref.h $(BUILD)/xmamba_luts.h
	$(CC) $(CFLAGS) -c $< -o $@

# Do not link libriscv: the spike binary already contains it and exports its
# symbols; a second copy registers Spike's built-in plugins twice and aborts.
$(BUILD)/libxmamba.so: xmamba_spike.cc xmamba_ref.h $(BUILD)/xmamba_ref.o
	$(CXX) $(CXXFLAGS) -shared xmamba_spike.cc $(BUILD)/xmamba_ref.o -o $@

# Plain host library for ctypes-based checks (no Spike needed).
$(BUILD)/libxmamba_ref.so: $(BUILD)/xmamba_ref.o
	$(CC) -shared $< -o $@

check: $(BUILD)/libxmamba_ref.so
	$(PYTHON) check_ref.py $(BUILD)/libxmamba_ref.so

clean:
	rm -rf $(BUILD)
