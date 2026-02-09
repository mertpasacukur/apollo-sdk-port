/*!
 * \brief     ADF4382 Lock Detect Control Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_LDCTRL_TYPES_H__
#define __ADI_ADF4382_LDCTRL_TYPES_H__

#include "adi_adf4382_types.h"
/*!
 * \brief ADF4382 lock detect control config
 */

typedef struct {
    uint8_t en_ldwin;
    uint8_t en_loss_of_lock;
} adi_adf4382_ldctrl_config_t;

#endif // !__ADI_ADF4382_TYPES_H__