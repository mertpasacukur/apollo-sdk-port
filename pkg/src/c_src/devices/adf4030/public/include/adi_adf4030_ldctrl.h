/*!
 * \brief     Header file declaring APIs for ADF4030's Lock Detect functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_LDCTRL
 * @{
 */

#ifndef __ADI_ADF4030_LDCTRL_H__
#define __ADI_ADF4030_LDCTRL_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_ldctrl_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Set Lock Detect Configuration.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   config                  Pointer to Lock Detect configuration struct. \ref adi_adf4030_ldctrl_config_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_ldctrl_config_set(adi_adf4030_device_t *adf4030, adi_adf4030_ldctrl_config_t *config);


/**
 * \brief   Get Lock Detect Status.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[out]  status                  Reads back Lock Status. Reads 1 if PLL is locked, else 0.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_ldctrl_status_get(adi_adf4030_device_t *adf4030, uint8_t *status);


/**
 * \brief   Get Reference Input Clock Amplitude Status.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[out]  status                  Reads back Ref Input Status. Reads 1 if ref amplitude above threshold, else 0.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_ldctrl_ref_status_get(adi_adf4030_device_t *adf4030, uint8_t *status);


/**
 * \brief   Wait for Lock Detect.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   timeout_ms              Time in milliseconds, to wait to lock detect.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_ldctrl_lock_wait(adi_adf4030_device_t *adf4030, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_LDCTRL_TYPES_H__

/*! @} */
