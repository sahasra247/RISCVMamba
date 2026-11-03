`timescale 1ns / 1ps

// Self-checking testbench for mb_mac_unit (mb.mac4 / mb.racc).
// Behaves like the CPU: one command at a time, waits for its response, with random
// idle gaps and random rsp_ready stalls. Operands are scrambled after the accept
// edge to prove the unit latches them (they are valid for one cycle only).
//
// Plusargs: +VECTORS=<file>  +COUNT=<records>
// Record (36 hex digits): header16 | rs1 | rs2 | expected_rsp | expected_acc
//   header[9:0] = function id, header[15] = also compare accumulator after the response
module tb_mb_mac;
    parameter integer OUT_BITS = 8;
    parameter integer STALL_PCT = 30;
    localparam integer MAX_VECTORS = 400000;

    reg clk = 0;
    always #5 clk = ~clk;

    reg         reset = 1;
    reg         cmd_valid = 0;
    reg  [9:0]  fid = 0;
    reg  [31:0] in0 = 0, in1 = 0;
    reg         rsp_ready = 0;
    wire        cmd_ready, rsp_valid;
    wire [31:0] rsp_data, dbg_acc;

    mb_mac_unit #(.OUT_BITS(OUT_BITS)) dut (
        .clk(clk), .reset(reset),
        .cmd_valid(cmd_valid), .cmd_ready(cmd_ready),
        .cmd_payload_function_id(fid),
        .cmd_payload_inputs_0(in0), .cmd_payload_inputs_1(in1),
        .rsp_valid(rsp_valid), .rsp_ready(rsp_ready),
        .rsp_payload_outputs_0(rsp_data),
        .dbg_acc(dbg_acc)
    );

    reg [143:0] vectors [0:MAX_VECTORS-1];
    integer count, index;
    integer seed;
    integer n_mac, n_racc, n_other, n_acc_checks, n_stall_cycles, n_cycles;
    string filename;

    function integer rnd(input integer n);
        begin
            rnd = $unsigned($random(seed)) % n;
        end
    endfunction

    // Random rsp_ready, changed on the falling edge (away from the sampling edge).
    always @(negedge clk) rsp_ready = (rnd(100) >= STALL_PCT);

    // ---------------------------------------------------------------- monitor
    // Protocol rules: a pending response is held until accepted; no response without a
    // command; outputs are never X after reset.
    reg        prev_valid = 0, prev_ready = 1;
    reg [31:0] prev_data = 0;
    integer    accepted_cmds = 0, taken_rsps = 0;
    always @(posedge clk) begin
        if (!reset) begin
            if (rsp_valid !== 1'b0 && rsp_valid !== 1'b1)
                $fatal(1, "rsp_valid is X/Z at %0t", $time);
            if (cmd_ready !== 1'b0 && cmd_ready !== 1'b1)
                $fatal(1, "cmd_ready is X/Z at %0t", $time);
            if (prev_valid && !prev_ready) begin
                if (!rsp_valid)
                    $fatal(1, "rsp_valid dropped before rsp_ready at %0t", $time);
                if (rsp_data !== prev_data)
                    $fatal(1, "rsp data changed while stalled at %0t", $time);
            end
            if (rsp_valid && rsp_ready) begin
                if (taken_rsps >= accepted_cmds)
                    $fatal(1, "response without an outstanding command at %0t", $time);
                taken_rsps = taken_rsps + 1;
            end
            if (cmd_valid && cmd_ready) accepted_cmds = accepted_cmds + 1;
            if (rsp_valid && !rsp_ready) n_stall_cycles = n_stall_cycles + 1;
        end
        prev_valid <= rsp_valid;
        prev_ready <= rsp_ready;
        prev_data  <= rsp_data;
        n_cycles = n_cycles + 1;
    end

    // ------------------------------------------------------------- one command
    task automatic do_row(input integer idx, input [143:0] rec);
        reg [15:0] hdr;
        reg [31:0] rs1, rs2, exp_rsp, exp_acc, got_rsp;
        integer gap, cyc;
        reg accepted, got;
        begin
            hdr = rec[143:128];
            rs1 = rec[127:96];
            rs2 = rec[95:64];
            exp_rsp = rec[63:32];
            exp_acc = rec[31:0];

            gap = (rnd(100) < 50) ? 0 : rnd(5);
            repeat (gap) @(posedge clk);
            #1;
            cmd_valid = 1; fid = hdr[9:0]; in0 = rs1; in1 = rs2;

            // wait for the edge where cmd_valid && cmd_ready (sampled before the edge)
            accepted = 0; cyc = 0;
            while (!accepted) begin
                @(posedge clk);
                if (cmd_ready) accepted = 1;
                cyc = cyc + 1;
                if (!accepted && cyc > 2000)
                    $fatal(1, "row %0d: cmd_ready never rose (hang)", idx);
                #1;
            end
            cmd_valid = 0;
            fid = $random(seed); in0 = $random(seed); in1 = $random(seed);

            // wait for the edge where rsp_valid && rsp_ready
            got = 0; cyc = 0;
            while (!got) begin
                @(posedge clk);
                if (rsp_valid && rsp_ready) begin
                    got = 1;
                    got_rsp = rsp_data;
                end
                cyc = cyc + 1;
                if (!got && cyc > 2000)
                    $fatal(1, "row %0d: response never arrived (hang), fid=%03x", idx, hdr[9:0]);
            end
            #1;
            if (got_rsp !== exp_rsp)
                $fatal(1, "row %0d: fid=%03x rs1=%08x rs2=%08x rsp=%08x expected=%08x",
                       idx, hdr[9:0], rs1, rs2, got_rsp, exp_rsp);

            if (hdr[15]) begin
                repeat (3) @(posedge clk);
                #1;
                if (dbg_acc !== exp_acc)
                    $fatal(1, "row %0d: fid=%03x rs1=%08x rs2=%08x acc=%08x expected=%08x",
                           idx, hdr[9:0], rs1, rs2, dbg_acc, exp_acc);
                n_acc_checks = n_acc_checks + 1;
            end

            if (hdr[9:0] == 10'h000) n_mac = n_mac + 1;
            else if (hdr[9:0] == 10'h008) n_racc = n_racc + 1;
            else n_other = n_other + 1;
        end
    endtask

    // -------------------------------------------------------------------- main
    initial begin
        seed = 32'h1badf00d;
        n_mac = 0; n_racc = 0; n_other = 0; n_acc_checks = 0; n_stall_cycles = 0; n_cycles = 0;
        if (!$value$plusargs("VECTORS=%s", filename)) $fatal(1, "Missing +VECTORS");
        if (!$value$plusargs("COUNT=%d", count)) $fatal(1, "Missing +COUNT");
        if (count < 1 || count > MAX_VECTORS) $fatal(1, "Invalid vector count %0d", count);
        $readmemh(filename, vectors, 0, count - 1);

        // reset
        reset = 1;
        repeat (4) @(posedge clk);
        #1 reset = 0;
        @(posedge clk); #1;
        if (cmd_ready !== 1'b1 || rsp_valid !== 1'b0 || dbg_acc !== 32'd0)
            $fatal(1, "bad state after reset: ready=%b valid=%b acc=%h", cmd_ready, rsp_valid, dbg_acc);

        for (index = 0; index < count; index = index + 1)
            do_row(index, vectors[index]);

        // reset must clear the accumulator and any pending state
        do_row(count, {16'h8000, 32'h01010101, 32'h01010101, 32'd0, 32'd4});
        @(negedge clk); reset = 1;
        @(posedge clk); #1;
        reset = 0;
        @(posedge clk); #1;
        if (dbg_acc !== 32'd0 || rsp_valid !== 1'b0 || cmd_ready !== 1'b1)
            $fatal(1, "reset did not clear the unit");

        $display("PASS mb_mac OUT_BITS=%0d records=%0d mac4=%0d racc=%0d other=%0d acc_checks=%0d stall_cycles=%0d cycles=%0d",
                 OUT_BITS, count, n_mac, n_racc, n_other, n_acc_checks, n_stall_cycles, n_cycles);
        $finish;
    end

    // global watchdog
    initial begin
        #1000000000;
        $fatal(1, "global watchdog expired");
    end
endmodule
