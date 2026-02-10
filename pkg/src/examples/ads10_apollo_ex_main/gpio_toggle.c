/*!
 * \brief     Apollo CMOS GPIO toggle example running on ADS10 platform
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__linux__)
#include <unistd.h>
#endif
#include <math.h>
#include "adi_apollo.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_utils.h"

static int32_t apollo_cmos_gpio_output_toggle(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t gpio_index);
static int32_t apollo_cmos_gpio_input_read(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t gpio_index);
static int32_t ads10_syncpad_gpio_sel(adi_fpga_apollo_device_t *fpga_device, uint8_t pad_index);
static int32_t num_gpios_to_test_get(adi_fpga_apollo_device_t* fpga_device, uint8_t* num_gpios);

int32_t gpio_toggle(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                     int argc, char *argv[], int argc_ofst) {

    int32_t err;

    uint8_t gpio_index = 0x00;
    uint8_t num_gpios_to_test;

   /* Get the number of GPIOs to test based on FPGA support */
    err = num_gpios_to_test_get(fpga_device, &num_gpios_to_test);
    ADI_CMS_ERROR_RETURN(err)

    /* Test Function 1: FPGA GPIO Toggles every 50 ms.
	 * FPGA GPIO set as OUTPUT, connected Apollo GPIO set as INPUT. */
    printf("Running Apollo GPIO read test...\n");
    for (gpio_index = 0; gpio_index < num_gpios_to_test; ++gpio_index) {
        err = apollo_cmos_gpio_input_read(device, fpga_device, gpio_index);
        if (err != API_CMS_ERROR_OK) {
            break;
        }
    }
    printf("Apollo GPIO Read Test:  %s.\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    if (err != API_CMS_ERROR_OK) {
        printf("Test FAILED for gpio_index:  %d.\n", gpio_index);
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Test Function 2: Apollo GPIO Toggles every 50 ms.
	 * Apollo GPIOs set as OUTPUT, connected FPGA GPIOs set as INPUT. */
    printf("Running Apollo GPIO write test...\n");
    for (gpio_index = 0; gpio_index < num_gpios_to_test; ++gpio_index) {
        err = apollo_cmos_gpio_output_toggle(device, fpga_device, gpio_index);
        if (err != API_CMS_ERROR_OK) {
            break;
        }
    }
    printf("Apollo GPIO Toggle Write Test: %s.\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    if (err != API_CMS_ERROR_OK) {
        printf("Test FAILED for gpio_index:  %d.\n", gpio_index);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

/**
 * Configure Apollo CMOS GPIOs [0:30] or SYNC PADS [31:46] as CMOS GPIOs, OUTPUT and Toggle the pin every 50 ms.
 * Corresponding, connected FPGA GPIO set as Input and read the pin state.
 */
static int32_t apollo_cmos_gpio_output_toggle(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t gpio_index)
{
    int32_t err = API_CMS_ERROR_TEST_FAILED;
	uint8_t num = 0x00;
    uint8_t gpio_state = 0x00;

    ADI_CMS_INVALID_PARAM_CHECK(gpio_index > (ADI_FPGA_GPIO_NUM - 1));

    // For SYNCPADs, Connect the Apollo SYNCPAD GPIO to corresponding FPGA CMOS GPIO.
    if ((gpio_index >= ADI_APOLLO_SYNCINB0_B) && (gpio_index <= (ADI_APOLLO_SYNCOUTB1_A + 1))) {
        err = ads10_syncpad_gpio_sel(fpga_device, gpio_index);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Configure the provided gpio_index as CMOS, Enable the CMOS Mode and set direction as OUTPUT.
    err = adi_apollo_gpio_cmos_gpio_mode_set(device, gpio_index, ADI_APOLLO_GPIO_DIR_OUTPUT);
    ADI_CMS_ERROR_RETURN(err);

    // Toggle Apollo GPIO which is configured as OUTPUT, while reading the GPIO state from FPGA.
    for (uint8_t i = 0; i < 10; ++i) {
        num ^= 0x01;
        num &= 0x01;

        // Set Apollo GPIO.
        err = adi_apollo_gpio_cmos_output_set(device, gpio_index, num);
        ADI_CMS_ERROR_RETURN(err);

        // Get FPGA GPIO state.
        err = adi_fpga_apollo_gpio_input_get(fpga_device, gpio_index, &gpio_state);
        ADI_CMS_ERROR_RETURN(err);

        gpio_state &= 0x01;
        adi_apollo_hal_delay_us(device, 50000);

        ADI_CMS_CHECK(num != gpio_state, API_CMS_ERROR_TEST_FAILED);
    }
    return err;
}

/**
 * Configure Apollo CMOS GPIOs [0:30] or SYNC PADS [31:46] as CMOS GPIOs, INPUT and read the pin state.
 * Corresponding, connected FPGA GPIO set as OUTPUT and Toggles every 50 ms.
 */
static int32_t apollo_cmos_gpio_input_read(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t gpio_index)
{
    int32_t err = API_CMS_ERROR_TEST_FAILED;
	uint8_t num = 0x01;
    uint8_t gpio_state = 0x00;

	ADI_CMS_INVALID_PARAM_CHECK(gpio_index > (ADI_FPGA_GPIO_NUM - 1));

    // For SYNCPADs, Connect the Apollo SYNCPAD GPIO to corresponding FPGA CMOS GPIO.
    if ((gpio_index >= ADI_APOLLO_SYNCINB0_B) && (gpio_index <= (ADI_APOLLO_SYNCOUTB1_A + 1))) {
        err = ads10_syncpad_gpio_sel(fpga_device, gpio_index);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Configure the provided gpio_index as CMOS, Enable the CMOS Mode and set direction as INPUT.
    err = adi_apollo_gpio_cmos_gpio_mode_set(device, gpio_index, ADI_APOLLO_GPIO_DIR_INPUT);
    ADI_CMS_ERROR_RETURN(err);

    // Toggle FPGA GPIO which is configured as OUTPUT, while reading the GPIO state from Apollo.
    for (uint8_t i = 0; i < 10; ++i) {
        num ^= 0x01;
        num &= 0x01;

        // Set FPGA GPIO.
        err = adi_fpga_apollo_gpio_output_set(fpga_device, gpio_index, num);
        ADI_CMS_ERROR_RETURN(err);
        // Get Apollo GPIO state.
        err = adi_apollo_gpio_cmos_input_get(device, gpio_index, &gpio_state);
        ADI_CMS_ERROR_RETURN(err);

        gpio_state &= 0x01;
        adi_apollo_hal_delay_us(device, 50000);

        ADI_CMS_CHECK(num != gpio_state, API_CMS_ERROR_TEST_FAILED);
    }
    return err;
}

static int32_t ads10_syncpad_gpio_sel(adi_fpga_apollo_device_t *fpga_device, uint8_t pad_index)
{
    int32_t err = API_CMS_ERROR_TEST_FAILED;

    // SYNCIN PAD index range.
    if ((pad_index >= ADI_APOLLO_SYNCINB0_B) && (pad_index <= (ADI_APOLLO_SYNCINB1_A + 1))) {

        // Switch Selection for sides A0/B0 or A1/B1 on Evaluation board.
        uint8_t fpga_syncin_link = 0;

        switch (pad_index) {
            case (ADI_APOLLO_SYNCINB0_B):
            case (ADI_APOLLO_SYNCINB0_B + 1):
            case (ADI_APOLLO_SYNCINB0_A):
            case (ADI_APOLLO_SYNCINB0_A + 1):
                fpga_syncin_link = ADI_FPGA_SYNCINB_LINK_AB_0;
                break;

            case (ADI_APOLLO_SYNCINB1_B):
            case (ADI_APOLLO_SYNCINB1_B + 1):
            case (ADI_APOLLO_SYNCINB1_A):
            case (ADI_APOLLO_SYNCINB1_A + 1):
                fpga_syncin_link = ADI_FPGA_SYNCINB_LINK_AB_1;
                break;

            default:
                ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
        }

        // Connect the Apollo SYNCIN PAD to FPGA CMOS GPIO.
        err = adi_fpga_apollo_syncinb_ab_sel(fpga_device, ADI_FPGA_SYNCINB_GPIO_FMC, fpga_syncin_link);
        ADI_CMS_ERROR_RETURN(err);
    }

    // SYNCOUT PAD index range.
    else if ((pad_index >= ADI_APOLLO_SYNCOUTB0_B) && (pad_index <= (ADI_APOLLO_SYNCOUTB1_A + 1))) {

        // printf("Indexed GPIO %d is SYNCOUT PAD configured as CMOS pins!!\n", pad_index);

        // Switch Selection for sides A0/B0 or A1/B1 on Evaluation board.
        uint8_t fpga_syncout_link = 0;

        switch (pad_index) {
            case (ADI_APOLLO_SYNCOUTB0_B):
            case (ADI_APOLLO_SYNCOUTB0_B + 1):
            case (ADI_APOLLO_SYNCOUTB0_A):
            case (ADI_APOLLO_SYNCOUTB0_A + 1):
                fpga_syncout_link = ADI_FPGA_SYNCOUTB_LINK_AB_0;
                break;

            case (ADI_APOLLO_SYNCOUTB1_B):
            case (ADI_APOLLO_SYNCOUTB1_B + 1):
            case (ADI_APOLLO_SYNCOUTB1_A):
            case (ADI_APOLLO_SYNCOUTB1_A + 1):
                fpga_syncout_link = ADI_FPGA_SYNCOUTB_LINK_AB_1;
                break;

            default:
                ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
        }

        // Connect the Apollo SYNCOUT PAD to FPGA CMOS GPIO.
        err = adi_fpga_apollo_syncoutb_ab_sel(fpga_device, ADI_FPGA_SYNCOUTB_GPIO_FMC, fpga_syncout_link);
        ADI_CMS_ERROR_RETURN(err);
    }

    // index out of SYNCPAD range.
    else {
        printf("Indexed pad %d is not a Valid SYNCPAD!!\n", pad_index);
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
    }

    return err;
}

static int32_t num_gpios_to_test_get(adi_fpga_apollo_device_t* fpga_device, uint8_t* num_gpios)
{
    int32_t err;

    // Feature flags indicate FPGA support for HW FSRC and TPL
    // Images that have HW FSRC don't fully support SYNCPAD GPIOs
    adi_fpga_feature_flag_t fpga_feature_flags;
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    *num_gpios = (fpga_feature_flags.tx_hw_fsrc ? ADI_APOLLO_NUM_CMOS_GPIO : ADI_APOLLO_NUM_GPIO);

    return API_CMS_ERROR_OK;
}
