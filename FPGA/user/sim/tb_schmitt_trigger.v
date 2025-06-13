`timescale 1ns / 1ps

module tb_schmitt_trigger;
  // 参数定义
  parameter WD = 14;  // 数据位宽
  parameter CLK_PERIOD = 10;  // 10ns时钟周期 (100MHz)
  parameter SIN_PERIOD = 100;  // 正弦波周期1μs
  parameter SIM_TIME = 10000;  // 仿真时间10μs
  parameter NOISE_AMPLITUDE = 500;  // 噪声幅度
  // parameter THRESHOLD = 2000;  // 施密特触发器阈值


  // 信号声明
  reg                 clk;
  reg                 rstn;
  reg signed [WD-1:0] input_signal;
  wire                output_signal;

  // 内部变量
  real                sin_value;
  reg signed [WD-1:0] noise;
  reg signed [WD-1:0] clean_sine;
  integer             i;

  // 实例化被测单元
  schmitt_trigger #(
      .WD(WD)
  ) dut (
      .clk      (clk),
      .rstn     (rstn),
      .D        (input_signal),
      .threshold(14'd2000),
      .Q        (output_signal)
  );

  // 生成时钟
  initial begin
    clk = 0;
    forever #(CLK_PERIOD / 2) clk = ~clk;
  end

  // 生成复位信号
  initial begin
    rstn = 0;
    #(CLK_PERIOD * 5) rstn = 1;  // 5个时钟周期后释放复位
  end

  // 生成正弦波加噪声输入
  initial begin
    input_signal = 0;

    // 等待复位完成
    #(CLK_PERIOD * 10);

    // 生成10个周期的正弦波
    for (i = 0; i < SIM_TIME; i = i + 1) begin
      // 计算正弦值 (振幅约为32767/4 = 8191/4)
      sin_value    = $sin(2.0 * 3.14159 * i / SIN_PERIOD) * 2000;
      clean_sine   = $rtoi(sin_value);

      // 生成随机噪声
      noise        = ($random % (2 * NOISE_AMPLITUDE)) - NOISE_AMPLITUDE;

      // 将正弦波和噪声相加
      input_signal = clean_sine + noise;

      // 确保信号不超出范围
      if (input_signal > 2 ** (WD - 1) - 1) input_signal = 2 ** (WD - 1) - 1;
      else if (input_signal < -(2 ** (WD - 1))) input_signal = -(2 ** (WD - 1));

      #CLK_PERIOD;
    end
  end

  // 结束仿真
  initial begin
    #(SIM_TIME * CLK_PERIOD);
    $display("Simulation completed");
    $finish;
  end

  // 波形输出
  initial begin
    $dumpfile("./user/sim/tb_schmitt_trigger.vcd");
    $dumpvars(0, tb_schmitt_trigger);
  end

  // 结果监控
  always @(posedge clk) begin
    if (rstn) begin
      $display("Time: %t, Input: %d, Output: %d, Clean Sine: %d", $time, input_signal, output_signal, clean_sine);
    end
  end

endmodule
