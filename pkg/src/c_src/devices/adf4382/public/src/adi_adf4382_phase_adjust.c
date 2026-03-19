/*!
 * \brief     ADF4382 Phase Adjustment Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_phase_adjust.h"
#include "adi_adf4382_regmap_inline_bfs.h"
#include "adi_adf4382_rfout_types.h"


#define PERIOD_IN_DEGREE 360

int32_t adi_adf4382_phase_adjust_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_phase_adjust_config_t *config, uint8_t *phase_adj_rb)
{

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(config);
    ADI_CMS_RANGE_CHECK(config->phase_mode, 0, 1);
    ADI_CMS_RANGE_CHECK(config->ref_freq_hz, ADI_ADF4382_REF_MIN, ADI_ADF4382_REF_MAX);
    ADI_CMS_RANGE_CHECK(config->rfout_freq_hz, ADI_ADF4382_RFOUT_MIN, ADI_ADF4382_RFOUT_MAX);

    int32_t err;
    int32_t return_value;
    uint64_t phase_adj_fs = 0;
    uint64_t phase_adj_degree = 0;
    uint64_t calc_1 = 0;

    /* Charge Pump Current List in uA */
    uint64_t cpcurrentlist[] = { 0.79e3, 0.99e3, 1.19e3, 1.38e3, 1.59e3, 1.98e3, 2.39e3, 2.79e3, 3.18e3, 3.97e3, 4.77e3, 5.57e3, 6.33e3, 7.91e3, 9.51e3, 11.1e3 };

    /* Setting Polarity for Phase Adjust */
    if (config->phase_value_fs < 0) {
        err = adi_adf4382_bf___REG0032___PHASE_ADJ_POL_set(adf4382, 0);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_adf4382_bf___REG0032___PHASE_ADJ_POL_set(adf4382, 1);
        ADI_CMS_ERROR_RETURN(err);
    }

    phase_adj_fs = abs(config->phase_value_fs); // need to add 10^-15 to this value since floating points are dangerous, its been used as femto seconds.

    if (ADI_ADF4382_PHASE_ADJ_BLEED_CURRENT == config->phase_mode) { // enable phase adjust

        err = adi_adf4382_bf___REG0032___DEL_MODE_set(adf4382, config->phase_mode);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4382_bf___REG001F___EN_BLEED_set(adf4382, 1);
        ADI_CMS_ERROR_RETURN(err);

        /* Checking Phase Limit */
        /* #Warning: This value could exceed 32 bit. It may create bugs in some machines. */
        phase_adj_degree = config->rfout_freq_hz * phase_adj_fs;
        phase_adj_degree /= 1e6; // Convert Femto seconds to Nano seconds.
        phase_adj_degree *= PERIOD_IN_DEGREE; // Converting to a Degree Value
        phase_adj_degree /= 1e6; // Calculations are finished, There will be no precision lost. Converted to nano seconds to micro seconds.

        if (phase_adj_degree > PERIOD_IN_DEGREE * 1e3) {
            return_value = API_CMS_ERROR_INVALID_PARAM;
        } else {

            /* Calculation of Phase Adjust Value.
             * Ref: ADF4382 Datasheet "Bleed Current Phase Adjustment" Section
             */

            uint8_t cp_i_index = 0;
            err = adi_adf4382_bf___REG001F___CP_I_get(adf4382, &cp_i_index);
            ADI_CMS_ERROR_RETURN(err);

            uint8_t r_div = 1;
            err = adi_adf4382_bf___REG0020___R_DIV_get(adf4382, &r_div);
            ADI_CMS_ERROR_RETURN(err);

            uint8_t rdblr = 0;
            err = adi_adf4382_bf___REG0020___EN_RDBLR_get(adf4382, &rdblr);
            ADI_CMS_ERROR_RETURN(err);
            rdblr = rdblr > 0 ? 2 : 1;
            uint64_t pfd_freq = (config->ref_freq_hz / r_div) * rdblr;

            /* #Warning: This value could exceed 32 bit. It may create bugs in some machines. */
            calc_1 = (phase_adj_degree * 511 * cpcurrentlist[cp_i_index]) / 285;
            calc_1 *= pfd_freq;
            calc_1 /= config->rfout_freq_hz;
            calc_1 /= PERIOD_IN_DEGREE; // Degree to Value
            uint16_t calc_1_rem = (calc_1 % 1000); // Avg
            calc_1 = (calc_1_rem >= 500) ? (calc_1 - calc_1_rem + 1000) : calc_1;
            calc_1 /= 1e3; // Converting ms to s.

            calc_1 &= 0xFF; // LSB 8 Bits.

            err = adi_adf4382_bf___REG0033___PHASE_ADJUSTMENT_set(adf4382, calc_1);
            ADI_CMS_ERROR_RETURN(err);

            if (config->trigger_mode == ADI_ADF4382_PHASE_ADJ_TRIGGER_SPI) {

                /* Disable Auto Alignment */
                err = adi_adf4382_bf___REG0032___EN_AUTO_ALIGN_set(adf4382, 0);
                ADI_CMS_ERROR_RETURN(err);

                err = adi_adf4382_bf___REG0034___PHASE_ADJ_set(adf4382, 0x01);
                ADI_CMS_ERROR_RETURN(err);

            } else {
                /* Enable Auto Alignment */
                err = adi_adf4382_bf___REG0032___EN_AUTO_ALIGN_set(adf4382, 1);
                ADI_CMS_ERROR_RETURN(err);
            }

            err = adi_adf4382_bf___REG0033___PHASE_ADJUSTMENT_get(adf4382, phase_adj_rb);
            ADI_CMS_ERROR_RETURN(err);

            return_value = API_CMS_ERROR_OK;

        }

    } else if (ADI_ADF4382_PHASE_ADJ_SIGMA_DELTA == config->phase_mode) {
        /* #TO DO: Developing... */

        // err = adi_adf4382_bf___REG0032___DEL_MODE_set(adf4382, config->phase_mode);
        // ADI_CMS_ERROR_RETURN(err);

        return_value = API_CMS_ERROR_INVALID_PARAM;
    } else {
        return_value = API_CMS_ERROR_INVALID_PARAM;
    }

    return return_value;
}

int32_t adi_adf4382_phase_adjust_resync_enable(adi_adf4382_device_t *adf4382, uint8_t en_phase_resync)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(en_phase_resync, 0, 1);

    err = adi_adf4382_bf___REG001E___EN_PHASE_RESYNC_set(adf4382, en_phase_resync);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_phase_adjust_auto_align_enable(adi_adf4382_device_t *adf4382, uint8_t en_auto_align)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(en_auto_align, 0, 1);

    err = adi_adf4382_bf___REG0032___EN_AUTO_ALIGN_set(adf4382, en_auto_align);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}