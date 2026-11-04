#include <stdint.h>
#include "fmath.h"

typedef union { float f; uint32_t u; } fbits_t;

#define LN2_HI 6.93145752e-1f    /* ln2 split so n*LN2_HI is exact */
#define LN2_LO 1.42860677e-6f
#define LOG2E  1.44269504f

float mb_expf(float x)
{
    if (x > 88.0f)  x = 88.0f;
    if (x < -87.0f) return 0.0f;
    /* x = n*ln2 + r, |r| <= ln2/2 */
    float t = x * LOG2E;
    int n = (int)(t + (t >= 0 ? 0.5f : -0.5f));
    float r = (x - (float)n * LN2_HI) - (float)n * LN2_LO;
    /* exp(r) by Taylor to r^7: error < 1e-8 on |r| <= 0.347 */
    float p = 1.0f + r * (1.0f + r * (0.5f + r * (1.0f / 6 + r * (1.0f / 24 + r * (1.0f / 120
              + r * (1.0f / 720 + r * (1.0f / 5040)))))));
    fbits_t s = { .u = (uint32_t)(n + 127) << 23 };
    return p * s.f;
}

float mb_logf(float x)
{
    fbits_t v = { .f = x };
    int e = (int)((v.u >> 23) & 0xff) - 127;
    v.u = (v.u & 0x007fffff) | 0x3f800000;           /* m in [1, 2) */
    float m = v.f;
    if (m > 1.41421356f) { m *= 0.5f; e++; }         /* m in [0.707, 1.414) */
    /* log(m) = 2 atanh(s), s = (m-1)/(m+1), |s| <= 0.172 */
    float s = (m - 1.0f) / (m + 1.0f), s2 = s * s;
    float lm = 2.0f * s * (1.0f + s2 * (1.0f / 3 + s2 * (1.0f / 5 + s2 * (1.0f / 7 + s2 * (1.0f / 9)))));
    return lm + (float)e * LN2_HI + (float)e * LN2_LO;
}

float mb_log1pf(float x)
{
    float u = 1.0f + x;
    if (u == 1.0f) return x;
    return mb_logf(u) * (x / (u - 1.0f));            /* corrects the rounding of 1 + x */
}

float mb_rsqrtf(float x)
{
    fbits_t v = { .f = x };
    v.u = 0x5f3759df - (v.u >> 1);
    float y = v.f;
    for (int i = 0; i < 3; i++)                      /* Newton: error ~1e-7 after 3 steps */
        y = y * (1.5f - 0.5f * x * y * y);
    return y;
}

float mb_siluf(float x)
{
    if (x < -80.0f) return 0.0f;
    return x / (1.0f + mb_expf(-x));
}

float mb_softplusf(float x)
{
    if (x > 20.0f) return x;
    return mb_log1pf(mb_expf(x));
}
