#if !defined(VERSAL_PLATFORM)
/*!
 * @brief     CDMA API Source File
 *
 * @copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_CDMA_API__
 * @{
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdlib.h>
#if defined(__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#endif
#include "xil_io.h"
#include "fpga_regs.h"
#include "ps7_ads9cfg.h"
#include "xaxicdma.h"
#include "zynq_cdma.h"
#include "mz_reg.h"
#include "hal_cdma.h"
#include "hal_fpga.h"
#include "fpga_regs.h"
#include "yoda_util.h"
#include "simple_dma.h"
#include "jesd204_cfg_settings.h"
#include "xparameters_ads9v2.h"
#include "fpga_reg_util.h"
#include "sw_version.h"
#include "ps7_ads9cfg.h"
#include "cdmaapi.h"

/*============= D E F I N E S ==============*/
#define MZDDRADDR_0     0x3C000000
#define MZDDRADDR_1     0x3F900000
#define MZDDRADDR_2     0x3FB00000

#define FIFOADDR        0x80060000

#define HMCADDR         0x41000000
#define AXI_CDMA_0      0x60000000

static int      g_fd;
static uint32_t g_hmcbaseaddr;
static uint32_t g_size;
static uint32_t g_count;
static uint32_t g_accul;
static uint32_t g_optype;
static void*    g_mapbase;

static void XAxiCdma_AresetPulseGenerate(void);
/*============= C O D E ====================*/
static int XAxiCdma_SimplePollExample(XAxiCdma_Config* CfgPtr,
				      XAxiCdma* InstancePtr, u16 DeviceId,
				      u32 keyhole_enable, u32 kh_w, u32 SrcAddr,
				      u32 DstAddr, int Length)
{
    int Status, Error;
    int Timeout, Retries = 10;  /* try 10 times on submission */

    /* Initialize the XAxiCdma device. */
    CfgPtr = XAxiCdma_LookupConfig(DeviceId);
    if (!CfgPtr) {
        return XST_FAILURE;
    }
    Status = XAxiCdma_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);
    
    /* If Xilinx AXI CDM reset hangs, only option to recover is to reset Micro-Zed */
    if (Status == XST_RESET_ERROR) {        
        printf("TRYING TO RECOVER FROM XILINX CDMA RESET FAULTY STATE\n\n");

        XAxiCdma_AresetPulseGenerate();

        Status = XAxiCdma_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);//sync();
    }
    
    if (Status != XST_SUCCESS) {
        return Status;
    }

    Status = XAxiCdma_SelectKeyHole(InstancePtr, kh_w, keyhole_enable);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /* Disable interrupts, we use polling mode */
    /* Flush the buffer before DMA transmission */
    do {
        Status = XAxiCdma_SimpleTransfer(InstancePtr, SrcAddr, DstAddr, Length,
					 NULL, NULL);
        if (Status == XST_SUCCESS)
            break;
    } while (-- Retries);

    if (!Retries) {
        printf("CDMA re-tried over too many times, and failed.\r\n");
        return XST_FAILURE;
    }

    Retries = 0x01ffffff;
    while (XAxiCdma_IsBusy(InstancePtr) && (Retries != 0)) {
        Retries--;
#ifdef DEBUG_CDMA
        printf("XAXICDMA_SR_OFFSET: 0x%8x\n",
	       XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_SR_OFFSET));
#else
	;
#endif
    }

    if (!Retries) {
        printf("CDMA IsBusy timeout.\r\n");
        return XST_DEVICE_BUSY;
    }

    Error = XAxiCdma_GetError(InstancePtr);
    if (Error != 0x00) {
        // Need to reset the hardware to restore to the correct state
        XAxiCdma_Reset(InstancePtr);
        Timeout = 10;
        do {
            if (XAxiCdma_ResetIsDone(InstancePtr)) {
                break;
            }
            usleep(20);
        } while (-- Timeout);
        printf("CDMA Error: 0x%x.\r\n", Error);

        return XST_FAILURE;
    }

    /* Test finishes successfully */
    return XST_SUCCESS;
}

