/*!
 * \brief     PMBus Protocol Types header file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup PMBUS
 * @{
 */

#ifndef __ADI_PMBUS_TYPES_H__
#define __ADI_PMBUS_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"
#include "adi_smbus_types.h"

/*============= D E F I N E S ==============*/

#define ADI_PMBUS_CMD_PAGE                        0x00
#define ADI_PMBUS_CMD_OPERATION                   0x01
#define ADI_PMBUS_CMD_ON_OFF_CONFIG               0x02
#define ADI_PMBUS_CMD_CLEAR_FAULTS                0x03
#define ADI_PMBUS_CMD_PAGE_PLUS_WRITE             0x05
#define ADI_PMBUS_CMD_PAGE_PLUS_READ              0x06
#define ADI_PMBUS_CMD_WRITE_PROTECT               0x10
#define ADI_PMBUS_CMD_STORE_USER_ALL              0x15
#define ADI_PMBUS_CMD_RESTORE_USER_ALL            0x16
#define ADI_PMBUS_CMD_CAPABILITY                  0x19
#define ADI_PMBUS_CMD_MFR_COMPARE_USER_ALL        0xF0
#define ADI_PMBUS_CMD_SMBALERT_MASK               0x1B
#define ADI_PMBUS_CMD_VOUT_MODE                   0x20
#define ADI_PMBUS_CMD_VOUT_COMMAND                0x21
#define ADI_PMBUS_CMD_VOUT_MAX                    0x24
#define ADI_PMBUS_CMD_VOUT_MARGIN_HIGH            0x25
#define ADI_PMBUS_CMD_VOUT_MARGIN_LOW             0x26
#define ADI_PMBUS_CMD_VIN_ON                      0x35
#define ADI_PMBUS_CMD_VIN_OFF                     0x36
#define ADI_PMBUS_CMD_VOUT_OV_FAULT_LIMIT         0x40
#define ADI_PMBUS_CMD_VOUT_OV_FAULT_RESPONSE      0x41
#define ADI_PMBUS_CMD_VOUT_OV_WARN_LIMIT          0x42
#define ADI_PMBUS_CMD_VOUT_UV_WARN_LIMIT          0x43
#define ADI_PMBUS_CMD_VOUT_UV_FAULT_LIMIT         0x44
#define ADI_PMBUS_CMD_VOUT_UV_FAULT_RESPONSE      0x45
#define ADI_PMBUS_CMD_IOUT_OC_FAULT_LIMIT         0x46
#define ADI_PMBUS_CMD_IOUT_OC_WARN_LIMIT          0x4A
#define ADI_PMBUS_CMD_OT_FAULT_LIMIT              0x4F
#define ADI_PMBUS_CMD_OT_FAULT_RESPONSE           0x50
#define ADI_PMBUS_CMD_OT_WARN_LIMIT               0x51
#define ADI_PMBUS_CMD_UT_WARN_LIMIT               0x52
#define ADI_PMBUS_CMD_UT_FAULT_LIMIT              0x53
#define ADI_PMBUS_CMD_UT_FAULT_RESPONSE           0x54
#define ADI_PMBUS_CMD_VIN_OV_FAULT_LIMIT          0x55
#define ADI_PMBUS_CMD_VIN_OV_FAULT_RESPONSE       0x56
#define ADI_PMBUS_CMD_VIN_OV_WARN_LIMIT           0x57
#define ADI_PMBUS_CMD_VIN_UV_WARN_LIMIT           0x58
#define ADI_PMBUS_CMD_VIN_UV_FAULT_LIMIT          0x59
#define ADI_PMBUS_CMD_VIN_UV_FAULT_RESPONSE       0x5A
#define ADI_PMBUS_CMD_TON_DELAY                   0x60
#define ADI_PMBUS_CMD_TON_RISE                    0x61
#define ADI_PMBUS_CMD_TON_MAX_FAULT_LIMIT         0x62
#define ADI_PMBUS_CMD_TON_MAX_FAULT_RESPONSE      0x63
#define ADI_PMBUS_CMD_TOFF_DELAY                  0x64
#define ADI_PMBUS_CMD_TOFF_FALL                   0x65
#define ADI_PMBUS_CMD_TOFF_MAX_WARN_LIMIT         0x66
#define ADI_PMBUS_CMD_IIN_OC_WARN_LIMIT           0x5D
#define ADI_PMBUS_CMD_STATUS_BYTE                 0x78
#define ADI_PMBUS_CMD_STATUS_WORD                 0x79
#define ADI_PMBUS_CMD_STATUS_VOUT                 0x7A
#define ADI_PMBUS_CMD_STATUS_IOUT                 0x7B
#define ADI_PMBUS_CMD_STATUS_INPUT                0x7C
#define ADI_PMBUS_CMD_STATUS_TEMP                 0X7D
#define ADI_PMBUS_CMD_STATUS_CML                  0x7E
#define ADI_PMBUS_CMD_STATUS_MFR_SPECIFIC         0x80
#define ADI_PMBUS_CMD_READ_VIN                    0x88
#define ADI_PMBUS_CMD_READ_IIN                    0x89
#define ADI_PMBUS_CMD_READ_VOUT                   0x8B
#define ADI_PMBUS_CMD_READ_IOUT                   0x8C
#define ADI_PMBUS_CMD_READ_TEMP_1                 0x8D
#define ADI_PMBUS_CMD_READ_TEMP_2                 0x8E
#define ADI_PMBUS_CMD_READ_DUTY_CYCLE             0x94
#define ADI_PMBUS_CMD_READ_FREQ                   0x95
#define ADI_PMBUS_CMD_READ_POUT                   0x96
#define ADI_PMBUS_CMD_READ_PIN                    0x97
#define ADI_PMBUS_CMD_PMBUS_REVISION              0x98
#define ADI_PMBUS_CMD_USER_DATA_03                0xB3
#define ADI_PMBUS_CMD_USER_DATA_04                0xB4
#define ADI_PMBUS_CMD_MFR_STATUS_2                0xB7
#define ADI_PMBUS_CMD_MFR_EE_UNLOCK               0xBD
#define ADI_PMBUS_CMD_MFR_EE_ERASE                0xBE
#define ADI_PMBUS_CMD_MFR_EE_DATA                 0xBF
#define ADI_PMBUS_CMD_MFR_CONFIG_LTC2977          0xD0
#define ADI_PMBUS_CMD_MFR_CONFIG_ALL              0xD1
#define ADI_PMBUS_CMD_MFR_ADC_CONTROL             0xD8
#define ADI_PMBUS_CMD_MFR_WATCHDOG_T_FIRST        0xE2
#define ADI_PMBUS_CMD_MFR_WATCHDOG_T              0xE3
#define ADI_PMBUS_CMD_MFR_PADS                    0xE5
#define ADI_PMBUS_CMD_MFR_ADDRESS                 0xE6
#define ADI_PMBUS_CMD_MFR_SPECIAL_ID              0xE7
#define ADI_PMBUS_CMD_MFR_FAULT_LOG_STORE         0xEA
#define ADI_PMBUS_CMD_MFR_FAULT_LOG_RESTORE       0xEB
#define ADI_PMBUS_CMD_MFR_FAULT_LOG_CLEAR         0xEC
#define ADI_PMBUS_CMD_MFR_READ_IIN                0xED
#define ADI_PMBUS_CMD_MFR_FAULT_LOG_STATUS        0xED
#define ADI_PMBUS_CMD_MFR_FAULT_LOG               0xEE
#define ADI_PMBUS_CMD_MFR_COMMON                  0xEF
#define ADI_PMBUS_CMD_MFR_SPARE_0                 0xF7
#define ADI_PMBUS_CMD_MFR_SPARE_1                 0xF8
#define ADI_PMBUS_CMD_MFR_SPARE_2                 0xF9
#define ADI_PMBUS_CMD_MFR_SPARE_3                 0xFA
#define ADI_PMBUS_CMD_MFR_TEMP_1_GAIN             0xF8
#define ADI_PMBUS_CMD_MFR_TEMP_1_OFFSET           0xF9
#define ADI_PMBUS_CMD_MFR_EEPROM_STATUS           0xF1
#define ADI_PMBUS_CMD_MFR_RAIL_ADDRESS            0xFA
#define ADI_PMBUS_CMD_MFR_RESET                   0xFD

