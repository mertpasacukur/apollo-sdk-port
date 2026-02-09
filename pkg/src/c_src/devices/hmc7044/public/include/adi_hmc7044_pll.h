/*!
 * \brief     HMC7044 PLL Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PLL_H__
#define __ADI_HMC7044_PLL_H__

#include "adi_hmc7044_pll_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure and enable Reference Clock Inputs
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clk_in                            Reference clock Input index \ref adi_hmc7044_clkin_e.
 * \param[in] config                            Input Buffer Configuration for Reference Clock Input.
 *                                              Refer to \ref adi_hmc7044_pll_ipbuffer_settings_e for configuration mask settings.
 *                                              For example for ac coupled inputs with 100 Ohm internal termination.
 * \param[in] enable                            Enable setting for Reference clock.
 *                                                0 Disable Input
 *                                                1 Enable Input
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll_clkin_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkin_e clk_in, adi_hmc7044_pll_ipbuffer_settings_e config, uint8_t enable);

/**
 * @brief  Configure the operation during Loss of Signal(LOS)
 *
 * \param[in] hmc7044 [context variable]       Pointer to the device structure
 * \param[in] config                           LOS configuration struct \ref adi_hmc7044_pll_los_config_t
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll_los_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_pll_los_config_t *config);

/**
 * @brief  Toggle the enabled PLLs
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] pll                               PLL(s) to toggle \ref adi_hmc7044_pll_sel_e
 * \param[in] enable                            Whether or not the PLL should be enabled.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll_enable_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_pll_sel_e pll, uint8_t enable);

/**
 * @brief  Gets the enabled PLL mask.
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure
 * \param[out] pll                              Mask of enabled PLL(s) \ref adi_hmc7044_pll_sel_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll_enable_get(adi_hmc7044_device_t *hmc7044, adi_hmc7044_pll_sel_e *pll);

/**
 * @brief  Gets the currently locked PLLs
 *
 * \param[in]  hmc7044 [context variable]        Pointer to the device structure
 * \param[out] locked                            The currently locked PLLs \ref adi_hmc70944_pll_locked_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll_locked_get(adi_hmc7044_device_t *hmc7044, adi_hmc70944_pll_locked_e *locked);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_PLL_H__
