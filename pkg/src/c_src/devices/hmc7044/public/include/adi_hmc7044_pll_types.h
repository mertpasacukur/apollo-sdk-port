/*!
 * \brief     HMC7044 PLL Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PLL_TYPES_H__
#define __ADI_HMC7044_PLL_TYPES_H__

#include "adi_hmc7044_types.h"

typedef enum {
    ADI_HMC7044_PLL_IPBUFFER_INTERNAL_100_OHM = 0x1,
    ADI_HMC7044_PLL_IPBUFFER_AC_COUPLED_MODE  = 0x2,
    ADI_HMC7044_PLL_IPBUFFER_LVPECL_MODE      = 0x4,
    ADI_HMC7044_PLL_IPBUFFER_HIGH_Z_MODE      = 0x8,
    ADI_HMC7044_PLL_IPBUFFER_CONFIG_ALL       = 0xF
} adi_hmc7044_pll_ipbuffer_settings_e;

typedef enum {
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_0   = 0,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_2   = 1,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_4   = 2,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_8   = 3,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_16  = 4,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_32  = 5,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_64  = 6,
    ADI_HMC7044_PLL_LOS_WAIT_CYCLES_128 = 7,
} adi_hmc7044_pll_los_wait_cycles_e;

typedef struct {
    adi_hmc7044_pll_los_wait_cycles_e timer_cycles;
    uint8_t                           input_prescaler_en;
    uint8_t                           vcxo_prescaler_en;
} adi_hmc7044_pll_los_config_t;

typedef enum {
    ADI_HMC7044_PLL_SEL_PLL1 = 0x01,
    ADI_HMC7044_PLL_SEL_PLL2 = 0x02
} adi_hmc7044_pll_sel_e;

typedef enum {
    ADI_HMC7044_PLL_LOCKED_NONE = 0x00,
    ADI_HMC7044_PLL_LOCKED_PLL1 = 0x01,
    ADI_HMC7044_PLL_LOCKED_PLL2 = 0x02,
} adi_hmc70944_pll_locked_e;

#endif // !__ADI_HMC7044_PLL_TYPES_H__
