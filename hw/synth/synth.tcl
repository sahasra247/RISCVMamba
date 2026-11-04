# ==============================================================================
# Vivado Out-of-Context Synthesis Script for Xmamba CFU (Artix-7 XC7A100T)
# Part: xc7a100tcsg324-1
# Target: Hardware evaluation for Member A (Sahasra)
# ==============================================================================

set PART "xc7a100tcsg324-1"
set TOP  "Cfu"
set TARGET_PERIOD_NS 20.0  ;# 50 MHz target clock

puts "=========================================================="
puts " Starting Out-of-Context Synthesis for Xmamba CFU"
puts " Target Part:  $PART"
puts " Top Module:   $TOP"
puts " Clock Period: $TARGET_PERIOD_NS ns (50 MHz)"
puts "=========================================================="

# Create in-memory project
create_project -in_memory -part $PART

# Read RTL files
# Use monolithic cfu.v if present; otherwise read separate modules
if {[file exists "../rtl/cfu.v"]} {
    read_verilog "../rtl/cfu.v"
} elseif {[file exists "rtl/cfu.v"]} {
    read_verilog "rtl/cfu.v"
} else {
    read_verilog "../rtl/mb_mac_unit.v"
    read_verilog "../rtl/mb_xmamba_unit.v"
    read_verilog "../rtl/cfu_top.v"
}

# Run synthesis in out-of-context mode
synth_design -top $TOP -part $PART -mode out_of_context -flatten_hierarchy rebuilt

# Create target clock constraint for timing analysis
create_clock -period $TARGET_PERIOD_NS -name clk [get_ports clk]

# Output directory
set REPORT_DIR "reports"
file mkdir $REPORT_DIR

# Generate reports
report_utilization -file "${REPORT_DIR}/utilization_${TOP}.rpt" -pb "${REPORT_DIR}/utilization_${TOP}.pb"
report_timing_summary -file "${REPORT_DIR}/timing_summary_${TOP}.rpt"
report_power -file "${REPORT_DIR}/power_${TOP}.rpt"

puts "=========================================================="
puts " Synthesis complete!"
puts " Reports written to: $REPORT_DIR/"
puts "   - utilization_${TOP}.rpt"
puts "   - timing_summary_${TOP}.rpt"
puts "   - power_${TOP}.rpt"
puts "=========================================================="
