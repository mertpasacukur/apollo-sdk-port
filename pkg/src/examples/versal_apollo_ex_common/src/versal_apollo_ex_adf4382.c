/*!
 * \brief     Versal Apollo examples common ADF4382 functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "xil_printf.h"

#include "versal_apollo_ex_adf4382.h"
#include "adi_apollo_mcs_cal_types.h"
#include "versal_hal.h"


int32_t versal_apollo_ex_adf4382_hal_config(adi_adf4382_device_t *adf4382, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn)
{
    int32_t err;
    uint8_t spi_wire_mode;
    adi_adf4382_hal_t *hal = &adf4382->hal_info;
    hal_spi_config_t *spi_cfg;

    ADI_CMS_NULL_PTR_CHECK(adf4382);

    err = versal_adf4382_hal_config_data(adf4382, &versal_adf4382_hal_spi_config_init);
    ADI_CMS_ERROR_RETURN(err);

    hal->delay_us = &versal_wait_us;
    hal->spi_read = &versal_cms_spi_read;
    hal->spi_write = &versal_cms_spi_write;
    spi_cfg = (hal_spi_config_t *)hal->user_data;

    /* Set ADF4382 SPI mode */
    spi_wire_mode = (spi_cfg->spi_3wire == 0) ? 0x18 : 0x00;
    err = adi_adf4382_core_spi_reg_set(adf4382, 0x00, spi_wire_mode);
    ADI_CMS_ERROR_RETURN(err);

    /* Verify SPI read is functional */
    err = adi_adf4382_core_spi_reg_test(adf4382);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: ADF4382 SPI Reg Test Failed! err=%d\r\n", err);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t versal_apollo_ex_adf4382_startup(adi_adf4382_device_t *adf4382,
                                            uint64_t rfout_freq_hz, uint64_t ref_freq_hz)
{
    int32_t err;
    adi_adf4382_clkctrl_config_t clkctrl_config = {
        .en_adcclk = 1,
        .en_dnclk = 1,
        .en_drclk = 1,
    };

    adi_adf4382_rfout_output_config_t rfout_config = {
        .ref_freq_hz = ref_freq_hz,
        .en_doubler  = 1,
        .r_div       = 1,
        .ld_count    = 14,
        .clk1_opwr   = 5,
        .clk2_opwr   = 9,
        .rfout_freq_hz = rfout_freq_hz,
    };

    if (err = versal_apollo_ex_adf4382_hal_config(adf4382, NULL, NULL), err != API_CMS_ERROR_OK) {
        xil_printf("Unable to configure ADF4382 HAL: %d\r\n", err);
        return err;
    }

    if (err = adi_adf4382_core_init(adf4382), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_core_init(): %d\r\n", err);
        return err;
    }

    if (err = adi_adf4382_clkctrl_config_set(adf4382, &clkctrl_config), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_clkctrl_config_set(): %d\r\n", err);
        return err;
    }

    if (err = adi_adf4382_rfout_output_set(adf4382, &rfout_config), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_rfout_output_set(): %d\r\n", err);
        return err;
    }

    if (err = adi_adf4382_pdctrl_pd_set(adf4382, ADI_ADF4382_PDCTRL_TARGET_ALL
                                               | ADI_ADF4382_PDCTRL_TARGET_CLKOUT1
                                               | ADI_ADF4382_PDCTRL_TARGET_CLKOUT2, 0),
        err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_pdctrl_pd_set(): %d\r\n", err);
        return err;
    }

    /* Query lock status (timeout = 500ms) */
    if (err = adi_adf4382_ldctrl_lock_wait(adf4382, 500000), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_ldctrl_lock_wait(): %d\r\n", err);
        return err;
    }
    return API_CMS_ERROR_OK;
}

int32_t versal_apollo_ex_adf4382_mcs_init_config(adi_adf4382_device_t *adf4382,
                                                    adi_apollo_ex_adf4382_mcs_init_config_t *adf4382_mcs_init_config,
                                                    uint8_t *phase_adj_rb)
{
    int32_t err = API_CMS_ERROR_OK;

    if (err = adi_adf4382_cpctrl_bleed_word_set(adf4382, &adf4382_mcs_init_config->bleed_word_config), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_cpctrl_bleed_word_set(). err: %d\r\n", err);
        goto end;
    }

    if (err = adi_adf4382_cpctrl_bleed_polarity_set(adf4382, adf4382_mcs_init_config->bleed_pol), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_cpctrl_bleed_polarity_set(). err: %d\r\n", err);
        goto end;
    }

    if (err = adi_adf4382_bf___REG0032___BLEED_ADJ_CAL_set(adf4382, adf4382_mcs_init_config->bleed_adj_cal), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_bf___REG0032___BLEED_ADJ_CAL_set(). err: %d\r\n", err);
        goto end;
    }

    if (err = adi_adf4382_phase_adjust_config_set(adf4382, &adf4382_mcs_init_config->phase_adj_config, phase_adj_rb), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_phase_adjust_config_set(). err: %d\r\n", err);
        goto end;
    }

    if (err = adi_adf4382_cpctrl_en_bleed_set(adf4382, adf4382_mcs_init_config->en_bleed), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_cpctrl_en_bleed_set(). err: %d\r\n", err);
        goto end;
    }

    if (err = adi_adf4382_phase_adjust_resync_enable(adf4382, adf4382_mcs_init_config->en_phase_resync), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_phase_adjust_resync_enable(). err: %d\r\n", err);
        goto end;
    }

    if (err = adi_adf4382_phase_adjust_auto_align_enable(adf4382, adf4382_mcs_init_config->en_auto_align), err != API_CMS_ERROR_OK) {
        xil_printf("Error in adi_adf4382_phase_adjust_auto_align_enable(). err: %d\r\n", err);
        goto end;
    }

end:
    return err;
}

int32_t versal_apollo_ex_adf4382_del_cnt_get(adi_adf4382_device_t *adf4382, uint8_t *bleed_pol, int8_t *coarse_current, int16_t *fine_current)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t reg_read = 0;
    uint16_t del_cnt_raw = 0;

    err = adi_adf4382_core_spi_reg_get(adf4382, 0x064, &reg_read);
    ADI_CMS_ERROR_RETURN(err);
    del_cnt_raw = reg_read;

    err = adi_adf4382_core_spi_reg_get(adf4382, 0x065, &reg_read);
    ADI_CMS_ERROR_RETURN(err);
    del_cnt_raw |= (reg_read << 8);

    *fine_current = del_cnt_raw & 0x01FF;
    *coarse_current = (del_cnt_raw & 0x1E00) >> 9;
    *bleed_pol = del_cnt_raw >> 15;

    xil_printf("ADF4382 DEL_CNT: --> \t");
    xil_printf("bleed_pol: %d. \t", *bleed_pol);
    xil_printf("current_coarse: %d. \t", *coarse_current);
    xil_printf("current_fine: %d. \t", *fine_current);
    xil_printf("Raw: %d. \r\n", del_cnt_raw);

    return API_CMS_ERROR_OK;
}
