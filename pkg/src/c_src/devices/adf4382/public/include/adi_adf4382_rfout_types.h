/*!
 * \brief     ADF4382 RFOUT Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_RFOUT_TYPES_H__
#define __ADI_ADF4382_RFOUT_TYPES_H__

#define ADI_ADF4382_REF_MIN                   10000000U    // 10MHz
#define ADI_ADF4382_REF_MAX                   5000000000U  // 5GHz
#define ADI_ADF4382_VCO_FREQ_MIN              11000000000U // 11GHz
#define ADI_ADF4382_VCO_FREQ_MAX              22000000000U // 22GHz
#define ADI_ADF4382_RFOUT_MIN                 687500000U   // 687.50MHz
#define ADI_ADF4382_RFOUT_MAX                 (ADI_ADF4382_VCO_FREQ_MAX)
#define ADI_ADF4382_MOD1WORD                  0x2000000U   // 2^25
#define ADI_ADF4382_MOD2WORD_MAX              0xFFFFFFU    // 2^24 - 1
#define ADI_ADF4382_PHASE_RESYNC_MOD2WORD_MAX 0x1FFFFU     // 2^17 - 1
#define ADI_ADF4382_CHANNEL_SPACING_MAX       78125U
#define ADI_ADF4382_DCLK_DIV1_0_MAX           160000000U   // 160MHz
#define ADI_ADF4382_DCLK_DIV1_1_MAX           320000000U   // 320MHz
#define ADI_ADF4382_M_VCO_BIAS_POST_CAL_FIX   0x3F
#define ADI_ADF4382_CLK_OPWR_MIN              0U
#define ADI_ADF4382_CLK_OPWR_MAX              11U
#define ADI_ADF4382_LD_COUNT_MIN              (ADI_ADF4382_CLK_OPWR_MIN)
#define ADI_ADF4382_LD_COUNT_MAX              31U
#define ADI_ADF4382_R_DIV_MIN                 1U
#define ADI_ADF4382_R_DIV_MAX                 63U
#define ADI_ADF4382_RFOUT_FPFD_MIN_HZ         540U
#define ADI_ADF4382_RFOUT_FPFD_MAX_HZ_INT     625000000U
#define ADI_ADF4382_RFOUT_FPFD_MAX_HZ_FRAC    250000000U

#include "adi_adf4382_types.h"
/*!
 * \brief ADF4382 output frequency config
 */
typedef struct {
    uint64_t ref_freq_hz;
    uint64_t rfout_freq_hz;
    uint8_t en_doubler;
    uint8_t r_div;
    uint8_t ld_count;
    uint8_t clk1_opwr;
    uint8_t clk2_opwr;
} adi_adf4382_rfout_output_config_t;

#endif // !__ADI_ADF4382_TYPES_H__