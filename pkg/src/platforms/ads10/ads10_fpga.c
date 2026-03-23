#if !defined(VERSAL_PLATFORM)

/*!
 * @brief    ADS10/FPGA API
 *
 * @copyright copyright(c) 2021 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "ads10_fpga.h"
#include "adi_fpga_jesd_registers.h"
#include "ads10_hal.h"
#include "adi_fpga_apollo_gpio_types.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef ADS10
#include "cdmaapi.h"
#endif /* ADS10 */
#ifdef VCU128
#include "axi_dmac.h"
#endif /* VCU128 */

/*============= D E F I N E S ==============*/
#define DMA_1MSIZE             0x100000
#define DMA_128KSIZE           0x020000
#define DMA_MAXSIZE            DMA_128KSIZE

#ifdef ADS10
	#define DMA_TRANSFER   CdmaTransfer
	/* The address used by DMA engine to access the FIFO through axi_addr_trans_0
	* address translator
	* This is the default addrress for ARMv7 architecture (e.g. microZed) */
	#define FPGA_FIFO_ADDR         0x80060000
	#define MZ_DDR_DMA_ADDR        0x3F800000
#endif /* ADS10 */

#ifdef VCU128
	#define DMA_TRANSFER     axi_dmac_transfer_start
	/* The real FIFO Address - it is stored in the DATA_FIFO_BASE_ADDRESS FPGA
	* register (e.g. offset 0x0912)
	* There is no address translation on VCU128 (aka MIcroblaze) */
	#define FPGA_FIFO_ADDR         0x30100000
	#define MZ_DDR_DMA_ADDR        0xFC800000

#endif /* VCU128 */



static int32_t ads10_fpga_dma_transfer(uint8_t* data, uint32_t length,
				       uint8_t fpga_write);


/*============= C O D E ====================*/
int32_t ads10_fpga_mem_write(uint32_t mem_addr, uint32_t num_bytes,
			     uint8_t *raw_bytes)
{

	int32_t  err = API_CMS_ERROR_OK;
	uint8_t  fpga_write = 1;

	ADI_CMS_NULL_PTR_CHECK(raw_bytes);

	uint32_t data = 0x0;
	ads10_axi_reg_write32(GT_TX_PATTERN_ADDR, mem_addr);
	ads10_axi_reg_write32(GT_TX_PATTERN_LEN, num_bytes);
	/* Set GT_TX_DATA_MODE to 0 */
	ads10_axi_reg_read32(GT_TX_PATTERN_CTRL, &data);
	data &= (~GT_TX_DATA_MODE_MASK);
	ads10_axi_reg_write32(GT_TX_PATTERN_CTRL, data);
	/* Set GT_TX_PTN_LOAD_START to 1 */
	ads10_axi_reg_read32(GT_TX_PATTERN_LOAD_CTRL, &data);
	data |= GT_TX_PTN_LOAD_START_MASK;
	ads10_axi_reg_write32(GT_TX_PATTERN_LOAD_CTRL, data);

	err = ads10_fpga_dma_transfer(raw_bytes, num_bytes, fpga_write);
	ADI_CMS_ERROR_RETURN(err);

	return err;
}


int32_t ads10_fpga_mem_read(uint32_t mem_addr, uint32_t num_cap_bytes,
			    uint8_t *cap_buff)
{
	int32_t  err = API_CMS_ERROR_OK ;
	uint8_t  fpga_write = 0;

	ADI_CMS_NULL_PTR_CHECK(cap_buff);

	err = ads10_fpga_dma_transfer(cap_buff, num_cap_bytes, fpga_write);
	ADI_CMS_ERROR_RETURN(err);

	if (err != API_CMS_ERROR_OK) {
		printf("%s(): ads10_fpga_dma_transfer() failed with error code: %d\n",
		       __func__, err);
	}

	return err;
}

