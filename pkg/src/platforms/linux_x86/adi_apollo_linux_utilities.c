/*!
 * \brief     APIs for CPU FW file LOAD
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_APOLLO_LINUX_UTILITIES
 * @{
 */
#include "adi_apollo_linux_utilities.h"
#include "adi_apollo_arm.h" // For memory address
#include "adi_apollo_config.h"


#define MAX_PATH_LENGTH 256

int32_t adi_apollo_utilities_fileload(adi_apollo_device_t *device, char *filename, uint32_t cpu_address, uint16_t chunk_sz_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    FILE* fp = NULL;
    uint32_t file_size;
    uint32_t byte_count = 0;
    
    adi_apollo_arm_binary_info_t binary_info;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_LOG_FUNC();

    ADI_APOLLO_NULL_POINTER_RETURN(filename);

    fp = fopen(filename, "rb");
    ADI_APOLLO_NULL_POINTER_RETURN(fp);

    /* Determine file size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        err = API_CMS_ERROR_ERROR;
        ADI_APOLLO_ERROR_RETURN(err);
    }

    /* ftell returns long type */
    file_size = (uint32_t) ftell(fp);

    /* Rewind the file pointer to beginning of the file */
    if (fseek(fp, 0U, SEEK_SET) < 0) {
        err = 1;
        printf("Unable to Rewind File Pointer for CPU Binary\n");
        ADI_APOLLO_ERROR_RETURN(err);
    }

    uint8_t fw_image_buf[file_size+1];

    byte_count = fread(&fw_image_buf[0U], 1, file_size, fp); // TODO: read entire file, but can be read into chunks to save memory
    binary_info.cpu_start_addr = cpu_address;
    binary_info.binary_ptr = &fw_image_buf[0];
    binary_info.binary_sz_bytes = byte_count;
    binary_info.is_cont = 0;
    err = adi_apollo_arm_memload(device, &binary_info, chunk_sz_bytes);
    ADI_APOLLO_ERROR_RETURN(err);

    fclose(fp);

    return API_CMS_ERROR_OK;
}

int32_t adi_apollo_utilities_file_to_byte_arr(adi_apollo_device_t *device, char *file_name, uint8_t **byte_arr, uint32_t *binary_sz_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    FILE *fp = NULL;
    uint32_t file_size;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_LOG_FUNC();
    ADI_APOLLO_NULL_POINTER_RETURN(file_name);
    ADI_APOLLO_NULL_POINTER_RETURN(byte_arr);
    ADI_APOLLO_NULL_POINTER_RETURN(binary_sz_bytes);

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        return API_CMS_ERROR_FILE_OPEN;
    }

    /* Determine file size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        err = API_CMS_ERROR_FILE_OPERATION;
        goto end;
    }

    /* ftell returns long type */
    file_size = (uint32_t) ftell(fp);

    /* Rewind the file pointer to beginning of the file */
    if (fseek(fp, 0U, SEEK_SET) < 0) {
        printf("Unable to Rewind File Pointer for Binary File\n");
        err = API_CMS_ERROR_FILE_OPERATION;
        goto end;
    }

    *byte_arr = (uint8_t *)calloc((file_size + 1), sizeof(uint8_t));
    if (*byte_arr == NULL) {
        err = API_CMS_ERROR_MEM_ALLOC;
        goto end;
    }

    *binary_sz_bytes = (uint32_t) fread(*byte_arr, 1, file_size, fp);

end:
    fclose(fp);

    return err;
}

int32_t adi_apollo_utilities_byte_arr_to_file(adi_apollo_device_t *device, char *file_name, uint8_t *byte_arr, size_t binary_sz_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    FILE *fp = NULL;
    size_t n_bytes_written;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_LOG_FUNC();
    ADI_APOLLO_NULL_POINTER_RETURN(file_name);
    ADI_APOLLO_NULL_POINTER_RETURN(byte_arr);

    fp = fopen(file_name, "w");
    if (fp == NULL) {
        return API_CMS_ERROR_FILE_OPEN;
    }

    n_bytes_written = fwrite(byte_arr, sizeof(uint8_t), binary_sz_bytes, fp);
    if (n_bytes_written != binary_sz_bytes) {
        err = API_CMS_ERROR_FILE_WRITE;
        goto end;
    }

end:
    fclose(fp);

    return err;
}

