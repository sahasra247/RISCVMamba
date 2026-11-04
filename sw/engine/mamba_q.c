/* Fixed-point Mamba engine.
 *
 * One source, four builds (the ablation table in Milestones.md):
 *   (none)                         RV32IM software baseline
 *   -DXM_MAC                       + MAC unit       (mb.mac4, mb.racc)
 *   -DXM_MAC -DXM_NL               + nonlinear unit (mb.exp, mb.silu, mb.softplus)
 *   -DXM_MAC -DXM_NL -DXM_SCAN     full Xmamba      (+ mb.rst/layer/lda/ldbc/scan)
 * Every build is bit-exact with model/mamba_fixed.py. The arithmetic comes from
 * sim/spike/xmamba_ref.c, the same code Spike executes for the instructions.
 * Formats: x/u/z/B/C/y Q16, delta Q24, exp(delta*A) Q30, h Q20; linear layers
 * int8 x int8 with per-row (weights) and per-vector (inputs) power-of-two scales. */
#include <stdint.h>
#include <stddef.h>
#include "mamba_q.h"
#include "bm.h"
/* The reference arithmetic is compiled into this file so the compiler can
 * inline it: the software baseline must be as fast as plain C allows. */
#include "xmamba_ref.c"
#if defined(XM_MAC) || defined(XM_NL) || defined(XM_SCAN)
#include "xmamba.h"
#endif

#define D   MB_D_MODEL
#define ED  MB_D_INNER
#define N   MB_D_STATE
#define R   MB_DT_RANK
#define K   MB_D_CONV
#define V   MB_VOCAB
#define NL  MB_N_LAYER
#define PAD4(n) (((n) + 3) & ~3)
#define QX  16

#ifdef XM_NL
#define SILU(x)     mb_silu(x)
#define SOFTPLUS(x) mb_softplus(x)
#define EXP(x)      mb_exp(x)
#else
#define SILU(x)     xm_silu(x)
#define SOFTPLUS(x) xm_softplus(x)
#define EXP(x)      xm_exp(x)
#endif

#ifdef XM_SCAN
_Static_assert(N == XM_N, "mb.scan has a fixed state size of 16");
_Static_assert(NL * ED <= XM_MAX_CH, "model has more channels than the scan unit holds");
#endif

typedef struct {
    const int8_t *q;     /* [rows][PAD4(cols)], 4-byte aligned */
    const int8_t *sw;    /* [rows] per-row shift */
} qlin_t;

typedef struct {
    const int32_t *norm, *conv_w, *conv_b, *dt_b, *A, *Dk;
    qlin_t in_proj, x_proj, dt_proj, out_proj;
} qlayer_t;

static struct {
    qlin_t embed, head;
    qlayer_t layer[NL];
    const int32_t *norm_f;
    uint64_t eps_q32;
} W;

static int32_t conv_st[NL][ED][K - 1];   /* Q16, oldest first */
#ifdef XM_SCAN
static int32_t hbuf[ED * N];             /* debug copy read back with mb.rdh */
#else
static int32_t h[NL][ED][N];             /* Q20 */
#endif

static int32_t x[D], xn[D], xz[2 * ED], u[ED], xdbl[R + 2 * N], dtv[ED], delta[ED], y[ED], o[D];
static int8_t  act[PAD4(ED > D ? ED : D)] __attribute__((aligned(4)));

/* ---------------------------------------------------------------- profile */

const char *const mbq_prof_name[MBQ_P_COUNT] = {
    "embed", "rmsnorm", "in_proj", "conv1d+silu", "x_proj", "dt_proj",
    "softplus", "scan", "gate silu(z)", "out_proj", "residual", "final norm+head" };
uint64_t mbq_prof[MBQ_P_COUNT];

#define PROF(id, stmt) do { uint64_t _i0 = bm_instret(); stmt; mbq_prof[id] += bm_instret() - _i0; } while (0)

