#if !defined(VERSAL_PLATFORM)
/*!
 * @brief     Analog Devices DMA API Source File
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_AXICDMA_API__
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
#include <pthread.h>
#include <unistd.h>
#include <sys/reboot.h>
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
#include <stdlib.h>
#include "axi_dmac.h"
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/
#define MZDDRADDR_0     0x3C000000
#define MZDDRADDR_1     0x3F900000
#define MZDDRADDR_2     0x3FB00000
#define FIFOADDR        0x30060000
#define HMCADDR         0x41000000
#ifdef ADS10
#define AXI_CDMA_0      0x60000000
#endif /* ADS10 */
#ifdef VCU128
#define AXI_CDMA_0      0x44A00000
#endif /* VCU128 */


/* This is the critical section object (statically allocated). */
static pthread_mutex_t axi_dmac_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static const char AXI_DMAC_DEFAULT_NAME[12] = "DMA_Default";

static void axi_dmac_aresetpulsegenerate(void);
static int32_t axi_dmac_read(struct axi_dmac *dmac, uint32_t reg_addr,
                      uint32_t *reg_data);
static int32_t axi_dmac_write(struct axi_dmac *dmac, uint32_t reg_addr,
                       uint32_t reg_data);
static int32_t axi_dmac_is_transfer_ready(struct axi_dmac *dmac, bool *rdy);
static int32_t axi_dmac_init(struct axi_dmac * *dmac_core,
                       uint32_t base_address);
static int32_t axi_dmac_remove(struct axi_dmac *dmac);

static int32_t axi_dmac_transfer_wait_completion(struct axi_dmac *dmac,
                                          uint32_t timeout_ms);
static void axi_dmac_transfer_stop(struct axi_dmac *dmac);

static void axi_dmac_readregisters(void);
static void axi_dmac_dbg_reg(void);

/*============= C O D E ====================*/

/*******************************************************************************
 * @brief Start a DMA transfer.
 *
 * @param dmac - DMAC istance.
 * @param dma_transfer - Structure containing transfer details.
 *
 * @return 0 for success, -1 in case of failure.
*******************************************************************************/

int32_t axi_dmac_transfer_start(uint32_t src_addr, uint32_t dst_addr, uint32_t size, uint8_t cdma_write, uint8_t keyhole)
{
    uint32_t reg_val, burst_size, err;
    struct axi_dmac *dmac;

    err = 0;

    if (size == 0)
        return 0; /* Nothing to do. */

    /* Enter the critical section -- other threads are locked out */
    pthread_mutex_lock(&axi_dmac_mutex);

    Hal_CdmaInit(AXI_CDMA_0, 0x1000);
    axi_dmac_init(&dmac, AXI_CDMA_0);

    /* Set current transfer parameters. */
    dmac->transfer.size = size;
    dmac->transfer.cyclic = false;
    dmac->transfer.dest_addr = dst_addr;
    dmac->transfer.src_addr = src_addr;

    dmac->remaining_size = size;
    dmac->next_dest_addr = dst_addr;
    dmac->next_src_addr =  src_addr;

    /* Enable DMA if not already enabled. */
    axi_dmac_read(dmac, AXI_DMAC_REG_CTRL, &reg_val);
    if (!(reg_val & AXI_DMAC_CTRL_ENABLE))
    {
        axi_dmac_write(dmac, AXI_DMAC_REG_CTRL, 0x0);
        axi_dmac_write(dmac, AXI_DMAC_REG_CTRL, AXI_DMAC_CTRL_ENABLE);
        axi_dmac_write(dmac, AXI_DMAC_REG_IRQ_MASK, 0x0);
    }

    axi_dmac_read(dmac, AXI_DMAC_REG_TRANSFER_SUBMIT, &reg_val);
    /* If we don't have a start of transfer then start compute
	 * values and trigger next transfer. */
    if (!(reg_val & AXI_DMAC_QUEUE_FULL))
    {
        switch (dmac->direction)
        {
        case DMA_DEV_TO_MEM:
            dmac->init_addr = dmac->next_dest_addr;
            axi_dmac_write(dmac, AXI_DMAC_REG_DEST_ADDRESS, dmac->next_dest_addr);
            axi_dmac_write(dmac, AXI_DMAC_REG_DEST_STRIDE, 0x0);
            break;
        case DMA_MEM_TO_DEV:
            dmac->init_addr = dmac->next_src_addr;
            axi_dmac_write(dmac, AXI_DMAC_REG_SRC_ADDRESS, dmac->next_src_addr);
            axi_dmac_write(dmac, AXI_DMAC_REG_SRC_STRIDE, 0x0);
            break;
        case DMA_MEM_TO_MEM:
            dmac->init_addr = dmac->next_src_addr;
            axi_dmac_write(dmac, AXI_DMAC_REG_DEST_ADDRESS, dmac->next_dest_addr);
            axi_dmac_write(dmac, AXI_DMAC_REG_SRC_ADDRESS, dmac->next_src_addr);
            break;
        default:
            err = -1;
            goto deinit; /* Other directions are not supported yet. */
        }

        /* Compute the burst size. */
        if (dmac->remaining_size > dmac->max_length)
        {
            burst_size = dmac->max_length;
        }
        else
        {
            burst_size = dmac->remaining_size - 1;
        }

        /* Compute next address (src or dest, or both, depending on type of transfer). */
        switch (dmac->direction)
        {
        case DMA_DEV_TO_MEM:
            dmac->next_dest_addr = dmac->next_dest_addr + (burst_size + 1);
            break;
        case DMA_MEM_TO_DEV:
            dmac->next_src_addr = dmac->next_src_addr + (burst_size + 1);
            break;
        case DMA_MEM_TO_MEM:
            dmac->next_dest_addr = dmac->next_dest_addr + (burst_size + 1);
            dmac->next_src_addr = dmac->next_src_addr + (burst_size + 1);
            break;
        default:
            err = -1;
            printf("%s[%u]: err = -1\n", __FILE__, __LINE__);
            goto deinit;
        }

        /* Compute remaining size. */
        dmac->remaining_size = dmac->remaining_size - (burst_size + 1);

        /* Specify the length of the transfer and trigger transfer. */
		axi_dmac_write(dmac, AXI_DMAC_REG_X_LENGTH, burst_size);

        axi_dmac_write(dmac, AXI_DMAC_REG_TRANSFER_SUBMIT, AXI_DMAC_TRANSFER_SUBMIT);
    }
    else
    {
        err = -1;
        axi_dmac_transfer_stop(dmac);
        goto deinit;
    }

    err = axi_dmac_transfer_wait_completion(dmac, 5000);
    if (err != 0)
        axi_dmac_transfer_stop(dmac);
deinit:

    Hal_CdmaDeinit();

    axi_dmac_remove(dmac);

    /*Leave the critical section -- other threads can now pthread_mutex_lock()  */
    pthread_mutex_unlock(&axi_dmac_mutex);

    return err;
}

