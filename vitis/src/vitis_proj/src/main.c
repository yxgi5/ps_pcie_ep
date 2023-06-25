/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_ep_enable_example.c
*
* This file contains a design example for using PS PCIe IP and its
* driver. This is an example to show the usage of driver APIs which configures
* PS PCIe EndPoint.
*
* The example initializes the PS PCIe EndPoint and shows how to use the API's.
*
* This code will illustrate how the XPciePsu  and its standalone driver can
* be used to:
*  - Initialize a PS PCIe bridge core built as an end point
*  - Retrieve root complex configuration assigned to end point
*  - Provides ingress translation setup
*
* We tried to use as much of the driver's API calls as possible to show the
* reader how each call could be used and that probably made the example not
* the shortest way of doing the tasks shown as they could be done.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
*  1.0   tk  02/13		Initial version
*</pre>
*******************************************************************************/

/******************************** Include Files *******************************/
#include "xpciepsu_ep.h"
#include "stdio.h"
#include "xil_printf.h"
#include "xparameters.h" /* Defines for XPAR constants */
#include "xpciepsu_common.h"
#include <xscugic.h>
#include <xil_exception.h>

#include "sleep.h"
#include "xvidc.h"
#include "tpg/tpg.h"
#include "clk_wiz/clk_wiz.h"
#include "xvtc.h"
#include "vtc/video_resolution.h"
#include "vtc/vtiming_gen.h"
#include "xaxivdma.h"

#define VDMA_ID          		XPAR_AXIVDMA_0_DEVICE_ID
#if defined (__MICROBLAZE__)
	#define DDR_BASEADDR XPAR_MICROBLAZE_DCACHE_BASEADDR
#else
	#define DDR_BASEADDR XPAR_DDR_MEM_BASEADDR
#endif

#define FRAME_BUFFER_BASE_ADDR  	(DDR_BASEADDR + (0x10000000))

#define FRAME_BUFFER_SIZE       0x2000000    //0x2000000 for max 4KW RGB888 8bpc
#define FRAME_BUFFER_1			FRAME_BUFFER_BASE_ADDR
#define FRAME_BUFFER_2			FRAME_BUFFER_BASE_ADDR + FRAME_BUFFER_SIZE
#define FRAME_BUFFER_3			FRAME_BUFFER_BASE_ADDR + (FRAME_BUFFER_SIZE*2)

XClk_Wiz ClkWizInst0;
XVtc        VtcInst0;       /**< Instance of the VTC core. */
XVtc_Config *VtcConfig0;
XV_tpg tpg_inst0;
XV_tpg_Config *tpg_config0;
u32 bckgndId0=XTPG_BKGND_COLOR_BARS;
XVidC_ColorFormat colorFmtIn0 = XVIDC_CSF_RGB;
XVidC_ColorFormat colorFmtOut0 = XVIDC_CSF_RGB;

/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/
#define INGRESS_NUM	0x0		/* Ingress num to setup ingress */
#define BAR_NUM		0x2		/* Bar no to setup ingress */
#define PS_DDR_ADDR	0x1000000	/* 32 or 64 bit PS DDR Addr
						to setup ingress */
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define PS_PCIE_AXI_INTR_ID	(117U + 32U)
/***************************** Function Prototypes ****************************/

int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId);
void XPciePsu_EP_IntrHandler(XPciePsu *PciePsuPtr);
int XPciePsu_EP_InitIntr(void);

/**************************** Variable Definitions ****************************/
/* PCIe IP Instance */
static XPciePsu PciePsuInstance;
XScuGic INTCinst;

/******************************************************************************/
/**
* This function handles doorbell interrupts for PCIE Endpoint
*
* @param   PciePsuPtr is a pointer to an instance of XPciePsu data
*
* @return  -None
*
* @note    None
*
*********************************************************************************/

