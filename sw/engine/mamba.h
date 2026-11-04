/* Float Mamba inference engine, one token at a time.
 * Sizes are compile-time constants from the generated model_cfg.h; all state
 * lives in static arrays (no malloc). Weights are read in place from the blob. */
#ifndef MAMBA_H
#define MAMBA_H

#include <stdint.h>
#include "model_cfg.h"

/* Parse and check the weight blob against model_cfg.h; returns 0 on success. */
int  mamba_init(const void *blob);

/* Clear conv and scan state (start of a sequence). */
void mamba_reset(void);

/* Run one token; writes logits[MB_VOCAB]. If layer_out is not NULL, the
 * residual stream after each layer is copied to layer_out[MB_N_LAYER][MB_D_MODEL]. */
void mamba_step(int token, float *logits, float *layer_out);

/* Read-only view of a layer's scan state h[MB_D_INNER][MB_D_STATE] (for tests). */
const float *mamba_debug_h(int layer);

#endif
