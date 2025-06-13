#include "wave_set.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>

Wave_data wave_data[FFT_LENGTH / 2];

// 模拟SPI通信函数，时钟周期2us，0.5Mbps的传输速率，高位优先，SCK初始高，SS初始高
void SPI_Init() {
  writeBit(SPI_SS1, 1);
  writeBit(SPI_SS2, 1);
  writeBit(SPI_SCK, 0);
  writeBit(SPI_MOSI, 0);
}
void SPI_comm(int Data, char Ch) {
  char i;
  // 片选
  if (Ch == 0)
    writeBit(SPI_SS1, 0);  // SPI1
  else
    writeBit(SPI_SS2, 0);  // SPI2

  usleep(1);

  // power down
  for (i = 0; i < 8; i++) {
    writeBit(SPI_SCK, 1);   // 拉高SCK
    writeBit(SPI_MOSI, 0);  // MOSI传输(第13bit)
    usleep(1);
    writeBit(SPI_SCK, 0);  // 拉低SCK
    usleep(1);
  }

  for (i = 0; i < 16; i++) {
    writeBit(SPI_SCK, 1);                         // 拉高SCK
    writeBit(SPI_MOSI, (Data & 0x8000) ? 1 : 0);  // MOSI传输(第13bit)
    usleep(1);
    writeBit(SPI_SCK, 0);  // 拉低SCK
    usleep(1);

    Data <<= 1;  // 左移Data
  }

  writeBit(SPI_SCK, 1);  // 重新拉高SCK
  usleep(1);

  // 取消片选
  if (Ch == 0)
    writeBit(SPI_SS1, 1);  // SPI1
  else
    writeBit(SPI_SS2, 1);  // SPI2
}
void writeBit(char pin, int value) {
  uint8_t reg = Xil_In8(GPIO1_DATA_ADDR);
  if (value)
    reg |= (1 << pin);  // 对应位置一
  else
    reg &= ~(1 << pin);  // 对应位置零
  Xil_Out8(GPIO1_DATA_ADDR, reg);
}
void writeBit_ctrl(char pin, int value) {
  uint8_t reg = Xil_In8(AMP_CONTROL1);
  if (value)
    reg |= (1 << pin);  // 对应位置一
  else
    reg &= ~(1 << pin);  // 对应位置零
  Xil_Out8(AMP_CONTROL1, reg);
}

// 设置波形
// 0-正弦波，1-锯齿波，2-方波
void set_wave_type(channel_data data) {
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_WAVE, data.signal_type);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_WAVE, data.signal_type);
}

// 设置偏置
void set_offset(channel_data data) {
  short off_word;
  if (data.offset >= 0)
    off_word = (short)((data.offset + DAC_AMP * OFF_CONST1_POS) / 2 /
                       OFF_CONST1_POS / DAC_AMP * (pow(2, 16)));
  else
    off_word = (short)((data.offset + DAC_AMP * OFF_CONST1_NEG) / 2 /
                       OFF_CONST1_NEG / DAC_AMP * (pow(2, 16)));
  SPI_Init();
  SPI_comm(off_word, data.channel);
}

// 设置正弦波频率
void set_sin_freq(channel_data data) {
  unsigned freq_word;
  freq_word = (unsigned)(pow(2, 32) * data.frequency / CLOCK_FREQ);
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_SIN_FRE, freq_word);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_SIN_FRE, freq_word);
}
// 设置正弦波幅度
void set_sin_amp(channel_data data) {
  short amp_word;
  char channel_sel;
  int amp_times;

  if(data.amplitude <= DAC_AMP / 2)
  {
    amp_times = AMP_1X;
    amp_word = (short)((pow(2, 16)-2) * data.amplitude / DAC_AMP / SIN_AMP1_1X);
  }
  else {
    amp_times = AMP_10X;
    amp_word = (short)((pow(2, 16)-2) * data.amplitude / DAC_AMP / SIN_AMP1_10X);
  }
  
  if (data.channel == 0){
    channel_sel = AMP_CTRL_CH1;
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_AMP, amp_word);      
  }
    
  else {
    channel_sel = AMP_CTRL_CH2;
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_AMP, amp_word);
  }

  writeBit_ctrl(channel_sel, amp_times);
}
// 设置正弦波相位
void set_sin_pha(channel_data data) {
  unsigned pha_word;
  if(data.phase <= 180)
    pha_word = (unsigned)(pow(2, 32) * (data.phase + 180) / 360);
  else
    pha_word = (unsigned)(pow(2, 32) * (data.phase - 180) / 360);

  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_PHA, pha_word);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_PHA, pha_word);
}

