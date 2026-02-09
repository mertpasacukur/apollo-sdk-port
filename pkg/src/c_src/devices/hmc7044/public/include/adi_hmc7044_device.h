/*!
 * \brief     HMC7044 Device Level Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_DEVICE_H__
#define __ADI_HMC7044_DEVICE_H__

#include "adi_hmc7044_device_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize device
 *         This API must be called first before any other API calls.
 *         It performs internal API initialization of the memory and API states.
 *         If device member hw_open is not NULL it shall call the function
 *         to which it points. This feature may be used to get and initialize the
 *         hardware resources required by the API and the device.
 *         For example GPIO, SPI etc.
 *         Its is recommended to call the Reset API after this API to ensure all
 *         SPI registers are reset to ADI recommended defaults.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 */
int32_t adi_hmc7044_device_init(adi_hmc7044_device_t *hmc7044);

/**
 * @brief  De-initialize device.
 *         This API must be called last. No other API should be called after this call.
 *         It performs internal API De-initialization of the memory and API states.
 *         If device member hw_close is not NULL it shall call the function
 *         to which it points. This feature may be used to De-initialize and release
 *         any hardware resources required by the API and the device.
 *         For example GPIO, SPI etc.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 */
int32_t adi_hmc7044_device_deinit(adi_hmc7044_device_t *hmc7044);

/**
 * @brief  Reset device
 *         Issues a hard reset or soft reset of the device.
 *         Performs a full reset of device via the hardware pin (hard) or
 *         via the SPI register (soft).
 *         Resetting all SPI registers to default and triggering the required
 *         initialization sequence.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure.
 * \param[in] hw_reset                          A parameter to indicate if the reset issues is to be via the
 *                                              hardware pin or SPI register.
 *                                                A value of 1 indicates a hardware reset is required.
 *                                                A value of 0 indicates a software reset is required.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_reset(adi_hmc7044_device_t *hmc7044, uint8_t hw_reset);

/**
 * @brief  put device to sleep mode
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] sleepmode                         Indicate whether hmc7044 is used
 *                                                1 - put hmc7044 to sleep
 *                                                0 - enable hmc7044
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 */
int32_t adi_hmc7044_device_sleep_set(adi_hmc7044_device_t *hmc7044, uint8_t sleepmode);

/**
 * @brief  HMC7044 clock configuration top level API,  need to call \ref adi_hmc7044_device_fsm_div_restart() API after this API call
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] config                            Ref clk config to apply to the selected sources
 *                                                clkin                                 Channel mask of desired input channel
 *                                                clkout                                Channel mask of desired output channel
 *                                                ref_clk_hz                            Ref clk from clock input channel
 *                                                ref_priority[ADI_HMC7044_CLKIN_COUNT] Ref clk priority
 *                                                vcxo_hz                               Desired fvcxo clk frequency
 *                                                output_hz_scale                       Scale of output_freq, relative to Hz.
 *                                                output_freq[ADI_HMC7044_CLKOUT_COUNT] Desired generated clocks on desired output channel
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_clkout_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_clkout_config_t *config);

/**
 * @brief  Set Internal VCO to be used.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] vco_sel                           VCO selection as defined by the enum \ref adi_hmc7044_int_vco_sel_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_int_vco_sel_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_int_vco_sel_e vco_sel);

/**
 * @brief  Get Internal VCO setting.
 *
 * \param[in]  hmc7044 [context variable]        Pointer to the device structure
 * \param[out] vco_sel                           VCO selection as defined by the enum \ref adi_hmc7044_int_vco_sel_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_int_vco_sel_get(adi_hmc7044_device_t *hmc7044, adi_hmc7044_int_vco_sel_e *vco_sel);

/**
 * @brief  Set the state of the external VCO divider.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] enable                            Toggle the external VCO div. by 2.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_ext_vco_div_enable_set(adi_hmc7044_device_t *hmc7044, uint8_t enable);

/**
 * @brief  Sets the VCO distribution path priority mode.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] priority                          VCO clock distribution path priority \ref adi_hmc7044_clk_dist_priority_e.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_clk_dist_priority_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clk_dist_priority_e priority);

/**
 * @brief  Restarts the dividers/FSMs
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_fsm_div_restart_run(adi_hmc7044_device_t *hmc7044);

/**
 * @brief  Initiates a reseed request.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_reseed_run(adi_hmc7044_device_t *hmc7044);

/**
 * @brief Configure GPI
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] gpi                               GPIs to configure \ref adi_hmc7044_device_gpi_e
 * \param[in] config                            Config setting for GPI defined by the enum adi_hmc7044_gpi_op_config_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_gpi_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpi_e gpi, adi_hmc7044_gpi_config_t *config);

/**
 * @brief Configure GPI Mode
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] gpi                               GPIs to configure \ref adi_hmc7044_device_gpi_e
 * \param[in] mode                              GPI mode \ref adi_hmc7044_device_gpi_mode_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_gpi_mode_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpi_e gpi, adi_hmc7044_device_gpi_mode_e mode);

/**
 * @brief Configure GPI Enable State
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] gpi                               GPIs to configure \ref adi_hmc7044_device_gpi_e
 * \param[in] enable                            GPI mode enable state
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_gpi_enable_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpi_e gpi, uint8_t enable);

/**
 * @brief Configure GPO
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] gpo                               GPOs to configure \ref adi_hmc7044_device_gpo_e
 * \param[in] config                            Config setting for GPO defined by the enum adi_hmc7044_gpo_op_config_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_gpo_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpo_e gpo, adi_hmc7044_gpo_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_DEVICE_H__