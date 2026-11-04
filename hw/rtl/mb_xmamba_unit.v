`timescale 1ns / 1ps

module mb_xmamba_unit #(
    parameter integer LAYERS = 2,
    parameter integer CHANNELS = 8,
    parameter integer STATES = 4
)(
    input wire clk,
    input wire reset,
    input wire cmd_valid,
    output wire cmd_ready,
    input wire [9:0] cmd_payload_function_id,
    input wire [31:0] cmd_payload_inputs_0,
    input wire [31:0] cmd_payload_inputs_1,
    output reg rsp_valid,
    input wire rsp_ready,
    output reg [31:0] rsp_payload_outputs_0
);
    localparam integer STATE_COUNT = LAYERS * CHANNELS * STATES;
    localparam integer CONST_STRIDE = CHANNELS * STATES + CHANNELS;
    reg signed [31:0] hidden [0:STATE_COUNT-1];
    reg signed [15:0] constants [0:LAYERS*CONST_STRIDE-1];
    reg signed [15:0] coeff_b [0:LAYERS*STATES-1];
    reg signed [15:0] coeff_c [0:LAYERS*STATES-1];
    integer layer_id, channel_id, state_id, clear_index, reset_index, reset_layer;
    reg [2:0] phase;
    reg scanning, clearing;
    reg signed [31:0] delta, activation, decay, injection, output_sum;
    wire [2:0] group_id = cmd_payload_function_id[2:0];
    wire [6:0] operation = cmd_payload_function_id[9:3];
    wire signed [31:0] operand0 = $signed(cmd_payload_inputs_0);
    wire signed [15:0] low_half = $signed(cmd_payload_inputs_0[15:0]);
    wire signed [15:0] high_half = $signed(cmd_payload_inputs_0[31:16]);
    wire signed [15:0] constant_value = constants[layer_id * CONST_STRIDE + channel_id * STATES + state_id];
    wire signed [15:0] skip_value = constants[layer_id * CONST_STRIDE + CHANNELS * STATES + channel_id];
    wire signed [15:0] current_b = coeff_b[layer_id * STATES + state_id];
    wire signed [15:0] current_c = coeff_c[layer_id * STATES + state_id];
    wire signed [31:0] current_hidden = hidden[(layer_id * CHANNELS + channel_id) * STATES + state_id];

    assign cmd_ready = !rsp_valid && !scanning && !clearing;

    `include "xmamba_tables.vh"

    function signed [31:0] saturate;
        input signed [63:0] value;
        begin
            if (value > 64'sd2147483647) saturate = 32'sh7fffffff;
            else if (value < -64'sd2147483648) saturate = 32'sh80000000;
            else saturate = value[31:0];
        end
    endfunction

    function signed [31:0] qmul;
        input signed [31:0] left_value, right_value;
        reg signed [63:0] product;
        begin
            product = left_value * right_value;
            qmul = saturate(product >>> 8);
        end
    endfunction

    function signed [31:0] qadd;
        input signed [31:0] left_value, right_value;
        begin
            qadd = saturate({{32{left_value[31]}}, left_value} + {{32{right_value[31]}}, right_value});
        end
    endfunction

    function signed [31:0] nonlinear;
        input [6:0] kind;
        input signed [31:0] value;
        reg signed [31:0] position, fraction, lower_value, upper_value;
        reg [5:0] table_index;
        begin
            if (kind == 0 && value >= 0) nonlinear = 32'sd256;
            else if (value <= -2048) nonlinear = 0;
            else if (kind != 0 && value >= 2048) nonlinear = value;
            else begin
                position = (kind == 0) ? -value : value + 2048;
                table_index = position[12:7];
                fraction = {25'd0, position[6:0]};
                case (kind)
                    0: begin lower_value = exp_table(table_index); upper_value = exp_table(table_index + 6'd1); end
                    1: begin lower_value = silu_table(table_index); upper_value = silu_table(table_index + 6'd1); end
                    default: begin lower_value = softplus_table(table_index); upper_value = softplus_table(table_index + 6'd1); end
                endcase
                nonlinear = lower_value + (((upper_value - lower_value) * fraction) >>> 7);
            end
        end
    endfunction

    always @(posedge clk) begin
        if (reset) begin
            rsp_valid <= 0;
            rsp_payload_outputs_0 <= 0;
            scanning <= 0;
            clearing <= 0;
            layer_id <= 0;
            channel_id <= 0;
            state_id <= 0;
            clear_index <= 0;
            phase <= 0;
            delta <= 0;
            activation <= 0;
            decay <= 0;
            injection <= 0;
            output_sum <= 0;
            for (reset_index = 0; reset_index < STATE_COUNT; reset_index = reset_index + 1)
                hidden[reset_index] <= 0;
            for (reset_layer = 0; reset_layer < LAYERS; reset_layer = reset_layer + 1)
                for (reset_index = 0; reset_index < CONST_STRIDE; reset_index = reset_index + 1)
                    constants[reset_layer * CONST_STRIDE + reset_index] <= 0;
            for (reset_index = 0; reset_index < LAYERS * STATES; reset_index = reset_index + 1) begin
                coeff_b[reset_index] <= 0;
                coeff_c[reset_index] <= 0;
            end
        end else begin
            if (rsp_valid && rsp_ready) rsp_valid <= 0;
            if (cmd_valid && cmd_ready) begin
                rsp_payload_outputs_0 <= 0;
                rsp_valid <= 1;
                if (group_id == 1 && operation < 3)
                    rsp_payload_outputs_0 <= nonlinear(operation, operand0);
                else if (group_id == 2) begin
                    case (operation)
                        0: if (cmd_payload_inputs_0 < CONST_STRIDE)
                            constants[layer_id * CONST_STRIDE + cmd_payload_inputs_0] <= cmd_payload_inputs_1[15:0];
                        1: if (cmd_payload_inputs_0 < LAYERS) layer_id <= cmd_payload_inputs_0;
                        2: if (cmd_payload_inputs_0 < STATES) begin
                            coeff_b[layer_id * STATES + cmd_payload_inputs_0] <= cmd_payload_inputs_1[15:0];
                            coeff_c[layer_id * STATES + cmd_payload_inputs_0] <= cmd_payload_inputs_1[31:16];
                        end
                        3: if (cmd_payload_inputs_1 < CHANNELS) begin
                            channel_id <= cmd_payload_inputs_1;
                            delta <= {{16{low_half[15]}}, low_half};
                            activation <= {{16{high_half[15]}}, high_half};
                            state_id <= 0;
                            output_sum <= 0;
                            phase <= 0;
                            scanning <= 1;
                            rsp_valid <= 0;
                        end
                        4: begin clearing <= 1; clear_index <= 0; rsp_valid <= 0; end
                        default: begin end
                    endcase
                end
            end
            if (clearing) begin
                hidden[clear_index] <= 0;
                if (clear_index == STATE_COUNT - 1) begin
                    clearing <= 0;
                    rsp_valid <= 1;
                    rsp_payload_outputs_0 <= 0;
                end else clear_index <= clear_index + 1;
            end
            if (scanning) begin
                case (phase)
                    0: begin
                        decay <= nonlinear(0, qmul(delta, {{16{constant_value[15]}}, constant_value}));
                        injection <= qmul(qmul(delta, {{16{current_b[15]}}, current_b}), activation);
                        phase <= 1;
                    end
                    1: begin
                        hidden[(layer_id * CHANNELS + channel_id) * STATES + state_id] <=
                            qadd(qmul(decay, current_hidden), injection);
                        phase <= 2;
                    end
                    2: begin
                        output_sum <= qadd(output_sum, qmul({{16{current_c[15]}}, current_c}, current_hidden));
                        if (state_id == STATES - 1) phase <= 3;
                        else begin state_id <= state_id + 1; phase <= 0; end
                    end
                    default: begin
                        rsp_payload_outputs_0 <= qadd(output_sum, qmul({{16{skip_value[15]}}, skip_value}, activation));
                        scanning <= 0;
                        rsp_valid <= 1;
                    end
                endcase
            end
        end
    end
endmodule