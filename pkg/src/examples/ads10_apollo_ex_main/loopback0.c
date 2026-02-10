#if defined(__linux__)

/*!
 * \brief     ADS10 Apollo Loopback0 test
 *            The example demonstrates Apollo's Loopback 0 feature where the DSP blocks are bypassed
 *            and an input signal on an ADC will be transmitted to the mapped DAC via the lowest latency path.
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
#include <math.h>
#include "adi_apollo.h"
#include "adi_apollo_loopback.h"
#include "adi_apollo_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_ctl.h"

#define TEST_CASES 5

static int32_t loopback0_setup(adi_apollo_device_t *device, uint16_t adc_select, uint8_t rdptr_rst, uint8_t wrptr_rst, uint16_t *xbar_a, uint16_t *xbar_b);
static int32_t loopback0_iterate(adi_apollo_device_t *device, uint16_t adc_select, uint16_t *overflow_status);
static int32_t loopback0_cleanup(adi_apollo_device_t *device);

int32_t loopback0(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    uint16_t overflow_status; // Which adcs are overflowed
    uint32_t run_cals = ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC | ADI_ADS10_APOLLO_CAL_ADC_BG;
    uint16_t side_a_mux[ADI_APOLLO_ADC_PER_SIDE_NUM / 2] = {ADI_APOLLO_ADC_0, ADI_APOLLO_ADC_1};
    uint16_t side_b_mux[ADI_APOLLO_ADC_PER_SIDE_NUM / 2] = {ADI_APOLLO_ADC_0, ADI_APOLLO_ADC_1};

    // Define iterate parameters - sweep of 4 configurations
    uint16_t adcs[TEST_CASES] = {ADI_APOLLO_ADC_A0, ADI_APOLLO_ADC_A1, ADI_APOLLO_ADC_B0, ADI_APOLLO_ADC_B1, ADI_APOLLO_ADC_A0 | ADI_APOLLO_ADC_A1 | ADI_APOLLO_ADC_B0 | ADI_APOLLO_ADC_B1, }; // Which adcs to enable

    err = loopback0_setup(device, ADI_APOLLO_ADC_ALL, 2, 2, &side_a_mux[0], &side_b_mux[0]);
    ADI_CMS_ERROR_GOTO(err, end);

    // Enable Tx-Rx lb0 before cal
    err = adi_apollo_loopback_lb0_tx_enable_set(device, ADI_APOLLO_ADC_ALL_4T4R, 1);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_apollo_loopback_lb0_rx_enable_set(device, ADI_APOLLO_SIDE_ALL, 1);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, run_cals);
    ADI_CMS_ERROR_GOTO(err, end);

    EXCTL_SIGGEN_ON(0x33, 0);

    for (int i = 0; i < TEST_CASES; i++) {

        // Iterate over test cases

        err = loopback0_iterate(device, adcs[i], &overflow_status);
        ADI_CMS_ERROR_GOTO(err, end);

        /* Define test cases for loopback 0 - this is arbitrary for this example.
        We check that the output frequency matches the input frequency at 2000MHz and 2100MHz
        and the output power changes appropriately with input power. Also ensure there is no
        output on DACs that are not enabled. */

        EXCTL_SIGGEN_FREQ(2000, 0);
        EXCTL_SIGGEN_LEVEL(6, 0);
        EXCTL_TX_MEAS_FREQ(adcs[i], 2000, 10, 0);
        EXCTL_TX_MEAS_NOSIG(adcs[i] ^ adcs[TEST_CASES - 1], interactive);

        EXCTL_SIGGEN_FREQ(2100, 0);
        EXCTL_TX_MEAS_FREQ(adcs[i], 2100, 10, 0);
        EXCTL_TX_REF(adcs[i], 2100, 0);
        EXCTL_TX_MEAS_NOSIG(adcs[i] ^ adcs[TEST_CASES - 1], interactive);

        EXCTL_SIGGEN_LEVEL(-6, 0);
        EXCTL_TX_MEAS_FREQ(adcs[i], 2100, 10, 0);
        EXCTL_TX_MEAS_NOSIG(adcs[i] ^ adcs[TEST_CASES - 1], 0);
        EXCTL_TX_MEAS_LEVEL(adcs[i], 2100, -6, .5, interactive);
    }

end:
    ADI_CMS_ERROR_RETURN(loopback0_cleanup(device));
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

    return API_CMS_ERROR_OK;
}

static int32_t loopback0_iterate(adi_apollo_device_t *device, uint16_t adc_select, uint16_t *overflow_status) {

    int32_t err;
    uint8_t i;
    uint16_t sides;
    uint16_t adc;
    uint8_t overflow_bool;

    // Get side(s) to enable
    err = adi_apollo_utils_side_from_adc_select_get(device, adc_select, &sides);
    ADI_CMS_ERROR_RETURN(err);

    // Make sure that adcs/sides that are not specified for this test case are disabled
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

    // Get and clear overflow status
    *overflow_status = ADI_APOLLO_ADC_NONE;

    for (i = 0; i < ADI_APOLLO_ADC_NUM; i++) {
        adc = adc_select & (ADI_APOLLO_ADC_A0 << i);
        if (adc) {
            err = adi_apollo_loopback_lb0_overflow_status_get(device, adc, &overflow_bool);
            ADI_CMS_ERROR_RETURN(err);

            if (overflow_bool) {
                *overflow_status = *overflow_status | adc;
            }
        }
    }

    return API_CMS_ERROR_OK;
}

static int32_t loopback0_cleanup(adi_apollo_device_t *device) {
    int32_t err;

    // Disable all
    err = adi_apollo_loopback_lb0_tx_enable_set(device, ADI_APOLLO_ADC_ALL, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb0_rx_enable_set(device, ADI_APOLLO_SIDE_ALL, 0);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

#endif /* defined(__linux__) */
