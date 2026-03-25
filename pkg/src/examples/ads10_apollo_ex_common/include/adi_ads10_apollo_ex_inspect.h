#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples common inspect functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADS10_APOLLO_COMMON_EX_INSPECT_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_INSPECT_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_apollo_arm_types.h"
#include "adi_fpga_apollo_types.h"
#include "adi_ads10_apollo_ex_clk.h"

/*!
* \brief Structure containing core data path info. Contains frequencies and data rates
*/
typedef struct {
    uint64_t cnco_freq_hz;   /* CNCO (Coarse NCO) frequency in Hz */
    uint64_t fnco_freq_hz;   /* FNCO (Fine NCO) frequency in Hz */
    uint64_t nco_freq_hz;    /* Total NCO frequency (CNCO + FNCO) in Hz */
    uint64_t data_rate_hz;   /* Data rate after all decimation/interpolation in Hz */
} adi_ads10_apollo_channel_info_t;

/**
 * \brief Structure containing block selectors for Apollo datapath configuration
 */
typedef struct {
    adi_apollo_blk_sel_t cdxc_sel;      /* CDDC/CDUC block selector; see enum \ref adi_apollo_cddc_select_e and \ref adi_apollo_cduc_select_e for valid values */
    adi_apollo_blk_sel_t cnco_sel;      /* CNCO block selector; see enum \ref adi_apollo_coarse_nco_select_e for valid values */
    adi_apollo_blk_sel_t fnco_sel;      /* FNCO block selector; see enum \ref adi_apollo_fine_nco_select_e for valid values */
    adi_apollo_blk_sel_t fdxc_sel;      /* FDDC/FDUC block selector; see enum \ref adi_apollo_fine_ddc_select_e and \ref adi_apollo_fine_duc_select_e for valid values */
    adi_apollo_blk_sel_t fsrc_sel;      /* FSRC block selector; see enum \ref adi_apollo_fsrc_sel_e for valid values */
} adi_ads10_apollo_channel_selectors_t;

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
int32_t adi_ads10_apollo_ex_inspect_lane_rates(adi_fpga_apollo_device_t* fpga_device,
                                               adi_apollo_top_t *profile,
                                               adi_ads10_apollo_clk_mode_e clk_mode);

/**
 * \brief Returns current RX channel information for a specific datapath configuration.
 *
 * This function inspects the RX datapath using the provided block selectors and returns
 * the calculated channel information (frequencies and data rate) for the selected path.
 *
 * \note Selectors must specify exactly one block per type and all must be on the same side.
 *
 * \param[in]  device            Pointer to the Apollo device data structure
 * \param[in]  profile           Pointer to the profile data structure
 * \param[in]  channel_selectors Block selectors for the RX datapath configuration
 * \param[out] channel_info      Calculated channel information (frequencies, data rate)
 *
 * \retval API_CMS_ERROR_OK      API completed successfully
 * \retval <0                    Failed; see \ref adi_cms_error_e for details
 */
int32_t adi_ads10_apollo_ex_inspect_rx_channel_get(adi_apollo_device_t *device,
                                                   adi_apollo_top_t *profile,
                                                   adi_ads10_apollo_channel_selectors_t channel_selectors,
                                                   adi_ads10_apollo_channel_info_t *channel_info);

/**
 * \brief Returns current TX channel information for a specific datapath configuration.
 *
 * This function inspects the TX datapath using the provided block selectors and returns
 * the calculated channel information (frequencies and data rate) for the selected path.
 *
 * \note Selectors must specify exactly one block per type and all must be on the same side.
 *
 * \param[in]  device            Pointer to the Apollo device data structure
 * \param[in]  profile           Pointer to the profile data structure
 * \param[in]  channel_selectors Block selectors for the TX datapath configuration
 * \param[out] channel_info      Calculated channel information (frequencies, data rate)
 *
 * \retval API_CMS_ERROR_OK      API completed successfully
 * \retval <0                    Failed; see \ref adi_cms_error_e for details
 */
int32_t adi_ads10_apollo_ex_inspect_tx_channel_get(adi_apollo_device_t *device,
                                                   adi_apollo_top_t *profile,
                                                   adi_ads10_apollo_channel_selectors_t channel_selectors,
                                                   adi_ads10_apollo_channel_info_t *channel_info);

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
int32_t adi_ads10_apollo_ex_inspect_tx_freq_get(adi_apollo_device_t *device,
                                                adi_apollo_top_t *profile,
                                                double ratio,
                                                double *tone_freq_mhz);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_INSPECT_H__ */

#endif /* !defined(VERSAL_PLATFORM) */
