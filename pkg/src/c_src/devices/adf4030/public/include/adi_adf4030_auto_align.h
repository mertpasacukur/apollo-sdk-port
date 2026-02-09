/*!
 * \brief     Header file declaring APIs for ADF4030's Auto Alignment functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_AUTO_ALIGN
 * @{
 */

#ifndef __ADI_ADF4030_AUTO_ALIGN_H__
#define __ADI_ADF4030_AUTO_ALIGN_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Set TDC Offset applied in terms of delay to target channel for alignment procedure.
 *          This is a closed loop delay adjustment procedure and the adjusted delay is reflected exactly when measured.
 *          Use this procedure to introduce delays that compensate round trip delays on BSYNC channels.
 *
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   tdc_target_channel      TDC Target Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   ref_input_freq          Reference input frequency in Hz.
 * \param[in]   delay_in_fs             The delay offset which will be added to Target Channel in FemtoSeconds.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_auto_align_delay_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e tdc_target_channel, uint64_t ref_input_freq, int64_t delay_in_fs);


/**
 * \brief   Sets the number of Alignment Cycles used in a Single Automatic Alignment Request.
 * \note    The maximum number of iterations is 8. i.e. iter_number = 7.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   iter_number             The desired number of alignment cycles to be executed minus 1.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_auto_align_iteration_set(adi_adf4030_device_t *adf4030, uint8_t iter_number);


/**
 * \brief   Set time delay threshold value for auto alignment procedure.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   threshold_in_fs         Threshold Value for Alignment.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_auto_align_threshold_set(adi_adf4030_device_t *adf4030, uint32_t threshold_in_fs);


/**
 * \brief   Auto Align one BSYNC Target channel to a BSYNC Source Channel.
 *
 * \note    Only 1 channel_id selection per call.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   tdc_source_channel      TDC Source Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 * \param[in]   tdc_target_channel      TDC Target Channel ID. ADI_ADF4030_CHANNEL_ID_0 to _9. \ref adi_adf4030_channel_id_e.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_auto_align_single_channel_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e tdc_source_channel, adi_adf4030_channel_id_e tdc_target_channel);


/**
 * \brief   Auto Align Multiple channel at once.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   multi_ch_sel            Target BSYNC Channels - Each bit represent one bsync channel.  \ref adi_adf4030_channel_id_e.
 *                                      i.e. (1 << ADI_ADF4030_CHANNEL_ID_0) | (1 << ADI_ADF4030_CHANNEL_ID_1) | ...
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_auto_align_multi_channel_set(adi_adf4030_device_t *adf4030, uint16_t multi_ch_sel);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_AUTO_ALIGN_H__

/*! @} */
