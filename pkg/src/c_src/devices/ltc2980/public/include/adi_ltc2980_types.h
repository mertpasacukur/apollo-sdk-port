/*!
 * \brief     LTC2980-24 Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTC2980_TYPES
 * \ingroup     LTC2980
 * @{
 */

#ifndef __ADI_LTC2980_TYPES_H__
#define __ADI_LTC2980_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_smbus_types.h"

/*============= D E F I N E S ==============*/

/**
 * \brief LTC2980 API Version.
 */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_ltc2980_version_t;

/**
 * \brief LTC2980 Fault status struct
 */
typedef struct {
    uint16_t status_word;       /*!< Two byte summary of the unit’s fault condition. CMD Code: 0x79 */
    uint8_t status_vout;        /*!< Output voltage fault and warning status. CMD Code: 0x7A */
    uint8_t status_input;       /*!< Input voltage fault and warning status. CMD Code: 0x7C */
    uint8_t status_temp;        /*!< Temperature fault and warning status. CMD Code: 0x7D */
    uint8_t status_cml;         /*!< Communication and memory fault and warning status. CMD Code: 0x7E */
    uint8_t status_mfr;         /*!< Manufacturer specific fault and state information. CMD Code: 0x80 */
} adi_ltc2980_fault_status_t;

/*!
 * \brief   The LTC2980-24 contains three independent LTC2977 devices. 
            Select between sub-device A, B or C for configuration.
 */
typedef enum {
    ADI_LTC2980_SUB_DEVICE_A    = 0x00,
    ADI_LTC2980_SUB_DEVICE_B    = 0x01,
    ADI_LTC2980_SUB_DEVICE_C    = 0x02,
    ADI_LTC2980_NUM_SUB_DEVICE  = 0x03
} adi_ltc2980_sub_device_id_e;

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
typedef int32_t(*adi_ltc2980_delay_us_t)(void *dev_obj, uint32_t us);

/**
 * \brief LTC2980 HAL Structure.
 */
typedef struct {
    void *                        user_data;    /*!< Pointer to generic user data. */
    adi_ltc2980_delay_us_t        delay_us;     /*!< Function Pointer to HAL delay function. \ref adi_ltc2980_delay_us_t */
} adi_ltc2980_hal_t;

/**
 * \brief LTC2980 Device Structure.
 */
typedef struct {
    adi_ltc2980_hal_t   hal_info;               /*!< LTC2980 HAL Structure. \ref adi_ltc2980_hal_t */
    adi_smbus_hal_t     smbus;                  /*!< SMBus Protocol HAL (Hardware Abstract Layer) Structure. \ref adi_smbus_hal_t */
    uint32_t            device_a_i2c_addr;      /*!< LTC2980 sub-device A I2C address. */
    uint32_t            device_b_i2c_addr;      /*!< LTC2980 sub-device B I2C address. */
    uint32_t            device_c_i2c_addr;      /*!< LTC2980 sub-device C I2C address. */
} adi_ltc2980_device_t;

#endif

#endif // __ADI_LTC2980_TYPES_H__

/*! @} */