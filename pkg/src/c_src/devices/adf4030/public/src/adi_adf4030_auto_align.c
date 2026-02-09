/*!
 * \brief     Source file implementing APIs for ADF4030's Auto Alignment functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_AUTO_ALIGN
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_auto_align.h"
#include "adi_adf4030_bf.h"

/*============= D E F I N E S ==============*/
#define ADI_ADF4030_ALIGN_CYCLES_MIN_COUNT          (0U)
#define ADI_ADF4030_ALIGN_CYCLES_MAX_COUNT          (7U)
#define ADI_ADF4030_ADEL_M_STEP_IN_FEMTO_SEC        (1400U)     // Analog adjustable delay is typically in 1.4 ps steps.
#define ADI_ADF4030_ADEL_M_MAX_STEP_SIZE            (0x3FU)     // ADEL_M bitfield is 6 bit wide. (2^6 - 1).
#define ADI_ADF4030_MAX_THRESHOLD_IN_FEMTO_SEC      (ADI_ADF4030_ADEL_M_MAX_STEP_SIZE * ADI_ADF4030_ADEL_M_STEP_IN_FEMTO_SEC)

/*============= C O D E ====================*/


int32_t adi_adf4030_auto_align_delay_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e tdc_target_channel, uint64_t ref_input_freq, int64_t delay_in_fs)
{
    int32_t err;
    uint64_t vco_freq = 0;
    int64_t vco_period_in_fs = 0;
    int16_t tdc_offset = 0;
    uint8_t ndiv_value = 0;
    uint8_t rdiv_value = 0;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(tdc_target_channel, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);
    ADI_ADF4030_VALIDATE_RANGE(ref_input_freq, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);

    err = adi_adf4030_bf___REG0056___NDIV_get(adf4030, &ndiv_value);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0057___RDIV_get(adf4030, &rdiv_value);
    ADI_ADF4030_CHECK_ERR_OK(err);

    // Calculating VCO Frequency
    vco_freq = (ref_input_freq / rdiv_value) * ndiv_value;

    // Creating calculation space to avoid overflowing registers
    vco_freq = vco_freq / 1000000;

    // VCO Period in Femtoseconds
    vco_period_in_fs = 1000000000 / vco_freq;
    vco_period_in_fs /= (1 << 9);

    tdc_offset = delay_in_fs / vco_period_in_fs;

    switch (tdc_target_channel) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG001d___TDC_OFFSET0_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG001f___TDC_OFFSET1_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG0021___TDC_OFFSET2_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG0023___TDC_OFFSET3_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG0025___TDC_OFFSET4_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG0027___TDC_OFFSET5_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG0029___TDC_OFFSET6_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG002b___TDC_OFFSET7_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG002d___TDC_OFFSET8_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG002f___TDC_OFFSET9_set(adf4030, tdc_offset);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }

    err = adi_adf4030_bf___REG0011___TDC_CLK_SEL_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_auto_align_iteration_set(adi_adf4030_device_t *adf4030, uint8_t iter_number)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(iter_number, ADI_ADF4030_ALIGN_CYCLES_MIN_COUNT, ADI_ADF4030_ALIGN_CYCLES_MAX_COUNT);

    if (iter_number == 0) {
        err = adi_adf4030_bf___REG0037___EN_AUTO_ITER_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);

        return API_CMS_ERROR_OK;
    }

    err = adi_adf4030_bf___REG0011___TDC_CLK_SEL_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___EN_CYCS_RED_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0035___AUTO_ALIGN_THOLD_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___EN_AUTO_ITER_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___ALIGN_CYCLES_set(adf4030, iter_number);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_auto_align_threshold_set(adi_adf4030_device_t *adf4030, uint32_t threshold_in_fs)
{
    int32_t err;
    uint8_t threshold = 0;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(threshold_in_fs, 0, ADI_ADF4030_MAX_THRESHOLD_IN_FEMTO_SEC);

    if (threshold_in_fs == 0) {
        err = adi_adf4030_bf___REG0037___EN_AUTO_ITER_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);

        return API_CMS_ERROR_OK;
    }

    threshold = threshold_in_fs / ADI_ADF4030_ADEL_M_STEP_IN_FEMTO_SEC;

    err = adi_adf4030_bf___REG0011___TDC_CLK_SEL_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___EN_CYCS_RED_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___EN_AUTO_ITER_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0035___AUTO_ALIGN_THOLD_set(adf4030, threshold);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_auto_align_single_channel_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e tdc_source_channel, adi_adf4030_channel_id_e tdc_target_channel)
{
    int32_t err;
    uint8_t read_data = 0;
    uint16_t timeout = 0xEFF;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(tdc_source_channel, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);
    ADI_ADF4030_VALIDATE_RANGE(tdc_target_channel, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);

    err = adi_adf4030_bf___REG0011___MANUAL_MODE_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___EN_AUTO_ALIGN_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___EN_SERIAL_ALIGN_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___TDC_SOURCE_set(adf4030, tdc_source_channel);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0010___TDC_TARGET_set(adf4030, tdc_target_channel);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG008f___FSM_BUSY_get(adf4030, &read_data);
    ADI_ADF4030_CHECK_ERR_OK(err);

    while (timeout--) {
        if (read_data) {
            err = adi_adf4030_bf___REG008f___FSM_BUSY_get(adf4030, &read_data);
            ADI_ADF4030_CHECK_ERR_OK(err);
        } else {
            return API_CMS_ERROR_OK;
        }

        adf4030->hal_info.delay_us(NULL, 2000); // 2 ms Delay
    }

    return API_CMS_ERROR_OPERATION_TIMEOUT;
}


int32_t adi_adf4030_auto_align_multi_channel_set(adi_adf4030_device_t *adf4030, uint16_t multi_ch_sel)
{
    int32_t err;
    uint16_t bit_counter_buffer = multi_ch_sel;
    uint8_t bit_number = 0;
    uint8_t tdc_source = 0;

    while (bit_counter_buffer > 0) {
        bit_number += bit_counter_buffer & 1;
        bit_counter_buffer >>= 1;
        tdc_source++;
    }

    if (bit_number < 1) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    tdc_source--;

    err = adi_adf4030_bf___REG0011___MANUAL_MODE_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___EN_AUTO_ALIGN_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___TDC_SOURCE_set(adf4030, tdc_source);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___TDC_CLK_SEL_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0035___BSYNC_CAL_ON_set(adf4030, multi_ch_sel);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0037___EN_SERIAL_ALIGN_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0010___TDC_TARGET_set(adf4030, tdc_source);
    ADI_ADF4030_CHECK_ERR_OK(err);

    uint8_t read_data = 0;
    uint16_t timeout = 0xEFF;
    err = adi_adf4030_bf___REG008f___FSM_BUSY_get(adf4030, &read_data);
    ADI_ADF4030_CHECK_ERR_OK(err);

    while (timeout--) {
        if (read_data) {
            err = adi_adf4030_bf___REG008f___FSM_BUSY_get(adf4030, &read_data);
            ADI_ADF4030_CHECK_ERR_OK(err);
        } else {
            return API_CMS_ERROR_OK;
        }

        adf4030->hal_info.delay_us(NULL, 2000); // 2 ms Delay
    }

    return API_CMS_ERROR_OPERATION_TIMEOUT;
}


/*! @} */
