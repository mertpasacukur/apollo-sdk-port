/*!
 * @brief     FW image provider header for Versal (QSPI Flash based).
 *            Ported from ads10_fw_provider.h — reads FW images from MT25QU02G QSPI flash
 *            at predefined offsets using Xilinx PS QSPI driver.
 */

#ifndef __VERSAL_FW_PROVIDER__
#define __VERSAL_FW_PROVIDER__

#include <stdint.h>
#include "adi_apollo_startup_types.h"

/*============= D E F I N E S ==============*/

/*
 * FW image flash offsets for MT25QU02G.
 * These must match the flash programming layout.
 * TODO: Update offsets based on actual flash partition table.
 */
#define VERSAL_FW_FLASH_OFFSET_CPU0_B           0x01000000
#define VERSAL_FW_FLASH_OFFSET_CPU1_B           0x01100000
#define VERSAL_FW_FLASH_OFFSET_SE_01030000      0x01030000
#define VERSAL_FW_FLASH_OFFSET_SE_20000000      0x01200000
#define VERSAL_FW_FLASH_OFFSET_SE_02000000      0x01400000
#define VERSAL_FW_FLASH_OFFSET_SE_21000000      0x01600000
#define VERSAL_FW_FLASH_OFFSET_SEP_01030000     0x01800000
#define VERSAL_FW_FLASH_OFFSET_SEP_20000000     0x01A00000
#define VERSAL_FW_FLASH_OFFSET_SEP_02000000     0x01C00000
#define VERSAL_FW_FLASH_OFFSET_SEP_21000000     0x01E00000

/* Max FW image size (2MB should be sufficient) */
#define VERSAL_FW_MAX_IMAGE_SIZE                (2 * 1024 * 1024)

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Creates a Versal FW image provider (QSPI flash based).
 *          Equivalent to ads10_fw_provider_create().
 */
adi_apollo_fw_provider_t *versal_fw_provider_create(adi_apollo_device_t *device);

/**
 * @brief   Open FW provider for a specific FW ID. Prepares QSPI for read.
 */
int32_t versal_fw_provider_open(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id);

/**
 * @brief   Close FW provider after reading.
 */
int32_t versal_fw_provider_close(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id);

/**
 * @brief   Get FW bytes from QSPI flash.
 */
int32_t versal_fw_provider_get(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id,
                                uint8_t **byte_arr, uint32_t *bytes_read);

#ifdef __cplusplus
}
#endif
#endif /* __VERSAL_FW_PROVIDER__ */
