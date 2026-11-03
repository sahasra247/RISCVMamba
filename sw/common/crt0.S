/* Bare-metal start-up for RV32IM on Spike (and later the CFU Playground SoC).
 * Runs in M-mode: set gp/sp, zero .bss, call main, exit with its return value. */
    .section .text.init, "ax"
    .globl _start
_start:
    .option push
    .option norelax
    la   gp, __global_pointer$
    .option pop
    la   sp, __stack_top

    la   t0, __bss_start
    la   t1, __bss_end
1:  bgeu t0, t1, 2f
    sw   zero, 0(t0)
    addi t0, t0, 4
    j    1b
2:
    la   t0, trap_entry
    csrw mtvec, t0

    call main
    call bm_exit          /* never returns */

/* Any trap (illegal instruction, misaligned access, ...) ends the run with
 * a distinctive exit code so a test script sees it as a failure. */
    .align 2
trap_entry:
    csrr a0, mcause
    call bm_trap
3:  j 3b
