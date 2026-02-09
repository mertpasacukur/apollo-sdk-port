/*!
 * \brief     ADF4382 Lock Detect Control Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_ldctrl.h"
#include "adi_adf4382_regmap_inline_bfs.h"

int32_t adi_adf4382_ldctrl_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_ldctrl_config_t *config)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(config->en_ldwin, 0, 1);
    ADI_CMS_RANGE_CHECK(config->en_loss_of_lock, 0, 1);

    int32_t err;

    // Enable/Disable LD Window
    err = adi_adf4382_bf___REG002d___EN_LDWIN_set(adf4382, config->en_ldwin);
    ADI_CMS_ERROR_RETURN(err);

    //Enable/Disable Loss of Lock
    err = adi_adf4382_bf___REG002d___EN_LOL_set(adf4382, config->en_loss_of_lock);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_ldctrl_status_get(adi_adf4382_device_t *adf4382, uint8_t *status)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);

    int32_t err;

    // Get lock status
    err = adi_adf4382_bf___REG0058___LOCKED_get(adf4382,status);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_ldctrl_lock_wait(adi_adf4382_device_t *adf4382, uint32_t timeout_us)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);

    int32_t err;
    uint8_t status = 0;

    // Get lock status
    for (uint32_t i = 0; i <= timeout_us; i += 10000) {
        err = adi_adf4382_ldctrl_status_get(adf4382, &status);
        ADI_CMS_ERROR_RETURN(err);
        if (status) {
            return API_CMS_ERROR_OK;
        }
        adf4382->hal_info.delay_us(adf4382, 10000);
    }
    ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OPERATION_TIMEOUT);
    return err;
}

