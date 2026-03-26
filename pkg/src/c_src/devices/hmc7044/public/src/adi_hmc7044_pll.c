/*!
 * \brief     HMC7044 Device Level Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_hmc7044_pll.h"
#include "adi_hmc7044_core.h"
#include "hmc7044_reg.h"
#include "hmc7044_hal.h"

static int32_t input_config_set(adi_hmc7044_device_t *device, uint8_t clk_in, uint8_t config, uint8_t enable);

int32_t adi_hmc7044_pll_clkin_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkin_e clk_in, adi_hmc7044_pll_ipbuffer_settings_e config, uint8_t enable)
{
    int32_t err, i;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(clk_in, ADI_HMC7044_CLKIN_NONE, ADI_HMC7044_CLKIN_ALL);

    for (i = 0; i < ADI_HMC7044_CLKIN_COUNT; i++) {
        if (clk_in & (1 << i)) {
            err = input_config_set(hmc7044, i, (uint8_t)config, enable);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll_los_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_pll_los_config_t *config)
{
    int32_t err;
    uint8_t reg_val = 0x0;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);

    if (config->timer_cycles > HMC7044_PLL1_LOS_VALID_TIMER(ADI_UTILS_ALL)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_PLL1_LOS_TIMER_CTRL_REG, config->timer_cycles);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val |= config->input_prescaler_en ? 0x0 : HMC7044_LOS_INPUT_PRESCALER_BYPASS;
    reg_val |= config->vcxo_prescaler_en  ? HMC7044_LOS_VCXO_PRESCALER_EN : 0x0;
    err = hmc7044_spi_reg_set(hmc7044, HMC7044_PLL1_LOS_MODE_REG, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

static int32_t input_config_set(adi_hmc7044_device_t *device, uint8_t clk_in, uint8_t config, uint8_t enable)
{
    int32_t err;
    uint16_t reg_addr;

    if (((config & ADI_HMC7044_PLL_IPBUFFER_HIGH_Z_MODE)     && (config & ADI_HMC7044_PLL_IPBUFFER_AC_COUPLED_MODE)) ||
        ((config & ADI_HMC7044_PLL_IPBUFFER_HIGH_Z_MODE)     && (config & ADI_HMC7044_PLL_IPBUFFER_LVPECL_MODE)) ||
        ((config & ADI_HMC7044_PLL_IPBUFFER_AC_COUPLED_MODE) && (config & ADI_HMC7044_PLL_IPBUFFER_LVPECL_MODE)) ||
        ((config > ADI_HMC7044_PLL_IPBUFFER_CONFIG_ALL))) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

#ifndef APPLY_VERSAL_HARDWARE_BASED_CHANGE
    // TODO API: HMC7044_CLK_IP_BUFF_BASE_RE(0xA) kontrolu kaldirildi.
    reg_addr = ((clk_in * HMC7044_CLK_IP_BUFF_OFFSET) + HMC7044_CLK_IP_BUFF_BASE_REG);
    err = hmc7044_spi_reg_set(device, reg_addr, ((config << 1) | (!!enable)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
#endif

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll_enable_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_pll_sel_e pll, uint8_t enable)
{
    int32_t err;
    uint8_t rb;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(pll, ADI_HMC7044_PLL_SEL_PLL1, ADI_HMC7044_PLL_SEL_PLL1 | ADI_HMC7044_PLL_SEL_PLL2);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_ENABLE_CTRL_REG, &rb);
    ADI_HMC7044_CHECK_ERR_OK(err);

    rb &= ~pll;

    if (enable) {
        rb |= pll;
    }

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_GLOBAL_ENABLE_CTRL_REG, rb);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll_enable_get(adi_hmc7044_device_t *hmc7044, adi_hmc7044_pll_sel_e *pll)
{
    int32_t err;
    uint8_t rb;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, HMC7044_GLOBAL_ENABLE_CTRL_REG, &rb);
    ADI_HMC7044_CHECK_ERR_OK(err);

    *pll = rb & 0x03;

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll_locked_get(adi_hmc7044_device_t *hmc7044, adi_hmc70944_pll_locked_e *locked)
{
    int32_t err;
    uint8_t rb;

    ADI_HMC7044_PTR_CHECK(hmc7044);

    err = hmc7044_spi_reg_get(hmc7044, 0x7D, &rb);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (rb & 0x08) {
        *locked = ADI_HMC7044_PLL_LOCKED_PLL1 | ADI_HMC7044_PLL_LOCKED_PLL2;
    } else if (rb & 0x01) {
        *locked = ADI_HMC7044_PLL_LOCKED_PLL2;
    } else {
        err = hmc7044_spi_reg_get(hmc7044, 0x7C, &rb);
        ADI_HMC7044_CHECK_ERR_OK(err);

        *locked = (rb & 0x20) ? ADI_HMC7044_PLL_LOCKED_PLL1
                              : ADI_HMC7044_PLL_LOCKED_NONE;
    }

    return API_CMS_ERROR_OK;
}
