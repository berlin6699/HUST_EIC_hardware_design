`timescale 1ns / 1ps
module tb_dds ();
  reg        clk;
  reg        rstn;
  reg [ 1:0] wave_type;  // Waveform type (0: sine, 1: triangle, 2: square)
  reg [15:0] amp_word;  // Amplitude control word
  reg [31:0] sin_fre_word;  // Frequency control word
  reg [31:0] pha_word;  // Phase control word
  reg [31:0] sqr_cnt_word;  // Square wave counter word
  reg [31:0] sqr_cmp_word;  // Square wave compare word
  always #5 clk = ~clk;  // 100 MHz clock


  initial begin
    clk  = 0;
    rstn = 0;
    #80ns rstn = 1;  // Release reset after 20 ns 
  end


  initial begin
    amp_word     = 16'h2000;  // Amplitude control word
    sin_fre_word = 32'h28F5C29;  // Frequency control word(1MHz)
    pha_word     = 32'h0000_0000;  // Phase control word
    sqr_cnt_word = 32'd100;
    sqr_cmp_word = 32'd50;  // Square wave compare word
    wave_type    = 2'b00;  // Sine wave
    #80ns;
    #10us;  //10 cycle
    pha_word  = 32'h8000_0000;  // Phase control word
    amp_word  = 16'h1000;  // Amplitude control word
    wave_type = 2'b01;  // Triangle wave
    #10us;  //10 cycle

    amp_word = 16'h0800;  // Amplitude control word
    #10us;
    amp_word = 16'h0400;  // Amplitude control word
    #10us;
    amp_word = 16'h0200;  // Amplitude control word
    #10us;
    amp_word = 16'h0100;  // Amplitude control word
    #10us;
    amp_word = 16'h0080;  // Amplitude control word
    #10us;

    pha_word  = 32'h0000_0000;  // Phase control word
    amp_word  = 16'h0800;  // Amplitude control word
    wave_type = 2'b10;  // Square wave
    #10us;  //10 cycle


    $finish;  // End simulation after reading all addresses
  end

  // initial begin
  //   $dumpfile("./user/sim/tb_dds.vcd");
  //   $dumpvars(0, tb_dds);
  // end

  // output declaration of module dds
  wire [13:0] wave_out;

  dds u_dds (
      .clock         (clk),
      .rstn          (rstn),
      .en            (1),
      .wave_type     (wave_type),
      .amp_word      (amp_word),
      .pha_word      (pha_word),
      .sin_fre_word  (sin_fre_word),
      .tri_incre_word(sin_fre_word),
      .tri_decre_word(sin_fre_word),
      .sqr_cnt_word  (sqr_cnt_word),
      .sqr_cmp_word  (sqr_cmp_word),
      .wave_out      (wave_out)
  );


endmodule  //tb_sin_dds
