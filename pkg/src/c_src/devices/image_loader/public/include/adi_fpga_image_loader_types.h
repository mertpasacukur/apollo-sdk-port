/*!
 * \brief     Apollo FPGA platform image loading types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_PLATFORM
 * @{
 */

#ifndef __ADI_FPGA_IMAGE_LOADER_TYPES_H__
#define __ADI_FPGA_IMAGE_LOADER_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/

/*!
 * \brief Image Loader Version Structure
 */
typedef struct
{
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} adi_fpga_image_loader_version_t;

#ifndef CLIENT_IGNORE

/*!
 * \brief Image Loader user callback function type.
 *
 * \return API_CMS_ERROR_OK         Completed Successfully
 * \return <0                       Failed. \ref adi_cms_error_e for details.
 */
typedef int32_t (*adi_fpga_image_loader_config_cb_t)(void *user_obj);   /*!< Pointer to user specific object. May be NULL. */

/*!
 * \brief Fpga image loader structure
 */
typedef struct adi_fpga_image_loader_t {
    char *platform_type_str;

    int32_t (*version_get)(struct adi_fpga_image_loader_t *, adi_fpga_image_loader_version_t *version);
    int32_t (*dir_get)(struct adi_fpga_image_loader_t *, uint8_t dir[], uint32_t *len);
    int32_t (*configure)(struct adi_fpga_image_loader_t *, uint8_t file[], uint32_t len);

    adi_fpga_image_loader_config_cb_t pre_cfg_cb;           /*!< Function pointer to user pre FPGA image load */
    adi_fpga_image_loader_config_cb_t post_cfg_cb;          /*!< Function pointer to user post FPGA image load */

    void *pre_cfg_cb_obj;
    void *post_cfg_cb_obj;

    void *tag;                  // Implementation specific struct/obj
} adi_fpga_image_loader_t;

#endif  // !CLIENT_IGNORE

#endif // !__ADI_FPGA_IMAGE_LOADER_TYPES_H__

/*! @} */