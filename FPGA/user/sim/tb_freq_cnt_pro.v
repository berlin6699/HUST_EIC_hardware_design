`timescale 1ns / 1ps

module tb_freq_cnt_pro;
  // 参数定义
  parameter WD = 14;  // 数据位宽
  parameter CLK_PERIOD = 100;  // 100ns时钟周期 (10MHz)
  parameter SIN_PERIOD = 1e6;  // 正弦波周期100us (10kHz)
  parameter SIN_CYCLES = 50;  // 正弦波周期数
  parameter SIM_TIME = SIN_CYCLES * SIN_PERIOD / CLK_PERIOD;  // 正弦波点数
  parameter NOISE_AMPLITUDE = 1;  // 噪声幅度
  // parameter THRESHOLD = 2000;  // 施密特触发器阈值


  // 信号声明
  reg                 clk;
  reg                 rstn;
  reg signed [WD-1:0] input_signal;

  // 内部变量
  real                sin_value;
  reg signed [WD-1:0] noise;
  reg signed [WD-1:0] clean_sine;
  integer             i;

  // 实例化被测单元
  wire       [  31:0] cnt;

  freq_cnt_pro #(
      .WD(14)
  ) u_freq_cnt_pro (
      .clk      (clk),
      .rstn     (rstn),
      .signal_in(input_signal),
      .threshold(1000),
      .times    (4),
      .cnt_out  (cnt)
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
      sin_value    = $sin(2.0 * 3.14159 * i * CLK_PERIOD / SIN_PERIOD) * 2000;
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
    #(CLK_PERIOD * 10);
    #(SIM_TIME * SIN_PERIOD);
    // $display("SIN_TIME=%d", SIN_PERIOD);
    $display("Simulation completed");
    $finish;
  end

  // 波形输出
  initial begin
    $dumpfile("./user/sim/tb_freq_cnt_pro.vcd");
    $dumpvars(0, tb_freq_cnt_pro);
  end

  // 结果监控
  always @(posedge clk) begin
    if (rstn) begin
      $display("Time: %t, Input: %d, cnt: %d, Clean Sine: %d", $time, input_signal, cnt, clean_sine);
    end
  end

endmodule
