/*!
 * \brief     Versal Apollo examples common ADF4030 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4030_types.h"
#include "adi_adf4030_hal.h"
#include "adi_adf4030_bf.h"

#include "adi_adf4030_auto_align.h"
#include "adi_adf4030_bsync_term.h"
#include "adi_adf4030_bsyncout.h"
#include "adi_adf4030_core.h"
#include "adi_adf4030_ldctrl.h"
#include "adi_adf4030_pdctrl.h"
#include "adi_adf4030_pll.h"
#include "adi_adf4030_tdc.h"

#ifndef __VERSAL_APOLLO_EX_ADF4030_H__
#define __VERSAL_APOLLO_EX_ADF4030_H__

int32_t versal_apollo_ex_adf4030_configure_hal(adi_adf4030_device_t *adf4030, uint8_t dev_id);

int32_t versal_apollo_ex_adf4030_vco_freq_calc(uint64_t ref_input_freq, uint64_t bsync_out_freq, uint64_t *vco_out_freq);

int32_t versal_apollo_ex_adf4030_startup(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq_hz, uint64_t vco_out_freq_hz);

int32_t versal_apollo_ex_adf4030_bsync_output_set(adi_adf4030_device_t *adf4030,
                                                     uint16_t bsync_out_ch_sel,
                                                     uint64_t vco_out_freq_hz,
                                                     uint64_t bsync_out_freq_hz,
                                                     uint8_t boost_en);

int32_t versal_apollo_ex_adf4030_bsync_input_set(adi_adf4030_device_t *adf4030, uint16_t bsync_in_ch_sel);

int32_t versal_apollo_ex_adf4030_align_bsync_out(adi_adf4030_device_t *adf4030,
                                                    uint16_t bsync_in_ch_sel,
                                                    uint16_t bsync_out_ch_sel,
                                                    uint64_t bsync_out_freq_hz);

#endif /* __VERSAL_APOLLO_EX_ADF4030_H__ */
