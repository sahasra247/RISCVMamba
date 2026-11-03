// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See VCfu.h for the primary calling header

#include "VCfu.h"
#include "VCfu__Syms.h"

//==========

void VCfu::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate VCfu::eval\n"); );
    VCfu__Syms* __restrict vlSymsp = this->__VlSymsp;  // Setup global symbol table
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
#ifdef VL_DEBUG
    // Debug assertions
    _eval_debug_assertions();
#endif  // VL_DEBUG
    // Initialize
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) _eval_initial_loop(vlSymsp);
    // Evaluate till stable
    int __VclockLoop = 0;
    QData __Vchange = 1;
    do {
        VL_DEBUG_IF(VL_DBG_MSGF("+ Clock loop\n"););
        _eval(vlSymsp);
        if (VL_UNLIKELY(++__VclockLoop > 100)) {
            // About to fail, so enable debug to see what's not settling.
            // Note you must run make with OPT=-DVL_DEBUG for debug prints.
            int __Vsaved_debug = Verilated::debug();
            Verilated::debug(1);
            __Vchange = _change_request(vlSymsp);
            Verilated::debug(__Vsaved_debug);
            VL_FATAL_MT("/mnt/c/Users/pulumati/Downloads/sahasra/CFU_playground/mbmac_pkg/rtl/cfu_top.v", 3, "",
                "Verilated model didn't converge\n"
                "- See DIDNOTCONVERGE in the Verilator manual");
        } else {
            __Vchange = _change_request(vlSymsp);
        }
    } while (VL_UNLIKELY(__Vchange));
}

void VCfu::_eval_initial_loop(VCfu__Syms* __restrict vlSymsp) {
    vlSymsp->__Vm_didInit = true;
    _eval_initial(vlSymsp);
    // Evaluate till stable
    int __VclockLoop = 0;
    QData __Vchange = 1;
    do {
        _eval_settle(vlSymsp);
        _eval(vlSymsp);
        if (VL_UNLIKELY(++__VclockLoop > 100)) {
            // About to fail, so enable debug to see what's not settling.
            // Note you must run make with OPT=-DVL_DEBUG for debug prints.
            int __Vsaved_debug = Verilated::debug();
            Verilated::debug(1);
            __Vchange = _change_request(vlSymsp);
            Verilated::debug(__Vsaved_debug);
            VL_FATAL_MT("/mnt/c/Users/pulumati/Downloads/sahasra/CFU_playground/mbmac_pkg/rtl/cfu_top.v", 3, "",
                "Verilated model didn't DC converge\n"
                "- See DIDNOTCONVERGE in the Verilator manual");
        } else {
            __Vchange = _change_request(vlSymsp);
        }
    } while (VL_UNLIKELY(__Vchange));
}

