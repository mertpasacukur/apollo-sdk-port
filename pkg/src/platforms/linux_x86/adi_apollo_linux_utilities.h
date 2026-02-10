/*!
 * \brief     Apollo CPU file load Utilities headers
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
#ifndef __ADI_APOLLO_LINUX_UTILITIES_H__
#define __ADI_APOLLO_LINUX_UTILITIES_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "adi_cms_api_common.h"
#include "adi_apollo_hal.h"
#include "adi_apollo_arm.h"

#define FW_TRANSFER_CHUNK_SIZE          (16 * 1024)     // A0 has max stream limit of 16k

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Create specified directory
 *
 * \param[in]  device           Context variable - Pointer to the APOLLO device data structure
 * \param[in]  parents          No error if existing, make parent directories as needed
 * \param[in]  path             Character array containing path to create
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_apollo_utilities_mkdir(adi_apollo_device_t *device, bool parents, char *path);

/**
 * \brief  Load firmware binary file to CPU ram
 *
 * \param[in]  device           Context variable - Pointer to the APOLLO device data structure
 * \param[in]  file_name        String of Filename with full path
 * \param[in]  cpu_address      CPU firmware memory location
 * \param[in]  chunk_sz_bytes   Number of bytes to transfer to CPU at a time
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_apollo_utilities_fileload(adi_apollo_device_t *device, char *filename, uint32_t cpu_address, uint16_t chunk_sz_bytes);


/**
 * \brief   Platform utility that reads data from a file and store it in a byte array
 *          The binary file data, from the file_name, is read into the array which pointed to, *byte_arr
 *          Function allocates memory depending upon the file size, caller is responsible for freeing allocated memory
 *
 * \param[in]  device               Context variable - Pointer to the APOLLO device data structure
 * \param[in]  file_name            String of binary file with its full path
 * \param[in]  byte_arr             Pointer to the buffer pointer where the binary file data is stored
 * \param[in]  binary_sz_bytes      Size of file in bytes
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_apollo_utilities_file_to_byte_arr(adi_apollo_device_t *device, char *file_name, uint8_t **byte_arr, uint32_t *binary_sz_bytes);

/**
 * \brief   Platform utility that reads data from a file starting at offset and store it in a byte array
 *          The binary file data, from the file_name, is read into the array which pointed to, *byte_arr
 *          Function allocates memory depending upon the length, caller is responsible for freeing allocated memory
 * 
 * \param[in]  device               Context variable - Pointer to the APOLLO device data structure
 * \param[in]  file_name            String of binary file with its full path
 * \param[in]  offset               Displacement of the memory location from the starting location of the file
 * \param[in]  length               Amount of data to read, starting from offset. 
 * \param[in]  byte_arr             Pointer to the buffer pointer where the binary file data is stored
 * \param[in]  binary_sz_bytes      Size of memory allocated for byte_arr in bytes. (Should be same as length)
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_apollo_utilities_file_to_byte_arr_by_offset(adi_apollo_device_t *device, char *file_name, uint32_t offset, uint32_t length, uint8_t **byte_arr, uint32_t *binary_sz_bytes);

/**
 * \brief   Platform utility that writes a byte array to a file.
 *
 * \param[in]  device               Context variable - Pointer to the APOLLO device data structure
 * \param[in]  file_name            String of binary file with its full path
 * \param[in]  byte_arr             Pointer to the byte array
 * \param[in]  binary_sz_bytes      Number of bytes to write to file
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_apollo_utilities_byte_arr_to_file(adi_apollo_device_t *device, char *file_name, uint8_t *byte_arr, size_t binary_sz_bytes);


/**
 * \brief   Platform utility that writes a byte array to a file and can append multiple arrays.
 *
 * \param[in]  device               Context variable - Pointer to the APOLLO device data structure
 * \param[in]  file_name            String of binary file with its full path
 * \param[in]  byte_arr             Pointer to the byte array
 * \param[in]  binary_sz_bytes      Number of bytes to write to file
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_apollo_utilities_byte_arr_to_file_append(adi_apollo_device_t *device, char *file_name, uint8_t *byte_arr, size_t binary_sz_bytes);

/**
 * \brief   Platform utility that reads a file of 16 bit samples
 * 
 * \param[in]  device               Context variable - Pointer to the APOLLO device data structure
 * \param[in]  file_name            String of binary file with its full path
 * \param[in]  sample_arr           Pointer to the array where the samples are stored
 * \param[in]  arr_size             Pointer to number of samples read from file
 * \param[in]  max_samples          Maximum number of samples to read from file (0 if entire file is desired)
 */
int32_t adi_apollo_utilites_file_to_16b_samples_arr(adi_apollo_device_t *device, char *file_name, int16_t **sample_arr, uint32_t *arr_size, uint32_t max_samples);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_APOLLO_LINUX_UTILITIES_H__ */
