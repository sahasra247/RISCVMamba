/* Spike places DRAM at 0x80000000. 64 MiB is plenty for a <5M-parameter model. */
OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY { RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 64M }

SECTIONS
{
  .text : {
    *(.text.init)
    *(.text .text.*)
  } > RAM

  .rodata : ALIGN(8) { *(.rodata .rodata.* .srodata .srodata.*) } > RAM

  /* HTIF mailbox: Spike finds these by symbol name. */
  .tohost : ALIGN(64) { *(.tohost) } > RAM

  .data : ALIGN(8) {
    *(.data .data.*)
    __global_pointer$ = . + 0x800;
    *(.sdata .sdata.*)
  } > RAM

  .bss (NOLOAD) : ALIGN(8) {
    __bss_start = .;
    *(.sbss .sbss.* .bss .bss.* COMMON)
    . = ALIGN(8);
    __bss_end = .;
  } > RAM

  . = ALIGN(16);
  . += 64K;                 /* stack */
  __stack_top = .;
  _end = .;
}
