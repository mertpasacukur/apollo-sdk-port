/*!
 * \brief     ADF4382 Power Down Control Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_PDCTRL_H__
#define __ADI_ADF4382_PDCTRL_H__

#include "adi_adf4382_pdctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set Power-Down State
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure.
 * \param[in] target                            Power-Down target \ref adi_adf4382_pdctrl_target_e.
 * \param[in] pd                                Whether or not the target is powered-down.
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_pdctrl_pd_set(adi_adf4382_device_t *adf4382, adi_adf4382_pdctrl_target_e target, uint8_t pd);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_PDCTRL_H__