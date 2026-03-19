/*!
 * @brief     Analog Devices DMA API Header File
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

#ifndef AXI_DMAC_H
#define AXI_DMAC_H

/*============= I N C L U D E S ============*/
#include <stdbool.h>

/*============= D E F I N E S ==============*/
#define DDRTOHMC               1
#define HMCTODDR               2
#define DDRTOHMC_ALT           3
#define DMA_1MSIZE             0x100000
#define DMA_128KSIZE           0x020000
#define DMA_MAXSIZE            DMA_128KSIZE

#define NO_OS_BIT(x) (1 << (x))
#define NO_OS_GENMASK(h, l) ({                    \
    uint32_t t = (uint32_t)(~0UL);                \
    t = t << (NO_OS_BITS_PER_LONG - (h - l + 1)); \
    t = t >> (NO_OS_BITS_PER_LONG - (h + 1));     \
    t;                                            \
})

#define AXI_DMAC_REG_IRQ_MASK 0x80
#define AXI_DMAC_REG_IRQ_PENDING 0x84
#define AXI_DMAC_IRQ_SOT NO_OS_BIT(0)
#define AXI_DMAC_IRQ_EOT NO_OS_BIT(1)

#define AXI_DMAC_REG_INTF_DESC 0x010
#define AXI_DMAC_DMA_TYPE_DEST NO_OS_GENMASK(5, 4)
#define AXI_DMAC_DMA_TYPE_SRC NO_OS_GENMASK(13, 12)
//Define macro for src and dest

#define AXI_DMAC_REG_CTRL 0x400
#define AXI_DMAC_CTRL_ENABLE NO_OS_BIT(0)
#define AXI_DMAC_CTRL_DISABLE 0u
#define AXI_DMAC_CTRL_PAUSE NO_OS_BIT(1)

#define AXI_DMAC_REG_TRANSFER_ID 0x404
#define AXI_DMAC_REG_TRANSFER_SUBMIT 0x408
#define AXI_DMAC_TRANSFER_SUBMIT NO_OS_BIT(0)
#define AXI_DMAC_QUEUE_FULL NO_OS_BIT(0)
#define AXI_DMAC_REG_FLAGS 0x40c
#define AXI_DMAC_REG_DEST_ADDRESS 0x410
#define AXI_DMAC_REG_SRC_ADDRESS 0x414
#define AXI_DMAC_REG_X_LENGTH 0x418
#define AXI_DMAC_REG_Y_LENGTH 0x41c
#define AXI_DMAC_REG_DEST_STRIDE 0x420
#define AXI_DMAC_REG_SRC_STRIDE 0x424
#define AXI_DMAC_REG_TRANSFER_DONE 0x428

#define AXI_DMAC_BUS_TYPE_AXI_MM 0
#define AXI_DMAC_BUS_TYPE_AXI_STREAM 1
#define AXI_DMAC_BUS_TYPE_FIFO 2

#define AXI_DMAC_REG_IRQ_MASK 0x80
#define AXI_DMAC_REG_IRQ_PENDING 0x84
#define AXI_DMAC_IRQ_SOT NO_OS_BIT(0)
#define AXI_DMAC_IRQ_EOT NO_OS_BIT(1)

#define AXI_DMAC_REG_INTF_DESC 0x010
#define AXI_DMAC_DMA_TYPE_DEST NO_OS_GENMASK(5, 4)
#define AXI_DMAC_DMA_TYPE_SRC NO_OS_GENMASK(13, 12)
//Define macro for src and dest

#define AXI_DMAC_REG_CTRL 0x400
#define AXI_DMAC_CTRL_ENABLE NO_OS_BIT(0)
#define AXI_DMAC_CTRL_DISABLE 0u
#define AXI_DMAC_CTRL_PAUSE NO_OS_BIT(1)

#define AXI_DMAC_REG_TRANSFER_ID 0x404
#define AXI_DMAC_REG_TRANSFER_SUBMIT 0x408
#define AXI_DMAC_TRANSFER_SUBMIT NO_OS_BIT(0)
#define AXI_DMAC_QUEUE_FULL NO_OS_BIT(0)
#define AXI_DMAC_REG_FLAGS 0x40c
#define AXI_DMAC_REG_DEST_ADDRESS 0x410
#define AXI_DMAC_REG_SRC_ADDRESS 0x414
#define AXI_DMAC_REG_X_LENGTH 0x418
#define AXI_DMAC_REG_Y_LENGTH 0x41c
#define AXI_DMAC_REG_DEST_STRIDE 0x420
#define AXI_DMAC_REG_SRC_STRIDE 0x424
#define AXI_DMAC_REG_TRANSFER_DONE 0x428