void XPciePsu_EP_IntrHandler(XPciePsu *PciePsuPtr)
{
	u32 val, size;

	XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
			DMA0_CHAN_AXI_INTR_STATUS,
			AXI_INTR_STATUS);
	val=XPciePsu_ReadReg(PciePsuPtr->Config.DmaBaseAddr,
			DMA0_CHAN_SCRATCH0);
	xil_printf("In Interrupt handler Value @SCRATCH0 = %x  \n", val);

	if (val == INGRESS_TEST_DONE)
	{
		 val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,INGRESS0_CONTROL);
		if (val & 0x00000001U) {
			size = (1 << INGRESS_SIZE_ENCODING) * (1 << INGRESS_MIN_SIZE);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					INGRESS_TRANS_SET_OFFSET,
					size);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA_PCIE_INTR_ASSRT_REG_OFFSET,
					PCIE_INTR_STATUS);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA0_CHAN_SCRATCH0,
					0X0U);
		}
		else {
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					INGRESS_TRANS_SET_OFFSET,
					0x0U);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA_PCIE_INTR_ASSRT_REG_OFFSET,
					PCIE_INTR_STATUS);
			XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
					DMA0_CHAN_SCRATCH0,
					0x0U);
		}
	}
}

/******************************************************************************/
/**
* This function Initializes interrupts for PCIe EndPoint
*
* @param    None
*
* @return   - XST_SUCCESS if successful
*           - XST_FAILURE if unsuccessful
*
* @note     None
*
********************************************************************************/

int XPciePsu_EP_InitIntr(void)
{
	XScuGic_Config *IntcConfig;
	int Status;

	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
			return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&INTCinst, IntcConfig, IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &INTCinst);

	Status = XScuGic_Connect(&INTCinst, PS_PCIE_AXI_INTR_ID, (Xil_ExceptionHandler) XPciePsu_EP_IntrHandler, (void *) &PciePsuInstance);
	if (Status != XST_SUCCESS)
			return XST_FAILURE;
	XScuGic_Enable(&INTCinst, PS_PCIE_AXI_INTR_ID);

	Xil_ExceptionEnable();

	return Status;
}


void clkwiz_vtc_cfg(void)
{
    u32 Status;

    // dynamic config clkwiz
    Status = XClk_Wiz_dynamic_reconfig(&ClkWizInst0, XPAR_CLK_WIZ_0_DEVICE_ID);
    if (Status != XST_SUCCESS)
    {
      xil_printf("XClk_Wiz0 dynamic reconfig failed.\r\n");
//      return XST_FAILURE;
    }
    xil_printf("XClk_Wiz0 dynamic reconfig ok\n\r");

    // vtc configuration
    VtcConfig0 = XVtc_LookupConfig(XPAR_VTC_0_DEVICE_ID);
    Status = XVtc_CfgInitialize(&VtcInst0, VtcConfig0, VtcConfig0->BaseAddress);
    if(Status != XST_SUCCESS)
    {
        xil_printf("VTC0 Initialization failed %d\r\n", Status);
//      return(XST_FAILURE);
    }

    vtiming_gen_run(&VtcInst0, VIDEO_RESOLUTION_1080P, 2);
}

void tpg_config()
{
    u32 Status;

    // tpg0
    xil_printf("TPG0 Initializing\n\r");

    Status = XV_tpg_Initialize(&tpg_inst0, XPAR_V_TPG_0_DEVICE_ID);
    if(Status!= XST_SUCCESS)
    {
        xil_printf("TPG0 configuration failed\r\n");
//      return(XST_FAILURE);
    }

    tpg_cfg(&tpg_inst0, 1080, 1920, colorFmtIn0, bckgndId0);

    //Configure the moving box of the TPG0
    tpg_box(&tpg_inst0, 50, 1);

    //Start the TPG0
    XV_tpg_EnableAutoRestart(&tpg_inst0);
    XV_tpg_Start(&tpg_inst0);
    xil_printf("TPG0 started!\r\n");
}

void clear_display(void)
{
	u32 bytePerPixels = 3;
	u32 line = 0;
	u32 column = 0;

	line = 1920;
	column = 1080;

	Xil_DCacheDisable();
    memset((void *)FRAME_BUFFER_1,0xff,line*column*bytePerPixels);//background
//    memset(FRAME_BUFFER_2,0xff,line*column*bytePerPixels);//background
//    memset(FRAME_BUFFER_3,0xff,line*column*bytePerPixels);//background
	Xil_DCacheEnable();
}

