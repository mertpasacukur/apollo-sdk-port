/*!
 * \brief     Header file declaring data types for ADF4030's Power Down control functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_PDCTRL
 * @{
 */

#ifndef __ADI_ADF4030_PDCTRL_TYPES_H__
#define __ADI_ADF4030_PDCTRL_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
/*!
 * \brief   ADF4030 power down control config
 */
typedef enum {
    ADI_ADF4030_PDCTRL_ALL   = 0x1,
    ADI_ADF4030_PDCTRL_ADC   = 0x2,
    ADI_ADF4030_PDCTRL_LD    = 0x4,
    ADI_ADF4030_PDCTRL_PLL   = 0x8,
    ADI_ADF4030_PDCTRL_NDIV  = 0x10,
    ADI_ADF4030_PDCTRL_OSC   = 0x20,
    ADI_ADF4030_PDCTRL_PFDCP = 0x40,
    ADI_ADF4030_PDCTRL_PKDET = 0x80,
    ADI_ADF4030_PDCTRL_RDIV  = 0x100,
} adi_adf4030_pdctrl_e;


/*!
 * \brief   ADF4030 power down control config for BSYNC channels.
 */
typedef enum {
    ADI_ADF4030_BSYNC_PDCTRL_PATH_0 = 0x1,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_1 = 0x2,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_2 = 0x4,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_3 = 0x8,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_4 = 0x10,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_5 = 0x20,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_6 = 0x40,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_7 = 0x80,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_8 = 0x100,
    ADI_ADF4030_BSYNC_PDCTRL_PATH_9 = 0x200,
} adi_adf4030_bsync_pdctrl_e;


#endif // !__ADI_ADF4030_TYPES_H__

/*! @} */
