/*!
 * \brief     Versal Apollo examples common HMC7044 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_hmc7044_device_types.h"
#include "versal_hal.h"

#ifndef __VERSAL_APOLLO_EX_HMC7044_H__
#define __VERSAL_APOLLO_EX_HMC7044_H__

/**
 * \brief   Initializes HMC7044 device HAL.
 */
int32_t versal_apollo_ex_hmc7044_hal_config(adi_hmc7044_device_t *hmc7044, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

/**
 * \brief   Initializes and configures HMC7044.
 */
int32_t versal_apollo_ex_hmc7044_startup(adi_hmc7044_device_t *hmc7044,
                                            uint64_t ref_freq_hz,
                                            adi_hmc7044_device_rational_freq_t *sysref_hz,
                                            adi_hmc7044_device_rational_freq_t *fpga_ref_hz,
                                            adi_hmc7044_device_rational_freq_t *dev_ref_clk_hz);

/**
 * \brief   Perform reset to all HMC7044 dividers and sync by running reseed.
 */
int32_t versal_apollo_ex_hmc7044_reset_fsm_reseed(adi_hmc7044_device_t *hmc7044);

#endif /* __VERSAL_APOLLO_EX_HMC7044_H__ */