/*******************************************************************************
 * @brief Wait for DMA transfer to be completed.
 *
 * @param dmac - DMAC istance.
 * @param timeout_ms - Number of ms to wait for completion of transfer.
 *
 * @return 0 for success, -1 in case trasnfer not completed in specified time.
*******************************************************************************/
static int32_t axi_dmac_transfer_wait_completion(struct axi_dmac *dmac,
                                          uint32_t timeout_ms)
{
    uint32_t timeout = 0;
    uint32_t reg_val = 0;

    if (dmac->irq_option == IRQ_ENABLED)
    {
        while (!dmac->transfer.transfer_done)
        {
            timeout++;
            delay_ms(1);
            if (timeout == timeout_ms)
            {
                printf("Error transferring data using DMA.\n");
                return -1;
            }
        }
    }
    else if (dmac->irq_option == IRQ_DISABLED)
    {
        axi_dmac_read(dmac, AXI_DMAC_REG_IRQ_PENDING, &reg_val);
        while (reg_val != (AXI_DMAC_IRQ_SOT | AXI_DMAC_IRQ_EOT))
        {
            timeout++;
            delay_ms(1);

            if (timeout >= timeout_ms)
            {
                printf("Error transferring data using DMA.\n");
                axi_dmac_dbg_reg();
                return -1;
            }
            axi_dmac_read(dmac, AXI_DMAC_REG_IRQ_PENDING, &reg_val);
        }
        axi_dmac_write(dmac, AXI_DMAC_REG_IRQ_PENDING,
                        (AXI_DMAC_IRQ_SOT | AXI_DMAC_IRQ_EOT));
    }

    return 0;
}

/*******************************************************************************
 * @brief Stop a DMA transfer.
 *
 * @param dmac - DMAC istance.
 *
 * @return None
*******************************************************************************/
static void axi_dmac_transfer_stop(struct axi_dmac *dmac)
{
    axi_dmac_write(dmac, AXI_DMAC_REG_CTRL, AXI_DMAC_CTRL_DISABLE);
}
/**
 * @brief   Generates a pulse on GPIO971 (connected to EMIO11) that resets system peripheral hardware asynchronously.
 */
static __maybe_unused void axi_dmac_aresetpulsegenerate(void)
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

/*******************************************************************************
 * @brief Wrapper for AXI IO through UIO/devmem read function.
 *
 * @param dmac - DMAC istance containing UIO index (/dev/uioX)/base address.
 * @param reg_addr - Address to read from.
 * @param reg_data - Location where read data will be stored.
 *
 * @return 0 - success.
*******************************************************************************/
static int32_t axi_dmac_read(struct axi_dmac *dmac,
                      uint32_t reg_addr,
                      uint32_t *reg_data)
{
    *reg_data = Hal_CdmaIn32(reg_addr);
    
    return 0;
}


