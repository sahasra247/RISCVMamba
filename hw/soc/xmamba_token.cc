#include <stdio.h>
#include "xmamba.h"
#include "xmamba_token.h"

namespace {

xmamba::Fixture fixture;
xmamba::Reference reference;
xmamba::Pipeline software, hardware;
xmamba::Trace expected, observed;

void initialize() {
  puts("[1/3] Initializing fixture & software reference...");
  fixture.init();
  reference.reset();
  software.init({&reference, xmamba::Reference::call}, fixture);
  puts("[2/3] Pre-loading constants & resetting CFU state...");
  hardware.init({nullptr, xmamba::hardware_instruction}, fixture);
  puts("[3/3] Initialization complete.");
}

void print_profile() {
  puts("kernel,software_cycles,cfu_cycles,software_instret,cfu_instret");
  for (unsigned stage = 0; stage < xmamba::kStageCount; ++stage) {
    printf("%s,%llu,%llu,%llu,%llu\n", xmamba::stage_names[stage],
           static_cast<unsigned long long>(expected.profile[stage].cycles),
           static_cast<unsigned long long>(observed.profile[stage].cycles),
           static_cast<unsigned long long>(expected.profile[stage].instructions),
           static_cast<unsigned long long>(observed.profile[stage].instructions));
  }
  printf("total,%llu,%llu,%llu,%llu\n",
         static_cast<unsigned long long>(expected.total.cycles),
         static_cast<unsigned long long>(observed.total.cycles),
         static_cast<unsigned long long>(expected.total.instructions),
         static_cast<unsigned long long>(observed.total.instructions));
}

int checked_step(unsigned token, unsigned position, bool profile) {
  puts("  -> Running software reference step...");
  const int want = software.step(token, expected);
  puts("  -> Running CFU hardware accelerated step...");
  const int got = hardware.step(token, observed);
  if (got < 0 || got != want || !xmamba::same_trace(expected, observed)) {
    printf("FAIL XMAMBA token=%u position=%u next=%d expected=%d\n", token, position, got, want);
    for (unsigned layer = 0; layer < xmamba::kLayers; ++layer)
      for (unsigned stage = 0; stage < xmamba::kStageCount; ++stage)
        for (unsigned index = 0; index < expected.lengths[layer][stage]; ++index)
          if (expected.values[layer][stage][index] != observed.values[layer][stage][index]) {
            printf("first mismatch: layer=%u stage=%s index=%u got=%ld expected=%ld\n",
                   layer, xmamba::stage_names[stage], index,
                   static_cast<long>(observed.values[layer][stage][index]),
                   static_cast<long>(expected.values[layer][stage][index]));
            return -1;
          }
    return -1;
  }
  printf("PASS XMAMBA token=%u position=%u next=%d logits=", token, position, got);
  for (unsigned token_id = 0; token_id < xmamba::kVocab; ++token_id)
    printf(" %ld", static_cast<long>(observed.values[0][xmamba::kHead][token_id]));
  puts("");
  if (profile) print_profile();
  return got;
}

}

void do_xmamba_token() {
  puts("Xmamba deterministic fixture: d=4 layers=2 channels=8 states=4 vocab=8");
  initialize();
  if (checked_step(1, 0, true) >= 0) puts("PASS XMAMBA SINGLE TOKEN");
}

void do_xmamba_sequence() {
  initialize();
  int next = 1;
  int32_t first_logits[xmamba::kVocab];
  for (unsigned position = 0; position < 10; ++position) {
    next = checked_step(position < 2 ? 1 : unsigned(next), position, false);
    if (next < 0) return;
    if (position == 0) memcpy(first_logits, observed.values[0][xmamba::kHead], sizeof(first_logits));
  }
  software.reset_sequence();
  hardware.reset_sequence();
  if (checked_step(1, 0, false) < 0 ||
      memcmp(first_logits, observed.values[0][xmamba::kHead], sizeof(first_logits)) != 0) {
    puts("FAIL XMAMBA SEQUENCE RESET");
    return;
  }
  puts("PASS XMAMBA SEQUENCE AND RESET");
}