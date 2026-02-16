#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples common clocking functions implementation
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_types.h"
#include "adi_ads10_apollo_ex_clk.h"
#include "adi_ads10_apollo_ex_adf4382.h"
#include "adi_ads10_apollo_ex_hmc7044.h"
#include "adi_hmc7044_device_types.h"
#include "adi_fpga_apollo_private.h"
#include "adi_fpga_jesd_registers.h"
#include "adi_fpga_apollo_common_types.h"
#include "adi_fpga_apollo_clk.h"
#include "ads10_hal.h"
#include "ads10_fpga.h"

#define DEV_CLK_MODE_MASK ( \
    ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER \
    | ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_DUAL \
    | ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382 \
)

#define FPGA_CLK_MODE_MASK ( \
    ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL \
    | ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_FMC \
)

int32_t adi_ads10_apollo_ex_configure_profile_clks(adi_fpga_apollo_device_t *fpga_device,
                                                   uint32_t ltc6955_clk_khz,
                                                   adi_apollo_top_t *profile,
                                                   adi_ads10_apollo_clk_mode_e mode)
{

    int32_t err;

    err = adi_fpga_apollo_clk_line_rate_div_pgm(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    return adi_ads10_apollo_ex_configure_clks(fpga_device,
                                              ltc6955_clk_khz,
                                              profile->clk_cfg.dev_clk_freq_kHz,
                                              profile->mcs_cfg.internal_sysref_prd_digclk_cycles_center,
                                              profile->clk_cfg.clocking_mode,
                                              fpga_device->state_info.clk_info.max_link_rate_khz,
                                              fpga_device->state_info.clk_info.ref_clk_div,
                                              mode);
}

int32_t adi_ads10_apollo_ex_configure_clks(adi_fpga_apollo_device_t *fpga_device,
                                           uint32_t ltc6955_clk_khz,
                                           uint32_t dev_clk_khz,
                                           uint16_t digclk_cycles,
                                           adi_apollo_divg_mode_e divg_mode,
                                           uint32_t lane_rate_khz,
                                           uint8_t fpga_clk_div,
                                           adi_ads10_apollo_clk_mode_e mode)
{
    int32_t err;
    uint32_t ads10_sysref = 0, ads10_vcxo = 0, ads10_mgt = 0;
    adi_ads10_apollo_clk_mode_e dev_clk_mode = mode & DEV_CLK_MODE_MASK;
    adi_ads10_apollo_clk_mode_e fpga_clk_mode = mode & FPGA_CLK_MODE_MASK;
    adi_adf4382_device_t adf4382_device = { {0} };
    adi_hmc7044_device_t hmc7044_device = { {0} };
    adi_hmc7044_device_rational_freq_t sysref_hz = {
        .freq_hz = dev_clk_khz * 1e3,
        .div = (divg_mode == ADI_APOLLO_CLOCKING_MODE_SDR_DIV_8 ? 8 : 4) * digclk_cycles
    };
    adi_hmc7044_device_rational_freq_t fpga_ref_hz = {
        .freq_hz = lane_rate_khz * 1e3,
        .div = fpga_clk_div
    };

    ADI_CMS_SINGLE_SELECT_CHECK(dev_clk_mode);
    ADI_CMS_SINGLE_SELECT_CHECK(fpga_clk_mode);

    switch (fpga_clk_mode) {
        case ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_FMC:
            err = adi_ads10_apollo_ex_hmc7044_startup(&hmc7044_device,
                                                      ((uint64_t) (ltc6955_clk_khz * 1e3)),
                                                      &sysref_hz,
                                                      &fpga_ref_hz);
            ADI_CMS_ERROR_RETURN(err);

            ads10_sysref = 1;
            ads10_vcxo   = 1;
            ads10_mgt    = 1;
            break;
        default:
            // Fall through
        case ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL:
            // NOP
            break;
    }

    err = adi_fpga_apollo_private_write32_bitfield(fpga_device, CLK_SRC_SEL, MGT_REFCLK_SEL_MASK, ads10_mgt);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga_device, CLK_SRC_SEL, SYSREF_SEL_MASK, ads10_sysref);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga_device, MISC_0, AD9528_VCXO_SELECT_MASK, ads10_vcxo);
    ADI_CMS_ERROR_RETURN(err);

    switch (dev_clk_mode) {
        case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382:
            err = adi_ads10_apollo_ex_adf4382_startup(&adf4382_device,
                                                      ((uint64_t) (dev_clk_khz * 1e3)),
                                                      ((uint64_t) (ltc6955_clk_khz * 1e3)));
            ADI_CMS_ERROR_RETURN(err);
            break;
        default:
            // Fall through
        case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER:
            // Fall through
        case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_DUAL:
            // NOP
            break;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_clk_power_cal(adi_apollo_device_t *device,
                                          adi_ads10_apollo_clk_mode_e clk_mode,
                                          uint64_t rfout_freq_hz,
                                          uint64_t ref_freq_hz)
{
    int32_t err = API_CMS_ERROR_OK;
    bool power_good = false;
    adi_apollo_clk_input_power_status_e pwr_stat_a, pwr_stat_b;
    adi_adf4382_device_t adf4382 = { {0} };
    uint8_t clk1_opwr = 5;

    if (clk_mode & ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382) {
        err = adi_ads10_apollo_ex_adf4382_hal_config(&adf4382,
                                                     NULL,
                                                     &ads10_fpga_fmcb_aux_gpio_write);
        ADI_CMS_ERROR_RETURN(err);

        while (!power_good) {
            // Set clock1 output power
            err = adi_adf4382_rfout_clk_opwr_set(&adf4382, ADI_ADF4382_CLK1_SEL, clk1_opwr);
            ADI_CMS_ERROR_RETURN(err);

            err = adi_apollo_clk_mcs_input_power_status_get(device, &pwr_stat_a, &pwr_stat_b);
            ADI_CMS_ERROR_RETURN(err);

            if (pwr_stat_a == ADI_APOLLO_CLK_PWR_UNDERDRIVEN) {
                clk1_opwr++;
            } else if (pwr_stat_a == ADI_APOLLO_CLK_PWR_OVERDRIVEN) {
                clk1_opwr--;
            } else {
                power_good = true;
            }
        }
    } else {
        err = adi_apollo_clk_mcs_input_power_status_get(device, &pwr_stat_a, &pwr_stat_b);
        ADI_CMS_ERROR_RETURN(err);
    }

    printf("Clock input power detection A: %s\n", !pwr_stat_a ? "GOOD" : (pwr_stat_a == ADI_APOLLO_CLK_PWR_UNDERDRIVEN ? "UNDERDRIVEN" : (pwr_stat_a == ADI_APOLLO_CLK_PWR_OVERDRIVEN ? "OVERDRIVEN" : "UNUSED")));
    printf("Clock input power detection B: %s\n", !pwr_stat_b ? "GOOD" : (pwr_stat_b == ADI_APOLLO_CLK_PWR_UNDERDRIVEN ? "UNDERDRIVEN" : (pwr_stat_b == ADI_APOLLO_CLK_PWR_OVERDRIVEN ? "OVERDRIVEN" : "UNUSED")));

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
