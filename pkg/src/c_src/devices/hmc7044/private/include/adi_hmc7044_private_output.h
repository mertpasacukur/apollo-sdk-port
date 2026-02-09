/*!
 * \brief     HMC7044 Output Functions
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PRIVATE_OUTPUT_H__
#define __ADI_HMC7044_PRIVATE_OUTPUT_H__

#include "adi_hmc7044_private_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Configure the Output Driver Controls
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index. Range 0-13
 * \param[in] output_sel                        Output Signal Selection
 * \param[in] ch_div                            Ouput Channel Divider setting Range 1-4049
 * \param[in] mode                              Performace mode Enable
 *                                                1- High Performance Mode Enabled
 *                                                0- Normal Output Mode Enabled
 * \param[in] enable                            Performance mode enabled.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_config_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, adi_hmc7044_op_source_e output_sel, uint16_t ch_div, uint8_t mode, uint8_t enable);

/**
 * @brief  Configure the Output Driver Controls
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index.
 * \param[in] config                            A pointer to a structure defining the desired Output Driver Configuration.
 *                                                mode                Output Signal Type.
 *                                                  Valid options CML, LVPECL, LVDS or CMOS.
 *                                                impedance           Output Signal Impedendence.
 *                                                  Valid Options: Internal Resistor Disabled, 100 Ohm, 50 Ohm
 *                                                dynamic_driver_en   Dynamic Driver Control for Pulse Generator Mode Only
 *                                                  0 - Driver Enable Controlled by Channel Enable
 *                                                  1 - Driver Dynamically Enabled with Pulse Gen Events.
 *                                                force_mute_en       Idle at Logic 0 Mode Enable for Pulse Generator Mode Only
 *                                                  0 - Normal Mode
 *                                                  1 - Force to Logic 0
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_driver_config_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, adi_hmc7044_op_driver_config_t *config);

/**
 * @brief  Configure the Output Analog and Digital Delay Adjustments
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index.
 * \param[in] coarse_adj                        Digital Coarse Delay Adjustment in 1/2 VCO Clk Step sizesd.Range 0-17
 * \param[in] fine_adj                          Analog Fine Delay Adjustment in 25ps. Range 0 to 24.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_delay_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, uint8_t coarse_adj, uint8_t fine_adj);

/**
 * @brief  Configure the Output Analog and Digital Delay Adjustments
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index.
 * \param[in] startup_mode                      Set the channel start mode. Range 0-1
 *                                                0 - Asynchronous
 *                                                1 - Dynamic / Pulse Generator Mode
 * \param[in] slip_mode_en                      Slip Event Enable Setting. Channel processes Slip events. Range 0-1
 * \param[in] sync_mode_en                      Sync Event Enable Setting. Channel Process Sync Events. Range 0-1.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_sync_config_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, uint8_t startup_mode, uint8_t slip_mode_en, uint8_t sync_mode_en);

/**
 * @brief  Configure the Channel For Multislip Configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index.
 * \param[in] multi_slip_en                     Multi Slip Enable Setting.Range 0-1.
 * \param[in] slip_delay                        Multi Slip Delay Setting in VCO cycles. Range 0-4095.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_multi_slip_config_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, uint8_t multi_slip_en, uint16_t slip_delay);

/**
 * @brief  Configure the Output Enable
 * Note: All configuration setting should be completed before enabling.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index.
 * \param[in] enable                            Output Signal Enable Range 0-1
 *                                                0 - Disable
 *                                                1 - Enable.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_enable_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, uint8_t enable);

/**
 * @brief  Configure the Output performance setting
 * Note: All configuration setting should be completed before enabling.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] enable                            Optimum performance enable
 *                                                0 - Disable
 *                                                1 - Enable
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_output_performance_set(adi_hmc7044_device_t *hmc7044, uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_PRIVATE_OUTPUT_H__
