/*!
 * \brief     PMBus Commands source file.
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


/*============= I N C L U D E S ============*/
#include "adi_pmbus.h"


/******************************************** GENERIC READ / WRITE APIS FOR SUPPORTED DATA FORMATS ********************************************/

int32_t adi_pmbus_send_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t send_byte)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_send_byte(smbus, device_addr, send_byte);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_reg_write_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t data_byte)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_byte(smbus, device_addr, command, data_byte);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_reg_write_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t data_word)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, command, data_word);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_reg_read_byte(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, command, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_reg_read_word(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, command, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_l11_write(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t l11_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, command, l11_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_l11_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t *l11_val)
{
    uint32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, command, l11_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_l16_write(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t l16_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, command, l16_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_l16_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t command, uint16_t *l16_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, command, l16_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


/********************************************** COMMAND SPECIFIC APIS **********************************************/

int32_t adi_pmbus_page_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t page)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_PAGE, page);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

uint8_t adi_pmbus_page_get( adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_PAGE, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_write_protect_enable(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_WRITE_PROTECT, EN_LEVEL_1 );
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_write_protect_disable(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_WRITE_PROTECT, DISABLE);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_vout_mode_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_VOUT_MODE, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_vout_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t vout_l16)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, ADI_PMBUS_CMD_VOUT_COMMAND, vout_l16);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_vout_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *vout_l16)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_VOUT_COMMAND, vout_l16);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_vout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *vout_read_l16)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_READ_VOUT, vout_read_l16);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_iout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *iout_read_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_READ_IOUT, iout_read_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_pout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *pout_read_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_READ_POUT, pout_read_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_vout_max_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t vout_max_l16)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, ADI_PMBUS_CMD_VOUT_MAX, vout_max_l16);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_vout_max_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *vout_max_l16)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_VOUT_MAX, vout_max_l16);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_ton_delay_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t delay_ms_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, ADI_PMBUS_CMD_TON_DELAY, delay_ms_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_ton_delay_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *delay_ms_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_TON_DELAY, delay_ms_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_ton_rise_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t time_ms_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, ADI_PMBUS_CMD_TON_RISE, time_ms_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_toff_delay_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t delay_ms_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, ADI_PMBUS_CMD_TOFF_DELAY, delay_ms_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_toff_fall_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t time_ms_l11)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_word(smbus, device_addr, ADI_PMBUS_CMD_TOFF_FALL, time_ms_l11);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_operation_set(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t operation)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_OPERATION, operation);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_operation_get(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_OPERATION, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_restore_from_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_send_byte(smbus, device_addr, ADI_PMBUS_CMD_RESTORE_USER_ALL);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_store_to_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_send_byte(smbus, device_addr, ADI_PMBUS_CMD_STORE_USER_ALL);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_wait_idle(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_BUSY;
    uint16_t busyCycles = 1000;
    uint8_t mfr_common = 0;

    while ( busyCycles-- > 0 ) {
        err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_COMMON, &mfr_common);
        ADI_CMS_ERROR_RETURN(err);

        if ( (mfr_common & NOT_BUSY) == NOT_BUSY ) {
            return API_CMS_ERROR_OK;
        }
        err = API_CMS_ERROR_I2C_BUSY;
    }
    return err;
}

int32_t adi_pmbus_lock_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_PMOD_NVM_LOCK;
    uint8_t mfr_unlock = 0;

    err = adi_pmbus_wait_idle(smbus, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_EE_UNLOCK, 0x00);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_pmbus_wait_idle(smbus, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_EE_UNLOCK, &mfr_unlock);
    ADI_CMS_ERROR_RETURN(err);

    // User EEPROM is locked.
    if (mfr_unlock == 0x00)
        return API_CMS_ERROR_OK;
    else
        return API_CMS_ERROR_PMOD_NVM_LOCK;
}

int32_t adi_pmbus_unlock_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_PMOD_NVM_LOCK;
    uint8_t mfr_unlock = 0;

    // Lock NVM just incase it was unlocked previously to avoid unlock error.
    err = adi_pmbus_lock_nvm(smbus, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_pmbus_wait_idle(smbus, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    // To unlock user EEPROM space for MFR_EE_ERASE and MFR_EE_DATA read or write operations with PEC allowed:
    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_EE_UNLOCK, 0x2B);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_EE_UNLOCK, 0xD4);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_pmbus_wait_idle(smbus, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    // Reading MFR_EE_UNLOCK returns the last byte written or zero if the part is locked.
    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_EE_UNLOCK, &mfr_unlock);
    ADI_CMS_ERROR_RETURN(err);

    if (mfr_unlock == 0x00)
        return API_CMS_ERROR_PMOD_NVM_LOCK;
    if (mfr_unlock == 0xD4)
        return API_CMS_ERROR_OK;

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_erase_nvm(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_pmbus_wait_idle(smbus, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_smbus_write_byte(smbus, device_addr, ADI_PMBUS_CMD_MFR_EE_ERASE, 0x2B);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_word_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint16_t *status_word)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_word(smbus, device_addr, ADI_PMBUS_CMD_STATUS_WORD, status_word);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_vout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_vout)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_STATUS_VOUT, status_vout);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_iout_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_iout)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_STATUS_IOUT, status_iout);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_input_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_input)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_STATUS_INPUT, status_input);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_temp_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_temp)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_STATUS_TEMP, status_temp);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_cml_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_cml)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_STATUS_CML, status_cml);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_status_mfr_read(adi_smbus_hal_t *smbus, uint32_t device_addr, uint8_t *status_mfr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_read_byte(smbus, device_addr, ADI_PMBUS_CMD_STATUS_MFR_SPECIFIC, status_mfr);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_pmbus_clear_faults(adi_smbus_hal_t *smbus, uint32_t device_addr)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    err = adi_smbus_send_byte(smbus, device_addr, ADI_PMBUS_CMD_CLEAR_FAULTS);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/


/*! @} */