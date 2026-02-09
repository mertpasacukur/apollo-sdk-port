/*!
 * \brief     LTC2980 PMBus Commands source file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTC2980_PMBUS
 * \ingroup     LTC2980
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_ltc2980_pmbus.h"
#include "adi_pmbus.h"

static uint32_t calc_sub_device_addr(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id);


/*==================== P U B L I C   A P I   C O D E ====================*/

int32_t adi_ltc2980_pmbus_send_byte(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint8_t send_byte)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_send_byte(smbus_hal, device_addr, send_byte);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_reg_write_byte(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint8_t data_byte)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_reg_write_byte(smbus_hal, device_addr, command, data_byte);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_reg_write_word(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint16_t data_word)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_reg_write_word(smbus_hal, device_addr, command, data_word);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_reg_read_byte(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_reg_read_byte(smbus_hal, device_addr, command, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_reg_read_word(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint16_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_reg_read_word(smbus_hal, device_addr, command, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_l11_write(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint16_t l11_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_l11_write(smbus_hal, device_addr, command, l11_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_l11_read(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint16_t *l11_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_l11_read(smbus_hal, device_addr, command, l11_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_l16_write(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint16_t l16_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_l16_write(smbus_hal, device_addr, command, l16_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_l16_read(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint16_t command, uint16_t *l16_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_l16_read(smbus_hal, device_addr, command, l16_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_page_set(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint8_t page)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_page_set(smbus_hal, device_addr, page);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

uint8_t adi_ltc2980_pmbus_page_get(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_page_get(smbus_hal, device_addr, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_write_protect_enable(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_write_protect_enable(smbus_hal, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_write_protect_disable(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_write_protect_disable(smbus_hal, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_vout_mode_get(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_vout_mode_get(smbus_hal, device_addr, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_operation_set(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint8_t operation)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;
    
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_operation_set(smbus_hal, device_addr, operation);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2980_pmbus_operation_get(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id, uint8_t *return_val)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltc2980);

    device_addr = calc_sub_device_addr(ltc2980, sub_dev_id);
    smbus_hal = &ltc2980->smbus;

    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);

    err = adi_pmbus_operation_get(smbus_hal, device_addr, return_val);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}
/********************************************************************************************************************/


static uint32_t calc_sub_device_addr(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    // Use a switch statement to return the appropriate sub-device address
    switch (sub_dev_id) {
        case ADI_LTC2980_SUB_DEVICE_A:
            return ltc2980->device_a_i2c_addr;
        case ADI_LTC2980_SUB_DEVICE_B:
            return ltc2980->device_b_i2c_addr;
        case ADI_LTC2980_SUB_DEVICE_C:
            return ltc2980->device_c_i2c_addr;
        default:
            return 0;   // This should not be reached, but handle it just in case
    }
}
/*! @} */
