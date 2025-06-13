module amp_ctl_div (
    input  [3:0] gpio_in,
    output       c1k1,
    output       c1k2,
    output       c2k1,
    output       c2k2
);
  assign c1k1 = gpio_in[0];
  assign c1k2 = gpio_in[1];
  assign c2k1 = gpio_in[2];
  assign c2k2 = gpio_in[3];

endmodule  //amp_ctl_div
