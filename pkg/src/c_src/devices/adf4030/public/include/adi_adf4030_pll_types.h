/*!
 * \brief     Header file declaring data types for ADF4030's PLL Output functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_PLL
 * @{
 */

#ifndef __ADI_ADF4030_PLL_TYPES_H__
#define __ADI_ADF4030_PLL_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
/*!
 * \brief ADF4030 PLL Output Config
 */
typedef struct {
    uint8_t r_div;
    uint8_t n_div;
    uint64_t pfd_freq;
    uint64_t vco_freq;
} adi_adf4030_pll_config_t;

#endif // !__ADI_ADF4030_PLL_TYPES_H__

/*! @} */
