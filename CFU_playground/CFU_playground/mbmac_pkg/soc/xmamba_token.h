#ifndef XMAMBA_TOKEN_H
#define XMAMBA_TOKEN_H

#include "xmamba_ref.h"

namespace xmamba {

enum Stage { kNorm, kInProj, kConvStage, kXProj, kDelta, kScan, kGate,
             kOutProj, kResidual, kFinalNorm, kHead, kStageCount };
static const char* const stage_names[kStageCount] = {
    "rmsnorm", "in_proj", "conv+silu", "x_proj", "dt+softplus", "scan",
    "gate", "out_proj", "residual", "final_norm", "lm_head"};

struct Counters { uint64_t cycles, instructions; };

inline Counters counters() {
  Counters result = {0, 0};
#ifdef __riscv
  uint32_t cyc = 0, inst = 0;
  asm volatile("csrr %0, mcycle" : "=r"(cyc) :: "memory");
  asm volatile("csrr %0, minstret" : "=r"(inst) :: "memory");
  result.cycles = cyc;
  result.instructions = inst;
#endif
  return result;
}

struct Trace {
  int32_t values[kLayers][kStageCount][2 * kChannels];
  unsigned lengths[kLayers][kStageCount];
  Counters profile[kStageCount], total;

  void capture(unsigned layer, Stage stage, const int32_t* data, unsigned count, Counters start) {
    const Counters end = counters();
    profile[stage].cycles += end.cycles - start.cycles;
    profile[stage].instructions += end.instructions - start.instructions;
    lengths[layer][stage] = count;
    for (unsigned index = 0; index < count; ++index) values[layer][stage][index] = data[index];
  }
};

struct Fixture {
  int8_t embedding[kVocab][kWidth];
  int8_t in_proj[kLayers][2 * kChannels][kWidth];
  int8_t conv[kLayers][kChannels][kConv];
  int8_t conv_bias[kLayers][kChannels];
  int8_t x_proj[kLayers][kRank + 2 * kStates][kChannels];
  int8_t dt_proj[kLayers][kChannels][kRank];
  int16_t dt_bias[kLayers][kChannels];
  int8_t out_proj[kLayers][kWidth][kChannels];
  int8_t head[kVocab][kWidth];
  int16_t norm[kLayers][kWidth], final_norm[kWidth];
  int16_t constants[kLayers][kConstants];

  static int8_t next(uint32_t& seed, unsigned range) {
    seed = seed * 1664525u + 1013904223u;
    return int8_t(int32_t((seed >> 16) % (2 * range + 1)) - int32_t(range));
  }

  template <unsigned Rows, unsigned Cols>
  static void fill(int8_t (&matrix)[Rows][Cols], uint32_t& seed, unsigned range) {
    for (unsigned row = 0; row < Rows; ++row)
      for (unsigned col = 0; col < Cols; ++col) matrix[row][col] = next(seed, range);
  }

  void init() {
    uint32_t seed = 0x584d414d;
    fill(embedding, seed, 24);
    for (unsigned layer = 0; layer < kLayers; ++layer) {
      fill(in_proj[layer], seed, 64);
      fill(x_proj[layer], seed, 64);
      fill(dt_proj[layer], seed, 32);
      fill(out_proj[layer], seed, 48);
      for (unsigned channel = 0; channel < kChannels; ++channel) {
        for (unsigned tap = 0; tap < kConv; ++tap) conv[layer][channel][tap] = int8_t(8 * (tap + 1));
        conv_bias[layer][channel] = next(seed, 2);
        dt_bias[layer][channel] = int16_t(64 + 8 * channel);
        for (unsigned state = 0; state < kStates; ++state)
          constants[layer][channel * kStates + state] = -int16_t(128 * (state + 1) + 8 * channel);
        constants[layer][kChannels * kStates + channel] = 256;
      }
      for (unsigned channel = 0; channel < kWidth; ++channel) norm[layer][channel] = int16_t(240 + 8 * channel);
    }
    fill(head, seed, 64);
    for (unsigned channel = 0; channel < kWidth; ++channel) final_norm[channel] = 256;
  }
};

inline uint32_t isqrt(uint32_t value) {
  uint32_t result = 0, bit = uint32_t(1) << 30;
  while (bit > value) bit >>= 2;
  while (bit) {
    if (value >= result + bit) { value -= result + bit; result = (result >> 1) + bit; }
    else result >>= 1;
    bit >>= 2;
  }
  return result;
}

inline void rmsnorm(const int32_t* input, const int16_t* gamma, int32_t* output) {
  uint32_t squares = 0;
  for (unsigned channel = 0; channel < kWidth; ++channel) squares += uint32_t(input[channel] * input[channel]);
  const int32_t denominator = int32_t(isqrt(squares / kWidth + 1)) * 16;
  for (unsigned channel = 0; channel < kWidth; ++channel)
    output[channel] = sat8(input[channel] * gamma[channel] / denominator);
}

inline int32_t to_activation(int32_t value) { return sat8(floor_shift(int64_t(value) + 8, 4)); }

struct Pipeline {
  Backend backend;
  const Fixture* weights;
  int32_t history[kLayers][kChannels][kConv];

