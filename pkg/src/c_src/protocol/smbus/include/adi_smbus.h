/*!
 * \brief     SMBus Protocol header file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup SMBUS
 * @{
 */

#ifndef __ADI_SMBUS_H__
#define __ADI_SMBUS_H__

/*============= I N C L U D E S ============*/
#include "adi_smbus_types.h"


/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLIENT_IGNORE

/**
 * \brief       Send PMBus commands in the send byte format.
 *              Following the device address, a byte of data is sent a.k.a Send Byte.
 *              All or part of the Send Byte may contribute to the command and rest to data.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   data            A byte of data to be written.
 *
 * \return      Status of the transaction.
 */
int32_t adi_smbus_send_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t data);


/**
 * \brief       Send PMBus commands in the write byte format.
 *              Following the device address, the first byte sent is command and the next byte is data to be written.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMBus command to send.
 * \param[in]   data_byte       A byte of data to be written.
 *
 * \return      Status of the transaction.
 */
int32_t adi_smbus_write_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t data_byte);


/**
 * \brief       Send PMBus commands in the write word format.
 *              Following the device address, the first sent byte is command and the next two bytes is data to be written.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMBus command to send.
 * \param[in]   data_word       A word(2 bytes) of data to be written.
 *
 * \return      Status of the transaction.
 */
int32_t adi_smbus_write_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t data_word);


/**
 * \brief       Read data from the PMBus device in read byte format.
 *              Following the device address, first the I2C host must write a command to the peripheral device.
 *              Then it must follow that command with a repeated START condition to denote a read from that device’s address.
 *              The peripheral then returns one byte of data.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMBus command to send.
 * \param[out]  return_val      Pointer to variable to store received (1 byte) data.
 *
 * \return      Status of the transaction.
 */
int32_t adi_smbus_read_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t* return_val);


/**
 * \brief       Read data from the PMBus device in read word format.
 *              Following the device address, first the I2C host must write a command to the peripheral device.
 *              Then it must follow that command with a repeated START condition to denote a read from that device’s address.
 *              The peripheral then returns two bytes of data.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMBus command to send.
 * \param[out]  return_val      Pointer to variable to store received (2 bytes) data.
 *
 * \return      Status of the transaction.
 */
int32_t adi_smbus_read_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t* return_val);


#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif

#endif /*__ADI_SMBUS_H__*/

/*! @} */
