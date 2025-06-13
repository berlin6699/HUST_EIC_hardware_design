#include <stdio.h>
#include <xaxidma.h>

#include "adc_dma_ctrl.h"
#include "common_arg.h"
#include "dds.h"
#include "fft_handle.h"
#include "freq_meas.h"
#include "math.h"
#include "sleep.h"
#include "uart_intc.h"
#include "usart_hmi.h"
#include "wave_set.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xuartps.h"
/*
GPIO1的0通道对应两个SPI的SCK，1通道对应两个SPI的MOSI，2、3通道分别对应两个SPI的SS
从仿真波形来看方波的峰值在1275ns，锯齿波的峰值在1275+3*10=1305ns，正弦波的峰值在1345ns，分别延后了3个T和4个T
*/


int main(void) {
  // 设置GPIO方向为输出
  Xil_Out8(GPIO1_SET_ADDR, 0x00); 


  uart_init();
  ch_data_init();

  // 使能微处理器中断

  // Initialize the ADC and DMA controller
  int Status;
  Status = XAxiDma_Intc_Initial(XPAR_ADC_FFT_DMA_AXI_DMA_0_BASEADDR, &AxiDma);
  if (Status != XST_SUCCESS) {
    xil_printf("[ERROR] DMA initialization failed\r\n");
    return Status;
  }

  printf("Test\t\n");
  xil_printf("[OK] DMA initialization finished\r\n");
  usleep(100000);

  Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)RX_BUFFER_BASE,
                                  FFT_LENGTH * 2 * sizeof(u32),
                                  XAXIDMA_DEVICE_TO_DMA);
  ltc2220_sample(LTC2220_BASE, FFT_LENGTH);

  while (1) {
    // 串口屏控制部分
    TJC_USART();

    if (dma_rx_done) {
      dma_rx_done = 0;
      Xil_DCacheInvalidateRange((UINTPTR)RX_BUFFER_BASE,
                                FFT_LENGTH * 2 * sizeof(u32));

      XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)RX_BUFFER_BASE,
                             FFT_LENGTH * 2 * sizeof(u32),
                             XAXIDMA_DEVICE_TO_DMA);
      ltc2220_sample(LTC2220_BASE, FFT_LENGTH);
    }
    usleep(400000);
  }
  return 0;
}
