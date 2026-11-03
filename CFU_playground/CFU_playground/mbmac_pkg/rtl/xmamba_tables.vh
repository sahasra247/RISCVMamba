function signed [31:0] exp_table;
  input [5:0] index;
  begin
    case (index)
      6'd0: exp_table = 32'sd256;
      6'd1: exp_table = 32'sd155;
      6'd2: exp_table = 32'sd94;
      6'd3: exp_table = 32'sd57;
      6'd4: exp_table = 32'sd35;
      6'd5: exp_table = 32'sd21;
      6'd6: exp_table = 32'sd13;
      6'd7: exp_table = 32'sd8;
      6'd8: exp_table = 32'sd5;
      6'd9: exp_table = 32'sd3;
      6'd10: exp_table = 32'sd2;
      6'd11: exp_table = 32'sd1;
      6'd12: exp_table = 32'sd1;
      6'd13: exp_table = 32'sd0;
      6'd14: exp_table = 32'sd0;
      6'd15: exp_table = 32'sd0;
      6'd16: exp_table = 32'sd0;
      default: exp_table = 32'sd0;
    endcase
  end
endfunction
function signed [31:0] silu_table;
  input [5:0] index;
  begin
    case (index)
      6'd0: silu_table = -32'sd1;
      6'd1: silu_table = -32'sd1;
      6'd2: silu_table = -32'sd2;
      6'd3: silu_table = -32'sd2;
      6'd4: silu_table = -32'sd4;
      6'd5: silu_table = -32'sd6;
      6'd6: silu_table = -32'sd9;
      6'd7: silu_table = -32'sd13;
      6'd8: silu_table = -32'sd18;
      6'd9: silu_table = -32'sd26;
      6'd10: silu_table = -32'sd36;
      6'd11: silu_table = -32'sd49;
      6'd12: silu_table = -32'sd61;
      6'd13: silu_table = -32'sd70;
      6'd14: silu_table = -32'sd69;
      6'd15: silu_table = -32'sd48;
      6'd16: silu_table = 32'sd0;
      6'd17: silu_table = 32'sd80;
      6'd18: silu_table = 32'sd187;
      6'd19: silu_table = 32'sd314;
      6'd20: silu_table = 32'sd451;
      6'd21: silu_table = 32'sd591;
      6'd22: silu_table = 32'sd732;
      6'd23: silu_table = 32'sd870;
      6'd24: silu_table = 32'sd1006;
      6'd25: silu_table = 32'sd1139;
      6'd26: silu_table = 32'sd1271;
      6'd27: silu_table = 32'sd1402;
      6'd28: silu_table = 32'sd1532;
      6'd29: silu_table = 32'sd1662;
      6'd30: silu_table = 32'sd1790;
      6'd31: silu_table = 32'sd1919;
      6'd32: silu_table = 32'sd2047;
      default: silu_table = 32'sd0;
    endcase
  end
endfunction
function signed [31:0] softplus_table;
  input [5:0] index;
  begin
    case (index)
      6'd0: softplus_table = 32'sd0;
      6'd1: softplus_table = 32'sd0;
      6'd2: softplus_table = 32'sd0;
      6'd3: softplus_table = 32'sd0;
      6'd4: softplus_table = 32'sd1;
      6'd5: softplus_table = 32'sd1;
      6'd6: softplus_table = 32'sd2;
      6'd7: softplus_table = 32'sd3;
      6'd8: softplus_table = 32'sd5;
      6'd9: softplus_table = 32'sd8;
      6'd10: softplus_table = 32'sd12;
      6'd11: softplus_table = 32'sd20;
      6'd12: softplus_table = 32'sd32;
      6'd13: softplus_table = 32'sd52;
      6'd14: softplus_table = 32'sd80;
      6'd15: softplus_table = 32'sd121;
      6'd16: softplus_table = 32'sd177;
      6'd17: softplus_table = 32'sd249;
      6'd18: softplus_table = 32'sd336;
      6'd19: softplus_table = 32'sd436;
      6'd20: softplus_table = 32'sd544;
      6'd21: softplus_table = 32'sd660;
      6'd22: softplus_table = 32'sd780;
      6'd23: softplus_table = 32'sd904;
      6'd24: softplus_table = 32'sd1029;
      6'd25: softplus_table = 32'sd1155;
      6'd26: softplus_table = 32'sd1282;
      6'd27: softplus_table = 32'sd1409;
      6'd28: softplus_table = 32'sd1537;
      6'd29: softplus_table = 32'sd1664;
      6'd30: softplus_table = 32'sd1792;
      6'd31: softplus_table = 32'sd1920;
      6'd32: softplus_table = 32'sd2048;
      default: softplus_table = 32'sd0;
    endcase
  end
endfunction
