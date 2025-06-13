module schmitt_trigger #(
    parameter WD = 14
) (
    input                  clk,
    input                  rstn,
    input  signed [WD-1:0] D,
    input  signed [WD-1:0] threshold,
    output reg             Q
);

  reg signed [WD-1:0] prev_Q;

  always @(posedge clk or negedge rstn) begin
    if (~rstn) begin
      Q      <= 0;
      prev_Q <= 0;
    end
    else begin
      if (D > threshold && prev_Q == 1'b0) begin
        Q <= 1'b1;  // Trigger on rising edge
      end
      else if (D < -threshold && prev_Q == 1'b1) begin
        Q <= 1'b0;  // Trigger on falling edge
      end
      prev_Q <= Q;  // Store previous value of Q
    end
  end

endmodule  //schmitt_trigger
