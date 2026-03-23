#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     Versal Apollo examples common HMC7044 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "xil_printf.h"

#include "versal_apollo_ex_hmc7044.h"

#include "adi_hmc7044_clkout.h"
#include "adi_hmc7044_core.h"
#include "adi_hmc7044_device.h"
#include "adi_hmc7044_pll.h"
#include "versal_hmc7044_pl.h"

#include "versal_hal.h"

#define ADI_ADF4030_REF_FREQ_MAX      		250000000U  // 250Mhz

/*
 * HMC7044 SPI wrapper callbacks — route HAL calls to custom PL IP (3-wire).
 * The adi_hmc7044 HAL expects standard CMS SPI callback signatures.
 * We wrap hmcRead/hmcWrite to match those signatures.
 */
static int32_t hmc7044_pl_spi_write(void *user_data, uint8_t *indata, uint32_t len)
{
    (void)user_data;

    /*
     * HMC7044 SPI frame (24-bit): [R/W(1)] [W1W0(2)] [A12..A0(13)] [D7..D0(8)]
     * For write: bit23=0, bits22-21=W1W0, bits20-8=addr, bits7-0=data
     * indata[] contains the raw SPI frame bytes (MSB first).
     * Typically len=3: indata[0]=cmd|addr_hi, indata[1]=addr_lo, indata[2]=data
     */
    if (len < 3) return API_CMS_ERROR_INVALID_PARAM;

    uint16_t address = ((uint16_t)(indata[0] & 0x1F) << 8) | (uint16_t)indata[1];
    uint8_t data = indata[2];

    return hmcWrite(address, data);
}

static int32_t hmc7044_pl_spi_read(void *user_data, uint8_t *indata, uint8_t *outdata, uint32_t len)
{
    (void)user_data;

    /*
     * HMC7044 SPI frame for read: bit23=1, bits20-8=addr
     * indata[0]=cmd|addr_hi, indata[1]=addr_lo
     * outdata[2]=read data (returned in last byte position)
     */
    if (len < 3) return API_CMS_ERROR_INVALID_PARAM;

    uint16_t address = ((uint16_t)(indata[0] & 0x1F) << 8) | (uint16_t)indata[1];
    uint8_t data = 0;
    int32_t err = hmcRead(address, &data);

    if (outdata != NULL && len >= 3) {
        outdata[0] = 0;
        outdata[1] = 0;
        outdata[2] = data;
    }

    return err;
}

int32_t versal_apollo_ex_hmc7044_hal_config(adi_hmc7044_device_t *hmc7044, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn)
{
    int32_t err;
    adi_hmc7044_hal_t* hal = &hmc7044->hal_info;

    ADI_CMS_NULL_PTR_CHECK(hmc7044);

    err = versal_hmc7044_hal_config_data(hmc7044, &versal_hmc7044_hal_spi_config_init);
    ADI_CMS_ERROR_RETURN(err);

    hal->delay_us = &versal_wait_us;

    /* Route HMC7044 SPI through custom PL IP (3-wire) instead of XSpi */
    hal->spi_read = &hmc7044_pl_spi_read;
    hal->spi_write = &hmc7044_pl_spi_write;

    xil_printf("HMC7044: HAL wired to custom PL IP (3-wire SPI)\r\n");

    /* Verify SPI read is functional */
    err = adi_hmc7044_core_spi_reg_test(hmc7044);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: HMC7044 SPI Reg Test Failed! err=%d\r\n", err);
        xil_printf("NOTE: hmcRead/hmcWrite must be implemented in versal_hmc7044_pl.c\r\n");
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t versal_apollo_ex_hmc7044_startup(adi_hmc7044_device_t *hmc7044,
                                            uint64_t ref_freq_hz,
                                            adi_hmc7044_device_rational_freq_t *sysref_hz,
                                            adi_hmc7044_device_rational_freq_t *fpga_ref_hz)
{
    int32_t err = 0;
    uint64_t fpga_ref_freq_hz = fpga_ref_hz->freq_hz / fpga_ref_hz->div;
    adi_hmc7044_device_rational_freq_t adf4030_ref_hz;

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

    /* FMCA path: SCLKOUT1 = ADF4030 ref */
    adi_hmc7044_device_rational_freq_t sclkout_1 = adf4030_ref_hz;

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
            sclkout_1,      // SCLKOUT1   ADF4030_REF_IN
            *fpga_ref_hz,   // CLKOUT2    FPGA_REFCLK_5
            *sysref_hz,     // SCLKOUT3   ADF4030_BSYNC0_INPUT
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

    err = versal_apollo_ex_hmc7044_hal_config(hmc7044, NULL, NULL);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_reset(hmc7044, 0);
    ADI_CMS_ERROR_RETURN(err);

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

    err = versal_apollo_ex_hmc7044_reset_fsm_reseed(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_clkout_config_set(hmc7044, &device_clkout);
    ADI_CMS_ERROR_RETURN(err);

    /* SCLKOUT3 = ADF4030_BSYNC0_INPUT (SIGNAL_LVPECL for FMCA) */
    err = adi_hmc7044_clkout_channel_driver_config_set(hmc7044, 3, &bsync0_clkout_driver_config);
    ADI_CMS_ERROR_RETURN(err);

    err = versal_apollo_ex_hmc7044_reset_fsm_reseed(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t versal_apollo_ex_hmc7044_reset_fsm_reseed(adi_hmc7044_device_t *hmc7044)
{
    int32_t err = 0;
    ADI_CMS_NULL_PTR_CHECK(hmc7044);

    err = adi_hmc7044_device_fsm_div_restart_run(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_hmc7044_device_reseed_run(hmc7044);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
