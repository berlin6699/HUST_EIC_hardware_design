#ifndef _WAVE_SET_H_
#define _WAVE_SET_H_

#include <stdio.h>
#include <xaxidma.h>

#include "adc_dma_ctrl.h"
#include "common_arg.h"
#include "dds.h"
#include "fft_handle.h"
#include "freq_meas.h"
#include "math.h"
#include "sleep.h"
#include "usart_hmi.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xuartps.h"

/*
bit1是通道一控制，bit3是通道二控制，1为1x，0为10x
*/
// 相关参数
#define CLOCK_FREQ 1.0e8f
#define DAC_AMP 3.26

// 相关地址
#define GPIO_DEVICE_ID XPAR_XGPIOPS_0_DEVICE_ID
#define GPIO1_SET_ADDR XPAR_XGPIO_0_BASEADDR + XGPIO_TRI_OFFSET
#define GPIO1_DATA_ADDR XPAR_XGPIO_0_BASEADDR + XGPIO_DATA_OFFSET
#define DDS_BASEADDR XPAR_USER_DUAL_DDS_0_BASEADDR

#define AMP_CONTROL1 XPAR_XGPIO_3_BASEADDR
#define AMP_CTRL_CH1 0
#define AMP_CTRL_CH2 2
#define AMP_10X 0
#define AMP_1X 1

// DDS模块各寄存器偏移
#define CH1_WAVE 0
#define CH1_AMP 4
#define CH1_PHA 8
#define CH1_SIN_FRE 12
#define CH1_TRI_INC 16
#define CH1_TRI_DEC 20
#define CH1_SQU_CNT 24
#define CH1_SQU_CMP 28

#define CH2_WAVE 32
#define CH2_AMP 36
#define CH2_PHA 40
#define CH2_SIN_FRE 44
#define CH2_TRI_INC 48
#define CH2_TRI_DEC 52
#define CH2_SQU_CNT 56
#define CH2_SQU_CMP 60
#define CH_RST_EN 64

#define SPI_SCK 0
#define SPI_MOSI 1
#define SPI_SS1 2
#define SPI_SS2 3

#define FFT_LENGTH 8192  // FFT长度
#define HEIGHT 255       // 波形高度
#define WIDTH 512
#define LOW_PIX 0   // 波形低点
#define LEFT_PIX 0  // 波形左边界

#define USER_DUAL_DDS_mReadReg(BaseAddress, RegOffset) \
  Xil_In32((BaseAddress) + (RegOffset))
#define USER_DUAL_DDS_mWriteReg(BaseAddress, RegOffset, Data) \
  Xil_Out32((BaseAddress) + (RegOffset), (Data))

// 比例系数
#define OFF_CONST1_NEG -7.3
#define OFF_CONST1_POS -5.5

#define SIN_AMP1_1X 1.02
#define SIN_AMP1_10X 10.3

#define SAW_AMP1_1X 1.02
#define SAW_AMP1_10X 10.3

#define SQU_AMP1_1X 1.02
#define SQU_AMP1_10X 10.3


// FFT计算结果结构体
typedef struct Wave_data {
  short value;  // 幅度值
  short index;  // 频率值
} Wave_data;    // 波形数据

extern Wave_data wave_data[FFT_LENGTH / 2];

extern char wave_type, channel;
extern float frequency, duty, amplitude, offset, phase;

void set_wave_type(channel_data data);

void set_offset(channel_data data);

void set_sin_freq(channel_data data);
void set_sin_amp(channel_data data);
void set_sin_pha(channel_data data);

void set_saw_freq_and_duty(channel_data data);
void set_saw_amp(channel_data data);
void set_saw_pha(channel_data data);

void set_squ_freq(channel_data data);
void set_squ_amp(channel_data data);
void set_squ_duty(channel_data data);
void set_squ_pha(channel_data data);

void Enable_output(channel_data data);
void Disable_output(channel_data data);
void Reset();

void SPI_Init();
void SPI_comm(int Data, char Ch);
void writeBit(char pin, int value);
void writeBit_ctrl(char pin, int value);

void fft_preproc(void);
// void fft_transmit(void);

#endif
