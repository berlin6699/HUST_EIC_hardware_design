#include "adc_dma_ctrl.h"
// s2mm interrupt flag
volatile int dma_rx_done = 0;

// DMA instace
XAxiDma AxiDma;

// DMA buffer for receiving data
// aligned to 4 bytes for DMA transfer
// u32 DmaRxBuffer[16384] __attribute__((aligned(8)));

void ltc2220_sample(u32 adc_addr, u32 length) {
  // xil_printf("start sample\r\n");
  LTC2220_SAMPLE_mWriteReg(adc_addr, LTC2220_LENGTH, length);
  LTC2220_SAMPLE_mWriteReg(adc_addr, LTC2220_START, 1);
}

/*
 *Initial DMA and connect interrupt to handler, open s2mm interrupt
 *
 *@param Baseaddr    DMA base address
 *@param XAxiDma     DMA pointer
 *@param InstancePtr GIC pointer
 *
 *@note  none
 */
int XAxiDma_Intc_Initial(UINTPTR Baseaddr, XAxiDma *AxiDma) {
  XAxiDma_Config *CfgPtr;
  int Status;
  /* Initialize the XAxiDma device. */
  CfgPtr = XAxiDma_LookupConfig(Baseaddr);
  if (!CfgPtr) {
    xil_printf("[ERROR] No config found for %d\r\n", Baseaddr);
    return XST_FAILURE;
  }

  Status = XAxiDma_CfgInitialize(AxiDma, CfgPtr);
  if (Status != XST_SUCCESS) {
    xil_printf("[ERROR] Initialization failed %d\r\n", Status);
    return XST_FAILURE;
  }

  Status =
      XSetupInterruptSystem(AxiDma, &RxIntrHandler, CfgPtr->IntrId[0],
                            CfgPtr->IntrParent, XINTERRUPT_DEFAULT_PRIORITY);

  if (Status != XST_SUCCESS) {
    return Status;
  }

  /* Disable MM2S interrupt, Enable S2MM interrupt */
  XAxiDma_IntrEnable(AxiDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
  XAxiDma_IntrDisable(AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

  return XST_SUCCESS;
}

static void RxIntrHandler(void *Callback) {
  u32 IrqStatus;
  XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

  /* Read pending interrupts */
  IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);

  /* Acknowledge pending interrupts */
  XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

  /*
   * If no interrupt is asserted, we do not do anything
   */
  if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
    return;
  }

  /*
   * If error interrupt is asserted, raise error flag, reset the
   * hardware to recover from the error, and return with no further
   * processing.
   */
  if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {
    /* Reset could fail and hang
     * NEED a way to handle this or do not call it??
     */
    xil_printf("[ERROR] DMA Interrupt Error\r\n");
    XAxiDma_Reset(AxiDmaInst);
    while (1) {
      if (XAxiDma_ResetIsDone(AxiDmaInst)) {
        break;
      }
    }
    return;
  }

  /*
   * If completion interrupt is asserted, then set RxDone flag
   */
  if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {
    // xil_printf("[OK] DMA Interrupt OK\r\n");
    dma_rx_done = 1;
  }
}
