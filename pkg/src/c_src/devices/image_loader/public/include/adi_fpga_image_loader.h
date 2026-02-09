/*!
 * \brief     Apollo FPGA platform image loading APIs
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_PLATFORM
 * @{
 */
#ifndef __ADI_FPGA_IMAGE_LOADER_H__
#define __ADI_FPGA_IMAGE_LOADER_H__

/*============= I N C L U D E S ============*/
#include "adi_fpga_image_loader_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief        Get version of Image Loader
 *
 * \param[in]   obj             [Context variable] - Pointer to the FPGA image loader device data structure
 * \param[out]  version         The image loader version
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_image_loader_version_get(adi_fpga_image_loader_t *obj,
                                          adi_fpga_image_loader_version_t *version);

/**
 * \brief Returns the target dir location for storing FPGA images
 *
 * \param[in] obj               [Context variable] - Pointer to the FPGA image loader device data structure
 * \param[out] fpga_image_dir   Folder location for FPGA images
 * \param[in, out] len          Length of path string
 * *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_image_loader_dir_get(adi_fpga_image_loader_t *obj, uint8_t fpga_image_dir[], uint32_t *len);

/**
 * \brief Load the platform FPGA image file
 *
 * \param[in] obj               [Context variable] - Pointer to the FPGA image loader device data structure
 * \param[in] fpga_image_file   FPGA image name
 * \param[in] len               Length of path string
 * *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_image_loader_configure(adi_fpga_image_loader_t *obj, uint8_t fpga_image_file[], uint32_t len);

#ifndef CLIENT_IGNORE

/**
 * \brief Assign optional user function to be called prior to FPGA image load.
 *
 * \param[in] obj               [Context variable] - Pointer to the FPGA image loader device data structure
 * \param[in] cb                Callback function pointer. Set to NULL if unused.
 * \param[in] cb_obj            User defined cb data object
 * *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_image_loader_pre_config_cb_set(adi_fpga_image_loader_t *obj, adi_fpga_image_loader_config_cb_t cb, void *cb_obj);

/**
 * \brief Assign optional user function to be called after FPGA image load.
 *
 * \param[in] obj               [Context variable] - Pointer to the FPGA image loader device data structure
 * \param[in] cb                Callback function pointer. Set to NULL if unused.
 * \param[in] cb_obj            User defined cb data object
 * *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_image_loader_post_config_cb_set(adi_fpga_image_loader_t *obj, adi_fpga_image_loader_config_cb_t cb, void *cb_obj);

#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif

#endif /* __ADI_FPGA_IMAGE_LOADER_H__ */
/*! @} */