void vdma0_config_32(void)
{
    /* Start of VDMA Configuration */
    u32 bytePerPixels = 3;

    int offset0 = 0; // (y*w+x)*Bpp
//    int offset1 = 0; // (y*w+x)*Bpp

    u32 stride0 = 1920;
    u32 width0 = 1920;
    u32 height0 = 1080;
//    u32 stride1 = 1920;  // crop keeps write Stride
//    u32 width1 = 1920;
//    u32 height1 = 1080;

    /* Configure the Write interface (S2MM)*/
    // S2MM Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x30, 0x8B);
    //S2MM Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xAC, FRAME_BUFFER_1 + offset0);
    //S2MM Start Address 2
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB0, FRAME_BUFFER_2 + offset0);
    //S2MM Start Address 3
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB4, FRAME_BUFFER_3 + offset0);
    //S2MM Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA8, stride0*bytePerPixels);
    // S2MM HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA4, width0*bytePerPixels);
    // S2MM VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA0, height0);

//    /* Configure the Read interface (MM2S)*/
//    // MM2S Control Register
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x8B);
//    // MM2S Start Address 1
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, FRAME_BUFFER_1 + offset1);
//    // MM2S Start Address 2
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, FRAME_BUFFER_2 + offset1);
//    // MM2S Start Address 3
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, FRAME_BUFFER_3 + offset1);
//    // MM2S Frame delay / Stride register
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, stride1*bytePerPixels);
//    // MM2S HSIZE register
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, width1*bytePerPixels);
//    // MM2S VSIZE register
//    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, height1);

    //xil_printf("VDMA started!\r\n");
    /* End of VDMA Configuration */
}

void vdma_config(void)
{
	vdma0_config_32();
}

/******************************************************************************/
/**
* This function is the entry point for PCIe EndPoint Example
*
* @param    None
*
* @return   - XST_SUCCESS if successful
*       - XST_FAILURE if unsuccessful.
*
* @note     None.
*
*******************************************************************************/

int main()
{
	int Status = XST_SUCCESS;
#ifdef XPAR_PSU_PCIE_DEVICE_ID
	XPciePsu_InitEndPoint(&PciePsuInstance, XPAR_PSU_PCIE_DEVICE_ID);

	XPciePsu_EP_InitIntr();
	xil_printf("Waiting for PCIe Link up\r\n");
	XPciePsu_EP_WaitForLinkup(&PciePsuInstance);
	xil_printf("PCIe Link up...\r\n");

	XPciePsu_EP_BridgeInitialize(&PciePsuInstance);
	xil_printf("Bridge Init done...\r\n");

	XPciePsu_EP_WaitForEnumeration(&PciePsuInstance);

	xil_printf("Host driver indicated ready\r\n");
	int result = XPciePsu_EP_SetupIngress(&PciePsuInstance,
			INGRESS_NUM, BAR_NUM, PS_DDR_ADDR);
	if (result == XST_FAILURE) {
		xil_printf("PCIE ingress setup failed\r\n");
	} else {
		xil_printf("PCIE Ingress Test done\r\n");
	}

    clkwiz_vtc_cfg();
    tpg_config();
    vdma_config();
	while(1);

#endif
	return Status;
}

/******************************************************************************/
/**
* This function initializes a PSU PCIe EndPoint complex.
*
* @param    PciePsuPtr is a pointer to an instance of XPciePsu data
*       structure represents a root complex.
* @param    DeviceId is PSU PCIe root complex unique ID
*
* @return   - XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note     None.
*
*
*******************************************************************************/

int XPciePsu_InitEndPoint(XPciePsu *PciePsuPtr, u16 DeviceId)
{
	const XPciePsu_Config *ConfigPtr;
	ConfigPtr = XPciePsu_LookupConfig(DeviceId);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	if (ConfigPtr->PcieMode != XPCIEPSU_MODE_ENDPOINT) {
		xil_printf("Psu pcie mode is not configured as endpoint\r\n");
		return XST_FAILURE;
	}
	XPciePsu_EP_CfgInitialize(PciePsuPtr, ConfigPtr);
	return XST_SUCCESS;
}
