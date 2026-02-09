/*!
 * \brief     ADF4382 Charge Pump Control Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_CPCTRL_TYPES_H__
#define __ADI_ADF4382_CPCTRL_TYPES_H__
#define ADI_ADF4382_BLEED_CONSTANT                    512U
#define ADI_ADF4382_MIN_BLEED_WORD                    0U
#define ADI_ADF4382_MAX_BLEED_WORD                    8191


#include "adi_adf4382_types.h"
typedef enum {
    ADI_ADF4382_CPCTRL_CP_I_0p7mA = 0x00,
    ADI_ADF4382_CPCTRL_CP_I_0p9mA = 0x01,
    ADI_ADF4382_CPCTRL_CP_I_1p1mA = 0x02,
    ADI_ADF4382_CPCTRL_CP_I_1p3mA = 0x03,
    ADI_ADF4382_CPCTRL_CP_I_1p4mA = 0x04,
    ADI_ADF4382_CPCTRL_CP_I_1p8mA = 0x05,
    ADI_ADF4382_CPCTRL_CP_I_2p2mA = 0x06,
    ADI_ADF4382_CPCTRL_CP_I_2p5mA = 0x07,
    ADI_ADF4382_CPCTRL_CP_I_2p9mA = 0x08,
    ADI_ADF4382_CPCTRL_CP_I_3p6mA = 0x09,
    ADI_ADF4382_CPCTRL_CP_I_4p3mA = 0x0A,
    ADI_ADF4382_CPCTRL_CP_I_5p0mA = 0x0B,
    ADI_ADF4382_CPCTRL_CP_I_5p8mA = 0x0C,
    ADI_ADF4382_CPCTRL_CP_I_7p2mA = 0x0D,
    ADI_ADF4382_CPCTRL_CP_I_8p6mA = 0x0E,
    ADI_ADF4382_CPCTRL_CP_I_10p1mA = 0x0F
} adi_adf4382_cpctrl_cp_i_e;

typedef struct {
    uint8_t cp_i_up;
    uint8_t cp_i_down;
} adi_adf4382_cpctrl_cp_i_config_t;

typedef enum {
    ADI_ADF4382_CPCTRL_BLEED_POLARITY_SINK,
    ADI_ADF4382_CPCTRL_BLEED_POLARITY_SOURCE
} adi_adf4382_cpctrl_bleed_polarity_e;

typedef struct {
    uint8_t bleed_current_coarse;
    uint8_t bleed_current_fine;
} adi_adf4382_cpctrl_bleed_word_config_t;

#endif // !__ADI_ADF4382_CPCTRL_TYPES_H__
