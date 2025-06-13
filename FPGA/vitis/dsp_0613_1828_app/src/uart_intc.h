#ifndef _UART_INTC_H_
#define _UART_INTC_H_
#include "stdint.h"
#include "xuartps.h"
#include "xparameters.h"
#include "xinterrupt_wrap.h"


#define UART_INT_IRQ_ID XPAR_XUARTPS_1_INTR
#define UART_BASEADDR XPAR_UART1_BASEADDR
#define UART_INT_FUNC UartIntr_Handler

extern XUartPs Uart_Inst;
int uart_init();
void UartIntr_Handler(void *call_back_ref);

#endif