/*******************************************************************************
 * @brief Wrapper for AXI IO through UIO/devmem wrrite function.
 *
 * @param dmac - DMAC istance containing UIO index (/dev/uioX)/base address.
 * @param reg_addr - Address to write to.
 * @param reg_data - Data to be written.
 *
 * @return 0 - success.
*******************************************************************************/
static int32_t axi_dmac_write(struct axi_dmac *dmac,
                       uint32_t reg_addr,
                       uint32_t reg_data)
{
    return (Hal_CdmaOut32(reg_addr, reg_data));
}

/*******************************************************************************
 * @brief Get transfer status (value of dmac->transfer.transfer_done flag).
 *
 * @note This function should not be used if DMA IRQ is not used.
 *
 * @param dmac - DMAC istance.
 * @param rdy - Transfer status (0 - not ready, 1 - ready).
 *
 * @return 0 - success.
*******************************************************************************/
static __maybe_unused int32_t axi_dmac_is_transfer_ready(struct axi_dmac *dmac, bool *rdy)
{
    *rdy = dmac->transfer.transfer_done;

    return 0;
}

/*******************************************************************************
 * @brief Get DMAC capabilities.
 *
 * @param dmac - DMAC istance.
 *
 * @return 0 for success, -1 in case of failure.
*******************************************************************************/
static int32_t axi_dmac_detect_caps(struct axi_dmac *dmac)
{
    uint32_t reg_val, initial_reg_val = 0;
    uint32_t src_mem_mapped = 0;
    uint32_t dest_mem_mapped = 0;
    uint32_t data = 0;

    dmac->max_length = -1;
    dmac->direction = INVALID_DIR;

    /* Check if HW cyclic possible */
    axi_dmac_read(dmac, AXI_DMAC_REG_FLAGS, &initial_reg_val);
    axi_dmac_write(dmac, AXI_DMAC_REG_FLAGS, DMA_CYCLIC);
    axi_dmac_read(dmac, AXI_DMAC_REG_FLAGS, &reg_val);
    if (reg_val == DMA_CYCLIC)
        dmac->hw_cyclic = true;
    /* Restore initial value for AXI_DMAC_REG_FLAGS register */
    axi_dmac_write(dmac, AXI_DMAC_REG_FLAGS, initial_reg_val);

    axi_dmac_read(dmac, AXI_DMAC_REG_X_LENGTH, &data);
    /* Get maximum burst size and set value. */
    axi_dmac_write(dmac, AXI_DMAC_REG_X_LENGTH, dmac->max_length);
    axi_dmac_read(dmac, AXI_DMAC_REG_X_LENGTH, &dmac->max_length);

    /* Get transfer direction and set value. */
    axi_dmac_write(dmac, AXI_DMAC_REG_DEST_ADDRESS, 0xffffffff);
    axi_dmac_read(dmac, AXI_DMAC_REG_DEST_ADDRESS, &dest_mem_mapped);
    axi_dmac_write(dmac, AXI_DMAC_REG_SRC_ADDRESS, 0xffffffff);
    axi_dmac_read(dmac, AXI_DMAC_REG_SRC_ADDRESS, &src_mem_mapped);

    if (dest_mem_mapped && !src_mem_mapped)
    {
        dmac->direction = DMA_DEV_TO_MEM;
    }
    else if (!dest_mem_mapped && src_mem_mapped)
    {
        dmac->direction = DMA_MEM_TO_DEV;
    }
    else if (dest_mem_mapped && src_mem_mapped)
    {
        dmac->direction = DMA_MEM_TO_MEM;
    }
    else
    {
        printf("Destination and source memory-mapped interfaces not supported.\n");
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * @brief Get transfer status (value of dmac->transfer.transfer_done flag).
 *
 * @param dmac_core - DMAC istance.
 * @param init - Structure containing initializing data.
 *
 * @return 0 for success, -1 in case of failure.
*******************************************************************************/
static int32_t axi_dmac_init(struct axi_dmac **dmac_core,
                      uint32_t base_address)
{
    struct axi_dmac *dmac;

    dmac = (struct axi_dmac *)calloc(1, sizeof(*dmac));
    if (!dmac)
        return -1;

    dmac->name = AXI_DMAC_DEFAULT_NAME;
    dmac->base = base_address;
    dmac->irq_option = IRQ_DISABLED;

    *dmac_core = dmac;

    int32_t status = axi_dmac_detect_caps(*dmac_core);
    if (status < 0)
        return -1;

    return 0;
}

/*******************************************************************************
 * @brief Remove the DMAC instance.
 *
 * @param dmac - DMAC istance.
 *
 * @return 0 for success, -1 in case of failure.
*******************************************************************************/
static int32_t axi_dmac_remove(struct axi_dmac *dmac)
{
    if (!dmac)
        return -1;

    free(dmac);

    return 0;
}

#define XAXICDMA_READREGISTERS_SIZE 25

const static u32 axi_dmac_readregisters_addr[XAXICDMA_READREGISTERS_SIZE] =
{
    AXI_DMAC_VERSION_OFFSET,
    AXI_DMAC_PERIPHERAL_ID_OFFSET,
    AXI_DMAC_SCRATCH_OFFSET,
    AXI_DMAC_IDENTIFICATION_OFFSET,
    AXI_DMAC_INTERFACE_DESCRIPTION_OFFSET,
    AXI_DMAC_IQR_MASK_OFFSET,
    AXI_DMAC_IRQ_PENDING_OFFSET,
    AXI_DMAC_IRQ_SOURCE_OFFSET,
    AXI_DMAC_CONTROL_OFFSET,
    AXI_DMAC_TRANSFER_ID_OFFSET,
    AXI_DMAC_TRANSFER_SUBMIT_OFFSET,
    AXI_DMAC_FLAGS_OFFSET,
    AXI_DMAC_DEST_ADDRESS_OFFSET,
    AXI_DMAC_SRC_ADDRESS_OFFSET,
    AXI_DMAC_X_LENGTH_OFFSET,
    AXI_DMAC_Y_LENGTH_OFFSET,
    AXI_DMAC_DEST_STRIDE_OFFSET,
    AXI_DMAC_SRC_STRIDE_OFFSET,
    AXI_DMAC_TRANSFER_DONE_OFFSET,
    AXI_DMAC_ACTIVE_TRANSFER_ID_OFFSET,
    AXI_DMAC_CURRENT_SRC_ADDRESS_OFFSET,
    AXI_DMAC_CURRENT_DST_ADDRESS_OFFSET,
    AXI_DMAC_TRANSFER_PROGESS_OFFSET,
    AXI_DMAC_DEST_ADDRESS_HIGH_OFFSET,
    AXI_DMAC_SRC_ADDRESS_HIGH_OFFSET,
};

static __maybe_unused void axi_dmac_readregisters(void)
{
    u16 pos;
    u32 addr_offset, value_read;

    printf("\n--------------------------\nAXI DMA Registers Value:\n--------------------------\n");
    for (pos = 0; pos < XAXICDMA_READREGISTERS_SIZE; pos++)
    {
        addr_offset = axi_dmac_readregisters_addr[pos];
		axi_dmac_read(NULL, addr_offset, &value_read);
        printf("Register 0x%04X: 0x%08X ", addr_offset, value_read);
		if ((pos + 1) % 3  == 0) printf("\n");
    }
}

static void axi_dmac_dbg_reg(void)
{
    u32 Transfer_Progress, Active_Transfer_Id, Current_Src_Address,
        Current_Dest_Address, Transfer_Id, Transfer_Submit, Transfer_Done, 
        Irq_Source;

    axi_dmac_read(NULL, AXI_DMAC_TRANSFER_PROGESS_OFFSET, &Transfer_Progress);
    axi_dmac_read(NULL, AXI_DMAC_TRANSFER_ID_OFFSET, &Transfer_Id);
    axi_dmac_read(NULL, AXI_DMAC_ACTIVE_TRANSFER_ID_OFFSET, &Active_Transfer_Id);
    axi_dmac_read(NULL, AXI_DMAC_CURRENT_SRC_ADDRESS_OFFSET, &Current_Src_Address);
    axi_dmac_read(NULL, AXI_DMAC_CURRENT_DST_ADDRESS_OFFSET, &Current_Dest_Address);
    axi_dmac_read(NULL, AXI_DMAC_TRANSFER_SUBMIT_OFFSET, &Transfer_Submit);
    axi_dmac_read(NULL, AXI_DMAC_TRANSFER_DONE_OFFSET, &Transfer_Done);
    axi_dmac_read(NULL, AXI_DMAC_IRQ_SOURCE_OFFSET, &Irq_Source);

    printf("XAxiCdma_IsBusy. TRANSFER_PROGESS: 0x%08X, TRANSFER_ID: 0x%08X, ACTIVE_TRANSFER_ID: 0x%08X, CURRENT_SRC_ADDRESS: 0x%08X, CURRENT_DST_ADDRESS: 0x%08X , TRANSFER_SUBMIT: 0x%08X, TRANSFER_DONE: 0x%08X, IRQ_SOURCE: 0x%08X\n",
           Transfer_Progress, Transfer_Id, Active_Transfer_Id, Current_Src_Address,
           Current_Dest_Address, Transfer_Submit, Transfer_Done, Irq_Source);
}

/*! @} */

#endif /* !defined(VERSAL_PLATFORM) */
