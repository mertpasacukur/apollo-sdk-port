/*!
 * \brief     Header file declaring data types for ADF4030's TDC functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_TDC
 * @{
 */

#ifndef __ADI_ADF4030_TDC_TYPES_H__
#define __ADI_ADF4030_TDC_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_bsyncout_types.h"
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
/**
 * \enum adf4030_tdc_chop_switches_e
 * \brief Force the Chop Value to Use When EN_TDC_PINGPONG= 0.
 */
typedef enum  {
    ADI_ADF4030_CHOP_SWITCH_PASS_THROUGH  = 0x00,   // The chop switches in the TDC path of all active BSYNC receivers are forced to a pass-through configuration (default)
    ADI_ADF4030_CHOP_SWITCH_CROSS_CONNECT = 0x01    // The chop switches in the TDC path of all active BSYNC receivers are forced to a cross-connect configuration
} adf4030_tdc_chop_switches_e;


/**
 * \enum adf4030_tdc_ping_pong_switches_e
 * \brief Enable ping pong switches in the TDC path This bit is meaningful only when AUTO_DELAY_CORR bit is cleared to 0.
 */
typedef enum  {
    ADI_ADF4030_PING_PONG_SWITCH_PASS_THROUGH  = 0x00,  // The ping pong switches in the TDC path are forced to a pass-through configuration (default)
    ADI_ADF4030_PING_PONG_SWITCH_CROSS_CONNECT = 0x01   // The ping pong switches in the TDC path are forced to a cross-connect configuration
} adf4030_tdc_ping_pong_switches_e;


/**
 * \enum adf4030_tdc_ping_pong_e
 * \brief Enables the Pingpong Algorithm for the TDC. This bit enables the automatic path delay correction.
 */
typedef enum  {
    ADI_ADF4030_PING_PONG_DISABLED = 0x00,  // The automatic path delay correction is disabled and CHOP_M and PINGPONG_M bits determine the TDC path setting (default)
    ADI_ADF4030_PING_PONG_ENABLED  = 0x01   // The automatic path delay correction is enabled and CHOP_M and PINGPONG_M bits state does not matter
} adf4030_tdc_ping_pong_e;


/**
 * \enum adf4030_tdc_clk_select_e
 * \brief When this bit is cleared to 0 (default value), the target clock serves as the period clock. When this bit is set to 1, the source clock serves as the period clock
 */
typedef enum  {
    ADI_ADF4030_TDC_CLK_TARGET = 0x00,
    ADI_ADF4030_TDC_CLK_SOURCE = 0x01
} adf4030_tdc_clk_select_e;


/**
 * \enum adf4030_tdc_avg_sample_e
 * \brief Determines the number of TDC measurements that are averaged. The expression of the number of TDC measurements is AVG=64 x 2^AVGEXP. - 2^(CIC_DEC_RATE+6)
 */
typedef enum  {
    ADI_ADF4030_TDC_AVG_64_SAMPLES   = 0x00,
    ADI_ADF4030_TDC_AVG_128_SAMPLES  = 0x01,
    ADI_ADF4030_TDC_AVG_256_SAMPLES  = 0x02,
    ADI_ADF4030_TDC_AVG_512_SAMPLES  = 0x03,
    ADI_ADF4030_TDC_AVG_1024_SAMPLES = 0x04,
    ADI_ADF4030_TDC_AVG_2048_SAMPLES = 0x05,
    ADI_ADF4030_TDC_AVG_4096_SAMPLES = 0x06, // default
    ADI_ADF4030_TDC_AVG_8192_SAMPLES = 0x07,
    ADI_ADF4030_TDC_AVG_16K_SAMPLES  = 0x08,
    ADI_ADF4030_TDC_AVG_32K_SAMPLES  = 0x09,
    ADI_ADF4030_TDC_AVG_65K_SAMPLES  = 0x0A,
    ADI_ADF4030_TDC_AVG_131K_SAMPLES = 0x0B,
    ADI_ADF4030_TDC_AVG_262K_SAMPLES = 0x0C,
    ADI_ADF4030_TDC_AVG_524K_SAMPLES = 0x0D,
    ADI_ADF4030_TDC_AVG_1M_SAMPLES   = 0x0E,
    ADI_ADF4030_TDC_AVG_2M_SAMPLES   = 0x0F,
} adf4030_tdc_avg_sample_e;


/*!
 * \brief ADF4030 TDC measurement Config
 */
typedef struct {
    uint32_t common_tdc_offset;     // Common TDC Offset to Apply to ∆Ndel.
    uint16_t tdc_channel_offsets[10];
    uint8_t tdc_rshift;
    uint8_t tdc_timeout_start;      // The duration of the pause is t_SETTLE = 8 x (tdc_timeout_start + 1) x t_CORECLOCK.
    uint8_t tdc_timeout_phase;      // Pause Duration after a TDC Measurement Chop or Ping Pong Switch Event
    adf4030_tdc_clk_select_e tdc_clk_select;
    adf4030_tdc_avg_sample_e tdc_avg_sample_number;
    adf4030_tdc_chop_switches_e chop_switch;
    adf4030_tdc_ping_pong_switches_e ping_pong_switch;
    adf4030_tdc_ping_pong_e ping_pong_algo;
} adi_adf4030_tdc_config_t;


#endif // !__ADI_ADF4030_TDC_TYPES_H__

/*! @} */
