/*!
 * \brief     Header file declaring APIs for ADF4030's PLL Output functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_PLL
 * @{
 */

#ifndef __ADI_ADF4030_PLL_H__
#define __ADI_ADF4030_PLL_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_pll_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Set PLL/VCO Config.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   ref_input_freq          Reference input frequency in Hz.
 * \param[in]   config                  Pointer to PLL configuration struct. \ref adi_adf4030_pll_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_pll_config_set(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, adi_adf4030_pll_config_t *config);


/**
 * \brief   Get PLL/VCO Config.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   ref_input_freq          Reference input frequency in Hz.
 * \param[out]  config                  Pointer to PLL configuration struct. \ref adi_adf4030_pll_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_pll_config_get(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, adi_adf4030_pll_config_t *config);


/**
 * \brief   Set PLL/VCO Output Frequency.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   ref_input_freq          Reference input frequency in Hz.
 * \param[in]   vco_out_freq            PLL/VCO out frequency in Hz.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_pll_freq_set(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, uint64_t vco_out_freq);


/**
 * \brief   Enable PLL/VCO Auto Calibration.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   enable                  Enable / Disable PLL Auto Calibration.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_pll_cal_enable(adi_adf4030_device_t *adf4030, uint8_t enable);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_PLL_H__

/*! @} */