int cdmatransfer_ddrtobram(uint32_t ddr_addr, uint32_t size)
{
    XAxiCdma AxiCdmaInstance;
    XAxiCdma_Config* CfgPtr = NULL;
    uint32_t BRAM_Addr = 0x80080000;
    int status, err = -1;

    if (size <= 128 * 1024) {
        if (Hal_CdmaInit(0x60000000, 0x1000) != 0) {
            printf("Call Hal_Init error.\r\n");
            return -1;
        }
        status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance, DMA_CTRL_DEVICE_ID, 0, XAXICDMA_KEYHOLE_WRITE, ddr_addr, BRAM_Addr, size);
        if (status != XST_SUCCESS) {
            printf("XAxiCdma_SgIntr Transfer Failed\r\n");
            return XST_FAILURE;
        }
        else {
            err = 0;
        }
        Hal_CdmaDeinit();
    }
    else {
        printf("Size is larger than BRAM Size. It should be less than 128KB.\r\n");
    }

    return err;
}

int cdmatransfer_bramtoddr(uint32_t ddr_addr, uint32_t size)
{
    XAxiCdma AxiCdmaInstance;
    XAxiCdma_Config* CfgPtr = NULL;
    uint32_t BRAM_Addr = 0x80080000;
    int status, err = -1;

    if (size <= 128 * 1024) {
        Hal_CdmaInit(0x60000000, 0x100);
        status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance, DMA_CTRL_DEVICE_ID, 0, XAXICDMA_KEYHOLE_READ, BRAM_Addr, ddr_addr, size);
        if (status != XST_SUCCESS) {
            printf("XAxiCdma_SgIntr Transfer Failed\r\n");
            return XST_FAILURE;
        }
        else {
            err = 0;
        }
        Hal_CdmaDeinit();
    }
    else {
        printf("Size is larger than BRAM Size. It should be less than 128KB.\r\n");
    }

    return err;
}

int cdmatransfer_ddrtohmc(uint32_t ddr_addr, uint32_t hmc_addr, uint32_t size)
{
    XAxiCdma AxiCdmaInstance;
    XAxiCdma_Config* CfgPtr = NULL;
    uint32_t FIFO_Addr = 0x80060000;
    int status, err = 0;
    unsigned int sleep_us = 10;
    unsigned int timeout = 1000000 / sleep_us;

    Hal_FpgaInit(0x40000000, 0x10000);
    FPGA_REG_WR_FIELD(REG_FPGA_REGS0_GT_TX_PATTERN_LOAD_CTRL, 0x1, GT_TX_PATTERN_LOAD_CTRL_GT_TX_PTN_LOAD_STOP);
    usleep(100);

    FPGA_REG_WR(REG_FPGA_REGS0_GT_TX_PATTERN_ADDR, hmc_addr);
    FPGA_REG_WR(REG_FPGA_REGS0_GT_TX_PATTERN_LEN, size);
    FPGA_REG_WR_FIELD(REG_FPGA_REGS0_GT_TX_PATTERN_CTRL, 0x0, GT_TX_PATTERN_CTRL_GT_TX_DATA_MODE);
    FPGA_REG_WR_FIELD(REG_FPGA_REGS0_GT_TX_PATTERN_LOAD_CTRL, 0x1, GT_TX_PATTERN_LOAD_CTRL_GT_TX_PTN_LOAD_START);
    Hal_FpgaDeinit();
    usleep(500000*2);

    Hal_CdmaInit(0x60000000, 0x1000);
    usleep(100);
    status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance, DMA_CTRL_DEVICE_ID, 1, XAXICDMA_KEYHOLE_WRITE, ddr_addr, FIFO_Addr, size);
    if (status != XST_SUCCESS) {
        printf("XAxiCdma_SgIntr Transfer Failed\r\n");
        return XST_FAILURE;
    }
    Hal_CdmaDeinit();
    usleep(100);

    Hal_FpgaInit(0x40000000, 0x10000);
    while(FPGA_REG_RD_FIELD(REG_FPGA_REGS0_GT_TX_PATTERN_LOAD_CTRL, GT_TX_PATTERN_LOAD_CTRL_GT_TX_PTN_LOAD_RUNNING))
    {
        usleep(sleep_us);

        if (timeout-- == 0) {
            printf("TIMEOUT, pattern load: cdmatransfer_ddrtohmc\n"); 
            FPGA_REG_WR_FIELD(REG_FPGA_REGS0_GT_TX_PATTERN_LOAD_CTRL, 0x1, GT_TX_PATTERN_LOAD_CTRL_GT_TX_PTN_LOAD_STOP);
            timeout = 5000000 / sleep_us;
            err = -1;
            break;
        }
    }
    Hal_FpgaDeinit();
    usleep(100);

    return err;
}

