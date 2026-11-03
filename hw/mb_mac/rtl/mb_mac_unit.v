`timescale 1ns / 1ps

// mb_mac_unit: the mb.mac4 and mb.racc instructions of the RISCVMamba/Xmamba CFU.
//
//   function id = {funct7, funct3}; both instructions use funct3 = 0
//     funct7 = 0  mb.mac4 rs1, rs2 : acc += sum_{i=0..3} int8(rs1.lane[i]) * int8(rs2.lane[i])
//                                    lane 0 = bits [7:0] ... lane 3 = bits [31:24]
//                                    response value is 0
//     funct7 = 1  mb.racc rd, rs1  : rd = saturate_OUT_BITS( (acc + 2^(s-1)) >>> s ), s = rs1[4:0]
//                                    (round half up; no rounding term when s = 0), then acc = 0
//     anything else               : responds with 0, state unchanged (never hangs the CPU)
//
// Accumulator: 32-bit two's complement, wraps on overflow (no saturation).
//   One mac4 adds at most 4 * 128 * 128 = 2^16, so overflow needs >= 2^15 mac4 ops.
//
// Timing (CFU handshake, registered response):
//   mb.mac4: accepted on edge A, response visible after A, accumulator updated after A+2.
//   mb.racc: waits until earlier mac4 work has reached the accumulator, then answers.
//   cmd_ready is low while a response is pending or a racc is running.
//
// Reset is synchronous and active high, as in CFU Playground.
module mb_mac_unit #(
    parameter integer OUT_BITS = 8          // racc saturates to a signed OUT_BITS-bit range
)(
    input  wire        clk,
    input  wire        reset,
    input  wire        cmd_valid,
    output wire        cmd_ready,
    input  wire [9:0]  cmd_payload_function_id,
    input  wire [31:0] cmd_payload_inputs_0,
    input  wire [31:0] cmd_payload_inputs_1,
    output wire        rsp_valid,
    input  wire        rsp_ready,
    output wire [31:0] rsp_payload_outputs_0,
    output wire [31:0] dbg_acc              // test/debug view of the accumulator
);
    // ------------------------------------------------------------------ decode
    wire [2:0] funct3 = cmd_payload_function_id[2:0];
    wire [6:0] funct7 = cmd_payload_function_id[9:3];
    wire is_mac4 = (funct3 == 3'd0) && (funct7 == 7'd0);
    wire is_racc = (funct3 == 3'd0) && (funct7 == 7'd1);

    reg        rsp_valid_r;
    reg [31:0] rsp_data_r;
    reg        racc_busy;
    reg [4:0]  racc_shift;

    assign cmd_ready             = ~rsp_valid_r & ~racc_busy;
    assign rsp_valid             = rsp_valid_r;
    assign rsp_payload_outputs_0 = rsp_data_r;

    wire accept = cmd_valid & cmd_ready;

    // ------------------------------------------- datapath (no reset needed)
    // Stage 1: four signed 8x8 products, registered.
    // Stage 2: sum of the four products (18-bit signed), registered.
    // Stage 3: add into the 32-bit accumulator (in the control block below).
    wire signed [7:0] a0 = cmd_payload_inputs_0[7:0];
    wire signed [7:0] a1 = cmd_payload_inputs_0[15:8];
    wire signed [7:0] a2 = cmd_payload_inputs_0[23:16];
    wire signed [7:0] a3 = cmd_payload_inputs_0[31:24];
    wire signed [7:0] b0 = cmd_payload_inputs_1[7:0];
    wire signed [7:0] b1 = cmd_payload_inputs_1[15:8];
    wire signed [7:0] b2 = cmd_payload_inputs_1[23:16];
    wire signed [7:0] b3 = cmd_payload_inputs_1[31:24];

    wire signed [15:0] m0 = a0 * b0;
    wire signed [15:0] m1 = a1 * b1;
    wire signed [15:0] m2 = a2 * b2;
    wire signed [15:0] m3 = a3 * b3;

    reg signed [15:0] prod0, prod1, prod2, prod3;
    reg signed [17:0] psum;

    wire signed [17:0] e0 = {{2{prod0[15]}}, prod0};
    wire signed [17:0] e1 = {{2{prod1[15]}}, prod1};
    wire signed [17:0] e2 = {{2{prod2[15]}}, prod2};
    wire signed [17:0] e3 = {{2{prod3[15]}}, prod3};

    always @(posedge clk) begin
        prod0 <= m0;
        prod1 <= m1;
        prod2 <= m2;
        prod3 <= m3;
        psum  <= e0 + e1 + e2 + e3;
    end

    // ------------------------------------------------- racc: round + saturate
    reg signed [31:0] acc;
    assign dbg_acc = acc;

    localparam signed [32:0] SAT_MAX =  (33'sd1 <<< (OUT_BITS - 1)) - 33'sd1;
    localparam signed [32:0] SAT_MIN = -(33'sd1 <<< (OUT_BITS - 1));

    wire signed [32:0] acc_ext  = {acc[31], acc};
    wire signed [32:0] rnd_add  = (racc_shift == 5'd0) ? 33'sd0
                                  : (33'sd1 <<< (racc_shift - 5'd1));
    wire signed [32:0] biased   = acc_ext + rnd_add;
    wire signed [32:0] shifted  = biased >>> racc_shift;
    wire signed [32:0] sat      = (shifted > SAT_MAX) ? SAT_MAX :
                                  (shifted < SAT_MIN) ? SAT_MIN : shifted;
    wire        [31:0] racc_result = sat[31:0];

    // ----------------------------------------------------------------- control
    reg v1, v2;                          // mac4 work in stage 1 / stage 2
    wire racc_done = racc_busy & ~v1 & ~v2;

    always @(posedge clk) begin
        if (reset) begin
            v1          <= 1'b0;
            v2          <= 1'b0;
            rsp_valid_r <= 1'b0;
            rsp_data_r  <= 32'd0;
            racc_busy   <= 1'b0;
            racc_shift  <= 5'd0;
            acc         <= 32'sd0;
        end else begin
            v1 <= accept & is_mac4;
            v2 <= v1;

            // the CPU took the response
            if (rsp_valid_r & rsp_ready) rsp_valid_r <= 1'b0;

            // a new command (cmd_ready guarantees no response is pending)
            if (accept) begin
                if (is_racc) begin
                    racc_busy  <= 1'b1;
                    racc_shift <= cmd_payload_inputs_0[4:0];
                end else begin
                    rsp_valid_r <= 1'b1;
                    rsp_data_r  <= 32'd0;       // mac4 and unknown ops return 0
                end
            end

            // stage 3: accumulate
            if (v2) acc <= acc + $signed({{14{psum[17]}}, psum});

            // racc: finish once the accumulator is up to date
            if (racc_done) begin
                rsp_data_r  <= racc_result;
                rsp_valid_r <= 1'b1;
                racc_busy   <= 1'b0;
                acc         <= 32'sd0;
            end
        end
    end
endmodule
