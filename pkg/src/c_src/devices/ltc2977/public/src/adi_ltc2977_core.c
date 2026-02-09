/*!
 * \brief     LTC2977 Core Functionality.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTC2977_CORE
 * \ingroup     LTC2977
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_ltc2977_core.h"
#include "adi_pmbus.h"

/*==================== P U B L I C   A P I   C O D E ====================*/
int32_t adi_ltc2977_core_version_get(adi_ltc2977_device_t *ltc2977, adi_ltc2977_version_t *apiVersion)
{
    apiVersion->major = 0;
    apiVersion->minor = 1;
    apiVersion->patch = 0;

    return API_CMS_ERROR_OK;
}


int32_t adi_ltc2977_core_vout_set(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t vout[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr = ltc2977->device_i2c_addr;
    adi_smbus_hal_t *smbus_hal = &ltc2977->smbus;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(vout);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_vout_set(smbus_hal, device_addr, vout[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_vout_measure(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t read_vout[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr = ltc2977->device_i2c_addr;
    adi_smbus_hal_t *smbus_hal = &ltc2977->smbus;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(read_vout);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_vout_read(smbus_hal, device_addr, &read_vout[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_ton_delay_set(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t ton_delay[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr = ltc2977->device_i2c_addr;
    adi_smbus_hal_t *smbus_hal = &ltc2977->smbus;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(ton_delay);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_ton_delay_set(smbus_hal, device_addr, ton_delay[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_ton_rise_set(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t ton_rise[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr = ltc2977->device_i2c_addr;
    adi_smbus_hal_t *smbus_hal = &ltc2977->smbus;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(ton_rise);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_ton_rise_set(smbus_hal, device_addr, ton_rise[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_toff_delay_set(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t toff_delay[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr = ltc2977->device_i2c_addr;
    adi_smbus_hal_t *smbus_hal = &ltc2977->smbus;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(toff_delay);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_toff_delay_set(smbus_hal, device_addr, toff_delay[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_wait_ms(adi_ltc2977_device_t *ltc2977, uint32_t delay_ms)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(ltc2977->hal_info.delay_us);

    err = ltc2977->hal_info.delay_us(ltc2977->hal_info.user_data, delay_ms * 1000);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_enable_channels(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint32_t num_page_channels, bool ch_enable, bool use_sequence_off, bool use_global_page)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr = ltc2977->device_i2c_addr;
    adi_smbus_hal_t *smbus_hal = &ltc2977->smbus;
    uint8_t global_page = 0xFF;
    uint8_t turn_off = (use_sequence_off == true) ? SOFT_OFF : TURN_OFF_IMMEDIATELY;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    // Turn ON
    if (ch_enable == true) {
        // All channels programmed at once.
        if (use_global_page == true) {
            err = adi_pmbus_page_set(smbus_hal, device_addr, global_page);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_pmbus_operation_set(smbus_hal, device_addr, TURN_ON);
            ADI_CMS_ERROR_RETURN(err);
        }
        // One Channel at a time.
        else if (use_global_page == false) {
            for (i = 0; i < num_page_channels; ++i) {
                err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
                ADI_CMS_ERROR_RETURN(err);
                err = adi_pmbus_operation_set(smbus_hal, device_addr, TURN_ON);
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }

    // Turn OFF
    else if (ch_enable == false) {
        // All channels programmed at once.
        if (use_global_page == true) {
            err = adi_pmbus_page_set(smbus_hal, device_addr, global_page);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_pmbus_operation_set(smbus_hal, device_addr, turn_off);
            ADI_CMS_ERROR_RETURN(err);
        }
        // One Channel at a time.
        else if (use_global_page == false) {
            for (i = 0; i < num_page_channels; ++i) {
                err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
                ADI_CMS_ERROR_RETURN(err);
                err = adi_pmbus_operation_set(smbus_hal, device_addr, turn_off);
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_power_up(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t ton_delay[], uint16_t ton_rise[], uint32_t num_page_channels, bool use_global_on)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(ltc2977->smbus.user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(ton_delay);
    ADI_CMS_NULL_PTR_CHECK(ton_rise);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    // Set TON_RISE in all cases.
    err = adi_ltc2977_core_ton_rise_set(ltc2977, page, ton_rise, num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    // Set TON_DELAY in all cases.
    err = adi_ltc2977_core_ton_delay_set(ltc2977, page, ton_delay, num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    // Turn ON happens after TON_DELAY command generated delay and channels are paged globally.
    if (use_global_on == true) {
        err = adi_ltc2977_core_enable_channels(ltc2977, 0x00, 0, true, false, use_global_on);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Turn ON happens after TON_DELAY command generated delay and all channels are paged in an order.
    else if (use_global_on == false) {
        err = adi_ltc2977_core_enable_channels(ltc2977, page, num_page_channels, true, false, use_global_on );
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_power_down(adi_ltc2977_device_t *ltc2977, uint8_t page[], uint16_t toff_delay[], uint32_t num_page_channels, bool use_sequence_off, bool use_global_off)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(ltc2977->smbus.user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(toff_delay);
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 8);

    // Set TOFF_DELAY in all cases.
    err = adi_ltc2977_core_toff_delay_set(ltc2977, page, toff_delay, num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    // Turn OFF happens after TOFF_DELAY command generated delay and channels are paged globally.
    if (use_global_off == true) {
        err = adi_ltc2977_core_enable_channels(ltc2977, 0x00, 0, false, use_sequence_off, use_global_off);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Turn OFF happens after TOFF_DELAY command generated delay and all channels are paged in an order.
    else if (use_global_off == false) {
        err = adi_ltc2977_core_enable_channels(ltc2977, page, num_page_channels, false, use_sequence_off, use_global_off );
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ltc2977_core_device_id_get(adi_ltc2977_device_t *ltc2977, uint16_t *device_id, uint8_t *is_ltc2977) {
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    adi_smbus_hal_t *smbus_hal;
    uint32_t device_addr;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(ltc2977->smbus.user_data);
    ADI_CMS_NULL_PTR_CHECK(is_ltc2977);

    device_addr = ltc2977->device_i2c_addr;
    smbus_hal = &ltc2977->smbus;

    err = adi_pmbus_page_set(smbus_hal, device_addr, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_pmbus_reg_read_word(smbus_hal, device_addr, ADI_PMBUS_CMD_MFR_SPECIAL_ID, device_id);
    ADI_CMS_ERROR_RETURN(err);

    *is_ltc2977 = (*device_id == 0x0131);

    return API_CMS_ERROR_OK;
}

/*! @} */
