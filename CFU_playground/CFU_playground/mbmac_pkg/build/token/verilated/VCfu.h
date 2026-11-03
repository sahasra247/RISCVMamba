// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Primary design header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef _VCFU_H_
#define _VCFU_H_  // guard

#include "verilated.h"

//==========

class VCfu__Syms;

//----------

VL_MODULE(VCfu) {
  public:
    
    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    VL_IN8(clk,0,0);
    VL_IN8(cmd_valid,0,0);
    VL_OUT8(cmd_ready,0,0);
    VL_OUT8(rsp_valid,0,0);
    VL_IN8(rsp_ready,0,0);
    VL_IN8(reset,0,0);
    VL_IN16(cmd_payload_function_id,9,0);
    VL_IN(cmd_payload_inputs_0,31,0);
    VL_IN(cmd_payload_inputs_1,31,0);
    VL_OUT(rsp_payload_outputs_0,31,0);
    
    // LOCAL SIGNALS
    // Internals; generally not touched by application code
    CData/*0:0*/ Cfu__DOT__mac_ready;
    CData/*0:0*/ Cfu__DOT__xm_ready;
    CData/*0:0*/ Cfu__DOT__xm_valid;
    CData/*0:0*/ Cfu__DOT__pending;
    CData/*0:0*/ Cfu__DOT__active_mac;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__is_racc;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__rsp_valid_r;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__racc_busy;
    CData/*4:0*/ Cfu__DOT__u_mac__DOT__racc_shift;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__accept;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__v1;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__v2;
    CData/*0:0*/ Cfu__DOT__u_mac__DOT__racc_done;
    CData/*2:0*/ Cfu__DOT__u_xmamba__DOT__phase;
    CData/*0:0*/ Cfu__DOT__u_xmamba__DOT__scanning;
    CData/*0:0*/ Cfu__DOT__u_xmamba__DOT__clearing;
    SData/*15:0*/ Cfu__DOT__u_mac__DOT__prod0;
    SData/*15:0*/ Cfu__DOT__u_mac__DOT__prod1;
    SData/*15:0*/ Cfu__DOT__u_mac__DOT__prod2;
    SData/*15:0*/ Cfu__DOT__u_mac__DOT__prod3;
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__constant_value;
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__skip_value;
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__current_b;
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__current_c;
    IData/*31:0*/ Cfu__DOT__xm_result;
    IData/*31:0*/ Cfu__DOT__u_mac__DOT__rsp_data_r;
    IData/*17:0*/ Cfu__DOT__u_mac__DOT__psum;
    IData/*31:0*/ Cfu__DOT__u_mac__DOT__acc;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__layer_id;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__channel_id;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__state_id;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__clear_index;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__delta;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__activation;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__decay;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__injection;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__output_sum;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__current_hidden;
    QData/*32:0*/ Cfu__DOT__u_mac__DOT__shifted;
    IData/*31:0*/ Cfu__DOT__u_xmamba__DOT__hidden[64];
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__constants[80];
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__coeff_b[8];
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT__coeff_c[8];
    
    // LOCAL VARIABLES
    // Internals; generally not touched by application code
    CData/*0:0*/ Cfu__DOT____Vcellinp__u_xmamba__cmd_valid;
    CData/*0:0*/ __Vclklast__TOP__clk;
    SData/*15:0*/ Cfu__DOT__u_xmamba__DOT____Vlvbound2;
    
    // INTERNAL VARIABLES
    // Internals; generally not touched by application code
    VCfu__Syms* __VlSymsp;  // Symbol table
    
    // CONSTRUCTORS
  private:
    VL_UNCOPYABLE(VCfu);  ///< Copying not allowed
  public:
    /// Construct the model; called by application code
    /// The special name  may be used to make a wrapper with a
    /// single model invisible with respect to DPI scope names.
    VCfu(const char* name = "TOP");
    /// Destroy the model; called (often implicitly) by application code
    ~VCfu();
    
    // API METHODS
    /// Evaluate the model.  Application must call when inputs change.
    void eval() { eval_step(); }
    /// Evaluate when calling multiple units/models per time step.
    void eval_step();
    /// Evaluate at end of a timestep for tracing, when using eval_step().
    /// Application must call after all eval() and before time changes.
    void eval_end_step() {}
    /// Simulation complete, run final blocks.  Application must call on completion.
    void final();
    
    // INTERNAL METHODS
  private:
    static void _eval_initial_loop(VCfu__Syms* __restrict vlSymsp);
  public:
    void __Vconfigure(VCfu__Syms* symsp, bool first);
  private:
    static QData _change_request(VCfu__Syms* __restrict vlSymsp);
    static QData _change_request_1(VCfu__Syms* __restrict vlSymsp);
  public:
    static void _combo__TOP__3(VCfu__Syms* __restrict vlSymsp);
  private:
    void _ctor_var_reset() VL_ATTR_COLD;
  public:
    static void _eval(VCfu__Syms* __restrict vlSymsp);
  private:
#ifdef VL_DEBUG
    void _eval_debug_assertions();
#endif  // VL_DEBUG
  public:
    static void _eval_initial(VCfu__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _eval_settle(VCfu__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _sequent__TOP__1(VCfu__Syms* __restrict vlSymsp);
    static void _settle__TOP__2(VCfu__Syms* __restrict vlSymsp) VL_ATTR_COLD;
} VL_ATTR_ALIGNED(VL_CACHE_LINE_BYTES);

//----------


#endif  // guard
