/*!
 * \brief     ADF4382 MUXOUT Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_MUXOUT_TYPES_H__
#define __ADI_ADF4382_MUXOUT_TYPES_H__


#include "adi_adf4382_types.h"
/*!
 * \brief ADF4382 MUXOUT Output Controls
 */
typedef enum {
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_HIGH_Z,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LKDET,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LOW_0,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LOW_1,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_DIV_RCLK_2,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_DIV_NCLK_2,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_VCO_TESTMODE,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LOW_2,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_HIGH,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_VCOCAL_RBAND_2,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_VCOCAL_NBAND_2,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LOW_3,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LOW_4,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_LOW_5,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_REF_OK,
    ADI_ADF4382_MUXOUT_OUTPUTCTRL_ANALOG_TEST
} adi_adf4382_muxout_outputctrl_e;

#endif // !__ADI_ADF4382_TYPES_H__