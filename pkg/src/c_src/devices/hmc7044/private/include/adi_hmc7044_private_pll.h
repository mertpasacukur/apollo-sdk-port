/*!
 * \brief     HMC7044 PLL Functions
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PRIVATE_PLL_H__
#define __ADI_HMC7044_PRIVATE_PLL_H__

#include "adi_hmc7044_private_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure and enable Reference Clock Inputs
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clk_in                            Reference clock Input index. Range 0 to 3 and Oscillator input index as 4
 * \param[in] config                            Input Buffer Configuration for Reference Clock Input.
 *                                              Refer to adi_hmc7044_ip_buffer_settings_e for configuration mask settings.
 *                                              For example for ac coupled inputs with 100 Ohm internal termination.
 *                                              Set config to IPBUFFER_INTERNAL_100_OHM_EN | PBUFFER_AC_COUPLED_MODE_EN
 *                                              and Input Buffer Configuration for Reference Oscillator Input
 * \param[in] enable                            Enable setting for Reference clock.
 *                                                0 Disable Input
 *                                                1 Enable Input
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_input_reference_set(adi_hmc7044_device_t *hmc7044, uint8_t clk_in, uint8_t config, uint8_t enable);

/**
 * @brief Configure and enable Reference Clock Inputs
 *
 * @pre-requisite  adi_hmc7044_input_reference_set should be called with required CLKIN and OSCIN configs
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clk_in                            Reference clock Input index. Range 0 to 3 and Oscillator input index as 4
 * \param[in] enable                            Enable setting for Reference clock.
 *                                                0 Disable Input
 *                                                1 Enable Input
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_enable_input_reference_set(adi_hmc7044_device_t *hmc7044, uint8_t clk_in, uint8_t enable);

/**
 * @brief readback Reference Clock Inputs
 *
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure
 * \param[out] status                           Readback the setting for Reference clock.
 *                                                0 using onboard crystal
 *                                                1 using external reference
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_input_reference_get(adi_hmc7044_device_t *hmc7044,  uint8_t *status);

/**
 * @brief Set the input reference priority order
 *
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] priority                          Pointer to clock reference priority list.
 *                                              where priority[0]
 *                                              value represents the clock input index with the highest priority.
 * \param[in] nof_ref                            Size of/ Number of clock reference priority list. Range 1 to 4
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_input_reference_priority_set(adi_hmc7044_device_t *hmc7044, uint8_t priority[4], uint8_t nof_ref);

/**
 * @brief  Configure the operation during Loss of Signal(LOS)
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] timer_cycles                      Setting to configure the number of timer cycle to wait
 *                                              before exiting LOS state after reference becomes valid.
 *                                              Where the set value x, translatest 2^x timer cycles.
 *                                              Valid Range 0 to 7
 *                                              For example
 *                                                0 - no wait
 *                                                7 - 128 timer cycles.
 * \param[in] ip_prescaler                      LOS operation bypasses input prescaler.
 *                                                1 - bypass input prescaler
 *                                                0 - do not bypass input prescaler
 * \param[in] vcxo_prescaler                    LOS operation uses VCXO prescaler.
 *                                                1 - Use VCXO prescaler
 *                                                0 - do not Use VCXO prescaler
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_input_reference_los_config_set(adi_hmc7044_device_t *hmc7044, uint8_t timer_cycles, uint8_t ip_prescaler, uint8_t vcxo_prescaler);

/**
 * @brief  Configure the Input Reference Input Prescaler LCM Dividers
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] clk_in                            Reference Clock input index. Range 0 to 3
 * \param[in] lcm_div                           Value of prescaler divider setpoint 1 to 255
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_input_reference_prescaler_config_set(adi_hmc7044_device_t *hmc7044, uint8_t clk_in, uint8_t lcm_div);

/**
 * @brief  Configure the Oscillator Input Prescaler LCM Divider
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] lcm_div                           Value of prescaler divider setpoint 1 to 255
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_input_reference_oscin_prescaler_config_set(adi_hmc7044_device_t *hmc7044, uint8_t lcm_div);

/**
 * @brief  Configure the PLL1 PFD
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] r_div                             RefA input divider. 16-bit Value
 * \param[in] n_div                             Feedback divider to PLL1 16-bit Value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll1_config_set(adi_hmc7044_device_t *hmc7044, uint16_t r_div, uint16_t n_div);

/**
 * @brief  Configure the PLL2 PFD
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] freq_dbl_en                       Enable Pre-divider Frequency Doubler
 * \param[in] r_div                             PLL2 R divider. 12 bit vlaue Range 1 to 4095
 * \param[in] n_div                             16 Bit Feedback divider to PLL2
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll2_config_set(adi_hmc7044_device_t *hmc7044, uint8_t freq_dbl_en, uint16_t r_div, uint16_t n_div);

/**
 * @brief  Get pll1 and pll2  lock status detected by device
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure.
 * \param[out] status                           Pointer to return bitwise representation of pll1 and pll2 lock status
 *                                              Bit value 0f 0-> unlocked, bit value of 1->locked
 *                                              BIT[0] Represents PLL1 LOCK status
 *                                              BIT[1] Represents PLL2 LOCK status
 *                                              BIT[2] Represents PLL1 AND PLL2 Lock status
 *                                              Ref also di_hmc7044_pll_status_mask_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_device_pll_lock_status_get(adi_hmc7044_device_t *hmc7044, uint8_t *status);

/**
 * @brief Configure PLL1 HOldover Exit set
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] config                            Holdover configuration object as defined by adi_hmc_pll1_holdover_config_t
 *                                                exit_action    PLL1 exit action mode
 *                                                  HMC7004_HO_EA_RESET_DIVIDERS,
 *                                                  HMC7044_HO_EA_DO_NOTHING_1,
 *                                                  HMC7044_HO_EA_DO_NOTHING_2,
*                                                   HMC7044_HO_EA_DAC_ASSIT
 *                                                exit_criteria  PLL1 exit criteria mode
 *                                                  HMC7044_HO_EC_LOS_GONE_0,
 *                                                  HMC7044_HO_EC_ZERO_PHASE_ERR,
 *                                                  HMC7044_HO_EC_LOS_GONE_2,
 *                                                  HMC7044_HO_EC_IMMEDIATE
 *                                                holdover_dac   Holdover DAC value
 *                                                adc_tracking   ADC tracking
 *                                                  1 - Disable ADC tracking
 *                                                  0 - Use DAC Hold value
 *                                                quick_mode     Force DAC to holdover
 *                                                  1 - Force DAC control value to DAC holdover value immediately
 *                                                  0 - Force DAC control value to DAC holdover value gradually
 *                                                holdover_bw    Tracking BW reduction value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll1_holdover_exit_ctrl_set(adi_hmc7044_device_t *hmc7044, adi_hmc_pll1_holdover_config_t *config);

/**
 * @brief set charge pump for PLL1
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] charge_pump                       Charge pump value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll1_charge_pump_ctrl_set(adi_hmc7044_device_t *hmc7044, uint8_t charge_pump);

/**
 * @brief set PLL1 dectect control
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] slip_use                          Use slip indicator
 * \param[in] lock_detect_timer                 PLL1 lock detect center depth value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll1_lock_detect_set(adi_hmc7044_device_t *hmc7044, uint8_t slip_use, uint8_t lock_detect_timer);

/**
 * @brief set PLL1 detect control
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] bypass_debounce                   Bypass debouncer in manual mode
 * \param[in] manual_switch                     manual reference mode
 * \param[in] holdover_dac_use                  Holdover uses
 *                                                0 - Tristate hcarge pump
 *                                                1 - Holdover DAC
 * \param[in] autorevert_switch                 Revert to PLL2 best option if available again
 * \param[in] automode_switch                   Automatic swtiching mode using the reference clk priority
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll1_reference_switch_ctrl_set(adi_hmc7044_device_t *hmc7044, uint8_t bypass_debounce, uint8_t manual_switch, uint8_t holdover_dac_use, uint8_t autorevert_switch, uint8_t automode_switch);

/**
 * @brief set PLL1 hold offo time control set
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] hold_off_time                     Holdoff timer value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll1_holdoff_time_ctrl_set(adi_hmc7044_device_t *hmc7044, uint8_t hold_off_time);

/**
 * @brief set charge pump for PLL2
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] charge_pump                       Charge pump value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_pll2_charge_pump_ctrl_set(adi_hmc7044_device_t *hmc7044, uint8_t charge_pump);

/**
 * @brief set OSC out path
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] divider                           Oscillator output divider ratio
 * \param[in] enable                            Enable Oscillator output path
 *                                                0 - Disable path
 *                                                1 - Enable path
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_oscout_path_config_set(adi_hmc7044_device_t *hmc7044, uint8_t divider, uint8_t enable);

/**
 * @brief  Configure the Output Driver Controls for Oscillators
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] output_ch                         Ouput Channel Index.
 * \param[in] config                            A pointer to a structure defining the desired Output Driver Configuration.
 *                                                mode        Output Signal Type.
 *                                                  Valid options CML, LVPECL, LVDS or CMOS.
 *                                                impedance   Output Signal Impedendence.
 *                                                  Valid Options: Internal Resistor Disabled, 100 Ohm, 50 Ohm
 *                                                dynamic_driver_en   Enable Oscillator driver
 *                                                  0 - Driver Disable
 *                                                  1 - Driver Enable
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_oscout_driver_config_set(adi_hmc7044_device_t *hmc7044, uint8_t output_ch, adi_hmc7044_op_driver_config_t *config);

/**
 * @brief  Config PLLs (R1 N1 R2 N2) based on  pfd frequcencies
 *         for desired up distribution Frequency
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] ref_clk_freq_hz                   On board osc ref clk
 * \param[in] pfd1_freq_hz                      Phase frequency detector for pll1
 * \param[in] pfd2_freq_hz                      Phase frequency detector for pll2
 * \param[in] dist_freq_hz                      Distribution frequency
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_distribution_clk_config(adi_hmc7044_device_t *hmc7044, uint64_t ref_clk_freq_hz, uint64_t pfd1_freq_hz, uint64_t dist_freq_hz, uint64_t pfd2_freq_hz);

/**
 * @brief  HMC7044 pll configuration
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] ref_ch                            Channel mask of Clock Input reference source
 * \param[in] ref_clk_freq_hz                   Ref clk from clock input channel
 * \param[in] fvcxo_clk_freq_hz                 Desired fvcxo clk frequency
 * \param[in] fpfd1_freq_hz                     Phase frequency detector for pll1
 * \param[in] fvco_freq_hz                      Desired fvco clk frequency
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_private_pll_config(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clk_in_e ref_ch, uint64_t ref_clk_freq_hz, uint64_t fvcxo_clk_freq_hz, uint64_t fpfd1_freq_hz, uint64_t fvco_freq_hz);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_PRIVATE_PLL_H__