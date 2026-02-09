/*!
 * \brief     HMC7044 Core Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_CORE_TYPES_H__
#define __ADI_HMC7044_CORE_TYPES_H__

#include "adi_hmc7044_types.h"

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_hmc7044_version_t;

#endif // !__ADI_HMC7044_CORE_TYPES_H__