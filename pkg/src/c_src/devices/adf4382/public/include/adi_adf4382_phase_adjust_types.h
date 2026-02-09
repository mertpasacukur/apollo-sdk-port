/*!
 * \brief     ADF4382 Phase Adjustment Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADF4382_PHASE_ADJUST_TYPES_H__
#define __ADI_ADF4382_PHASE_ADJUST_TYPES_H__

#include "adi_adf4382_types.h"

/*!
 * \brief ADF4382 Phase Adjust Mode select
 */
typedef enum {
    ADI_ADF4382_PHASE_ADJ_BLEED_CURRENT = 0x00,
    ADI_ADF4382_PHASE_ADJ_SIGMA_DELTA = 0x01
} adi_adf4382_phase_adjust_mode_e;

/*!
 * \brief ADF4382 Phase Adjust Trigger Mode select
 */
typedef enum {
    ADI_ADF4382_PHASE_ADJ_TRIGGER_SPI = 0x00,
    ADI_ADF4382_PHASE_ADJ_TRIGGER_2WIRE = 0x01
} adi_adf4382_phase_adjust_trigger_mode_e;

/*!
 * \brief ADF4382 Phase Adjustment config structure
 */
typedef struct {
    uint64_t ref_freq_hz;
    uint64_t rfout_freq_hz;
    int32_t phase_value_fs; // femto seconds
    adi_adf4382_phase_adjust_mode_e phase_mode;
    adi_adf4382_phase_adjust_trigger_mode_e trigger_mode;
} adi_adf4382_phase_adjust_config_t;



#endif // !__ADI_ADF4382_PHASE_ADJUST_TYPES_H__