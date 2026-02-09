/*!
 * \brief     ADL6331 CORE Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADL6331_CORE_TYPES_H__
#define __ADI_ADL6331_CORE_TYPES_H__

#include "adi_adl6331_types.h"

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_adl6331_version_t;

typedef enum {
    ADI_ADL6331_MUXOUT_NONE,
    ADI_ADL6331_MUXOUT_ADC_INPUT,
    ADI_ADL6331_MUXOUT_ADC_CLK,
    ADI_ADL6331_MUXOUT_V33_FUSE_SUPPLY_VOLTAGE_LEVEL,
    ADI_ADL6331_MUXOUT_V33_AMP1_SUPPLY_VOLTAGE_LEVEL,
    ADI_ADL6331_MUXOUT_V33_AMP2_SUPPLY_VOLTAGE_LEVEL,
    ADI_ADL6331_MUXOUT_AMP1_INPUT_CM_VOLTAGE_LEVEL,
    ADI_ADL6331_MUXOUT_AMP2_INPUT_CM_VOLTAGE_LEVEL,
    ADI_ADL6331_MUXOUT_PTAT,
    ADI_ADL6331_MUXOUT_AMP1_CELL_PTAT_BIAS_MONITOR,
    ADI_ADL6331_MUXOUT_AMP2_CELL_PTAT_BIAS_MONITOR,
    ADI_ADL6331_MUXOUT_AMP1_CELL_ZTAT_BIAS_MONITOR,
    ADI_ADL6331_MUXOUT_AMP2_CELL_ZTAT_BIAS_MONITOR,
    ADI_ADL6331_MUXOUT_AMP1_CELL_GAIN_EF_BIAS_MONITOR,
    ADI_ADL6331_MUXOUT_AMP2_CELL_GAIN_EF_BIAS_MONITOR,
} adi_adl6331_muxout_e;

typedef struct {
    uint8_t amp1_enable;
    uint8_t amp2_enable;
    uint8_t dsa_enable;
    uint8_t amux_enable;
    uint8_t ldo_1p8_enable;
} adi_adl6331_init_t;

typedef struct {
    uint8_t lsb_first;
    uint8_t big_endian;
    uint8_t sdo_active;
} adi_adl6331_spi_init_t;

#endif // !__ADI_ADL6331_CORE_TYPES_H__
