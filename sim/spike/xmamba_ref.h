/* Xmamba reference semantics: the single source of truth for what every
 * Xmamba instruction computes, bit for bit (spec/xmamba-isa.md).
 *
 *   - Spike calls xm_exec() for each custom-0 instruction.
 *   - The RV32IM software baseline uses the same arithmetic functions.
 *   - RTL test vectors are generated from these functions.
 * It is plain C99 (no OS, no malloc) so it builds for the host and for RV32.
 * Bit-exact with model/mamba_fixed.py. */
#ifndef XMAMBA_REF_H
#define XMAMBA_REF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------- encoding */
#define XM_OPCODE      0x0B          /* custom-0 */
#define XM_F3_MAC      0
#define XM_F3_NL       1
#define XM_F3_SCAN     2

#define XM_F7_MAC4     0             /* funct3 = MAC  */
#define XM_F7_RACC     1
#define XM_F7_EXP      0             /* funct3 = NL   */
#define XM_F7_SILU     1
#define XM_F7_SOFTPLUS 2
#define XM_F7_RST      0             /* funct3 = SCAN */
#define XM_F7_LAYER    1
#define XM_F7_LDA      2
#define XM_F7_LDBC     3
#define XM_F7_SCAN     4
#define XM_F7_RDH      5

/* ------------------------------------------------------------------ unit */
#define XM_N       16                /* state size per channel (fixed in v1) */
/* Channels over all layers (layers x E*d). 2048 fits the planned model exactly
 * (4 x 512) and costs 2048*16*4 B for A + the same for h + 2048*4 B for D
 * = 264 KiB of the Arty A7-100T's 607.5 KiB block RAM. A synthesis parameter. */
#ifndef XM_MAX_CH
#define XM_MAX_CH  2048
#endif
#define XM_LDA_D   0x80000000u       /* mb.lda rs1 bit 31: 0 = A[index], 1 = D[index] */
#define XM_LDBC_C  0x100u            /* mb.ldbc rs1 bit 8: 0 = B[n], 1 = C[n] */

typedef struct {
    int32_t  acc;                    /* MAC accumulator */
    uint32_t base, chan;             /* scan: first channel of the layer, channel counter */
    int32_t  B[XM_N], C[XM_N];       /* Q16, this token */
    int32_t  A[XM_MAX_CH * XM_N];    /* Q16, = -exp(A_log) */
    int32_t  D[XM_MAX_CH];           /* Q16 */
    int32_t  h[XM_MAX_CH * XM_N];    /* Q20 scan state */
} xm_unit_t;

void xm_reset(xm_unit_t *u);         /* power-on: everything zero */

/* Execute one instruction. Returns 0, or -1 for an undefined encoding or an
 * out-of-range index (Spike raises illegal-instruction; RTL behaviour is
 * undefined). *rd receives the result (0 for instructions without one). */
int xm_exec(xm_unit_t *u, unsigned funct3, unsigned funct7,
            uint32_t rs1, uint32_t rs2, uint32_t *rd);

/* ----------------------------------------------------- shared arithmetic */
/* Small helpers are inline so the software baseline is not slowed by calls. */
static inline int64_t xm_rshr(int64_t v, int s)    /* round half up; s <= 0 shifts left */
{
    if (s > 0) return (int64_t)((uint64_t)v + ((uint64_t)1 << (s - 1))) >> s;
    return (int64_t)((uint64_t)v << -s);
}
static inline int32_t xm_sat32(int64_t v)
{
    return v > INT32_MAX ? INT32_MAX : v < INT32_MIN ? INT32_MIN : (int32_t)v;
}
static inline int32_t xm_wrap32(int64_t v) { return (int32_t)(uint32_t)(uint64_t)v; }
static inline int32_t xm_racc_value(int32_t acc, int32_t sh)   /* sat32(rshr(acc, clamp(sh))) */
{
    if (sh < -31) sh = -31;
    if (sh > 31) sh = 31;
    return xm_sat32(xm_rshr(acc, sh));
}

int32_t xm_exp(int32_t x_q24);       /* exp(x), x <= 0 (clamped)    -> Q30 */
int32_t xm_sigmoid(int32_t x_q16);   /*                              -> Q30 */
int32_t xm_silu(int32_t x_q16);      /*                              -> Q16 */
int32_t xm_softplus(int32_t v_q16);  /* torch threshold 20           -> Q24 */
int32_t xm_racc_ref(int32_t acc, int32_t sh);     /* exported xm_racc_value, for host tools */

/* One channel of the selective scan (what mb.scan does):
 * h[n] = exp(delta*A[n])*h[n] + (delta*u)*B[n];  returns y = C.h + D*u (Q16). */
int32_t xm_scan_channel(int32_t *h, const int32_t *A, int32_t Dk,
                        const int32_t *B, const int32_t *C, int32_t delta_q24, int32_t u_q16);

#ifdef __cplusplus
}
#endif
#endif
