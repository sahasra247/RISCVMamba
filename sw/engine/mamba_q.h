/* Fixed-point Mamba engine: the RV32IM software baseline.
 * Bit-exact with model/mamba_fixed.py (same integers, same rounding, same
 * order). Sizes from the generated model_cfg.h; no malloc. */
#ifndef MAMBA_Q_H
#define MAMBA_Q_H

#include <stdint.h>
#include "model_cfg.h"

int  mbq_init(const void *blob);           /* 0 on success */
void mbq_reset(void);

/* One token: logits[MB_VOCAB] in Q16. layer_out[MB_N_LAYER][MB_D_MODEL] (Q16)
 * is filled if not NULL. */
void mbq_step(int token, int32_t *logits, int32_t *layer_out);

const int32_t *mbq_debug_h(int layer);     /* h[MB_D_INNER][MB_D_STATE], Q20 */

/* Per-kernel instruction counts, accumulated over all mbq_step calls. */
enum { MBQ_P_EMBED, MBQ_P_NORM, MBQ_P_IN_PROJ, MBQ_P_CONV, MBQ_P_X_PROJ, MBQ_P_DT_PROJ,
       MBQ_P_SOFTPLUS, MBQ_P_SCAN, MBQ_P_GATE, MBQ_P_OUT_PROJ, MBQ_P_RESIDUAL, MBQ_P_HEAD,
       MBQ_P_COUNT };
extern const char *const mbq_prof_name[MBQ_P_COUNT];
extern uint64_t mbq_prof[MBQ_P_COUNT];

#endif
