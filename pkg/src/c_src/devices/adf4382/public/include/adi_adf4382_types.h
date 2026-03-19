/*!
 * \brief     ADF4382 Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_TYPES_H__
#define __ADI_ADF4382_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include "adi_cms_api_common.h"

#ifndef CLIENT_IGNORE

typedef int32_t(*adi_adf4382_spi_write_t)(void *user_data, const uint8_t *in_data, uint32_t size_bytes, adi_cms_hal_txn_config_t *txn_config);
typedef int32_t(*adi_adf4382_spi_read_t)(void *user_data, const uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes, adi_cms_hal_txn_config_t *txn_config);
typedef int32_t(*adi_adf4382_delay_us_t)(void *user_data, uint32_t us);

/*!
 * \brief ADF4382 registers access descriptors
 */
typedef struct {
    void *                        user_data;
    adi_adf4382_spi_write_t       spi_write;            /*!< Function Pointer to HAL SPI write function */
    adi_adf4382_spi_read_t        spi_read;             /*!< Function Pointer to HAL SPI read function */
    adi_adf4382_delay_us_t        delay_us;             /*!< Function Pointer to HAL delay function */
} adi_adf4382_hal_t;

/*!
 * \brief Device Structure
 */
typedef struct {
    adi_adf4382_hal_t hal_info;
} adi_adf4382_device_t;

/**
 * \brief ADF4382 Chip Version. Used to select default reg initialization values.
 */
typedef enum {
    ADI_ADF4382_CHIP_VER_AUTO = 0,  /*! ADF4382 Chip Version is determined by reading 0x67 reg. */
    ADI_ADF4382_CHIP_VER_U2 = 4,    /*! ADF4382 Chip Version is selected as U2. */
    ADI_ADF4382_CHIP_VER_U4 = 7,    /*! ADF4382 Chip Version is selected as U4. */
    ADI_ADF4382_CHIP_VER_U5_A = 11, /*! ADF4382 Chip Version is selected as U5. */
    ADI_ADF4382_CHIP_VER_U5_B = 12, /*! ADF4382 Chip Version is selected as U5. */
    ADI_ADF4382_CHIP_VER_U5_C = 13,  /*! ADF4382 Chip Version is selected as U5. */
    ADI_ADF4382_CHIP_VER_U5_D = 18,  /*! ADF4382 Chip Version is selected as U5. */

} adi_adf4382_chip_ver_sel_e;

typedef enum {
    ADI_ADF4382_CLK1_SEL = 0,
    ADI_ADF4382_CLK2_SEL = 1
} adi_adf4382_clk_sel_e;
#endif

#endif // !__ADI_ADF4382_TYPES_H__
