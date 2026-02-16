#if defined(__linux__)
/*!
 * \brief     Source file implementing FW image provider for ADS10
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
#include "adi_cms_api_common.h"
#include "adi_apollo_hal.h"
#include "adi_apollo_linux_utilities.h"
#include "ads10_fw_provider.h"

#define MAX_PATH_LEN 256
#define DESC_STR "ads10-linux"

typedef struct {
    char fw_image_dir[MAX_PATH_LEN];
    adi_apollo_device_t *device;
    uint8_t* buffer;
} ads10_fw_provider_obj_t;

static const char* fw_file[ADI_APOLLO_FW_ID_MAX] =
{
    "APOLLO_FW_CPU0_B.bin",
    "APOLLO_FW_CPU1_B.bin",

    "app_signed_encrypted_B/flash_image_0x01030000.bin",
    "app_signed_encrypted_B/flash_image_0x20000000.bin",
    "app_signed_encrypted_B/flash_image_0x02000000.bin",
    "app_signed_encrypted_B/flash_image_0x21000000.bin",

    "app_signed_encrypted_prod_B/flash_image_0x01030000.bin",
    "app_signed_encrypted_prod_B/flash_image_0x20000000.bin",
    "app_signed_encrypted_prod_B/flash_image_0x02000000.bin",
    "app_signed_encrypted_prod_B/flash_image_0x21000000.bin"
};

adi_apollo_fw_provider_t * ads10_fw_provider_create(adi_apollo_device_t *device, char *fw_image_dir)
{
    ads10_fw_provider_obj_t* ads10_fw_provider;

    if (device == NULL) {
        return NULL;
    }

    if (fw_image_dir == NULL) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "fw_image_dir param is null");
        return NULL;
    }

    if (strlen(fw_image_dir) > (MAX_PATH_LEN - 1)) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "fw_image_dir length too long. Max is %d", MAX_PATH_LEN);
        return NULL;
    }
    
    adi_apollo_fw_provider_t *obj = (adi_apollo_fw_provider_t *) malloc(sizeof(adi_apollo_fw_provider_t));
    if (obj == NULL) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "malloc returned null");
        return NULL;
    }
    
    obj->tag = (ads10_fw_provider_obj_t *) malloc(sizeof(ads10_fw_provider_obj_t));
    if (obj->tag == NULL) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "malloc returned null");
        return NULL;
    }
    ads10_fw_provider = (ads10_fw_provider_obj_t *)obj->tag;

    obj->desc = DESC_STR;
    ads10_fw_provider->device = device;
    ads10_fw_provider->buffer = NULL;

    strncpy(ads10_fw_provider->fw_image_dir, fw_image_dir, MAX_PATH_LEN);

    return obj;
}

int32_t ads10_fw_provider_open(adi_apollo_fw_provider_t* obj, adi_apollo_startup_fw_id_e fw_id)
{
    ads10_fw_provider_obj_t* ads10_fw_provider;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(obj->tag);

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    ads10_fw_provider = (ads10_fw_provider_obj_t*)obj->tag;

    adi_apollo_hal_log_write(ads10_fw_provider->device, ADI_CMS_LOG_MSG, "open fw_file[%d] = %s", fw_id, fw_file[fw_id]);

    return API_CMS_ERROR_OK;
}

int32_t ads10_fw_provider_close(adi_apollo_fw_provider_t* obj, adi_apollo_startup_fw_id_e fw_id)
{
    ads10_fw_provider_obj_t* ads10_fw_provider;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(obj->tag);

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    ads10_fw_provider = (ads10_fw_provider_obj_t*)obj->tag;

    adi_apollo_hal_log_write(ads10_fw_provider->device, ADI_CMS_LOG_MSG, "close fw_file[%d] = %s", fw_id, fw_file[fw_id]);

    if (ads10_fw_provider->buffer != NULL) {
        free(ads10_fw_provider->buffer);
        ads10_fw_provider->buffer = NULL;
    }

    return API_CMS_ERROR_OK;
}

int32_t ads10_fw_provider_get(adi_apollo_fw_provider_t* obj, adi_apollo_startup_fw_id_e fw_id, uint8_t ** byte_arr, uint32_t* bytes_read)
{
    int32_t err = API_CMS_ERROR_OK;
    char * fw_image_path;
    ads10_fw_provider_obj_t* ads10_fw_provider;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(byte_arr);
    ADI_CMS_NULL_PTR_CHECK(bytes_read);
    ADI_CMS_NULL_PTR_CHECK(obj->tag);

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

	ads10_fw_provider = (ads10_fw_provider_obj_t *)(obj->tag);

    adi_apollo_hal_log_write(ads10_fw_provider->device, ADI_CMS_LOG_MSG, "get fw_file[%d] = %s", fw_id, fw_file[fw_id]);

    if (ads10_fw_provider->buffer != NULL) {
        printf("Buffer was not freed from previous transaction\n");
        return API_CMS_ERROR_ERROR;
    }

    fw_image_path = (char *)malloc(strlen(ads10_fw_provider->fw_image_dir) + strlen("/") + strlen(fw_file[fw_id]) + 1);
    ADI_CMS_MEM_ALLOC_CHECK(fw_image_path);

    /* Create absolute path to fw image */
    *fw_image_path = '\0';
    strcat(fw_image_path, ads10_fw_provider->fw_image_dir);
    strcat(fw_image_path, "/");
    strcat(fw_image_path, fw_file[fw_id]);

    /* Get raw file bytes. User responsible for free memory allocated by function. */
    err = adi_apollo_utilities_file_to_byte_arr(ads10_fw_provider->device, fw_image_path, byte_arr, bytes_read);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Save buffer ptr for freeing memory in close. */
    ads10_fw_provider->buffer = *byte_arr;

end:
    free(fw_image_path);

    return err;
}


/*! @} */

#endif /* defined(__linux__) */
