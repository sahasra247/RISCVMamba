`timescale 1ns / 1ps

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
  wire select_mac = cmd_payload_function_id[2:0] == 0;
  wire mac_ready, mac_valid, xm_ready, xm_valid;
  wire [31:0] mac_result, xm_result;
  reg pending, active_mac;
  assign cmd_ready = !pending && (select_mac ? mac_ready : xm_ready);
  assign rsp_valid = pending && (active_mac ? mac_valid : xm_valid);
  assign rsp_payload_outputs_0 = active_mac ? mac_result : xm_result;

  always @(posedge clk) begin
    if (reset) begin
      pending <= 0;
      active_mac <= 0;
    end else begin
      if (cmd_valid && cmd_ready) begin
        pending <= 1;
        active_mac <= select_mac;
      end
      if (rsp_valid && rsp_ready) pending <= 0;
    end
  end

  mb_mac_unit #(.OUT_BITS(8)) u_mac (
    .clk(clk), .reset(reset),
    .cmd_valid(cmd_valid && !pending && select_mac), .cmd_ready(mac_ready),
    .cmd_payload_function_id(cmd_payload_function_id),
    .cmd_payload_inputs_0(cmd_payload_inputs_0),
    .cmd_payload_inputs_1(cmd_payload_inputs_1),
    .rsp_valid(mac_valid), .rsp_ready(rsp_ready && pending && active_mac),
    .rsp_payload_outputs_0(mac_result),
    .dbg_acc()
  );

  mb_xmamba_unit u_xmamba (
    .clk(clk), .reset(reset),
    .cmd_valid(cmd_valid && !pending && !select_mac), .cmd_ready(xm_ready),
    .cmd_payload_function_id(cmd_payload_function_id),
    .cmd_payload_inputs_0(cmd_payload_inputs_0),
    .cmd_payload_inputs_1(cmd_payload_inputs_1),
    .rsp_valid(xm_valid), .rsp_ready(rsp_ready && pending && !active_mac),
    .rsp_payload_outputs_0(xm_result)
  );
endmodule
