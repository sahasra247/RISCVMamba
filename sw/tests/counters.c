/* Stage 0 check: read mcycle/minstret around code regions and confirm the
 * numbers make sense. The int8 dot product is a first look at the kernel
 * mb.mac4 will replace (four int8 lanes per 32-bit word, lane 0 = bits 7:0). */
#include <stdint.h>
#include "bm.h"

#define N_WORDS 256                        /* 1024 int8 elements */

static uint32_t wa[N_WORDS], wb[N_WORDS];

__attribute__((noinline))
static int32_t dot_int8_packed(const uint32_t *a, const uint32_t *b, int n)
{
    int32_t acc = 0;
    for (int i = 0; i < n; i++) {
        uint32_t x = a[i], y = b[i];
        for (int l = 0; l < 4; l++)
            acc += (int32_t)(int8_t)(x >> (8 * l)) * (int32_t)(int8_t)(y >> (8 * l));
    }
    return acc;
}

__attribute__((noinline))
static void spin(int n)
{
    for (int i = 0; i < n; i++) __asm__ volatile("");
}

static int fail;

static void report(const char *name, bm_perf_t p)
{
    bm_printf("%s cycles=%llu instret=%llu\n", name,
              (unsigned long long)p.cycles, (unsigned long long)p.instret);
}

int main(void)
{
    bm_perf_t p;

    /* 1. Measurement overhead: an empty region. */
    BM_PERF_BEGIN(p);
    BM_PERF_END(p);
    report("empty      ", p);
    uint64_t overhead = p.instret;

    /* 2. Counters scale with work: 10x the loop trips => ~10x the instructions. */
    bm_perf_t p1, p10;
    BM_PERF_BEGIN(p1);  spin(1000);  BM_PERF_END(p1);
    BM_PERF_BEGIN(p10); spin(10000); BM_PERF_END(p10);
    report("spin 1000  ", p1);
    report("spin 10000 ", p10);
    uint64_t r = (p10.instret - overhead) * 100 / (p1.instret - overhead);
    if (r < 980 || r > 1020) { bm_printf("FAIL scaling ratio x100 = %llu\n", (unsigned long long)r); fail = 1; }

    /* 3. int8 dot product: deterministic data with a closed-form check. */
    int32_t expect = 0;
    for (int i = 0; i < N_WORDS; i++) {
        uint32_t x = 0, y = 0;
        for (int l = 0; l < 4; l++) {
            int k = 4 * i + l;
            int8_t va = (int8_t)((k * 37 + 11) & 0xff);
            int8_t vb = (int8_t)((k * 91 + 5) & 0xff);
            x |= (uint32_t)(uint8_t)va << (8 * l);
            y |= (uint32_t)(uint8_t)vb << (8 * l);
            expect += va * vb;
        }
        wa[i] = x; wb[i] = y;
    }
    BM_PERF_BEGIN(p);
    int32_t got = dot_int8_packed(wa, wb, N_WORDS);
    BM_PERF_END(p);
    report("dot int8x1024", p);
    bm_printf("  result=%d expected=%d  instret/MAC x100=%llu\n", got, expect,
              (unsigned long long)(p.instret * 100 / (4 * N_WORDS)));
    if (got != expect) { bm_printf("FAIL dot product\n"); fail = 1; }

    /* 4. Counters are 64-bit and monotonic. */
    uint64_t c0 = bm_cycles(), c1 = bm_cycles();
    if (c1 <= c0) { bm_printf("FAIL mcycle not increasing\n"); fail = 1; }

    bm_printf(fail ? "counters: FAIL\n" : "counters: PASS\n");
    return fail;
}
