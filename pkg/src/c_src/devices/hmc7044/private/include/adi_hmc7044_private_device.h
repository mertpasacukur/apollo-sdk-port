/*!
 * \brief     HMC7044 Private Device Functions
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PRIVATE_DEVICE_H__
#define __ADI_HMC7044_PRIVATE_DEVICE_H__

#include "adi_hmc7044_private_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Open device
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_INVALID_SPI_XFER_PTR   Invalid HAL SPI XFER FUNCTION
 * @return API_CMS_ERROR_INVALID_DELAYUS_PTR    Invalid HAL SPI XFER FUNCTION
 * @return API_CMS_ERROR_DELAY_US               Invalid HAL SPI XFER FUNCTION
 */
  int32_t adi_hmc7044_private_device_hw_open(adi_hmc7044_private_device_t *hmc7044);

 /**
 * @brief  Close device
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_INVALID_DELAYUS_PTR    Invalid HAL SPI XFER FUNCTION
 * @return API_CMS_ERROR_DELAY_US               Invalid HAL SPI XFER FUNCTION
 */
  int32_t adi_hmc7044_private_device_hw_close(adi_hmc7044_private_device_t *hmc7044);

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
 int32_t adi_hmc7044_private_device_init(adi_hmc7044_device_t *hmc7044);

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
 int32_t adi_hmc7044_private_device_sleep_set(adi_hmc7044_device_t *hmc7044, uint8_t sleepmode);

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
 int32_t adi_hmc7044_private_device_deinit(adi_hmc7044_device_t *hmc7044);

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
 int32_t adi_hmc7044_private_device_reset(adi_hmc7044_device_t *hmc7044, uint8_t hw_reset);

