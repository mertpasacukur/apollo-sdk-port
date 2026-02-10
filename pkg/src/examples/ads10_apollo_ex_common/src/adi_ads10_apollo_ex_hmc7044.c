/*!
 * \brief     ADS10 Apollo examples common HMC7044 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_ads10_apollo_ex_hmc7044.h"

// Import All HMC7044 Feature Headers
#include "adi_hmc7044_clkout.h"
#include "adi_hmc7044_core.h"
#include "adi_hmc7044_device.h"
#include "adi_hmc7044_pll.h"

#include "ads10_fpga.h"

#define ADI_ADF4030_REF_FREQ_MAX      		250000000U  // 250Mhz

int32_t adi_ads10_apollo_ex_hmc7044_hal_config(adi_hmc7044_device_t *hmc7044, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn)
{
    int32_t err;
    adi_hmc7044_hal_t* hal = &hmc7044->hal_info;
    adi_hmc7044_gpo_config_t gpo_config = {
        .enabled = 1,
        .mode = ADI_HMC7044_DEVICE_GPO_MODE_SDATA_SPI,
        .output = ADI_HMC7044_DEVICE_GPO_OUTPUT_CMOS,
    };

    ADI_CMS_NULL_PTR_CHECK(hmc7044);
    ADI_CMS_NULL_PTR_CHECK(sdo_en_fcn);

    err = ads10_hmc7044_hal_config_data(hmc7044, &ads10_hmc7044_hal_spi_config_init);
    ADI_CMS_ERROR_RETURN(err);

    hal->delay_us = &ads10_wait_us;
    hal->spi_read = &ads10_cms_spi_read;
    hal->spi_write = &ads10_cms_spi_write;

    // Check if SPI read is functional
    err = adi_hmc7044_core_spi_reg_test(hmc7044);

    // If SPI read fails, configure for FMCB SPI read
    if (err == API_CMS_ERROR_SPI_XFER) {
        printf("\n");
        printf("*** WARNING: HMC7044 SPI Reg Test Failed!! ***\n");
        printf("Configuring HMC7044 GPO3 in SDATA mode and Retrying...\n");

        // Configure FMCB SPI read
        hal->spi_read = &ads10_cms_fmcb_spi_read;
        hal_spi_fmcb_config_t *fmcb_cfg = (hal_spi_fmcb_config_t *) calloc(1, sizeof(hal_spi_fmcb_config_t));
        ADI_CMS_MEM_ALLOC_CHECK(fmcb_cfg);

        fmcb_cfg->spi_byte_0[0] = 255;
        fmcb_cfg->target[0] = 0;
        fmcb_cfg->sdo_en_context = sdo_en_context;
        fmcb_cfg->sdo_en_fcn = sdo_en_fcn;

        hal_spi_config_t *spi_cfg = (hal_spi_config_t *)hal->user_data;
        spi_cfg->user_data = fmcb_cfg;
        spi_cfg->spi_3wire = 0; // Set to 4 wire mode for using HMC GPO config

        err = adi_hmc7044_device_gpo_config_set(hmc7044, ADI_HMC7044_DEVICE_GPO3, &gpo_config);
        ADI_CMS_ERROR_RETURN(err);

        // Verify read/write
        err = adi_hmc7044_core_spi_reg_test(hmc7044);
        printf("HMC7044 SPI Reg Test: %s.\n", (err == 0) ? "PASSED" : "FAILED");
        ADI_CMS_ERROR_RETURN(err);
        printf("\n");
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_hmc7044_startup(adi_hmc7044_device_t *hmc7044,
                                            uint64_t ref_freq_hz,
                                            adi_hmc7044_device_rational_freq_t *sysref_hz,
                                            adi_hmc7044_device_rational_freq_t *fpga_ref_hz)
{
    int32_t err = 0;
    uint64_t fpga_ref_freq_hz = fpga_ref_hz->freq_hz / fpga_ref_hz->div;
    adi_hmc7044_device_rational_freq_t adf4030_ref_hz;
    bool is_fmcb_brd = false;

    ADI_CMS_NULL_PTR_CHECK(hmc7044);
    ADI_CMS_NULL_PTR_CHECK(sysref_hz);
    ADI_CMS_NULL_PTR_CHECK(fpga_ref_hz);


    if (fpga_ref_freq_hz < ADI_ADF4030_REF_FREQ_MAX) {
        adf4030_ref_hz.freq_hz = fpga_ref_hz->freq_hz;
        adf4030_ref_hz.div = fpga_ref_hz->div;
    } else {
        adf4030_ref_hz.freq_hz = fpga_ref_hz->freq_hz;
        adf4030_ref_hz.div = (fpga_ref_hz->div * 2);
    }

    // DEBUG_ONLY
    // printf("hmc7044_ref_freq_hz: %lld.\n", ref_freq_hz);
    // printf("sysref_hz: %lld.\n", sysref_hz->freq_hz / sysref_hz->div);
    // printf("fpga_ref_hz: %lld \t freq_hz: %lld \t div: %lld.\n", fpga_ref_freq_hz, fpga_ref_hz->freq_hz, fpga_ref_hz->div);
    // printf("adf4030_ref_hz: %lld \t freq_hz: %lld \t div: %lld.\n", (adf4030_ref_hz.freq_hz / adf4030_ref_hz.div), adf4030_ref_hz.freq_hz, adf4030_ref_hz.div);

    adi_hmc7044_device_rational_freq_t sclkout_1 = (is_fmcb_brd == true) ? *sysref_hz : adf4030_ref_hz;

    adi_hmc7044_clkout_driver_config_t bsync0_clkout_driver_config = {
        .mode              = SIGNAL_LVPECL,
        .impedance         = ADI_CMS_NO_INTERNAL_RESISTOR,
        .force_mute_en     = 0,
        .dynamic_driver_en = 1
    };

    adi_hmc7044_clkout_config_t clkout = {
        .ch_div          = 0,
        .enable          = 0,
        .high_perform_en = 0,
        .mux_sel         = ADI_HMC7044_CLKOUT_MUX_SEL_CH_DIV
    };
    adi_hmc7044_pll_los_config_t los_config = {
        .timer_cycles       = ADI_HMC7044_PLL_LOS_WAIT_CYCLES_8,
        .input_prescaler_en = 1,
        .vcxo_prescaler_en  = 0
    };
    adi_hmc7044_clkout_sync_config_t sync_config = {
        .startup_mode = ADI_HMC7044_CLKOUT_STARTUP_MODE_ASYNC,
        .slip_mode_en = 1,
        .sync_mode_en = 1
    };
    adi_hmc7044_clkout_multi_slip_config_t multi_slip_config = {
        .multi_slip_en = 0,
        .slip_delay    = 0
    };
    adi_hmc7044_device_clkout_config_t device_clkout = {
        .clkin                 = ADI_HMC7044_CLKIN0,
        .clkout                = ADI_HMC7044_SCLKOUT1
                               | ADI_HMC7044_CLKOUT2
                               | ADI_HMC7044_SCLKOUT3
                               | ADI_HMC7044_SCLKOUT5
                               | ADI_HMC7044_CLKOUT8
                               | ADI_HMC7044_SCLKOUT9
                               | ADI_HMC7044_CLKOUT10
                               | ADI_HMC7044_SCLKOUT11
                               | ADI_HMC7044_CLKOUT12
                               | ADI_HMC7044_SCLKOUT13,
        .ref_clk_hz            = ref_freq_hz,
        .vcxo_hz               = 125e6,
        .ref_priority          = {
            ADI_HMC7044_CLKIN0,
            ADI_HMC7044_CLKIN1,
            ADI_HMC7044_CLKIN2,
            ADI_HMC7044_CLKIN3
        },
        .output_freq           = {
            { 0 },
            sclkout_1,      // SCLKOUT1   ADF4030_REF_IN(FMCA) APOLLO_SYSREF(FMCB)
            *fpga_ref_hz,   // CLKOUT2    FPGA_REFCLK_5
            *sysref_hz,     // SCLKOUT3   ADF4030_BSYNC0_INPUT(FMCA) FPGA_SYSREF(FMCB)
            { 0 },
            *fpga_ref_hz,   // SCLKOUT5   FPGA_REFCLK_4
            { 0 },
            { 0 },
            *fpga_ref_hz,   // CLKOUT8    FPGA_GBCLK0
            *fpga_ref_hz,   // SCLKOUT9   FPGA_GBCLK1
            *fpga_ref_hz,   // CLKOUT10   FPGA_REFCLK_0
            *fpga_ref_hz,   // SCLKOUT11  FPGA_REFCLK_1
            *fpga_ref_hz,   // CLKOUT12   FPGA_REFCLK_2
            *fpga_ref_hz    // SCLKOUT13  FPGA_REFCLK_3
        },
        .clkout_config         = {
            .ch_div            = 0,
            .enable            = 1,
            .high_perform_en   = 0,
            .mux_sel           = ADI_HMC7044_CLKOUT_MUX_SEL_CH_DIV
        },
        .clkout_driver_config  = {
            .mode              = SIGNAL_LVDS,
            .impedance         = ADI_CMS_NO_INTERNAL_RESISTOR,
            .force_mute_en     = 0,
            .dynamic_driver_en = 1
        }
    };

	adi_hmc7044_gpo_config_t gpo_config = {
        .enabled = 1,
        .mode = ADI_HMC7044_DEVICE_GPO_MODE_SDATA_SPI,
        .output = ADI_HMC7044_DEVICE_GPO_OUTPUT_CMOS,
    };

    err = adi_ads10_apollo_ex_hmc7044_hal_config(hmc7044,
                                                 NULL,
                                                 &ads10_fpga_fmcb_aux_spi2_sdo_alt_enable_set);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_reset(hmc7044, 0);
    ADI_CMS_ERROR_RETURN(err);

	// FMCB Eval Brd requires GPO3 to be configured as SDATA
    if (is_fmcb_brd == true) {
        err = adi_hmc7044_device_gpo_config_set(hmc7044, ADI_HMC7044_DEVICE_GPO3, &gpo_config);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_hmc7044_device_init(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_gpi_enable_set(hmc7044, ADI_HMC7044_DEVICE_GPI3, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_clkout_config_set(hmc7044, ADI_HMC7044_OUTPUT_ALL, &clkout);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_pll_clkin_config_set(hmc7044,
                                           ADI_HMC7044_CLKIN0 | ADI_HMC7044_CLKIN1,
                                           ADI_CMS_INTERNAL_RESISTOR_100_OHM | ADI_HMC7044_PLL_IPBUFFER_AC_COUPLED_MODE,
                                           1);
    ADI_CMS_ERROR_RETURN(err);


    err = adi_hmc7044_pll_los_config_set(hmc7044, &los_config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_int_vco_sel_set(hmc7044, ADI_HMC7044_HMC7044_INT_VCO_SEL_2GHZ);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_clkout_sync_config_set(hmc7044, device_clkout.clkout, &sync_config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_clkout_multi_slip_config_set(hmc7044, ADI_HMC7044_SCLKOUT3, &multi_slip_config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_hmc7044_reset_fsm_reseed(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_clkout_config_set(hmc7044, &device_clkout);
    ADI_CMS_ERROR_RETURN(err);

    // SCLKOUT3   ADF4030_BSYNC0_INPUT(FMCA: SIGNAL_LVPECL) FPGA_SYSREF(FMCB: SIGNAL_LVDS)
    if (is_fmcb_brd == false) {
        err = adi_hmc7044_clkout_channel_driver_config_set(hmc7044, 3, &bsync0_clkout_driver_config);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_ads10_apollo_ex_hmc7044_reset_fsm_reseed(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_hmc7044_reset_fsm_reseed(adi_hmc7044_device_t *hmc7044)
{
    int32_t err = 0;
    ADI_CMS_NULL_PTR_CHECK(hmc7044);

    err = adi_hmc7044_device_fsm_div_restart_run(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_reseed_run(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}
