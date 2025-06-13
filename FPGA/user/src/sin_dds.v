`timescale 1 ns / 1 ns

module sin_dds #(
    // Output signal width.
    parameter OUTPUT_WIDTH = 12,
    // Phase width.
    parameter PHASE_WIDTH  = 32
) (
    // Clock input
    input                     clock,
    // Asynchronous rstn with active high
    input                     rstn,
    // Enable input
    input                     en,
    // Frequency control word.
    // (out clk frequency * 2^PHASE_WIDTH)/clock frequency
    input [PHASE_WIDTH-1 : 0] fre_word,

    // Phase control word.
    // (out phase * 2^PHASE_WIDTH) / (clock frequency * 360°)
    input [PHASE_WIDTH-1 : 0] pha_word,

    // Out Sine wave.
    output signed [OUTPUT_WIDTH-1 : 0] wave_sin
);

  wire [PHASE_WIDTH-1:0] Q;

  accumlator #(
      .WIDTH(PHASE_WIDTH)
  ) u_accumlator (
      //ports
      .clock  (clock),
      .rstn   (rstn),
      .clr    (1'b0),
      .add_sub(1'b0),
      .D      (fre_word),
      .Q      (Q)
  );

  reg [PHASE_WIDTH-1:0] phase;
  always @(posedge clock or negedge rstn) begin
    if (~rstn) begin
      phase <= 0;
    end
    else begin
      phase <= Q + pha_word;
    end
  end

  reg [9:0] addr;
  always @(posedge clock or negedge rstn) begin
    if (~rstn) begin
      addr <= 0;
    end
    else begin
      addr <= phase[PHASE_WIDTH-1:PHASE_WIDTH-10];
    end
  end

  wire [15:0] douta;
  sin_rom u_sin_rom (
      //ports
      .clka (clock),
      .rstn (rstn),
      .addra(addr),
      .douta(douta)
  );

  assign wave_sin = en ? douta >>> (16 - OUTPUT_WIDTH) : 'b0;

endmodule  //sin_dds
