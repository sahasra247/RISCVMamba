/* Xmamba intrinsics: one inline function per instruction (spec/xmamba-isa.md).
 * All are custom-0 R-type, emitted with the assembler's .insn directive, so
 * no compiler changes are needed. They are volatile because the unit holds
 * state (accumulator, scan memory) the compiler cannot see. */
#ifndef XMAMBA_H
#define XMAMBA_H

#include <stdint.h>

#define XM_INSN_R0(f3, f7, a, b) \
    __asm__ volatile(".insn r CUSTOM_0, " #f3 ", " #f7 ", x0, %0, %1" :: "r"(a), "r"(b))
#define XM_INSN_R(f3, f7, rd, a, b) \
    __asm__ volatile(".insn r CUSTOM_0, " #f3 ", " #f7 ", %0, %1, %2" : "=r"(rd) : "r"(a), "r"(b))

/* MAC group (funct3 = 0) */
static inline void mb_mac4(uint32_t a, uint32_t b) { XM_INSN_R0(0, 0, a, b); }
static inline int32_t mb_racc(int32_t shift)
{ int32_t r; XM_INSN_R(0, 1, r, shift, 0); return r; }

/* Nonlinear group (funct3 = 1) */
static inline int32_t mb_exp(int32_t x_q24)      { int32_t r; XM_INSN_R(1, 0, r, x_q24, 0); return r; }
static inline int32_t mb_silu(int32_t x_q16)     { int32_t r; XM_INSN_R(1, 1, r, x_q16, 0); return r; }
static inline int32_t mb_softplus(int32_t v_q16) { int32_t r; XM_INSN_R(1, 2, r, v_q16, 0); return r; }

/* Scan group (funct3 = 2) */
static inline void mb_rst(void)                       { XM_INSN_R0(2, 0, 0, 0); }
static inline void mb_layer(uint32_t base_channel)    { XM_INSN_R0(2, 1, base_channel, 0); }
static inline void mb_lda(uint32_t index, int32_t v)  { XM_INSN_R0(2, 2, index, v); }
static inline void mb_ldbc(uint32_t sel_n, int32_t v) { XM_INSN_R0(2, 3, sel_n, v); }
static inline int32_t mb_scan(int32_t delta_q24, int32_t u_q16)
{ int32_t r; XM_INSN_R(2, 4, r, delta_q24, u_q16); return r; }
static inline int32_t mb_rdh(uint32_t index)          { int32_t r; XM_INSN_R(2, 5, r, index, 0); return r; }

#define MB_LDA_D  0x80000000u   /* mb.lda: write D[index] instead of A[index] */
#define MB_LDBC_C 0x100u        /* mb.ldbc: write C[n] instead of B[n] */

#endif
