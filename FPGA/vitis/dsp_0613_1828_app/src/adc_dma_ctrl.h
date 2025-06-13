#ifndef _ADC_DMA_CTRL_H_
#define _ADC_DMA_CTRL_H_
#include <xaxidma.h>
#include <xparameters.h>

#include "xil_cache.h"
#include "xinterrupt_wrap.h"
#include "xscugic.h"

#define ADC_SAMPLE_RATE 10e6  // 10MHz
#define FFT_LENGTH 8192       // FFT length

#define RX_BUFFER_BASE (0x01000000+0x01000000)

#define LTC2220_BASE XPAR_ADC_FFT_DMA_LTC2220_SAMPLE_0_BASEADDR
#define LTC2220_START 0x0
#define LTC2220_LENGTH 0x4
#define LTC2220_SAMPLE_mReadReg(BaseAddress, RegOffset) \
  Xil_In32((BaseAddress) + (RegOffset))
#define LTC2220_SAMPLE_mWriteReg(BaseAddress, RegOffset, Data) \
  Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

extern XAxiDma AxiDma;
extern volatile int dma_rx_done;
// extern u32 DmaRxBuffer[16384];

void ltc2220_sample(u32 adc_addr, u32 length);

int XAxiDma_Intc_Initial(UINTPTR Baseaddr, XAxiDma *AxiDma);
static void RxIntrHandler(void *Callback);
#endif  // _ADC_DMA_CTRL_H_
