/*!
 * \brief     ADF4382 CORE Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_CORE_TYPES_H__
#define __ADI_ADF4382_CORE_TYPES_H__

#include "adi_adf4382_types.h"

/**
 * \brief ADF4382 API Version.
 */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_adf4382_version_t;

#endif // !__ADI_ADF4382_CORE_TYPES_H__