VL_INLINE_OPT void VCfu::_sequent__TOP__1(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_sequent__TOP__1\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Variables
    CData/*6:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index;
    CData/*6:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__kind;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index;
    CData/*5:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index;
    CData/*0:0*/ __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r;
    CData/*0:0*/ __Vdly__Cfu__DOT__xm_valid;
    CData/*0:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__scanning;
    CData/*0:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__clearing;
    CData/*2:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__phase;
    CData/*0:0*/ __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0;
    CData/*6:0*/ __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__constants__v80;
    CData/*0:0*/ __Vdlyvset__Cfu__DOT__u_xmamba__DOT__constants__v80;
    CData/*2:0*/ __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__coeff_b__v8;
    CData/*0:0*/ __Vdlyvset__Cfu__DOT__u_xmamba__DOT__coeff_b__v8;
    CData/*2:0*/ __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__coeff_c__v8;
    CData/*5:0*/ __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__hidden__v64;
    CData/*0:0*/ __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v64;
    CData/*5:0*/ __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__hidden__v65;
    CData/*0:0*/ __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v65;
    SData/*15:0*/ __Vdlyvval__Cfu__DOT__u_xmamba__DOT__constants__v80;
    SData/*15:0*/ __Vdlyvval__Cfu__DOT__u_xmamba__DOT__coeff_b__v8;
    SData/*15:0*/ __Vdlyvval__Cfu__DOT__u_xmamba__DOT__coeff_c__v8;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__position;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__fraction;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__lower_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__upper_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__position;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__fraction;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__lower_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__upper_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__left_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__right_value;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__Vfuncout;
    IData/*31:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__Vfuncout;
    IData/*31:0*/ __Vdly__Cfu__DOT__u_mac__DOT__acc;
    IData/*31:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__state_id;
    IData/*31:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__clear_index;
    IData/*31:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__decay;
    IData/*31:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__injection;
    IData/*31:0*/ __Vdly__Cfu__DOT__u_xmamba__DOT__output_sum;
    IData/*31:0*/ __Vdlyvval__Cfu__DOT__u_xmamba__DOT__hidden__v65;
    IData/*31:0*/ __Vilp;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__product;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__product;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__product;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__product;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__product;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__product;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__value;
    QData/*63:0*/ __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__value;
    // Body
    __Vdly__Cfu__DOT__u_mac__DOT__acc = vlTOPp->Cfu__DOT__u_mac__DOT__acc;
    __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r = vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r;
    __Vdly__Cfu__DOT__u_xmamba__DOT__output_sum = vlTOPp->Cfu__DOT__u_xmamba__DOT__output_sum;
    __Vdly__Cfu__DOT__u_xmamba__DOT__injection = vlTOPp->Cfu__DOT__u_xmamba__DOT__injection;
    __Vdly__Cfu__DOT__u_xmamba__DOT__decay = vlTOPp->Cfu__DOT__u_xmamba__DOT__decay;
    __Vdly__Cfu__DOT__u_xmamba__DOT__phase = vlTOPp->Cfu__DOT__u_xmamba__DOT__phase;
    __Vdly__Cfu__DOT__u_xmamba__DOT__clear_index = vlTOPp->Cfu__DOT__u_xmamba__DOT__clear_index;
    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__coeff_b__v8 = 0U;
    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__constants__v80 = 0U;
    __Vdly__Cfu__DOT__u_xmamba__DOT__clearing = vlTOPp->Cfu__DOT__u_xmamba__DOT__clearing;
    __Vdly__Cfu__DOT__u_xmamba__DOT__scanning = vlTOPp->Cfu__DOT__u_xmamba__DOT__scanning;
    __Vdly__Cfu__DOT__u_xmamba__DOT__state_id = vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id;
    __Vdly__Cfu__DOT__xm_valid = vlTOPp->Cfu__DOT__xm_valid;
    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v64 = 0U;
    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v65 = 0U;
    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0 = 0U;
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_mac__DOT__racc_shift = 0U;
    } else {
        if (vlTOPp->Cfu__DOT__u_mac__DOT__accept) {
            if (vlTOPp->Cfu__DOT__u_mac__DOT__is_racc) {
                vlTOPp->Cfu__DOT__u_mac__DOT__racc_shift 
                    = (0x1fU & vlTOPp->cmd_payload_inputs_0);
            }
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_mac__DOT__rsp_data_r = 0U;
    } else {
        if (vlTOPp->Cfu__DOT__u_mac__DOT__accept) {
            if ((1U & (~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__is_racc)))) {
                vlTOPp->Cfu__DOT__u_mac__DOT__rsp_data_r = 0U;
            }
        }
        if (vlTOPp->Cfu__DOT__u_mac__DOT__racc_done) {
            vlTOPp->Cfu__DOT__u_mac__DOT__rsp_data_r 
                = (IData)((VL_LTS_IQQ(1,33,33, 0x7fULL, vlTOPp->Cfu__DOT__u_mac__DOT__shifted)
                            ? 0x7fULL : (VL_GTS_IQQ(1,33,33, 0x1ffffff80ULL, vlTOPp->Cfu__DOT__u_mac__DOT__shifted)
                                          ? 0x1ffffff80ULL
                                          : vlTOPp->Cfu__DOT__u_mac__DOT__shifted)));
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_mac__DOT__racc_busy = 0U;
    } else {
        if (vlTOPp->Cfu__DOT__u_mac__DOT__accept) {
            if (vlTOPp->Cfu__DOT__u_mac__DOT__is_racc) {
                vlTOPp->Cfu__DOT__u_mac__DOT__racc_busy = 1U;
            }
        }
        if (vlTOPp->Cfu__DOT__u_mac__DOT__racc_done) {
            vlTOPp->Cfu__DOT__u_mac__DOT__racc_busy = 0U;
        }
    }
    if (vlTOPp->reset) {
        __Vdly__Cfu__DOT__u_mac__DOT__acc = 0U;
    } else {
        if (vlTOPp->Cfu__DOT__u_mac__DOT__v2) {
            __Vdly__Cfu__DOT__u_mac__DOT__acc = (vlTOPp->Cfu__DOT__u_mac__DOT__acc 
                                                 + 
                                                 ((0xfffc0000U 
                                                   & ((- (IData)(
                                                                 (1U 
                                                                  & (vlTOPp->Cfu__DOT__u_mac__DOT__psum 
                                                                     >> 0x11U)))) 
                                                      << 0x12U)) 
                                                  | vlTOPp->Cfu__DOT__u_mac__DOT__psum));
        }
        if (vlTOPp->Cfu__DOT__u_mac__DOT__racc_done) {
            __Vdly__Cfu__DOT__u_mac__DOT__acc = 0U;
        }
    }
    if (vlTOPp->reset) {
        __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r = 0U;
    } else {
        if (((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r) 
             & (((IData)(vlTOPp->rsp_ready) & (IData)(vlTOPp->Cfu__DOT__pending)) 
                & (IData)(vlTOPp->Cfu__DOT__active_mac)))) {
            __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r = 0U;
        }
        if (vlTOPp->Cfu__DOT__u_mac__DOT__accept) {
            if ((1U & (~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__is_racc)))) {
                __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r = 1U;
            }
        }
        if (vlTOPp->Cfu__DOT__u_mac__DOT__racc_done) {
            __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r = 1U;
        }
    }
    if (vlTOPp->reset) {
        __Vdly__Cfu__DOT__xm_valid = 0U;
        vlTOPp->Cfu__DOT__xm_result = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__scanning = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__clearing = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__state_id = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__clear_index = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__phase = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__decay = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__injection = 0U;
        __Vdly__Cfu__DOT__u_xmamba__DOT__output_sum = 0U;
        __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0 = 1U;
    } else {
        if (((IData)(vlTOPp->Cfu__DOT__xm_valid) & 
             (((IData)(vlTOPp->rsp_ready) & (IData)(vlTOPp->Cfu__DOT__pending)) 
              & (~ (IData)(vlTOPp->Cfu__DOT__active_mac))))) {
            __Vdly__Cfu__DOT__xm_valid = 0U;
        }
        if (((IData)(vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid) 
             & (IData)(vlTOPp->Cfu__DOT__xm_ready))) {
            vlTOPp->Cfu__DOT__xm_result = 0U;
            __Vdly__Cfu__DOT__xm_valid = 1U;
            if (((1U == (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                 & (3U > (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                   >> 3U))))) {
                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value 
                    = vlTOPp->cmd_payload_inputs_0;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind 
                    = (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                >> 3U));
                if (((0U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind)) 
                     & VL_LTES_III(1,32,32, 0U, __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value))) {
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__Vfuncout = 0x100U;
                } else {
                    if (VL_GTES_III(1,32,32, 0xfffff800U, __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value)) {
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__Vfuncout = 0U;
                    } else {
                        if (((0U != (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind)) 
                             & VL_LTES_III(1,32,32, 0x800U, __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value))) {
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__Vfuncout 
                                = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value;
                        } else {
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__position 
                                = ((0U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind))
                                    ? (- __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value)
                                    : ((IData)(0x800U) 
                                       + __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__value));
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index 
                                = (0x3fU & (__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__position 
                                            >> 7U));
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__fraction 
                                = (0x7fU & __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__position);
                            if ((0U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind))) {
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index 
                                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index;
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__Vfuncout 
                                    = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                        ? 0U : ((0x10U 
                                                 & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                 ? 0U
                                                 : 
                                                ((8U 
                                                  & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                  ? 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                    ? 0U
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 0U
                                                     : 1U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 1U
                                                     : 2U)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 3U
                                                     : 5U)))
                                                  : 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 8U
                                                     : 0xdU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 0x15U
                                                     : 0x23U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 0x39U
                                                     : 0x5eU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__index))
                                                     ? 0x9bU
                                                     : 0x100U))))));
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__lower_value 
                                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__1__Vfuncout;
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index 
                                    = (0x3fU & ((IData)(1U) 
                                                + (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index)));
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__Vfuncout 
                                    = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                        ? 0U : ((0x10U 
                                                 & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                 ? 0U
                                                 : 
                                                ((8U 
                                                  & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                  ? 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                    ? 0U
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 0U
                                                     : 1U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 1U
                                                     : 2U)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 3U
                                                     : 5U)))
                                                  : 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 8U
                                                     : 0xdU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 0x15U
                                                     : 0x23U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 0x39U
                                                     : 0x5eU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__index))
                                                     ? 0x9bU
                                                     : 0x100U))))));
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__upper_value 
                                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__2__Vfuncout;
                            } else {
                                if ((1U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__kind))) {
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0U
                                                       : 0x7ffU)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x77fU
                                                       : 0x6feU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x67eU
                                                       : 0x5fcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x57aU
                                                       : 0x4f7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x473U
                                                       : 0x3eeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x366U
                                                       : 0x2dcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x24fU
                                                       : 0x1c3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x13aU
                                                       : 0xbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0x50U
                                                       : 0U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0xffffffd0U
                                                       : 0xffffffbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0xffffffbaU
                                                       : 0xffffffc3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0xffffffcfU
                                                       : 0xffffffdcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0xffffffe6U
                                                       : 0xffffffeeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0xfffffff3U
                                                       : 0xfffffff7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                       ? 0xfffffffaU
                                                       : 0xfffffffcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__index))
                                                      ? 0xfffffffeU
                                                      : 0xffffffffU)))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__lower_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__3__Vfuncout;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index 
                                        = (0x3fU & 
                                           ((IData)(1U) 
                                            + (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index)));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0U
                                                       : 0x7ffU)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x77fU
                                                       : 0x6feU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x67eU
                                                       : 0x5fcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x57aU
                                                       : 0x4f7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x473U
                                                       : 0x3eeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x366U
                                                       : 0x2dcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x24fU
                                                       : 0x1c3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x13aU
                                                       : 0xbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0x50U
                                                       : 0U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0xffffffd0U
                                                       : 0xffffffbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0xffffffbaU
                                                       : 0xffffffc3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0xffffffcfU
                                                       : 0xffffffdcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0xffffffe6U
                                                       : 0xffffffeeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0xfffffff3U
                                                       : 0xfffffff7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                       ? 0xfffffffaU
                                                       : 0xfffffffcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__index))
                                                      ? 0xfffffffeU
                                                      : 0xffffffffU)))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__upper_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__4__Vfuncout;
                                } else {
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0U
                                                       : 0x800U)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x780U
                                                       : 0x700U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x680U
                                                       : 0x601U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x581U
                                                       : 0x502U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x483U
                                                       : 0x405U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x388U
                                                       : 0x30cU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x294U
                                                       : 0x220U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x1b4U
                                                       : 0x150U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0xf9U
                                                       : 0xb1U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x79U
                                                       : 0x50U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x34U
                                                       : 0x20U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 0x14U
                                                       : 0xcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 8U
                                                       : 5U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__index))
                                                       ? 3U
                                                       : 2U)
                                                      : 1U)
                                                     : 0U))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__lower_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__5__Vfuncout;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index 
                                        = (0x3fU & 
                                           ((IData)(1U) 
                                            + (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__table_index)));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0U
                                                       : 0x800U)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x780U
                                                       : 0x700U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x680U
                                                       : 0x601U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x581U
                                                       : 0x502U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x483U
                                                       : 0x405U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x388U
                                                       : 0x30cU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x294U
                                                       : 0x220U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x1b4U
                                                       : 0x150U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0xf9U
                                                       : 0xb1U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x79U
                                                       : 0x50U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x34U
                                                       : 0x20U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 0x14U
                                                       : 0xcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 8U
                                                       : 5U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__index))
                                                       ? 3U
                                                       : 2U)
                                                      : 1U)
                                                     : 0U))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__upper_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__6__Vfuncout;
                                }
                            }
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__Vfuncout 
                                = (__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__lower_value 
                                   + VL_SHIFTRS_III(32,32,32, 
                                                    VL_MULS_III(32,32,32, 
                                                                (__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__upper_value 
                                                                 - __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__lower_value), __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__fraction), 7U));
                        }
                    }
                }
                vlTOPp->Cfu__DOT__xm_result = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__0__Vfuncout;
            } else {
                if ((2U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))) {
                    if ((0U == (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                         >> 3U)))) {
                        if ((0x28U > vlTOPp->cmd_payload_inputs_0)) {
                            vlTOPp->Cfu__DOT__u_xmamba__DOT____Vlvbound2 
                                = (0xffffU & vlTOPp->cmd_payload_inputs_1);
                            if ((0x4fU >= (0x7fU & 
                                           (((IData)(0x28U) 
                                             * vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                            + vlTOPp->cmd_payload_inputs_0)))) {
                                __Vdlyvval__Cfu__DOT__u_xmamba__DOT__constants__v80 
                                    = vlTOPp->Cfu__DOT__u_xmamba__DOT____Vlvbound2;
                                __Vdlyvset__Cfu__DOT__u_xmamba__DOT__constants__v80 = 1U;
                                __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__constants__v80 
                                    = (0x7fU & (((IData)(0x28U) 
                                                 * vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                                + vlTOPp->cmd_payload_inputs_0));
                            }
                        }
                    } else {
                        if ((1U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                            if ((2U == (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                 >> 3U)))) {
                                if ((4U > vlTOPp->cmd_payload_inputs_0)) {
                                    __Vdlyvval__Cfu__DOT__u_xmamba__DOT__coeff_b__v8 
                                        = (0xffffU 
                                           & vlTOPp->cmd_payload_inputs_1);
                                    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__coeff_b__v8 = 1U;
                                    __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__coeff_b__v8 
                                        = (7U & ((vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id 
                                                  << 2U) 
                                                 + vlTOPp->cmd_payload_inputs_0));
                                    __Vdlyvval__Cfu__DOT__u_xmamba__DOT__coeff_c__v8 
                                        = (0xffffU 
                                           & (vlTOPp->cmd_payload_inputs_1 
                                              >> 0x10U));
                                    __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__coeff_c__v8 
                                        = (7U & ((vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id 
                                                  << 2U) 
                                                 + vlTOPp->cmd_payload_inputs_0));
                                }
                            } else {
                                if ((3U == (0x7fU & 
                                            ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                                    if ((8U > vlTOPp->cmd_payload_inputs_1)) {
                                        __Vdly__Cfu__DOT__u_xmamba__DOT__state_id = 0U;
                                        __Vdly__Cfu__DOT__u_xmamba__DOT__output_sum = 0U;
                                        __Vdly__Cfu__DOT__u_xmamba__DOT__phase = 0U;
                                        __Vdly__Cfu__DOT__u_xmamba__DOT__scanning = 1U;
                                        __Vdly__Cfu__DOT__xm_valid = 0U;
                                    }
                                } else {
                                    if ((4U == (0x7fU 
                                                & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                   >> 3U)))) {
                                        __Vdly__Cfu__DOT__u_xmamba__DOT__clearing = 1U;
                                        __Vdly__Cfu__DOT__u_xmamba__DOT__clear_index = 0U;
                                        __Vdly__Cfu__DOT__xm_valid = 0U;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (vlTOPp->Cfu__DOT__u_xmamba__DOT__clearing) {
            __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v64 = 1U;
            __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__hidden__v64 
                = (0x3fU & vlTOPp->Cfu__DOT__u_xmamba__DOT__clear_index);
            if ((0x3fU == vlTOPp->Cfu__DOT__u_xmamba__DOT__clear_index)) {
                __Vdly__Cfu__DOT__u_xmamba__DOT__clearing = 0U;
                __Vdly__Cfu__DOT__xm_valid = 1U;
                vlTOPp->Cfu__DOT__xm_result = 0U;
            } else {
                __Vdly__Cfu__DOT__u_xmamba__DOT__clear_index 
                    = ((IData)(1U) + vlTOPp->Cfu__DOT__u_xmamba__DOT__clear_index);
            }
        }
        if (vlTOPp->Cfu__DOT__u_xmamba__DOT__scanning) {
            if ((0U == (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__phase))) {
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__right_value 
                    = ((0xffff0000U & ((- (IData)((1U 
                                                   & ((IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__constant_value) 
                                                      >> 0xfU)))) 
                                       << 0x10U)) | (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__constant_value));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__right_value 
                    = vlTOPp->Cfu__DOT__u_xmamba__DOT__activation;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__left_value 
                    = vlTOPp->Cfu__DOT__u_xmamba__DOT__delta;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__right_value 
                    = ((0xffff0000U & ((- (IData)((1U 
                                                   & ((IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__current_b) 
                                                      >> 0xfU)))) 
                                       << 0x10U)) | (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__current_b));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__product 
                    = VL_MULS_QQQ(64,64,64, VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__left_value), 
                                  VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__right_value));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__left_value 
                    = vlTOPp->Cfu__DOT__u_xmamba__DOT__delta;
                __Vdly__Cfu__DOT__u_xmamba__DOT__phase = 1U;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__value 
                    = VL_SHIFTRS_QQI(64,64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__product, 8U);
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__product 
                    = VL_MULS_QQQ(64,64,64, VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__left_value), 
                                  VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__right_value));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__Vfuncout 
                    = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__value)
                        ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__value)
                                          ? 0x80000000U
                                          : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__value)));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__value 
                    = VL_SHIFTRS_QQI(64,64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__product, 8U);
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__Vfuncout 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__9__Vfuncout;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__Vfuncout 
                    = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__value)
                        ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__value)
                                          ? 0x80000000U
                                          : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__value)));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__8__Vfuncout;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__Vfuncout 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__18__Vfuncout;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__kind = 0U;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__left_value 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__17__Vfuncout;
                if (VL_LTES_III(1,32,32, 0U, __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value)) {
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__Vfuncout = 0x100U;
                } else {
                    if (VL_GTES_III(1,32,32, 0xfffff800U, __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value)) {
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__Vfuncout = 0U;
                    } else {
                        if (((0U != (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__kind)) 
                             & VL_LTES_III(1,32,32, 0x800U, __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value))) {
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__Vfuncout 
                                = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value;
                        } else {
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__position 
                                = ((0U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__kind))
                                    ? (- __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value)
                                    : ((IData)(0x800U) 
                                       + __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__value));
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index 
                                = (0x3fU & (__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__position 
                                            >> 7U));
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__fraction 
                                = (0x7fU & __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__position);
                            if ((0U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__kind))) {
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index 
                                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index;
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__Vfuncout 
                                    = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                        ? 0U : ((0x10U 
                                                 & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                 ? 0U
                                                 : 
                                                ((8U 
                                                  & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                  ? 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                    ? 0U
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 0U
                                                     : 1U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 1U
                                                     : 2U)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 3U
                                                     : 5U)))
                                                  : 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 8U
                                                     : 0xdU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 0x15U
                                                     : 0x23U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 0x39U
                                                     : 0x5eU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__index))
                                                     ? 0x9bU
                                                     : 0x100U))))));
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__lower_value 
                                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__10__Vfuncout;
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index 
                                    = (0x3fU & ((IData)(1U) 
                                                + (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index)));
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__Vfuncout 
                                    = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                        ? 0U : ((0x10U 
                                                 & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                 ? 0U
                                                 : 
                                                ((8U 
                                                  & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                  ? 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                    ? 0U
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 0U
                                                     : 1U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 1U
                                                     : 2U)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 3U
                                                     : 5U)))
                                                  : 
                                                 ((4U 
                                                   & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                   ? 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 8U
                                                     : 0xdU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 0x15U
                                                     : 0x23U))
                                                   : 
                                                  ((2U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                    ? 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 0x39U
                                                     : 0x5eU)
                                                    : 
                                                   ((1U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__index))
                                                     ? 0x9bU
                                                     : 0x100U))))));
                                __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__upper_value 
                                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__exp_table__11__Vfuncout;
                            } else {
                                if ((1U == (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__kind))) {
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0U
                                                       : 0x7ffU)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x77fU
                                                       : 0x6feU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x67eU
                                                       : 0x5fcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x57aU
                                                       : 0x4f7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x473U
                                                       : 0x3eeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x366U
                                                       : 0x2dcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x24fU
                                                       : 0x1c3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x13aU
                                                       : 0xbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0x50U
                                                       : 0U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0xffffffd0U
                                                       : 0xffffffbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0xffffffbaU
                                                       : 0xffffffc3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0xffffffcfU
                                                       : 0xffffffdcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0xffffffe6U
                                                       : 0xffffffeeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0xfffffff3U
                                                       : 0xfffffff7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                       ? 0xfffffffaU
                                                       : 0xfffffffcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__index))
                                                      ? 0xfffffffeU
                                                      : 0xffffffffU)))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__lower_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__12__Vfuncout;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index 
                                        = (0x3fU & 
                                           ((IData)(1U) 
                                            + (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index)));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0U
                                                       : 0x7ffU)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x77fU
                                                       : 0x6feU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x67eU
                                                       : 0x5fcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x57aU
                                                       : 0x4f7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x473U
                                                       : 0x3eeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x366U
                                                       : 0x2dcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x24fU
                                                       : 0x1c3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x13aU
                                                       : 0xbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0x50U
                                                       : 0U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0xffffffd0U
                                                       : 0xffffffbbU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0xffffffbaU
                                                       : 0xffffffc3U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0xffffffcfU
                                                       : 0xffffffdcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0xffffffe6U
                                                       : 0xffffffeeU)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0xfffffff3U
                                                       : 0xfffffff7U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                       ? 0xfffffffaU
                                                       : 0xfffffffcU))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__index))
                                                      ? 0xfffffffeU
                                                      : 0xffffffffU)))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__upper_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__silu_table__13__Vfuncout;
                                } else {
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0U
                                                       : 0x800U)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x780U
                                                       : 0x700U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x680U
                                                       : 0x601U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x581U
                                                       : 0x502U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x483U
                                                       : 0x405U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x388U
                                                       : 0x30cU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x294U
                                                       : 0x220U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x1b4U
                                                       : 0x150U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0xf9U
                                                       : 0xb1U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x79U
                                                       : 0x50U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x34U
                                                       : 0x20U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 0x14U
                                                       : 0xcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 8U
                                                       : 5U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__index))
                                                       ? 3U
                                                       : 2U)
                                                      : 1U)
                                                     : 0U))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__lower_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__14__Vfuncout;
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index 
                                        = (0x3fU & 
                                           ((IData)(1U) 
                                            + (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__table_index)));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__Vfuncout 
                                        = ((0x20U & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                            ? ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                ? 0U
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                    ? 0U
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                     ? 0U
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 0U
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0U
                                                       : 0x800U)))))
                                            : ((0x10U 
                                                & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                ? (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x780U
                                                       : 0x700U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x680U
                                                       : 0x601U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x581U
                                                       : 0x502U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x483U
                                                       : 0x405U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x388U
                                                       : 0x30cU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x294U
                                                       : 0x220U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x1b4U
                                                       : 0x150U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0xf9U
                                                       : 0xb1U))))
                                                : (
                                                   (8U 
                                                    & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                    ? 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x79U
                                                       : 0x50U)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x34U
                                                       : 0x20U))
                                                     : 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 0x14U
                                                       : 0xcU)
                                                      : 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 8U
                                                       : 5U)))
                                                    : 
                                                   ((4U 
                                                     & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                     ? 
                                                    ((2U 
                                                      & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                      ? 
                                                     ((1U 
                                                       & (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__index))
                                                       ? 3U
                                                       : 2U)
                                                      : 1U)
                                                     : 0U))));
                                    __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__upper_value 
                                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__softplus_table__15__Vfuncout;
                                }
                            }
                            __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__Vfuncout 
                                = (__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__lower_value 
                                   + VL_SHIFTRS_III(32,32,32, 
                                                    VL_MULS_III(32,32,32, 
                                                                (__Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__upper_value 
                                                                 - __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__lower_value), __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__fraction), 7U));
                        }
                    }
                }
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__product 
                    = VL_MULS_QQQ(64,64,64, VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__left_value), 
                                  VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__right_value));
                __Vdly__Cfu__DOT__u_xmamba__DOT__decay 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__nonlinear__7__Vfuncout;
                __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__value 
                    = VL_SHIFTRS_QQI(64,64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__product, 8U);
                __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__Vfuncout 
                    = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__value)
                        ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__value)
                                          ? 0x80000000U
                                          : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__value)));
                __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__Vfuncout 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__19__Vfuncout;
                __Vdly__Cfu__DOT__u_xmamba__DOT__injection 
                    = __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__16__Vfuncout;
            } else {
                if ((1U == (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__phase))) {
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__right_value 
                        = vlTOPp->Cfu__DOT__u_xmamba__DOT__injection;
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__right_value 
                        = vlTOPp->Cfu__DOT__u_xmamba__DOT__current_hidden;
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__left_value 
                        = vlTOPp->Cfu__DOT__u_xmamba__DOT__decay;
                    __Vdly__Cfu__DOT__u_xmamba__DOT__phase = 2U;
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__product 
                        = VL_MULS_QQQ(64,64,64, VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__left_value), 
                                      VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__right_value));
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__value 
                        = VL_SHIFTRS_QQI(64,64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__product, 8U);
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__Vfuncout 
                        = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__value)
                            ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__value)
                                              ? 0x80000000U
                                              : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__value)));
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__Vfuncout 
                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__22__Vfuncout;
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__left_value 
                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__21__Vfuncout;
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__value 
                        = ((((QData)((IData)((- (IData)(
                                                        (1U 
                                                         & (__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__left_value 
                                                            >> 0x1fU)))))) 
                             << 0x20U) | (QData)((IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__left_value))) 
                           + (((QData)((IData)((- (IData)(
                                                          (1U 
                                                           & (__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__right_value 
                                                              >> 0x1fU)))))) 
                               << 0x20U) | (QData)((IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__right_value))));
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__Vfuncout 
                        = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__value)
                            ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__value)
                                              ? 0x80000000U
                                              : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__value)));
                    __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__Vfuncout 
                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__23__Vfuncout;
                    __Vdlyvval__Cfu__DOT__u_xmamba__DOT__hidden__v65 
                        = __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__20__Vfuncout;
                    __Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v65 = 1U;
                    __Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__hidden__v65 
                        = (0x3fU & (VL_MULS_III(6,32,32, (IData)(4U), 
                                                (VL_MULS_III(32,32,32, (IData)(8U), vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id) 
                                                 + vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id)) 
                                    + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id));
                } else {
                    if ((2U == (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__phase))) {
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__right_value 
                            = vlTOPp->Cfu__DOT__u_xmamba__DOT__current_hidden;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__left_value 
                            = ((0xffff0000U & ((- (IData)(
                                                          (1U 
                                                           & ((IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__current_c) 
                                                              >> 0xfU)))) 
                                               << 0x10U)) 
                               | (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__current_c));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__product 
                            = VL_MULS_QQQ(64,64,64, 
                                          VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__left_value), 
                                          VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__right_value));
                        if ((3U == vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id)) {
                            __Vdly__Cfu__DOT__u_xmamba__DOT__phase = 3U;
                        } else {
                            __Vdly__Cfu__DOT__u_xmamba__DOT__state_id 
                                = ((IData)(1U) + vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id);
                            __Vdly__Cfu__DOT__u_xmamba__DOT__phase = 0U;
                        }
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__value 
                            = VL_SHIFTRS_QQI(64,64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__product, 8U);
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__Vfuncout 
                            = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__value)
                                ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__value)
                                                  ? 0x80000000U
                                                  : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__value)));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__Vfuncout 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__26__Vfuncout;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__right_value 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__25__Vfuncout;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__left_value 
                            = vlTOPp->Cfu__DOT__u_xmamba__DOT__output_sum;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__value 
                            = ((((QData)((IData)((- (IData)(
                                                            (1U 
                                                             & (__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__left_value 
                                                                >> 0x1fU)))))) 
                                 << 0x20U) | (QData)((IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__left_value))) 
                               + (((QData)((IData)(
                                                   (- (IData)(
                                                              (1U 
                                                               & (__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__right_value 
                                                                  >> 0x1fU)))))) 
                                   << 0x20U) | (QData)((IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__right_value))));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__Vfuncout 
                            = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__value)
                                ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__value)
                                                  ? 0x80000000U
                                                  : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__value)));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__Vfuncout 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__27__Vfuncout;
                        __Vdly__Cfu__DOT__u_xmamba__DOT__output_sum 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__24__Vfuncout;
                    } else {
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__right_value 
                            = vlTOPp->Cfu__DOT__u_xmamba__DOT__activation;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__left_value 
                            = ((0xffff0000U & ((- (IData)(
                                                          (1U 
                                                           & ((IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__skip_value) 
                                                              >> 0xfU)))) 
                                               << 0x10U)) 
                               | (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__skip_value));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__product 
                            = VL_MULS_QQQ(64,64,64, 
                                          VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__left_value), 
                                          VL_EXTENDS_QI(64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__right_value));
                        __Vdly__Cfu__DOT__u_xmamba__DOT__scanning = 0U;
                        __Vdly__Cfu__DOT__xm_valid = 1U;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__value 
                            = VL_SHIFTRS_QQI(64,64,32, __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__product, 8U);
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__Vfuncout 
                            = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__value)
                                ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__value)
                                                  ? 0x80000000U
                                                  : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__value)));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__Vfuncout 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__30__Vfuncout;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__right_value 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__qmul__29__Vfuncout;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__left_value 
                            = vlTOPp->Cfu__DOT__u_xmamba__DOT__output_sum;
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__value 
                            = ((((QData)((IData)((- (IData)(
                                                            (1U 
                                                             & (__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__left_value 
                                                                >> 0x1fU)))))) 
                                 << 0x20U) | (QData)((IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__left_value))) 
                               + (((QData)((IData)(
                                                   (- (IData)(
                                                              (1U 
                                                               & (__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__right_value 
                                                                  >> 0x1fU)))))) 
                                   << 0x20U) | (QData)((IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__right_value))));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__Vfuncout 
                            = (VL_LTS_IQQ(1,64,64, 0x7fffffffULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__value)
                                ? 0x7fffffffU : (VL_GTS_IQQ(1,64,64, 0xffffffff80000000ULL, __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__value)
                                                  ? 0x80000000U
                                                  : (IData)(__Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__value)));
                        __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__Vfuncout 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__saturate__31__Vfuncout;
                        vlTOPp->Cfu__DOT__xm_result 
                            = __Vfunc_Cfu__DOT__u_xmamba__DOT__qadd__28__Vfuncout;
                    }
                }
            }
        }
    }
    vlTOPp->Cfu__DOT__u_mac__DOT__acc = __Vdly__Cfu__DOT__u_mac__DOT__acc;
    vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r = __Vdly__Cfu__DOT__u_mac__DOT__rsp_valid_r;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__clear_index = __Vdly__Cfu__DOT__u_xmamba__DOT__clear_index;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__phase = __Vdly__Cfu__DOT__u_xmamba__DOT__phase;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__decay = __Vdly__Cfu__DOT__u_xmamba__DOT__decay;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__injection = __Vdly__Cfu__DOT__u_xmamba__DOT__injection;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__output_sum = __Vdly__Cfu__DOT__u_xmamba__DOT__output_sum;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__scanning = __Vdly__Cfu__DOT__u_xmamba__DOT__scanning;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__clearing = __Vdly__Cfu__DOT__u_xmamba__DOT__clearing;
    vlTOPp->Cfu__DOT__u_xmamba__DOT__state_id = __Vdly__Cfu__DOT__u_xmamba__DOT__state_id;
    vlTOPp->Cfu__DOT__xm_valid = __Vdly__Cfu__DOT__xm_valid;
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[0U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[1U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[2U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[3U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[4U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[5U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[6U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[7U] = 0U;
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__coeff_b__v8) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_c[__Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__coeff_c__v8] 
            = __Vdlyvval__Cfu__DOT__u_xmamba__DOT__coeff_c__v8;
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[0U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[1U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[2U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[3U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[4U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[5U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[6U] = 0U;
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[7U] = 0U;
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__coeff_b__v8) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__coeff_b[__Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__coeff_b__v8] 
            = __Vdlyvval__Cfu__DOT__u_xmamba__DOT__coeff_b__v8;
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0) {
        __Vilp = 0U;
        while ((__Vilp <= 0x3fU)) {
            vlTOPp->Cfu__DOT__u_xmamba__DOT__hidden[__Vilp] = 0U;
            __Vilp = ((IData)(1U) + __Vilp);
        }
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v64) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__hidden[__Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__hidden__v64] = 0U;
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v65) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__hidden[__Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__hidden__v65] 
            = __Vdlyvval__Cfu__DOT__u_xmamba__DOT__hidden__v65;
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__hidden__v0) {
        __Vilp = 0U;
        while ((__Vilp <= 0x4fU)) {
            vlTOPp->Cfu__DOT__u_xmamba__DOT__constants[__Vilp] = 0U;
            __Vilp = ((IData)(1U) + __Vilp);
        }
    }
    if (__Vdlyvset__Cfu__DOT__u_xmamba__DOT__constants__v80) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__constants[__Vdlyvdim0__Cfu__DOT__u_xmamba__DOT__constants__v80] 
            = __Vdlyvval__Cfu__DOT__u_xmamba__DOT__constants__v80;
    }
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
    vlTOPp->Cfu__DOT__u_mac__DOT__v2 = ((~ (IData)(vlTOPp->reset)) 
                                        & (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__v1));
    vlTOPp->Cfu__DOT__u_mac__DOT__psum = (0x3ffffU 
                                          & (((((0x30000U 
                                                 & ((- (IData)(
                                                               (1U 
                                                                & ((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod0) 
                                                                   >> 0xfU)))) 
                                                    << 0x10U)) 
                                                | (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod0)) 
                                               + ((0x30000U 
                                                   & ((- (IData)(
                                                                 (1U 
                                                                  & ((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod1) 
                                                                     >> 0xfU)))) 
                                                      << 0x10U)) 
                                                  | (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod1))) 
                                              + ((0x30000U 
                                                  & ((- (IData)(
                                                                (1U 
                                                                 & ((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod2) 
                                                                    >> 0xfU)))) 
                                                     << 0x10U)) 
                                                 | (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod2))) 
                                             + ((0x30000U 
                                                 & ((- (IData)(
                                                               (1U 
                                                                & ((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod3) 
                                                                   >> 0xfU)))) 
                                                    << 0x10U)) 
                                                | (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__prod3))));
    vlTOPp->Cfu__DOT__mac_ready = (1U & ((~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r)) 
                                         & (~ (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__racc_busy))));
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__active_mac = 0U;
    } else {
        if (((IData)(vlTOPp->cmd_valid) & (IData)(vlTOPp->cmd_ready))) {
            vlTOPp->Cfu__DOT__active_mac = (0U == (7U 
                                                   & (IData)(vlTOPp->cmd_payload_function_id)));
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__activation = 0U;
    } else {
        if (((IData)(vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid) 
             & (IData)(vlTOPp->Cfu__DOT__xm_ready))) {
            if ((1U & (~ ((1U == (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                          & (3U > (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                            >> 3U))))))) {
                if ((2U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))) {
                    if ((0U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                         >> 3U)))) {
                        if ((1U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                            if ((2U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                 >> 3U)))) {
                                if ((3U == (0x7fU & 
                                            ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                                    if ((8U > vlTOPp->cmd_payload_inputs_1)) {
                                        vlTOPp->Cfu__DOT__u_xmamba__DOT__activation 
                                            = ((0xffff0000U 
                                                & ((- (IData)(
                                                              (1U 
                                                               & (vlTOPp->cmd_payload_inputs_0 
                                                                  >> 0x1fU)))) 
                                                   << 0x10U)) 
                                               | (0xffffU 
                                                  & (vlTOPp->cmd_payload_inputs_0 
                                                     >> 0x10U)));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__delta = 0U;
    } else {
        if (((IData)(vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid) 
             & (IData)(vlTOPp->Cfu__DOT__xm_ready))) {
            if ((1U & (~ ((1U == (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                          & (3U > (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                            >> 3U))))))) {
                if ((2U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))) {
                    if ((0U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                         >> 3U)))) {
                        if ((1U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                            if ((2U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                 >> 3U)))) {
                                if ((3U == (0x7fU & 
                                            ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                                    if ((8U > vlTOPp->cmd_payload_inputs_1)) {
                                        vlTOPp->Cfu__DOT__u_xmamba__DOT__delta 
                                            = ((0xffff0000U 
                                                & ((- (IData)(
                                                              (1U 
                                                               & (vlTOPp->cmd_payload_inputs_0 
                                                                  >> 0xfU)))) 
                                                   << 0x10U)) 
                                               | (0xffffU 
                                                  & vlTOPp->cmd_payload_inputs_0));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__pending = 0U;
    } else {
        if (((IData)(vlTOPp->cmd_valid) & (IData)(vlTOPp->cmd_ready))) {
            vlTOPp->Cfu__DOT__pending = 1U;
        }
        if (((IData)(vlTOPp->rsp_valid) & (IData)(vlTOPp->rsp_ready))) {
            vlTOPp->Cfu__DOT__pending = 0U;
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id = 0U;
    } else {
        if (((IData)(vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid) 
             & (IData)(vlTOPp->Cfu__DOT__xm_ready))) {
            if ((1U & (~ ((1U == (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                          & (3U > (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                            >> 3U))))))) {
                if ((2U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))) {
                    if ((0U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                         >> 3U)))) {
                        if ((1U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                            if ((2U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                 >> 3U)))) {
                                if ((3U == (0x7fU & 
                                            ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                                    if ((8U > vlTOPp->cmd_payload_inputs_1)) {
                                        vlTOPp->Cfu__DOT__u_xmamba__DOT__channel_id 
                                            = vlTOPp->cmd_payload_inputs_1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (vlTOPp->reset) {
        vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id = 0U;
    } else {
        if (((IData)(vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid) 
             & (IData)(vlTOPp->Cfu__DOT__xm_ready))) {
            if ((1U & (~ ((1U == (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                          & (3U > (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                            >> 3U))))))) {
                if ((2U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))) {
                    if ((0U != (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                         >> 3U)))) {
                        if ((1U == (0x7fU & ((IData)(vlTOPp->cmd_payload_function_id) 
                                             >> 3U)))) {
                            if ((2U > vlTOPp->cmd_payload_inputs_0)) {
                                vlTOPp->Cfu__DOT__u_xmamba__DOT__layer_id 
                                    = vlTOPp->cmd_payload_inputs_0;
                            }
                        }
                    }
                }
            }
        }
    }
    vlTOPp->Cfu__DOT__xm_ready = (1U & (((~ (IData)(vlTOPp->Cfu__DOT__xm_valid)) 
                                         & (~ (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__scanning))) 
                                        & (~ (IData)(vlTOPp->Cfu__DOT__u_xmamba__DOT__clearing))));
    vlTOPp->Cfu__DOT__u_mac__DOT__v1 = ((~ (IData)(vlTOPp->reset)) 
                                        & ((IData)(vlTOPp->Cfu__DOT__u_mac__DOT__accept) 
                                           & ((0U == 
                                               (7U 
                                                & (IData)(vlTOPp->cmd_payload_function_id))) 
                                              & (0U 
                                                 == 
                                                 (0x7fU 
                                                  & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                     >> 3U))))));
    vlTOPp->Cfu__DOT__u_mac__DOT__prod0 = (0xffffU 
                                           & VL_MULS_III(16,16,16, 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & vlTOPp->cmd_payload_inputs_0))), 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & vlTOPp->cmd_payload_inputs_1)))));
    vlTOPp->Cfu__DOT__u_mac__DOT__prod1 = (0xffffU 
                                           & VL_MULS_III(16,16,16, 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & (vlTOPp->cmd_payload_inputs_0 
                                                                              >> 8U)))), 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & (vlTOPp->cmd_payload_inputs_1 
                                                                              >> 8U))))));
    vlTOPp->Cfu__DOT__u_mac__DOT__prod2 = (0xffffU 
                                           & VL_MULS_III(16,16,16, 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & (vlTOPp->cmd_payload_inputs_0 
                                                                              >> 0x10U)))), 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & (vlTOPp->cmd_payload_inputs_1 
                                                                              >> 0x10U))))));
    vlTOPp->Cfu__DOT__u_mac__DOT__prod3 = (0xffffU 
                                           & VL_MULS_III(16,16,16, 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & (vlTOPp->cmd_payload_inputs_0 
                                                                              >> 0x18U)))), 
                                                         (0xffffU 
                                                          & VL_EXTENDS_II(16,8, 
                                                                          (0xffU 
                                                                           & (vlTOPp->cmd_payload_inputs_1 
                                                                              >> 0x18U))))));
    vlTOPp->rsp_payload_outputs_0 = ((IData)(vlTOPp->Cfu__DOT__active_mac)
                                      ? vlTOPp->Cfu__DOT__u_mac__DOT__rsp_data_r
                                      : vlTOPp->Cfu__DOT__xm_result);
    vlTOPp->rsp_valid = ((IData)(vlTOPp->Cfu__DOT__pending) 
                         & ((IData)(vlTOPp->Cfu__DOT__active_mac)
                             ? (IData)(vlTOPp->Cfu__DOT__u_mac__DOT__rsp_valid_r)
                             : (IData)(vlTOPp->Cfu__DOT__xm_valid)));
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
}

VL_INLINE_OPT void VCfu::_combo__TOP__3(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_combo__TOP__3\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->Cfu__DOT__u_mac__DOT__is_racc = ((0U == 
                                              (7U & (IData)(vlTOPp->cmd_payload_function_id))) 
                                             & (1U 
                                                == 
                                                (0x7fU 
                                                 & ((IData)(vlTOPp->cmd_payload_function_id) 
                                                    >> 3U))));
    vlTOPp->Cfu__DOT____Vcellinp__u_xmamba__cmd_valid 
        = (((IData)(vlTOPp->cmd_valid) & (~ (IData)(vlTOPp->Cfu__DOT__pending))) 
           & (0U != (7U & (IData)(vlTOPp->cmd_payload_function_id))));
    vlTOPp->cmd_ready = ((~ (IData)(vlTOPp->Cfu__DOT__pending)) 
                         & ((0U == (7U & (IData)(vlTOPp->cmd_payload_function_id)))
                             ? (IData)(vlTOPp->Cfu__DOT__mac_ready)
                             : (IData)(vlTOPp->Cfu__DOT__xm_ready)));
    vlTOPp->Cfu__DOT__u_mac__DOT__accept = ((((IData)(vlTOPp->cmd_valid) 
                                              & (~ (IData)(vlTOPp->Cfu__DOT__pending))) 
                                             & (0U 
                                                == 
                                                (7U 
                                                 & (IData)(vlTOPp->cmd_payload_function_id)))) 
                                            & (IData)(vlTOPp->Cfu__DOT__mac_ready));
}

void VCfu::_eval(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_eval\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    if (((IData)(vlTOPp->clk) & (~ (IData)(vlTOPp->__Vclklast__TOP__clk)))) {
        vlTOPp->_sequent__TOP__1(vlSymsp);
    }
    vlTOPp->_combo__TOP__3(vlSymsp);
    // Final
    vlTOPp->__Vclklast__TOP__clk = vlTOPp->clk;
}

VL_INLINE_OPT QData VCfu::_change_request(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_change_request\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    return (vlTOPp->_change_request_1(vlSymsp));
}

VL_INLINE_OPT QData VCfu::_change_request_1(VCfu__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_change_request_1\n"); );
    VCfu* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // Change detection
    QData __req = false;  // Logically a bool
    return __req;
}

#ifdef VL_DEBUG
void VCfu::_eval_debug_assertions() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    VCfu::_eval_debug_assertions\n"); );
    // Body
    if (VL_UNLIKELY((cmd_valid & 0xfeU))) {
        Verilated::overWidthError("cmd_valid");}
    if (VL_UNLIKELY((cmd_payload_function_id & 0xfc00U))) {
        Verilated::overWidthError("cmd_payload_function_id");}
    if (VL_UNLIKELY((rsp_ready & 0xfeU))) {
        Verilated::overWidthError("rsp_ready");}
    if (VL_UNLIKELY((reset & 0xfeU))) {
        Verilated::overWidthError("reset");}
    if (VL_UNLIKELY((clk & 0xfeU))) {
        Verilated::overWidthError("clk");}
}
#endif  // VL_DEBUG
