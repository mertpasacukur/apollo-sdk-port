/*!
 * \brief     ADF4382 Lock Detect Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_LDCTRL_H__
#define __ADI_ADF4382_LDCTRL_H__

#include "adi_adf4382_ldctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Configure Lock Detect
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] config                            Lock Detect Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_ldctrl_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_ldctrl_config_t *config);

/**
 * \brief Get lock detector status
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[out] status                           Lock Status
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_ldctrl_status_get(adi_adf4382_device_t *adf4382, uint8_t *status);


/**
 * \brief Wait for ADF4382 to lock (note: timeout does not account for SPI transaction times)
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] timeout_us                        Timeout duration for locking
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_OPERATION_TIMEOUT      Lock did not complete before timeout reached
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_ldctrl_lock_wait(adi_adf4382_device_t *adf4382, uint32_t timeout_us);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_LDCTRL_H__