/*!
 * \brief     Header file declaring APIs for ADF4030's BSYNC Output functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_BSYNCOUT
 * @{
 */

#ifndef __ADI_ADF4030_BSYNCOUT_H__
#define __ADI_ADF4030_BSYNCOUT_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_bsyncout_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Set BSYNC Channel's output configuration.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   channel_id              BSYNC Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   config                  BSYNC Output Configuration. \ref adi_adf4030_bsyncout_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_bsyncout_config_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsyncout_config_t *config);


/**
 * \brief   Set BSYNC Channel's output frequency based on PLL/VCO output frequency.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   channel_id              BSYNC Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   vco_out_freq            PLL/VCO output frequency in Hz.
 * \param[in]   bsync_out_freq          BSYNC Channel output frequency in Hz.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_bsyncout_freq_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, uint64_t vco_out_freq, uint64_t bsync_out_freq);


/**
 * \brief   Set BSYNC Channel output drive.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   channel_id              BSYNC Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   bsync_drive             BSYNC Channel Drive. ADI_ADF4030_CHANNEL_BSYNC_DRIVE_RX and _TX. \ref adi_adf4030_bsync_drive_e.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_bsyncout_drive_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsync_drive_e bsync_drive);


/**
 * \brief   Enable / Disable BSYNC Channel's output amplitude boost.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   channel_id              BSYNC Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   enable                  Setting enable to 1 boosts BSYNC channel's output amplitude.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_bsyncout_boost_enable(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, uint8_t enable);


/**
 * \brief   Read back output configuration for all BSYNC Channels.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   ref_input_freq          Reference input frequency in Hz.
 * \param[in]   config                  BSYNC Output Configuration. \ref adi_adf4030_bsyncout_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */

int32_t adi_adf4030_bsyncout_config_get(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, adi_adf4030_bsyncout_config_t *config);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_BSYNCOUT_H__

/*! @} */
