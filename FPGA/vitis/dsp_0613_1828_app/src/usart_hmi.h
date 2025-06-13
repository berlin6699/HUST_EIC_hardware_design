#ifndef _USARTHMI_H_
#define _USARTHMI_H_

#include "stdint.h"
#include <stdarg.h>
#include "xparameters.h"
#include "uart_intc.h"
#include "stdio.h"
#include "wave_set.h"
#include "common_arg.h"


#define RINGBUFF_LEN (500) // 定义最大接收字节数 500

#define usize        getRingBuffLenght()
#define code_c()     initRingBuff()
#define udelete(x)   deleteRingBuff(x)
#define u(x)         read1BFromRingBuff(x)
#define STR_LENGTH 100

typedef struct
{
    uint16_t Head;
    uint16_t Tail;
    uint16_t Lenght;
    uint8_t Ring_data[RINGBUFF_LEN];
} RingBuff_t;

/**
    打印到屏幕串口
*/
void Init_HMI(void);
void TJCPrintf(const char *cmd, ...);
void UART_printf(const char *format, ...);
void initRingBuff(void);
void writeRingBuff(uint8_t data);
void deleteRingBuff(uint16_t size);
uint16_t getRingBuffLenght(void);
uint8_t read1BFromRingBuff(uint16_t position);

void HMI_Control(void);



float little_endian2real_number();// 将小端字节序转换为实数
float little_endian2real_number_signed();
void signal_type_judge(void);// 判断信号类型
void output_control(void);// 输出控制函数
void update_adc_output(void);// 更新ADC采集后输出的参数
void adc_output_control(void);// ADC采集输出控制函数
void adc_page_control(void);// ADC采集页面控制函数
void TJC_USART(void);
void ch_data_init(void);


// //生成两个通道数据结构体
// extern channel_data ch1_data, ch2_data;
//生成两个通道数据结构体


//ADC采集判断相关变量，将在ADC采集判断函数中被赋值
// 0:正弦波, 1:方波, 2:锯齿波, 3:三角波
// extern int adc_signal_type; // ADC采集信号类型
extern double adc_frequency; // ADC采集频率
extern int adc_multiplier; // ADC采集倍频系数
extern int adc_channel; // ADC采集输出通道


#endif
