/*!
 * \brief     Source file implementing FPGA image loader for ADS10
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
#include <errno.h>

#include "adi_cms_api_common.h"
#include "adi_fpga_image_loader.h"
#include "ads10_fpga_config.h"

#define MAX_PATH_LEN 256
#define PLATFORM_TYPE_STR "ads10"

/*============= C O D E ====================*/
typedef struct {
    char fpga_image_dir[MAX_PATH_LEN];
} ads10_image_loader_obj_t;

static int32_t version_get(adi_fpga_image_loader_t *obj, adi_fpga_image_loader_version_t *version);
static int32_t dir_get(adi_fpga_image_loader_t *obj, uint8_t dir[], uint32_t *len);
static int32_t configure(adi_fpga_image_loader_t *obj, uint8_t file[], uint32_t len);

static uint16_t api_version[3] = {1, 0, 0}; // {major, minor, patch}

adi_fpga_image_loader_t *adi_fpga_image_loader_ads10_create(char *fpga_image_dir)
{
    if (fpga_image_dir == NULL) {
        return NULL;
    }
    
    if (strlen(fpga_image_dir) > (MAX_PATH_LEN - 1)) {
        return NULL;
    }

    adi_fpga_image_loader_t *il = (adi_fpga_image_loader_t *) malloc(sizeof(adi_fpga_image_loader_t));
    if (il == NULL) {
        return NULL;
    }
    
    il->tag = (ads10_image_loader_obj_t *) malloc(sizeof(ads10_image_loader_obj_t));
    if (il->tag == NULL) {
        return NULL;
    }

    il->platform_type_str = PLATFORM_TYPE_STR;
    il->version_get = version_get;
    il->dir_get = dir_get;
    il->configure = configure;

    /* Init callback fields */
    il->pre_cfg_cb = NULL;
    il->pre_cfg_cb_obj = NULL;
    il->post_cfg_cb = NULL;
    il->post_cfg_cb_obj = NULL;

    strcpy(((ads10_image_loader_obj_t *)il->tag)->fpga_image_dir, fpga_image_dir);

    return il;
}

int32_t adi_fpga_image_loader_ads10_destroy(adi_fpga_image_loader_t *il)
{
    ADI_CMS_NULL_PTR_CHECK(il);

    ADI_CMS_MEM_ALLOC_FREE(il->tag);
    ADI_CMS_MEM_ALLOC_FREE(il);

    return API_CMS_ERROR_OK;
}

static int32_t version_get(adi_fpga_image_loader_t *obj, adi_fpga_image_loader_version_t *version)
{
    version->major = api_version[0];
    version->minor = api_version[1];
    version->patch = api_version[2];
    
    return API_CMS_ERROR_OK;
}

static int32_t dir_get(adi_fpga_image_loader_t *obj, uint8_t fpga_image_path[], uint32_t *len)
{
    strcpy((char *) fpga_image_path, ((ads10_image_loader_obj_t *)obj->tag)->fpga_image_dir);
    *len = strlen((char *) fpga_image_path);
    
    return API_CMS_ERROR_OK;
}

static int32_t configure(adi_fpga_image_loader_t *obj, uint8_t fpga_image_file[], uint32_t len)
{
    int err = 0;
    uint8_t *data = NULL;
    uint32_t filesize, rd_byte;
    FILE *fp = NULL;

    if ((fp = fopen((char *) fpga_image_file, "rb")) == NULL) {
        printf("Error opening file %s: %s.\r\n",
               (char *) fpga_image_file,
               strerror(errno));
        err = API_CMS_ERROR_FILE_OPEN;
        goto end;
    }
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = (uint8_t *)malloc(filesize * sizeof(uint8_t));
    if (data == NULL) {
        printf("Error allocating memory for the data stream: %s\r\n",
               strerror(errno));
        err = API_CMS_ERROR_MEM_ALLOC;
        goto end;
    }

    rd_byte = fread(data, sizeof(uint8_t), filesize, fp);
    fclose(fp);

    if (rd_byte != filesize) {
        printf("Error: Read file size does not match!\r\n");
        err = API_CMS_ERROR_FILE_READ;
        goto err_free;
    }

    /* Load the FPGA image */
    err = ads10_fpga_config_load_image(data, filesize);
    if (err != API_CMS_ERROR_OK) {
        err = API_CMS_ERROR_PLATFORM_IMAGE_LOAD;
    }

err_free:
    free(data);
end:
    return err;
}

/*! @} */