  void init(Backend selected, const Fixture& fixture) {
    backend = selected;
    weights = &fixture;
    backend.op(8);
    for (unsigned layer = 0; layer < kLayers; ++layer) {
      backend.op(10, layer);
      for (unsigned address = 0; address < kConstants; ++address)
        backend.op(2, address, uint32_t(fixture.constants[layer][address]));
    }
    reset_sequence();
  }

  void reset_sequence() {
    memset(history, 0, sizeof(history));
    backend.op(34);
  }

  int32_t dot(const int8_t* weight, const int32_t* input, unsigned length) const {
    for (unsigned offset = 0; offset < length; offset += 4) {
      uint32_t packed_weight = 0, packed_input = 0;
      for (unsigned lane = 0; lane < 4 && offset + lane < length; ++lane) {
        packed_weight |= uint32_t(uint8_t(weight[offset + lane])) << (8 * lane);
        packed_input |= uint32_t(uint8_t(input[offset + lane])) << (8 * lane);
      }
      backend.op(0, packed_weight, packed_input);
    }
    return backend.op(8, 6);
  }

  template <unsigned Rows, unsigned Cols>
  void matvec(const int8_t (&matrix)[Rows][Cols], const int32_t* input, int32_t* output) const {
    for (unsigned row = 0; row < Rows; ++row) output[row] = dot(matrix[row], input, Cols);
  }

  int step(unsigned token, Trace& trace) {
    if (token >= kVocab) return -1;
    memset(&trace, 0, sizeof(trace));
    const Counters start = counters();
    int32_t residual[kWidth], normalized[kWidth], projected[2 * kChannels];
    int32_t activation[kChannels], selective[kRank + 2 * kStates], delta[kChannels];
    int32_t scanned[kChannels], gated[kChannels], output[kWidth], logits[kVocab];
    for (unsigned channel = 0; channel < kWidth; ++channel) residual[channel] = weights->embedding[token][channel];
    for (unsigned layer = 0; layer < kLayers; ++layer) {
      backend.op(10, layer);
      Counters mark = counters();
      rmsnorm(residual, weights->norm[layer], normalized);
      trace.capture(layer, kNorm, normalized, kWidth, mark);
      mark = counters();
      matvec(weights->in_proj[layer], normalized, projected);
      trace.capture(layer, kInProj, projected, 2 * kChannels, mark);
      mark = counters();
      for (unsigned channel = 0; channel < kChannels; ++channel) {
        for (unsigned tap = 0; tap + 1 < kConv; ++tap) history[layer][channel][tap] = history[layer][channel][tap + 1];
        history[layer][channel][kConv - 1] = projected[channel];
        const int32_t convolved = sat8(dot(weights->conv[layer][channel], history[layer][channel], kConv) + weights->conv_bias[layer][channel]);
        activation[channel] = to_activation(backend.op(9, uint32_t(convolved * 16)));
      }
      trace.capture(layer, kConvStage, activation, kChannels, mark);
      mark = counters();
      matvec(weights->x_proj[layer], activation, selective);
      trace.capture(layer, kXProj, selective, kRank + 2 * kStates, mark);
      mark = counters();
      matvec(weights->dt_proj[layer], selective, delta);
      for (unsigned channel = 0; channel < kChannels; ++channel)
        delta[channel] = backend.op(17, uint32_t(delta[channel] * 16 + weights->dt_bias[layer][channel]));
      trace.capture(layer, kDelta, delta, kChannels, mark);
      mark = counters();
      for (unsigned state = 0; state < kStates; ++state)
        backend.op(18, state, pack16(selective[kRank + state] * 16, selective[kRank + kStates + state] * 16));
      for (unsigned channel = 0; channel < kChannels; ++channel)
        scanned[channel] = backend.op(26, pack16(delta[channel], activation[channel] * 16), channel);
      trace.capture(layer, kScan, scanned, kChannels, mark);
      mark = counters();
      for (unsigned channel = 0; channel < kChannels; ++channel)
        gated[channel] = to_activation(qmul(scanned[channel], backend.op(9, uint32_t(projected[kChannels + channel] * 16))));
      trace.capture(layer, kGate, gated, kChannels, mark);
      mark = counters();
      matvec(weights->out_proj[layer], gated, output);
      trace.capture(layer, kOutProj, output, kWidth, mark);
      mark = counters();
      for (unsigned channel = 0; channel < kWidth; ++channel) residual[channel] = sat8(residual[channel] + output[channel]);
      trace.capture(layer, kResidual, residual, kWidth, mark);
    }
    Counters mark = counters();
    rmsnorm(residual, weights->final_norm, normalized);
    trace.capture(0, kFinalNorm, normalized, kWidth, mark);
    mark = counters();
    matvec(weights->head, normalized, logits);
    trace.capture(0, kHead, logits, kVocab, mark);
    unsigned next_token = 0;
    for (unsigned token_id = 1; token_id < kVocab; ++token_id)
      if (logits[token_id] > logits[next_token]) next_token = token_id;
    const Counters end = counters();
    trace.total = {end.cycles - start.cycles, end.instructions - start.instructions};
    return int(next_token);
  }
};

inline bool same_trace(const Trace& left, const Trace& right) {
  return memcmp(left.lengths, right.lengths, sizeof(left.lengths)) == 0 &&
         memcmp(left.values, right.values, sizeof(left.values)) == 0;
}

}

void do_xmamba_token();
void do_xmamba_sequence();

#endif