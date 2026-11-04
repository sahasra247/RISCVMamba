/* Single-precision math for the float engine (no libm on bare metal).
 * Accuracy is close to 1 ulp over the ranges the model uses; these are the
 * reference the fixed-point lookup tables are later measured against. */
#ifndef FMATH_H
#define FMATH_H

float mb_expf(float x);
float mb_logf(float x);          /* x > 0 */
float mb_log1pf(float x);        /* x > -1 */
float mb_rsqrtf(float x);        /* x > 0 */
float mb_siluf(float x);
float mb_softplusf(float x);     /* same threshold (20) as torch.nn.functional.softplus */

#endif
