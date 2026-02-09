/*!
 * \brief     SMBus Protocol Types header file.
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

#ifndef __ADI_SMBUS_TYPES_H__
#define __ADI_SMBUS_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"


/*============= F U N C T I O N  P O I N T E R S ============*/

/**
 * \brief       Platform dependent I2C Read / Write transaction function pointer.
 *
 * \param[in]       dev_obj             Pointer to platform specific data.
 * \param[in]       device_addr         I2C Device address.
 * \param[in]       reg_addr            Register address / Command for the device.
 * \param[in,out]   data                Array of 8-bit data to send to or receive from I2C Master.
 * \param[in]       data_num_bytes      Size of data used in read / write transaction in bytes.
 * \param[in]       is_read             Indicates if the I2C transaction is read(TRUE) or write(FALSE).
 *
 * \return      0 for success.
 * \return      Any non-zero value indicates an error.
 */
typedef int32_t(*adi_smbus_hal_xfer_t)(void *dev_obj, uint32_t device_addr, uint16_t  reg_addr, uint8_t data[], uint16_t data_num_bytes, bool is_read);


/**
 * \brief       Platform dependent I2C Read Function pointer.
 *
 * \param[in]   dev_obj         Pointer to platform specific data.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   reg_addr        I2C Device Register to be read.
 * \param[out]  data            Read back register value stored in an array of 8-bit data.
 * \param[in]   data_num_bytes  Number of bytes to read.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      <0                  Failed. \ref adi_cms_error_e for details.
 */
typedef int32_t(*adi_smbus_hal_read_t)(void *dev_obj, uint32_t device_addr, uint8_t reg_addr, uint8_t data[], uint16_t data_num_bytes);


/**
 * \brief       Platform dependent I2C Write Function pointer.
 *
 * \param[in]   dev_obj         Pointer to platform specific data.
 * \param[in]   device_addr     I2C Device address.
 * \param[in]   reg_addr        I2C Device Register to be read.
 * \param[in]   data            Data to be written to device's register stored in an array of 8-bit.
 * \param[in]   data_num_bytes  Number of bytes to write.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      <0                  Failed. \ref adi_cms_error_e for details.
 */
typedef int32_t(*adi_smbus_hal_write_t)(void *dev_obj, uint32_t device_addr, uint8_t reg_addr, uint8_t data[], uint16_t data_num_bytes);


/**
 * \brief SMBus Protocol HAL (Hardware Abstract Layer) Structure.
 */
typedef struct {
    void*                   user_data;      /*!< Pointer to SMbus related user data. */
    adi_smbus_hal_read_t    read;           /*!< Platform SMBus read transaction function ptr. \ref adi_smbus_hal_read_t */
    adi_smbus_hal_write_t   write;          /*!< Platform SMBus write transaction function ptr. \ref adi_smbus_hal_write_t */
    adi_smbus_hal_xfer_t    xfer;           /*!< Platform SMBus rd/wr transaction function ptr. \ref adi_smbus_hal_xfer_t */
} adi_smbus_hal_t;

#endif  /*__ADI_SMBUS_TYPES_H__*/

/*! @} */