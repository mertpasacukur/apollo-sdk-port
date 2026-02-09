/*!
 * \brief     ADL6332 Signal Path Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_adl6332_sigpath.h"
#include "adi_adl6332_hal.h"

#define SIG_PATH0_2_ADDR   0x10A // RFSTATE A
#define SIG_PATH1_2_ADDR   0x10B // RFSTATE B
#define SIG_PATH2_2_ADDR   0x10C // RFSTATE C
#define SIG_PATH3_2_ADDR   0x10D // RFSTATE D
#define SIG_PATH4_1_ADDR   0x107 // CROSS_Z
#define SIG_PATH4_3_ADDR   0x113 // CROSS_Z
#define SIG_PATH5_1_ADDR   0x108 // CROSS_P
#define SIG_PATH5_3_ADDR   0x114 // CROSS_P
#define SIG_PATH1_1_ADDR   0x104 // AMP1 IGREF
#define SIG_PATH2_1_ADDR   0x105 // AMP1 IDREF_Z
#define SIG_PATH3_1_ADDR   0x106 // AMP1 IDREF_P
#define SIG_PATH1_3_ADDR   0x110 // AMP2 IGREF
#define SIG_PATH2_3_ADDR   0x111 // AMP2 IDREF_Z
#define SIG_PATH3_3_ADDR   0x112 // AMP2 IDREF_P

#define NVM_IGREF_OFFSET   7
#define IGREF_OFFSET       0
#define NVM_IDREF_Z_OFFSET 7
#define IDREF_Z_OFFSET     0
#define NVM_IDREF_P_OFFSET 7
#define IDREF_P_OFFSET     0
#define AMP1_BYPASS_OFFSET 6
#define AMP2_BYPASS_OFFSET 7
#define DSA_ATTN_OFFSET    0

#define __ADL6332_RFSTATE_SET(adl6332, chip_id, addr, err, config) \
    err = adi_adl6332_hal_reg_set(adl6332, chip_id, addr, \
        (  (!!config->amp1_bypass) << AMP1_BYPASS_OFFSET) \
        | ((!!config->amp2_bypass) << AMP2_BYPASS_OFFSET) \
        | ((config->dsa_attn)      << DSA_ATTN_OFFSET)); \
    ADI_CMS_ERROR_RETURN(err);

static int32_t crossp_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ptat_bias_trm_e trim);
static int32_t crossz_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ztat_bias_trm_e trim);

int32_t adi_adl6332_sigpath_rfstate_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_rfstate_select_e rfstate, adi_adl6332_rfstate_t *config)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_RANGE_CHECK(rfstate, ADI_ADL6332_RFSTATE_SELECT_A, ADI_ADL6332_RFSTATE_SELECT_D);
    ADI_CMS_NULL_PTR_CHECK(config);
    ADI_CMS_RANGE_CHECK(config->dsa_attn, ADI_ADL6332_DSA_ATTN_MIN, ADI_ADL6332_DSA_ATTN_MAX);

    switch (rfstate) {
        case ADI_ADL6332_RFSTATE_SELECT_A:
            __ADL6332_RFSTATE_SET(adl6332, chip_id, SIG_PATH0_2_ADDR, err, config)
            break;
        case ADI_ADL6332_RFSTATE_SELECT_B:
            __ADL6332_RFSTATE_SET(adl6332, chip_id, SIG_PATH1_2_ADDR, err, config)
            break;
        case ADI_ADL6332_RFSTATE_SELECT_C:
            __ADL6332_RFSTATE_SET(adl6332, chip_id, SIG_PATH2_2_ADDR, err, config)
            break;
        case ADI_ADL6332_RFSTATE_SELECT_D:
            __ADL6332_RFSTATE_SET(adl6332, chip_id, SIG_PATH3_2_ADDR, err, config)
            break;
        default:
            return API_CMS_ERROR_ERROR;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adl6332_sigpath_crossp_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ptat_bias_trm_e trim)
{
    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_RANGE_CHECK(trim, ADI_ADL6332_PTAT_BIAS_TRM_400uA, ADI_ADL6332_PTAT_BIAS_TRM_960uA);

    return crossp_set(adl6332, chip_id, amp, trim);
}

int32_t adi_adl6332_sigpath_crossz_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ztat_bias_trm_e trim)
{
    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_RANGE_CHECK(trim, ADI_ADL6332_ZTAT_BIAS_TRM_0p80mA, ADI_ADL6332_ZTAT_BIAS_TRM_2p06mA);

    return crossz_set(adl6332, chip_id, amp, trim);
}

int32_t adi_adl6332_sigpath_trim_config_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_trim_config_t *config)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(config);
    ADI_CMS_RANGE_CHECK(config->igref,   ADI_ADL6332_MAIN_BIAS_TRM_150mA,  ADI_ADL6332_MAIN_BIAS_TRM_250mA);
    ADI_CMS_RANGE_CHECK(config->idref_z, ADI_ADL6332_ZTAT_BIAS_TRM_0p80mA, ADI_ADL6332_ZTAT_BIAS_TRM_2p06mA);
    ADI_CMS_RANGE_CHECK(config->idref_p, ADI_ADL6332_PTAT_BIAS_TRM_400uA,  ADI_ADL6332_PTAT_BIAS_TRM_960uA);
    ADI_CMS_RANGE_CHECK(config->cross_z, ADI_ADL6332_ZTAT_BIAS_TRM_0p80mA, ADI_ADL6332_ZTAT_BIAS_TRM_2p06mA);
    ADI_CMS_RANGE_CHECK(config->cross_p, ADI_ADL6332_PTAT_BIAS_TRM_400uA,  ADI_ADL6332_PTAT_BIAS_TRM_960uA);

    if (ADI_CMS_MASK_MATCH(amp, ADI_ADL6332_AMP_SELECT_1)) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH1_1_ADDR,
              ((!!config->use_nvm_igref) << NVM_IGREF_OFFSET) | ((config->igref) << IGREF_OFFSET));
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH2_1_ADDR,
              ((!!config->use_nvm_idref_z) << NVM_IDREF_Z_OFFSET) | ((config->idref_z) << IDREF_Z_OFFSET));
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH3_1_ADDR,
              ((!!config->use_nvm_idref_p) << NVM_IDREF_P_OFFSET) | ((config->idref_p) << IDREF_P_OFFSET));
        ADI_CMS_ERROR_RETURN(err);

        err = crossz_set(adl6332, chip_id, ADI_ADL6332_AMP_SELECT_1, config->cross_z);
        ADI_CMS_ERROR_RETURN(err);

        err = crossp_set(adl6332, chip_id, ADI_ADL6332_AMP_SELECT_1, config->cross_p);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (ADI_CMS_MASK_MATCH(amp, ADI_ADL6332_AMP_SELECT_2)) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH1_3_ADDR,
              ((!!config->use_nvm_igref) << NVM_IGREF_OFFSET) | ((config->igref) << IGREF_OFFSET));
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH2_3_ADDR,
              ((!!config->use_nvm_idref_z) << NVM_IDREF_Z_OFFSET) | ((config->idref_z) << IDREF_Z_OFFSET));
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH3_3_ADDR,
              ((!!config->use_nvm_idref_p) << NVM_IDREF_P_OFFSET) | ((config->idref_p) << IDREF_P_OFFSET));
        ADI_CMS_ERROR_RETURN(err);

        err = crossz_set(adl6332, chip_id, ADI_ADL6332_AMP_SELECT_2, config->cross_z);
        ADI_CMS_ERROR_RETURN(err);

        err = crossp_set(adl6332, chip_id, ADI_ADL6332_AMP_SELECT_2, config->cross_p);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

static int32_t crossp_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ptat_bias_trm_e trim)
{
    int32_t err;

    if (ADI_CMS_MASK_MATCH(amp, ADI_ADL6332_AMP_SELECT_1)) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH5_1_ADDR, trim);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (ADI_CMS_MASK_MATCH(amp, ADI_ADL6332_AMP_SELECT_2)) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH5_3_ADDR, trim);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

static int32_t crossz_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ztat_bias_trm_e trim)
{
    int32_t err;

    if (ADI_CMS_MASK_MATCH(amp, ADI_ADL6332_AMP_SELECT_1)) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH4_1_ADDR, trim);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (ADI_CMS_MASK_MATCH(amp, ADI_ADL6332_AMP_SELECT_2)) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH4_3_ADDR, trim);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}
