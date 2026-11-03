#ifndef XMAMBA_H
#define XMAMBA_H

#include "cfu.h"
#include "xmamba_ref.h"

#ifdef CFU_SOFTWARE_DEFINED
#error "Xmamba hardware demo requires real custom instructions; remove CFU_SOFTWARE_DEFINED"
#endif

namespace xmamba {

inline int32_t hardware_instruction(void*, unsigned function, uint32_t operand0, uint32_t operand1) {
  switch (function) {
    case 0: return signed_word(cfu_op0_hw(0, operand0, operand1));
    case 8: return signed_word(cfu_op0_hw(1, operand0, operand1));
    case 1: return signed_word(cfu_op1_hw(0, operand0, operand1));
    case 9: return signed_word(cfu_op1_hw(1, operand0, operand1));
    case 17: return signed_word(cfu_op1_hw(2, operand0, operand1));
    case 2: return signed_word(cfu_op2_hw(0, operand0, operand1));
    case 10: return signed_word(cfu_op2_hw(1, operand0, operand1));
    case 18: return signed_word(cfu_op2_hw(2, operand0, operand1));
    case 26: return signed_word(cfu_op2_hw(3, operand0, operand1));
    case 34: return signed_word(cfu_op2_hw(4, operand0, operand1));
    default: return 0;
  }
}

}
#endif