/*!
 * \brief     Versal Apollo examples common clocking functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_fpga_apollo_types.h"
#include "apollo_cpu_device_profile_types.h"
#include "adi_apollo.h"

#ifndef __VERSAL_APOLLO_EX_CLK_H__
#define __VERSAL_APOLLO_EX_CLK_H__

typedef enum {
    VERSAL_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER = 0x01,   /*!< Set Device Clk Mode to External Center (CLK_C) */
    VERSAL_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_DUAL   = 0x02,   /*!< Set Device Clk Mode to External Dual (CLK_A + CLK_B) */
    VERSAL_APOLLO_CLK_MODE_DEV_CLK_ADF4382         = 0x04,   /*!< Set Device Clk Mode to ADF4382 */
    VERSAL_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL       = 0x08,   /*!< Set FPGA Clk Mode to External */
    VERSAL_APOLLO_CLK_MODE_FPGA_CLK_FMC            = 0x10,   /*!< Set FPGA Clk Mode to FMC (HMC7044) */
    VERSAL_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_PLL    = 0x20,   /*!< Set Device Clk Mode to PLL ref from External */
    VERSAL_APOLLO_CLK_MODE_DEV_CLK_HMC7044_PLL     = 0x40    /*!< Set Device Clk Mode to PLL ref from HMC7044 */
} versal_apollo_clk_mode_e;

/**
 * \brief Configures the clocks.
 */
int32_t versal_apollo_ex_configure_clks(adi_fpga_apollo_device_t *fpga_device,
                                           uint32_t ltc6955_clk_khz,
                                           uint32_t dev_clk_khz,
                                           uint32_t ref_clk_freq_khz,
                                           uint16_t digclk_cycles,
                                           adi_apollo_divg_mode_e divg_mode,
                                           uint32_t lane_rate_khz,
                                           uint8_t fpga_clk_div,
                                           versal_apollo_clk_mode_e mode);

/**
 * \brief Configures clocks based on device profile.
 */
int32_t versal_apollo_ex_configure_profile_clks(adi_fpga_apollo_device_t *fpga_device,
                                                   uint32_t ltc6955_clk_khz,
                                                   adi_apollo_top_t *profile,
                                                   versal_apollo_clk_mode_e mode);

/**
 * \brief Displays clk power level, if ADF4382 calibrate until power level is good.
 */
int32_t versal_apollo_ex_clk_power_cal(adi_apollo_device_t *device,
                                          versal_apollo_clk_mode_e clk_mode,
                                          uint64_t rfout_freq_hz,
                                          uint64_t ref_freq_hz);

#endif /* __VERSAL_APOLLO_EX_CLK_H__ */
