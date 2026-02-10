/*!
 * \brief     ADS10 Apollo examples common ADF4030 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4030_types.h"
#include "adi_adf4030_hal.h"
#include "adi_adf4030_bf.h"

// Import All ADF4030 Feature Headers
#include "adi_adf4030_auto_align.h"
#include "adi_adf4030_bsync_term.h"
#include "adi_adf4030_bsyncout.h"
#include "adi_adf4030_core.h"
#include "adi_adf4030_ldctrl.h"
#include "adi_adf4030_pdctrl.h"
#include "adi_adf4030_pll.h"
#include "adi_adf4030_tdc.h"

#ifndef __ADI_ADS10_APOLLO_EX_COMMON_ADF4030_H__
#define __ADI_ADS10_APOLLO_EX_COMMON_ADF4030_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Initializes ADF4030 HAL configuration.
 *          Sets up SPI HAL and assigns HAL functions to device func_ptrs.
 *
 * \param[in]   adf4030 [context variable]      Pointer to the device structure.
 * \param[in]   dev_id                          Unique address of device controlled by address pins on the board. \ref adi_adf4030_dev_id_e.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4030_configure_hal(adi_adf4030_device_t *adf4030, uint8_t dev_id);

/**
 * \brief   Calculate PLL/VCO Output Frequency based on reference input and bsync output frequencies.
 *
 * \param[in]   ref_input_freq_hz               Reference Freq (in Hertz) provided to ADF4030.
 * \param[in]   bsync_out_freq_hz               BSYNC Channel output frequency in Hz.
 * \param[out]  vco_out_freq                    PLL/VCO out frequency in Hz.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4030_vco_freq_calc(uint64_t ref_input_freq, uint64_t bsync_out_freq, uint64_t *vco_out_freq);

/**
 * \brief   Initializes ADF4030.
 *          Performs a device soft-reset, loads default registers,
 *          power up internal blocks and configure device PLL/VCO.
 *
 * \param[in]   adf4030 [context variable]      Pointer to the device structure.
 * \param[in]   ref_input_freq_hz               Reference Freq (in Hertz) provided to ADF4030.
 * \param[in]   vco_out_freq_hz                 PLL/VCO Output Freq (in Hertz).
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4030_startup(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq_hz, uint64_t vco_out_freq_hz);

/**
 * \brief   Configure BSYNC channels as output, set output freq and if required, boost the output amplitude.
 *
 * \param[in]   adf4030 [context variable]      Pointer to the device structure.
 * \param[in]   bsync_out_ch_sel                Target BSYNC Channels - Each bit represent one bsync channel.  \ref adi_adf4030_channel_id_e.
 *                                              i.e. (1 << ADI_ADF4030_CHANNEL_ID_0) | (1 << ADI_ADF4030_CHANNEL_ID_1) | ...
 * \param[in]   vco_out_freq_hz                 PLL/VCO Output Freq (in Hertz).
 * \param[in]   bsync_out_freq_hz               BSYNC Channel output frequency in Hz.
 * \param[in]   boost_en                        Setting it to 1 boosts BSYNC channel's output amplitude.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4030_bsync_output_set(adi_adf4030_device_t *adf4030,
                                                     uint16_t bsync_out_ch_sel,
                                                     uint64_t vco_out_freq_hz,
                                                     uint64_t bsync_out_freq_hz,
                                                     uint8_t boost_en);


/**
 * \brief   Configure BSYNC channels as input.
 *
 * \param[in]   adf4030 [context variable]      Pointer to the device structure.
 * \param[in]   bsync_in_ch_sel                 Source BSYNC Channels - Each bit represent one bsync channel.  \ref adi_adf4030_channel_id_e.
 *                                              i.e. (1 << ADI_ADF4030_CHANNEL_ID_0) | (1 << ADI_ADF4030_CHANNEL_ID_1) | ...
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4030_bsync_input_set(adi_adf4030_device_t *adf4030, uint16_t bsync_in_ch_sel);


/**
 * \brief   Auto Align BSYNC Target channel to a BSYNC Source Channel and perform TDC measurement between them.
 *
 * \param[in]   adf4030 [context variable]      Pointer to the device structure.
 * \param[in]   bsync_in_ch_sel                 Source BSYNC Channels - Each bit represent one bsync channel.  \ref adi_adf4030_channel_id_e.
 *                                              i.e. (1 << ADI_ADF4030_CHANNEL_ID_0) | (1 << ADI_ADF4030_CHANNEL_ID_1) | ...
 * \param[in]   bsync_out_ch_sel                Target BSYNC Channels - Each bit represent one bsync channel.  \ref adi_adf4030_channel_id_e.
 *                                              i.e. (1 << ADI_ADF4030_CHANNEL_ID_0) | (1 << ADI_ADF4030_CHANNEL_ID_1) | ...
 * \param[in]   bsync_out_freq_hz               BSYNC Channel output frequency in Hz.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4030_align_bsync_out(adi_adf4030_device_t *adf4030,
                                                    uint16_t bsync_in_ch_sel,
                                                    uint16_t bsync_out_ch_sel,
                                                    uint64_t bsync_out_freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_COMMON_ADF4030_H__ */
