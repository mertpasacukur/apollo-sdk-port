/*!
 * \brief     Header file declaring data types for ADF4030's BSYNC Termination functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADF4030_BSYNC_TERM
 * @{
 */

#ifndef __ADI_ADF4030_BSYNC_TERM_TYPES_H__
#define __ADI_ADF4030_BSYNC_TERM_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
/*!
 * \brief   ADF4030 Bsync Termination Config
 */
typedef struct {
    uint8_t ac_coupled;
    uint8_t link_tx;    // CON_RL_DRV_
    uint8_t link_rx;    // CON_RL_RCV_
    uint8_t float_tx;   // FLOAT_ON_DRV_
    uint8_t float_rx;   // FLOAT_ON_RCV_
} adi_adf4030_bsync_term_config_t;

#endif // !__ADI_ADF4030_BSYNC_TERM_TYPES_H__

/*! @} */
