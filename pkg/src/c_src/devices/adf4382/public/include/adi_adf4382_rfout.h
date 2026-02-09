/*!
 * \brief     ADF4382 RFOUT Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_RFOUT_H__
#define __ADI_ADF4382_RFOUT_H__

#include "adi_adf4382_rfout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set RFOUT Frequency
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] config                            RFOUT Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_rfout_output_set(adi_adf4382_device_t *adf4382, adi_adf4382_rfout_output_config_t *config);

/**
 * \brief Set CLK OPWR level
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] clk_sel                           clock select \ref adi_adf4382_clk_sel_e
 * \param[in] clk_opwr                          CLK OPWR level
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_rfout_clk_opwr_set(adi_adf4382_device_t *adf4382, uint16_t clk_sel, uint8_t clk_opwr);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_RFOUT_H__