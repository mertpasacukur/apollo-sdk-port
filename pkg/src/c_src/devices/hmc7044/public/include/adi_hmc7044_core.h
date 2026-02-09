/*!
 * \brief     HMC7044 Core Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_CORE_H__
#define __ADI_HMC7044_CORE_H__

#include "adi_hmc7044_core_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get version of the API
 *
 * \param[in]  hmc7044 [context variable]       Reference to the device.
 * \param[out] apiVersion                       The API version
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_hmc7044_core_version_get(adi_hmc7044_device_t *hmc7044, adi_hmc7044_version_t *apiVersion);

/**
 * @brief Perform SPI register write access to device
 *
 * \param[in] hmc7044 [context variable]        Pointer to the device structure
 * \param[in] address                           SPI address to which the value of data parameter shall be written
 * \param[in] data                              8-bit value to be written to SPI register defined
 *                                              by the address parameter.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_hmc7044_core_spi_reg_set(adi_hmc7044_device_t *hmc7044, uint16_t address, uint8_t data);

/**
 * @brief Perform SPI register read access to device.
 *
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure
 * \param[in]  address                          SPI address from which the value of data parameter shall be read,
 * \param[out] data                             Pointer to an 8-bit variable to which the value of the
 *                                              SPI register at the address defined by address parameter
 *                                              shall be stored.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_hmc7044_core_spi_reg_get(adi_hmc7044_device_t *hmc7044, uint16_t address, uint8_t *data);

/**
 * \brief Verify SPI read and write transaction to the scratchpad register.
 *
 * \param[in]  hmc7044 [context variable]       Pointer to the device structure
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_hmc7044_core_spi_reg_test(adi_hmc7044_device_t *hmc7044);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_HMC7044_CORE_H__
