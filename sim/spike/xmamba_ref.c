/* Xmamba reference semantics. Each function mirrors model/mamba_fixed.py;
 * see xmamba_ref.h and spec/xmamba-isa.md. */
#include "xmamba_ref.h"
#include "xmamba_luts.h"

#define QX  16
#define QDT 24
#define QH  20
#define QDA 30

/* ---------------------------------------------------------------- helpers */

static inline int64_t pwl(const int32_t *t, int64_t idx, int64_t frac, int fb)
{
    int64_t a = t[idx], b = t[idx + 1];
    return a + xm_rshr((b - a) * frac, fb);
}

/* ---------------------------------------------------- nonlinear functions */

int32_t xm_exp(int32_t x_q24)
{
    int64_t x = x_q24 > 0 ? 0 : x_q24;
    int64_t t = xm_rshr(x * XM_LOG2E_Q30, 30);          /* Q24, <= 0 */
    int64_t n = t >> 24;                                /* floor */
    int64_t f = t - (int64_t)((uint64_t)n << 24);       /* Q24 in [0, 1) */
    int64_t v = pwl(xm_exp2_q29, f >> XM_EXP_FB, f & ((1 << XM_EXP_FB) - 1), XM_EXP_FB);
    int64_t sh = -(n + 1);
    if (sh >= 62) return 0;
    return (int32_t)xm_rshr(v, (int)sh);
}

int32_t xm_sigmoid(int32_t x_q16)
{
    int64_t x = x_q16, a = x < 0 ? -x : x, s;
    if (a < ((int64_t)XM_SIG_RANGE << QX))
        s = pwl(xm_sig_q30, a >> XM_SIG_FB, a & ((1 << XM_SIG_FB) - 1), XM_SIG_FB);
    else
        s = (int64_t)1 << 30;
    return (int32_t)(x < 0 ? ((int64_t)1 << 30) - s : s);
}

int32_t xm_silu(int32_t x_q16)
{
    return (int32_t)xm_rshr((int64_t)x_q16 * xm_sigmoid(x_q16), 30);
}

int32_t xm_softplus(int32_t v_q16)
{
    int64_t v = v_q16, a = v < 0 ? -v : v, r;
    int64_t ea = -(int64_t)((uint64_t)a << (QDT - QX));
    int32_t e = xm_exp(ea < INT32_MIN ? INT32_MIN : (int32_t)ea);    /* Q30, exp(-|v|) */
    if (a < ((int64_t)XM_SP_RANGE << QX))
        r = pwl(xm_spr_q30, a >> XM_SP_FB, a & ((1 << XM_SP_FB) - 1), XM_SP_FB);
    else
        r = xm_spr_q30[XM_SP_SEGS];
    int64_t sp_neg = xm_rshr((int64_t)e * r, 60 - QDT);
    int64_t vq = (int64_t)((uint64_t)v << (QDT - QX));
    int64_t out = v <= 0 ? sp_neg : vq + sp_neg;
    if (v > ((int64_t)20 << QX)) out = vq;
    return out > INT32_MAX ? INT32_MAX : (int32_t)out;
}

/* Out-of-line copy of the inline helper, so host tools (ctypes) can call it. */
int32_t xm_racc_ref(int32_t acc, int32_t sh) { return xm_racc_value(acc, sh); }

int32_t xm_scan_channel(int32_t *h, const int32_t *A, int32_t Dk,
                        const int32_t *B, const int32_t *C, int32_t delta, int32_t u)
{
    int32_t du = xm_wrap32(xm_rshr((int64_t)delta * u, QX));       /* Q24 */
    int64_t acc = 0;
    for (int n = 0; n < XM_N; n++) {
        int64_t a = xm_rshr((int64_t)delta * A[n], QX);             /* Q24, <= 0 */
        if (a < INT32_MIN) a = INT32_MIN;
        int64_t dA = xm_exp((int32_t)a);                            /* Q30 */
        int64_t dBu = xm_rshr((int64_t)du * B[n], QDT + QX - QH);   /* Q20 */
        int32_t hn = xm_wrap32(xm_rshr(dA * h[n], QDA) + dBu);
        h[n] = hn;
        acc += (int64_t)hn * C[n];
    }
    return xm_wrap32(xm_rshr(acc, QH) + xm_rshr((int64_t)Dk * u, QX));
}

/* ------------------------------------------------------------ the unit */

void xm_reset(xm_unit_t *u)
{
    unsigned char *p = (unsigned char *)u;
    for (unsigned long i = 0; i < sizeof *u; i++) p[i] = 0;
}

int xm_exec(xm_unit_t *u, unsigned f3, unsigned f7, uint32_t rs1, uint32_t rs2, uint32_t *rd)
{
    *rd = 0;
    switch (f3) {
    case XM_F3_MAC:
        if (f7 == XM_F7_MAC4) {
            int32_t s = 0;
            for (int l = 0; l < 4; l++)
                s += (int32_t)(int8_t)(rs1 >> (8 * l)) * (int32_t)(int8_t)(rs2 >> (8 * l));
            u->acc = (int32_t)((uint32_t)u->acc + (uint32_t)s);          /* wraps */
            return 0;
        }
        if (f7 == XM_F7_RACC) {
            int32_t sh = (int32_t)(rs1 << 26) >> 26;                    /* signed rs1[5:0] */
            *rd = (uint32_t)xm_racc_value(u->acc, sh);
            u->acc = 0;
            return 0;
        }
        return -1;

    case XM_F3_NL:
        if (f7 == XM_F7_EXP)      { *rd = (uint32_t)xm_exp((int32_t)rs1);      return 0; }
        if (f7 == XM_F7_SILU)     { *rd = (uint32_t)xm_silu((int32_t)rs1);     return 0; }
        if (f7 == XM_F7_SOFTPLUS) { *rd = (uint32_t)xm_softplus((int32_t)rs1); return 0; }
        return -1;

    case XM_F3_SCAN:
        switch (f7) {
        case XM_F7_RST:
            for (unsigned i = 0; i < XM_MAX_CH * XM_N; i++) u->h[i] = 0;
            u->chan = 0;
            return 0;
        case XM_F7_LAYER:
            if (rs1 >= XM_MAX_CH) return -1;
            u->base = rs1; u->chan = 0;
            return 0;
        case XM_F7_LDA: {
            uint32_t idx = rs1 & ~XM_LDA_D;
            if (rs1 & XM_LDA_D) { if (idx >= XM_MAX_CH) return -1; u->D[idx] = (int32_t)rs2; }
            else                { if (idx >= XM_MAX_CH * XM_N) return -1; u->A[idx] = (int32_t)rs2; }
            return 0;
        }
        case XM_F7_LDBC: {
            uint32_t n = rs1 & (XM_N - 1);
            if (rs1 & XM_LDBC_C) u->C[n] = (int32_t)rs2; else u->B[n] = (int32_t)rs2;
            return 0;
        }
        case XM_F7_SCAN: {
            uint32_t ch = u->base + u->chan;
            if (ch >= XM_MAX_CH) return -1;
            *rd = (uint32_t)xm_scan_channel(&u->h[ch * XM_N], &u->A[ch * XM_N], u->D[ch],
                                            u->B, u->C, (int32_t)rs1, (int32_t)rs2);
            u->chan++;
            return 0;
        }
        case XM_F7_RDH:
            if (rs1 >= XM_MAX_CH * XM_N) return -1;
            *rd = (uint32_t)u->h[rs1];
            return 0;
        }
        return -1;
    }
    return -1;
}
