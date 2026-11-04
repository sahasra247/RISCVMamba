/* Float Mamba engine. Each kernel is its own noinline function so it can be
 * profiled with instret counters and later swapped for fixed-point and
 * Xmamba versions one at a time. Mirrors model/mamba_step.py:block_step. */
#include <stdint.h>
#include <stddef.h>
#include "mamba.h"
#include "fmath.h"

#define D   MB_D_MODEL
#define ED  MB_D_INNER
#define N   MB_D_STATE
#define R   MB_DT_RANK
#define K   MB_D_CONV
#define V   MB_VOCAB
#define NL  MB_N_LAYER

#define MAGIC 0x424D414Du

typedef struct {
    const float *norm;       /* [D]        */
    const float *in_proj;    /* [2ED][D]   */
    const float *conv_w;     /* [ED][K]    */
    const float *conv_b;     /* [ED]       */
    const float *x_proj;     /* [R+2N][ED] */
    const float *dt_w;       /* [ED][R]    */
    const float *dt_b;       /* [ED]       */
    const float *A_log;      /* [ED][N]    */
    const float *Dskip;      /* [ED]       */
    const float *out_proj;   /* [D][ED]    */
} layer_w_t;

static struct {
    const float *embed;      /* [V][D] */
    layer_w_t    layer[NL];
    const float *norm_f;     /* [D]    */
    const float *head;       /* [V][D] */
    float        eps;
} W;

/* Constants and state, as the Xmamba unit will hold them. */
static float A[NL][ED][N];           /* -exp(A_log), computed once */
static float conv_st[NL][ED][K - 1]; /* last K-1 conv inputs, oldest first */
static float h[NL][ED][N];           /* scan state */

/* Scratch for one token. */
static float x[D], xn[D], xz[2 * ED], u[ED], xdbl[R + 2 * N], delta[ED], y[ED], o[D];

/* ------------------------------------------------------------------ init */

int mamba_init(const void *blob)
{
    const uint32_t *hdr = blob;
    const uint32_t want[] = { MAGIC, 1, D, NL, V, N, MB_EXPAND, K, R, MB_TIED };
    for (unsigned i = 0; i < sizeof want / sizeof want[0]; i++)
        if (hdr[i] != want[i]) return -(int)(i + 1);
    union { uint32_t u; float f; } e = { .u = hdr[10] };
    W.eps = e.f;

    const float *p = (const float *)(hdr + 16);
    W.embed = p; p += V * D;
    for (int l = 0; l < NL; l++) {
        layer_w_t *L = &W.layer[l];
        L->norm = p;     p += D;
        L->in_proj = p;  p += 2 * ED * D;
        L->conv_w = p;   p += ED * K;
        L->conv_b = p;   p += ED;
        L->x_proj = p;   p += (R + 2 * N) * ED;
        L->dt_w = p;     p += ED * R;
        L->dt_b = p;     p += ED;
        L->A_log = p;    p += ED * N;
        L->Dskip = p;    p += ED;
        L->out_proj = p; p += D * ED;
        for (int i = 0; i < ED; i++)
            for (int n = 0; n < N; n++)
                A[l][i][n] = -mb_expf(L->A_log[i * N + n]);
    }
    W.norm_f = p; p += D;
    W.head = MB_TIED ? W.embed : p;
    mamba_reset();
    return 0;
}

void mamba_reset(void)
{
    float *c = &conv_st[0][0][0], *s = &h[0][0][0];
    for (unsigned i = 0; i < sizeof conv_st / sizeof(float); i++) c[i] = 0.0f;
    for (unsigned i = 0; i < sizeof h / sizeof(float); i++) s[i] = 0.0f;
}

const float *mamba_debug_h(int layer) { return &h[layer][0][0]; }

/* --------------------------------------------------------------- kernels */

