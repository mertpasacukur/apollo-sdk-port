/*!
 * \brief     ADF4382 MUXOUT Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_MUXOUT_H__
#define __ADI_ADF4382_MUXOUT_H__

#include "adi_adf4382_muxout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set MUXOUT voltage Frequency
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] cmos_ov                           MUXOUT Voltage Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_muxout_voltage_set(adi_adf4382_device_t *adf4382, uint8_t cmos_ov);

/**
 * \brief Set MUXOUT
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure.
 * \param[in] muxout                            Muxout Control value \ref adi_adf4382_muxout_outputctrl_e.
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_muxout_outputctrl_set(adi_adf4382_device_t *adf4382, adi_adf4382_muxout_outputctrl_e muxout);
#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_RFOUT_H__