#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "xmamba_token.h"

static void dump_trace_json(const char* filepath, const std::map<std::string, std::vector<int32_t>>& data) {
  FILE* fp = std::fopen(filepath, "w");
  if (!fp) return;
  std::fprintf(fp, "{\n");
  bool first_key = true;
  for (const auto& kv : data) {
    if (!first_key) std::fprintf(fp, ",\n");
    first_key = false;
    std::fprintf(fp, "  \"%s\": [", kv.first.c_str());
    for (size_t i = 0; i < kv.second.size(); ++i) {
      std::fprintf(fp, "%s%d", (i > 0 ? ", " : ""), kv.second[i]);
    }
    std::fprintf(fp, "]");
  }
  std::fprintf(fp, "\n}\n");
  std::fclose(fp);
}

#ifdef XMAMBA_VERILATOR
#include "VCfu.h"
#include "verilated.h"

struct RtlBackend {
  VCfu dut;
  uint64_t ticks = 0;
  unsigned commands = 0;

  void tick() {
    dut.clk = 0;
    dut.eval();
    dut.clk = 1;
    dut.eval();
    ++ticks;
  }

  RtlBackend() {
    dut.reset = 1;
    dut.cmd_valid = 0;
    dut.rsp_ready = 0;
    tick();
    tick();
    dut.reset = 0;
    tick();
  }

  static int32_t call(void* context, unsigned function, uint32_t operand0, uint32_t operand1) {
    RtlBackend& rtl = *static_cast<RtlBackend*>(context);
    VCfu& dut = rtl.dut;
    dut.cmd_payload_function_id = function;
    dut.cmd_payload_inputs_0 = operand0;
    dut.cmd_payload_inputs_1 = operand1;
    dut.cmd_valid = 1;
    dut.rsp_ready = 0;
    dut.eval();
    unsigned watchdog = 0;
    while (!dut.cmd_ready) {
      if (++watchdog > 1000) std::abort();
      rtl.tick();
    }
    rtl.tick();
    dut.cmd_valid = 0;
    dut.cmd_payload_function_id = 1023;
    dut.cmd_payload_inputs_0 = 0xdeadbeef;
    dut.cmd_payload_inputs_1 = 0xfedcba98;
    dut.eval();
    while (!dut.rsp_valid) {
      if (++watchdog > 1000) std::abort();
      rtl.tick();
    }
    const uint32_t result = dut.rsp_payload_outputs_0;
    for (unsigned stall = 0; stall < rtl.commands % 4; ++stall) {
      rtl.tick();
      if (!dut.rsp_valid || dut.rsp_payload_outputs_0 != result) std::abort();
    }
    dut.rsp_ready = 1;
    rtl.tick();
    dut.rsp_ready = 0;
    ++rtl.commands;
    return xmamba::signed_word(result);
  }
};
#endif

struct RecordingBackend {
  xmamba::Reference reference;
  FILE* stream;

  static int32_t call(void* context, unsigned function, uint32_t operand0, uint32_t operand1) {
    RecordingBackend& recorder = *static_cast<RecordingBackend*>(context);
    const int32_t result = recorder.reference.execute(function, operand0, operand1);
    std::fprintf(recorder.stream, "%04x%08x%08x%08x00000000\n", function, operand0, operand1, uint32_t(result));
    return result;
  }
};

int main(int argc, char** argv) {
  using namespace xmamba;
  if (argc != 2) return 2;
  FILE* stream = std::fopen(argv[1], "w");
  if (!stream) return 2;
  const char* use_trained_env = std::getenv("USE_TRAINED_MODEL");
  bool use_trained = (use_trained_env && (std::strcmp(use_trained_env, "1") == 0 || std::strcmp(use_trained_env, "true") == 0));
  Fixture fixture;
  fixture.init(use_trained);
  Reference baseline;
  RecordingBackend recorder;
  recorder.stream = stream;
#ifdef XMAMBA_VERILATOR
  Verilated::commandArgs(argc, argv);
  RtlBackend rtl;
  Backend actual = {&rtl, RtlBackend::call};
#else
  Backend actual = {&recorder, RecordingBackend::call};
#endif
  Pipeline software, accelerated;
  software.init({&baseline, Reference::call}, fixture);
  accelerated.init(actual, fixture);
  Trace expected, observed, first;
  int next = 1;
  bool state_changed = false;
  std::map<std::string, std::vector<int32_t>> ref_dump;
  std::map<std::string, std::vector<int32_t>> dut_dump;
  for (unsigned sequence = 0; sequence < 2; ++sequence) {
    software.reset_sequence();
    accelerated.reset_sequence();
    for (unsigned position = 0; position < 10; ++position) {
      const unsigned token = position < 2 ? 1 : unsigned(next);
      next = software.step(token, expected);
      const int got = accelerated.step(token, observed);
      if (got != next || !same_trace(expected, observed)) {
        std::fprintf(stderr, "FAIL sequence=%u position=%u next=%d expected=%d\n", sequence, position, got, next);
        for (unsigned layer = 0; layer < kLayers; ++layer)
          for (unsigned stage = 0; stage < kStageCount; ++stage)
            for (unsigned index = 0; index < expected.lengths[layer][stage]; ++index)
              if (observed.values[layer][stage][index] != expected.values[layer][stage][index])
                std::fprintf(stderr, "layer=%u stage=%s index=%u got=%d want=%d\n", layer, stage_names[stage], index,
                             observed.values[layer][stage][index], expected.values[layer][stage][index]);
        return 1;
      }
      if (sequence == 0) {
        for (unsigned layer = 0; layer < kLayers; ++layer) {
          for (unsigned stage = 0; stage < kStageCount; ++stage) {
            std::string key = "token_" + std::to_string(position) + "_L" + std::to_string(layer) + "_" + stage_names[stage];
            std::vector<int32_t> r_v, d_v;
            for (unsigned idx = 0; idx < expected.lengths[layer][stage]; ++idx) {
              r_v.push_back(expected.values[layer][stage][idx]);
              d_v.push_back(observed.values[layer][stage][idx]);
            }
            ref_dump[key] = r_v;
            dut_dump[key] = d_v;
          }
        }
      }
      if (position == 0 && sequence == 0) first = observed;
      if (position == 1 && sequence == 0) state_changed = !same_trace(first, observed);
      if (position == 0 && sequence == 1 && !same_trace(first, observed)) return 1;
      std::printf("PASS sequence=%u position=%u token=%u next=%d logits=", sequence, position, token, got);
      for (unsigned token_id = 0; token_id < kVocab; ++token_id) std::printf(" %d", observed.values[0][kHead][token_id]);
      std::puts("");
    }
  }
  std::fclose(stream);
  const char* ref_path = std::getenv("XMAMBA_REF_DUMP");
  const char* dut_path = std::getenv("XMAMBA_DUT_DUMP");
  if (ref_path) dump_trace_json(ref_path, ref_dump);
  if (dut_path) dump_trace_json(dut_path, dut_dump);
  if (!state_changed || accelerated.step(kVocab, observed) != -1) return 1;
#ifdef XMAMBA_VERILATOR
  std::printf("PASS live RTL token pipeline: %u commands, %llu CFU clock ticks (not CPU cycles)\n",
              rtl.commands, static_cast<unsigned long long>(rtl.ticks));
#else
  std::puts("PASS software token pipeline and instruction recording");
#endif
  return 0;
}