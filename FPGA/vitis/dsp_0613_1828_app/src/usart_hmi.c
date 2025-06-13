/**
使用注意事项:
    1.将tjc_usart_hmi.c和tjc_usart_hmi.h 分别导入工程
    2.在需要使用的函数所在的头文件中添加 #include "tjc_usart_hmi.h"
    3.使用前请将 HAL_UART_Transmit_IT()
这个函数改为你的单片机的串口发送单字节函数 3.TJCPrintf和printf用法一样

*/

#include "usart_hmi.h"

#include <xil_printf.h>

#include "common_arg.h"

channel_data ch1_data, ch2_data, ch_times_data;

// ADC采集判断相关变量，将在ADC采集判断函数中被赋值
//  0:正弦波, 1:方波, 2:锯齿波, 3:三角波
//  int adc_signal_type = 0; // ADC采集信号类型
double adc_frequency = 0;  // ADC采集频率
int adc_multiplier = 1;    // ADC采集倍频系数
int adc_channel = 0;       // ADC采集输出通道
char flag = 0;

RingBuff_t ringBuff;  // 创建一个ringBuff的缓冲区

void TJCPrintf(const char *str, ...) {
  uint8_t end = 0xff;
  char buffer[STR_LENGTH + 1];  // 数据长度
  uint8_t i = 0;
  va_list arg_ptr;
  va_start(arg_ptr, str);
  vsnprintf(buffer, STR_LENGTH + 1, str, arg_ptr);
  va_end(arg_ptr);
  while ((i < STR_LENGTH) && (i < strlen(buffer))) {
    XUartPs_SendByte(XPAR_UART1_BASEADDR, buffer[i++]);
  }

  XUartPs_SendByte(XPAR_UART1_BASEADDR, end);

  XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
  XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
}