int32_t adi_apollo_utilities_file_to_byte_arr_by_offset(adi_apollo_device_t *device, char *file_name, uint32_t offset, uint32_t length, uint8_t **byte_arr, uint32_t *binary_sz_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    FILE *fp = NULL;
    uint32_t file_size;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_LOG_FUNC();
    ADI_APOLLO_NULL_POINTER_RETURN(file_name);
    ADI_APOLLO_NULL_POINTER_RETURN(byte_arr);
    ADI_APOLLO_NULL_POINTER_RETURN(binary_sz_bytes);

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        return API_CMS_ERROR_FILE_OPEN;
    }

    /* Determine file size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        err = API_CMS_ERROR_FILE_OPERATION;
        goto end;
    }

    /* ftell returns long type */
    file_size = (uint32_t) ftell(fp);

    // printf("file_size: %d \t offset: %d \t length: %d \t offset + length: %d.\n", file_size, offset, length, (offset + length));

    if (file_size < (offset + length)) {
        err = API_CMS_ERROR_INVALID_PARAM;
        goto end;
    }

    /* Move the file pointer to offset location */
    if (fseek(fp, offset, SEEK_SET) != 0) {
        printf("Unable to offset File Pointer for Binary File\n");
        err = API_CMS_ERROR_FILE_OPERATION;
        goto end;
    }

    *byte_arr = (uint8_t *)calloc((length + 1), sizeof(uint8_t));
    if (*byte_arr == NULL) {
        err = API_CMS_ERROR_MEM_ALLOC;
        goto end;
    }

    *binary_sz_bytes = (uint32_t) fread(*byte_arr, 1, length, fp);

end:
    fclose(fp);

    return err;
}

int32_t adi_apollo_utilities_byte_arr_to_file_append(adi_apollo_device_t *device, char *file_name, uint8_t *byte_arr, size_t binary_sz_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    FILE *fp = NULL;
    size_t n_bytes_written;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_LOG_FUNC();
    ADI_APOLLO_NULL_POINTER_RETURN(file_name);
    ADI_APOLLO_NULL_POINTER_RETURN(byte_arr);

    fp = fopen(file_name, "a");
    if (fp == NULL) {
        return API_CMS_ERROR_FILE_OPEN;
    }

    n_bytes_written = fwrite(byte_arr, sizeof(uint8_t), binary_sz_bytes, fp);
    if (n_bytes_written != binary_sz_bytes) {
        err = API_CMS_ERROR_FILE_WRITE;
        goto end;
    }

end:
    fclose(fp);

    return err;
}

int32_t adi_apollo_utilities_mkdir(adi_apollo_device_t *device, bool parents, char *path)
{
    int err;
    char command[MAX_PATH_LENGTH + 10];

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_LOG_FUNC();
    ADI_APOLLO_NULL_POINTER_RETURN(path);
    ADI_APOLLO_INVALID_PARAM_RETURN(strlen(path) > MAX_PATH_LENGTH);

    sprintf(command, "mkdir %s %s", parents ? "-p" : "", path);
    err = system(command);
    if (err) {
        return API_CMS_ERROR_FILE_OPERATION;
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_apollo_utilites_file_to_16b_samples_arr(adi_apollo_device_t *device, char *file_name, int16_t **sample_arr, uint32_t *arr_size, uint32_t max_samples)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t sample_cnt = 0;
    uint32_t num_samples = 0;
    char line[256];

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_APOLLO_LOG_FUNC();
    ADI_CMS_NULL_PTR_CHECK(file_name);
    ADI_CMS_NULL_PTR_CHECK(sample_arr);
    ADI_CMS_NULL_PTR_CHECK(arr_size);

    /* Open file of samples */
    FILE *fp = fopen(file_name, "r");
    ADI_CMS_FILE_OPEN_CHECK(fp);

    /* get number of samples in file*/
    while (fgets(line, sizeof(line), fp) != NULL) {
        num_samples++;
    }
    ADI_CMS_FILE_CLOSE(fp);

    /* allocate memory */
    if (max_samples != 0) {
        num_samples = (num_samples > max_samples) ? max_samples : num_samples;
    }
    *sample_arr = (int16_t *)calloc(num_samples, sizeof(int16_t));
    if (*sample_arr == NULL) {
        ADI_CMS_ERROR_GOTO(API_CMS_ERROR_MEM_ALLOC, end);
    }

    fp = fopen(file_name, "r");
    ADI_CMS_FILE_OPEN_CHECK(fp);

    /* write samples to array */
    while (fscanf(fp, "%hd", &(*sample_arr)[sample_cnt]) != EOF) {
        sample_cnt++;
        if (sample_cnt >= num_samples) {
            break;
        }
    }

    *arr_size = sample_cnt;

end:
    ADI_CMS_FILE_CLOSE(fp);
    return err;
}

