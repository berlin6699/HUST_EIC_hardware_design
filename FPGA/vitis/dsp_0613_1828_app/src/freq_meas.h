#ifndef __FREQ_MEAS_H__
#define __FREQ_MEAS_H__
#include "xgpio.h"
#include "xparameters.h"
#include "xil_types.h"
#include <stdio.h>
#define FREQ_MEAS_CTL_ADDR XPAR_FREQ_CNT_AXI_GPIO_0_BASEADDR
#define FREQ_MEAS_CNT_ADDR XPAR_FREQ_CNT_AXI_GPIO_1_BASEADDR

#define WRITE_TIMES(times) Xil_Out32(FREQ_MEAS_CTL_ADDR + 0x0, times)
#define WRITE_THRESHOLD(threshold) Xil_Out32(FREQ_MEAS_CTL_ADDR + 0x8, threshold)

#define READ_CNT() Xil_In32(FREQ_MEAS_CNT_ADDR + 0x0)

double freq_meas(void);
#endif  // __FREQ_MEAS_H__
