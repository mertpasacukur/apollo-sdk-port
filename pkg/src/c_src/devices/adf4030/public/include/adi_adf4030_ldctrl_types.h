/*!
 * \brief     Header file declaring data types for ADF4030's Lock Detect functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_LDCTRL
 * @{
 */

#ifndef __ADI_ADF4030_LDCTRL_TYPES_H__
#define __ADI_ADF4030_LDCTRL_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
/**
 * \brief   ADF4030 Lock Detect Config Struct
 */
typedef struct {
    uint8_t ld_count;
    uint8_t en_ldwin;
    uint8_t en_loss_of_lock;
} adi_adf4030_ldctrl_config_t;

#endif // !__ADI_ADF4030_LDCTRL_TYPES_H__

/*! @} */