static inline int bitlen32(uint32_t v) { return v ? 32 - __builtin_clz(v) : 0; }
static inline int bitlen64(uint64_t v) { return v ? 64 - __builtin_clzll(v) : 0; }

static uint64_t isqrt64(uint64_t n)          /* floor(sqrt(n)), like math.isqrt */
{
    uint64_t r = 0, bit = (uint64_t)1 << 62;
    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) { n -= r + bit; r = (r >> 1) + bit; }
        else r >>= 1;
        bit >>= 2;
    }
    return r;
}

/* ---------------------------------------------------------------- kernels */

/* Q16 vector -> int8 in act[] (zero-padded to 4); returns the shift s. */
static int quant_act(const int32_t *v, int n)
{
    uint32_t m = 0;
    for (int i = 0; i < n; i++) {
        uint32_t a = v[i] < 0 ? -(uint32_t)v[i] : (uint32_t)v[i];
        if (a > m) m = a;
    }
    int s = bitlen32(m) - 7;
    if (xm_rshr(m, s) > 127) s++;
    for (int i = 0; i < n; i++) {
        int64_t q = xm_rshr(v[i], s);
        act[i] = (int8_t)(q > 127 ? 127 : q < -127 ? -127 : q);
    }
    for (int i = n; i < PAD4(n); i++) act[i] = 0;
    return s;
}

/* out = Wq . act, rescaled to Q16 with saturation (mb.mac4 / mb.racc). */
__attribute__((noinline))
static void matvec_q(int32_t *out, const qlin_t *L, const int32_t *v, int rows, int cols)
{
    int s = quant_act(v, cols);
    int cp = PAD4(cols);
#ifdef XM_MAC
    const uint32_t *a = (const uint32_t *)act;
    for (int r = 0; r < rows; r++) {
        const uint32_t *w = (const uint32_t *)(L->q + (size_t)r * cp);
        for (int c = 0; c < cp / 4; c++) mb_mac4(w[c], a[c]);
        out[r] = mb_racc(L->sw[r] - s);
    }
#else
    for (int r = 0; r < rows; r++) {
        const int8_t *w = L->q + (size_t)r * cp;
        int32_t acc = 0;
        for (int c = 0; c < cp; c += 4)
            acc += w[c] * act[c] + w[c + 1] * act[c + 1] + w[c + 2] * act[c + 2] + w[c + 3] * act[c + 3];
        out[r] = xm_racc_value(acc, L->sw[r] - s);
    }
#endif
}

__attribute__((noinline))
static void rmsnorm_q(int32_t *out, const int32_t *in, const int32_t *w, int n)
{
    uint64_t ss = 0;
    for (int i = 0; i < n; i++) ss += (uint64_t)((int64_t)in[i] * in[i]);
    uint64_t msq = ss / (uint64_t)n + W.eps_q32;
    int k = (62 - bitlen64(msq)) / 2;
    if (k > 14) k = 14;
    uint64_t r = isqrt64(msq << (2 * k));
    int64_t inv = (int64_t)(((uint64_t)1 << (48 + k)) / r);
    for (int i = 0; i < n; i++) {
        int64_t xw = xm_rshr((int64_t)in[i] * w[i], QX);
        out[i] = xm_wrap32(xm_rshr(xw * inv, 32));
    }
}

__attribute__((noinline))
static void conv1d_silu_q(int32_t *out, const int32_t *in, int32_t st[][K - 1],
                          const int32_t *w, const int32_t *b)
{
    for (int i = 0; i < ED; i++) {
        const int32_t *wi = w + i * K;
        int64_t acc = 0;
        for (int k = 0; k < K - 1; k++) acc += (int64_t)st[i][k] * wi[k];
        acc += (int64_t)in[i] * wi[K - 1];
        for (int k = 0; k < K - 2; k++) st[i][k] = st[i][k + 1];
        st[i][K - 2] = in[i];
        out[i] = SILU(xm_wrap32(xm_rshr(acc, QX) + b[i]));
    }
}

