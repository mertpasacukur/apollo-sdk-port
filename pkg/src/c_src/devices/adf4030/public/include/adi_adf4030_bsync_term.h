/*!
 * \brief     Header file declaring APIs for ADF4030's BSYNC Termination functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADF4030_BSYNC_TERM
 * @{
 */

#ifndef __ADI_ADF4030_BSYNC_TERM_H__
#define __ADI_ADF4030_BSYNC_TERM_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_bsync_term_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Set Termination configuration for given BSYNC channel.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   channel_id              BSYNC Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   config                  BSYNC Termination Configuration. \ref adi_adf4030_bsync_term_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_bsync_term_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsync_term_config_t *config);


/**
 * \brief   Get Termination configuration of given BSYNC channel.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   channel_id              BSYNC Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[out]  config                  BSYNC Termination Configuration. \ref adi_adf4030_bsync_term_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_bsync_term_get(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsync_term_config_t *config);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_BSYNC_TERM_H__

/*! @} */
