#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo FFT Sniffer test
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
#include "adi_apollo_gpio.h"
#include "adi_apollo_sniffer.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_inspect.h"

#define TEST_CASES 2
#define FFT_EN_GPIO 12
#define FFT_HOLD_GPIO 13
#define FFT_DONE_GPIO 14

static int32_t rx_sniffer_setup(adi_apollo_device_t *device, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_init_t *config);
static int32_t rx_sniffer_iterate(adi_apollo_device_t *device, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_param_t *config, adi_apollo_sniffer_fft_data_t *fft_data);
static int32_t rx_sniffer_gpio_iterate(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_param_t *config, adi_apollo_sniffer_fft_data_t *fft_data);
static int32_t rx_sniffer_cleanup(adi_apollo_device_t *device, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_param_t *config);

static int32_t rx_sniffer_populate_default_params(adi_apollo_sniffer_mode_e mode, adi_apollo_sniffer_param_t *config);
static int32_t rx_sniffer_print_fft_data(adi_apollo_device_t *device, adi_apollo_sniffer_fft_data_t data);
static int32_t rx_sniffer_bin_frequencies_get(adi_apollo_device_t *device, int bin, double *low, double * high);

static char *test_mode_names[4] = {"Normal Magnitude", "Instant Magnitude", "Normal IQ", "Instant IQ"};

