/*!
 * \brief     SMBus Protocol source file.
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


/*============= I N C L U D E S ============*/
#include "adi_smbus.h"

/*==================== C O D E ====================*/


int32_t adi_smbus_send_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t data)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    if (smbus->write != NULL) {
        err = smbus->write(smbus->user_data, device_addr, data, NULL, 0);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = smbus->xfer(smbus->user_data, device_addr, data, NULL, 0, false);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

int32_t adi_smbus_write_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t data_byte)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    if (smbus->write != NULL) {
        err = smbus->write(smbus->user_data, device_addr, command, &data_byte, 1);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = smbus->xfer(smbus->user_data, device_addr, command, &data_byte, 1, false);
        ADI_CMS_ERROR_RETURN(err);
    }
    return err;
}

int32_t adi_smbus_write_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t data_word)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    uint8_t writeData[2] = {0};
    writeData[0] = (uint8_t) data_word & 0xFF;
    writeData[1] = (uint8_t) (data_word >> 8) & 0xFF;

    if (smbus->write != NULL) {
        err = smbus->write(smbus->user_data, device_addr, command, writeData, 2);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = smbus->xfer(smbus->user_data, device_addr, command, writeData, 2, false);
        ADI_CMS_ERROR_RETURN(err);
    }
    return err;
}

int32_t adi_smbus_read_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t* return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    if (smbus->read != NULL) {
        err = smbus->read(smbus->user_data, device_addr, command, return_val, 1);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = smbus->xfer(smbus->user_data, device_addr, command, return_val, 1, true);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

int32_t adi_smbus_read_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t* return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint8_t readBack[2] = {0};

    if (smbus->read != NULL) {
        err = smbus->read(smbus->user_data, device_addr, command, readBack, 2);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = smbus->xfer(smbus->user_data, device_addr, command, readBack, 2, true);
        ADI_CMS_ERROR_RETURN(err);
    }

    *return_val = (uint16_t) (readBack[1] << 8) | (readBack[0]);

    return err;
}

/*! @} */