#if !defined(VERSAL_PLATFORM)

/*!
 * @brief     FW image provider header for ADS10
 *
 * @copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS10__
 * @{
 */

 #ifndef __ADS10_FW_PROVIDER__
 #define __ADS10_FW_PROVIDER__
 
#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include "adi_apollo_startup_types.h"

 /*============= D E F I N E S ==============*/

 
 /*============= E X P O R T S ==============*/
 #ifdef __cplusplus
 extern "C" {
 #endif
 
/**
 * \brief Creates an instance of an ADS10 FW image provider
 *
 * \param[in] device                Context variable - Pointer to the APOLLO device data structure
 * \param[in] fw_image_dir          Directory containing FW images
 *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
adi_apollo_fw_provider_t * ads10_fw_provider_create(adi_apollo_device_t *device, char *fw_image_dir);

/**
 * \brief Firmware provider open function. Called before the get function.
 *
 * \param[in] fw_provider           Context variable - Pointer to firmware provider object
 * \param[in] fw_id                 FW file type ID for the binary used in current transaction \ref adi_apollo_startup_fw_id_e
 *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t ads10_fw_provider_open(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id);

/**
 * \brief Firmware provider close function. Called after the get function.
 *
 * \param[in] fw_provider           Context variable - Pointer to firmware provider object
 * \param[in] fw_id                 FW file ID for the binary used in current transaction \ref adi_apollo_startup_fw_id_e
 *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t ads10_fw_provider_close(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id);

/**
 * \brief Firmware provider get function. Returns the raw bytes for the requested FW binary image.
 *
 * \param[in] fw_provider           Context variable - Pointer to firmware provider object
 * \param[in] fw_id                 FW file ID for the binary used in current transaction \ref adi_apollo_startup_fw_id_e
 * \param[out] byte_arr             Pointer which is assigned to the buffer containing the raw FW image
 * \param[out] bytes_read           Pointer to param which contains the number of bytes in byte_arr
 *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t ads10_fw_provider_get(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id, uint8_t **byte_arr, uint32_t *bytes_read);

#ifdef __cplusplus
 }
 #endif
 #endif /*__ADS10_FW_PROVIDER__*/
#endif /* !defined(VERSAL_PLATFORM) */
