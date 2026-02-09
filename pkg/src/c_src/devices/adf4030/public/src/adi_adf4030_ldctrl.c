/*!
 * \brief     Source file implementing APIs for ADF4030's lock detect functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_LDCTRL
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_ldctrl.h"
#include "adi_adf4030_bf.h"

/*============= C O D E ====================*/


int32_t adi_adf4030_ldctrl_config_set(adi_adf4030_device_t *adf4030, adi_adf4030_ldctrl_config_t *config)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(config->en_ldwin, 0, 1);
    ADI_ADF4030_VALIDATE_RANGE(config->en_loss_of_lock, 0, 1);

    // Enable/Disable LD Window
    err = adi_adf4030_bf___REG005b___EN_LDWIN_set(adf4030, config->en_ldwin);
    ADI_ADF4030_CHECK_ERR_OK(err);

    //Enable/Disable Loss of Lock
    err = adi_adf4030_bf___REG005b___EN_LOL_set(adf4030, config->en_loss_of_lock);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_ldctrl_status_get(adi_adf4030_device_t *adf4030, uint8_t *status)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);

    // Get lock status
    err = adi_adf4030_bf___REG0090___DLD_get(adf4030, status);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_ldctrl_ref_status_get(adi_adf4030_device_t *adf4030, uint8_t *status)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);

    // Check Ref input amplitude above threshold
    err = adi_adf4030_bf___REG008f___REF_OK_get(adf4030, status);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_ldctrl_lock_wait(adi_adf4030_device_t *adf4030, uint32_t timeout_ms)
{
    int32_t err;
    uint32_t i = 0;
    uint8_t status = 0;

    ADI_ADF4030_PTR_CHECK(adf4030);

    // Get lock status
    for (i = 0; i <= timeout_ms; i++) {
        err = adi_adf4030_ldctrl_status_get(adf4030, &status);
        ADI_ADF4030_CHECK_ERR_OK(err);

        if (status) {
            return API_CMS_ERROR_OK;
        }

        adf4030->hal_info.delay_us(adf4030, 1000);
    }

    return API_CMS_ERROR_OPERATION_TIMEOUT;
}


/*! @} */
