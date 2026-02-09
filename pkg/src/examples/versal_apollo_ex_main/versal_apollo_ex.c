/*!
 * @brief     Versal Apollo example common functions implementation.
 *            Ported from adi_ads10_apollo_ex.c — HAL wiring for Versal platform.
 *
 * @note      All API calls preserved exactly from original.
 *            Only platform HAL function pointers changed.
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

/*============= I M P L E M E N T A T I O N ==============*/

int32_t versal_apollo_ex_configure_hal(adi_apollo_device_t *apollo_device,
                                        versal_hal_config_t *versal_platform)
{
    adi_apollo_hal_t *hal = &apollo_device->hal_info;
    adi_apollo_hal_regio_spi_desc_t *spi0_desc = &hal->spi0_desc;
    adi_apollo_hal_regio_spi_desc_t *spi1_desc = &hal->spi1_desc;

    /* Assign platform data */
    hal->dev_hal_info = versal_platform;

    /* Wire up platform callbacks — Versal implementations */
    hal->log_write       = &versal_log_write;
    hal->delay_us        = &versal_wait_us;
    hal->reset_pin_ctrl  = &versal_hw_rst_pin_ctrl_apollo;

    /* SPI 0 Config — Apollo (dedicated SPI instance 0) */
    spi0_desc->is_used = 1;
    spi0_desc->spi_config.addr_inc = ADI_APOLLO_DEVICE_SPI_ADDR_INC_AUTO;
    spi0_desc->spi_config.msb = ADI_APOLLO_DEVICE_SPI_MSB_FIRST;
    spi0_desc->spi_config.sdo = ADI_APOLLO_DEVICE_SPI_SDO;
    spi0_desc->dev_obj = versal_platform->spi0;
    spi0_desc->xfer = NULL;
    spi0_desc->read = &versal_spi_read;
    spi0_desc->write = &versal_spi_write;
    spi0_desc->poll_read = NULL;
    spi0_desc->rd_stream_en = 1;
    spi0_desc->wr_stream_en = 1;

    /* Configure SPI chip selects: SPI0 CS=0 (Apollo), SPI1 not used for Apollo */
    versal_apollo_hal_config_data(apollo_device, 0, 0);

    /* SPI1 — not used for Apollo register access (used for clock chips separately) */
    spi1_desc->is_used = 0;

    /* HSCI — not used on Versal */
    hal->hsci_desc.is_used = 0;

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_apollo_ex_configure_startup(adi_apollo_device_t *apollo_device)
{
    adi_apollo_startup_t *startup_info = &apollo_device->startup_info;

    /*
     * Create the QSPI flash-based FW provider.
     * This replaces the ADS10's file-based provider.
     */
    startup_info->fw_provider = versal_fw_provider_create(apollo_device);
    startup_info->open  = versal_fw_provider_open;
    startup_info->close = versal_fw_provider_close;
    startup_info->get   = versal_fw_provider_get;

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_apollo_ex_configure_fpga_hal(adi_fpga_apollo_device_t *fpga_device,
                                             versal_hal_config_t *versal_platform)
{
    adi_apollo_fpga_hal_t *hal = &fpga_device->hal_info;

    /* Assign Versal FPGA register access implementations */
    hal->reg_read  = &versal_axi_reg_read32;
    hal->reg_write = &versal_axi_reg_write32;
    hal->delay_us  = &versal_wait_us;
    hal->mem_write = &versal_fpga_mem_write;
    hal->mem_read  = &versal_fpga_mem_read;

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_apollo_ex_startup(adi_apollo_device_t *device, adi_apollo_top_t *dev_profile)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(dev_profile);

    /* Execute startup — SAME API call as ADS10, completely unchanged */
    err = adi_apollo_startup_execute(device, dev_profile, ADI_APOLLO_STARTUP_SEQ_DEFAULT);
    ADI_APOLLO_ERROR_RETURN(err);

    /* Print version info */
    uint16_t maj, min, rc;
    err = adi_apollo_device_api_revision_get(device, &maj, &min, &rc);
    if (err == API_CMS_ERROR_OK) {
        xil_printf("Apollo API version: %d.%d.%d\r\n", maj, min, rc);
    }

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_apollo_ex_fpga_pre_reset(adi_fpga_apollo_device_t *fpga_device)
{
    /* Same as ADS10 — stop play/capture to reduce power before reset */
    /* TODO: Adapt FPGA register offsets if Versal FPGA design differs */

    /* These calls use the FPGA HAL we wired up above — platform independent */
    /* adi_fpga_apollo_private_write32_bitfield(fpga_device, SEQ_CTRL_2, SEQ_FIRST_TRIG_CNT_MASK, 1); */
    /* adi_fpga_apollo_private_write32_bitfield(fpga_device, SEQ_CTRL_2, SEQ_SECOND_TRIG_CNT_MASK, 1); */
    /* adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set(fpga_device, 0); */

    /* NOTE: Commented out because the FPGA design on Versal may have different
     * register layout. Uncomment when FPGA design is confirmed to match ADS10. */

    return API_CMS_ERROR_OK;
}
