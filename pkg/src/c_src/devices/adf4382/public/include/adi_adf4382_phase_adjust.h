/*!
 * \brief     ADF4382 Phase Adjustment Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_PHASE_ADJUST_H__
#define __ADI_ADF4382_PHASE_ADJUST_H__

#include "adi_adf4382_phase_adjust_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set Phase Adjustment Configuration
 *
 * \param[in]   adf4382 [context variable]      Pointer to the device structure.
 * \param[in]   config                          Phase Adjustment Configuration
 * \param[out]  phase_adj_rb                    Readback programed PHASE_ADJUSTMENT value
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_phase_adjust_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_phase_adjust_config_t *config, uint8_t *phase_adj_rb);

/**
 * \brief Set Phase Resync Enable
 *
 * \param[in]   adf4382 [context variable]      Pointer to the device structure.
 * \param[in]   en_phase_resync                 Enable(1) / Disable(0) Phase Resync Mode
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_phase_adjust_resync_enable(adi_adf4382_device_t *adf4382, uint8_t en_phase_resync);

/**
 * \brief Set Phase Auto Align Enable
 *
 * \param[in]   adf4382 [context variable]      Pointer to the device structure.
 * \param[in]   en_auto_align                   Enable(1) / Disable(0) Phase Align Mode
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_phase_adjust_auto_align_enable(adi_adf4382_device_t *adf4382, uint8_t en_auto_align);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_PHASE_ADJUST_H__