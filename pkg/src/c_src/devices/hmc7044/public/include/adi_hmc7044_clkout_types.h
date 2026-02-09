/*!
 * \brief     HMC7044 CLKOUT Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_CLKOUT_TYPES_H__
#define __ADI_HMC7044_CLKOUT_TYPES_H__

#include "adi_hmc7044_types.h"

#define ADI_HMC7044_CLKOUT_VALIDATE_STARTUP_MODE(mode)    if ((mode) != ADI_HMC7044_CLKOUT_STARTUP_MODE_ASYNC && (mode) != ADI_HMC7044_CLKOUT_STARTUP_MODE_DYNAMIC) return API_CMS_ERROR_INVALID_PARAM

typedef enum {
    ADI_HMC7044_CLKOUT_MUX_SEL_CH_DIV       = 0x0,
    ADI_HMC7044_CLKOUT_MUX_SEL_ANALOG_DELAY = 0x1,
    ADI_HMC7044_CLKOUT_MUX_SEL_CH_PAIR      = 0x2,
    ADI_HMC7044_CLKOUT_MUX_SEL_IP_VC0       = 0x3
} adi_hmc7044_clkout_mux_sel_e;

typedef enum {
    ADI_HMC7044_CLKOUT_STARTUP_MODE_ASYNC   = 0x00,
    ADI_HMC7044_CLKOUT_STARTUP_MODE_DYNAMIC = 0x03
} adi_hmc7044_clkout_startup_mode_e;

typedef enum {
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_LEVEL      = 0,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_1_PULSE    = 1,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_2_PULSE    = 2,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_4_PULSE    = 3,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_8_PULSE    = 4,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_16_PULSE   = 5,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_16_PULSE2  = 6,
    ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_CONTINUOUS = 7
} adi_hmc7044_clkout_sysref_pulse_gen_mode_e;

typedef struct {
    adi_hmc7044_clkout_mux_sel_e mux_sel;
    uint16_t                     ch_div;
    uint8_t                      high_perform_en;
    uint8_t                      enable;
} adi_hmc7044_clkout_config_t;

typedef struct {
    adi_hmc7044_clkout_startup_mode_e startup_mode;
    uint8_t                           slip_mode_en;
    uint8_t                           sync_mode_en;
} adi_hmc7044_clkout_sync_config_t;

typedef struct {
    uint8_t  multi_slip_en;
    uint16_t slip_delay;
} adi_hmc7044_clkout_multi_slip_config_t;

typedef struct {
    adi_cms_signal_type_e mode;                 /*!< Output Driver Mode */
    adi_cms_signal_impedance_type_e impedance;  /*!< Output Driver Impedance */
    uint8_t dynamic_driver_en;                  /*!< Dynamic Driver Mode Enable */
    uint8_t force_mute_en;                      /*!< Force Mute Enable */
} adi_hmc7044_clkout_driver_config_t;

#endif // !__ADI_HMC7044_CORE_TYPES_H__