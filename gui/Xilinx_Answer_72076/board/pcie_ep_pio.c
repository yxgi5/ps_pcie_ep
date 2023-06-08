/******************************************************************************
*
* (c) Copyright 2014 - 2015 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <unistd.h>
#include <xdebug.h>
#include <xil_types.h>
#include <xil_io.h>
#include <xil_cache.h>
#include <xil_mmu.h>
#include <xscugic.h>
#include <xil_exception.h>

#include "pcie_ep_pio.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

XScuGic INTCinst;

/*
 * Read bar address of specified bar number
 */
void read_bar(u32 bar_num, u32 *bar_lo, u32 *bar_hi)
{
	printf("BAR Read\n");
	*bar_lo = Xil_In32(BAR0_OFFSET_LO + (bar_num * 0x4));
	//*bar_hi = Xil_In32(BAR0_OFFSET_HI + (bar_num * 0x4));
	// For 32-bit BARs, bar_hi is always 0x0
	*bar_hi = 0;

	printf("BAR%d LO configured by host 0x%lx\n",bar_num, *bar_lo);
	printf("BAR%d HI configured by host 0x%lx\n",bar_num, *bar_hi);
}

void read_bar_JRL(u32 bar_num, u32 *bar_lo, u32 *bar_hi, u32 bar_size)
{
	printf("BAR Read JRL\n");
	*bar_lo = Xil_In32(BAR0_OFFSET_LO + (bar_num * 0x4));
	*bar_hi = *bar_lo + bar_size - 0x4;

	printf("BAR%d LO configured by host 0x%lx\n",bar_num, *bar_lo);
	printf("BAR%d HI configured by host 0x%lx\n",bar_num, *bar_hi);
}

/*
 *Function setting up Ingress translation
 */
void SetupIngress(u32 src_lo, u32 src_hi, u32 dst_lo, u32 trans_size, u32 trans_off)
{
	u32 val;

	/*
	 * Using Ingress Address Translation 0 to setup translation
	 * to PS DDR
	 */
	Xil_Out32((INGRESS0_SRC_BASE_LO + (trans_off * 0x20)),
				src_lo & ~0xf);
	Xil_Out32((INGRESS0_SRC_BASE_HI + (trans_off * 0x20)),
				src_hi);

	printf("Done writing the Ingress Src registers\n");

	Xil_Out32((INGRESS0_DST_BASE_LO + (trans_off * 0x20)), dst_lo);
	Xil_Out32((INGRESS0_DST_BASE_HI + (trans_off * 0x20)), 0U);

	printf("Done writing the Ingress Dst registers\n");

	val = Xil_In32(INGRESS0_CONTROL);

	printf("Read Ingress Control register\n");

	val &= (u32)(~INGRESS_SIZE_MASK);
	val |= (((u32)trans_size << INGRESS_SIZE_SHIFT) |
		(u32)INGRESS_ENABLE | (u32)INGRESS_SECURITY_ENABLE);
	val |= INGRESS_RD_WR_ATTR;
	printf("Set ingress control register to %x, %u\n", val, val);
	Xil_Out32((INGRESS0_CONTROL + (trans_off * 0x20)), val);

	printf("Done setting up the ingress trasnslation registers\n");

}

void DisplayIngress(u32 trans_off) {

	u32 x;
	u32 address;
	printf("Displaying translation for Region %d\n", trans_off);

	address = (INGRESS0_SRC_BASE_LO + (trans_off * 0x20));
	x = Xil_In32(address);
	printf("%x : %x\n", address, x);

	address = (INGRESS0_SRC_BASE_HI + (trans_off * 0x20));
	x = Xil_In32(address);
	printf("%x : %x\n", address, x);

	address = (INGRESS0_DST_BASE_LO + (trans_off * 0x20));
	x = Xil_In32(address);
	printf("%x : %x\n", address, x);

	address = (INGRESS0_DST_BASE_HI + (trans_off * 0x20));
	x = Xil_In32(address);
	printf("%x : %x\n", address, x);

	address = (INGRESS0_CONTROL + (trans_off * 0x20));
	x = Xil_In32(address);
	printf("%x : %x\n", address, x);
}