int ddmatransfer_hmctoddr(uint32_t ddr_addr, uint32_t hmc_addr, uint32_t size)
{
    XAxiCdma AxiCdmaInstance;
    XAxiCdma_Config* CfgPtr = NULL;
    uint32_t FIFO_Addr = 0x80060000;
    int status, err = -1;

    Hal_CdmaInit(0x60000000, 0x1000);
    usleep(100);
    status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance, DMA_CTRL_DEVICE_ID, 1, XAXICDMA_KEYHOLE_READ, FIFO_Addr, ddr_addr, size);
    if (status != XST_SUCCESS) {
        printf("XAxiCdma_SgIntr Transfer Failed\r\n");
        return XST_FAILURE;
    }
    else {
        err = 0;
    }
    Hal_CdmaDeinit();

    return err;
}

int cdma_start(uint32_t base_addr, uint32_t size, int op_type)
{
    int err = -1;

    g_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (g_fd > 0) {
    g_optype = op_type;
        if (op_type == DDRTOHMC) {
            g_mapbase = mmap(0, DMA_1MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, MZDDRADDR_0);
        } else if (op_type == DDRTOHMC_ALT) {
            g_mapbase = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, MZDDRADDR_0);
        } else {
            g_mapbase = mmap(0, DMA_1MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, MZDDRADDR_2);
        }
        if (g_mapbase != (void*)-1) {
            g_hmcbaseaddr = base_addr;
            g_size = size;
            g_accul = g_count = 0;
            err = 0;
        } else {
            printf("memory map error, address is 0x%x.\r\n", base_addr);
            close(g_fd);
        }
    } else {
        printf("Open /dev/mem error.\r\n");
    }

    return err;
}

int cdma_inprogress(int op_type, uint8_t* pdata, uint32_t data_size)
{
    int tmp, err = -1;

    if (op_type == DDRTOHMC) {
        if (g_accul + data_size > DMA_1MSIZE) {
            memcpy((void *)((uintptr_t)g_mapbase + g_accul), pdata, (DMA_1MSIZE - g_accul));
            tmp = data_size - (DMA_1MSIZE - g_accul);
            g_accul = DMA_1MSIZE;
        } else {
            memcpy((void *)((uintptr_t)g_mapbase + g_accul), pdata, data_size);
            g_accul += data_size;
            tmp = 0;
        }
        if (g_accul >= DMA_1MSIZE) {
            if (cdmatransfer_ddrtohmc(MZDDRADDR_0, g_hmcbaseaddr + g_count, DMA_1MSIZE) == 0) {
                g_count += DMA_1MSIZE;
                if (tmp) {
                    memcpy(g_mapbase, pdata + (data_size - tmp), tmp);
                    g_accul = tmp;
                } else {
                    g_accul = 0;
                }
                err = 0;
            }
        } else {
            err = 0;
        }
    } else if (op_type == DDRTOHMC_ALT) {
        memcpy((void *)((uintptr_t)g_mapbase + g_accul), pdata, data_size);
        err = cdmatransfer_ddrtohmc(MZDDRADDR_0, g_hmcbaseaddr + g_count, data_size);
        if (err != 0) {
            goto end;
        }
        g_accul = 0;
    } else {
        if (ddmatransfer_hmctoddr(MZDDRADDR_2, g_hmcbaseaddr + g_count, DMA_1MSIZE) == 0) {
            memcpy(pdata, g_mapbase, data_size);
            g_count += DMA_1MSIZE;
            err = 0;
        } else {
            err = -1;
            goto end;
        }
    }

end:
    return err;
}

