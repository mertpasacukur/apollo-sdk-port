/*!
 * \brief     HMC7044 Device Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_DEVICE_TYPES_H__
#define __ADI_HMC7044_DEVICE_TYPES_H__

#include "adi_hmc7044_types.h"
#include "adi_hmc7044_clkout_types.h"

#define ADI_HMC7044_GPI_COUNT 4
#define ADI_HMC7044_GPO_COUNT 4


typedef struct {
    uint64_t freq_hz;
    uint64_t div;
} adi_hmc7044_device_rational_freq_t;

typedef struct {
    adi_hmc7044_clkin_e                clkin;
    adi_hmc7044_clkout_e               clkout;
    uint64_t                           ref_clk_hz;
    adi_hmc7044_clkin_e                ref_priority[ADI_HMC7044_CLKIN_COUNT];
    uint64_t                           vcxo_hz;
    adi_hmc7044_device_rational_freq_t output_freq[ADI_HMC7044_CLKOUT_COUNT];
    adi_hmc7044_clkout_config_t        clkout_config;
    adi_hmc7044_clkout_driver_config_t clkout_driver_config;
} adi_hmc7044_device_clkout_config_t;

typedef enum {
    ADI_HMC7044_DEVICE_GPI1 = 1,
    ADI_HMC7044_DEVICE_GPI2 = 2,
    ADI_HMC7044_DEVICE_GPI3 = 4,
    ADI_HMC7044_DEVICE_GPI4 = 8
} adi_hmc7044_device_gpi_e;

typedef enum {
    ADI_HMC7044_DEVICE_GPI_MODE_FORCE_PLL1_HOLDOVER   = 0x1,
    ADI_HMC7044_DEVICE_GPI_MODE_MAN_SEL_REF_PLL1_BIT1 = 0x2,
    ADI_HMC7044_DEVICE_GPI_MODE_MAN_SEL_REF_PLL1_BIT0 = 0x3,
    ADI_HMC7044_DEVICE_GPI_MODE_SLEEP                 = 0x4,
    ADI_HMC7044_DEVICE_GPI_MODE_ISSUE_MUTE            = 0x5,
    ADI_HMC7044_DEVICE_GPI_MODE_MAN_SEL_INT_VCO_TYPE  = 0x6,
    ADI_HMC7044_DEVICE_GPI_MODE_HIGH_PERF_PLL2_VCO    = 0x7,
    ADI_HMC7044_DEVICE_GPI_MODE_ISSUE_PULSE_GEN_REQ   = 0x8,
    ADI_HMC7044_DEVICE_GPI_MODE_ISSUE_RESEED_REQ      = 0x9,
    ADI_HMC7044_DEVICE_GPI_MODE_ISSUE_RESTART_REQ     = 0xA,
    ADI_HMC7044_DEVICE_GPI_MODE_FORCE_FANOUT          = 0xB,
    ADI_HMC7044_DEVICE_GPI_MODE_ISSUE_SLIP_REQ        = 0xD
} adi_hmc7044_device_gpi_mode_e;

typedef enum {
    ADI_HMC7044_DEVICE_GPO1 = 1,
    ADI_HMC7044_DEVICE_GPO2 = 2,
    ADI_HMC7044_DEVICE_GPO3 = 4,
    ADI_HMC7044_DEVICE_GPO4 = 8
} adi_hmc7044_device_gpo_e;

typedef enum {
    ADI_HMC7044_DEVICE_GPO_OUTPUT_OPEN_DRAIN = 0,
    ADI_HMC7044_DEVICE_GPO_OUTPUT_CMOS = 1
} adi_hmc7044_device_gpo_output_mode_e;

typedef enum {
    ADI_HMC7044_DEVICE_GPO_MODE_ALARM = 0x00,
    ADI_HMC7044_DEVICE_GPO_MODE_SDATA_SPI = 0x01,
    ADI_HMC7044_DEVICE_GPO_MODE_FORCE_1 = 0x1F,
    ADI_HMC7044_DEVICE_GPO_MODE_FORCE_0 = 0x20
} adi_hmc7044_device_gpo_mode_e;

typedef struct {
    uint8_t enabled;
    adi_hmc7044_device_gpi_mode_e mode;
} adi_hmc7044_gpi_config_t;

typedef struct {
    uint8_t enabled;
    adi_hmc7044_device_gpo_mode_e mode;
    adi_hmc7044_device_gpo_output_mode_e output;
} adi_hmc7044_gpo_config_t;

#endif // !__ADI_HMC7044_CORE_TYPES_H__
