/*!
 * \brief     ADF4382 Clock Control Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_clkctrl.h"
#include "adi_adf4382_regmap_inline_bfs.h"

int32_t adi_adf4382_clkctrl_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_clkctrl_config_t *config)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(config->en_dnclk, 0, 1);
    ADI_CMS_RANGE_CHECK(config->en_drclk, 0, 1);
    ADI_CMS_RANGE_CHECK(config->en_adcclk, 0, 1);

    int32_t err;
    err = adi_adf4382_bf___REG002d___EN_DNCLK_set(adf4382, config->en_dnclk);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_adf4382_bf___REG002d___EN_DRCLK_set(adf4382, config->en_drclk);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_adf4382_bf___REG0031___EN_ADC_CLK_set(adf4382, config->en_adcclk);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}