/*!
 * @brief     Versal Apollo example common header.
 *
 *            Declarations for HAL wiring and startup functions.
 *            Port of adi_ads10_apollo_ex.h for Versal Standalone.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef __VERSAL_APOLLO_EX_H__
#define __VERSAL_APOLLO_EX_H__

#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "versal_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Wire up the platform-specific APOLLO HAL for the Apollo API.
 *          Sets SPI read/write, delay, reset, log callbacks.
 */
int32_t versal_apollo_ex_configure_hal(adi_apollo_device_t *apollo_device,
                                       adi_fpga_apollo_hal_config_t *versal_platform,
                                       uint8_t enable_hsci);

/**
 * @brief   Wire up the platform-specific FPGA HAL for the FPGA API.
 *          Sets reg_read, reg_write, delay, mem_read, mem_write callbacks.
 */
int32_t versal_apollo_ex_configure_fpga_hal(adi_fpga_apollo_device_t *fpga_device,
                                            adi_fpga_apollo_hal_config_t *versal_platform);

/**
 * @brief   Configure the startup sequence info (FW provider from QSPI flash).
 */
int32_t versal_apollo_ex_configure_startup(adi_apollo_device_t *apollo_device);

/**
 * @brief   Execute Apollo startup sequence (FW load + profile + DP config).
 */
int32_t versal_apollo_ex_startup(adi_apollo_device_t *device, adi_apollo_top_t *dev_profile);

#ifdef __cplusplus
}
#endif

#endif /* __VERSAL_APOLLO_EX_H__ */
