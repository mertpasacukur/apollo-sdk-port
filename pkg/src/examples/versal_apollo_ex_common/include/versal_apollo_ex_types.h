/*!
 * @brief     Versal Apollo examples common types
 *
 *            Versal Standalone types — replaces original platform types.
 *            Only defines what the Versal example code actually uses.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef __VERSAL_APOLLO_COMMON_EX_TYPES_H__
#define __VERSAL_APOLLO_COMMON_EX_TYPES_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/

/*
 * On Versal baremetal there is no filesystem.
 * FW_IMAGES_DIR is kept for API compatibility but points to QSPI flash offsets
 * managed by versal_fw_provider.c.
 */
#define FW_IMAGES_DIR           "qspi_flash"

#endif /* __VERSAL_APOLLO_COMMON_EX_TYPES_H__ */
