/* Run the float engine on the reference tokens and compare every layer's
 * output and the logits with model/mamba_step.py, on the target itself.
 * Prints per-token relative error and instructions; exits 0 on PASS. */
#include <stdint.h>
#include "bm.h"
#include "mamba.h"

#ifndef TOL
#define TOL 1e-5f                 /* max |c - ref| / max |ref|, per tensor */
#endif
#define HSZ (MB_D_INNER * MB_D_STATE)

extern const uint8_t mamba_weights[], mamba_ref[];

static float logits[MB_VOCAB], layer_out[MB_N_LAYER * MB_D_MODEL];

static float fabs_(float v) { return v < 0 ? -v : v; }

static float rel_err(const float *c, const float *r, int n)
{
    float e = 0.0f, m = 0.0f;
    for (int i = 0; i < n; i++) {
        float d = fabs_(c[i] - r[i]);
        if (d > e) e = d;
        if (fabs_(r[i]) > m) m = fabs_(r[i]);
    }
    return m > 0.0f ? e / m : e;
}

/* bm_printf has no %f: print v as d.ddde-XX */
static void put_sci(float v)
{
    if (v != v) { bm_puts("nan"); return; }
    if (v < 0) { bm_putc('-'); v = -v; }
    if (v == 0.0f) { bm_puts("0.000e+00"); return; }
    int e = 0;
    while (v >= 10.0f) { v /= 10.0f; e++; }
    while (v < 1.0f) { v *= 10.0f; e--; }
    int m = (int)(v * 1000.0f + 0.5f);
    if (m >= 10000) { m /= 10; e++; }
    bm_printf("%d.%03de%c%02d", m / 1000, m % 1000, e < 0 ? '-' : '+', e < 0 ? -e : e);
}

static int argmax(const float *v, int n)
{
    int b = 0;
    for (int i = 1; i < n; i++) if (v[i] > v[b]) b = i;
    return b;
}

int main(void)
{
    int rc = mamba_init(mamba_weights);
    if (rc) { bm_printf("FAIL: weight blob does not match model_cfg.h (field %d)\n", -rc); return 1; }

    const uint32_t *rh = (const uint32_t *)mamba_ref;
    if (rh[0] != 0x424D414Du) { bm_printf("FAIL: bad ref blob\n"); return 1; }
    int ntok = (int)rh[2];
    const int32_t *tokens = (const int32_t *)(rh + 4);
    const float *ref = (const float *)(tokens + ntok);

    bm_printf("mamba_float: d=%d layers=%d N=%d E=%d R=%d V=%d tokens=%d\n",
              MB_D_MODEL, MB_N_LAYER, MB_D_STATE, MB_EXPAND, MB_DT_RANK, MB_VOCAB, ntok);

    float worst = 0.0f;
    int fail = 0;
    for (int t = 0; t < ntok; t++) {
        bm_perf_t p;
        BM_PERF_BEGIN(p);
        mamba_step(tokens[t], logits, layer_out);
        BM_PERF_END(p);

        float lerr = 0.0f;
        for (int l = 0; l < MB_N_LAYER; l++) {
            float e = rel_err(layer_out + l * MB_D_MODEL, ref + l * MB_D_MODEL, MB_D_MODEL);
            if (e > lerr) lerr = e;
            if (!(e <= TOL)) { bm_printf("  token %d layer %d: rel err ", t, l); put_sci(e); bm_putc('\n'); fail = 1; }
        }
        ref += MB_N_LAYER * MB_D_MODEL;
        float herr = 0.0f;
        for (int l = 0; l < MB_N_LAYER; l++) {
            float e = rel_err(mamba_debug_h(l), ref + l * HSZ, HSZ);
            if (e > herr) herr = e;
            if (!(e <= TOL)) { bm_printf("  token %d layer %d: h rel err ", t, l); put_sci(e); bm_putc('\n'); fail = 1; }
        }
        ref += MB_N_LAYER * HSZ;
        if (herr > worst) worst = herr;
        float gerr = rel_err(logits, ref, MB_VOCAB);
        int am_c = argmax(logits, MB_VOCAB), am_r = argmax(ref, MB_VOCAB);
        ref += MB_VOCAB;
        if (!(gerr <= TOL) || am_c != am_r) fail = 1;
        if (lerr > worst) worst = lerr;
        if (gerr > worst) worst = gerr;

        bm_printf("token %d (in %3d): layers ", t, (int)tokens[t]);
        put_sci(lerr);
        bm_puts("  h ");
        put_sci(herr);
        bm_puts("  logits ");
        put_sci(gerr);
        bm_printf("  argmax %d/%d  instret %llu\n", am_c, am_r, (unsigned long long)p.instret);
    }
    bm_puts("worst rel err ");
    put_sci(worst);
    bm_puts(" (tol ");
    put_sci(TOL);
    bm_printf(")\nmamba_float: %s\n", fail ? "FAIL" : "PASS");
    return fail;
}
