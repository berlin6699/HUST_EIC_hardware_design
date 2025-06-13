module triangle_dds (
    input             [31:0] incre_word,
    decre_word,
    input             [31:0] pha_word,
    input                    clk,
    rstn,
    en,
    output reg signed [13:0] data
);

  reg  [31:0] cnt;
  reg         direction;
  reg         sign;

  wire [31:0] cnt_total;
  wire [31:0] minus_cnt1, minus_cnt2;
  wire [31:0] next_cnt;

  assign cnt_total  = cnt + pha_word;
  assign minus_cnt1 = 32'h8000_0000 - cnt_total;
  assign minus_cnt2 = cnt_total - 32'hFFFF_FFFF;
  assign next_cnt   = (direction == 1'b0) ? cnt_total + incre_word : cnt_total + decre_word;


  always @(posedge clk or negedge rstn) begin
    if (!rstn) begin
      cnt  <= 32'b0;
      data <= 14'sb0;  // 初始化输出
      sign <= 1'b0;
    end
    else begin
      if (en) begin
        if (direction == 1'b0) cnt <= cnt + incre_word;
        else cnt <= cnt + decre_word;

        if ({sign, direction} == 2'b00) data <= cnt_total[31:18] << 1;
        else if ({sign, direction} == 2'b01) begin
          data <= minus_cnt1[31:18] << 1;
          if (minus_cnt1[30] == 1'b1) data <= 14'h1FFF;
        end
        else if ({sign, direction} == 2'b11) data <= minus_cnt1[31:18] << 1;
        else data <= minus_cnt2[31:18] << 1;
      end
    end
  end

  always @(posedge clk or negedge rstn) begin
    if (!rstn) begin
      direction <= 1'b0;
    end
    else begin
      if (en == 1'b1) begin
        if (next_cnt[31] == 1'b1)  // 当最高位为1时切换方向
          sign <= 1'b1;
        else if (next_cnt[31] == 1'b0)  // 当输出为0时切换回递增
          sign <= 1'b0;

        if (next_cnt[31:30] == 2'b01 | next_cnt[31:30] == 2'b10) direction <= 1'b1;
        else if (next_cnt[31:30] == 2'b11 | next_cnt[31:30] == 2'b00) direction <= 1'b0;
      end
    end
  end


endmodule
