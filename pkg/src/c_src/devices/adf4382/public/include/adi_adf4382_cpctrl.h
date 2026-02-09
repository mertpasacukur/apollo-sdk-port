/*!
 * \brief     ADF4382 CPCTRL Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_CPCTRL_H__
#define __ADI_ADF4382_CPCTRL_H__

#include "adi_adf4382_cpctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set CP Current
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] current                           CP current value \ref adi_adf4382_cpctrl_cp_i_e.
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_cpctrl_cp_i_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_cp_i_e current);

/**
 * \brief Set Charge Pump Test Enable
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] cptest_enable                     Enable Charge Pump Test Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_cpctrl_cptest_enable_set(adi_adf4382_device_t *adf4382, uint8_t cptest_enable);

/**
 * \brief Set Charge Pump Current Control
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] config                            Charge Pump Current Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_cpctrl_cp_i_config_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_cp_i_config_t *config);

/**
 * \brief Set Bleed Enable
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] en_bleed                          Enable Bleed Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_cpctrl_en_bleed_set(adi_adf4382_device_t *adf4382, uint8_t en_bleed);

/**
 * \brief Set Bleed Polarity
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] polarity                          polarity control value \ref adi_adf4382_cpctrl_bleed_pol_e.
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_cpctrl_bleed_polarity_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_bleed_polarity_e polarity);

/**
 * \brief Set Bleed Word
 *
 * \param[in] adf4382 [context variable]        Pointer to the device structure
 * \param[in] config                            Bleed Word Configuration
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return API_CMS_ERROR_INVALID_HANDLE_PTR     Invalid Device Handle
 * \return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_adf4382_cpctrl_bleed_word_set(adi_adf4382_device_t *adf4382, adi_adf4382_cpctrl_bleed_word_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4382_CPCTRL_H__
