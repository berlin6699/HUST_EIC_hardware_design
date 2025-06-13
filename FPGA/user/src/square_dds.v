module square_dds (
    input             [31:0] cnt_word,
    input             [31:0] cmp_word,
    input             [31:0] pha_word,
    input  signed     [13:0] amp_word,
    input                    clk,
    input                    rstn,
    input                    en,
    output reg signed [13:0] data
);

  reg  [31:0] cnt;
  wire [31:0] cnt_total;

  assign cnt_total = cnt + pha_word;

  always @(posedge clk, negedge rstn) begin
    if (!rstn) begin
      cnt <= 1'b0;
      data = 'b0;
    end
    else begin
      if (en) begin
        if (cnt >= cnt_word) begin
          cnt <= 1'b0;
        end
        else begin
          cnt <= cnt + 1'b1;
        end
      end
      else begin
        cnt <= cnt;
      end
    end
  end

  always @(cnt_total) begin
    if (en) begin
      if (cnt_total >= cmp_word) data <= amp_word;
      else data <= 'b0;
    end
  end
endmodule
