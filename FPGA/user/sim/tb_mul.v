module tb_mul ();
  reg                clk;
  reg                reset;

  reg signed  [17:0] a;
  reg signed  [17:0] b;
  wire signed [35:0] c;


  initial begin

    reset = 1;
    clk   = 0;
    a     = 0;
    b     = 0;

    #200 reset = 0;

    #20 a = -19;
    b = 253;

    #20 a = -22;
    b = -33;

    #20 a = -35;
    b = 46;

    #20 a = 8;
    b = 9;

  end

  always #10 clk = ~clk;


  MULT_MACRO #(
      .DEVICE ("7SERIES"),  // Target Device: "7SERIES" 
      .LATENCY(3),          // Desired clock cycle latency, 0-4
      .WIDTH_A(18),         // Multiplier A-input bus width, 1-25
      .WIDTH_B(18)          // Multiplier B-input bus width, 1-18
  ) MULT_MACRO_inst (
      .P  (c),     // Multiplier output bus, width determined by WIDTH_P parameter
      .A  (a),     // Multiplier input A bus, width determined by WIDTH_A parameter
      .B  (b),     // Multiplier input B bus, width determined by WIDTH_B parameter
      .CE (1'b1),  // 1-bit active high input clock enable
      .CLK(clk),   // 1-bit positive edge clock input
      .RST(reset)  // 1-bit input active high reset
  );
endmodule  //tb_mul
