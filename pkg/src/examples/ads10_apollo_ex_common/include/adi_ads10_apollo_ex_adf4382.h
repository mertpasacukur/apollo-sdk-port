/*!
 * \brief     ADS10 Apollo examples common ADF4382 functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_types.h"
#include "adi_adf4382_regmap_inline_bfs.h"

// Import All ADF4382 Feature Headers
#include "adi_adf4382_clkctrl.h"
#include "adi_adf4382_core.h"
#include "adi_adf4382_cpctrl.h"
#include "adi_adf4382_ldctrl.h"
#include "adi_adf4382_muxout.h"
#include "adi_adf4382_pdctrl.h"
#include "adi_adf4382_phase_adjust.h"
#include "adi_adf4382_rfout.h"

#include "ads10_hal.h"

#ifndef __ADI_ADS10_APOLLO_EX_COMMON_ADF4382_H__
#define __ADI_ADS10_APOLLO_EX_COMMON_ADF4382_H__

/**
 * @brief MCS Init Cal specific ADF4382 configuration.
 */
typedef struct {
    // Bleed Current Setting
    adi_adf4382_cpctrl_bleed_word_config_t bleed_word_config;
    adi_adf4382_cpctrl_bleed_polarity_e bleed_pol;
    uint8_t bleed_adj_cal;
    // Phase Adjust Setting
    adi_adf4382_phase_adjust_config_t phase_adj_config;
    uint8_t en_bleed;
    uint8_t en_phase_resync;
    uint8_t en_auto_align;
} adi_apollo_ex_adf4382_mcs_init_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Initializes ADF4382 device HAL and for FMCB sets up means for external GPIO control for enabling level shifters.
 *
 * \param[in]   adf4382                     Context variable - Pointer to the ADF4382 device data structure
 * \param[in]   sdo_en_context              User device context needed for external GPIO control by sdo_en_fcn function
 * \param[in]   sdo_en_fcn                  Ptr to Function that can set external GPIO as Output and it's state
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4382_hal_config(adi_adf4382_device_t *adf4382, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

/**
 * \brief   Initializes and configures ADF4382.
 *
 * \param[in]   adf4382                     Context variable - Pointer to the ADF4382 device data structure
 * \param[in]   rfout_freq_hz               Expected Output Freq (in Hertz) from ADF4382.
 * \param[in]   ref_freq_hz                 Reference Freq (in Hertz) provided to ADF4382.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4382_startup(adi_adf4382_device_t *adf4382,
                                            uint64_t rfout_freq_hz, uint64_t ref_freq_hz);

/**
 * \brief   Configures ADF4382 for MCS init Cal.
 *
 * \param[in]   adf4382                     Context variable - Pointer to the ADF4382 device data structure
 * \param[in]   adf4382_mcs_init_config     Pointer to a struct that store ADF4382 setting during MCS Cal. \ref adi_apollo_ex_adf4382_mcs_init_config_t
 * \param[out]  phase_adj_rb                Readback programed PHASE_ADJUSTMENT value.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4382_mcs_init_config(adi_adf4382_device_t *adf4382,
                                                    adi_apollo_ex_adf4382_mcs_init_config_t *adf4382_mcs_init_config,
                                                    uint8_t *phase_adj_rb);

/**
 * \brief   Reads back Bleed code sent to charge pump and returns current's coarse, fine and polarity.
 *
 * \param[in]   adf4382                     Context variable - Pointer to the ADF4382 device data structure
 * \param[out]  bleed_pol                   Bleed Current Polarity value.
 * \param[out]  coarse_current              Bleed Current Coarse value.
 * \param[out]  fine_current                Bleed Current Fine value.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adf4382_del_cnt_get(adi_adf4382_device_t *adf4382, uint8_t *bleed_pol, int8_t *coarse_current, int16_t *fine_current);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_COMMON_ADF4382_H__ */
