/* Run the fixed-point engine on the reference tokens and require every
 * integer to equal model/mamba_fixed.py exactly: each layer's output, each
 * layer's scan state h, and the logits. Then print the per-kernel profile
 * (instructions per token), which is the input to the ISA v0 draft. */
#include <stdint.h>
#include "bm.h"
#include "mamba_q.h"

#define HSZ (MB_D_INNER * MB_D_STATE)

extern const uint8_t mamba_weights[], mamba_ref[];

static int32_t logits[MB_VOCAB], layer_out[MB_N_LAYER * MB_D_MODEL];
static long mismatches;

static void check(const char *what, int t, int l, const int32_t *got, const int32_t *ref, int n)
{
    int bad = 0, first = -1;
    for (int i = 0; i < n; i++)
        if (got[i] != ref[i]) { if (first < 0) first = i; bad++; }
    if (bad) {
        if (mismatches < 20)
            bm_printf("  MISMATCH token %d layer %d %s: %d of %d differ; first [%d] got %d want %d\n",
                      t, l, what, bad, n, first, got[first], ref[first]);
        mismatches += bad;
    }
}

int main(void)
{
    int rc = mbq_init(mamba_weights);
    if (rc) { bm_printf("FAIL: weight blob does not match model_cfg.h (field %d)\n", -rc); return 1; }
    const uint32_t *rh = (const uint32_t *)mamba_ref;
    if (rh[0] != 0x5251424Du) { bm_printf("FAIL: bad ref blob (not a fixed-point reference)\n"); return 1; }
    int ntok = (int)rh[2];
    const int32_t *tokens = (const int32_t *)(rh + 4);
    const int32_t *ref = tokens + ntok;

    bm_printf("mamba_fixed: d=%d layers=%d N=%d E=%d R=%d V=%d tokens=%d\n",
              MB_D_MODEL, MB_N_LAYER, MB_D_STATE, MB_EXPAND, MB_DT_RANK, MB_VOCAB, ntok);

    uint64_t total = 0;
    for (int t = 0; t < ntok; t++) {
        bm_perf_t p;
        BM_PERF_BEGIN(p);
        mbq_step(tokens[t], logits, layer_out);
        BM_PERF_END(p);
        total += p.instret;

        for (int l = 0; l < MB_N_LAYER; l++)
            check("x", t, l, layer_out + l * MB_D_MODEL, ref + l * MB_D_MODEL, MB_D_MODEL);
        ref += MB_N_LAYER * MB_D_MODEL;
        for (int l = 0; l < MB_N_LAYER; l++)
            check("h", t, l, mbq_debug_h(l), ref + l * HSZ, HSZ);
        ref += MB_N_LAYER * HSZ;
        check("logits", t, -1, logits, ref, MB_VOCAB);
        ref += MB_VOCAB;
        bm_printf("token %d (in %3d): instret %llu  %s\n", t, (int)tokens[t],
                  (unsigned long long)p.instret, mismatches ? "MISMATCH" : "bit-exact");
    }

    /* Profile: instructions per token per kernel (includes counter overhead). */
    uint64_t sum = 0;
    for (int k = 0; k < MBQ_P_COUNT; k++) sum += mbq_prof[k];
    bm_printf("\nprofile (instructions per token, average over %d tokens)\n", ntok);
    bm_printf("  %-16s %12s %7s\n", "kernel", "instret", "share");
    for (int k = 0; k < MBQ_P_COUNT; k++) {
        uint64_t per = mbq_prof[k] / (uint64_t)ntok;
        uint64_t pm = mbq_prof[k] * 1000 / total;               /* per mille */
        bm_printf("  %-16s %12llu %4llu.%llu%%\n", mbq_prof_name[k],
                  (unsigned long long)per, (unsigned long long)(pm / 10), (unsigned long long)(pm % 10));
    }
    uint64_t other = total > sum ? total - sum : 0;
    bm_printf("  %-16s %12llu\n  %-16s %12llu\n", "(loop/other)", (unsigned long long)(other / ntok),
              "total", (unsigned long long)(total / ntok));

    bm_printf("mamba_fixed: %s (%ld mismatching values)\n", mismatches ? "FAIL" : "PASS", mismatches);
    return mismatches != 0;
}
