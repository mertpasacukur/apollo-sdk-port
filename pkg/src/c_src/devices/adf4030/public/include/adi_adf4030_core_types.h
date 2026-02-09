/*!
 * \brief     Header file declaring data types for ADF4030's basic core functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_CORE
 * @{
 */

#ifndef __ADI_ADF4030_CORE_TYPES_H__
#define __ADI_ADF4030_CORE_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
/**
 * \brief   ADF4030 API Version.
 */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_adf4030_core_version_t;

#endif // !__ADI_ADF4030_CORE_TYPES_H__

/*! @} */
