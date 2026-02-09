/*!
 * \brief     LTC2977 PMBus Commands header file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTC2977_PMBUS
 * \ingroup     LTC2977
 * @{
 */

#ifndef __ADI_LTC2977_PMBUS_H__
#define __ADI_LTC2977_PMBUS_H__

/*============= I N C L U D E S =============*/
#include "adi_ltc2977_types.h"
#include "adi_pmbus_types.h"
#include "adi_smbus.h"

/*============= E X P O R T S ==============*/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************** GENERIC READ / WRITE APIS FOR SUPPORTED DATA FORMATS ********************************************/

/**
 * \brief       Send PMbus command and its associated value.
 *              SMBus Protocol Type: Send Byte Format.
 *              Following the device address, a byte of data is sent a.k.a Send Byte.
 *              All or part of the Send Byte may contribute to the command and rest to data.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   send_byte       A byte of data/command to be written.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_send_byte(adi_ltc2977_device_t *ltc2977, uint8_t send_byte);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Write Byte Format.
 *              Following the device address, the first byte sent is command and the next byte is data to be written.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[in]   data_byte       A byte of data to be written.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_reg_write_byte(adi_ltc2977_device_t *ltc2977, uint16_t command, uint8_t data_byte);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Write Word Format.
 *              Following the device address, the first byte sent is command and the next two bytes is data to be written.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[in]   data_word       A word(2 bytes) of data to be written.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_reg_write_word(adi_ltc2977_device_t *ltc2977, uint16_t command, uint16_t data_word);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Read Byte Format.
 *              Following the device address, first the I2C host must write a command to the peripheral device.
 *              Then it must follow that command with a repeated START condition to denote a read from that device’s address.
 *              The peripheral then returns one byte of data.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[out]  return_val      Pointer to variable to store received (1 byte) data.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_reg_read_byte(adi_ltc2977_device_t *ltc2977, uint16_t command, uint8_t *return_val);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Read Word Format.
 *              Following the device address, first the I2C host must write a command to the peripheral device.
 *              Then it must follow that command with a repeated START condition to denote a read from that device’s address.
 *              The peripheral then returns two bytes of data.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[out]  return_val      Pointer to variable to store received (2 bytes) data.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_reg_read_word(adi_ltc2977_device_t *ltc2977, uint16_t command, uint16_t *return_val);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: L11 (LINEAR11).
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[in]   l11_val         Value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_l11_write(adi_ltc2977_device_t *ltc2977, uint16_t command, uint16_t l11_val);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: L11 (LINEAR11).
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[out]  l11_val         Pointer to variable that stores read back value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_l11_read(adi_ltc2977_device_t *ltc2977, uint16_t command, uint16_t *l11_val);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: L16 (LINEAR16).
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[in]   l16_val         Value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_l16_write(adi_ltc2977_device_t *ltc2977, uint16_t command, uint16_t l16_val);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: L16 (LINEAR16).
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   command         PMbus command to send.
 * \param[out]  l16_val         Pointer to variable that stores read back value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_l16_read(adi_ltc2977_device_t *ltc2977, uint16_t command, uint16_t *l16_val);


/********************************************** COMMAND SPECIFIC APIS **********************************************/

/**
 * \brief       Set Page to program individual channel.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   page            Page value corresponding to a channel.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_page_set(adi_ltc2977_device_t *ltc2977, uint8_t page);

/**
 * \brief       Get Page of current channel.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[out]  return_val      Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.

 */
uint8_t adi_ltc2977_pmbus_page_get(adi_ltc2977_device_t *ltc2977, uint8_t *return_val);

/**
 * \brief       Enable write protect to provide protection against accidental programming.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_write_protect_enable(adi_ltc2977_device_t *ltc2977);

/**
 * \brief       Disable write protect.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_write_protect_disable(adi_ltc2977_device_t *ltc2977);

/**
 * \brief       Get the mode and exponent for all commands with an L16 data format.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[out]  return_val      Pointer to variable that stores VOUT_MODE value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_vout_mode_get(adi_ltc2977_device_t *ltc2977, uint8_t *return_val);

/**
 * \brief       Set operation action for individual channel.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[in]   operation       Action value corresponding to an operation.
 *                              Refer device datasheet for specific action items.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_operation_set(adi_ltc2977_device_t *ltc2977, uint8_t operation);

/**
 * \brief       Get operation action for individual channel.
 *
 * \param[in]   ltc2977         Context variable - Pointer to the LTC2977 device data structure.
 * \param[out]  return_val      Pointer to variable that stores read back value.
 *                              Refer device datasheet for specific action items.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltc2977_pmbus_operation_get(adi_ltc2977_device_t *ltc2977, uint8_t *return_val);

/********************************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /*__ADI_LTC2977_PMBUS_H__*/

/*! @} */
