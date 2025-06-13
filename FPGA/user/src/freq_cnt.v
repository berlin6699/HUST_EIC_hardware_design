module freq_cnt #(
    parameter WD = 14
) (
    input                  clk,
    input                  rstn,
    input  signed [WD-1:0] signal_in,
    input  signed [WD-1:0] threshold,
    output reg    [  31:0] cnt_out
);

  reg        prev_Q;
  reg [31:0] cnt;

  schmitt_trigger #(
      .WD(WD)
  ) u_schmitt_trigger (
      .clk      (clk),
      .rstn     (rstn),
      .D        (signal_in),
      .threshold(threshold),
      .Q        (Q)
  );

  always @(posedge clk or negedge rstn) begin
    if (~rstn) begin
      prev_Q  <= 'b0;
      cnt     <= 'b0;
      cnt_out <= 'b0;
    end
    else begin
      prev_Q <= Q;
      if (Q && (~prev_Q)) begin
        cnt     <= 'b0;
        cnt_out <= cnt;
      end
      else cnt <= cnt + 1;
    end
  end

endmodule  //freq_count
