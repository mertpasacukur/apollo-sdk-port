/*!
 * \brief     HMC7044 CLKOUT Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_CLKOUT_H__
#define __ADI_HMC7044_CLKOUT_H__

#include "adi_hmc7044_clkout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Configure the Output Driver Controls
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clkout                            Ouput Channel mask \ref adi_hmc7044_clkout_e.
 * \param[in] config                            Ouput Channel configuration.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_config_t *config);

/**
 * @brief  Configure the Output Driver Controls
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] channel_index                     Ouput Channel Index (0-13)
 * \param[in] config                            Ouput Channel configuration.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_channel_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_config_t *config);

/**
 * @brief  Configure the Output Analog and Digital Delay Adjustments
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clkout                            Ouput Channel mask \ref adi_hmc7044_clkout_e.
 * \param[in] config                            SYNC config.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_sync_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_sync_config_t *config);

/**
 * @brief  Configure the Output Analog and Digital Delay Adjustments
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] channel_index                     Ouput Channel Index.
 * \param[in] config                            SYNC config.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_channel_sync_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_sync_config_t *config);

/**
 * @brief  Configure the SYSREF Pulse Generator Mode
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] mode                              Pulse generator mode \ref adi_hmc7044_clkout_sysref_pulse_gen_mode_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_sysref_pulse_gen_mode_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_sysref_pulse_gen_mode_e mode);

/**
 * @brief  Set the CLKOUT Multislip Configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clkout                            Ouput Channel mask \ref adi_hmc7044_clkout_e.
 * \param[in] config                            Multi Slip config struct.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_multi_slip_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_multi_slip_config_t *config);

/**
 * @brief  Set the Channel Multislip Configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] channel_index                     Ouput Channel Index.
 * \param[in] config                            Multi Slip config struct.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_channel_multi_slip_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_multi_slip_config_t *config);

/**
 * @brief  Set the CLKOUT Driver Configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clkout                            Ouput Channel mask \ref adi_hmc7044_clkout_e.
 * \param[in] config                            \ref adi_hmc7044_clkout_driver_config_t config struct.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_driver_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_driver_config_t *config);

/**
 * @brief  Set the Channel Driver Configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] channel_index                     Ouput Channel Index.
 * \param[in] config                            \ref adi_hmc7044_clkout_driver_config_t config struct.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clkout_channel_driver_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_driver_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_CLKOUT_H__