/********************************************************
函数名：  	HAL_UART_RxCpltCallback
作者：
日期：    	2022.10.08
功能：    	串口接收中断,将接收到的数据写入环形缓冲区
输入参数：
返回值： 		void
修改记录：
**********************************************************/
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {

//     if (huart->Instance == LCD_USART) // 判断是由哪个串口触发的中断
//     {
//         writeRingBuff(RxBuff[0]);
//         HAL_UART_Receive_IT(&LCD_huart, RxBuff, 1); // 重新使能串口2接收中断
//     }
// }

/********************************************************
函数名：  	initRingBuff
作者：
日期：    	2022.10.08
功能：    	初始化环形缓冲区
输入参数：
返回值： 		void
修改记录：
**********************************************************/
void initRingBuff(void) {
  // 初始化相关信息
  ringBuff.Head = 0;
  ringBuff.Tail = 0;
  ringBuff.Lenght = 0;
}

/********************************************************
函数名：  	writeRingBuff
作者：
日期：    	2022.10.08
功能：    	往环形缓冲区写入数据
输入参数：
返回值： 		void
修改记录：
**********************************************************/
void writeRingBuff(uint8_t data) {
  if (ringBuff.Lenght >= RINGBUFF_LEN)  // 判断缓冲区是否已满
  {
    return;
  }
  ringBuff.Ring_data[ringBuff.Tail] = data;
  ringBuff.Tail = (ringBuff.Tail + 1) % RINGBUFF_LEN;  // 防止越界非法访问
  ringBuff.Lenght++;
}

/********************************************************
函数名：  	deleteRingBuff
作者：
日期：    	2022.10.08
功能：    	删除串口缓冲区中相应长度的数据
输入参数：		要删除的长度
返回值： 		void
修改记录：
**********************************************************/
void deleteRingBuff(uint16_t size) {
  if (size >= ringBuff.Lenght) {
    initRingBuff();
    return;
  }
  for (int i = 0; i < size; i++) {
    if (ringBuff.Lenght == 0)  // 判断非空
    {
      initRingBuff();
      return;
    }
    ringBuff.Head = (ringBuff.Head + 1) % RINGBUFF_LEN;  // 防止越界非法访问
    ringBuff.Lenght--;
  }
}

/********************************************************
函数名：  	read1BFromRingBuff
作者：
日期：    	2022.10.08
功能：    	从串口缓冲区读取1字节数据
输入参数：		position:读取的位置
返回值： 		所在位置的数据(1字节)
修改记录：
**********************************************************/
uint8_t read1BFromRingBuff(uint16_t position) {
  uint16_t realPosition = (ringBuff.Head + position) % RINGBUFF_LEN;

  return ringBuff.Ring_data[realPosition];
}

/********************************************************
函数名：  	getRingBuffLenght
作者：
日期：    	2022.10.08
功能：    	获取串口缓冲区的数据数量
输入参数：
返回值： 		串口缓冲区的数据数量
修改记录：
**********************************************************/
uint16_t getRingBuffLenght() { return ringBuff.Lenght; }

/********************************************************
函数名：  	isRingBuffOverflow
作者：
日期：    	2022.10.08
功能：    	判断环形缓冲区是否已满
输入参数：
返回值： 		1:环形缓冲区已满 , 2:环形缓冲区未满
修改记录：
**********************************************************/
uint8_t isRingBuffOverflow() { return ringBuff.Lenght == RINGBUFF_LEN; }

void Init_HMI(void) {
  initRingBuff();  // 初始化环形缓冲区
  uart_init();
}

void UART_printf(const char *format, ...) {
  uint8_t end = 0xff;
  char buffer[256];  // 临时缓冲区，用于存储格式化后的字符串
  va_list args;

  // 初始化可变参数列表
  va_start(args, format);

  // 使用 vsprintf 将格式化后的字符串写入缓冲区
  vsprintf(buffer, format, args);

  // 结束可变参数的处理
  va_end(args);

  // 将缓冲区中的字符串逐字节发送
  for (char *ptr = buffer; *ptr != '\0'; ptr++) {
    XUartPs_SendByte(XPAR_UART1_BASEADDR, *ptr);
  }

  XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
  XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
  XUartPs_SendByte(XPAR_UART1_BASEADDR, end);
}

float little_endian2real_number() {
  uint32_t value = 0;

  value |= (uint32_t)u(2 + 0) << 0;   // byte0 - LSB
  value |= (uint32_t)u(2 + 1) << 8;   // byte1
  value |= (uint32_t)u(2 + 2) << 16;  // byte2
  value |= (uint32_t)u(2 + 3) << 24;  // byte3 - MSB

  float result = (float)value / 100.0f;

  udelete(8);  // 删除前8字节

  return result;
}

float little_endian2real_number_signed() {
  int value = 0;

  value |= (int)u(2 + 0) << 0;   // byte0 - LSB
  value |= (int)u(2 + 1) << 8;   // byte1
  value |= (int)u(2 + 2) << 16;  // byte2
  value |= (int)u(2 + 3) << 24;  // byte3 - MSB

  float result = (float)value / 100.0f;

  udelete(8);  // 删除前8字节

  return result;
}

void ch_data_init() {
  // 默认1kHz，2Vpp，0偏置占空比50相位0的正弦波
  ch1_data.channel = 0;
  ch1_data.signal_type = 0;
  ch1_data.frequency = 1000;
  ch1_data.amplitude = 1.0;
  ch1_data.offset = 0;
  ch1_data.duty = 0.5;
  ch1_data.phase = 0;

  ch2_data.channel = 1;
  ch2_data.signal_type = 0;
  ch2_data.frequency = 1000;
  ch2_data.amplitude = 1.0;
  ch2_data.offset = 0;
  ch2_data.duty = 0.5;
  ch2_data.phase = 0;

  // 倍频输出：默认正弦波，2Vpp，0偏置占空比50相位0
  ch_times_data.signal_type = 0;
  ch_times_data.channel = adc_channel;
  ch_times_data.amplitude = 1.0;
  ch_times_data.offset = 0;
  ch_times_data.duty = 0.5;
  ch_times_data.phase = 0;
}

void signal_type_judge(void) {
  char wave_type;
  // 如果是选择信号类型的指令
  if (usize >= 8 && u(0) == 0x55 && u(1) == 0xAA && u(6) == 0x0D &&
      u(7) == 0x0A) {
    // 先判断u(3)的值，以确定波形类型
    if (u(3) == 0xAD)  // 正弦波
    {
      wave_type = 0;
    } else if (u(3) == 0x94)  // 锯齿波
    {
      wave_type = 1;
    } else if (u(3) == 0x96)  // 方波
    {
      wave_type = 2;
    }
    // else if(u(3) == 0xB8) // 三角波
    // {
    //     wave_type = 3;
    // }

    ch1_data.signal_type = wave_type;
    printf("信号波形：%d\r\n", ch1_data.signal_type);
    udelete(8);
    set_wave_type(ch1_data);
  } else if (usize >= 8 && u(0) == 0x55 && u(1) == 0xAB && u(6) == 0x0D &&
             u(7) == 0x0A) {
    // 先判断u(3)的值，以确定波形类型
    if (u(3) == 0xAD)  // 正弦波
    {
      wave_type = 0;
    } else if (u(3) == 0x94)  // 锯齿波
    {
      wave_type = 1;
    } else if (u(3) == 0x96)  // 方波
    {
      wave_type = 2;
    }
    // else if(u(3) == 0xB8) // 三角波
    // {
    //     wave_type = 3;
    // }

    ch2_data.signal_type = wave_type;
    udelete(8);
    set_wave_type(ch2_data);  // 设置波形类型
  }
}

void output_control(void) {
  // 如果是channel1的输出控制指令
  if (usize >= 8 && u(0) == 0x55 && u(1) == 0xBA && u(6) == 0x0D &&
      u(7) == 0x0A) {
    udelete(8);
    printf("get channel1 output");
    ch1_data.frequency = little_endian2real_number();      // 读取频率
    ch1_data.amplitude = little_endian2real_number();      // 读取振幅
    ch1_data.offset = little_endian2real_number_signed();  // 读取偏置
    ch1_data.duty = little_endian2real_number();           // 读取占空比
    ch1_data.phase = little_endian2real_number();          // 读取相位

    printf("ch1_data.frequency:%f\r\n", ch1_data.frequency);
    printf("ch1_data.amplitude:%f\r\n", ch1_data.amplitude);
    printf("ch1_data.offset:%f\r\n", ch1_data.offset);
    printf("ch1_data.duty:%f\r\n", ch1_data.duty);
    printf("ch1_data.phase:%f\r\n", ch1_data.phase);

    set_wave_type(ch1_data);        // 设置波形类型
    set_offset(ch1_data);           // 设置偏置
    if (ch1_data.signal_type == 0)  // 正弦波
    {
      printf("get set sin");
      set_sin_freq(ch1_data);              // 设置频率
      set_sin_amp(ch1_data);               // 设置振幅
      set_sin_pha(ch1_data);               // 设置相位
    } else if (ch1_data.signal_type == 1)  // 锯齿波
    {
      set_saw_freq_and_duty(ch1_data);     // 设置频率和占空比
      set_saw_amp(ch1_data);               // 设置振幅
      set_saw_pha(ch1_data);               // 设置相位
    } else if (ch1_data.signal_type == 2)  // 方波
    {
      set_squ_freq(ch1_data);  // 设置频率
      set_squ_amp(ch1_data);   // 设置振幅
      set_squ_duty(ch1_data);  // 设置占空比
      set_squ_pha(ch1_data);   // 设置相位
    }
    // else if(ch1_data.signal_type == 3) // 三角波
    // {
    //     set_saw_freq_and_duty(ch1_data); // 设置频率和占空比
    //     set_saw_amp(ch1_data); // 设置振幅
    //     set_saw_pha(ch1_data); // 设置相位
    // }

    Enable_output(ch1_data);  // 开启输出
  }
  // 如果是channel2的输出控制指令
  else if (usize >= 8 && u(0) == 0x55 && u(1) == 0xBB && u(6) == 0x0D &&
           u(7) == 0x0A) {
    udelete(8);
    ch2_data.frequency = little_endian2real_number();      // 读取频率
    ch2_data.amplitude = little_endian2real_number();      // 读取振幅
    ch2_data.offset = little_endian2real_number_signed();  // 读取偏置
    ch2_data.duty = little_endian2real_number();           // 读取占空比
    ch2_data.phase = little_endian2real_number();          // 读取相位

    printf("ch2_data.frequency:%f\r\n", ch2_data.frequency);
    printf("ch2_data.amplitude:%f\r\n", ch2_data.amplitude);
    printf("ch2_data.offset:%f\r\n", ch2_data.offset);
    printf("ch2_data.duty:%f\r\n", ch2_data.duty);
    printf("ch2_data.phase:%f\r\n", ch2_data.phase);

    set_wave_type(ch2_data);        // 设置波形类型
    set_offset(ch2_data);           // 设置偏置
    if (ch2_data.signal_type == 0)  // 正弦波
    {
      set_sin_freq(ch2_data);              // 设置频率
      set_sin_amp(ch2_data);               // 设置振幅
      set_sin_pha(ch2_data);               // 设置相位
    } else if (ch2_data.signal_type == 1)  // 锯齿波
    {
      set_saw_freq_and_duty(ch2_data);     // 设置频率和占空比
      set_saw_amp(ch2_data);               // 设置振幅
      set_saw_pha(ch2_data);               // 设置相位
    } else if (ch2_data.signal_type == 2)  // 方波
    {
      set_squ_freq(ch2_data);  // 设置频率
      set_squ_amp(ch2_data);   // 设置振幅
      set_squ_duty(ch2_data);  // 设置占空比
      set_squ_pha(ch2_data);   // 设置相位
    }
    // else if(ch2_data.signal_type == 3) // 三角波
    // {
    //     set_saw_freq_and_duty(ch2_data); // 设置频率和占空比
    //     set_saw_amp(ch2_data); // 设置振幅
    //     set_saw_pha(ch2_data); // 设置相位
    // }

    Enable_output(ch2_data);  // 开启输出
  }

  // //如果是ADC采集后的倍频输出
  // if(usize >= 8 && u(0) == 0x55 && (u(1) == 0xDB || u(1) == 0xDC) && u(6) ==
  // 0x0D && u(7) == 0x0A)
  // {
  //     if(u(1) == 0xDB) // 采集通道1
  //     {
  //         adc_channel = 0; // 设置当前通道为1
  //     }
  //     else if(u(1) == 0xDC) // 采集通道2
  //     {
  //         adc_channel = 1; // 设置当前通道为2
  //     }

  //     udelete(8); // 删除前8字节
  // }
}

void update_adc_output(void) {
  // 一些判断逻辑，更新adc_signal_type, adc_frequency, adc_multiplier,
  // adc_channel等变量

  // 更新信号类型

  // 更新信号频率

  // 更新倍频数
  if (usize >= 8 && u(0) == 0x55 && u(1) == 0xDB && u(6) == 0x0D &&
      u(7) == 0x0A) {
    flag = 1;
    adc_multiplier = u(2) - '0';  // 倍频系数

    // 重新输出
    adc_output_control();
    printf("multiplier: %d\r\n", adc_multiplier);

    udelete(8);  // 删除前8字节
  }
  // 更新输出通道
  else if (usize >= 8 && u(0) == 0x55 && u(1) == 0xDC && u(6) == 0x0D &&
           u(7) == 0x0A) {
    flag = 1;
    adc_channel = u(5) - '1';  // 输出通道

    // 重新输出
    adc_output_control();
    printf("channel: %d\r\n", adc_channel);

    udelete(8);  // 删除前8字节
  }
}

void adc_output_control(void) {
  ch_times_data.channel = adc_channel;
  ch_times_data.amplitude = 1.0;
  ch_times_data.phase = 0;
  ch_times_data.offset = 0;
  ch_times_data.signal_type = 0;
  ch_times_data.frequency =
      adc_frequency * adc_multiplier;  // 计算倍频后实际频率

  printf("ch_times_data.frequency:%f\n", ch_times_data.frequency);
  printf("ch_times_data.channel:%d\n", ch_times_data.channel);
  // printf("ch_data.amplitude:%f\r\n",ch_times_data.amplitude);
  // printf("ch_data.offset:%f\r\n",ch_times_data.offset);
  // printf("ch_data.duty:%f\r\n",ch_times_data.duty);
  // printf("ch_data.phase:%f\r\n",ch_times_data.phase);

  set_wave_type(ch_times_data);  // 设置波形类型
  set_sin_freq(ch_times_data);   // 设置频率
  set_sin_amp(ch_times_data);    // 设置振幅
  set_sin_pha(ch_times_data);    // 设置相位
  set_offset(ch_times_data);     // 设置偏置

  // 选择通道
  Enable_output(ch_times_data);  // 开启输出
  flag = 0;
}

void adc_page_control(void) {
  // 点击“开始测量”
  if (usize >= 8 && u(0) == 0x55 && u(1) == 0xDA && u(6) == 0x0D &&
      u(7) == 0x0A) {
    // 进行FFT计算和绘图
    fft_preproc();  // 预处理FFT数据
    // fft_cal_amp((u32 *)RX_BUFFER_BASE, FFT_LENGTH);
    // fft_transmit(); // 传输FFT结果

    // 总共4096个点，均匀抽取其中512个点进行绘图，所以每隔8个画一个
    for (int i = FFT_LENGTH / 2 - 1; i >= 0; i -= 8) {
      int sum = 0;
      for (int j = i; j > i - 8; j--) {
        sum += wave_data[j].value;
      }
      sum = 4 * sum / 8;

      // 向曲线s0的通道0传输1个数据,add指令不支持跨页面
      //  printf("add s0.id,0,%d\xff\xff\xff",(int)(sum));
      TJCPrintf("add s0.id,0,%d", (int)(sum));
      // printf("result:%d\r\n",sum);
    }
    adc_frequency = freq_meas();
    int result = (int)round(adc_frequency);
    // printf("freq_mea result: %d\r\n", result);
    TJCPrintf("FFT.t4.txt=\"%d\"", result);
    /**************************************************************************/
    // 此处需要添加判断波形的函数update_adc_output()来更新adc_signal_type,
    // adc_frequency, adc_multiplier, adc_channel等变量

    adc_output_control();

    udelete(8);  // 删除前8字节
  }
  update_adc_output();
  // 然后打印波形类型和频率等信息到串口屏上
  //  printf("t3.txt=\"%d\"\xff\xff\xff",adc_signal_type);
  //  printf("t4.txt=\"%d\"\xff\xff\xff",adc_frequency);
  /**************************************************************************/

  // 按照adc采集相关变量进行波形输出，其值已经在上述判断波形函数中被正确赋值
  if (flag) adc_output_control();
}

// 串口屏控制逻辑
void TJC_USART(void) {
  while (usize >= 8) {
    // printf("usize>0\r\n");
    // 校验帧头帧尾是否匹配
    if (u(0) != 0x55 || u(6) != 0x0D || u(7) != 0x0A) {
      // 不匹配删除1字节
      udelete(1);
    } else {
      // 匹配，跳出循环
      break;
    }
  }

  // 判断信号类型
  signal_type_judge();
  // 输出控制
  output_control();
  // ADC采集页面控制
  adc_page_control();
}
