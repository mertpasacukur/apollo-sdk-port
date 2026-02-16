#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     Apollo ADS10 FPGA platform image loading APIs
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
#ifndef __ADI_FPGA_IMAGE_LOADER_ADS10_H__
#define __ADI_FPGA_IMAGE_LOADER_ADS10_H__

/*============= I N C L U D E S ============*/
#include "adi_fpga_image_loader_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLIENT_IGNORE
#endif /* CLIENT_IGNORE*/

/**
 * \brief Creates an instance of an ADS10 FPGA image loader
 *
 * \param[in] fpga_image_dir        Directory containing FPGA images
 *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
adi_fpga_image_loader_t * adi_fpga_image_loader_ads10_create(char *fpga_image_dir);

/**
 * \brief Destroy an FPGA image loader previously created with adi_fpga_image_loader_ads10_create()
 * 
 * Frees up all memory allocated.
 *
 * \param[in] il        FPGA image loader previously created with adi_fpga_image_loader_ads10_create()
 *
 * \return API_CMS_ERROR_OK                     Completed successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_image_loader_ads10_destroy(adi_fpga_image_loader_t *il);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_FPGA_IMAGE_LOADER_ADS10_H__ */
/*! @} */
#endif /* !defined(VERSAL_PLATFORM) */
