`timescale 1ns / 1ps

// Top level expected by CFU Playground. For now it only contains the MAC unit.
// Later: decode funct3 here and route to the nonlinear unit and the scan engine.
module Cfu (
  input               cmd_valid,
  output              cmd_ready,
  input      [9:0]    cmd_payload_function_id,
  input      [31:0]   cmd_payload_inputs_0,
  input      [31:0]   cmd_payload_inputs_1,
  output              rsp_valid,
  input               rsp_ready,
  output     [31:0]   rsp_payload_outputs_0,
  input               reset,
  input               clk
);
  mb_mac_unit #(.OUT_BITS(8)) u_mac (
    .clk(clk), .reset(reset),
    .cmd_valid(cmd_valid), .cmd_ready(cmd_ready),
    .cmd_payload_function_id(cmd_payload_function_id),
    .cmd_payload_inputs_0(cmd_payload_inputs_0),
    .cmd_payload_inputs_1(cmd_payload_inputs_1),
    .rsp_valid(rsp_valid), .rsp_ready(rsp_ready),
    .rsp_payload_outputs_0(rsp_payload_outputs_0),
    .dbg_acc()
  );
endmodule
