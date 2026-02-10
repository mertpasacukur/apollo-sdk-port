/*!
 * \brief     ADS10 Apollo examples common inspect functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__linux__)
#include <unistd.h>
#endif

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_apollo_arm_types.h"
#include "adi_fpga_apollo_types.h"
#include "adi_ads10_apollo_ex_clk.h"

/*!
* \brief Structure containing core data path info
*/
typedef struct {
    uint64_t cnco_freq_hz;
    uint64_t fnco_freq_hz;
    uint64_t nco_freq_hz;
    uint64_t data_rate_hz;
} adi_ads10_apollo_rx_channel_info_t;

#ifndef __ADI_ADS10_APOLLO_COMMON_EX_INSPECT_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_INSPECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Print the state of all JRx links
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_inspect_jrx_link_all(adi_apollo_device_t *device);

/**
 * \brief  Print the state of all JTx links
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_inspect_jtx_link_all(adi_apollo_device_t *device);

/**
 * \brief  Print the state of all InverseSinc channels
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_inspect_invsinc_all(adi_apollo_device_t *device);

/**
 * \brief  Print the state of all ADC channels
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_inspect_adc_all(adi_apollo_device_t *device);

/**
 * \brief Print out lanes rates and calculate appropriate dividers for FPGA
 * 
 * \param[in] fpga_device   Pointer to the FPGA device data structure \ref adi_fpga_apollo_device_t
 * \param[in] profile       Apollo device profile \ref adi_apollo_top_t
 * \param[in] clk_mod       clock setup \ref adi_ads10_apollo_clk_mode_e
 * 
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_inspect_lane_rates(adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t *profile, adi_ads10_apollo_clk_mode_e clk_mode);

/**
 * \brief Returns current channel information (hardcoded for side A channel 0)
 *
 * \param[in]   device           Pointer to the device data structure
 * \param[in]   profile          Pointer to profile data structure
 * \param[out]  channel_info     Calculated channel information
 *
 * \return API_CMS_ERROR_OK    API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_apollo_ex_inspect_rx_channel_get(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_ads10_apollo_rx_channel_info_t *channel_info);

/**
 * \brief Returns tone frequency for given ratio (hardcoded for side A channel 0)
 *
 * \param[in]   device           Pointer to the device data structure
 * \param[in]   profile          Pointer to profile data structure
 * \param[in]   ratio            Tone ratio of generated signal
 * \param[out]  tone_freq_mhz    Frequency of tone for given ratio
 *
 * \return API_CMS_ERROR_OK    API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_apollo_ex_inspect_tx_freq_get(adi_apollo_device_t *device, adi_apollo_top_t *profile, double ratio, double *tone_freq_mhz);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_INSPECT_H__ */
