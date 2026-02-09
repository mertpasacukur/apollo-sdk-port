/*!
 * \brief     HMC7044 CLKOUT Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_hmc7044_clkout.h"
#include "adi_hmc7044_private_output.h"
#include "hmc7044_reg.h"
#include "hmc7044_hal.h"

#define GET_OUPUT_CHANNEL_OFFSET(index) ((index) * HMC7044_CLK_OP_CTRL_OFFSET)
#define VALIDATE_DRIVER_CONFIG(config) \
    ADI_HMC7044_VALIDATE_RANGE(config->mode, SIGNAL_CMOS, SIGNAL_LVPECL); \
    ADI_HMC7044_VALIDATE_RANGE(config->impedance, ADI_CMS_NO_INTERNAL_RESISTOR, ADI_CMS_INTERNAL_RESISTOR_50_OHM);

static int32_t clkout_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_config_t *config);
static int32_t sync_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_sync_config_t *config);
static int32_t multi_slip_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_multi_slip_config_t *config);
static int32_t driver_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_driver_config_t *config);

int32_t adi_hmc7044_clkout_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_config_t *config)
{
    int32_t err, i;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(clkout, ADI_HMC7044_CLKOUT0, ADI_HMC7044_OUTPUT_ALL);
    ADI_HMC7044_VALIDATE_RANGE(config->ch_div, 0, HMC7044_CH_DIV_MAX);
    ADI_HMC7044_VALIDATE_RANGE(config->mux_sel, ADI_HMC7044_CLKOUT_MUX_SEL_CH_DIV, ADI_HMC7044_CLKOUT_MUX_SEL_IP_VC0);

    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (clkout & (1 << i)) {
            err = clkout_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(i), config);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_clkout_channel_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_config_t *config)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(channel_index, 0, ADI_HMC7044_CLKOUT_COUNT - 1);
    ADI_HMC7044_VALIDATE_RANGE(config->ch_div, 0, HMC7044_CH_DIV_MAX);
    ADI_HMC7044_VALIDATE_RANGE(config->mux_sel, ADI_HMC7044_CLKOUT_MUX_SEL_CH_DIV, ADI_HMC7044_CLKOUT_MUX_SEL_IP_VC0);

    return clkout_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(channel_index), config);
}

int32_t adi_hmc7044_clkout_sync_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_sync_config_t *config)
{
    int32_t err, i;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(clkout, ADI_HMC7044_CLKOUT0, ADI_HMC7044_OUTPUT_ALL);
    ADI_HMC7044_CLKOUT_VALIDATE_STARTUP_MODE(config->startup_mode);

    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (clkout & (1 << i)) {
            err = sync_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(i), config);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_clkout_channel_sync_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_sync_config_t *config)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(channel_index, 0, ADI_HMC7044_CLKOUT_COUNT - 1);
    ADI_HMC7044_CLKOUT_VALIDATE_STARTUP_MODE(config->startup_mode);

    return sync_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(channel_index), config);
}

int32_t adi_hmc7044_clkout_sysref_pulse_gen_mode_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_sysref_pulse_gen_mode_e mode)
{
    int32_t err;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_VALIDATE_RANGE(mode, ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_LEVEL, ADI_HMC7044_CLKOUT_SYSREF_PULSE_GEN_MODE_CONTINUOUS);

    err = hmc7044_spi_reg_set(hmc7044, HMC7044_SYSREF_SYNC_CTRL_REG, mode);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_clkout_multi_slip_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_multi_slip_config_t *config)
{
    int32_t err, i;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(clkout, ADI_HMC7044_CLKOUT0, ADI_HMC7044_OUTPUT_ALL);
    ADI_HMC7044_VALIDATE_RANGE(config->slip_delay, 0, HMC7044_SLIP_DELAY_MAX);

    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (clkout & (1 << i)) {
            err = multi_slip_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(i), config);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_clkout_channel_multi_slip_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_multi_slip_config_t *config)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(channel_index, 0, ADI_HMC7044_CLKOUT_COUNT - 1);
    ADI_HMC7044_VALIDATE_RANGE(config->slip_delay, 0, HMC7044_SLIP_DELAY_MAX);

    return multi_slip_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(channel_index), config);
}

int32_t adi_hmc7044_clkout_driver_config_set(adi_hmc7044_device_t *hmc7044, adi_hmc7044_clkout_e clkout, adi_hmc7044_clkout_driver_config_t *config)
{
    int32_t err, i;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    VALIDATE_DRIVER_CONFIG(config);

    for (i = 0; i < ADI_HMC7044_CLKOUT_COUNT; i++) {
        if (clkout & (1 << i)) {
            err = driver_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(i), config);
            ADI_HMC7044_CHECK_ERR_OK(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_clkout_channel_driver_config_set(adi_hmc7044_device_t *hmc7044, uint8_t channel_index, adi_hmc7044_clkout_driver_config_t *config)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(config);
    ADI_HMC7044_VALIDATE_RANGE(channel_index, 0, ADI_HMC7044_CLKOUT_COUNT - 1);
    VALIDATE_DRIVER_CONFIG(config);

    return driver_config_set(hmc7044, GET_OUPUT_CHANNEL_OFFSET(channel_index), config);
}

static int32_t clkout_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_config_t *config)
{
    int32_t err;
    uint16_t reg_addr;
    uint8_t reg_val;

    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + reg_offset);
    err = hmc7044_spi_reg_get(hmc7044, reg_addr, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_spi_reg_set(hmc7044,
                              reg_addr,
                              (reg_val & 0x7E)
                              | (config->high_perform_en ? HMC7044_CLK_OP_HIGH_PERFORM_EN : 0)
                              | (!!config->enable));
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_spi_reg_set(hmc7044,
                              (HMC7044_CLK_OP_CTRL_1_REG + reg_offset),
                              (uint8_t)(config->ch_div & 0xFF));
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_spi_reg_set(hmc7044,
                              (HMC7044_CLK_OP_CTRL_2_REG + reg_offset),
                              (uint8_t)((config->ch_div >> 8) & 0xFF));
    ADI_HMC7044_CHECK_ERR_OK(err);

    err = hmc7044_spi_reg_set(hmc7044,
                              (HMC7044_CLK_OP_CTRL_7_REG + reg_offset),
                              HMC7044_CLK_OP_MUX_SEL(config->mux_sel) & 0x03);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

static int32_t sync_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_sync_config_t *config)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;

    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + reg_offset);

    err = hmc7044_spi_reg_get(hmc7044, reg_addr, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val &= ~(HMC7044_CLK_OP_SYNC_EN |
                 HMC7044_CLK_OP_SLIP_EN |
                 HMC7044_CLK_OP_STARTUP_MODE(ADI_UTILS_ALL));
    reg_val |= HMC7044_CLK_OP_STARTUP_MODE(config->startup_mode);
    reg_val |= config->slip_mode_en ? HMC7044_CLK_OP_SLIP_EN : 0;
    reg_val |= config->sync_mode_en ? HMC7044_CLK_OP_SYNC_EN : 0;

    err = hmc7044_spi_reg_set(hmc7044, reg_addr, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

static int32_t multi_slip_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_multi_slip_config_t *config)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;

    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + reg_offset);
    err = hmc7044_spi_reg_get(hmc7044, reg_addr, &reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    reg_val &= ~(HMC7044_CLK_OP_MULTI_SLIP_EN);
    reg_val |= config->multi_slip_en ? HMC7044_CLK_OP_MULTI_SLIP_EN : 0;
    err = hmc7044_spi_reg_set(hmc7044, reg_addr, reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    if (config->multi_slip_en) {
        reg_val = (uint8_t)(config->slip_delay & 0xFF);
        reg_addr = (HMC7044_CLK_OP_CTRL_5_REG + reg_offset);
        err = hmc7044_spi_reg_set(hmc7044, reg_addr, reg_val);
        ADI_HMC7044_CHECK_ERR_OK(err);

        reg_val = (uint8_t)((config->slip_delay >> 8) &0xF);
        reg_addr = (HMC7044_CLK_OP_CTRL_6_REG + reg_offset);
        err = hmc7044_spi_reg_set(hmc7044, reg_addr, reg_val);
        ADI_HMC7044_CHECK_ERR_OK(err);
    }

    return API_CMS_ERROR_OK;
}

static int32_t driver_config_set(adi_hmc7044_device_t *hmc7044, uint32_t reg_offset, adi_hmc7044_clkout_driver_config_t *config)
{
    int32_t  err;
    uint8_t  reg_val = 0x0;

    if (config->impedance == ADI_CMS_INTERNAL_RESISTOR_100_OHM) {
        reg_val |= HMC7044_CLK_OP_DRIVER_IMPEDANCE(0x1);
    } else if (config->impedance == ADI_CMS_INTERNAL_RESISTOR_50_OHM) {
        reg_val |= HMC7044_CLK_OP_DRIVER_IMPEDANCE(0x3);
    }

    reg_val |= config->dynamic_driver_en ? HMC7044_CLK_OP_DYNAMIC_DR_EN   : 0x0;
    reg_val |= config->force_mute_en     ? HMC7044_CLK_OP_FORCE_MUTE(0x2) : 0x0;

    switch (config->mode) {
        case SIGNAL_CML:
            reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x0);
            break;
        case SIGNAL_LVPECL:
            reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x1);
            break;
        case SIGNAL_LVDS:
            reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x2);
            break;
        case SIGNAL_CMOS:
        default:
            reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x3);
            break;
    }

    err = hmc7044_spi_reg_set(hmc7044, (HMC7044_CLK_OP_CTRL_8_REG + reg_offset), reg_val);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}
