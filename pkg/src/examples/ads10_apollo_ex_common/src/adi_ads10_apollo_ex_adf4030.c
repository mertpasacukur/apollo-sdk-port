/*!
 * \brief     ADS10 Apollo examples common ADF4030 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "math.h"
#include "adi_ads10_apollo_ex_adf4030.h"
#include "adi_ads10_apollo_ex_types.h"
#include "ads10_hal.h"


/*==================== P U B L I C   A P I   C O D E ====================*/

int32_t adi_ads10_apollo_ex_adf4030_configure_hal(adi_adf4030_device_t *adf4030, uint8_t dev_id)
{
    int32_t err = 0;
    adi_adf4030_hal_t *hal = &adf4030->hal_info;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_RANGE_CHECK(dev_id, ADF4030_0, ADF4030_F);

    err = ads10_adf4030_hal_config_data(adf4030, &ads10_adf4030_hal_spi_config_init);
    ADI_CMS_ERROR_RETURN(err);

    hal->delay_us = &ads10_wait_us;
    hal->spi_read = &ads10_cms_spi_read;
    hal->spi_write = &ads10_cms_spi_write;

    // Set device id based off of unique HW addr pins.
    adf4030->dev_id = (adi_adf4030_dev_id_e) dev_id;

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_adf4030_vco_freq_calc(uint64_t ref_input_freq, uint64_t bsync_out_freq, uint64_t *vco_out_freq)
{
    int32_t err = API_CMS_ERROR_OK;
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t o_div_idx = 0;
    uint16_t r_div_idx = 0;

    uint16_t o_div_arr_len = floor(ADI_ADF4030_VCO_FREQ_MAX/bsync_out_freq) - ceil(ADI_ADF4030_VCO_FREQ_MIN/bsync_out_freq) + 1;
    uint16_t r_div_arr_len = floor(ref_input_freq/ADI_ADF4030_PFD_FREQ_MIN) - ceil(ref_input_freq/ADI_ADF4030_PFD_FREQ_MAX) + 1;
    uint16_t o_div_arr[o_div_arr_len];
    uint16_t r_div_arr[r_div_arr_len];
    double n_div = 0;
    // uint16_t o_div = 0;
    uint16_t n_div_optimal = ADI_ADF4030_N_DIV_MAX;
    uint64_t vco_freq = 0;
    uint8_t vco_out_found = 0;

    if ((ADI_ADF4030_VCO_FREQ_TYP % bsync_out_freq) == 0) {
        *vco_out_freq = ADI_ADF4030_VCO_FREQ_TYP;
        vco_out_found = 1;
        err = API_CMS_ERROR_OK;
        ADI_CMS_ERROR_GOTO(err, end);
    } else {
        for (i = ceil(ADI_ADF4030_VCO_FREQ_MIN/(float)bsync_out_freq); i <= floor(ADI_ADF4030_VCO_FREQ_MAX/(float)bsync_out_freq); i++) {
            if (i <= ADI_ADF4030_O_DIV_MAX) {
                o_div_arr[o_div_idx++] = i;
            }
        }

        for (j = ceil(ref_input_freq/(float)ADI_ADF4030_PFD_FREQ_MAX); j <= floor(ref_input_freq/(float)ADI_ADF4030_PFD_FREQ_MIN); j++) {
            if (j <= ADI_ADF4030_R_DIV_MAX) {
                r_div_arr[r_div_idx++] = j;
            }
        }

        for (i = 0; i < o_div_idx; i++) {
            vco_freq = (uint64_t) (o_div_arr[i] * bsync_out_freq);
            // printf("\n");

            for (j = 0; j < r_div_idx; j++) {
                n_div = ((vco_freq * r_div_arr[j]) / (float) ref_input_freq);
                if (n_div == floor((vco_freq * r_div_arr[j]) / (float) ref_input_freq)) {
                    if (n_div_optimal > n_div ) {
                        // printf("Optimal FOUND!!\n");
                        n_div_optimal = n_div;
                        *vco_out_freq = vco_freq;
                        vco_out_found = 1;
                    }
                    if (n_div > ADI_ADF4030_N_DIV_MAX) {
                        // printf("CANNOT BE CONFIGURED!!\n");
                        err = API_CMS_ERROR_NOT_SUPPORTED;
                        ADI_CMS_ERROR_GOTO(err, end);
                    } else {
                        // o_div = (uint16_t) ((ref_input_freq * n_div) / (bsync_out_freq * r_div_arr[j]));
                        // printf("VCO_OUT: %llu \t r_div: %d \t n_div: %d \t o_div: %d.\n", vco_freq, r_div_arr[j], (uint16_t) n_div, o_div);
                    }
                } else {
                    if ((o_div_idx >= o_div_arr_len) && (vco_out_found == 0)) {
                        // printf("FLOOR IS LAVA!!\n");
                        err = API_CMS_ERROR_INVALID_PARAM;
                        ADI_CMS_ERROR_GOTO(err, end);
                    }
                }
            }
        }
        // printf("vco_out_found = %d\n", vco_out_found);
        // printf("n_div_optimal = %d\n", n_div_optimal);
        // printf("vco_out_freq = %llu\n", *vco_out_freq);
    }

end:
    return err;
}


int32_t adi_ads10_apollo_ex_adf4030_startup(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq_hz, uint64_t vco_out_freq_hz)
{
    int32_t err = 0;
    uint8_t ref_status = 0;
    uint8_t lock_status = 0;
    uint32_t timeout_delay_us = 10000;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_RANGE_CHECK(ref_input_freq_hz, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);
    ADI_CMS_RANGE_CHECK(vco_out_freq_hz, ADI_ADF4030_VCO_FREQ_MIN, ADI_ADF4030_VCO_FREQ_MAX);

    err = adi_adf4030_core_soft_reset(adf4030);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_core_init(adf4030);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_core_power_up(adf4030);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_ldctrl_ref_status_get(adf4030, &ref_status);
    ADI_CMS_ERROR_RETURN(err);

    if(ref_status == 0) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_IN_REF_STATUS);
    }

    err = adi_adf4030_pll_cal_enable(adf4030, 1);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_pll_freq_set(adf4030, ref_input_freq_hz, vco_out_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_core_wait_us(adf4030, timeout_delay_us);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_pll_cal_enable(adf4030, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_ldctrl_lock_wait(adf4030, timeout_delay_us);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_ldctrl_status_get(adf4030, &lock_status);
    ADI_CMS_ERROR_RETURN(err);

    if(lock_status == 0) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_PLL_NOT_LOCKED);
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_adf4030_bsync_output_set(adi_adf4030_device_t *adf4030,
                                                     uint16_t bsync_out_ch_sel,
                                                     uint64_t vco_out_freq_hz,
                                                     uint64_t bsync_out_freq_hz,
                                                     uint8_t boost_en)
{
    int32_t err = 0;
    adi_adf4030_channel_id_e channel_id = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_RANGE_CHECK(vco_out_freq_hz, ADI_ADF4030_VCO_FREQ_MIN, ADI_ADF4030_VCO_FREQ_MAX);
    ADI_CMS_RANGE_CHECK(bsync_out_freq_hz, ADI_ADF4030_BSYNC_FREQ_MIN, ADI_ADF4030_BSYNC_FREQ_MAX);
    ADI_CMS_RANGE_CHECK(boost_en, 0, 1);

    for (channel_id = 0;  channel_id < ADI_ADF4030_CHANNEL_ID_NUM; ++channel_id) {
        if (bsync_out_ch_sel & (1 << channel_id)) {
            err = adi_adf4030_bsyncout_freq_set(adf4030, channel_id, vco_out_freq_hz, bsync_out_freq_hz);
            ADI_CMS_ERROR_RETURN(err);

            err = adi_adf4030_bsyncout_drive_set(adf4030, channel_id, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
            ADI_CMS_ERROR_RETURN(err);

            if (boost_en) {
                err = adi_adf4030_bsyncout_boost_enable(adf4030, channel_id, 1);
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_adf4030_bsync_input_set(adi_adf4030_device_t *adf4030, uint16_t bsync_in_ch_sel)
{
    int32_t err = 0;
    adi_adf4030_channel_id_e channel_id = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);

    for (channel_id = 0;  channel_id < ADI_ADF4030_CHANNEL_ID_NUM; ++channel_id) {
        if (bsync_in_ch_sel & (1 << channel_id)) {
            err = adi_adf4030_bsyncout_drive_set(adf4030, channel_id, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_RX);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_adf4030_align_bsync_out(adi_adf4030_device_t *adf4030,
                                                    uint16_t bsync_in_ch_sel,
                                                    uint16_t bsync_out_ch_sel,
                                                    uint64_t bsync_out_freq_hz)
{
    int32_t err;
    adi_adf4030_channel_id_e source_ch_id = 0;
    adi_adf4030_channel_id_e target_ch_id = 0;
    int64_t tdc_result = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_RANGE_CHECK(bsync_out_freq_hz, ADI_ADF4030_BSYNC_FREQ_MIN, ADI_ADF4030_BSYNC_FREQ_MAX);

    for (source_ch_id = 0;  source_ch_id < ADI_ADF4030_CHANNEL_ID_NUM; ++source_ch_id) {
        if (bsync_in_ch_sel & (1 << source_ch_id)) {
            for (target_ch_id = 0;  target_ch_id < ADI_ADF4030_CHANNEL_ID_NUM; ++target_ch_id) {
                if (bsync_out_ch_sel & (1 << target_ch_id)) {
                    err = adi_adf4030_auto_align_single_channel_set(adf4030, source_ch_id, target_ch_id);
                    ADI_CMS_ERROR_RETURN(err);

                    err = adi_adf4030_tdc_measure(adf4030, source_ch_id, target_ch_id, bsync_out_freq_hz, &tdc_result);
                    ADI_CMS_ERROR_RETURN(err);

                    printf("%-16s[%d-%d]: %12lld fs  %12.3f ps  %12.6f ns.\n", "tdc_result",source_ch_id, target_ch_id, tdc_result, (tdc_result / 1e3), (tdc_result / 1e6));
                }
            }
        }
    }

    return API_CMS_ERROR_OK;
}
