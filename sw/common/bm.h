/* Minimal bare-metal runtime: console output, exit, and performance counters.
 * The same API will be backed by the UART and timers on the CFU Playground SoC,
 * so kernels and measurement code are written once. */
#ifndef BM_H
#define BM_H

#include <stdint.h>

void bm_putc(char c);
void bm_puts(const char *s);                /* no trailing newline added */
int  bm_printf(const char *fmt, ...);      /* %d %u %x %s %c, with l/ll; width, 0-pad, '-' for %s */
void bm_exit(int code) __attribute__((noreturn));

/* 64-bit counters on RV32: read hi, lo, hi again so a carry between the two
 * halves is never mixed. On Spike, mcycle advances once per instruction, so
 * it equals minstret; on the FPGA, mcycle is the real clock count. */
#define BM_READ_CSR64(lo, hi)                                              \
    ({ uint32_t _h, _l, _h2;                                               \
       do {                                                                \
           __asm__ volatile("csrr %0, " #hi : "=r"(_h));                   \
           __asm__ volatile("csrr %0, " #lo : "=r"(_l));                   \
           __asm__ volatile("csrr %0, " #hi : "=r"(_h2));                  \
       } while (_h != _h2);                                                \
       ((uint64_t)_h << 32) | _l; })

static inline uint64_t bm_cycles(void)  { return BM_READ_CSR64(mcycle, mcycleh); }
static inline uint64_t bm_instret(void) { return BM_READ_CSR64(minstret, minstreth); }

/* Measure a region: BM_PERF_BEGIN(p); ...; BM_PERF_END(p); then p.cycles / p.instret. */
typedef struct { uint64_t cycles, instret; } bm_perf_t;
#define BM_PERF_BEGIN(p) do { (p).instret = bm_instret(); (p).cycles = bm_cycles(); } while (0)
#define BM_PERF_END(p)   do { uint64_t _c = bm_cycles(), _i = bm_instret();      \
                              (p).cycles = _c - (p).cycles;                      \
                              (p).instret = _i - (p).instret; } while (0)

#endif
