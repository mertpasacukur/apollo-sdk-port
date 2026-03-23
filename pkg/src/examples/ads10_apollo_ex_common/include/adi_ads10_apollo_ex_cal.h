/*!
 * \brief     ADS10 Apollo examples calibration functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <stdio.h>
#include <stdlib.h>

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_ads10_apollo_ex_types.h"

#ifndef __ADI_ADS10_APOLLO_EX_CAL_H__
#define __ADI_ADS10_APOLLO_EX_CAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Run select calibrations
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] profile       Pointer to device profile
 * \param[in] cals          Select calibrations to run. \ref adi_ads10_apollo_cal_e for options
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_cals_run(adi_apollo_device_t* device, adi_apollo_top_t *profile, uint16_t cals);

/**
 * \brief  Checks if SERDES Init (Foreground) cal needs to be run
 *
 * \param[in] profile       Pointer to device profile
 *
 * \return True: Run serdes Init cal
 */
bool adi_ads10_apollo_ex_run_serdes_init_cal_get(adi_apollo_top_t *profile);

/**
 * \brief  Checks if SERDES Tracking (background) cal needs to be run
 *
 * \param[in] profile       Pointer to device profile
 *
 * \return True: Run serdes BG cal
 */
bool adi_ads10_apollo_ex_run_serdes_track_cal_get(adi_apollo_top_t *profile);

/**
 * \brief   Read the current ADC calibration data from device and save to a file. BG cal is aborted before calData dump.
 *
 * \param[in] device            Context variable - Pointer to the APOLLO device data structure
 * \param[in] file_name         String of binary file with its full path
 * \param[in] adc_cal_chans     ADC selection mask
 * \param[in] mode              ADC mode selection. Random(0) or Sequential(1)
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adc_cal_data_dump_to_file(adi_apollo_device_t *device, char *file_name, uint16_t adc_cal_chans, uint8_t mode);


/**
 * \brief   Loads ADC calibration data from a file. BG cal is aborted before loading calData and later initialize to WARMBOOT_FROM_USER.
 *
 * \param[in] device            Context variable - Pointer to the APOLLO device data structure
 * \param[in] file_name         String of binary file with its full path
 * \param[in] adc_cal_chans     ADC selection mask
 * \param[in] mode              ADC mode selection. Random(0) or Sequential(1)
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adc_cal_data_reload_from_file(adi_apollo_device_t *device, char *file_name, uint16_t adc_cal_chans, uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_CAL_H__ */
