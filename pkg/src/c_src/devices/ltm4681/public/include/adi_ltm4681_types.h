/*!
 * \brief     LTM4681 Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTM4681_TYPES
 * \ingroup     LTM4681
 * @{
 */

#ifndef __ADI_LTM4681_TYPES_H__
#define __ADI_LTM4681_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_smbus_types.h"

/*============= D E F I N E S ==============*/

/**
 * \brief LTM4681 API Version.
 */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_ltm4681_version_t;

/**
 * \brief LTM4681 Fault status struct
 */
typedef struct {
    uint16_t status_word;       /*!< Two byte summary of the unit’s fault condition. CMD Code: 0x79 */
    uint8_t status_vout;        /*!< Output voltage fault and warning status. CMD Code: 0x7A */
    uint8_t status_iout;        /*!< Output current fault and warning status. CMD Code: 0x7B */
    uint8_t status_input;       /*!< Input voltage fault and warning status. CMD Code: 0x7C */
    uint8_t status_temp;        /*!< Temperature fault and warning status. CMD Code: 0x7D */
    uint8_t status_cml;         /*!< Communication and memory fault and warning status. CMD Code: 0x7E */
    uint8_t status_mfr;         /*!< Manufacturer specific fault and state information. CMD Code: 0x80 */
} adi_ltm4681_fault_status_t;

/*!
 * \brief  Select either channels 0 and 1 or channels 2 and 3 for configuration.
 */
typedef enum {
    ADI_LTM4681_CHAN_01   = 0x01,
    ADI_LTM4681_CHAN_23   = 0x02
} adi_ltm4681_chan_pair_id_e;

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
typedef int32_t(*adi_ltm4681_delay_us_t)(void *dev_obj, uint32_t us);

/**
 * \brief LTM4681 HAL Structure.
 */
typedef struct {
    void *                        user_data;    /*!< Pointer to generic user data. */
    adi_ltm4681_delay_us_t        delay_us;     /*!< Function Pointer to HAL delay function. \ref adi_ltm4681_delay_us_t */
} adi_ltm4681_hal_t;

/**
 * \brief LTM4681 Device Structure.
 */
typedef struct {
    adi_ltm4681_hal_t   hal_info;           /*!< LTM4681 HAL Structure. \ref adi_ltm4681_hal_t */
    adi_smbus_hal_t     smbus;              /*!< SMBus Protocol HAL (Hardware Abstract Layer) Structure. \ref adi_smbus_hal_t */
    uint32_t            chan_01_i2c_addr;   /*!< LTM4681_01 I2C device address. */
    uint32_t            chan_23_i2c_addr;   /*!< LTM4681_23 I2C device address. */
} adi_ltm4681_device_t;

#endif

#endif // __ADI_LTM4681_TYPES_H__

/*! @} */
