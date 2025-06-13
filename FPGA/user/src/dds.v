module dds (
    // Clock input
    input                clock,
    // Asynchronous rstn with active low
    input                rstn,
    // Enable signal
    input                en,
    // Wave select 00: Sine, 01: Triangle, 10: Square
    input         [ 1:0] wave_type,
    // Amplitude control word, [15:0] for sine and triangle waves, [13:0] for square wave
    input  signed [15:0] amp_word,
    // Phase control word
    input         [31:0] pha_word,
    // Frequency control word for sine wave
    input         [31:0] sin_fre_word,
    // Increment word for triangle wave
    input         [31:0] tri_incre_word,
    // Decrement word for triangle wave
    input         [31:0] tri_decre_word,
    // Count word for square wave
    input         [31:0] sqr_cnt_word,
    // Compare word for square wave
    input         [31:0] sqr_cmp_word,
    // Output sine wave
    output signed [13:0] wave_out
);

  `define SINE_WAVE 2'b00
  `define TRIANGLE_WAVE 2'b01
  `define SQUARE_WAVE 2'b10

  wire signed [13:0] sine_wave;
  wire signed [13:0] triangle_wave;
  wire signed [13:0] square_wave;

  wire signed [13:0] mul_in;  // Multiplier input A
  wire signed [29:0] mul_out;  // Multiplier output P

  assign mul_in   = (wave_type == `SINE_WAVE) ? sine_wave : ((wave_type == `TRIANGLE_WAVE) ? triangle_wave : 14'b0);
  assign wave_out = wave_type[1] ? square_wave : mul_out[29:16];  //LSB-truncated
  // Sine wave DDS
  sin_dds #(
      .OUTPUT_WIDTH(14),  // Output width for sine wave
      .PHASE_WIDTH (32)   // Phase width for sine wave
  ) u_sin_dds (
      .clock   (clock),
      .rstn    (rstn),
      .en      (en),
      .fre_word(sin_fre_word),
      .pha_word(pha_word),
      .wave_sin(sine_wave)
  );

  // Triangle wave DDS
  triangle_dds u_triangle_dds (
      .incre_word(tri_incre_word),
      .decre_word(tri_decre_word),
      .pha_word  (pha_word),
      .clk       (clock),
      .rstn      (rstn),
      .en        (en),
      .data      (triangle_wave)
  );

  // Square wave DDS
  square_dds u_square_dds (
      .cnt_word(sqr_cnt_word),
      .cmp_word(sqr_cmp_word),
      .pha_word(pha_word),
      .amp_word(amp_word[13:0]),  // Use lower 14 bits for amplitude
      .clk     (clock),
      .rstn    (rstn),
      .en      (en),
      .data    (square_wave)
  );


  MULT_MACRO #(
      .DEVICE ("7SERIES"),  // Target Device: "7SERIES" 
      .LATENCY(3),          // Desired clock cycle latency, 0-4
      .WIDTH_A(16),         // Multiplier A-input bus width, 1-25
      .WIDTH_B(14)          // Multiplier B-input bus width, 1-18
  ) MULT_MACRO_inst1 (
      .P  (mul_out),   // Multiplier output bus, width determined by WIDTH_P parameter
      .A  (amp_word),  // Multiplier input A bus, width determined by WIDTH_A parameter
      .B  (mul_in),    // Multiplier input B bus, width determined by WIDTH_B parameter
      .CE (1'b1),      // 1-bit active high input clock enable
      .CLK(clock),     // 1-bit positive edge clock input
      .RST(~rstn)      // 1-bit input active high reset
  );

endmodule  //dds
