/*!
 * \brief     Source file implementing APIs for ADF4030's TDC functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_TDC
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_tdc.h"
#include "adi_adf4030_bf.h"

/*============= C O D E ====================*/


int32_t adi_adf4030_tdc_measure(adi_adf4030_device_t *adf4030,
                                adi_adf4030_channel_id_e tdc_source_channel,
                                adi_adf4030_channel_id_e tdc_target_channel,
                                uint64_t source_out_freq_hz,
                                int64_t *tdc_result)
{

    int32_t err;
    uint8_t tdc_status = 0;
    uint16_t tdc_timeout = 0xEFF;
    uint32_t raw_time_diff = 0; // Normalized TDC ∆time/period Result.
    int64_t time_diff_mod = 0;
    int64_t clean_diff = 0;
    int64_t res_in_fs_temp = 0;
    int64_t res_in_fs_temp3 = 0;
    int64_t	res_in_fs = 0;

    ADI_ADF4030_PTR_CHECK(adf4030);

    err = adi_adf4030_bf___REG0016___CIC_DEC_RATE_set(adf4030, 0x0F);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___MANUAL_MODE_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0011___TDC_SOURCE_set(adf4030, tdc_source_channel);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0010___TDC_TARGET_set(adf4030, tdc_target_channel);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0016___TDC_ARM_M_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    /* Warnings: In the low Frequencies this delay can be larger. Because calculating of tdc takes more time with low frequencies */
    err = adi_adf4030_bf___REG008f___TDC_BUSY_get(adf4030, &tdc_status);
    ADI_ADF4030_CHECK_ERR_OK(err);

    while (tdc_status) {
        adf4030->hal_info.delay_us(adf4030->hal_info.user_data, 1000);
        err = adi_adf4030_bf___REG008f___TDC_BUSY_get(adf4030, &tdc_status);
        ADI_ADF4030_CHECK_ERR_OK(err);

        if (!(tdc_timeout--)) {
            break;
        }
    }

    // Add check if tdc_status is cleared
    if (tdc_status) {
        return API_CMS_ERROR_OPERATION_TIMEOUT;
    }

    err = adi_adf4030_bf___REG0073___TDIFF_MATH_get(adf4030, &raw_time_diff);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0016___TDC_ARM_M_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    time_diff_mod = raw_time_diff % (0x01 << 24); // took first 24 bit (0x01 << 24) = 16777216

    clean_diff = ((time_diff_mod + (1 << 23)) % (1 << 24)) - (1 << 23);

    res_in_fs_temp = ((clean_diff * 1000000000) / ((1 << 24)));
    res_in_fs_temp3 = res_in_fs_temp * 1000000;
    res_in_fs = (res_in_fs_temp3 / ((int64_t)source_out_freq_hz));

    *tdc_result = res_in_fs;

    return API_CMS_ERROR_OK;
}


/*! @} */