__attribute__((noinline))
static void softplus_vec(int32_t *out, const int32_t *in)
{
    for (int i = 0; i < ED; i++) out[i] = SOFTPLUS(in[i]);
}

#ifdef XM_SCAN
/* The unit holds h, A and D; per token it needs B and C, then one mb.scan per channel. */
__attribute__((noinline))
static void scan_q(int l, int32_t *yo, const int32_t *dl, const int32_t *uu,
                   const int32_t *B, const int32_t *C)
{
    mb_layer((uint32_t)(l * ED));
    for (int n = 0; n < N; n++) {
        mb_ldbc((uint32_t)n, B[n]);
        mb_ldbc((uint32_t)n | MB_LDBC_C, C[n]);
    }
    for (int i = 0; i < ED; i++) yo[i] = mb_scan(dl[i], uu[i]);
}
#else
/* Software scan; same arithmetic as xm_scan_channel(), with EXP() so the
 * MAC+NL build can use mb.exp. */
__attribute__((noinline))
static void scan_q(int l, int32_t *yo, const int32_t *dl, const int32_t *uu,
                   const int32_t *B, const int32_t *C)
{
    const int32_t *A = W.layer[l].A, *Dk = W.layer[l].Dk;
    for (int i = 0; i < ED; i++) {
        int32_t *hs = h[l][i];
        int32_t du = xm_wrap32(xm_rshr((int64_t)dl[i] * uu[i], QX));
        int64_t acc = 0;
        for (int n = 0; n < N; n++) {
            int64_t a = xm_rshr((int64_t)dl[i] * A[i * N + n], QX);
            if (a < INT32_MIN) a = INT32_MIN;
            int64_t dA = EXP((int32_t)a);
            int64_t dBu = xm_rshr((int64_t)du * B[n], 20);
            int32_t hn = xm_wrap32(xm_rshr(dA * hs[n], 30) + dBu);
            hs[n] = hn;
            acc += (int64_t)hn * C[n];
        }
        yo[i] = xm_wrap32(xm_rshr(acc, 20) + xm_rshr((int64_t)Dk[i] * uu[i], QX));
    }
}
#endif

__attribute__((noinline))
static void gate_q(int32_t *yo, const int32_t *z)
{
    for (int i = 0; i < ED; i++) yo[i] = xm_wrap32(xm_rshr((int64_t)yo[i] * SILU(z[i]), QX));
}

/* ------------------------------------------------------------------- init */

static const uint8_t *take_qlin(qlin_t *L, const uint8_t *p, int rows, int cols)
{
    L->q = (const int8_t *)p;  p += (size_t)rows * PAD4(cols);
    L->sw = (const int8_t *)p; p += PAD4(rows);
    return p;
}

static const uint8_t *take_i32(const int32_t **dst, const uint8_t *p, int n)
{
    *dst = (const int32_t *)p;
    return p + 4 * (size_t)n;
}

