/*!
 * \brief     ADF4382 Charge Pump Control Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_cpctrl.h"
#include "adi_adf4382_regmap_inline_bfs.h"

int32_t adi_adf4382_cpctrl_cp_i_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_cp_i_e current)
{
    int32_t err;
    uint8_t reg_data = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(current, ADI_ADF4382_CPCTRL_CP_I_0p7mA, ADI_ADF4382_CPCTRL_CP_I_10p1mA);

    err = adi_adf4382_bf___REG001F___CP_I_set(adf4382, current);
    ADI_CMS_ERROR_RETURN(err);

    // Apply settings to double-buffered bit fields by writing to Reg 0x10
    err = adi_adf4382_hal_reg_get(adf4382, 0x10, &reg_data);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_adf4382_hal_reg_set(adf4382, 0x10, reg_data);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_cpctrl_cptest_enable_set(adi_adf4382_device_t *adf4382, uint8_t cptest_enable)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(cptest_enable, 0, 1);

    int32_t err;

    err = adi_adf4382_bf___REG002e___EN_CPTEST_set(adf4382, cptest_enable);
    ADI_CMS_ERROR_RETURN(err);
    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_cpctrl_cp_i_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_cp_i_config_t *config)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(config);
    ADI_CMS_RANGE_CHECK(config->cp_i_up, 0, 1);
    ADI_CMS_RANGE_CHECK(config->cp_i_down, 0, 1);

    int32_t err;

    err = adi_adf4382_bf___REG002e___CP_UP_set(adf4382, config->cp_i_up);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG002e___CP_DOWN_set(adf4382, config->cp_i_down);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_cpctrl_en_bleed_set(adi_adf4382_device_t *adf4382, uint8_t en_bleed)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(en_bleed, 0, 1);

    int32_t err;

	err = adi_adf4382_bf___REG001F___EN_BLEED_set(adf4382, en_bleed);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_cpctrl_bleed_polarity_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_bleed_polarity_e polarity)
{
    int32_t err;
    uint8_t reg_data = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(polarity, ADI_ADF4382_CPCTRL_BLEED_POLARITY_SINK, ADI_ADF4382_CPCTRL_BLEED_POLARITY_SOURCE);

    err = adi_adf4382_bf___REG001F___BLEED_POL_set(adf4382, polarity);
    ADI_CMS_ERROR_RETURN(err);

    // Apply settings to double-buffered bit fields by writing to Reg 0x10
    err = adi_adf4382_hal_reg_get(adf4382, 0x10, &reg_data);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_adf4382_hal_reg_set(adf4382, 0x10, reg_data);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_cpctrl_bleed_word_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_bleed_word_config_t *config)
{
    int32_t err;
    uint32_t bleed_word = 0;
    uint8_t reg_data = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(config);

    bleed_word = config->bleed_current_coarse * ADI_ADF4382_BLEED_CONSTANT + config->bleed_current_fine;
    ADI_CMS_RANGE_CHECK(bleed_word, ADI_ADF4382_MIN_BLEED_WORD, ADI_ADF4382_MAX_BLEED_WORD);

    err = adi_adf4382_bf___REG001D___BLEED_I_set(adf4382, bleed_word);
    ADI_CMS_ERROR_RETURN(err);

    // Apply settings to double-buffered bit fields by writing to Reg 0x10
    err = adi_adf4382_hal_reg_get(adf4382, 0x10, &reg_data);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_adf4382_hal_reg_set(adf4382, 0x10, reg_data);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}