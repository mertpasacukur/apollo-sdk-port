#if !defined(VERSAL_PLATFORM)

/*!
 * @brief     Versal Manual Register Test Interface
 *
 *            Provides register read/write functions for manual testing via
 *            serial shell. Supports all four devices: APOLLO, HMC7044,
 *            ADF4382, ADF4030.
 *
 *            Usage from shell:
 *              versal_manual_test_write("APOLLO",  0x47000200, 0xAA);
 *              versal_manual_test_read("APOLLO",   0x47000200);
 *              versal_manual_test_write("HMC7044", 0x0001, 0x55);
 *              versal_manual_test_read("ADF4382",  0x0010);
 *              versal_manual_test_read("ADF4030",  0x0020);
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef VERSAL_MANUAL_TEST_H
#define VERSAL_MANUAL_TEST_H

#include <stdint.h>
#include "adi_apollo.h"
#include "adi_hmc7044_types.h"
#include "adi_adf4382_types.h"
#include "adi_adf4030_types.h"

/*!
 * @brief Initialize manual test module with device pointers.
 *
 *        Must be called after all devices are initialized (HAL wired,
 *        hw_open done). Stores pointers internally for later use by
 *        read/write functions.
 *
 * @param[in] apollo    Pointer to initialized Apollo device struct
 * @param[in] hmc7044   Pointer to initialized HMC7044 device struct (NULL if not used)
 * @param[in] adf4382   Pointer to initialized ADF4382 device struct (NULL if not used)
 * @param[in] adf4030   Pointer to initialized ADF4030 device struct (NULL if not used)
 */
void versal_manual_test_init(adi_apollo_device_t   *apollo,
                             adi_hmc7044_device_t  *hmc7044,
                             adi_adf4382_device_t  *adf4382,
                             adi_adf4030_device_t  *adf4030);

/*!
 * @brief Read a register from the specified device.
 *
 * @param[in]  device_name  Device name string: "APOLLO", "HMC7044", "ADF4382", "ADF4030"
 * @param[in]  address      Register address (32-bit for APOLLO, 16-bit for others)
 * @param[out] data         Pointer to store the read value (8-bit)
 *
 * @return API_CMS_ERROR_OK on success, error code otherwise
 */
int32_t versal_manual_test_read(const char *device_name, uint32_t address, uint8_t *data);

/*!
 * @brief Write a register on the specified device.
 *
 * @param[in] device_name  Device name string: "APOLLO", "HMC7044", "ADF4382", "ADF4030"
 * @param[in] address      Register address (32-bit for APOLLO, 16-bit for others)
 * @param[in] data         Value to write (8-bit)
 *
 * @return API_CMS_ERROR_OK on success, error code otherwise
 */
int32_t versal_manual_test_write(const char *device_name, uint32_t address, uint8_t data);

/*!
 * @brief Read and print a range of registers from the specified device.
 *
 * @param[in] device_name  Device name string
 * @param[in] start_addr   Start register address
 * @param[in] count        Number of consecutive registers to read
 *
 * @return API_CMS_ERROR_OK on success, error code otherwise
 */
int32_t versal_manual_test_dump(const char *device_name, uint32_t start_addr, uint32_t count);

#endif /* VERSAL_MANUAL_TEST_H */

#endif /* !defined(VERSAL_PLATFORM) */