int cdma_end(void)
{
    int err = 0;

    if (g_optype == DDRTOHMC) {
        if (g_accul != 0) {
            err = cdmatransfer_ddrtohmc(MZDDRADDR_0, g_hmcbaseaddr + g_count, DMA_1MSIZE);
        }
    }

    munmap(g_mapbase, g_optype == DDRTOHMC_ALT ? g_size : DMA_1MSIZE);
    g_hmcbaseaddr = g_size = 0;
    g_mapbase = NULL;
    close(g_fd);

    return err;
}


int CdmaTransfer(uint32_t src_addr, uint32_t dst_addr, uint32_t size,
	uint8_t cdma_write, uint8_t keyhole)
{
	XAxiCdma AxiCdmaInstance;
	XAxiCdma_Config* CfgPtr = NULL;
	int status; 

	/* Nothing to do for 0 bytes, just return. */
	if (size == 0)
		return 0;

	//printf("Hal_CdmaInit...   ");
	Hal_CdmaInit(AXI_CDMA_0, 0x1000);
#ifdef DEBUG_CDMA
	printf("XAxiCdma_SimplePollExample - src addr %x - size %d \r\n",
	       src_addr, size);
#endif
	// CDMA Simple (non-SG) Polling mode
	if ((cdma_write == 1) && (keyhole == 1))
		status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance,
			DMA_CTRL_DEVICE_ID, 1,
			XAXICDMA_KEYHOLE_WRITE,
			src_addr, dst_addr, size);
        else if ((cdma_write == 0) && (keyhole == 1))
		status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance,
			DMA_CTRL_DEVICE_ID, 1,
			XAXICDMA_KEYHOLE_READ,
			src_addr, dst_addr, size);
        else if ((cdma_write == 1) && (keyhole == 0))
		status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance,
			DMA_CTRL_DEVICE_ID, 0,
			XAXICDMA_KEYHOLE_WRITE,
			src_addr, dst_addr, size);
        else if ((cdma_write == 0) && (keyhole == 0))
		status = XAxiCdma_SimplePollExample(CfgPtr, &AxiCdmaInstance,
			DMA_CTRL_DEVICE_ID, 0,
			XAXICDMA_KEYHOLE_READ,
			src_addr, dst_addr, size);
        else
		printf("Error: Unknown CDMA transfer mode.\n");


	if (status != XST_SUCCESS) {
		printf("XAxiCdma_SgIntr Transfer Failed\r\n");
		return XST_FAILURE;
	}
	Hal_CdmaDeinit();

	return 0;
}

int cdma_clear(void)
{
    XAxiCdma_AresetPulseGenerate();

    return 0;
}

/**
 * @brief   Generates a pulse on GPIO971 (connected to EMIO11) that resets system peripheral hardware asynchronously.
 */
static void XAxiCdma_AresetPulseGenerate(void)
{    
    if (access("/sys/class/gpio/gpio971/value", F_OK) != 0)
    {
        system("echo 971 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo out > /sys/class/gpio/gpio971/direction");
        usleep(10 * 1000);
    }

    system("echo 1 > /sys/class/gpio/gpio971/value");
    usleep(10 * 1000);
    system("echo 0 > /sys/class/gpio/gpio971/value");
    usleep(100 * 1000);
}

/*! @} */

#endif /* !defined(VERSAL_PLATFORM) */
