// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See VCfu.h for the primary calling header

#include "VCfu.h"
#include "VCfu__Syms.h"

//==========

VL_CTOR_IMP(VCfu) {
    VCfu__Syms* __restrict vlSymsp = __VlSymsp = new VCfu__Syms(this, name());
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Reset internal values
    
    // Reset structure values
    _ctor_var_reset();
}

void VCfu::__Vconfigure(VCfu__Syms* vlSymsp, bool first) {
    if (false && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
    if (false && this->__VlSymsp) {}  // Prevent unused
    Verilated::timeunit(-9);
    Verilated::timeprecision(-12);
}

VCfu::~VCfu() {
    VL_DO_CLEAR(delete __VlSymsp, __VlSymsp = NULL);
}

void VCfu::_settle__TOP__2(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_settle__TOP__2\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->Cfu__DOT__u_mac__DOT__is_racc = ((0U == 
                                              (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                                             & (1U 
                                                == 
                                                (0x7fU 
                                                 & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                    >> 3U))));
    vlTOPp->Cfu__DOT__u_mac__DOT__shifted = (0x1ffffffffULL 
                                             & VL_SHIFTRS_QQI(33,33,5, 
                                                              (0x1ffffffffULL 
                                                               & ((((QData)((IData)(
                                                                                (1U 
                                                                                & (vlTOPp->Cfu__DOT__u_mac__DOT__acc 
                                                                                >> 0x1fU)))) 
                                                                    << 0x20U) 
                                                                   | (QData)((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__acc))) 
                                                                  + 
                                                                  ((0U 
                                                                    == (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__racc_shift))
                                                                    ? 0ULL
                                                                    : 
                                                                   (1ULL 
                                                                    << 
                                                                    (0x1fU 
                                                                     & ((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__racc_shift) 
                                                                        - (IData)(1U))))))), (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__racc_shift)));
    vlTOPp->Cfu__DOT__mac_ready = (1U & ((~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r)) 
                                         & (~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__racc_busy))));
    vlTOPp->Cfu__DOT__xm_ready = (1U & (((~ (IData)(vlTOPp->Cfu__DOT__xm_valid)) 
                                         & (~ (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__scanning))) 
                                        & (~ (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__clearing))));
    vlTOPp->rsp_payload_outputs_0 = ((IData)(vlTOPp->Cfu__DOT__active_mac)
                                      ? vlTOPp->Cfu__DOT__u_mac__DOT__rsp_data_r
                                      : vlTOPp->Cfu__DOT__xm_result);
    vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid 
        = (((IData)(vlTOPp->cmd_valid) & (~ (IData)(vlTOPp->Cfu__DOT__pending))) 
           & (0U != (7U & (IData)(vlTOPp->cmd_payload_function_id))));
    vlTOPp->Cfu__DOT__u_xmamba__DOT__skip_value = (
                                                   (0x4fU 
                                                    >= 
                                                    (0x7fU 
                                                     & ((IData)(0x20U) 
                                                        + 
                                                        (VL_MULS_III(7,32,32, (IData)(0x28U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                                         + vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id))))
                                                    ? 
                                                   vlTOPp->Cfu__DOT__u_xmamba__DOT__constants
                                                   [
                                                   (0x7fU 
                                                    & ((IData)(0x20U) 
                                                       + 
                                                       (VL_MULS_III(7,32,32, (IData)(0x28U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                                        + vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id)))]
                                                    : 0U);
    vlTOPp->Cfu__DOT__u_xmamba__DOT__current_b = vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b
        [(7U & (VL_MULS_III(3,32,32, (IData)(4U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id))];
    vlTOPp->Cfu__DOT__u_xmamba__DOT__current_c = vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c
        [(7U & (VL_MULS_III(3,32,32, (IData)(4U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id))];
    vlTOPp->rsp_valid = ((IData)(vlTOPp->Cfu__DOT__pending) 
                         & ((IData)(vlTOPp->Cfu__DOT__active_mac)
                             ? (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r)
                             : (IData)(vlTOPp->Cfu__DOT__xm_valid)));
    vlTOPp->Cfu__DOT__u_xmamba__DOT__constant_value 
        = ((0x4fU >= (0x7fU & ((VL_MULS_III(7,32,32, (IData)(0x28U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                + VL_MULS_III(7,32,32, (IData)(4U), vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id)) 
                               + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id)))
            ? vlTOPp->Cfu__DOT__u_xmamba__DOT__constants
           [(0x7fU & ((VL_MULS_III(7,32,32, (IData)(0x28U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                       + VL_MULS_III(7,32,32, (IData)(4U), vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id)) 
                      + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id))]
            : 0U);
    vlTOPp->Cfu__DOT__u_xmamba__DOT__current_hidden 
        = vlTOPp->Cfu__DOT__u_xmamba__DOT__hidden[(0x3fU 
                                                   & (VL_MULS_III(6,32,32, (IData)(4U), 
                                                                  (VL_MULS_III(32,32,32, (IData)(8U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                                                   + vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id)) 
                                                      + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id))];
    vlTOPp->Cfu__DOT__u_mac__DOT__racc_done = (((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__racc_busy) 
                                                & (~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__v1))) 
                                               & (~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__v2)));
    vlTOPp->Cfu__DOT__u_mac__DOT__accept = ((((IData)(vlTOPp->cmd_valid) 
                                              & (~ (IData)(vlTOPp->Cfu__DOT__pending))) 
                                             & (0U 
                                                == 
                                                (7U 
                                                 & (IData)(vlTOPp->cmd_payload_function_id)))) 
                                            & (IData)(vlTOPp->Cfu__DOT__mac_ready));
    vlTOPp->cmd_ready = ((~ (IData)(vlTOPp->Cfu__DOT__pending)) 
                         & ((0U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))
                             ? (IData)(vlTOPp->Cfu__DOT__mac_ready)
                             : (IData)(vlTOPp->Cfu__DOT__xm_ready)));
}

void VCfu::_eval_initial(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_eval_initial\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->__Vclklast__TOP__clk = vlTOPp->clk;
}

void VCfu::final() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::final\n"); );
    // Variables
    VCfu__Syms* __restrict vlSymsp = this->__VlSymsp;
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
}

void VCfu::_eval_settle(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_eval_settle\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->_settle__TOP__2(vlSymsp);
}

void VCfu::_ctor_var_reset() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_ctor_var_reset\n"); );
    // Body
    cmd_valid = VL_RAND_RESET_I(1);
    cmd_ready = VL_RAND_RESET_I(1);
    cmd_payload_function_id = VL_RAND_RESET_I(10);
    cmd_payload_inputs_0 = VL_RAND_RESET_I(32);
    cmd_payload_inputs_1 = VL_RAND_RESET_I(32);
    rsp_valid = VL_RAND_RESET_I(1);
    rsp_ready = VL_RAND_RESET_I(1);
    rsp_payload_outputs_0 = VL_RAND_RESET_I(32);
    reset = VL_RAND_RESET_I(1);
    clk = VL_RAND_RESET_I(1);
    Cfu__DOT__mac_ready = VL_RAND_RESET_I(1);
    Cfu__DOT__xm_ready = VL_RAND_RESET_I(1);
    Cfu__DOT__xm_valid = VL_RAND_RESET_I(1);
    Cfu__DOT__xm_result = VL_RAND_RESET_I(32);
    Cfu__DOT__pending = VL_RAND_RESET_I(1);
    Cfu__DOT__active_mac = VL_RAND_RESET_I(1);
    Cfu__DOT____Vcellinp__u_xmamba__cmd_valid = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__is_racc = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__rsp_valid_r = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__rsp_data_r = VL_RAND_RESET_I(32);
    Cfu__DOT__u_mac__DOT__racc_busy = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__racc_shift = VL_RAND_RESET_I(5);
    Cfu__DOT__u_mac__DOT__accept = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__prod0 = VL_RAND_RESET_I(16);
    Cfu__DOT__u_mac__DOT__prod1 = VL_RAND_RESET_I(16);
    Cfu__DOT__u_mac__DOT__prod2 = VL_RAND_RESET_I(16);
    Cfu__DOT__u_mac__DOT__prod3 = VL_RAND_RESET_I(16);
    Cfu__DOT__u_mac__DOT__psum = VL_RAND_RESET_I(18);
    Cfu__DOT__u_mac__DOT__acc = VL_RAND_RESET_I(32);
    Cfu__DOT__u_mac__DOT__shifted = VL_RAND_RESET_Q(33);
    Cfu__DOT__u_mac__DOT__v1 = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__v2 = VL_RAND_RESET_I(1);
    Cfu__DOT__u_mac__DOT__racc_done = VL_RAND_RESET_I(1);
    { int __Vi0=0; for (; __Vi0<64; ++__Vi0) {
            Cfu__DOT__u_xmamba__DOT__hidden[__Vi0] = VL_RAND_RESET_I(32);
    }}
    { int __Vi0=0; for (; __Vi0<80; ++__Vi0) {
            Cfu__DOT__u_xmamba__DOT__constants[__Vi0] = VL_RAND_RESET_I(16);
    }}
    { int __Vi0=0; for (; __Vi0<8; ++__Vi0) {
            Cfu__DOT__u_xmamba__DOT__coeff_b[__Vi0] = VL_RAND_RESET_I(16);
    }}
    { int __Vi0=0; for (; __Vi0<8; ++__Vi0) {
            Cfu__DOT__u_xmamba__DOT__coeff_c[__Vi0] = VL_RAND_RESET_I(16);
    }}
    Cfu__DOT__u_xmamba__DOT__layer_id = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__channel_id = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__state_id = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__clear_index = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__phase = VL_RAND_RESET_I(3);
    Cfu__DOT__u_xmamba__DOT__scanning = VL_RAND_RESET_I(1);
    Cfu__DOT__u_xmamba__DOT__clearing = VL_RAND_RESET_I(1);
    Cfu__DOT__u_xmamba__DOT__delta = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__activation = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__decay = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__injection = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__output_sum = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT__constant_value = VL_RAND_RESET_I(16);
    Cfu__DOT__u_xmamba__DOT__skip_value = VL_RAND_RESET_I(16);
    Cfu__DOT__u_xmamba__DOT__current_b = VL_RAND_RESET_I(16);
    Cfu__DOT__u_xmamba__DOT__current_c = VL_RAND_RESET_I(16);
    Cfu__DOT__u_xmamba__DOT__current_hidden = VL_RAND_RESET_I(32);
    Cfu__DOT__u_xmamba__DOT____Vlvbound2 = VL_RAND_RESET_I(16);
}
