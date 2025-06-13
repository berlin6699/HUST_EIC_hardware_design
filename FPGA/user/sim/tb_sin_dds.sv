`timescale 1ns / 1ps
module tb_sin_dds ();
  reg        clk;
  reg        rstn;
  reg [31:0] fre_word;  // Frequency control word
  reg [31:0] pha_word;  // Phase control word

  always #5 clk = ~clk;  // 100 MHz clock


  initial begin
    clk  = 0;
    rstn = 0;
    #20ns rstn = 1;  // Release reset after 20 ns 
  end


  initial begin
    fre_word = 32'h28F5C29;  // Frequency control word(1MHz)
    pha_word = 32'h0000_0000;  // Phase control word
    #20ns;
    #1us;  //1 cycle
    pha_word = 32'h8000_0000;  // Phase control word
    #1us;  //1 cycle
    fre_word = 31'hCCCCCCD;  //  Frequency control word(5MHz)
    #400ns;  //2 cycle

    $finish;  // End simulation after reading all addresses
  end

  initial begin
    $dumpfile("./user/sim/tb_sin_dds.vcd");
    $dumpvars(0, tb_sin_dds);
  end

  // output declaration of module sin_dds
  wire [14-1:0] wave_sin;

  sin_dds #(
      .OUTPUT_WIDTH(14),
      .PHASE_WIDTH (32)
  ) u_sin_dds (
      .clock   (clk),
      .rstn    (rstn),
      .fre_word(fre_word),
      .pha_word(pha_word),
      .wave_sin(wave_sin)
  );


endmodule  //tb_sin_dds
