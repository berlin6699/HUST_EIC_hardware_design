
module spi_div (
    input  [3:0] gpio_i,
    output       sck1,
    output       sck2,
    output       mosi1,
    output       mosi2,
    output       ss1,
    output       ss2
);

  // MAP GPIO to SPI
  assign sck1  = gpio_i[0];
  assign sck2  = gpio_i[0];

  assign mosi1 = gpio_i[1];
  assign mosi2 = gpio_i[1];

  assign ss1   = gpio_i[2];
  assign ss2   = gpio_i[3];

endmodule  //spi_div
