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

#if !defined __PCIE_EP_PIO_H__
#define __PCIE_EP_PIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define PS_DDR_ADDR					0x1000000
#define MY_LEDS						0x80000000

#define INTC_DEVICE_ID                          XPAR_SCUGIC_0_DEVICE_ID
#define PS_PCIE_AXI_INTR_ID                     (117U + 32U)

#define BAR_NUM0					0x0
#define BAR_NUM1					0x1
#define BAR_NUM2					0x2

#define INGRESS_NUM0				0x0
#define INGRESS_NUM1				0x1

#define BAR0_SIZE					0x10000U
#define BAR1_SIZE					0x8000U
#define BAR2_SIZE					0x10000U

#define PCIE_ATTRIB_BASE			0xFD480000U
#define AXIPCIE_MAIN_BASE			0xFD0E0000U
#define DREG_BASE					0xFD0F0000U
#define ECAM_BASE					0xE0000000U

#define COMMAND_REG					(ECAM_BASE + 0x4U)
#define BAR0_OFFSET_LO				(ECAM_BASE + 0x10U)
#define BAR0_OFFSET_HI				(ECAM_BASE + 0x14U)
#define BAR1_OFFSET_LO				(ECAM_BASE + 0x18U)
#define BAR1_OFFSET_HI				(ECAM_BASE + 0x1CU)
#define PCIE_STATUS					(PCIE_ATTRIB_BASE + 0x238U)

#define BREG_CONTROL				(AXIPCIE_MAIN_BASE + 0x208U)
#define BREG_BASE_LO				(AXIPCIE_MAIN_BASE + 0x210U)
#define BREG_BASE_HI				(AXIPCIE_MAIN_BASE + 0x214U)

#define ECAM_CONTROL				(AXIPCIE_MAIN_BASE + 0x228U)
#define ECAM_BASE_LO				(AXIPCIE_MAIN_BASE + 0x230U)
#define ECAM_BASE_HI				(AXIPCIE_MAIN_BASE + 0x234U)

#define DREG_CONTROL				(AXIPCIE_MAIN_BASE + 0x288U)
#define DREG_BASE_LO				(AXIPCIE_MAIN_BASE + 0x290U)
#define DREG_BASE_HI				(AXIPCIE_MAIN_BASE + 0x294U)

#define INGRESS0_CONTROL			(AXIPCIE_MAIN_BASE + 0x808U)
#define INGRESS0_SRC_BASE_LO		(AXIPCIE_MAIN_BASE + 0x810U)
#define INGRESS0_SRC_BASE_HI		(AXIPCIE_MAIN_BASE + 0x814U)
#define INGRESS0_DST_BASE_LO		(AXIPCIE_MAIN_BASE + 0x818U)
#define INGRESS0_DST_BASE_HI		(AXIPCIE_MAIN_BASE + 0x81CU)

#define INGRESS1_CONTROL			(AXIPCIE_MAIN_BASE + 0x818U)
#define INGRESS1_SRC_BASE_LO		(AXIPCIE_MAIN_BASE + 0x820U)
#define INGRESS1_SRC_BASE_HI		(AXIPCIE_MAIN_BASE + 0x824U)
#define INGRESS1_DST_BASE_LO		(AXIPCIE_MAIN_BASE + 0x828U)
#define INGRESS1_DST_BASE_HI		(AXIPCIE_MAIN_BASE + 0x82CU)

#define INGRESS_TRANS_SET_OFFSET    (DREG_BASE + 0x54U)
#define MSGF_DMA_MASK               (AXIPCIE_MAIN_BASE + 0x464)

#define PCIE_LINK_UP				0x000000001U
#define ECAM_ENABLE					0x000000001U
#define INGRESS_ENABLE				0x000000001U
#define INGRESS_SECURITY_ENABLE		0x000000004U
#define DMA_ENABLE					0x000000001U
#define BREG_SIZE					0x000000004U
#define BREG_ENABLE					0x000000001U
#define BREG_ENABLE_FORCE			0x000000002U

#define DMA0_CHAN_BASE                  0xFD0F0000
#define DMA0_CHAN_AXI_INTR              (DMA0_CHAN_BASE + 0x68)
#define AXI_INTR_ENABLE                 0x00000001U
#define MSGF_DMA_INTR_ENABLE            0x00000001U
#define DMA0_CHAN_AXI_INTR_STATUS       (DMA0_CHAN_BASE + 0x6C)
#define AXI_INTR_STATUS                 0x00000008U
#define DMA0_CHAN_SCRATCH0              (DMA0_CHAN_BASE + 0x50)
#define DMA_PCIE_INTR_ASSRT_REG_OFFSET  (DMA0_CHAN_BASE + 0x70)
#define PCIE_INTR_STATUS                0x00000008U


#define BREG_SIZE_SHIFT					16
#define INGRESS_SIZE_SHIFT				16

#define BREG_SIZE_MASK					0x00030000U
#define INGRESS_SIZE_MASK				0x001F0000U

#define INGRESS_TEST_DONE               0xCCCCCCCCU
//#define INGRESS_SIZE_ENCODING			0x00000008U /* 2^(12+8) = 1MB */
#define INGRESS_SIZE_ENCODING			0x00000000U /* 2^(12+0) = 4kB */
#define INGRESS_MIN_SIZE				0x0000000CU

#define INGRESS_RD_WR_ATTR				0xFF800000U

#endif /* __PCIE_EP_PIO_H__ */