// 设置锯齿波频率和占空比
void set_saw_freq_and_duty(channel_data data) {
  // 定义三个无符号整数变量，分别表示递增字、递减字和相位字
  unsigned inc_word, dec_word;
  inc_word =
      (unsigned)(pow(2, 31) / CLOCK_FREQ * data.frequency / (1-data.duty/100));
  dec_word = (unsigned)(pow(2, 31) / CLOCK_FREQ * data.frequency /
                        (data.duty / 100));
  if (data.channel == 0) {
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_TRI_INC, inc_word);
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_TRI_DEC, dec_word);
  } else {
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_TRI_INC, inc_word);
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_TRI_DEC, dec_word);
  }
}
// 设置锯齿波幅度
void set_saw_amp(channel_data data) {
  short amp_word;
  int amp_times;
  char channel_sel;

  if(data.amplitude <= DAC_AMP / 2)
  {
    amp_times = AMP_1X;
    amp_word = (short)((pow(2, 16)-2) * data.amplitude / DAC_AMP / SAW_AMP1_1X);
  }
  else {
    amp_times = AMP_10X;
    amp_word = (short)((pow(2, 16)-2) * data.amplitude / DAC_AMP / SAW_AMP1_10X);
  }

  if (data.channel == 0){
    channel_sel = AMP_CTRL_CH1;
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_AMP, amp_word);      
  }
  else {
    channel_sel = AMP_CTRL_CH2;
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_AMP, amp_word);
  }

  writeBit_ctrl(channel_sel, amp_times);
}
// 设置锯齿波相位
void set_saw_pha(channel_data data) {
  unsigned pha_word;
  if(data.phase <= 180)
    pha_word = (unsigned)(pow(2, 32) * (data.phase + 180) / 360);
  else
    pha_word = (unsigned)(pow(2, 32) * (data.phase - 180) / 360);

  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_PHA, pha_word);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_PHA, pha_word);
}

// 设置方波频率
void set_squ_freq(channel_data data) {
  unsigned cnt_word;
//   cnt_word = (unsigned)(CLOCK_FREQ / data.frequency) - 1;
    cnt_word =(uint32_t)((pow(2, 32)-1)/CLOCK_FREQ*data.frequency);
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_SQU_CNT, cnt_word);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_SQU_CNT, cnt_word);
}
// 设置方波幅度
void set_squ_amp(channel_data data) {
  unsigned amp_word;
  int amp_times;
  char channel_sel;

  if(data.amplitude <= DAC_AMP / 2) {
      amp_times = AMP_1X;
      amp_word = (unsigned)(data.amplitude / DAC_AMP / SQU_AMP1_1X * (pow(2, 14) - 2));
  }
  else {
      amp_times = AMP_10X;
      amp_word = (unsigned)(data.amplitude / DAC_AMP / SQU_AMP1_10X * (pow(2, 14) - 2));
  }

  if (data.channel == 0) {
    channel_sel = AMP_CTRL_CH1;
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_AMP, amp_word);   
  }
  else {
    channel_sel = AMP_CTRL_CH2;
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_AMP, amp_word);
  }

  writeBit_ctrl(channel_sel, amp_times);
}
// 设置方波占空比
void set_squ_duty(channel_data data) {
  unsigned cmp_word;
  cmp_word = (unsigned)((pow(2,32)-1) * (data.duty / 100));
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_SQU_CMP, cmp_word);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_SQU_CMP, cmp_word);
}
// 设置方波相位
void set_squ_pha(channel_data data) {
  unsigned pha_word;
  pha_word = (unsigned)(pow(2, 32) * data.phase / 360);
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH1_PHA, pha_word);
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH2_PHA, pha_word);
}

// 开启输出
void Enable_output(channel_data data) {
  Reset();
  char origin_state = USER_DUAL_DDS_mReadReg(DDS_BASEADDR, CH_RST_EN);
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN,
                            origin_state | 0x4);  // reg16的bit2为1
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN,
                            origin_state | 0x8);  // reg16的bit3为1
}
// 关闭输出
void Disable_output(channel_data data) {
  char origin_state = USER_DUAL_DDS_mReadReg(DDS_BASEADDR, CH_RST_EN);
  if (data.channel == 0)
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN,
                            origin_state & 0xb);  // reg16的bit2为0
  else
    USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN,
                            origin_state & 0x7);  // reg16的bit3为0
}

void Reset() {
  char origin_state = USER_DUAL_DDS_mReadReg(DDS_BASEADDR, CH_RST_EN);
  USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN,
                          origin_state & 0xC);  // 一次性重置双通道，使输出同步
  USER_DUAL_DDS_mWriteReg(DDS_BASEADDR, CH_RST_EN, origin_state | 0x3);
}

void fft_preproc() {
  int i;
  // 假设DMA得到的数组为 origin_fft_data
  // 得到幅度谱
  fft_cal_amp((u32 *)RX_BUFFER_BASE, FFT_LENGTH);

  // 找到最大最小值方便归一化
  double max, min;
  max = 0;
  min = 1e5;  // 初始化为极大值
  for (i = 0; i < FFT_LENGTH / 2; i++) {
    if (fft_data_amplitude[i].value > max) max = fft_data_amplitude[i].value;
    if (fft_data_amplitude[i].value < min) min = fft_data_amplitude[i].value;
  }

  double range = max - min;
  if (range < 1e-6) {
    range = 1.0;
  }

  // 归一化后计算坐标值
  for (i = 0; i < FFT_LENGTH / 2; i++) {
    wave_data[i].value =
        (short)((fft_data_amplitude[i].value - min) / range * HEIGHT + LOW_PIX);
    // wave_data[i].index = (short)(i * 2 / FFT_LENGTH * WIDTH + LEFT_PIX);
  }
}

// void fft_transmit() {
//   uint8_t end = 0xff;
//   int i;
//   for (i = 0; i < FFT_LENGTH / 2; i++) {
//     XUartPs_SendByte(XPAR_UART1_BASEADDR, wave_data[i].value);  // 传输幅度值
//     // XUartPs_SendByte(XPAR_UART1_BASEADDR, wave_data[i].index);  // 传输频率值
//   }

//   XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
//   XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
//   XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
// }