int32_t rx_sniffer(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    bool use_gpios = false;

    adi_apollo_sniffer_param_t sniffer_config;
    adi_apollo_sniffer_fft_data_t fft_data;
    adi_apollo_side_select_e side = ADI_APOLLO_SIDE_A;

    // Test each of the 4 modes
    adi_apollo_sniffer_mode_e test_modes[4] = {ADI_APOLLO_SNIFFER_INSTANT_MAGNITUDE, ADI_APOLLO_SNIFFER_NORMAL_MAGNITUDE, ADI_APOLLO_SNIFFER_INSTANT_IQ, ADI_APOLLO_SNIFFER_NORMAL_IQ};

    // Get default parameters for initialization
    rx_sniffer_populate_default_params(test_modes[0], &sniffer_config);

    if (use_gpios) {
        adi_apollo_gpio_quick_config_mode_set(device, ADI_APOLLO_QUICK_CFG_DISABLE);
        adi_apollo_gpio_cmos_func_mode_set(device, FFT_EN_GPIO, ADI_APOLLO_FUNC_FFT_ENABLE_A);
        adi_apollo_gpio_cmos_func_mode_set(device, FFT_HOLD_GPIO, ADI_APOLLO_FUNC_FFT_HOLD_A);
        adi_apollo_gpio_cmos_func_mode_set(device, FFT_DONE_GPIO, ADI_APOLLO_FUNC_FFT_DONE_A);
        sniffer_config.init.fft_enable_sel = ADI_APOLLO_SNIFFER_GPIO_INPUT;
        sniffer_config.init.fft_hold_sel = ADI_APOLLO_SNIFFER_GPIO_INPUT;
    }

    // Call setup
    err = rx_sniffer_setup(device, side, &sniffer_config.init);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_GOTO(err, end);

    // Iterate over test cases
    for (int i = 0; i < TEST_CASES; i++) {

        do {
        
            // Populate config per test mode
            rx_sniffer_populate_default_params(test_modes[i], &sniffer_config);

            // Run the iterator
            if (use_gpios) {
                err = rx_sniffer_gpio_iterate(device, fpga_device, side, &sniffer_config, &fft_data);
                ADI_CMS_ERROR_GOTO(err, end);
            } else {
                err = rx_sniffer_iterate(device, side, &sniffer_config, &fft_data);
                ADI_CMS_ERROR_GOTO(err, end);
            }

            // Print data to user and prompt input
            rx_sniffer_print_fft_data(device, fft_data);

            if (interactive) {
                printf("\nEnter to capture for test mode %s [%d], anything else to move to next test mode.\n",
                       test_mode_names[test_modes[i]], test_modes[i]);
            }
            
        } while (interactive && (getchar() == '\n'));
                  
    }

end:

    // Cleanup
    err = rx_sniffer_cleanup(device, side, &sniffer_config);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t rx_sniffer_setup(adi_apollo_device_t *device, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_init_t *config)
{
    int32_t err;

    /* Init sniffer */
    err = adi_apollo_sniffer_init(device, side_sel, config);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;

}

static int32_t rx_sniffer_iterate(adi_apollo_device_t *device, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_param_t *config, adi_apollo_sniffer_fft_data_t *fft_data)
{

    int32_t err;

    /* Program the sniffer */
    err = adi_apollo_sniffer_pgm(device, side_sel, &config->pgm);
    ADI_CMS_ERROR_RETURN(err);

    /* Get fft data*/
    err = adi_apollo_sniffer_data_get(device, side_sel, config, fft_data);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

static int32_t rx_sniffer_gpio_iterate(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_param_t *config, adi_apollo_sniffer_fft_data_t *fft_data)
{

    int32_t err;
    uint8_t fft_done = 0;

    /* Program the sniffer */
    err = adi_apollo_sniffer_pgm(device, side_sel, &config->pgm);
    ADI_CMS_ERROR_RETURN(err);

    // Set FFT enable high
    adi_fpga_apollo_gpio_output_set(fpga, FFT_EN_GPIO, 1);

    // Set FFT hold low
    adi_fpga_apollo_gpio_output_set(fpga, FFT_HOLD_GPIO, 0);

    // Wait for fft_done to go high
    while(fft_done == 0) {
        adi_fpga_apollo_gpio_input_get(fpga, FFT_DONE_GPIO, &fft_done);
        device->hal_info.delay_us(device, 1);
    }

    // Disable fft engine
    if (! config->read.run_fft_engine_background) {
        adi_fpga_apollo_gpio_output_set(fpga, FFT_EN_GPIO, 0);
    }

    // Hold fft
    adi_fpga_apollo_gpio_output_set(fpga, FFT_HOLD_GPIO, 1);

    err = adi_apollo_sniffer_fft_data_get(device, side_sel, config, fft_data);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

static int32_t rx_sniffer_cleanup(adi_apollo_device_t *device, adi_apollo_side_select_e side_sel, adi_apollo_sniffer_param_t *config)
{
    int32_t err;
    /* Disable sniffer */
    /* Get fft data*/
    err = adi_apollo_sniffer_enable_set(device, side_sel, 0);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t rx_sniffer_populate_default_params(adi_apollo_sniffer_mode_e mode, adi_apollo_sniffer_param_t *config)
{

    config->init.fft_hold_sel = 1;                 // 0 - gpio 1 - regmap
    config->init.fft_enable_sel = 1;               // 0 - gpio 1 - regmap
    config->init.real_mode = 1;                    // 1 real 0 complex
    config->init.max_threshold = 20;                // Max threshold for max
    config->init.min_threshold = 0;                // Min threshold for min
    config->init.sniffer_enable = 1;               // Enable spec sniffer

    config->pgm.sniffer_mode = mode;               // see \ref adi_apollo_sniffer_mode_e
    config->pgm.sort_enable = 1;                   // 1 enable 0 disable
    config->pgm.continuous_mode = 0;               // 1 continuous 0 single
    config->pgm.bottom_fft_enable = 0;             // 1 enable 0 disable
    config->pgm.window_enable = 0;                 // 1 enable 0 disable
    config->pgm.low_power_enable = 0;              // 1 enable 0 disable
    config->pgm.dither_enable = 0;                 // 1 enable 0 disable
    config->pgm.alpha_factor = 0;                  // exp. avg. 0 - disable 1-8 - enable
    config->pgm.adc = ADI_APOLLO_ADC_0;

    config->read.run_fft_engine_background = 0;
    config->read.timeout_us = 1000;

    if (mode > ADI_APOLLO_SNIFFER_INSTANT_MAGNITUDE) { // IQ mode necessities
        config->init.real_mode = 0;

        config->pgm.sort_enable = 0;
        config->pgm.continuous_mode = 0;
        config->pgm.alpha_factor = 0;
    }

    return API_CMS_ERROR_OK;
}


static int32_t rx_sniffer_print_fft_data(adi_apollo_device_t *device, adi_apollo_sniffer_fft_data_t data)
{
    int i;
    double low_bound, high_bound;
    char low_str[8], high_str[8];

    printf("FFT: %d points in from sniffer mode 0x%x\n", data.valid_data_length, data.data_mode);

    switch (data.data_mode) {
    
        /* Magnitude mode */
        case ADI_APOLLO_SNIFFER_INSTANT_MAGNITUDE:
        case ADI_APOLLO_SNIFFER_NORMAL_MAGNITUDE:

            for (i = 0; i < data.valid_data_length; i++){
                rx_sniffer_bin_frequencies_get(device, data.bin_q_data[i], &low_bound, &high_bound);
                snprintf(low_str, 8, "%.1f", low_bound);
                snprintf(high_str, 8, "%.1f", high_bound);
                printf("\tBin %3d [%8sMHz - %8sMHz]; Mag: %3d; %s%s\n", data.bin_q_data[i], low_str, high_str, data.mag_i_data[i], data.bin_below_threshold[i] ? "below threshold" : "", data.bin_above_threshold[i] ? "above threshold" : "");
            }
            break;

        /* IQ mode*/
        case ADI_APOLLO_SNIFFER_INSTANT_IQ:
        case ADI_APOLLO_SNIFFER_NORMAL_IQ:
            for (i = 0; i < data.valid_data_length; i++) {
                rx_sniffer_bin_frequencies_get(device, i, &low_bound, &high_bound);
                snprintf(low_str, 8, "%.1f", low_bound);
                snprintf(high_str, 8, "%.1f", high_bound);
                printf("\tBin %3d  [%8sMHz - %8sMHz]; I: %3d, Q: %3d;\n", i, low_str, high_str, data.mag_i_data[i], data.bin_q_data[i]);
            }
            break;
        
        /* Anything else */
        default:
            return API_CMS_ERROR_INVALID_PARAM;
    }

    printf("%s %s FFT\n", test_mode_names[data.data_mode], data.fft_is_complex ? "Complex" : "Real");

    return API_CMS_ERROR_OK;
}

static int32_t rx_sniffer_bin_frequencies_get(adi_apollo_device_t *device, int bin, double *low, double * high){
    double bin_width = (float) device->dev_info.dev_freq_hz / (ADI_APOLLO_SNIFFER_FFT_LENGTH * 1e6);

    bin = bin < 256 ? bin : bin - 512;

    *low = ((float)bin - .5) * bin_width;
    *high = *low + bin_width;

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
