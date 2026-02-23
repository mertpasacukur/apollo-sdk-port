/*!
 * @brief     FW image provider header for Versal (QSPI flash).
 *
 *            Reads firmware images from MT25QU02G QSPI flash instead of
 *            the filesystem-based approach used on the original Linux platform.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef __VERSAL_FW_PROVIDER_H__
#define __VERSAL_FW_PROVIDER_H__

#include <stdint.h>
#include "adi_apollo_startup_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Creates an instance of a Versal FW image provider (QSPI-based).
 *
 * @param[in] device        Context variable — Pointer to the APOLLO device data structure.
 * @param[in] fw_image_dir  Unused on Versal (kept for API compatibility). Pass FW_IMAGES_DIR.
 *
 * @return  Pointer to adi_apollo_fw_provider_t on success, NULL on failure.
 */
adi_apollo_fw_provider_t *versal_fw_provider_create(adi_apollo_device_t *device, char *fw_image_dir);

/**
 * @brief   Called before obtaining a FW binary. Logs the FW ID being loaded.
 */
int32_t versal_fw_provider_open(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id);

/**
 * @brief   Called after obtaining a FW binary. Frees the allocated buffer.
 */
int32_t versal_fw_provider_close(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id);

/**
 * @brief   Reads FW binary from QSPI flash. Allocates buffer and returns pointer.
 *
 * Flash layout per partition: [256-byte header][raw firmware data]
 * Header: [4B fw_id][4B fw_size][4B checksum][244B reserved]
 * TODO: fw_id and checksum validation not yet implemented.
 */
int32_t versal_fw_provider_get(adi_apollo_fw_provider_t *fw_provider,
                               adi_apollo_startup_fw_id_e fw_id,
                               uint8_t **byte_arr, uint32_t *bytes_read);

#ifdef __cplusplus
}
#endif

#endif /* __VERSAL_FW_PROVIDER_H__ */
