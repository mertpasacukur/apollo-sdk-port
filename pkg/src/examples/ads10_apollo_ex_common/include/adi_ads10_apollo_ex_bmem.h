/*!
 * \brief     ADS10 Apollo examples BMEM functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <stdio.h>
#include <stdlib.h>

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_ads10_apollo_ex_types.h"

#ifndef __ADI_ADS10_APOLLO_EX_BMEM_H__
#define __ADI_ADS10_APOLLO_EX_BMEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Standard BMEM AWG configuration
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] bmem_sel      Which BMEMs to target. \ref adi_apollo_bmem_sel_e for options
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_bmem_awg_config(adi_apollo_device_t* device, uint16_t bmem_sel);

/**
 * \brief  Write vector to BMEMs
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] bmem_sel      Which BMEMs to target. \ref adi_apollo_bmem_sel_e for options
 * \param[in] tone_ratio    Ratio of the BMEM tone to write
 * \param[in] backoff       Backoff in dB [note: expects negative number]
 * \param[in] debug         If true, writes vector to a file for analysis
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_bmem_awg_tone_write(adi_apollo_device_t* device, uint16_t bmem_sel, double tone_ratio, double backoff, bool debug);

/**
 * \brief  Read BMEM
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] bmem_sel      Which BMEMs to target. \ref adi_apollo_bmem_sel_e for options
 * \param[in] file_name     File name base for selected bmem 
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_bmem_capture(adi_apollo_device_t* device, uint8_t bmem_sel, char *file_name);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_BMEM_H__ */
