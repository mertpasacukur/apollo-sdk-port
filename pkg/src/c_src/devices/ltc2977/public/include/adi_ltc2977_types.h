/*!
 * \brief     LTC2977 Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTC2977_TYPES
 * \ingroup     LTC2977
 * @{
 */

#ifndef __ADI_LTC2977_TYPES_H__
#define __ADI_LTC2977_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_smbus_types.h"

/*============= D E F I N E S ==============*/

/**
 * \brief LTC2977 API Version.
 */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_ltc2977_version_t;

#ifndef CLIENT_IGNORE

/**
 * \brief  Platform dependent delay for specified number of microseconds.
 *
 * \param[in]   dev_obj         Pointer to platform specific data
 * \param[in]   us              Time to delay/sleep in microseconds
 *
 * \return 0 for success
 * \return Any non-zero value indicates an error
 */
typedef int32_t(*adi_ltc2977_delay_us_t)(void *dev_obj, uint32_t us);

/**
 * \brief LTC2977 HAL Structure.
 */
typedef struct {
    void *                        user_data;    /*!< Pointer to generic user data. */
    adi_ltc2977_delay_us_t        delay_us;     /*!< Function Pointer to HAL delay function. \ref adi_ltc2977_delay_us_t */
} adi_ltc2977_hal_t;

/**
 * \brief LTC2977 Device Structure.
 */
typedef struct {
    adi_ltc2977_hal_t   hal_info;           /*!< LTC2977 HAL Structure. \ref adi_ltc2977_hal_t */
    adi_smbus_hal_t     smbus;              /*!< SMBus Protocol HAL (Hardware Abstract Layer) Structure. \ref adi_smbus_hal_t */
    uint32_t            device_i2c_addr;    /*!< LTC2977 I2C device address. */
} adi_ltc2977_device_t;

#endif

#endif // __ADI_LTC2977_TYPES_H__

/*! @} */