/* Interrupt handler for SW interrupt in AXI domain */
void Intr_Handler(u32 base)
{
	u32 val, size;

	Xil_Out32(DMA0_CHAN_AXI_INTR_STATUS, AXI_INTR_STATUS);
	val = Xil_In32(DMA0_CHAN_SCRATCH0);
	printf("In Interrupt handler Value @SCRATCH0 = %0lx\n", val);

	if (val == INGRESS_TEST_DONE)
	{
		val = Xil_In32(INGRESS0_CONTROL);
		if (val & 0x00000001) {
			size = (1 << INGRESS_SIZE_ENCODING) * (1 << INGRESS_MIN_SIZE);
			Xil_Out32(INGRESS_TRANS_SET_OFFSET, size);
			Xil_Out32(DMA_PCIE_INTR_ASSRT_REG_OFFSET, PCIE_INTR_STATUS);
			Xil_Out32(DMA0_CHAN_SCRATCH0, 0x0);
		}
		else {
			Xil_Out32(INGRESS_TRANS_SET_OFFSET, 0x0);
			Xil_Out32(DMA_PCIE_INTR_ASSRT_REG_OFFSET, PCIE_INTR_STATUS);
			Xil_Out32(DMA0_CHAN_SCRATCH0, 0x0);
		}
	}

}

/*
 * Initialize interrupts
 */
int InitIntr()
{
	XScuGic_Config *IntcConfig;
	int Status;
	u32 base = 0xFD0F0000;

	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
			return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&INTCinst, IntcConfig, IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &INTCinst);


	Status = XScuGic_Connect(&INTCinst, PS_PCIE_AXI_INTR_ID, (Xil_ExceptionHandler) Intr_Handler, (void *) &base);
	if (Status != XST_SUCCESS)
			return XST_FAILURE;
	XScuGic_Enable(&INTCinst, PS_PCIE_AXI_INTR_ID);

	Xil_ExceptionEnable();

	return Status;
}

/*
 * Function for bridge initialization
 */
void BridgeInit()
{
	u32 val;

	/* Bridge Configurations */
	val = (u32)AXIPCIE_MAIN_BASE;
	Xil_Out32(BREG_BASE_LO, val);
	Xil_Out32(BREG_BASE_HI, 0U);

	val = Xil_In32(BREG_CONTROL);
	val &= ~(BREG_SIZE_MASK | BREG_ENABLE_FORCE);
	val |= (BREG_SIZE << BREG_SIZE_SHIFT);
	val |= BREG_ENABLE;
	Xil_Out32(BREG_CONTROL, val);

	/* DMA regs Configurations */
	val = (u32)DREG_BASE;
	Xil_Out32(DREG_BASE_LO, val);
	Xil_Out32(DREG_BASE_HI, 0U);

	val = Xil_In32(DREG_CONTROL);
	val |= DMA_ENABLE;
	Xil_Out32(DREG_CONTROL, val);

	/* ECAM Configurations */
	val = (u32)ECAM_BASE;
	Xil_Out32(ECAM_BASE_LO, val);
	Xil_Out32(ECAM_BASE_HI, 0U);

	/* ECAM Enable */
	val = Xil_In32(ECAM_CONTROL);
	val |= ECAM_ENABLE;
	Xil_Out32(ECAM_CONTROL, val);

	//- Enable AXI domain interrupt
	Xil_Out32(DMA0_CHAN_AXI_INTR, AXI_INTR_ENABLE);
	Xil_Out32(MSGF_DMA_MASK, MSGF_DMA_INTR_ENABLE);
}

int main(void)
{
	u32 val;
	u32 bar_base_lo, bar_base_hi;

	printf("PCIE Ingress Test start \n");
	InitIntr();
	printf("Interrupt initialized\n");

	printf("Waiting for PCIe Link up\n");
	do {
		val = Xil_In32(PCIE_STATUS);
	} while (!(val & PCIE_LINK_UP));
	printf("PCIe Link up...\n");

	BridgeInit();
	printf("Bridge Init done...\n");

	/*
	 * Wait for the host to enumerate us.
	 * on x86 BIOS sets memory enable bit
	 * on ARM64 it is set when driver gets inserted
	 */
	do {
			val = Xil_In32(COMMAND_REG);
			usleep(10);
	} while (!(val & 0x00000002U));

	printf("Host driver indicated ready\n");
	//read_bar_JRL(BAR_NUM2, &bar_base_lo, &bar_base_hi, BAR2_SIZE);
	read_bar(BAR_NUM2, &bar_base_lo, &bar_base_hi);

	SetupIngress(bar_base_lo, bar_base_hi, PS_DDR_ADDR, INGRESS_SIZE_ENCODING, INGRESS_NUM0);
	printf("Set up BAR2 to DDR (I hope!)\n");
	//read_bar_JRL(BAR_NUM1, &bar_base_lo, &bar_base_hi, BAR1_SIZE);
	read_bar(BAR_NUM1, &bar_base_lo, &bar_base_hi);
	SetupIngress(bar_base_lo, bar_base_hi, MY_LEDS, INGRESS_SIZE_ENCODING, INGRESS_NUM1);
	printf("Set translation to LEDs (I hope!)\n");
	DisplayIngress(INGRESS_NUM0);
	DisplayIngress(INGRESS_NUM1);


	while (1);

	printf("PCIE Ingress Test done\n");

	return 0;

}
