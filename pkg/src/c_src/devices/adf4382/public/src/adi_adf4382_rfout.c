/*!
 * \brief     ADF4382 RFOUT Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include "adi_adf4382_rfout.h"
#include "adi_adf4382_regmap_inline_bfs.h"

//static uint64_t __gcd(uint64_t, uint64_t);
static uint64_t __mod_div(uint64_t, uint64_t, uint64_t*);
static uint64_t __mod_div_ceil(uint64_t, uint64_t);
static uint64_t __mod_div_round(uint64_t, uint64_t);

int32_t adi_adf4382_rfout_output_set(adi_adf4382_device_t *adf4382, adi_adf4382_rfout_output_config_t *config)
{
    int32_t err;
    uint8_t reg0020 = 0;
    uint8_t reg0011 = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(config);
    ADI_CMS_RANGE_CHECK(config->ref_freq_hz, ADI_ADF4382_REF_MIN, ADI_ADF4382_REF_MAX);
    ADI_CMS_RANGE_CHECK(config->rfout_freq_hz, ADI_ADF4382_RFOUT_MIN, ADI_ADF4382_RFOUT_MAX);
    ADI_CMS_RANGE_CHECK(config->clk1_opwr, ADI_ADF4382_CLK_OPWR_MIN, ADI_ADF4382_CLK_OPWR_MAX);
    ADI_CMS_RANGE_CHECK(config->clk2_opwr, ADI_ADF4382_CLK_OPWR_MIN, ADI_ADF4382_CLK_OPWR_MAX);
    ADI_CMS_RANGE_CHECK(config->ld_count, ADI_ADF4382_LD_COUNT_MIN, ADI_ADF4382_LD_COUNT_MAX);
    ADI_CMS_RANGE_CHECK(config->r_div, ADI_ADF4382_R_DIV_MIN, ADI_ADF4382_R_DIV_MAX);

    uint8_t clkout_div;
    uint64_t tmp;
    uint64_t vco = 0;


    for (clkout_div = 0; clkout_div < 4; clkout_div++)
    {
        tmp = (1 << clkout_div) * config->rfout_freq_hz;
        if (tmp < ADI_ADF4382_VCO_FREQ_MIN || tmp > ADI_ADF4382_VCO_FREQ_MAX)
        {
            continue;
        }

        vco = tmp;
        break;
    }

    ADI_CMS_CHECK((vco == 0), API_CMS_ERROR_ERROR);

    uint64_t pfd_freq_hz_r;
    uint64_t pfd_freq_hz = __mod_div(config->ref_freq_hz, config->r_div, &pfd_freq_hz_r);
    if (config->en_doubler)
    {
        pfd_freq_hz *= 2;
        pfd_freq_hz_r *= 2;

        if (pfd_freq_hz_r > config->r_div)
        {
            pfd_freq_hz += pfd_freq_hz_r / config->r_div;
            pfd_freq_hz_r -= config->r_div;
        }
    }

    ADI_CMS_CHECK((pfd_freq_hz_r > 0), API_CMS_ERROR_ERROR);

    uint64_t tmp_r;
    uint16_t n_int = (uint16_t)__mod_div(config->rfout_freq_hz, pfd_freq_hz, &tmp_r);

    uint64_t residue = tmp_r * ADI_ADF4382_MOD1WORD;
    uint32_t frac1_word = (uint32_t)(residue / pfd_freq_hz);
    uint32_t frac2_word = 0;
    uint32_t mod2_word = 0;

    residue %= pfd_freq_hz;

    if (residue > 0)
    {
        mod2_word = 1;
        // TODO: Use the bitfield wrapper function once it's fixed.
        uint8_t en_phase_resync;
        err = adi_adf4382_bf___REG001E___EN_PHASE_RESYNC_get(adf4382,&en_phase_resync);
        ADI_CMS_ERROR_RETURN(err);

        uint32_t mod2_max = en_phase_resync
            ? ADI_ADF4382_PHASE_RESYNC_MOD2WORD_MAX
            : ADI_ADF4382_MOD2WORD_MAX;
        //uint32_t channel_spacing = 1;
        //uint64_t mod2_tmp;

        /** TODO: Figure out if this is needed.
        do
        {
            mod2_tmp = pfd_freq_hz / __gcd(channel_spacing * ADI_ADF4382_MOD1WORD, pfd_freq_hz);
            if (mod2_tmp > mod2_max)
            {
                channel_spacing *= 5;
            }
            else
            {
                mod2_word = mod2_tmp;
                break;
            }
        } while (channel_spacing < ADI_ADF4382_CHANNEL_SPACING_MAX);
        */

        if (!en_phase_resync)
        {
            mod2_word *= (mod2_max / mod2_word);
        }

        frac2_word = __mod_div_round(residue * mod2_word, pfd_freq_hz);
    }

    uint8_t ldwin_pw = 0;
    uint8_t int_mode;
    uint8_t en_bleed;

    if (frac1_word || frac2_word) { // Fractional Mode
        ADI_CMS_RANGE_CHECK(pfd_freq_hz, ADI_ADF4382_RFOUT_FPFD_MIN_HZ, ADI_ADF4382_RFOUT_FPFD_MAX_HZ_FRAC);

        int_mode = 0;
        en_bleed = 1;

        if (pfd_freq_hz <= 40e6) {
            ldwin_pw = 7;
        } else if (pfd_freq_hz <= 50e6) {
            ldwin_pw = 6;
        } else if (pfd_freq_hz <= 100e6) {
            ldwin_pw = 5;
        } else if (pfd_freq_hz <= 200e6) {
            ldwin_pw = 4;
        } else if (pfd_freq_hz <= 250e6) {
            if (config->rfout_freq_hz >= 5e9 && config->rfout_freq_hz < 64e8) {
                ldwin_pw = 3;
            } else {
                ldwin_pw = 2;
            }
        }
    } else { // Integer Mode
        ADI_CMS_RANGE_CHECK(pfd_freq_hz, ADI_ADF4382_RFOUT_FPFD_MIN_HZ, ADI_ADF4382_RFOUT_FPFD_MAX_HZ_INT);

        int_mode = 1;
        en_bleed = 0;

        uint16_t bleed_i;
        uint8_t cp_i;
        err = adi_adf4382_bf___REG001D___BLEED_I_get(adf4382,&bleed_i);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4382_bf___REG001F___CP_I_get(adf4382,&cp_i);
        ADI_CMS_ERROR_RETURN(err);

        ldwin_pw = __mod_div_ceil(bleed_i, pfd_freq_hz * cp_i) < 85 ? 0 : 1;
    }

    uint8_t dclk_div1 = 2;
    uint8_t div1 = 8;
    if (pfd_freq_hz <= ADI_ADF4382_DCLK_DIV1_0_MAX)
    {
        dclk_div1 = 0;
        div1 = 1;
    }
    else if (pfd_freq_hz <= ADI_ADF4382_DCLK_DIV1_1_MAX)
    {
        dclk_div1 = 1;
        div1 = 2;
    }

    uint8_t adc_clk_div = ((__mod_div(pfd_freq_hz, div1 * 1600000, &tmp_r) * 10) - 5) / 10;
    if (tmp_r > 0) {
        adc_clk_div++;
    }

    /*** Start Register Writes ***/
    err = adi_adf4382_hal_reg_get(adf4382, 0x20, &reg0020);
    ADI_CMS_ERROR_RETURN(err);

    reg0020 &= 0x80; // Mask EN_AUTOCAL Setting
    reg0020 |= (config->en_doubler & 0x1) << 6;     // Set EN_RDBLR
    reg0020 |= (config->r_div & 0x3F);      // Set R_DIV

    err = adi_adf4382_hal_reg_set(adf4382, 0x20, reg0020);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0015___INT_MODE_set(adf4382, int_mode);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG001F___EN_BLEED_set(adf4382, en_bleed);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0028___VAR_MOD_EN_set(adf4382, frac2_word != 0 ? 1 : 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG001A___MOD2WORD_set(adf4382, mod2_word == 0 ? 1 : mod2_word & 0xFFFFFF); // 24bit
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0012___FRAC1WORD_set(adf4382, frac1_word & 0x1FFFFFF); // 25bit
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0017___FRAC2WORD_set(adf4382, frac2_word & 0xFFFFFF); // 24bit
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0024___DCLK_DIV1_set(adf4382, dclk_div1);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0031___DCLK_MODE_set(adf4382, 1);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0031___CAL_CT_SEL_set(adf4382, 1);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0038___CAL_VTUNE_TO_set(adf4382, 124);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG003a___CAL_VCO_TO_set(adf4382, 250);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0037___CAL_COUNT_TO_set(adf4382, 202);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG003e___ADC_CLK_DIV_set(adf4382, adc_clk_div);
    ADI_CMS_ERROR_RETURN(err);

    // Set LD COUNT
    err = adi_adf4382_bf___REG002c___LD_COUNT_set(adf4382, config->ld_count);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG002c___LDWIN_PW_set(adf4382, ldwin_pw);
    ADI_CMS_ERROR_RETURN(err);

    // Set clock1 output power
    err = adi_adf4382_bf___REG0029___CLK1_OPWR_set(adf4382, config->clk1_opwr);
    ADI_CMS_ERROR_RETURN(err);

    // Set clock2 output power
    err = adi_adf4382_bf___REG0029___CLK2_OPWR_set(adf4382, config->clk2_opwr);
    ADI_CMS_ERROR_RETURN(err);

    // Need to set N_INT last to trigger an auto-calibration
    err = adi_adf4382_hal_reg_get(adf4382, 0x11, &reg0011);
    ADI_CMS_ERROR_RETURN(err);

    reg0011 &= 0x10; // Mask INV_RFOUT Setting
    reg0011 |= (clkout_div & 0x7) << 5;     // Set RFOUT_DIV
    reg0011 |= (n_int >> 8) & 0xF;          // Set N_INT[11:8]

    err = adi_adf4382_hal_reg_set(adf4382, 0x11, reg0011);
    ADI_CMS_ERROR_RETURN(err);

    // Apply settings to double-buffered bit fields by writing to Reg 0x10
    err = adi_adf4382_hal_reg_set(adf4382, 0x10, (n_int & 0xFF));
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_rfout_clk_opwr_set(adi_adf4382_device_t *adf4382, uint16_t clk_sel, uint8_t clk_opwr) {

    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(clk_sel, 0, 1);
    ADI_CMS_RANGE_CHECK(clk_opwr, ADI_ADF4382_CLK_OPWR_MIN, ADI_ADF4382_CLK_OPWR_MAX);

    if  (clk_sel == ADI_ADF4382_CLK1_SEL) {
        err = adi_adf4382_bf___REG0029___CLK1_OPWR_set(adf4382, clk_opwr);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_adf4382_bf___REG0029___CLK2_OPWR_set(adf4382, clk_opwr);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}
/*
static uint64_t __gcd(uint64_t a, uint64_t b)
{
    uint64_t gcd;

    if (a == 0) {
        return b;
    }

    if (b == 0)
    {
        return a;
    }

    return a > b ? __gcd(a % b, b) : __gcd(a, b % a);
}
*/

static uint64_t __mod_div(uint64_t n, uint64_t d, uint64_t *r)
{
    if (d == 0) {
        *r = n;
        return 0;
    } else {
        *r = n % d;
        return n / d;
    }
}

static uint64_t __mod_div_ceil(uint64_t n, uint64_t d)
{
    uint64_t r;
    uint64_t value = __mod_div(n, d, &r);
    if (r > 0)
    {
        value++;
    }

    return value;
}

static uint64_t __mod_div_round(uint64_t n, uint64_t d)
{
    uint64_t r;
    uint64_t value = __mod_div(n, d, &r);

    return value + ((r * 2) / d);
}
