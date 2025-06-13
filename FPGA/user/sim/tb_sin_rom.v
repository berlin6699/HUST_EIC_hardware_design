`timescale 1ns / 1ps
module tb_sin_rom ();
  reg       clk;
  reg       rstn;
  reg [9:0] addr;  // Address for the ROM
  always #5 clk = ~clk;  // 100 MHz clock


  initial begin
    clk  = 0;
    rstn = 0;
    #20 rstn = 1;  // Release reset after 20 ns 
  end


  initial begin
    addr = 10'd0;  // Initialize address to 0
    #30;  // Wait for 30 ns
    repeat (2048) begin
      #10;  // Wait for 10 ns
      addr = addr + 1;  // Increment address
    end
    $finish;  // End simulation after reading all addresses
  end

  initial begin
    $dumpfile("./user/sim/tb_sin_rom.vcd");
    $dumpvars(0, tb_sin_rom);
  end

  // output declaration of module Sin_ROM
  wire [15:0] douta;

  sin_rom u_sin_rom (
      .clka (clk),
      .rstn (rstn),
      .addra(addr),
      .douta(douta)
  );


endmodule  //tb_sin_rom
