/*
 * mb.mac4 / mb.racc tests for the RISCVMamba CFU (runs on the simulated or real SoC).
 * Based on the CFU Playground template menu.
 *
 *   mb.mac4  = cfu_op0(0, rs1, rs2)   funct3 = 0, funct7 = 0
 *   mb.racc  = cfu_op0(1, shift, 0)   funct3 = 0, funct7 = 1
 */

#include "proj_menu.h"

#include <stdint.h>
#include <stdio.h>

#include "cfu.h"
#include "menu.h"
#include "xmamba_token.h"

namespace {

void do_hello_world(void) { puts("Hello, World!!!\n"); }

// ---- thin wrappers (funct7 must be a compile-time constant) ----
volatile uint32_t g_sink;  // keeps the CFU call from being optimised away

inline void mb_mac4(uint32_t rs1, uint32_t rs2) { g_sink = cfu_op0(0, rs1, rs2); }
inline int32_t mb_racc(uint32_t shift) { return (int32_t)cfu_op0(1, shift, 0); }

// ---- software reference (same rules as mbmac/ref.py) ----
uint32_t ref_acc = 0;  // two's complement, wraps

inline int8_t lane(uint32_t w, int i) { return (int8_t)((w >> (8 * i)) & 0xff); }

void ref_mac4(uint32_t a, uint32_t b) {
  int32_t sum = 0;
  for (int i = 0; i < 4; i++) sum += (int32_t)lane(a, i) * (int32_t)lane(b, i);
  ref_acc += (uint32_t)sum;
}

int32_t ref_racc(uint32_t rs1) {
  uint32_t shift = rs1 & 31;
  int64_t acc = (int32_t)ref_acc;
  int64_t half = shift ? ((int64_t)1 << (shift - 1)) : 0;
  int64_t v = (acc + half) >> shift;  // arithmetic shift = floor
  if (v > 127) v = 127;
  if (v < -128) v = -128;
  ref_acc = 0;
  return (int32_t)v;
}

uint32_t rng_state = 2463534242u;
uint32_t rng() {  // xorshift32
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 17;
  rng_state ^= rng_state << 5;
  return rng_state;
}

int g_fails = 0;

void check(const char* name, int32_t got, int32_t want) {
  if (got == want) {
    printf("  PASS %-34s = %ld\n", name, (long)got);
  } else {
    printf("  FAIL %-34s got %ld expected %ld\n", name, (long)got, (long)want);
    g_fails++;
  }
}

// Explicit examples (values worked out by hand).
void do_mb_directed(void) {
  puts("\nmb.mac4 / mb.racc directed tests\n");
  g_fails = 0;
  mb_racc(0);  // clear any old accumulator

  mb_mac4(0x01010101, 0x01010101);                       // 1*1 four times
  check("mac4 ones, racc shift 0", mb_racc(0), 4);

  mb_mac4(0x04FD02FF, 0xF807FA05);                       // (-1,2,-3,4) . (5,-6,7,-8), lanes 0..3
  check("mac4 mixed signs = -70", mb_racc(0), -70);

  mb_mac4(0x00000001, 0x00000001);                       // acc = 1
  check("racc 1 >> 1 rounds half up", mb_racc(1), 1);   // 0.5 -> 1
  mb_mac4(0x000000FF, 0x00000001);                       // acc = -1
  check("racc -1 >> 1 rounds half up", mb_racc(1), 0);  // -0.5 -> 0
  mb_mac4(0x00000003, 0x00000001);                       // acc = 3
  check("racc 3 >> 1 = 2", mb_racc(1), 2);
  mb_mac4(0x000000FD, 0x00000001);                       // acc = -3
  check("racc -3 >> 1 = -1", mb_racc(1), -1);

  mb_mac4(0x7F7F7F7F, 0x7F7F7F7F);                       // 4*127*127 = 64516
  check("saturate high", mb_racc(0), 127);
  mb_mac4(0x80808080, 0x7F7F7F7F);                       // 4*(-128)*127 = -65024
  check("saturate low", mb_racc(0), -128);
  mb_mac4(0x7F7F7F7F, 0x7F7F7F7F);
  check("64516 >> 9 = 126", mb_racc(9), 126);            // 126.01 -> 126

  mb_mac4(0x00000005, 0x00000001);                       // acc = 5
  check("shift uses only low 5 bits", mb_racc(0xABCDE000u), 5);

  mb_mac4(0x00000007, 0x00000001);
  mb_racc(0);                                            // racc clears the accumulator
  check("racc cleared the accumulator", mb_racc(0), 0);

  printf("\nDirected: %s\n", g_fails ? "FAILED" : "ALL PASSED");
}

// Random sequences checked against the software reference.
void do_mb_random(void) {
  puts("\nmb.mac4 / mb.racc random test (2000 sequences)\n");
  int fails = 0, checked = 0;
  mb_racc(0);
  ref_acc = 0;
  for (int s = 0; s < 2000; s++) {
    int n = rng() % 9;
    for (int k = 0; k < n; k++) {
      uint32_t a = rng(), b = rng();
      mb_mac4(a, b);
      ref_mac4(a, b);
    }
    uint32_t shift = (rng() % 10 < 7) ? (rng() % 21) : (rng() % 32);
    uint32_t rs1 = ((rng() & 1) ? (rng() << 5) : 0) | shift;
    int32_t got = mb_racc(rs1);
    int32_t want = ref_racc(rs1);
    checked++;
    if (got != want) {
      if (fails < 10)
        printf("  FAIL seq %d: shift %lu got %ld expected %ld\n", s,
               (unsigned long)shift, (long)got, (long)want);
      fails++;
    }
  }
  printf("Random: %d/%d correct -> %s\n", checked - fails, checked,
         fails ? "FAILED" : "ALL PASSED");
}

// Back-to-back mac4 -> racc with no software between them.
void do_mb_hazard(void) {
  puts("\nmb.mac4 immediately followed by mb.racc\n");
  int fails = 0;
  for (int i = 0; i < 500; i++) {
    uint32_t a = rng(), b = rng();
    mb_racc(0);
    ref_acc = 0;
    mb_mac4(a, b);
    int32_t got = mb_racc(0);  // issued straight after the mac4
    ref_mac4(a, b);
    if (got != ref_racc(0)) fails++;
  }
  printf("Back-to-back: %d/500 wrong -> %s\n", fails, fails ? "FAILED" : "ALL PASSED");
}

struct Menu MENU = {
    "Project Menu",
    "project",
    {
        MENU_ITEM('h', "say Hello", do_hello_world),
        MENU_ITEM('d', "mb.mac4/racc directed tests", do_mb_directed),
        MENU_ITEM('r', "mb.mac4/racc random vs software", do_mb_random),
        MENU_ITEM('b', "mb.mac4 then mb.racc back-to-back", do_mb_hazard),
        MENU_ITEM('t', "Xmamba full token: software vs CFU", do_xmamba_token),
        MENU_ITEM('s', "Xmamba ten tokens and sequence reset", do_xmamba_sequence),
        MENU_END,
    },
};

};  // anonymous namespace

extern "C" void do_proj_menu() { menu_run(&MENU); }
