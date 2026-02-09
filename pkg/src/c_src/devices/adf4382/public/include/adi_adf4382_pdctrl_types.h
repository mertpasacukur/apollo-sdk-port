/*!
 * \brief     ADF4382 Power Down Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_PDCTRL_TYPES_H__
#define __ADI_ADF4382_PDCTRL_TYPES_H__

#include "adi_adf4382_types.h"
/*!
 * \brief ADF4382 power down control config
 */

typedef enum {
    ADI_ADF4382_PDCTRL_TARGET_CLKOUT1 = 0x01,
    ADI_ADF4382_PDCTRL_TARGET_CLKOUT2 = 0x02,
    ADI_ADF4382_PDCTRL_TARGET_PFD_CP = 0x04,
    ADI_ADF4382_PDCTRL_TARGET_LD = 0x08,
    ADI_ADF4382_PDCTRL_TARGET_VCO = 0x10,
    ADI_ADF4382_PDCTRL_TARGET_NDIV = 0x20,
    ADI_ADF4382_PDCTRL_TARGET_RDIV = 0x40,
    ADI_ADF4382_PDCTRL_TARGET_ALL    = 0x80,
    ADI_ADF4382_PDCTRL_TARGET_REF_DETECT = 0x100,
    ADI_ADF5382_PDCTRL_TARGET_SYNC = 0x200
} adi_adf4382_pdctrl_target_e;

#endif // !__ADI_ADF4382_TYPES_H__