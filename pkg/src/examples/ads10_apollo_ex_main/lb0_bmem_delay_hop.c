#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo Loopback0 BMEM delay test
 *
 * On both sides, an input on ADC0 is looped back to DAC0, with a delay applied to BMEM B-side.
 * With every hop, a phase shift change between the signals on an oscilloscope should be observed.
 *
 * Test setup:
 *   -- Signal generator @ 100MHz -> splitter -> ADC A0, ADC B0
 *   -- DAC A0 -> oscilloscope channel 1
 *   -- DAC B0 -> oscilloscope channel 2
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "adi_apollo.h"
#include "adi_apollo_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_ctl.h"

static int32_t loopback0_setup(adi_apollo_device_t *device, uint16_t adc_select, uint8_t rdptr_rst, uint8_t wrptr_rst, uint16_t *xbar_a, uint16_t *xbar_b);

int32_t lb0_bmem_delay_hop(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    uint32_t run_cals = ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC | ADI_ADS10_APOLLO_CAL_ADC_BG;
    uint16_t side_mux[ADI_APOLLO_DAC_PER_SIDE_NUM / 2] = {ADI_APOLLO_ADC_0, ADI_APOLLO_ADC_1};
    adi_apollo_bmem_delay_hop_t bmem_config = {0};
    uint8_t i = 0;

    bmem_config.hop_delay[0] = 0;
    bmem_config.hop_delay[1] = 70;
    bmem_config.hop_delay[2] = 140;
    bmem_config.hop_delay[3] = 210;
    bmem_config.parity_check_en = 1;

    // Configure BMEM
    err = adi_apollo_bmem_hsdin_delay_hop_config(device, ADI_APOLLO_BMEM_B0, &bmem_config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_trigts_bmem_trig_sel_mux_set(device, ADI_APOLLO_RX, ADI_APOLLO_BMEM_B0, ADI_APOLLO_TRIG_SPI);
    ADI_CMS_ERROR_RETURN(err);

    // Configure loopback
    err = loopback0_setup(device, ADI_APOLLO_ADC_A0 | ADI_APOLLO_ADC_B0, 2, 2, side_mux, side_mux);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb0_bmem_enable_set(device, ADI_APOLLO_SIDE_B, 1);
    ADI_CMS_ERROR_RETURN(err);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, run_cals);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_bmem_hsdin_delay_start(device, ADI_APOLLO_BMEM_B0);
    ADI_CMS_ERROR_RETURN(err);

    for (i = 0; i < 6; i++) {
        EXCTL_PAUSE(interactive);

        err = adi_apollo_trigts_trig_now(device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

static int32_t loopback0_setup(adi_apollo_device_t *device, uint16_t adc_select, uint8_t rdptr_rst, uint8_t wrptr_rst, uint16_t *xbar_a, uint16_t *xbar_b) {
    int32_t err;
    uint16_t sides;

    err = adi_apollo_utils_side_from_adc_select_get(device, adc_select, &sides);
    ADI_CMS_ERROR_RETURN(err);

    // Set FIFO read ptr and write ptr - 2 for B0 silicon, otherwise can be 0, 1, or 2
    err = adi_apollo_loopback_lb0_read_ptr_rst_set(device, adc_select, rdptr_rst);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb0_write_ptr_rst_set(device, sides, wrptr_rst);
    ADI_CMS_ERROR_RETURN(err);

    // Set xbar value
    if (xbar_a != NULL) {
        err = adi_apollo_loopback_lb0_tx_xbar_set(device, ADI_APOLLO_SIDE_A, xbar_a, 2);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (xbar_b != NULL) {
        err = adi_apollo_loopback_lb0_tx_xbar_set(device, ADI_APOLLO_SIDE_B, xbar_b, 2);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_apollo_loopback_lb0_tx_enable_set(device, ~adc_select, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb0_rx_enable_set(device, ~sides, 0);
    ADI_CMS_ERROR_RETURN(err);

    // Enable specified tx/rx
    printf("Enabling ADC: 0x%02X; Side: 0x%02X\n", adc_select, sides);

    err = adi_apollo_loopback_lb0_rx_enable_set(device, sides, 1);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb0_tx_enable_set(device, adc_select, 1);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
