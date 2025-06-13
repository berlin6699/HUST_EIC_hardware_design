#include "uart_intc.h"

XUartPs Uart_Inst;
// XScuGic ScuGic_Inst;

// uart初始化
int uart_init()
{
	XUartPs_Config *UartPs_Cfg;
	int Status;
	unsigned int IntrMask;

	// 查找配置信息
	UartPs_Cfg = XUartPs_LookupConfig(UART_BASEADDR);
	if(!UartPs_Cfg)
	{
		xil_printf("[ERROR] No config found for %d\r\n", UART_BASEADDR);
		return XST_FAILURE;
	}

	// 对uart控制器进行初始化
	Status = XUartPs_CfgInitialize(&Uart_Inst, UartPs_Cfg, UartPs_Cfg->BaseAddress);
	if(Status != XST_SUCCESS)
	{
		xil_printf("[ERROR] Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	// 检测硬件搭建是否正确
	Status = XUartPs_SelfTest(&Uart_Inst);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

    XUartPs_WriteReg(UART_BASEADDR, XUARTPS_ISR_OFFSET,
						 XUARTPS_IXR_RXOVR); // 清除中断状态

	// 设置中断系统
	Status = XSetupInterruptSystem(&Uart_Inst, &XUartPs_InterruptHandler,
                                 UartPs_Cfg->IntrId, UartPs_Cfg->IntrParent,
                                 XINTERRUPT_DEFAULT_PRIORITY);
	
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	
	// 设置波特率
	XUartPs_SetBaudRate(&Uart_Inst, 115200);
	// 设置RXFIFO触发阈值
	XUartPs_SetFifoThreshold(&Uart_Inst, 1);

	// 设置中断处理函数
	XUartPs_SetHandler(&Uart_Inst, (XUartPs_Handler)UartIntr_Handler, &Uart_Inst);

	// 中断掩码
	IntrMask = XUARTPS_IXR_RXOVR;

	if (Uart_Inst.Platform == XPLAT_ZYNQ_ULTRA_MP) {
    IntrMask |= XUARTPS_IXR_RBRK;
  }

	// 设置中断掩码
	XUartPs_SetInterruptMask(&Uart_Inst, IntrMask);

	// 设置操作模式
	XUartPs_SetOperMode(&Uart_Inst, XUARTPS_OPER_MODE_NORMAL);

	return XST_SUCCESS;
}

// 中断处理函数
void UartIntr_Handler(void *call_back_ref)
{
	XUartPs *uartinst = (XUartPs *)call_back_ref;
	u8 read_data = 0;
	u32 intr_status;
	// 读取中断ID寄存器
	intr_status = XUartPs_ReadReg(uartinst->Config.BaseAddress,
								  XUARTPS_IMR_OFFSET); // 读取掩码
	intr_status &= XUartPs_ReadReg(uartinst->Config.BaseAddress,
								   XUARTPS_ISR_OFFSET); // 读取状态
	if (intr_status & (u32)XUARTPS_IXR_RXOVR)
	{
		read_data = XUartPs_RecvByte(UART_BASEADDR); // 接收发送的字节
		XUartPs_WriteReg(uartinst->Config.BaseAddress, XUARTPS_ISR_OFFSET,
						 XUARTPS_IXR_RXOVR); // 清除中断状态
	}
	// 将数据写入环形缓冲区
	writeRingBuff(read_data);
}
