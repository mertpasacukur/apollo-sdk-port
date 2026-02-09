/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADF4382_HAL__
 * @{
 */

#ifndef __ADF4382_HAL__
#define __ADF4382_HAL__

#include "adi_adf4382_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Set ADI default register values
 *
 * \param[in]  adf4382  Context variable - Pointer to the ADF4382 device data structure
 * \param[in]  chip_ver ADF4382 Chip Version id. \ref adi_adf4382_chip_ver_sel_e
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_hal_reg_default_set(adi_adf4382_device_t *adf4382, uint8_t chip_ver);

/**
 * \brief  Get 8-bit register value.
 *
 * \param[in]  adf4382  Context variable - Pointer to the ADF4382 device data structure
 * \param[in]  reg      Register to read from
 * \param[out] data     Value of register (8-bit)
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_hal_reg_get(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t *data);

/**
 * \brief  Set the 8-bit register value.
 *
 * \param[in] adf4382  Context variable - Pointer to the ADF4382 device data structure
 * \param[in] reg      Register to write to
 * \param[in] data     Value to write to register
 *
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_hal_reg_set(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t data);

/**
 * \brief  Get bit field value.
 *
 * \param[in]  adf4382          Context variable - Pointer to the ADF4382 device data structure
 * \param[in]  reg              register to read from
 * \param[in]  info             information about bitfield
 * \param[out] value            value of bitfield
 * \param[in]  value_size_bytes no of bytes
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_private_hal_bf_get(adi_adf4382_device_t *adf4382, uint32_t reg, uint32_t info, uint8_t value[], uint8_t value_size_bytes);

/**
 * \brief Set bit field value using paged register addressing.
 *
 * \param[in] adf4382  Context variable - Pointer to the ADF4382 device data structure
 * \param[in] reg      Register to write. \ref adi_apollo_hal_paged_base_addr_set
 * \param[in] info     Information about bitfield
 * \param[in] value    Value of bitfield
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4382_private_hal_bf_set(adi_adf4382_device_t *adf4382, uint32_t reg, uint32_t info, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif // !__ADF4382_HAL__
