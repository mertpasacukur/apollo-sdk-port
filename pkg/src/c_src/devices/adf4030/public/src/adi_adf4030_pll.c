/*!
 * \brief     Source file implementing APIs for ADF4030's PLL Output functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_PLL
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_pll.h"
#include "adi_adf4030_bf.h"

/*============= C O D E ====================*/


int32_t adi_adf4030_pll_config_set(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, adi_adf4030_pll_config_t *config)
{
    int32_t err;
    uint64_t pfd_freq;
    uint64_t vco_freq;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(config);
    ADI_ADF4030_VALIDATE_RANGE(ref_input_freq, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);
    ADI_ADF4030_VALIDATE_RANGE(config->r_div, ADI_ADF4030_R_DIV_MIN, ADI_ADF4030_R_DIV_MAX);
    ADI_ADF4030_VALIDATE_RANGE(config->n_div, ADI_ADF4030_N_DIV_MIN, ADI_ADF4030_N_DIV_MAX);

    pfd_freq = ref_input_freq / (config->r_div);
    ADI_ADF4030_VALIDATE_RANGE(pfd_freq, ADI_ADF4030_PFD_FREQ_MIN, ADI_ADF4030_PFD_FREQ_MAX);

    config->pfd_freq = pfd_freq;

    vco_freq = pfd_freq * config->n_div;
    ADI_ADF4030_VALIDATE_RANGE(vco_freq, ADI_ADF4030_VCO_FREQ_MIN, ADI_ADF4030_VCO_FREQ_MAX);

    config->vco_freq = vco_freq;


    err = adi_adf4030_bf___REG0057___RDIV_set(adf4030, config->r_div);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0056___NDIV_set(adf4030, config->n_div);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_pll_config_get(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, adi_adf4030_pll_config_t *config)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(config);
    ADI_ADF4030_VALIDATE_RANGE(ref_input_freq, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);

    err = adi_adf4030_bf___REG0057___RDIV_get(adf4030, &config->r_div);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0056___NDIV_get(adf4030, &config->n_div);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->pfd_freq = ref_input_freq / (config->r_div);
    config->vco_freq = (config->pfd_freq) * (config->n_div);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_pll_freq_set(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, uint64_t vco_out_freq)
{
    int32_t err;
    uint64_t pfd_freq;
    uint32_t n_div = ADI_ADF4030_N_DIV_MIN;
    uint64_t r_div = 0;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(ref_input_freq, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);
    ADI_ADF4030_VALIDATE_RANGE(vco_out_freq, ADI_ADF4030_VCO_FREQ_MIN, ADI_ADF4030_VCO_FREQ_MAX);


    pfd_freq = vco_out_freq / n_div;

    while (pfd_freq > ADI_ADF4030_PFD_FREQ_MIN) {

        pfd_freq = vco_out_freq / (++n_div);
        ADI_ADF4030_VALIDATE_RANGE(n_div, ADI_ADF4030_N_DIV_MIN, ADI_ADF4030_N_DIV_MAX);

        if (((ref_input_freq % pfd_freq) == 0) && (pfd_freq < ADI_ADF4030_PFD_FREQ_MAX)) {
            break;
        }
    }

    // #WARNING: ref_input_freq can not be ref_input_freq = 10 MHZ PFD = 20 MHZ
    if (ref_input_freq % pfd_freq) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    r_div = ref_input_freq / pfd_freq;

    err = adi_adf4030_bf___REG0057___RDIV_set(adf4030, r_div);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0056___NDIV_set(adf4030, n_div);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0056___NDIV_set(adf4030, n_div);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_pll_cal_enable(adi_adf4030_device_t *adf4030, uint8_t enable)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(enable, 0, 1);

    err = adi_adf4030_bf___REG005a___PLL_CAL_EN_set(adf4030, enable);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


/*! @} */
