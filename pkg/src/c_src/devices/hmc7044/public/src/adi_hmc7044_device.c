/*!
 * \brief     HMC7044 Device Level Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_hmc7044_device.h"
#include "adi_hmc7044_clkout.h"
#include "adi_hmc7044_pll.h"
#include "hmc7044_reg.h"
#include "hmc7044_hal.h"
#include "adi_hmc7044_private_math.h"
#include "adi_hmc7044_private_pll.h"
#include "adi_hmc7044_private_device.h"
#include "adi_hmc7044_private_output.h"

static adi_cms_reg_data_t ADI_RECOMMENDED_INIT_TBL[] = {
    {0x09F,0x4D},
    {0x0A0,0xDF},
    {0x0A5,0x06},
    {0x0A8,0x06},
    {0x0B0,0x04},
};

static int32_t device_init(adi_hmc7044_device_t *hmc7044);

int32_t adi_hmc7044_device_init(adi_hmc7044_device_t *hmc7044)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);

    return device_init(hmc7044);
}

int32_t adi_hmc7044_device_deinit(adi_hmc7044_device_t *hmc7044)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_reset(adi_hmc7044_device_t *hmc7044, uint8_t hw_reset)
{
    int32_t err;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    if (hw_reset) {
        err = hmc7044_hw_reset(hmc7044);
        ADI_HMC7044_CHECK_ERR_OK(err);
    } else {
        err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_SW_RESET_CTRL_REG, HMC7044_SOFT_RESET);
        ADI_HMC7044_CHECK_ERR_OK(err);

        err = hmc7044_sw_delay_us(hmc7044, HMC7044_SPI_RESET_PERIOD_US);
        ADI_HMC7044_CHECK_ERR_OK(err);

        err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_SW_RESET_CTRL_REG, 0x0);
        ADI_HMC7044_CHECK_ERR_OK(err);
    }

    err = hmc7044_sw_delay_us(hmc7044, HMC7044_SPI_RESET_PERIOD_US);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return device_init(hmc7044);
}

int32_t adi_hmc7044_device_sleep_set(adi_hmc7044_device_t *hmc7044, uint8_t sleepmode)
{
    int32_t err;
    uint8_t reg_val;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val &= 0xFE;
    if (sleepmode) {
        reg_val |= 1;
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_clkout_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_clkout_config_t *config)
{
    int32_t err, i;
    adi_hmc70944_pll_locked_e pll_lock_st;
    uint8_t ref_priority[ADI_HMC7044_CLKIN_COUNT];
    uint64_t ref_div, fdist_dec, output_chan_divider,
             clkin_prescaler, vcxo_prescaler,
             pfd2_clk_hz_dec, fvco_clk_hz_dec,
             pll2ref_clk_hz,
             R1, N1, N2, R2;
    uint64_t M1 = 1, M2 = 1;
    adi_hmc7044_int_vco_sel_e vco_sel;
    adi_hmc7044_pll_sel_e enabled_plls;
    adi_hmc7044_device_rational_freq_t pfd2_gcd_hz = { 0 },
                                       pfd2_lcm_hz = { 0 },
                                       pfd1_clk_hz = { 0 },
                                       pfd2_clk_hz = { 0 },
                                       flcm_clk_hz = {
                                           .freq_hz = config->ref_clk_hz,
                                           .div = 1
                                       },
                                       fdist = { 0 },
                                       fvco_clk_hz = { 0 };
    bool is_fvco_clk_valid = false;
    uint64_t vco_window_low_limit[ADI_HMC7044_INT_VCO_COUNT] = {ADI_HMC7044_3GHZ_VCO_HZ_MIN, ADI_HMC7044_2GHZ_VCO_HZ_MIN};
    uint64_t vco_window_hi_limit[ADI_HMC7044_INT_VCO_COUNT] = {ADI_HMC7044_3GHZ_VCO_HZ_MAX, ADI_HMC7044_2GHZ_VCO_HZ_MAX};

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);

    ADI_HMC7044_VALIDATE_RANGE(config->clkin, ADI_HMC7044_CLKIN_NONE, ADI_HMC7044_CLKIN_ALL);
    ADI_HMC7044_VALIDATE_RANGE(config->ref_clk_hz, HMC7044_REF_CLK_FREQ_HZ_MIN, HMC7044_REF_CLK_FREQ_HZ_MAX);
    ADI_HMC7044_VALIDATE_RANGE(config->vcxo_hz, HMC7044_VCXO_CLK_FREQ_HZ_MIN, HMC7044_VCXO_CLK_FREQ_HZ_MAX);

    for (i = 0; i < ADI_HMC7044_CLKIN_COUNT; i++) {
        err = adi_hmc7044_private_pll_enable_input_reference_set(hmc7044, i, !!(config->clkin & (1 << i)));
        ADI_HMC7044_CHECK_ERR_OK(err);
    }
    // OSCIN
    err = adi_hmc7044_private_pll_enable_input_reference_set(hmc7044, 4, 1);
    ADI_HMC7044_CHECK_ERR_OK(err);

    for (i = 0; i < ADI_HMC7044_CLKIN_COUNT; i++) {
        switch (config->ref_priority[i]) {
            case ADI_HMC7044_CLKIN0:
                ref_priority[i] = 0;
                break;
            case ADI_HMC7044_CLKIN1:
                ref_priority[i] = 1;
                break;
            case ADI_HMC7044_CLKIN2:
                ref_priority[i] = 2;
                break;
            case ADI_HMC7044_CLKIN3:
                ref_priority[i] = 3;
                break;
            default:
                return API_CMS_ERROR_ERROR;
        }
    }

    err = adi_hmc7044_private_pll_input_reference_priority_set(hmc7044, ref_priority, ADI_HMC7044_CLKIN_COUNT);
    ADI_HMC7044_CHECK_ERR_OK(err);

    adi_hmc7044_private_math_rational_gcd(config->ref_clk_hz, 1, config->vcxo_hz, 1, &pfd1_clk_hz.freq_hz, &pfd1_clk_hz.div);
    while (pfd1_clk_hz.freq_hz / (pfd1_clk_hz.div * M1) > 50e6) {
        M1++;
    }
    pfd1_clk_hz.div *= M1;
    if (config->vcxo_hz / 65535 > (pfd1_clk_hz.freq_hz / pfd1_clk_hz.div)) {
        return API_CMS_ERROR_ERROR;
    }
    ref_div = (config->ref_clk_hz * pfd1_clk_hz.div) / pfd1_clk_hz.freq_hz;
    while (flcm_clk_hz.freq_hz / (flcm_clk_hz.div * M2) > 123.00e6 || ref_div % M2 != 0) {
        M2++;
    }
    clkin_prescaler = M2;
    flcm_clk_hz.div *= M2;
    vcxo_prescaler = ((config->vcxo_hz * pfd1_clk_hz.div) / pfd1_clk_hz.freq_hz) / (ref_div / clkin_prescaler);

    for (i = 0; i < ADI_HMC7044_CLKIN_COUNT; i++) {
        err = adi_hmc7044_private_pll_input_reference_prescaler_config_set(hmc7044, i, clkin_prescaler);
        ADI_HMC7044_CHECK_ERR_OK(err);
    }
    err = adi_hmc7044_private_pll_input_reference_oscin_prescaler_config_set(hmc7044, vcxo_prescaler);
    ADI_HMC7044_CHECK_ERR_OK(err);

    /* calculate pll2ref*/
    pll2ref_clk_hz = config->vcxo_hz <= HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MAX
        ? 2 * config->vcxo_hz
        : config->vcxo_hz;

    /*calculate fvco*/
    pfd2_gcd_hz.freq_hz = pll2ref_clk_hz;
    pfd2_gcd_hz.div = 1;
    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (config->output_freq[i].div != 0 && config->output_freq[i].freq_hz != 0 && (config->clkout & (1 << i))) {
            adi_hmc7044_private_math_rational_gcd(pfd2_gcd_hz.freq_hz, pfd2_gcd_hz.div, config->output_freq[i].freq_hz, config->output_freq[i].div, &pfd2_gcd_hz.freq_hz, &pfd2_gcd_hz.div);
        }
    }

    pfd2_lcm_hz.freq_hz = pfd2_gcd_hz.freq_hz;
    pfd2_lcm_hz.div = pfd2_gcd_hz.div;
    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (config->output_freq[i].div != 0 && config->output_freq[i].freq_hz != 0 && (config->clkout & (1 << i))) {
            adi_hmc7044_private_math_rational_lcm(pfd2_lcm_hz.freq_hz, pfd2_lcm_hz.div, config->output_freq[i].freq_hz, config->output_freq[i].div, &pfd2_lcm_hz.freq_hz, &pfd2_lcm_hz.div);
        }
    }
 
    /* Determine PLL2 fvco. Try for 3G VCO first, then 2G */
    for (int w = 0; w < ADI_HMC7044_INT_VCO_COUNT; w++) {
        for (ref_div = 1; ref_div < HMC7044_PLL2_R_DIV_MAX; ref_div++) {
            fdist.freq_hz = ref_div * pfd2_lcm_hz.freq_hz;
            fdist.div = pfd2_lcm_hz.div;
            fdist_dec = fdist.freq_hz / fdist.div;
            if (fdist_dec > vco_window_low_limit[w] && fdist_dec < vco_window_hi_limit[w]) {
                for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
                    if (config->output_freq[i].div != 0 && config->output_freq[i].freq_hz != 0 && (config->clkout & (1 << i))) {
                        output_chan_divider = (fdist.freq_hz * config->output_freq[i].div) / (fdist.div * config->output_freq[i].freq_hz);
                        if (output_chan_divider % 2 != 0) {
                            if (output_chan_divider != 1 && output_chan_divider != 3 && output_chan_divider != 5) {
                                break;
                            }
                        }
                    }
                }
                
                if (i == ADI_HMC7044_CLKOUT_COUNT) {
                    fvco_clk_hz.freq_hz = fdist.freq_hz;
                    fvco_clk_hz.div = fdist.div;
                    is_fvco_clk_valid = true;
                    break;
                }

            } else if (fdist_dec >= vco_window_hi_limit[w]) {
                break;
            }
        }

        if (is_fvco_clk_valid) {
            break;
        }
    }

    if (!is_fvco_clk_valid) {
        return API_CMS_ERROR_VCO_OUT_OF_RANGE;
    }

    fvco_clk_hz_dec = fvco_clk_hz.freq_hz / fvco_clk_hz.div;

    err = adi_hmc7044_device_int_vco_sel_get(hmc7044, &vco_sel);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (vco_sel != ADI_HMC7044_HMC7044_INT_VCO_SEL_DISABLED) { // Using internal VCO
        if (fvco_clk_hz_dec >= ADI_HMC7044_2GHZ_VCO_HZ_MIN && fvco_clk_hz_dec <= ADI_HMC7044_3GHZ_VCO_HZ_MAX) {
            if (fvco_clk_hz_dec >= ADI_HMC7044_3GHZ_VCO_HZ_MIN && vco_sel != ADI_HMC7044_HMC7044_INT_VCO_SEL_3GHZ) { // Use 3GHz VCO
                err = adi_hmc7044_device_int_vco_sel_set(hmc7044, ADI_HMC7044_HMC7044_INT_VCO_SEL_3GHZ);
                ADI_HMC7044_CHECK_ERR_OK(err);
            } else if (vco_sel != ADI_HMC7044_HMC7044_INT_VCO_SEL_2GHZ) { // Use 2GHz VCO
                err = adi_hmc7044_device_int_vco_sel_set(hmc7044, ADI_HMC7044_HMC7044_INT_VCO_SEL_2GHZ);
                ADI_HMC7044_CHECK_ERR_OK(err);
            }
        } else {
            return API_CMS_ERROR_VCO_OUT_OF_RANGE;
        }
    }

    /*calculate fpfd2*/
    adi_hmc7044_private_math_rational_gcd(fvco_clk_hz.freq_hz, fvco_clk_hz.div, pll2ref_clk_hz, 1, &pfd2_clk_hz.freq_hz, &pfd2_clk_hz.div);
    pfd2_clk_hz_dec = pfd2_clk_hz.freq_hz / pfd2_clk_hz.div;
    if (pfd2_clk_hz_dec < HMC7044_PD2_CLK_FREQ_HZ_MIN || pfd2_clk_hz_dec > HMC7044_PD2_CLK_FREQ_HZ_MAX) {
        return API_CMS_ERROR_ERROR;
    }

    /*calculate R1, N1, R2, N2*/
    R1 = (flcm_clk_hz.freq_hz * pfd1_clk_hz.div) / (flcm_clk_hz.div * pfd1_clk_hz.freq_hz);
    N1 = (config->vcxo_hz * pfd1_clk_hz.div) / pfd1_clk_hz.freq_hz;
    R2 = (pll2ref_clk_hz * pfd2_clk_hz.div) / pfd2_clk_hz.freq_hz;
    N2 = (fvco_clk_hz.freq_hz * pfd2_clk_hz.div) / (fvco_clk_hz.div * pfd2_clk_hz.freq_hz);

    err = adi_hmc7044_pll_enable_get(hmc7044, &enabled_plls);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = adi_hmc7044_private_pll_pll1_config_set(hmc7044, R1, N1);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_sw_delay_us(hmc7044, 10000);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (enabled_plls & ADI_HMC7044_PLL_SEL_PLL1) {
        err = adi_hmc7044_pll_locked_get(hmc7044, &pll_lock_st);
        ADI_HMC7044_CHECK_ERR_OK(err);

        if (!(pll_lock_st & ADI_HMC7044_PLL_LOCKED_PLL1)) {
            return API_CMS_ERROR_PLL_NOT_LOCKED;
        }
    }

    err = adi_hmc7044_private_pll_pll2_config_set(hmc7044, config->vcxo_hz > HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MAX, R2, N2);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_sw_delay_us(hmc7044, 100000);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = adi_hmc7044_device_fsm_div_restart_run(hmc7044);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_sw_delay_us(hmc7044, 100000);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (enabled_plls & ADI_HMC7044_PLL_SEL_PLL2) {
        err = adi_hmc7044_pll_locked_get(hmc7044, &pll_lock_st);
        ADI_HMC7044_CHECK_ERR_OK(err);

        if (!(pll_lock_st & ADI_HMC7044_PLL_LOCKED_PLL2)) {
            return API_CMS_ERROR_PLL_NOT_LOCKED;
        }
    }

    err = hmc7044_sw_delay_us(hmc7044, 100000);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = adi_hmc7044_private_output_performance_set(hmc7044, 1);
    ADI_HMC7044_CHECK_ERR_OK(err);

    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (config->clkout & (1 << i)) {
            err = adi_hmc7044_clkout_channel_driver_config_set(hmc7044, i, &config->clkout_driver_config);
            ADI_HMC7044_CHECK_ERR_OK(err);

            config->clkout_config.ch_div = (fvco_clk_hz.freq_hz * config->output_freq[i].div) / (fvco_clk_hz.div * config->output_freq[i].freq_hz);
            err = adi_hmc7044_clkout_channel_config_set(hmc7044, i, &config->clkout_config);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_int_vco_sel_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_int_vco_sel_e vco_sel)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(vco_sel, ADI_HMC7044_HMC7044_INT_VCO_SEL_DISABLED, ADI_HMC7044_HMC7044_INT_VCO_SEL_2GHZ);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_ENABLE_CTRL_REG, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val &= ~(0x18);

    if (vco_sel != ADI_HMC7044_HMC7044_INT_VCO_SEL_DISABLED)  {
        reg_val |= (vco_sel << 3);
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_ENABLE_CTRL_REG, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_int_vco_sel_get(adi_hmc7044_device_t *hmc7044, adi_hmc7044_int_vco_sel_e *vco_sel)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_ENABLE_CTRL_REG, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    *vco_sel = ((reg_val & 0x18) >> 3);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_ext_vco_div_enable_set(adi_hmc7044_device_t *hmc7044, uint8_t enable)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_EXT_VCO_CTRL, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val &= 0xFD; // ~0x02
    if (enable) {
        reg_val |= 0x02;
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_EXT_VCO_CTRL, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_clk_dist_priority_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clk_dist_priority_e priority)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(priority, ADI_HMC7044_CLK_DIST_PRIORITY_POWER, ADI_HMC7044_CLK_DIST_PRIORITY_NOISE);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val &= 0xBF; // ~0x40
    if (priority) {
        reg_val |= 0x40;
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_hmc7044_device_fsm_div_restart_run(adi_hmc7044_device_t *hmc7044)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (reg_val & 0x02) {
        reg_val &= 0xFD; // ~0x02
        err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
        ADI_HMC7044_CHECK_ERR_OK(err);
    }

    hmc7044_sw_delay_us(hmc7044, 1000);
    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, (reg_val | 0x02));
    ADI_HMC7044_CHECK_ERR_OK(err);

    hmc7044_sw_delay_us(hmc7044, 1000);
    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_reseed_run(adi_hmc7044_device_t *hmc7044)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (reg_val & 0x80) {
        reg_val &= 0x7F; // ~0x80
        err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
        ADI_HMC7044_CHECK_ERR_OK(err);
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, (reg_val | 0x80));
    ADI_HMC7044_CHECK_ERR_OK(err);

    hmc7044_sw_delay_us(hmc7044, 1000);
    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_gpi_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpi_e gpi, adi_hmc7044_gpi_config_t *config)
{
    uint8_t i;
    int32_t err;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_PTR_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(gpi, ADI_HMC7044_DEVICE_GPI1, ADI_HMC7044_DEVICE_GPI1 | ADI_HMC7044_DEVICE_GPI2 | ADI_HMC7044_DEVICE_GPI3 | ADI_HMC7044_DEVICE_GPI4);

    for (i = 0; i < ADI_HMC7044_GPI_COUNT; i++) {
        if ((1 << i) & gpi) {
            err = adi_hmc7044_private_device_gpi_config_set(hmc7044, i, (uint8_t)config->mode, config->enabled);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_gpi_mode_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpi_e gpi, adi_hmc7044_device_gpi_mode_e mode)
{
    uint8_t i;
    int32_t err;
    uint8_t rb;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(gpi, ADI_HMC7044_DEVICE_GPI1, ADI_HMC7044_DEVICE_GPI1 | ADI_HMC7044_DEVICE_GPI2 | ADI_HMC7044_DEVICE_GPI3 | ADI_HMC7044_DEVICE_GPI4);

    for (i = 0; i < ADI_HMC7044_GPI_COUNT; i++) {
        if ((1 << i) & gpi) {
            err = adi_hmc7044_private_device_spi_register_get(hmc7044, (HMC7044_GPI_CTRL_1_REG + i), &rb);
            ADI_HMC7044_CHECK_ERR_OK(err);
            err = adi_hmc7044_private_device_spi_register_set(hmc7044, (HMC7044_GPI_CTRL_1_REG + i), ((mode & 0xF) << 1) | (rb & 0xE1));
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_gpi_enable_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpi_e gpi, uint8_t enable)
{
    uint8_t i;
    int32_t err;
    uint8_t rb;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(gpi, ADI_HMC7044_DEVICE_GPI1, ADI_HMC7044_DEVICE_GPI1 | ADI_HMC7044_DEVICE_GPI2 | ADI_HMC7044_DEVICE_GPI3 | ADI_HMC7044_DEVICE_GPI4);

    for (i = 0; i < ADI_HMC7044_GPI_COUNT; i++) {
        if ((1 << i) & gpi) {
            err = adi_hmc7044_private_device_spi_register_get(hmc7044, (HMC7044_GPI_CTRL_1_REG + i), &rb);
            ADI_HMC7044_CHECK_ERR_OK(err);
            err = adi_hmc7044_private_device_spi_register_set(hmc7044, (HMC7044_GPI_CTRL_1_REG + i), (!!enable) | (rb & 0xFE));
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_gpo_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_device_gpo_e gpo, adi_hmc7044_gpo_config_t *config)
{
    uint8_t i;
    int32_t err;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_PTR_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(gpo, ADI_HMC7044_DEVICE_GPO1, ADI_HMC7044_DEVICE_GPO1 | ADI_HMC7044_DEVICE_GPO2 | ADI_HMC7044_DEVICE_GPO3 | ADI_HMC7044_DEVICE_GPO4);

    for (i = 0; i < ADI_HMC7044_GPO_COUNT; i++) {
        if ((1 << i) & gpo) {
            err = adi_hmc7044_private_device_gpo_config_set(hmc7044, i, (uint8_t)config->mode, (uint8_t)config->output, config->enabled);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }
    return API_CMS_ERROR_OK;
}

static int32_t device_init(adi_hmc7044_device_t *hmc7044)
{
    int32_t err;
    err = hmc7044_spi_reg_tbl_set(
        hmc7044,
        &ADI_RECOMMENDED_INIT_TBL[0],
        ADI_UTILS_ARRAY_SIZE(ADI_RECOMMENDED_INIT_TBL));
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}