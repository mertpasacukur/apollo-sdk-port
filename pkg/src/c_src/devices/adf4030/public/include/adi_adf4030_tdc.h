/*!
 * \brief     Header file declaring APIs for ADF4030's TDC functionality.
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

#ifndef __ADI_ADF4030_TDC_H__
#define __ADI_ADF4030_TDC_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_tdc_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Measures TDC value between BSYNC Source and BSYNC Target Channels' outputs.
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   tdc_source_channel      TDC Source Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   tdc_target_channel      TDC Target Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   source_out_freq_hz      TDC Source Channel Out Frequency.
 * \param[out]  tdc_result              Measured TDC value in femtoSeconds.
 *
 * \returns 0 for success
 */
int32_t adi_adf4030_tdc_measure(adi_adf4030_device_t *adf4030,
                                adi_adf4030_channel_id_e tdc_source_channel,
                                adi_adf4030_channel_id_e tdc_target_channel,
                                uint64_t source_out_freq_hz,
                                int64_t *tdc_result);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_TDC_H__

/*! @} */