int mbq_init(const void *blob)
{
    const uint32_t *hdr = blob;
    const uint32_t want[] = { 0x3851424Du, 1, D, NL, V, N, MB_EXPAND, K, R, MB_TIED };
    for (unsigned i = 0; i < sizeof want / sizeof want[0]; i++)
        if (hdr[i] != want[i]) return -(int)(i + 1);
    W.eps_q32 = hdr[10];

    const uint8_t *p = (const uint8_t *)(hdr + 16);
    p = take_qlin(&W.embed, p, V, D);
    for (int l = 0; l < NL; l++) {
        qlayer_t *L = &W.layer[l];
        p = take_i32(&L->norm, p, D);
        p = take_qlin(&L->in_proj, p, 2 * ED, D);
        p = take_i32(&L->conv_w, p, ED * K);
        p = take_i32(&L->conv_b, p, ED);
        p = take_qlin(&L->x_proj, p, R + 2 * N, ED);
        p = take_qlin(&L->dt_proj, p, ED, R);
        p = take_i32(&L->dt_b, p, ED);
        p = take_i32(&L->A, p, ED * N);
        p = take_i32(&L->Dk, p, ED);
        p = take_qlin(&L->out_proj, p, D, ED);
    }
    p = take_i32(&W.norm_f, p, D);
    if (MB_TIED) W.head = W.embed;
    else         take_qlin(&W.head, p, V, D);

#ifdef XM_SCAN
    /* A and D live in the unit; load them once. */
    for (int l = 0; l < NL; l++)
        for (int i = 0; i < ED; i++) {
            uint32_t ch = (uint32_t)(l * ED + i);
            for (int n = 0; n < N; n++) mb_lda(ch * N + (uint32_t)n, W.layer[l].A[i * N + n]);
            mb_lda(MB_LDA_D | ch, W.layer[l].Dk[i]);
        }
#endif
    mbq_reset();
    return 0;
}

void mbq_reset(void)
{
    int32_t *c = &conv_st[0][0][0];
    for (unsigned i = 0; i < sizeof conv_st / sizeof(int32_t); i++) c[i] = 0;
#ifdef XM_SCAN
    mb_rst();
#else
    int32_t *s = &h[0][0][0];
    for (unsigned i = 0; i < sizeof h / sizeof(int32_t); i++) s[i] = 0;
#endif
}

const int32_t *mbq_debug_h(int layer)
{
#ifdef XM_SCAN
    for (int i = 0; i < ED * N; i++) hbuf[i] = mb_rdh((uint32_t)(layer * ED * N + i));
    return hbuf;
#else
    return &h[layer][0][0];
#endif
}

/* ------------------------------------------------------------------- step */

void mbq_step(int token, int32_t *logits, int32_t *layer_out)
{
    PROF(MBQ_P_EMBED, {
        const int8_t *e = W.embed.q + (size_t)token * PAD4(D);
        int s = W.embed.sw[token] - QX;
        for (int i = 0; i < D; i++) x[i] = (int32_t)xm_rshr(e[i], s);
    });

    for (int l = 0; l < NL; l++) {
        const qlayer_t *L = &W.layer[l];
        PROF(MBQ_P_NORM,     rmsnorm_q(xn, x, L->norm, D));
        PROF(MBQ_P_IN_PROJ,  matvec_q(xz, &L->in_proj, xn, 2 * ED, D));
        PROF(MBQ_P_CONV,     conv1d_silu_q(u, xz, conv_st[l], L->conv_w, L->conv_b));
        PROF(MBQ_P_X_PROJ,   matvec_q(xdbl, &L->x_proj, u, R + 2 * N, ED));
        PROF(MBQ_P_DT_PROJ, {
            matvec_q(dtv, &L->dt_proj, xdbl, ED, R);
            for (int i = 0; i < ED; i++) dtv[i] = xm_wrap32((int64_t)dtv[i] + L->dt_b[i]);
        });
        PROF(MBQ_P_SOFTPLUS, softplus_vec(delta, dtv));
        PROF(MBQ_P_SCAN,     scan_q(l, y, delta, u, xdbl + R, xdbl + R + N));
        PROF(MBQ_P_GATE,     gate_q(y, xz + ED));
        PROF(MBQ_P_OUT_PROJ, matvec_q(o, &L->out_proj, y, D, ED));
        PROF(MBQ_P_RESIDUAL, {
            for (int i = 0; i < D; i++) x[i] = xm_wrap32((int64_t)x[i] + o[i]);
            if (layer_out)
                for (int i = 0; i < D; i++) layer_out[l * D + i] = x[i];
        });
    }
    PROF(MBQ_P_HEAD, {
        rmsnorm_q(xn, x, W.norm_f, D);
        matvec_q(logits, &W.head, xn, V, D);
    });
}
