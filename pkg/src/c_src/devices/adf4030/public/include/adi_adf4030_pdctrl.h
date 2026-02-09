/*!
 * \brief     Header file declaring APIs for ADF4030's Power Down control functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_PDCTRL
 * @{
 */

#ifndef __ADI_ADF4030_PDCTRL_H__
#define __ADI_ADF4030_PDCTRL_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_pdctrl_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Set Power-Down State for a target block
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   target_block            Power-Down target blocks. ADI_ADF4030_PDCTRL_ALL | _ADC | _LD | .... \ref adi_adf4030_pdctrl_e.
 * \param[in]   pd                      Set 1 to power-down target block else 0 to power-up.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_pdctrl_pd_set(adi_adf4030_device_t *adf4030, uint16_t target_block, uint8_t pd);


/**
 * \brief Set Power-Down State for a target BSYNC channels.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   target_path             Power-Down target path. ADI_ADF4030_BSYNC_PDCTRL_PATH_0 | _PATH_9 | ....  \ref adi_adf4030_bsync_pdctrl_e.
 * \param[in]   pd                      Set 1 to power-down target block else 0 to power-up.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_pdctrl_bsync_pd_set(adi_adf4030_device_t *adf4030, uint16_t target_path, uint8_t pd);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_PDCTRL_H__

/*! @} */