__attribute__((noinline))
static void k_rmsnorm(float *out, const float *in, const float *w, int n)
{
    float ss = 0.0f;
    for (int i = 0; i < n; i++) ss += in[i] * in[i];
    float r = mb_rsqrtf(ss / (float)n + W.eps);
    for (int i = 0; i < n; i++) out[i] = in[i] * r * w[i];
}

/* out[r] = sum_c M[r][c] * v[c], M row-major [rows][cols] */
__attribute__((noinline))
static void k_matvec(float *out, const float *M, const float *v, int rows, int cols)
{
    for (int r = 0; r < rows; r++) {
        const float *m = M + (size_t)r * cols;
        float acc = 0.0f;
        for (int c = 0; c < cols; c++) acc += m[c] * v[c];
        out[r] = acc;
    }
}

/* Causal depthwise conv over the last K inputs, then SiLU. Updates the window. */
__attribute__((noinline))
static void k_conv1d_silu(float *out, const float *in, float st[][K - 1],
                          const float *w, const float *b)
{
    for (int i = 0; i < ED; i++) {
        const float *wi = w + i * K;
        float acc = b[i];
        for (int k = 0; k < K - 1; k++) acc += st[i][k] * wi[k];
        acc += in[i] * wi[K - 1];
        for (int k = 0; k < K - 2; k++) st[i][k] = st[i][k + 1];
        st[i][K - 2] = in[i];
        out[i] = mb_siluf(acc);
    }
}

/* delta = softplus(dt_w @ dt_low + dt_b) */
__attribute__((noinline))
static void k_delta(float *out, const float *dt_low, const float *w, const float *b)
{
    for (int i = 0; i < ED; i++) {
        const float *wi = w + i * R;
        float acc = b[i];
        for (int r = 0; r < R; r++) acc += wi[r] * dt_low[r];
        out[i] = mb_softplusf(acc);
    }
}

/* Selective scan, one token: h = exp(delta*A)*h + delta*B*u ; y = C.h + D*u */
__attribute__((noinline))
static void k_scan(float *yo, float hs[][N], const float As[][N], const float *dl,
                   const float *uu, const float *B, const float *C, const float *Dk)
{
    for (int i = 0; i < ED; i++) {
        float du = dl[i] * uu[i], acc = 0.0f;
        for (int n = 0; n < N; n++) {
            float hn = mb_expf(dl[i] * As[i][n]) * hs[i][n] + du * B[n];
            hs[i][n] = hn;
            acc += hn * C[n];
        }
        yo[i] = acc + Dk[i] * uu[i];
    }
}

/* y *= silu(z) */
__attribute__((noinline))
static void k_gate(float *yo, const float *z)
{
    for (int i = 0; i < ED; i++) yo[i] *= mb_siluf(z[i]);
}

/* ------------------------------------------------------------------ step */

void mamba_step(int token, float *logits, float *layer_out)
{
    for (int i = 0; i < D; i++) x[i] = W.embed[token * D + i];

    for (int l = 0; l < NL; l++) {
        const layer_w_t *L = &W.layer[l];
        k_rmsnorm(xn, x, L->norm, D);
        k_matvec(xz, L->in_proj, xn, 2 * ED, D);                 /* u_in = xz[0:ED], z = xz[ED:] */
        k_conv1d_silu(u, xz, conv_st[l], L->conv_w, L->conv_b);
        k_matvec(xdbl, L->x_proj, u, R + 2 * N, ED);             /* dt_low | B | C */
        k_delta(delta, xdbl, L->dt_w, L->dt_b);
        k_scan(y, h[l], A[l], delta, u, xdbl + R, xdbl + R + N, L->Dskip);
        k_gate(y, xz + ED);
        k_matvec(o, L->out_proj, y, D, ED);
        for (int i = 0; i < D; i++) x[i] += o[i];
        if (layer_out)
            for (int i = 0; i < D; i++) layer_out[l * D + i] = x[i];
    }
    k_rmsnorm(xn, x, W.norm_f, D);
    k_matvec(logits, W.head, xn, V, D);
}
