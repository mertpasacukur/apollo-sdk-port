/*!
 * \brief     ADF4382 CLOCK CONTROL Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_CLKCTRL_H__
#define __ADI_ADF4382_CLKCTRL_H__

#include "adi_adf4382_clkctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Configure Digital and Analog Clocks 
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] config                            Clock Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_clkctrl_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_clkctrl_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_CLKCTRL_H__