/*============= E N U M S ==============*/

/*!
 * \brief  Enumerate levels of Write Protect restrictions for Power Modules.
 */
typedef enum {
    EN_LEVEL_1     =  0x80,             /*!< Disable all writes except to the WRITE_PROTECT, PAGE, MFR_EE_UNLOCK, and STORE_USER_ALL commands. */
    EN_LEVEL_2     =  0x40,             /*!< Disable all writes except to the WRITE_PROTECT, PAGE, MFR_EE_UNLOCK, STORE_USER_ALL, OPERATION, MFR_COMMAND_PLUS, MFR_PAGE_FF_MASK and CLEAR_FAULTS commands. */
    DISABLE        =  0x00              /*!< Enable writes to all commands. */
} adi_pmbus_protocol_write_protect_e;

/*!
 * \brief  Enumerate ON or OFF operation for Power Modules.
 */
typedef enum {
    TURN_OFF_IMMEDIATELY    =  0x00,    /*!< Turn off output immediately. No sequencing.  */
    TURN_ON                 =  0x80,    /*!< Turn on output at nominal. */
    MARGIN_LOW              =  0x98,    /*!<  */
    MARGIN_HIGH             =  0xA8,    /*!<  */
    SOFT_OFF                =  0x40     /*!< Turn off in sequence. Soft off. */
} adi_pmbus_protocol_operation_e;

/*!
 * \brief   Enumerate status information for the alert pin (ALERTB),
 *          share-clock pin (SHARE_CLK), write-protect pin (WP), and device busy state.
 */
typedef enum {
    ALERTB_HIGH         =   1 << 7,    /*!< ALERTB is de-asserted high.  */
    NOT_BUSY            =   1 << 6,    /*!< The device is available to process PMBus commands. */
    NOT_PENDING         =   1 << 5,    /*!< Calculations Not Pending. */
    NOT_IN_TRANSITION   =   1 << 4,    /*!< Device Outputs Not in Transition. */
    SHARE_CLK_LOW       =   1 << 1,    /*!< Share-clock pin is being held low. */
    WRITE_PROTECT_HIGH  =   1 << 0     /*!< Write-protect pin is high. */
} adi_pmbus_protocol_mfr_common_e;

#endif /*__ADI_PMBUS_TYPES_H__*/

/*! @} */