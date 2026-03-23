#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     Common functions for Apollo CE board power modules
 *
 * \copyright copyright(c) 2025 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_ads10_apollo_ex_power_modules.h"

/*============= D E F I N E S ==============*/
#define ADI_LTC2977_NUM_CHANNELS       8
#define ADI_LTC2980_NUM_CHANNELS       8
#define ADI_LTM4681_NUM_CHANNELS       2


static void ltc2980_a_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels);
static void ltc2980_b_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels);
static void ltc2980_c_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t vout_mode, uint8_t *ltc_page, uint16_t *ltc_read_vout, uint32_t ltc_num_page_channels);


/*==================== P U B L I C   A P I   C O D E ====================*/

int32_t adi_ex_pwr_modules_hal_config(ce_brd_pwr_modules_t *pwr_modules)
{
    int32_t err;
    adi_ltc2977_device_t *ltc2977 = &pwr_modules->ltc2977;
    adi_ltc2980_device_t *ltc2980 = &pwr_modules->ltc2980;
    adi_ltm4681_device_t *ltm4681 = &pwr_modules->ltm4681;

    ADI_CMS_NULL_PTR_CHECK(pwr_modules);
    ADI_CMS_NULL_PTR_CHECK(ltc2977);
    ADI_CMS_NULL_PTR_CHECK(ltc2980);
    ADI_CMS_NULL_PTR_CHECK(ltm4681);

    // Configure LTC2977 HAL
    ltc2977->hal_info.delay_us = &ads10_wait_us;
    ltc2977->smbus.read = &ads10_i2c_read;
    ltc2977->smbus.write = &ads10_i2c_write;
    ltc2977->smbus.xfer = NULL;

    err = ads10_ltc2977_hal_config_data(ltc2977);
    ADI_CMS_ERROR_RETURN(err);

    // Configure LTC2980 HAL
    ltc2980->hal_info.delay_us = &ads10_wait_us;
    ltc2980->smbus.read = &ads10_i2c_read;
    ltc2980->smbus.write = &ads10_i2c_write;
    ltc2980->smbus.xfer = NULL;

    err = ads10_ltc2980_hal_config_data(ltc2980);
    ADI_CMS_ERROR_RETURN(err);

    // Configure LTM4681 HAL
    ltm4681->hal_info.delay_us = &ads10_wait_us;
    ltm4681->smbus.read = &ads10_i2c_read;
    ltm4681->smbus.write = &ads10_i2c_write;
    ltm4681->smbus.xfer = NULL;

    err = ads10_ltm4681_hal_config_data(ltm4681);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_ex_pwr_modules_ltc2977_voltage_telemetry_print(adi_ltc2977_device_t *ltc2977)
{
    int32_t err;
    uint32_t ltc_num_page_channels = ADI_LTC2977_NUM_CHANNELS;
    uint8_t ltc_page[ADI_LTC2977_NUM_CHANNELS] = {0,1,2,3,4,5,6,7};
    uint8_t ltc_vout_mode = 0;
    uint16_t ltc_vout_l16[ADI_LTC2977_NUM_CHANNELS] = {0};
    float ltc_read_vout[ADI_LTC2977_NUM_CHANNELS] = {0.0};
    uint16_t vin_l11 = 0;
    float vin = 0;

    ADI_CMS_NULL_PTR_CHECK(ltc2977);

    printf("\nLTC2977 Voltage Monitoring.\n");

    err = adi_ltc2977_pmbus_l11_read(ltc2977, 0x88, &vin_l11);    // 0x88 = PMBUS_CMD_READ_VIN
    ADI_CMS_ERROR_RETURN(err);

    adi_pmbus_format_conv_l11_to_float(&vin_l11, &vin, 1);
    printf("LTC2977_READ_VIN: %.4f V.\n", vin);

    err = adi_ltc2977_pmbus_vout_mode_get(ltc2977, &ltc_vout_mode);
    ADI_CMS_ERROR_RETURN(err);
    ltc_vout_mode &= 0x1F;

    err = adi_ltc2977_core_vout_measure(ltc2977, ltc_page, ltc_vout_l16, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l16_to_float(ltc_vout_mode, ltc_vout_l16, ltc_read_vout, ltc_num_page_channels);

    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_VOUT: %.4f V.\n", ltc_read_vout[i]);
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ex_pwr_modules_ltc2980_voltage_telemetry_print(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    int32_t err;
    uint32_t ltc_num_page_channels = ADI_LTC2980_NUM_CHANNELS;
    uint8_t ltc_page[ADI_LTC2980_NUM_CHANNELS] = {0,1,2,3,4,5,6,7};
    uint8_t ltc_vout_mode = 0;
    uint16_t ltc_vout_l16[ADI_LTC2980_NUM_CHANNELS] = {0};
    float ltc_read_vout[ADI_LTC2980_NUM_CHANNELS] = {0.0};
    uint16_t vin_l11 = 0;
    float vin = 0;
    char *ltc_sub_dev_id = (sub_dev_id == ADI_LTC2980_SUB_DEVICE_A) ? "A" :
                           (sub_dev_id == ADI_LTC2980_SUB_DEVICE_B) ? "B" : "C";

    ADI_CMS_NULL_PTR_CHECK(ltc2980);
    ADI_CMS_INVALID_PARAM_CHECK(sub_dev_id >= ADI_LTC2980_NUM_SUB_DEVICE);

    printf("\nLTC2980_%s Voltage Monitoring.\n", ltc_sub_dev_id);
    err = adi_ltc2980_pmbus_l11_read(ltc2980, sub_dev_id, 0x88, &vin_l11);    // 0x88 = PMBUS_CMD_READ_VIN
    ADI_CMS_ERROR_RETURN(err);

    adi_pmbus_format_conv_l11_to_float(&vin_l11, &vin, 1);
    printf("LTC2980_%s_READ_VIN: %.4f V.\n", ltc_sub_dev_id, vin);

    err = adi_ltc2980_pmbus_vout_mode_get(ltc2980, sub_dev_id, &ltc_vout_mode);
    ADI_CMS_ERROR_RETURN(err);
    ltc_vout_mode &= 0x1F;

    err = adi_ltc2980_core_vout_measure(ltc2980, sub_dev_id, ltc_page, ltc_vout_l16, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l16_to_float(ltc_vout_mode, ltc_vout_l16, ltc_read_vout, ltc_num_page_channels);

    switch (sub_dev_id) {
        case ADI_LTC2980_SUB_DEVICE_A:
           ltc2980_a_telemetry(ltc2980, &ltc_page[0], &ltc_read_vout[0], ltc_num_page_channels);
           break;
        case ADI_LTC2980_SUB_DEVICE_B:
           ltc2980_b_telemetry(ltc2980, &ltc_page[0], &ltc_read_vout[0], ltc_num_page_channels);
           break;
        case ADI_LTC2980_SUB_DEVICE_C:
           ltc2980_c_telemetry(ltc2980, ltc_vout_mode, &ltc_page[0], &ltc_vout_l16[0], ltc_num_page_channels);
           break;
        default:
            printf("Invalid sub device %d\n", sub_dev_id);
            return API_CMS_ERROR_INVALID_PARAM;
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ex_pwr_modules_ltm4681_power_telemetry_print(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id)
{
    int32_t err;
    char *ltm_chan_id = (chan_id == ADI_LTM4681_CHAN_01) ? "01" : "23";
    uint32_t ltm_num_page_channels = ADI_LTM4681_NUM_CHANNELS;
    uint8_t ltm_page[ADI_LTM4681_NUM_CHANNELS] = {0,1};
    uint8_t ltm_vout_mode = 0;
    uint16_t ltm_vout_l16[ADI_LTM4681_NUM_CHANNELS] = {0.0};
    float ltm_read_vout[ADI_LTM4681_NUM_CHANNELS] = {0.0};
    uint16_t ltm_iout_l11[ADI_LTM4681_NUM_CHANNELS] = {0.0};
    float ltm_read_iout[ADI_LTM4681_NUM_CHANNELS] = {0.0};
    uint16_t ltm_pout_l11[ADI_LTM4681_NUM_CHANNELS] = {0.0};
    float ltm_read_pout[ADI_LTM4681_NUM_CHANNELS] = {0.0};
    uint16_t vin_l11 = 0;
    float vin = 0;

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));

    printf("\nLTM4681_%s Power Monitoring.\n", ltm_chan_id);
    err = adi_ltm4681_pmbus_l11_read(ltm4681, chan_id, 0x88, &vin_l11);    // 0x88 = PMBUS_CMD_READ_VIN
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(&vin_l11, &vin, 1);
    printf("LTM4681_%s_READ_VIN: %.4f V.\n", ltm_chan_id, vin);

    err = adi_ltm4681_pmbus_vout_mode_get(ltm4681, chan_id, &ltm_vout_mode);
    ADI_CMS_ERROR_RETURN(err);
    ltm_vout_mode &= 0x1F;

    err = adi_ltm4681_core_vout_measure(ltm4681, chan_id, ltm_page, ltm_vout_l16, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l16_to_float(ltm_vout_mode, ltm_vout_l16, ltm_read_vout, ltm_num_page_channels);

    err = adi_ltm4681_core_iout_measure(ltm4681, chan_id, ltm_page, ltm_iout_l11, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(ltm_iout_l11, ltm_read_iout, ltm_num_page_channels);

    err = adi_ltm4681_core_pout_measure(ltm4681, chan_id, ltm_page, ltm_pout_l11, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(ltm_pout_l11, ltm_read_pout, ltm_num_page_channels);

    for (uint8_t i = 0; i < ltm_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltm_page[i]);
        printf("READ_VOUT: %.4f V.\t", ltm_read_vout[i]);
        printf("READ_IOUT: %.4f A.\t", ltm_read_iout[i]);
        printf("READ_POUT: %.4f W.\n", ltm_read_pout[i]);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}


int32_t adi_ex_pwr_modules_ltc2977_status_faults_check_print(adi_ltc2977_device_t *ltc2977)
{
    int32_t err;
    uint32_t ltc_num_page_channels = ADI_LTC2977_NUM_CHANNELS;
    uint8_t ltc_page[ADI_LTC2977_NUM_CHANNELS] = {0,1,2,3,4,5,6,7};
    adi_ltc2977_fault_status_t ltc_read_status[ADI_LTC2977_NUM_CHANNELS] = {{0}};

    ADI_CMS_NULL_PTR_CHECK(ltc2977);

    printf("\nLTC2977 Fault Monitoring.\n");
    err = adi_ltc2977_core_fault_status_get(ltc2977, ltc_page, ltc_read_status, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t i = 0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d. STATUS CMD: \t", i, ltc_page[i]);
        printf("_WORD: 0x%04X  _VOUT: 0x%02X  _INPUT: 0x%02X  ",
                ltc_read_status[i].status_word, ltc_read_status[i].status_vout, ltc_read_status[i].status_input);
        printf("_TEMP: 0x%02X  _CML: 0x%02X  _MFR: 0x%02X.\n",
                ltc_read_status[i].status_temp, ltc_read_status[i].status_cml, ltc_read_status[i].status_mfr);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}


int32_t adi_ex_pwr_modules_ltc2980_status_faults_check_print(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    int32_t err;
    char *ltc_sub_dev_id = (sub_dev_id == ADI_LTC2980_SUB_DEVICE_A) ? "A" :
                           (sub_dev_id == ADI_LTC2980_SUB_DEVICE_B) ? "B" : "C";
    uint32_t ltc_num_page_channels = ADI_LTC2980_NUM_CHANNELS;
    uint8_t ltc_page[ADI_LTC2980_NUM_CHANNELS] = {0,1,2,3,4,5,6,7};
    adi_ltc2980_fault_status_t ltc_read_status[ADI_LTC2980_NUM_CHANNELS] = {{0}};

    ADI_CMS_NULL_PTR_CHECK(ltc2980);
    ADI_CMS_INVALID_PARAM_CHECK(sub_dev_id >= ADI_LTC2980_NUM_SUB_DEVICE);

    printf("\nLTC2980_%s Fault Monitoring.\n", ltc_sub_dev_id);
    err = adi_ltc2980_core_fault_status_get(ltc2980, sub_dev_id, ltc_page, ltc_read_status, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t i = 0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d. STATUS CMD: \t", i, ltc_page[i]);
        printf("_WORD: 0x%04X  _VOUT: 0x%02X  _INPUT: 0x%02X  ",
                ltc_read_status[i].status_word, ltc_read_status[i].status_vout, ltc_read_status[i].status_input);
        printf("_TEMP: 0x%02X  _CML: 0x%02X  _MFR: 0x%02X.\n",
                ltc_read_status[i].status_temp, ltc_read_status[i].status_cml, ltc_read_status[i].status_mfr);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}



int32_t adi_ex_pwr_modules_ltm4681_status_faults_check_print(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id)
{
    int32_t err = API_CMS_ERROR_ERROR;
    char *ltm_chan_id = (chan_id == ADI_LTM4681_CHAN_01) ? "01" : "23";
    uint32_t ltm_num_page_channels = ADI_LTM4681_NUM_CHANNELS;
    uint8_t ltm_page[ADI_LTM4681_NUM_CHANNELS] = {0,1};
    adi_ltm4681_fault_status_t ltm_read_status[ADI_LTM4681_NUM_CHANNELS] = {{0}};

    ADI_CMS_NULL_PTR_CHECK(ltm4681);
    ADI_CMS_INVALID_PARAM_CHECK((chan_id != ADI_LTM4681_CHAN_01) && (chan_id != ADI_LTM4681_CHAN_23));

    printf("\nLTM4681_%s Fault Monitoring.\n", ltm_chan_id);
    err = adi_ltm4681_core_fault_status_get(ltm4681, chan_id, ltm_page, ltm_read_status, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t i = 0; i < ltm_num_page_channels; ++i) {
        printf("Page[%d]: %d. STATUS CMD: \t", i, ltm_page[i]);
        printf("_WORD: 0x%04X  _VOUT: 0x%02X  _IOUT: 0x%02X  ",
                ltm_read_status[i].status_word, ltm_read_status[i].status_vout, ltm_read_status[i].status_iout);
        printf("_INPUT: 0x%02X  _TEMP: 0x%02X  _CML: 0x%02X  ",
                ltm_read_status[i].status_input, ltm_read_status[i].status_temp, ltm_read_status[i].status_cml);
        printf("_MFR: 0x%02X.\n", ltm_read_status[i].status_mfr);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}


/********************************************************************************************************************/

static void ltc2980_a_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels) {
    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_VOUT: %.4f V.\n", ltc_read_vout[i]);
    }
}

static void ltc2980_b_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels) {
    const float voltage_current_scalars[8] = {3.3003299, 3.3003299, 2.5, 2.5, 15.6232166, 0.0, 0.0, 500.0};
    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_IOUT: %.9f A.\n", ltc_read_vout[i] * voltage_current_scalars[i]);
    }
}

// For device C, some channels are set to high resolution (see mfr_config_adc_hires bit in mfr_config), so they have L11 format with mA as unit
static void ltc2980_c_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t vout_mode, uint8_t *ltc_page, uint16_t *ltc_read_vout, uint32_t ltc_num_page_channels) {
    const float voltage_current_scalars[8] = {0.0, 500.0, 3.3003299, 200.0, 0.0, 20.0, 0.0, 5.0};
    float value;
    char *unit;
    uint16_t mfr_cfg[8];
    adi_ltc2980_core_mfr_config_get(ltc2980, ADI_LTC2980_SUB_DEVICE_C, ltc_page, mfr_cfg, ltc_num_page_channels);

    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        if (mfr_cfg[i] & 0x200) {
            adi_pmbus_format_conv_l11_to_float(&ltc_read_vout[i], &value, 1);
            unit = "mA";
        } else {
            adi_pmbus_format_conv_l16_to_float(vout_mode, &ltc_read_vout[i], &value, 1);
            unit = "A";
        }
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_IOUT: %.4f %s.\n", value * voltage_current_scalars[i], unit);
    }
}

#endif /* !defined(VERSAL_PLATFORM) */
