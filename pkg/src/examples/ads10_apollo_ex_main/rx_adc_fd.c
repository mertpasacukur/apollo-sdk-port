#if defined(__linux__)

/*!
 * \brief     ADS10 Apollo Rx ADC Fast detect
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 * 
 * This example demonstrates enabling fast detect. Fast detect is programmed for
 * all ADCs, after which a loop is entered. On 'Enter',
 * the state of Apollo's fast detect pins are reported. Loop is exited when any 
 * key is input.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "adi_apollo.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_ads10_apollo_ex_cal.h"

#define GPIO_OFFSET 11

static uint32_t calculate_fd_threshold_magnitude(double dbfs);

int32_t rx_adc_fd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
	int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    uint16_t adc_cal_chans = ADI_APOLLO_ADC_A0 | ADI_APOLLO_ADC_A1 | ADI_APOLLO_ADC_B0 | ADI_APOLLO_ADC_B1;
    uint8_t gpio_read, fw_read;
    bool disable_quick_config = true;
    uint16_t adcs[] = {ADI_APOLLO_ADC_A0, ADI_APOLLO_ADC_A1, ADI_APOLLO_ADC_B0, ADI_APOLLO_ADC_B1};

    adi_apollo_adc_fast_detect_pgm_t fd_config = {
        .enable = 1,
        .dwell_cycles = (device->dev_info.is_8t8r == 1) ? 8 : 16,
        .lower_threshold = calculate_fd_threshold_magnitude(-17),
        .upper_threshold = calculate_fd_threshold_magnitude(-14),
    };

    printf("Configure fast detect\n");
    err = adi_apollo_adc_fast_detect_pgm(device, adc_cal_chans, &fd_config);
    ADI_CMS_ERROR_RETURN(err);

    if (disable_quick_config) {
        printf("Disabling quick config...\n");
        err = adi_apollo_gpio_quick_config_mode_set(device, 0);
    	ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, GPIO_OFFSET + 0, ADI_APOLLO_FUNC_ADC_FD_A0);
	    ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, GPIO_OFFSET + 1, ADI_APOLLO_FUNC_ADC_FD_A1);
	    ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, GPIO_OFFSET + 2, ADI_APOLLO_FUNC_ADC_FD_B0);
	    ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, GPIO_OFFSET + 3, ADI_APOLLO_FUNC_ADC_FD_B1);
	    ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    do {
        printf("\nFD FW GPIO\n");
        for (int i = 0; i < 4; i++) {
            err = adi_fpga_apollo_gpio_input_get(fpga_device, GPIO_OFFSET + i, &gpio_read);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_apollo_adc_fast_detect_status_get(device, adcs[i], &fw_read);
	        ADI_CMS_ERROR_RETURN(err);
            printf("%2d %2d %2d\n", i, fw_read, gpio_read);
        }

        if (interactive) {
            printf("Press enter to check fast detect, anything else to end.\n");
        }
    } while (interactive && (getchar() == '\n'));

	return err;
}

static uint32_t calculate_fd_threshold_magnitude(double dbfs) {
    return (int)(pow(10.0, (dbfs / 20)) * (1 << 11));
}


#endif /* defined(__linux__) */
