/*!
 * \brief     Header file declaring data types for ADF4030's BSYNC Output functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_BSYNCOUT
 * @{
 */

#ifndef __ADI_ADF4030_BSYNCOUT_TYPES_H__
#define __ADI_ADF4030_BSYNCOUT_TYPES_H__

#define ADI_ADF4030_BSYNC_CHANNEL_NUM   (10U)   // ADF4030 has 10 Bidirectional Synchronized Clock (BSYNC) Channels.

/*============= I N C L U D E S ============*/
#include "adi_adf4030_types.h"


/*============= D E F I N E S ==============*/
/**
 * @enum    adi_adf4030_odiv_select_e
 * @brief   ID of ADF4030 Bsync channels' odiv selection.
 */
typedef enum  {
    ADI_ADF4030_CHANNEL_ODIV1 = 0x00,
    ADI_ADF4030_CHANNEL_ODIV2 = 0x01
} adi_adf4030_odiv_select_e;


/**
 * @enum    adi_adf4030_bsync_drive_e
 * @brief   ID of ADF4030 Bsync channels' bysnc drive selection.
 */
typedef enum  {
    ADI_ADF4030_CHANNEL_BSYNC_DRIVE_RX = 0x00,      //  Disables Transmit driver, setting the BSYNC pins as IDLE.
    ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX = 0x01       //  Enables Transmit driver, actively drives the BSYNC pins.
} adi_adf4030_bsync_drive_e;


/*!
 * \brief   ADF4030 Channel Config
 */
typedef struct {
    uint8_t pd_tx_path;
    uint8_t channel_boost;
    uint64_t bsync_out_freq_hz;
    adi_adf4030_bsync_drive_e bsync_drive;
    adi_adf4030_channel_id_e channel_id;
    adi_adf4030_odiv_select_e odiv_select;
} adi_adf4030_channel_config_t;


/*!
 * \brief   ADF4030 Bsync Output Config
 */
typedef struct {
    uint16_t o_div1;
    uint16_t o_div2;
    uint64_t ref_input_freq_hz;
    adi_adf4030_channel_config_t bsync_channels[ADI_ADF4030_BSYNC_CHANNEL_NUM];
} adi_adf4030_bsyncout_config_t;

#endif // !__ADI_ADF4030_BSYNCOUT_TYPES_H__

/*! @} */