int32_t ads10_fpga_fmcb_aux_gpio_write(void* user_data, uint32_t gpio_index, uint8_t n_gpio_state)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t reg_addr = 0;
    uint32_t regRd = 0;
    uint32_t regWr = 0;
    uint8_t bit_index = 0;
	uint8_t gpio_state;

	gpio_state = (n_gpio_state) ? 0 : 1;
    bit_index = (gpio_index % 32);

    // Read GPIO Direction register to get current set direction.
    reg_addr = REG_GPIO_D_2_ADDR;
    err = ads10_axi_reg_read32(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIO at gpio_index as Output and other GPIOs as is.
    regWr = regRd | (1 << bit_index);
    err = ads10_axi_reg_write32(reg_addr, regWr);
    ADI_CMS_ERROR_RETURN(err);

    // Read present GPIO Write register.
    reg_addr = REG_GPIO_W_2_ADDR;
    err = ads10_axi_reg_read32(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIO State of gpio_index and other GPIOs as is.
    if (gpio_state == 1)
        regWr = regRd | (1 << bit_index);
    else
        regWr = regRd & ~(1 << bit_index);

    err = ads10_axi_reg_write32(reg_addr, regWr);
    ADI_CMS_ERROR_RETURN(err);

    reg_addr = REG_GPIO_W_2_ADDR;
    err = ads10_axi_reg_read32(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    reg_addr = REG_GPIO_D_2_ADDR;
    err = ads10_axi_reg_read32(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t ads10_fpga_fmcb_aux_spi2_sdo_alt_enable_set(void* user_data, uint32_t dummy_var, uint8_t value)
{
	ADI_CMS_RANGE_CHECK(value, 0, 1);
	uint32_t regRead = 0;
	uint32_t regWrite = 0;

    ads10_axi_reg_read32(SPI_MASTER_CTRL, &regRead);
    regRead = regRead & 0xFFFFEFFF;
    /* fmcb_gpio_sdo_en[12]: 0 : SDO pin set to SPI2 SDO  1 : SDO pin set to AUX_GPIO_3 */
	regWrite = (regRead | (value << 12));
	ads10_axi_reg_write32(SPI_MASTER_CTRL, regWrite);

    return API_CMS_ERROR_OK;
}

static int32_t ads10_fpga_dma_transfer(uint8_t* data, uint32_t length,
				       uint8_t fpga_write)
{
	int32_t err = API_CMS_ERROR_OK;
	uint32_t MZDDRAddr = MZ_DDR_DMA_ADDR;
	uint32_t FIFOAddr = FPGA_FIFO_ADDR;
	uint32_t mmap_size = DMA_MAXSIZE;
	int32_t  fd;
	uint32_t rd_last, rd_loop;
	uint8_t  cdma_write = fpga_write ? 1 : 0;
	uint8_t  keyhole = 0;
	uint32_t dma_src_addr = fpga_write ? MZDDRAddr : FIFOAddr;
	uint32_t dma_dst_addr = fpga_write ? FIFOAddr : MZDDRAddr;
	void *mem_base = NULL;

	ADI_CMS_NULL_PTR_CHECK(data);

	if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
		ADI_CMS_ERROR_RETURN(API_CMS_ERROR_DEV_MEM_OPEN);
	}

	mem_base = mmap(0, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, MZDDRAddr);
	close(fd);

	if (mem_base == (void*) - 1)
    {
        printf("%s(): DMA Memory Map error: %s.\n",
               __func__, strerror(errno));
        ADI_CMS_ERROR_GOTO(API_CMS_ERROR_MMAP, end);
    }

	rd_loop = length / DMA_MAXSIZE;
	rd_last = length % DMA_MAXSIZE;

	uint32_t offset = 0;
	int i;
	for (i = 0; i <= rd_loop; i++)
    {
		uint32_t count = (i == rd_loop) ? rd_last : DMA_MAXSIZE;
        if (cdma_write) {
            memcpy((uint8_t *)mem_base, (data + offset), count);
		}
		err = DMA_TRANSFER(dma_src_addr, dma_dst_addr, count,
			cdma_write, keyhole);
		ADI_CMS_ERROR_GOTO(err, end_unmap);
		if (!cdma_write)
            memcpy((data + offset), (uint8_t *)mem_base, count);

        offset += count;
	}
end_unmap:
    munmap(mem_base, mmap_size);
end:
	return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
