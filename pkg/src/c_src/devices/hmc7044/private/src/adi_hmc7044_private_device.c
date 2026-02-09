/*!
 * @brief     Device level API implementation
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __HMC7044_DEVICE_API__
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_utils.h"
#include "adi_hmc7044_private_pll.h"
#include "adi_hmc7044_private_output.h"
#include "adi_hmc7044_private_device.h"
#include "hmc7044_hal.h"
#include "hmc7044_reg.h"

/*============= D E F I N E S ==============*/

/*============= D A T A ====================*/
static uint8_t hmc7044_api_revision[3] = {0,2,2};
static adi_cms_reg_data_t ADI_RECOMMENDED_INIT_TBL[] = {
    {0x09F,0x4D},
    {0x0A0,0xDF},
    {0x0A5,0x06},
    {0x0A8,0x06},
    {0x0B0,0x04},
};

/*============= C O D E ====================*/
int32_t adi_hmc7044_private_device_hw_open(adi_hmc7044_private_device_t *device)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (device->hal_info.spi_write == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    if (device->hal_info.spi_read == ADI_INVALID_POINTER)
    {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    if (device->hal_info.delay_us == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_DELAYUS_PTR;
    }

    err = hmc7044_hw_open(device);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

#if 0 /*TODO: Finalise:Data sheet says its recommended but may be not required*/
    err = hmc7044_hw_reset(device);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
#endif

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_hw_close(adi_hmc7044_private_device_t *device)
{
    int32_t err;

#if 0 /*TODO: Finalise:Data sheet says its recommended but may be not required*/
    err = hmc7044_hw_reset(device);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
#endif

    err = hmc7044_hw_close(device);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_sleep_set(adi_hmc7044_device_t *device, uint8_t sleepmode)
{
    int32_t err;
    uint8_t reg_val;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    err = hmc7044_spi_reg_get(device, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    reg_val &= 0xFE;
    if (sleepmode)
    {
        reg_val |= 1;
    }

    err = hmc7044_spi_reg_set(device, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_init(adi_hmc7044_device_t *device)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    err = hmc7044_spi_reg_tbl_set(device, &ADI_RECOMMENDED_INIT_TBL[0],
        ADI_UTILS_ARRAY_SIZE(ADI_RECOMMENDED_INIT_TBL));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_reset(adi_hmc7044_device_t *device, uint8_t hw_reset)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (hw_reset > 1) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    if (hw_reset) {
        err = hmc7044_hw_reset(device);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    } else {
        err = hmc7044_spi_reg_set(device,
                HMC7044_GLOBAL_SW_RESET_CTRL_REG, HMC7044_SOFT_RESET);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
        err = hmc7044_sw_delay_us(device, HMC7044_SPI_RESET_PERIOD_US);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
        err = hmc7044_spi_reg_set(device, HMC7044_GLOBAL_SW_RESET_CTRL_REG, 0x0);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    err = hmc7044_sw_delay_us(device, HMC7044_SPI_RESET_PERIOD_US);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    err = hmc7044_spi_reg_tbl_set(device, &ADI_RECOMMENDED_INIT_TBL[0],
        ADI_UTILS_ARRAY_SIZE(ADI_RECOMMENDED_INIT_TBL));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_deinit(adi_hmc7044_device_t *device)
{
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_chip_id_get(adi_hmc7044_device_t *device, adi_cms_chip_id_t *chip_id)
{
    int err;
    uint8_t tmp_reg[3];

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (chip_id == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    err = hmc7044_spi_reg_get(device, HMC7044_CHIP_ID_0_REG, &tmp_reg[0]);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = hmc7044_spi_reg_get(device, HMC7044_CHIP_ID_1_REG, &tmp_reg[1]);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = hmc7044_spi_reg_get(device, HMC7044_CHIP_ID_2_REG, &tmp_reg[2]);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    /*TODO: Get details on this*/
    chip_id->dev_revision = tmp_reg[0];
    chip_id->prod_id = (((uint16_t) tmp_reg[2]) << 8);
    chip_id->prod_id |= (tmp_reg[1]);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_spi_register_set(adi_hmc7044_device_t *device, uint16_t addr, uint8_t val)
{
    return hmc7044_spi_reg_set(device, addr, val);
}

int32_t adi_hmc7044_private_device_spi_register_get(adi_hmc7044_device_t *device, uint16_t addr, uint8_t *val)
{
    return hmc7044_spi_reg_get(device, addr, val);
}

int32_t adi_hmc7044_private_device_api_revision_get(adi_hmc7044_device_t *device, uint8_t *rev_major, uint8_t *rev_minor, uint8_t *rev_rc)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    err = API_CMS_ERROR_OK;
    if (rev_major != ADI_INVALID_POINTER) {
        *rev_major = hmc7044_api_revision[0];
    } else {
        err = API_CMS_ERROR_INVALID_PARAM;
    }
    if (rev_minor != ADI_INVALID_POINTER) {
        *rev_minor = hmc7044_api_revision[1];
    } else {
        err = API_CMS_ERROR_INVALID_PARAM;
    }
    if(rev_rc != ADI_INVALID_POINTER) {
        *rev_rc = hmc7044_api_revision[2];
    } else {
        err = API_CMS_ERROR_INVALID_PARAM;
    }

    return err;
}

int32_t adi_hmc7044_private_device_trigger_restart(adi_hmc7044_device_t *device)
{
    uint8_t reg_val;
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    err = hmc7044_spi_reg_get(device, HMC7044_GLOBAL_SW_RESET_CTRL_REG, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val |= HMC7044_RESET_DIV_FSM;
    err = hmc7044_spi_reg_set(device, HMC7044_GLOBAL_SW_RESET_CTRL_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = hmc7044_sw_delay_us(device, HMC7044_DIV_RESET_PERIOD_US);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val &= (~HMC7044_RESET_DIV_FSM);
    err = hmc7044_spi_reg_set(device, HMC7044_GLOBAL_SW_RESET_CTRL_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_gpi_config_set(adi_hmc7044_device_t *device, uint8_t gpi_index, uint8_t gpi_config, uint8_t enable)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if ((gpi_index >= HMC7044_NOF_GPIO_MAX)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    gpi_config = (gpi_config << 1) | enable;
    err = adi_hmc7044_private_device_spi_register_set(device, (HMC7044_GPI_CTRL_1_REG + gpi_index), gpi_config);
    return err;
}

int32_t adi_hmc7044_private_device_gpo_config_set(adi_hmc7044_device_t *device, uint8_t gpo_index, uint8_t gpo_config, uint8_t mode, uint8_t enable)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if ((gpo_index >= HMC7044_NOF_GPIO_MAX)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    gpo_config = (gpo_config << 2) | (mode << 1) | enable;
    err = adi_hmc7044_private_device_spi_register_set(device, (HMC7044_GPO_CTRL_1_REG + gpo_index), gpo_config );
    return err;
}

int32_t adi_hmc7044_private_device_sdata_config_set(adi_hmc7044_device_t *device, uint8_t mode, uint8_t enable)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_SDATA_CTRL_REG, (mode << 1) | enable);
    return err;
}

int32_t adi_hmc7044_private_device_clk_config(adi_hmc7044_device_t *device, adi_hmc7044_clk_in_e ref_ch, adi_hmc7044_clk_config_t *config)
{
    uint8_t pll_lock_st =0x0;
    int32_t err;
    uint64_t flcm_clk_hz, pfd1_clk_hz, pfd2_clk_hz, clkin_prescaler, vcxo_prescaler, pll2ref_clk_hz, pfd2_lcm_hz, pfd2_gcd_hz, fvco_clk_hz = 0;
    uint64_t R1, N1, N2, R2, output_chan_divider;
    uint64_t i = 0, M1 = 1, M2 = 1;

    /*range check*/
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (ref_ch > HMC7044_CLK_IN_ALL) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if ((config->ref_clk_freq_hz > HMC7044_REF_CLK_FREQ_HZ_MAX || config->ref_clk_freq_hz < HMC7044_REF_CLK_FREQ_HZ_MIN)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    if (config->fvcxo_clk_freq_hz < HMC7044_VCXO_CLK_FREQ_HZ_MIN || config->fvcxo_clk_freq_hz > HMC7044_VCXO_CLK_FREQ_HZ_MAX) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    for (i = 0; i < HMC7044_NOF_OP_CH; i++) {
        if (config->output_freq_hz[i]) {
            uint16_t opchan = 1 << i;
            if ((config->output_ch & opchan) != opchan) {
                return API_CMS_ERROR_INVALID_PARAM;
            }
        }
    }

    /*enable clkin*/
    if (err = adi_hmc7044_private_pll_enable_input_reference_set(device, 0, (ref_ch & HMC7044_CLK_IN_0)), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_enable_input_reference_set(device, 1, (ref_ch & HMC7044_CLK_IN_1) >> 1), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_enable_input_reference_set(device, 2, (ref_ch & HMC7044_CLK_IN_2) >> 2), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_enable_input_reference_set(device, 3, (ref_ch & HMC7044_CLK_IN_3) >> 3), err != API_CMS_ERROR_OK)
        return err;
    /*enable oscin*/
    if (err = adi_hmc7044_private_pll_enable_input_reference_set(device, 4, 1), err != API_CMS_ERROR_OK)
        return err;
    /*set clkin priority*/
    if (err = adi_hmc7044_private_pll_input_reference_priority_set(device, config->ref_priority, 4), err != API_CMS_ERROR_OK)
        return err;

     /*calculate fPFD1*/
    pfd1_clk_hz = gcd(config->ref_clk_freq_hz, config->fvcxo_clk_freq_hz);
    while (pfd1_clk_hz / M1 > 50e6) {
        M1++;
    }
    pfd1_clk_hz = pfd1_clk_hz / M1;
    if (config->fvcxo_clk_freq_hz / 65535 > pfd1_clk_hz) {
        return API_CMS_ERROR_ERROR;
    }
    uint64_t ref_div = config->ref_clk_freq_hz / pfd1_clk_hz;
    flcm_clk_hz = config->ref_clk_freq_hz;
    while (flcm_clk_hz / M2 > 123.00e6 || ref_div % M2 !=0) {
        M2++;
    }
    clkin_prescaler = M2;
    flcm_clk_hz = config->ref_clk_freq_hz / M2;
    vcxo_prescaler = (config->fvcxo_clk_freq_hz / pfd1_clk_hz)/(ref_div / clkin_prescaler);

    if (err = adi_hmc7044_private_pll_input_reference_prescaler_config_set(device, 0, clkin_prescaler), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_input_reference_prescaler_config_set(device, 1, clkin_prescaler), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_input_reference_prescaler_config_set(device, 2, clkin_prescaler), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_input_reference_prescaler_config_set(device, 3, clkin_prescaler), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_hmc7044_private_pll_input_reference_oscin_prescaler_config_set(device, vcxo_prescaler), err != API_CMS_ERROR_OK)
        return err;

    /* calculate pll2ref*/
    pll2ref_clk_hz = config->fvcxo_clk_freq_hz <= HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MAX
        ? 2 * config->fvcxo_clk_freq_hz
        : config->fvcxo_clk_freq_hz;

    /*calculate fvco*/
    pfd2_gcd_hz = pll2ref_clk_hz;
    for (i = 0; i < HMC7044_NOF_OP_CH; i++) {
        if (config->output_freq_hz[i]) {
            pfd2_gcd_hz = gcd(pfd2_gcd_hz, config->output_freq_hz[i]);
        }
    }

    pfd2_lcm_hz = pfd2_gcd_hz;
    for (i = 0; i < HMC7044_NOF_OP_CH; i++) {
        if (config->output_freq_hz[i]) {
            pfd2_lcm_hz = lcm(pfd2_lcm_hz, config->output_freq_hz[i]);
        }
    }

    uint64_t  fdist;
    for (uint64_t k = 0; k < HMC7044_PLL2_R_DIV_MAX; k++) {
        fdist = k * pfd2_lcm_hz;
        if (fdist > HMC7044_VCO_CLK_FREQ_HZ_MIN && fdist < HMC7044_VCO_CLK_FREQ_HZ_MAX) {
            /*check output divider, should be 1,3,5 or even numbers*/
            for (i = 0; i < HMC7044_NOF_OP_CH; i++) {
                if (config->output_freq_hz[i]) {
                    output_chan_divider = fdist / config->output_freq_hz[i];
                    if (output_chan_divider % 2 != 0) {
                        if (output_chan_divider != 1 && output_chan_divider != 3 && output_chan_divider != 5) {
                            break;
                        }
                    }
                }
            }
            if (i == HMC7044_NOF_OP_CH) {
                fvco_clk_hz = fdist;
                break;
            }
        }
    }

    /*calculate fpfd2*/
    pfd2_clk_hz = gcd(fvco_clk_hz, pll2ref_clk_hz);

    if (pfd2_clk_hz < HMC7044_PD2_CLK_FREQ_HZ_MIN || pfd2_clk_hz > HMC7044_PD2_CLK_FREQ_HZ_MAX) {
        return API_CMS_ERROR_ERROR;
    }
    /*calculate R1, N1, R2, N2*/
    R1 = flcm_clk_hz / pfd1_clk_hz;
    N1 = config->fvcxo_clk_freq_hz / pfd1_clk_hz;
    R2 = pll2ref_clk_hz / pfd2_clk_hz;
    N2 = fvco_clk_hz / pfd2_clk_hz;

    /*pll config*/
    if (err = adi_hmc7044_private_pll_pll1_config_set(device, R1, N1), err != API_CMS_ERROR_OK)
        return err;

    if (err = hmc7044_sw_delay_us(device, 10000), err != API_CMS_ERROR_OK)
        return err;

    if (err = adi_hmc7044_private_pll_device_pll_lock_status_get(device, &pll_lock_st), err != API_CMS_ERROR_OK)
    	return err;

    if (!(pll_lock_st & HMC7044_PLL1_LOCK_ST))
        return API_CMS_ERROR_PLL_NOT_LOCKED;

    if (config->fvcxo_clk_freq_hz <= HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MAX)
    {
        if (err = adi_hmc7044_private_pll_pll2_config_set(device, 0x0, R2, N2), err != API_CMS_ERROR_OK)
            return err;
    }
    else
    {
        if (err = adi_hmc7044_private_pll_pll2_config_set(device, 0x1, R2, N2), err != API_CMS_ERROR_OK)
            return err;
    }
    if (err = hmc7044_sw_delay_us(device, 100000), err != API_CMS_ERROR_OK)
        return err;

    if (err = adi_hmc7044_private_device_reg_update(device), err != API_CMS_ERROR_OK)
                return err;
    if (err = hmc7044_sw_delay_us(device, 100000), err != API_CMS_ERROR_OK)
        return err;

    /*Get PLL Lock Status*/
    if (err = adi_hmc7044_private_pll_device_pll_lock_status_get(device, &pll_lock_st), err != API_CMS_ERROR_OK)
    	return err;
    if (!(pll_lock_st & HMC7044_PLL2_LOCK_ST))
        return API_CMS_ERROR_PLL_NOT_LOCKED;

    if (err = hmc7044_sw_delay_us(device, 100000), err != API_CMS_ERROR_OK)
        return err;

    /*output config*/

    if (err = adi_hmc7044_private_output_performance_set(device, 1), err != API_CMS_ERROR_OK)
        return err;

    /*output config*/
    adi_hmc7044_op_driver_config_t hmc_driver_config;
    hmc_driver_config.mode = SIGNAL_CML;
    hmc_driver_config.impedance = ADI_CMS_INTERNAL_RESISTOR_50_OHM;
    hmc_driver_config.force_mute_en = 0;
    hmc_driver_config.dynamic_driver_en = 0;
    /*Configure Clockout Frequencies*/
    for (i = 0; i < HMC7044_NOF_OP_CH; i++) {
        if ((config->output_ch >> i & 0x1)) {
            if (adi_hmc7044_private_output_driver_config_set(device, i, &hmc_driver_config), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_private_output_config_set(device, i, HMC7044_OP_SIG_CH_DIV, fvco_clk_hz / config->output_freq_hz[i], 0, 1), err != API_CMS_ERROR_OK)
                return err;
        }
    }
    return API_CMS_ERROR_OK;

}

int32_t adi_hmc7044_private_device_reg_update(adi_hmc7044_device_t *device)
{
    int32_t err;
    uint8_t reg_val;
    err = adi_hmc7044_private_device_spi_register_get(device, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    hmc7044_sw_delay_us(device, 1000);
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, (reg_val | 0x42));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    hmc7044_sw_delay_us(device, 1000);
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, (reg_val | 0x40));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_high_performance_set(adi_hmc7044_device_t *device)
{
    int32_t err;
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG, 0x60);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_vco_sel_set(adi_hmc7044_device_t *device, uint8_t vco_sel, uint8_t ext_vco_div_en)
{
    int32_t err;
    uint8_t reg_val = 0x00;

    if (vco_sel >= HMC7044_VCO_SEL_INVALID || ext_vco_div_en > 1)  {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    if (err = adi_hmc7044_private_device_spi_register_get(device, HMC7044_GLOBAL_ENABLE_CTRL_REG, &reg_val), err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val &= ~(0x18);

    if (vco_sel != HMC7044_VCO_EXTERNAL)  {
        reg_val |= (vco_sel << 3);
    }
    else {
        if (err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_EXT_VCO_CTRL, (ext_vco_div_en << 1)), err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    if (err =  adi_hmc7044_private_device_spi_register_set(device, HMC7044_GLOBAL_ENABLE_CTRL_REG, reg_val), err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_channel_out_en(adi_hmc7044_device_t *device, uint8_t ch_en)
{
    int32_t err;
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_GLOBAL_CH_ENABLE_CTRL_REG, ch_en);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_input_reference_path_en(adi_hmc7044_device_t *device, uint8_t sync_mode, uint8_t vco_input_mode, uint8_t sync_input_mode, uint8_t clk_in)
{
    int32_t err;
    uint8_t reg_val = 0x00;
    if (err =  adi_hmc7044_private_device_spi_register_get(device, HMC7044_GLOBAL_MODE_ENABLE_CTRL_REG, &reg_val), err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val &= ~((0x1 << 6) | (0x1<< 5) | (0x1 << 4) | (0x1));
    reg_val |= ((sync_mode << 6) | (vco_input_mode << 5) | (sync_input_mode << 4) | clk_in);
    err =  adi_hmc7044_private_device_spi_register_set(device, HMC7044_GLOBAL_MODE_ENABLE_CTRL_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_sysref_config_set(adi_hmc7044_device_t *device, uint8_t pulse_gen_mode)
{
    int32_t err;
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_SYSREF_SYNC_CTRL_REG, pulse_gen_mode);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_sync_config_set(adi_hmc7044_device_t *device, uint8_t sync_retime, uint8_t sync_pll2, uint8_t sync_polarity)
{
    int32_t err;
    uint8_t reg_val = (sync_retime << 2) | (sync_pll2 << 1) | sync_polarity;
    err =  adi_hmc7044_private_device_spi_register_set(device, HMC7044_SYNC_CTRL_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_sysref_timer_config_set(adi_hmc7044_device_t *device, uint16_t sysref_timer)
{
    int32_t err;
    err =  adi_hmc7044_private_device_spi_register_set(device, HMC7044_SYNC_TIMER_LSB_CTRL_REG, sysref_timer & 0xFF);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    err =  adi_hmc7044_private_device_spi_register_set(device, HMC7044_SYNC_TIMER_MSB_CTRL_REG, (sysref_timer >> 8) & 0xFF);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_private_device_alarm_mask_config_set(adi_hmc7044_device_t *device, adi_hmc7044_alarm_mask_config_t *config)
{
    int32_t err;
    uint8_t reg_val = (config->pll1_lock << 7) | (config->pll1_lock_aquisition << 6) | (config->pll1_lock_detect << 5) | (config->pll1_holdover_status << 4) | config->pll1_clkInx_status;
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_ALARM_MASK_CTRL_1_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    reg_val = (config->sync_request << 4) | (config->pll1_pll2_lock_detect << 3) | (config->clkoutputs_phase_status << 2) | (config->sysref_sync_status << 1) | config->pll2_lock_detect;
    err = adi_hmc7044_private_device_spi_register_set(device, HMC7044_ALARM_MASK_CTRL_2_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

/*! @! */
