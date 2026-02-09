/*!
 * \brief     Source file implementing APIs for the FPGA image loader
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_PLATFORM
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"
#include "adi_fpga_image_loader.h"

/*============= C O D E ====================*/

int32_t adi_fpga_image_loader_version_get(adi_fpga_image_loader_t *obj,
                                          adi_fpga_image_loader_version_t *version)
{
    ADI_CMS_NULL_PTR_CHECK(obj);
    return obj->version_get(obj, version);
}

int32_t adi_fpga_image_loader_dir_get(adi_fpga_image_loader_t *obj, uint8_t fpga_image_dir[], uint32_t *len)
{
    ADI_CMS_NULL_PTR_CHECK(obj);
    return obj->dir_get(obj, fpga_image_dir, len);
}

int32_t adi_fpga_image_loader_configure(adi_fpga_image_loader_t *obj, uint8_t fpga_image_file[], uint32_t len)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(fpga_image_file);

    if (obj->pre_cfg_cb) {
        err = obj->pre_cfg_cb(obj->pre_cfg_cb_obj);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = obj->configure(obj, fpga_image_file, len);
    ADI_CMS_ERROR_RETURN(err);

    if (obj->post_cfg_cb) {
        err = obj->post_cfg_cb(obj->post_cfg_cb_obj);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_image_loader_pre_config_cb_set(adi_fpga_image_loader_t *obj, adi_fpga_image_loader_config_cb_t cb, void *cb_obj)
{
    ADI_CMS_NULL_PTR_CHECK(obj);

    obj->pre_cfg_cb = cb;
    obj->pre_cfg_cb_obj = cb_obj;

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_image_loader_post_config_cb_set(adi_fpga_image_loader_t *obj,  adi_fpga_image_loader_config_cb_t cb, void *cb_obj)
{
    ADI_CMS_NULL_PTR_CHECK(obj);

    obj->post_cfg_cb = cb;
    obj->post_cfg_cb_obj = cb_obj;

    return API_CMS_ERROR_OK;
}

/*! @} */