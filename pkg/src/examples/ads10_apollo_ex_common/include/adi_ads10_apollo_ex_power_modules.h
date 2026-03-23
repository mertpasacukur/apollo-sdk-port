#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     Common functions for Apollo CE board power modules
 *
 * \copyright copyright(c) 2025 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADS10_APOLLO_EX_PWR_MODULE_H__
#define __ADI_ADS10_APOLLO_EX_PWR_MODULE_H__

/*============= I N C L U D E S ============*/

#include "adi_ads10_apollo_ex_types.h"
#include "ads10_hal.h"
#include "adi_ltc2977_core.h"
#include "adi_ltc2977_pmbus.h"
#include "adi_ltm4681_core.h"
#include "adi_ltm4681_pmbus.h"
#include "adi_ltc2980_core.h"
#include "adi_ltc2980_pmbus.h"
#include "adi_pmbus_format_conv.h"

/*!
 * \brief Apollo CE board power modules device struct
 */
typedef struct {
    adi_ltc2977_device_t ltc2977;
    adi_ltc2980_device_t ltc2980;
    adi_ltm4681_device_t ltm4681;
} ce_brd_pwr_modules_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Initializes power modules device HAL.
 *
 * \param[in]   pwr_modules             Context variable - Pointer to the CE board power modules device data structure
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_hal_config(ce_brd_pwr_modules_t *pwr_modules);


/**
 * \brief   Reads measured output voltage for every LTC2977 output channel.
 *          The measured reading is converted in volts and printed on console.
 *
 * \param[in]   ltc2977             Context variable - Pointer to the LTC2977 device data structure.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_ltc2977_voltage_telemetry_print(adi_ltc2977_device_t *ltc2977);


/**
 * \brief   Reads measured output voltage for every LTC2980 output channel.
 *          The measured reading is converted in volts (or amperes based on setup) and printed on console.
 *
 * \param[in]   ltc2980             Context variable - Pointer to the LTC2980 device data structure.
 * \param[in]   sub_dev_id          Sub-device selection ID. Select between sub-device A, B or C for configuration. \ref adi_ltc2980_sub_device_id_e.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_ltc2980_voltage_telemetry_print(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id);


/**
 * \brief   Reads measured output voltage, current and power for every LTM4681 output channel.
 *          The measured reading is converted in volts, amperes and watts and printed on console.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_ltm4681_power_telemetry_print(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id);


/**
 * \brief   Read status commands that summarizes most critical faults or warnings for LTC2977 and prints on console.
 *
 * \param[in]   ltc2977             Context variable - Pointer to the LTC2977 device data structure.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_ltc2977_status_faults_check_print(adi_ltc2977_device_t *ltc2977);


/**
 * \brief   Read status commands that summarizes most critical faults or warnings for LTC2980 and prints on console.
 *
 * \param[in]   ltc2980             Context variable - Pointer to the LTC2980 device data structure.
 * \param[in]   sub_dev_id          Sub-device selection ID. Select between sub-device A, B or C for configuration. \ref adi_ltc2980_sub_device_id_e.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_ltc2980_status_faults_check_print(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id);


/**
 * \brief   Read status commands that summarizes most critical faults or warnings for LTM4681 and prints on console.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ex_pwr_modules_ltm4681_status_faults_check_print(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_PWR_MODULE_H__ */

#endif /* !defined(VERSAL_PLATFORM) */
