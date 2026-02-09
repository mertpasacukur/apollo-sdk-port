/*!
 * \brief     ADF4382 Clock Control Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_CLKCTRL_TYPES_H__
#define __ADI_ADF4382_CLKCTRL_TYPES_H__

#include "adi_adf4382_types.h"
/*!
 * \brief ADF4382 clock control config
 */
typedef struct {
    uint8_t en_dnclk;
    uint8_t en_drclk;
    uint8_t en_adcclk;
} adi_adf4382_clkctrl_config_t;

#endif // !__ADI_ADF4382_TYPES_H__