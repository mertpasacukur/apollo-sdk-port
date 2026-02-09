/*!
 * @brief     Versal Apollo example common functions — ported from ADS10.
 *            HAL wiring adapted for Versal platform while preserving all
 *            platform-independent API call sequences exactly.
 */

#ifndef __VERSAL_APOLLO_EX_H__
#define __VERSAL_APOLLO_EX_H__

#include "adi_apollo.h"
#include "versal_hal.h"
#include "adi_fpga_apollo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Configure Apollo HAL for Versal platform.
 *          Equivalent to adi_ads10_apollo_ex_configure_hal().
 *          Wires up SPI read/write, delay, log, reset to Versal implementations.
 */
int32_t versal_apollo_ex_configure_hal(adi_apollo_device_t *apollo_device,
                                        versal_hal_config_t *versal_platform);

/**
 * @brief   Configure startup sequence for Versal.
 *          Assigns the QSPI-based FW provider.
 *          Equivalent to adi_ads10_apollo_ex_configure_startup().
 */
int32_t versal_apollo_ex_configure_startup(adi_apollo_device_t *apollo_device);

/**
 * @brief   Configure FPGA HAL for Versal.
 *          Wires up FPGA register read/write to Versal AXI access.
 *          Equivalent to adi_ads10_apollo_ex_configure_fpga_hal().
 */
int32_t versal_apollo_ex_configure_fpga_hal(adi_fpga_apollo_device_t *fpga_device,
                                             versal_hal_config_t *versal_platform);

/**
 * @brief   Generic startup: load FW, configure profile, bring up digital data path.
 *          Calls adi_apollo_startup_execute() — SAME as ADS10.
 */
int32_t versal_apollo_ex_startup(adi_apollo_device_t *device, adi_apollo_top_t *dev_profile);

/**
 * @brief   FPGA operations before resetting Apollo.
 *          Equivalent to adi_ads10_apollo_ex_fpga_pre_reset().
 */
int32_t versal_apollo_ex_fpga_pre_reset(adi_fpga_apollo_device_t *fpga_device);

#ifdef __cplusplus
}
#endif
#endif /* __VERSAL_APOLLO_EX_H__ */
