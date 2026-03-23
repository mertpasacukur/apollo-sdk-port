#if !defined(VERSAL_PLATFORM)

/*!
 * @brief     Versal Apollo examples common function implementation.
 *
 *            Direct port of adi_versal_apollo_ex.c for Versal Standalone.
 *            Wires versal_ HAL functions into the Apollo SDK callback structures
 *            using the EXACT SAME struct-field-assignment pattern as Versal.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

/*============= I N C L U D E S ============*/
#include <stdlib.h>
#include "adi_apollo.h"
#include "adi_apollo_hal.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"
#include "versal_apollo_ex.h"
#include "versal_hal.h"
#include "versal_fw_provider.h"
#include "versal_config.h"
#include "xil_printf.h"

/*============= C O N F I G U R E   H A L ==================================*/

int32_t versal_apollo_ex_configure_hal(adi_apollo_device_t *apollo_device,
                                       adi_fpga_apollo_hal_config_t *versal_platform,
                                       uint8_t enable_hsci)
{
    adi_apollo_hal_t *hal = &apollo_device->hal_info;
    adi_apollo_hal_regio_spi_desc_t *spi0_desc = &hal->spi0_desc;
    adi_apollo_hal_regio_spi_desc_t *spi1_desc = &hal->spi1_desc;

    hal->dev_hal_info = versal_platform;
    hal->log_write    = &versal_log_write;
    hal->delay_us     = &versal_wait_us;
    hal->reset_pin_ctrl = &versal_hw_rst_pin_ctrl_apollo;

    /* SPI 0 Config (AD9084) */
    spi0_desc->is_used = 1;
    spi0_desc->spi_config.addr_inc = ADI_APOLLO_DEVICE_SPI_ADDR_INC_AUTO;
    spi0_desc->spi_config.msb = ADI_APOLLO_DEVICE_SPI_MSB_FIRST;
    spi0_desc->spi_config.sdo = ADI_APOLLO_DEVICE_SPI_SDO;
    spi0_desc->dev_obj    = versal_platform->spi0;
    spi0_desc->xfer       = NULL;
    spi0_desc->read       = &versal_spi_read;
    spi0_desc->write      = &versal_spi_write;
    spi0_desc->poll_read  = NULL;
    spi0_desc->rd_stream_en = 1;
    spi0_desc->wr_stream_en = 1;

    versal_apollo_hal_config_data(apollo_device, 0, 1);  /* chip selects: SPI0=0, SPI1=1 */

    /* SPI1 config — not used for Apollo device directly */
    spi1_desc->is_used = 0;

    /* HSCI — not used on Versal initially */
    if (enable_hsci) {
        xil_printf("WARNING: HSCI not implemented on Versal\r\n");
    }

    return API_CMS_ERROR_OK;
}

/*============= C O N F I G U R E   F P G A   H A L ========================*/

int32_t versal_apollo_ex_configure_fpga_hal(adi_fpga_apollo_device_t *fpga_device,
                                            adi_fpga_apollo_hal_config_t *versal_platform)
{
    adi_apollo_fpga_hal_t *hal = &fpga_device->hal_info;

    (void)versal_platform;

    /* Assign the Versal FPGA HAL implementations */
    hal->reg_read   = &versal_axi_reg_read32;
    hal->reg_write  = &versal_axi_reg_write32;
    hal->delay_us   = &versal_wait_us;
    hal->mem_write  = &versal_fpga_mem_write;
    hal->mem_read   = &versal_fpga_mem_read;

    return API_CMS_ERROR_OK;
}

/*============= C O N F I G U R E   S T A R T U P ==========================*/

int32_t versal_apollo_ex_configure_startup(adi_apollo_device_t *apollo_device)
{
    adi_apollo_startup_t *startup_info = &apollo_device->startup_info;

    /*
     * Configure the startup info with a QSPI-based FW provider.
     * This replaces the filesystem-based provider from Versal/Linux.
     */
    startup_info->fw_provider = versal_fw_provider_create(apollo_device, FW_IMAGES_DIR);
    startup_info->open  = versal_fw_provider_open;
    startup_info->close = versal_fw_provider_close;
    startup_info->get   = versal_fw_provider_get;

    return API_CMS_ERROR_OK;
}

/*============= S T A R T U P ===============================================*/

int32_t versal_apollo_ex_startup(adi_apollo_device_t *device, adi_apollo_top_t *dev_profile)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(dev_profile);

    xil_printf("Running Apollo startup sequence...\r\n");

    err = adi_apollo_startup_execute(device, dev_profile, ADI_APOLLO_STARTUP_SEQ_DEFAULT);
    ADI_APOLLO_ERROR_RETURN(err);

    /* Print version info */
    uint16_t maj, min, rc;
    if (adi_apollo_device_api_revision_get(device, &maj, &min, &rc) == API_CMS_ERROR_OK) {
        xil_printf("API ver: %d.%d.%d\r\n", maj, min, rc);
    }

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
