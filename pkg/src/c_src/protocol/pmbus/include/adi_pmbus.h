/*!
 * \brief     PMBus Commands header file.
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

#ifndef __ADI_PMBUS_H__
#define __ADI_PMBUS_H__

/*============= I N C L U D E S =============*/
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
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   send_byte       A byte of data/command to be written.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_send_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t send_byte);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Write Byte Format.
 *              Following the device address, the first byte sent is command and the next byte is data to be written.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[in]   data_byte       A byte of data to be written.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_reg_write_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t data_byte);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Write Word Format.
 *              Following the device address, the first byte sent is command and the next two bytes is data to be written.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[in]   data_word       A word(2 bytes) of data to be written.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_reg_write_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t data_word);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Read Byte Format.
 *              Following the device address, first the I2C host must write a command to the peripheral device.
 *              Then it must follow that command with a repeated START condition to denote a read from that device’s address.
 *              The peripheral then returns one byte of data.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[out]  return_val      Pointer to variable to store received (1 byte) data.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_reg_read_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t *return_val);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: Reg (Register). SMBus Protocol Type: Read Word Format.
 *              Following the device address, first the I2C host must write a command to the peripheral device.
 *              Then it must follow that command with a repeated START condition to denote a read from that device’s address.
 *              The peripheral then returns two bytes of data.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[out]  return_val      Pointer to variable to store received (2 bytes) data.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_reg_read_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t *return_val);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: L11 (LINEAR11).
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[in]   l11_val         Value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_l11_write(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t l11_val);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: L11 (LINEAR11).
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[out]  l11_val         Pointer to variable that stores read back value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_l11_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t *l11_val);

/**
 * \brief       Send PMbus command and its associated value.
 *              Command Data Format: L16 (LINEAR16).
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[in]   l16_val         Value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_l16_write(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t l16_val);

/**
 * \brief       Read data associated to a command.
 *              Command Data Format: L16 (LINEAR16).
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   command         PMbus command to send.
 * \param[out]  l16_val         Pointer to variable that stores read back value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_l16_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t *l16_val);


/********************************************** COMMAND SPECIFIC APIS **********************************************/

/**
 * \brief       Set Page to program individual channel.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   page            Page value corresponding to a channel.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_page_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t page);

/**
 * \brief       Get Page of current channel.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  return_val      Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.

 */
uint8_t adi_pmbus_page_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *return_val);

/**
 * \brief       Enable write protect to provide protection against accidental programming.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_write_protect_enable(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Disable write protect.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_write_protect_disable(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Get the mode and exponent for all commands with an L16 data format.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  return_val      Pointer to variable that stores VOUT_MODE value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_vout_mode_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *return_val);

/**
 * \brief       Set output voltage.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   vout_l16        Output Voltage, in LINEAR16 numeric format, written to VOUT_COMMAND.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_vout_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t vout_l16);

/**
 * \brief       Get value of output voltage set.
 *              Reads back the value set in VOUT_COMMAND and it's not the measured output voltage.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  vout_l16        Pointer to variable that stores VOUT_COMMAND read back value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_vout_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *vout_l16);

/**
 * \brief       Get value of output voltage measured by the ADC.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  vout_read_l16   Pointer to variable that stores READ_VOUT read back value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_vout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *vout_read_l16);

/**
 * \brief       Get value of output current measured by the ADC.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  iout_read_l11   Pointer to variable that stores READ_IOUT read back value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_iout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *iout_read_l11);

/**
 * \brief       Get value of output power measured by the ADC.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  pout_read_l11   Pointer to variable that stores READ_POUT read back value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_pout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *pout_read_l11);

/**
 * \brief       Set maximum output voltage.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   vout_max_l16    Maximum Output Voltage, in LINEAR16 numeric format, written to VOUT_MAX.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_vout_max_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t vout_max_l16);

/**
 * \brief       Get value of maximum output voltage set.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  vout_max_l16    Pointer to variable that stores VOUT_MAX read back value in PMBus's LINEAR16 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_vout_max_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *vout_max_l16);

/**
 * \brief       Set delay in milli-seconds before the rails turns on.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   delay_ms_l11    Delay period in milli-seconds, in LINEAR11 numeric format, written to TON_DELAY.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_ton_delay_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t delay_ms_l11);

/**
 * \brief       Get value of delay in milli-seconds before the rails turns on.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  delay_ms_l11    Pointer to variable that stores TON_DELAY read back value in PMBus's LINEAR11 Numeric Format.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_ton_delay_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *delay_ms_l11);

/**
 * \brief       Set turn on rise time.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   time_ms_l11     Rise time in milli-seconds, in LINEAR11 numeric format, written to TON_RISE.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_ton_rise_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t time_ms_l11);

/**
 * \brief       Set delay in milli-seconds before the rails turns off.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   delay_ms_l11    Delay period in milli-seconds, in LINEAR11 numeric format, written to TOFF_DELAY.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_toff_delay_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t delay_ms_l11);

/**
 * \brief       Set turn off fall time.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   time_ms_l11     Fall time in milli-seconds, in LINEAR11 numeric format, written to TOFF_FALL.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_toff_fall_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t time_ms_l11);

/**
 * \brief       Set operation action for individual channel.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   operation       Action value corresponding to an operation. \ref adi_pmbus_protocol_operation_e for common operation actions.
 *                              Refer device datasheet for specific action items.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_operation_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t operation);

/**
 * \brief       Get operation action for individual channel.
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  return_val      Pointer to variable that stores read back value. \ref adi_pmbus_protocol_operation_e for common operation actions.
 *                              Refer device datasheet for specific action items.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_operation_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *return_val);

/**
 * \brief       Restores entire operating memory (volatile registers / USER RAM) with all the commands from EEPROM Memory (Non-volatile).
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_restore_from_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Stores entire operating memory (volatile registers / USER RAM) to the EEPROM Memory (Non-volatile).
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_store_to_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Waits for the device to be available to process PMBus commands.
 *              Polls (N-times) MFR_COMMON command to check if device is busy or idle.
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_wait_idle(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Locks the NVM.
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_lock_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Unlocks the NVM.
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_unlock_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Erases the entire contents of the user EEPROM space and configures this space to accept new program data.
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_erase_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr);

/**
 * \brief       Get 2-byte fault status value
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_word     Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_word_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *status_word);

/**
 * \brief       Get vout fault status
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_vout     Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_vout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_vout);

/**
 * \brief       Get iout fault status
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_iout     Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_iout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_iout);

/**
 * \brief       Get input fault status
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_input    Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_input_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_input);

/**
 * \brief       Get temperature fault status
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_temp     Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_temp_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_temp);

/**
 * \brief       Get cml (communications, memory and logic) fault status
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_cml     Pointer to variable that stores read back value.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_cml_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_cml);

/**
 * \brief       Get manufacturer specific fault data
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 * \param[out]  status_mfr      Pointer to variable that stores read back value.
 * 
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_status_mfr_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_mfr);
/**
 * \brief       Clear all faults
 *
 * \param[in]   smbus           Context variable - Pointer to the SMBus HAL data structure.
 * \param[in]   device_addr     I2C Device address.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_pmbus_clear_faults(adi_smbus_hal_t *smbus, uint32_t device_addr);

/********************************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /*__ADI_PMBUS_H__*/

/*! @} */
