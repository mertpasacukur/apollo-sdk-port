/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADF4030_HAL__
#define __ADF4030_HAL__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Set ADI default register values
 *
 * \param[in]   adf4030     Context variable - Pointer to the ADF4030 device data structure
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_hal_reg_default_set(adi_adf4030_device_t *adf4030);

/**
 * \brief   Get 8-bit register value.
 *
 * \param[in]   adf4030         Context variable - Pointer to the ADF4030 device data structure
 * \param[in]   reg             Register to read from
 * \param[out]  data            Value of register (8-bit)
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_hal_reg_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t *data);

/**
 * \brief   Set the 8-bit register value.
 *
 * \param[in]   adf4030         Context variable - Pointer to the ADF4030 device data structure
 * \param[in]   reg             Register to write to
 * \param[in]   data            Value to write to register
 *
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_hal_reg_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t data);

/**
 * \brief   Get bit field value.
 *
 * \param[in]   adf4030             Context variable - Pointer to the ADF4030 device data structure
 * \param[in]   reg                 Device register address to read bitfield value from.
 * \param[in]   info                Bitfield width and offset information.
 * \param[out]  value               Read back value of bitfield.
 * \param[in]   value_size_bytes    Number of bytes to read back.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_private_hal_bf_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint32_t info, uint8_t value[], uint8_t value_size_bytes);

/**
 * \brief   Set bit field value.
 *
 * \param[in]   adf4030             Context variable - Pointer to the ADF4030 device data structure
 * \param[in]   reg                 Device register address to write bitfield value.
 * \param[in]   info                Bitfield width and offset information.
 * \param[in]   value               Value of bitfield.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_private_hal_bf_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint32_t info, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif // !__ADF4030_HAL__
