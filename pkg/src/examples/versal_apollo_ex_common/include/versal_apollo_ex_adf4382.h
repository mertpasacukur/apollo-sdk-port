/*!
 * \brief     Versal Apollo examples common ADF4382 functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_types.h"
#include "adi_adf4382_regmap_inline_bfs.h"

#include "adi_adf4382_clkctrl.h"
#include "adi_adf4382_core.h"
#include "adi_adf4382_cpctrl.h"
#include "adi_adf4382_ldctrl.h"
#include "adi_adf4382_muxout.h"
#include "adi_adf4382_pdctrl.h"
#include "adi_adf4382_phase_adjust.h"
#include "adi_adf4382_rfout.h"

#include "versal_hal.h"

#ifndef __VERSAL_APOLLO_EX_ADF4382_H__
#define __VERSAL_APOLLO_EX_ADF4382_H__

/**
 * @brief MCS Init Cal specific ADF4382 configuration.
 */
typedef struct {
    adi_adf4382_cpctrl_bleed_word_config_t bleed_word_config;
    adi_adf4382_cpctrl_bleed_polarity_e bleed_pol;
    uint8_t bleed_adj_cal;
    adi_adf4382_phase_adjust_config_t phase_adj_config;
    uint8_t en_bleed;
    uint8_t en_phase_resync;
    uint8_t en_auto_align;
} adi_apollo_ex_adf4382_mcs_init_config_t;

int32_t adi_ads10_apollo_ex_adf4382_hal_config(adi_adf4382_device_t *adf4382, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

int32_t adi_ads10_apollo_ex_adf4382_startup(adi_adf4382_device_t *adf4382,
                                            uint64_t rfout_freq_hz, uint64_t ref_freq_hz);

int32_t adi_ads10_apollo_ex_adf4382_mcs_init_config(adi_adf4382_device_t *adf4382,
                                                    adi_apollo_ex_adf4382_mcs_init_config_t *adf4382_mcs_init_config,
                                                    uint8_t *phase_adj_rb);

int32_t adi_ads10_apollo_ex_adf4382_del_cnt_get(adi_adf4382_device_t *adf4382, uint8_t *bleed_pol, int8_t *coarse_current, int16_t *fine_current);

#endif /* __VERSAL_APOLLO_EX_ADF4382_H__ */
