#ifndef XMAMBA_REF_H
#define XMAMBA_REF_H

#include <stdint.h>
#include <string.h>
#include "xmamba_tables.h"

namespace xmamba {

enum { kLayers = 2, kWidth = 4, kChannels = 8, kStates = 4,
       kRank = 2, kConv = 4, kVocab = 8, kConstants = kChannels * (kStates + 1) };

inline int64_t floor_shift(int64_t value, unsigned shift) {
  const int64_t divisor = int64_t(1) << shift;
  return value / divisor - (value < 0 && value % divisor != 0);
}

inline int32_t sat32(int64_t value) {
  return value > INT32_MAX ? INT32_MAX : value < INT32_MIN ? INT32_MIN : int32_t(value);
}

inline int32_t sat8(int64_t value) {
  return value > 127 ? 127 : value < -128 ? -128 : int32_t(value);
}

inline int32_t signed_word(uint32_t value) {
  return value <= INT32_MAX ? int32_t(value) : int32_t(int64_t(value) - (int64_t(1) << 32));
}

inline int32_t signed_half(uint32_t value) {
  value &= 65535;
  return value < 32768 ? int32_t(value) : int32_t(value) - 65536;
}

inline int32_t qmul(int32_t left, int32_t right) {
  return sat32(floor_shift(int64_t(left) * right, 8));
}

inline int32_t qadd(int32_t left, int32_t right) {
  return sat32(int64_t(left) + right);
}

inline uint32_t pack16(int32_t low, int32_t high) {
  return uint32_t(uint16_t(low)) | (uint32_t(uint16_t(high)) << 16);
}

inline int32_t nonlinear(unsigned kind, int32_t value) {
  if (kind == 0 && value >= 0) return 256;
  if (value <= -2048) return 0;
  if (kind != 0 && value >= 2048) return value;
  const int32_t position = kind == 0 ? -value : value + 2048;
  const unsigned index = unsigned(position) / 128;
  const int32_t* table = kind == 0 ? xm_exp_table : kind == 1 ? xm_silu_table : xm_softplus_table;
  return table[index] + int32_t(floor_shift(int64_t(table[index + 1] - table[index]) * (position % 128), 7));
}

struct Reference {
  uint32_t accumulator;
  unsigned layer;
  int32_t hidden[kLayers][kChannels][kStates];
  int16_t constants[kLayers][kConstants];
  int16_t coeff_b[kLayers][kStates], coeff_c[kLayers][kStates];

  Reference() { reset(); }

  void reset() {
    accumulator = 0;
    layer = 0;
    memset(hidden, 0, sizeof(hidden));
    memset(constants, 0, sizeof(constants));
    memset(coeff_b, 0, sizeof(coeff_b));
    memset(coeff_c, 0, sizeof(coeff_c));
  }

  int32_t execute(unsigned function, uint32_t operand0, uint32_t operand1) {
    const unsigned group = function & 7, operation = function >> 3;
    if (group == 0) {
      if (operation == 0) {
        int32_t sum = 0;
        for (unsigned lane = 0; lane < 4; ++lane) {
          const unsigned byte0 = (operand0 >> (8 * lane)) & 255;
          const unsigned byte1 = (operand1 >> (8 * lane)) & 255;
          sum += (byte0 < 128 ? int32_t(byte0) : int32_t(byte0) - 256) *
                 (byte1 < 128 ? int32_t(byte1) : int32_t(byte1) - 256);
        }
        accumulator += uint32_t(sum);
      } else if (operation == 1) {
        const unsigned shift = operand0 & 31;
        const int64_t rounded = int64_t(signed_word(accumulator)) + (shift ? int64_t(1) << (shift - 1) : 0);
        accumulator = 0;
        return sat8(floor_shift(rounded, shift));
      }
    } else if (group == 1 && operation < 3) {
      return nonlinear(operation, signed_word(operand0));
    } else if (group == 2) {
      if (operation == 0 && operand0 < kConstants) {
        constants[layer][operand0] = int16_t(signed_half(operand1));
      } else if (operation == 1 && operand0 < kLayers) {
        layer = operand0;
      } else if (operation == 2 && operand0 < kStates) {
        coeff_b[layer][operand0] = int16_t(signed_half(operand1));
        coeff_c[layer][operand0] = int16_t(signed_half(operand1 >> 16));
      } else if (operation == 3 && operand1 < kChannels) {
        const int32_t delta = signed_half(operand0), activation = signed_half(operand0 >> 16);
        int32_t output = 0;
        for (unsigned state = 0; state < kStates; ++state) {
          const int32_t decay = nonlinear(0, qmul(delta, constants[layer][operand1 * kStates + state]));
          int32_t& cell = hidden[layer][operand1][state];
          cell = qadd(qmul(decay, cell), qmul(qmul(delta, coeff_b[layer][state]), activation));
          output = qadd(output, qmul(coeff_c[layer][state], cell));
        }
        return qadd(output, qmul(constants[layer][kChannels * kStates + operand1], activation));
      } else if (operation == 4) {
        memset(hidden, 0, sizeof(hidden));
      }
    }
    return 0;
  }

  static int32_t call(void* context, unsigned function, uint32_t operand0, uint32_t operand1) {
    return static_cast<Reference*>(context)->execute(function, operand0, operand1);
  }
};

struct Backend {
  void* context;
  int32_t (*instruction)(void*, unsigned, uint32_t, uint32_t);
  int32_t op(unsigned function, uint32_t operand0 = 0, uint32_t operand1 = 0) const {
    return instruction(context, function, operand0, operand1);
  }
};

}
#endif