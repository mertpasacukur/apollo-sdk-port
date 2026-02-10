#if defined(__linux__)

/*!
 * \brief     ADS10 Apollo ADC mode switching
 * 
 * Example will capture with default mode, then switch and capture three times. 
 * The switches will be executed by firmware (slowest), regmap (slow) and GPIO (fast)
 *     
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
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
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_bmem.h"

int32_t rx_adc_ms(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err;
    int32_t adc_cal_chans = ADI_APOLLO_ADC_A0 | ADI_APOLLO_ADC_A1 | ADI_APOLLO_ADC_B0 | ADI_APOLLO_ADC_B1;
    int32_t bmem = ADI_APOLLO_BMEM_A0 | ADI_APOLLO_BMEM_A1 | ADI_APOLLO_BMEM_B0 | ADI_APOLLO_BMEM_B1;

    // Configure GPIO to control ADC mode switching
    err = adi_apollo_gpio_quick_config_mode_set(device, ADI_APOLLO_QUICK_CFG_DISABLE);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, 0, ADI_APOLLO_FUNC_ADC_MODE_SWITCH);
    ADI_CMS_ERROR_RETURN(err);

    // Configure BMEM capture
    adi_apollo_bmem_capture_t bmem_config = {
        .sample_size = 0,
        .ramclk_ph_dis = 0,
        .st_addr_cpt = 0,
        .end_addr_cpt = ADI_APOLLO_BMEM_HSDIN_MEM_SIZE_WORDS - 1,
        .parity_check_en = 1
    };
    err = adi_apollo_bmem_hsdin_capture_config(device, bmem, &bmem_config);
    ADI_CMS_ERROR_RETURN(err);

    // Run Cals
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC | ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    // Capture with default mode
    err = adi_ads10_apollo_ex_inspect_adc_all(device);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_bmem_capture(device, ADI_APOLLO_BMEM_A0, "default");
    ADI_CMS_ERROR_RETURN(err);

    // Switch via FW
    err = adi_apollo_adc_mode_switch_prepare(device, adc_cal_chans);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_adc_mode_switch_execute(device, adc_cal_chans, ADI_APOLLO_ADC_MODE_SWITCH_BY_COMMAND);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_adc_mode_switch_restore(device, adc_cal_chans, true);
    ADI_CMS_ERROR_RETURN(err);

    // Capture with switched mode
    err = adi_ads10_apollo_ex_inspect_adc_all(device);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_bmem_capture(device, ADI_APOLLO_BMEM_A0, "fw_switch");
    ADI_CMS_ERROR_RETURN(err);

    // Switch via regmap
    err = adi_apollo_adc_mode_switch_prepare(device, adc_cal_chans);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_adc_mode_switch_execute(device, adc_cal_chans, ADI_APOLLO_ADC_MODE_SWITCH_BY_REGMAP);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_adc_mode_switch_restore(device, adc_cal_chans, true);
    ADI_CMS_ERROR_RETURN(err);

    // Capture with default mode
    err = adi_ads10_apollo_ex_inspect_adc_all(device);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_bmem_capture(device, ADI_APOLLO_BMEM_A0, "regmap_switch");
    ADI_CMS_ERROR_RETURN(err);

    // Switch via GPIOs
    err = adi_apollo_adc_mode_switch_prepare(device, adc_cal_chans);
    ADI_CMS_ERROR_RETURN(err);
    adi_fpga_apollo_gpio_output_set(fpga_device, 0, 0);
    adi_fpga_apollo_gpio_output_set(fpga_device, 0, 1);
    adi_fpga_apollo_gpio_output_set(fpga_device, 0, 0);
    err = adi_apollo_adc_mode_switch_restore(device, adc_cal_chans, true);
    ADI_CMS_ERROR_RETURN(err);

    // Capture with switched mode
    err = adi_ads10_apollo_ex_inspect_adc_all(device);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_bmem_capture(device, ADI_APOLLO_BMEM_A0, "gpio_switch");
    ADI_CMS_ERROR_RETURN(err);

    return err;

}

#endif /* defined(__linux__) */