/** @name Register offset definitions
 *   Register accesses are 32-bit.
 * @{
 */
#define AXI_DMAC_VERSION_OFFSET 0x00000000
#define AXI_DMAC_PERIPHERAL_ID_OFFSET 0x00000004
#define AXI_DMAC_SCRATCH_OFFSET 0x00000008
#define AXI_DMAC_IDENTIFICATION_OFFSET 0x0000000C
#define AXI_DMAC_INTERFACE_DESCRIPTION_OFFSET 0x00000010
#define AXI_DMAC_IQR_MASK_OFFSET 0x00000080
#define AXI_DMAC_IRQ_PENDING_OFFSET 0x00000084
#define AXI_DMAC_IRQ_SOURCE_OFFSET 0x00000088
#define AXI_DMAC_CONTROL_OFFSET 0x00000400
#define AXI_DMAC_TRANSFER_ID_OFFSET 0x00000404
#define AXI_DMAC_TRANSFER_SUBMIT_OFFSET 0x00000408
#define AXI_DMAC_FLAGS_OFFSET 0x0000040C
#define AXI_DMAC_DEST_ADDRESS_OFFSET 0x00000410
#define AXI_DMAC_SRC_ADDRESS_OFFSET 0x00000414
#define AXI_DMAC_X_LENGTH_OFFSET 0x00000418
#define AXI_DMAC_Y_LENGTH_OFFSET 0x0000041C
#define AXI_DMAC_DEST_STRIDE_OFFSET 0x00000420
#define AXI_DMAC_SRC_STRIDE_OFFSET 0x00000424
#define AXI_DMAC_TRANSFER_DONE_OFFSET 0x00000428
#define AXI_DMAC_ACTIVE_TRANSFER_ID_OFFSET 0x0000042C
#define AXI_DMAC_CURRENT_SRC_ADDRESS_OFFSET 0x00000438
#define AXI_DMAC_CURRENT_DST_ADDRESS_OFFSET 0x00000434
#define AXI_DMAC_TRANSFER_PROGESS_OFFSET 0x00000448
#define AXI_DMAC_DEST_ADDRESS_HIGH_OFFSET 0x00000490
#define AXI_DMAC_SRC_ADDRESS_HIGH_OFFSET 0x00000494

/*@}*/


/*============= E X P O R T S ==============*/

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
enum use_irq
{
    IRQ_DISABLED = 0,
    IRQ_ENABLED = 1
};

enum dma_direction
{
    INVALID_DIR = 0,
    DMA_DEV_TO_MEM = 1,
    DMA_MEM_TO_DEV = 2,
    DMA_MEM_TO_MEM = 3
};

enum dma_flags
{
    DMA_CYCLIC = 1,
    DMA_LAST = 2,
    DMA_PARTIAL_REPORTING_EN = 4
};

// Could be transformed to a bool
enum cyclic_transfer
{
    NO = 0,
    CYCLIC = 1
};

struct axi_dma_transfer
{
    uint32_t size;
    volatile bool transfer_done;
    enum cyclic_transfer cyclic;
    uint32_t src_addr;
    uint32_t dest_addr;
};

struct axi_dmac
{
    const char *name;
    uint32_t base;
    enum use_irq irq_option;
    enum dma_direction direction;
    bool hw_cyclic;
    uint32_t max_length;
    volatile struct axi_dma_transfer transfer;
    //Current sub-transfer properties
    uint32_t init_addr;
    uint32_t remaining_size;
    uint32_t next_src_addr;
    uint32_t next_dest_addr;
};

struct axi_dmac_init
{
    const char *name;
    uint32_t base;
    enum use_irq irq_option;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

int32_t axi_dmac_transfer_start(uint32_t src_addr, uint32_t dst_addr, uint32_t size, uint8_t cdma_write, uint8_t keyhole);


#endif

/*! @} */