/**
 * @brief  Get Device Chip ID
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure.
 * \param[out] chip_id                          A pointer to a variable of type adi_chip_id_t
 *                                              to return the details of the device id.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_chip_id_get(adi_hmc7044_device_t *hmc7044, adi_cms_chip_id_t *chip_id);

/**
 * @brief  Get API Revision Data
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure.
 * \param[out] rev_major                        Pointer to variable to store the Major Revision Number
 * \param[out] rev_minor                        Pointer to variable to store the Minor Revision Number
 * \param[out] rev_rc                           Pointer to variable to store the RC Revision Number
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_api_revision_get(adi_hmc7044_device_t *hmc7044, uint8_t *rev_major, uint8_t *rev_minor, uint8_t *rev_rc);

/**
 * @brief Perform SPI register write access to device
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] address                           SPI address to which the value of data parameter shall be written
 * \param[in] data                              8-bit value to be written to SPI register defined
 *                                              by the address parameter.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_spi_register_set(adi_hmc7044_device_t *hmc7044, uint16_t addr, uint8_t data);

/**
 * @brief Perform SPI register read access to device.
 *
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure
 * \param[in]  address                          SPI address from which the value of data parameter shall be read,
 * \param[out] data                             Pointer to an 8-bit variable to which the value of the
 *                                              SPI register at the address defined by address parameter
 *                                              shall be stored.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_spi_register_get(adi_hmc7044_device_t *hmc7044, uint16_t addr, uint8_t *data);

/**
 * @brief Trigger Internal Dividers and FSM restart via SPI
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_trigger_restart(adi_hmc7044_device_t *hmc7044);

/**
 * @brief Configure and enable GPI
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] gpi_index                         Reference GPI index. Range 0 to 3
 *
 * \param[in] gpi_config                        Config setting for GPI defined by the enum adi_hmc7044_gpi_op_config_e
 * \param[in] enable                            Enable GPI Driver.
 *                                                0 Disable driver
 *                                                1 Enable driver
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_gpi_config_set(adi_hmc7044_device_t *hmc7044, uint8_t gpi_index, uint8_t gpi_config, uint8_t enable);

/**
 * @brief Configure and enable GPO
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] gpo_index                         Reference GPO index. Range 0 to 3
 *
 * \param[in] gpo_config                        Config setting for GPO defined by the enum adi_hmc7044_gpo_op_config_e
 * \param[in] mode                              GPO driver mode
 * \param[in] enable                            Enable GPO Driver.
 *                                                0 Disable driver
 *                                                1 Enable driver
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_gpo_config_set(adi_hmc7044_device_t *hmc7044, uint8_t gpo_index, uint8_t gpo_config, uint8_t mode, uint8_t enable);

/**
 * @brief Configure and enable SDATA
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] mode                              SDATA driver mode
 * \param[in] enable                            Enable SDATA Driver.
 *                                                0 Disable driver
 *                                                1 Enable driver
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_sdata_config_set(adi_hmc7044_device_t *hmc7044, uint8_t mode, uint8_t enable);

/**
 * @brief  HMC register update
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_reg_update(adi_hmc7044_device_t *hmc7044);

/**
 * @brief  HMC set to High performace
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_high_performance_set(adi_hmc7044_device_t *hmc7044);

/**
 * @brief  Set VCO to be used. External VCO or one of the two Internal VCOs
 *         External VCO mod has an option to divide input by 2.
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] vco_sel                           VCO selection as defined by the enum adi_hmc7044_vco_sel_e
 * \param[in] ext_vco_div_en                    Parameter to enable external vco signal by two
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_vco_sel_set(adi_hmc7044_device_t *hmc7044, uint8_t vco_sel , uint8_t ext_vco_div_en);

/**
 * @brief  HMC Enable output Channel
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] ch_en                             Enable 0-6 to enable 14 output channels
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_channel_out_en(adi_hmc7044_device_t *hmc7044, uint8_t ch_en);

/**
 * @brief  Configure  input Reference path configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] sync_mode                         Sync Pin configuration with respect PLL2
 * \param[in] vco_input_mode                    CLKIN1 used for external VCO
 *                                                0 not used
 *                                                1 used
 * \param[in] sync_input_mode                   CLKIN0 used for external RF sync
 *                                                0  not used
 *                                                1 used
 * \param[in] clk_in                            CLKIN selected for PLL1 Reference, range 0-3
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_input_reference_path_en(adi_hmc7044_device_t *hmc7044, uint8_t sync_mode, uint8_t vco_input_mode, uint8_t sync_input_mode, uint8_t clk_in);

/**
 * @brief  Configure the SYNC
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] pulse_gen_mode                    Pulse Generation mode selection, as defined by the enum adi_hmc7044_sysref_mode_config_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_sysref_config_set(adi_hmc7044_device_t *hmc7044, uint8_t pulse_gen_mode);

/**
 * @brief  Configure the SYNC
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] sync_retime                       Retime the Sync
 *                                                0 Bypass the retime
 *                                                1 Retime the external SYNC from Ref 0
 *
 * \param[in] sync_pll2                         Reseed event thorugh PLL2
 *                                                0 Do not allow Reseed event through PLL2
 *                                                1 allow Reseed event through PLL2
 *
 * \param[in] sync_polarity                     SYNC Polarity
 *                                                0 Positive
 *                                                1 Negative
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_sync_config_set(adi_hmc7044_device_t *hmc7044, uint8_t sync_retime, uint8_t sync_pll2, uint8_t sync_polarity);

/**
 * @brief  Configure the SYREF Timer
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] sysref_timer                      12-bit SYSref Timer set point value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_sysref_timer_config_set(adi_hmc7044_device_t *hmc7044, uint16_t sysref_timer);

/**
* @brief  Configure the Alarm Mask
*
* \param[in] hmc7044 [context variable]        Pointer to the device structure
* \param[in] config                            Configuguration of the alarm mask as defined in adi_hmc7044_alarm_mask_config_t
*                                                pll1_lock                 PLL1 near lock signal to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                pll1_lock_aquisition      PLL1 lock acquisition signal to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                pll1_lock_detect          PLL1 lock detect signal to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                pll1_holdover_status      PLL1 holdover status signal to generate alarm signal
*                                                pll1_clkInx_status        CLKINx los to generate alarm signal, as defined by adi_hmc7044_clk_in_e enum
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                sync_request              sync request signals to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                pll1_pll2_lock_detect     PLL1 and PLL2 lock detect signals to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                clkoutputs_phase_status   Clock outputs phase status signal to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                sysref_sync_status        SYSREF sync status signal to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*                                                pll2_lock_detect          PLL2 lock detect signal to generate alarm signal
*                                                  0 - Do not allow
*                                                  1 - Allow
*
* @return API_CMS_ERROR_OK                     API Completed Successfully
* @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
* @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
* @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
*/
 int32_t adi_hmc7044_private_device_alarm_mask_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_alarm_mask_config_t *config);

/**
 * @brief  HMC7044 clock configuration top level API,  need to call adi_hmc7044_reg_update() API after this API call
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] ref_ch                            Channel mask of Clock Input reference source
 * \param[in] config                            Ref clk config to apply to the selected sources
 *                                                ref_clk_freq_hz                     Ref clk from clock input channel
 *                                                ref_priority[4]                     Ref clk priority
 *                                                fvcxo_clk_freq_hz                   Desired fvcxo clk frequency
 *                                                output_ch                           Channel mask of desired output channel
 *                                                output_freq_hz[HMC7044_NOF_OP_CH]   Desired generated clocks on desired output channel
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
 int32_t adi_hmc7044_private_device_clk_config(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clk_in_e ref_ch, adi_hmc7044_clk_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_PRIVATE_DEVICE_H__
