/*!
 * \brief     LTM4681 Core Functionality.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTM4681_CORE
 * \ingroup     LTM4681
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_ltm4681_core.h"
#include "adi_ltm4681_pmbus.h"
#include "adi_pmbus.h"


/*==================== P U B L I C   A P I   C O D E ====================*/

int32_t adi_ltm4681_core_version_get(adi_ltm4681_device_t *ltm4681, adi_ltm4681_version_t *apiVersion)
{
    apiVersion->major = 0;
    apiVersion->minor = 1;
    apiVersion->patch = 0;

    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_vout_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t vout[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(vout);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_vout_set(smbus_hal, device_addr, vout[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_vout_measure(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t read_vout[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t vout_adc_ctrl = 0;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(read_vout);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        vout_adc_ctrl = ( (i == 1) ? 0x09 : 0x05 );
        err = adi_pmbus_reg_write_byte(smbus_hal, device_addr, ADI_PMBUS_CMD_MFR_ADC_CONTROL, vout_adc_ctrl);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ltm4681_core_wait_ms(ltm4681, 16);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_vout_read(smbus_hal, device_addr, &read_vout[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_reg_write_byte(smbus_hal, device_addr, ADI_PMBUS_CMD_MFR_ADC_CONTROL, 0x00);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ltm4681_core_wait_ms(ltm4681, 50);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_iout_measure(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t read_iout[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t iout_adc_ctrl = 0;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(read_iout);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        iout_adc_ctrl = ( (i == 1) ? 0x0A : 0x06 );
        err = adi_pmbus_reg_write_byte(smbus_hal, device_addr, ADI_PMBUS_CMD_MFR_ADC_CONTROL, iout_adc_ctrl);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ltm4681_core_wait_ms(ltm4681, 16);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_iout_read(smbus_hal, device_addr, &read_iout[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_reg_write_byte(smbus_hal, device_addr, ADI_PMBUS_CMD_MFR_ADC_CONTROL, 0x00);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ltm4681_core_wait_ms(ltm4681, 50);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_pout_measure(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t read_pout[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(read_pout);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_pout_read(smbus_hal, device_addr, &read_pout[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_ton_delay_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t ton_delay[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(ton_delay);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_ton_delay_set(smbus_hal, device_addr, ton_delay[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_ton_rise_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t ton_rise[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(ton_rise);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_ton_rise_set(smbus_hal, device_addr, ton_rise[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_toff_delay_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t toff_delay[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(toff_delay);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_toff_delay_set(smbus_hal, device_addr, toff_delay[i]);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_wait_ms(adi_ltm4681_device_t *ltm4681, uint32_t delay_ms)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    ADI_CMS_NULL_PTR_CHECK(ltm4681->hal_info.delay_us);

    err = ltm4681->hal_info.delay_us(NULL, delay_ms * 1000);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_reset(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);

    err = adi_ltm4681_pmbus_send_byte(ltm4681, chan_id, ADI_PMBUS_CMD_MFR_RESET);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_enable_channels(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint32_t num_page_channels, bool ch_enable, bool use_sequence_off, bool use_global_page)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint8_t global_page = 0xFF;
    uint8_t turn_off = (use_sequence_off == true) ? SOFT_OFF : TURN_OFF_IMMEDIATELY;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
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

int32_t adi_ltm4681_core_power_up(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t ton_delay[], uint16_t ton_rise[], uint32_t num_page_channels, bool use_global_on)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(ton_delay);
    ADI_CMS_NULL_PTR_CHECK(ton_rise);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    // Set TON_RISE in all cases.
    err = adi_ltm4681_core_ton_rise_set(ltm4681, chan_id, page, ton_rise, num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    // Set TON_DELAY in all cases.
    err = adi_ltm4681_core_ton_delay_set(ltm4681, chan_id, page, ton_delay, num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    // Turn ON happens after TON_DELAY command generated delay and channels are paged globally.
    if (use_global_on == true) {
        err = adi_ltm4681_core_enable_channels(ltm4681, chan_id, 0x00, 0, true, false, use_global_on);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Turn ON happens after TON_DELAY command generated delay and all channels are paged in an order.
    else if (use_global_on == false) {
        err = adi_ltm4681_core_enable_channels(ltm4681, chan_id, page, num_page_channels, true, false, use_global_on);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_power_down(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t toff_delay[], uint32_t num_page_channels, bool use_sequence_off, bool use_global_off)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(toff_delay);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);

    // Set TOFF_DELAY in all cases.
    err = adi_ltm4681_core_toff_delay_set(ltm4681, chan_id, page, toff_delay, num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    // Turn ON happens after TON_DELAY command generated delay and channels are paged globally.
    if (use_global_off == true) {
        err = adi_ltm4681_core_enable_channels(ltm4681, chan_id, 0x00, 0, false, use_sequence_off, use_global_off);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Turn ON happens after TON_DELAY command generated delay and all channels are paged in an order.
    else if (use_global_off == false) {
        err = adi_ltm4681_core_enable_channels(ltm4681, chan_id, page, num_page_channels, false, use_sequence_off, use_global_off);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_fault_status_get(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], adi_ltm4681_fault_status_t status[], uint32_t num_page_channels)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;
    uint8_t i;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));
    ADI_CMS_INVALID_PARAM_CHECK(num_page_channels > 2);
    ADI_CMS_NULL_PTR_CHECK(page);
    ADI_CMS_NULL_PTR_CHECK(status);

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    for (i = 0; i < num_page_channels; ++i) {
        err = adi_pmbus_page_set(smbus_hal, device_addr, page[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_word_read(smbus_hal, device_addr, &status[i].status_word);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_vout_read(smbus_hal, device_addr, &status[i].status_vout);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_iout_read(smbus_hal, device_addr, &status[i].status_iout);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_input_read(smbus_hal, device_addr, &status[i].status_input);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_temp_read(smbus_hal, device_addr, &status[i].status_temp);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_cml_read(smbus_hal, device_addr, &status[i].status_cml);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_pmbus_status_mfr_read(smbus_hal, device_addr, &status[i].status_mfr);
        ADI_CMS_ERROR_RETURN(err);

    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ltm4681_core_fault_status_clear(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint32_t device_addr;
    adi_smbus_hal_t *smbus_hal;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    smbus_hal = &ltm4681->smbus;
    ADI_CMS_NULL_PTR_CHECK(smbus_hal);
    ADI_CMS_NULL_PTR_CHECK(smbus_hal->user_data);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));

    device_addr = (chan_id == ADI_LTM4681_CHAN_01) ? ltm4681->chan_01_i2c_addr : ltm4681->chan_23_i2c_addr;
    err = adi_pmbus_page_set(smbus_hal, device_addr, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_pmbus_clear_faults(smbus_hal, device_addr);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

/*! @} */
