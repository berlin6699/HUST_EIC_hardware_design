module clk_even_div #(
    parameter i_clk_fre = 100_000_000,
    parameter o_clk_fre = 1
) (
    input      i_clk,
    input      i_rstn,
    output     o_clk_p,
    output     o_clk_n,
    output reg o_clk_en
);
  localparam NUM_DIV = i_clk_fre / o_clk_fre;
  localparam WD = $clog2(NUM_DIV / 2);
  reg [WD-1:0] r_cnt = 0;
  reg          o_r_clk = 0;

  always @(posedge i_clk or negedge i_rstn) begin
    if (!i_rstn) begin
      r_cnt    <= 'd0;
      o_clk_en <= 'd0;
      o_r_clk  <= 1'b0;
    end
    else if (r_cnt < NUM_DIV / 2 - 1) begin
      r_cnt    <= r_cnt + 1'b1;
      o_clk_en <= 'd0;
      o_r_clk  <= o_r_clk;
    end
    else begin
      o_clk_en <= ~o_r_clk;
      r_cnt    <= 'd0;
      o_r_clk  <= ~o_r_clk;
    end
  end
  assign o_clk_p = o_r_clk;
  assign o_clk_n = ~o_r_clk;

endmodule  //divider
