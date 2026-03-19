/*!
 * \brief     ADL6332 Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADL6332_TYPES_H__
#define __ADI_ADL6332_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include "adi_cms_api_common.h"

typedef enum {
    ADI_ADL6332_CHIP_ID_0   = 0x01,
    ADI_ADL6332_CHIP_ID_1   = 0x02,
    ADI_ADL6332_CHIP_ID_2   = 0x04,
    ADI_ADL6332_CHIP_ID_3   = 0x08,
    ADI_ADL6332_CHIP_ID_4   = 0x10,
    ADI_ADL6332_CHIP_ID_5   = 0x20,
    ADI_ADL6332_CHIP_ID_6   = 0x40,
    ADI_ADL6332_CHIP_ID_7   = 0x80,
    ADI_ADL6332_CHIP_ID_ALL = 0xFF
} adi_adl6332_chip_id_e;

#ifndef CLIENT_IGNORE

typedef int32_t(*adi_adl6332_spi_write_t)(void *user_data, const uint8_t *in_data, uint32_t size_bytes, adi_cms_hal_txn_config_t *txn_config);
typedef int32_t(*adi_adl6332_spi_read_t)(void *user_data, const uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes, adi_cms_hal_txn_config_t *txn_config);
typedef int32_t(*adi_adl6332_delay_us_t)(void *user_data, uint32_t us);

/*!
 * \brief ADL6332 HAL descriptor
 */
typedef struct {
    void *                        user_data;
    adi_adl6332_spi_write_t       spi_write;            /*!< Function Pointer to HAL SPI write function */
    adi_adl6332_spi_read_t        spi_read;             /*!< Function Pointer to HAL SPI read function */
    adi_adl6332_delay_us_t        delay_us;             /*!< Function Pointer to HAL delay function */
} adi_adl6332_hal_t;

/*!
 * \brief Device Structure
 */
typedef struct {
    adi_adl6332_hal_t             hal_info;
    uint8_t                       rev;                  /*!< Eval Board revision */
} adi_adl6332_device_t;

#endif

#endif // !__ADI_ADL6332_TYPES_H__
