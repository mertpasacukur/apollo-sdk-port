/*!
 * \brief     ADS10 Apollo BMEM delay test
 * 
 * The profile id00_uc06xA2 is designed so that data on ADC0 is sent
 * through two parallel datapaths, each of which can be individually configured. 
 * Inputs to the two Rx datapaths will be identical and differences in the capture
 * are only expected if the datapaths are configured differently.
 * 
 * In this example, delay is applied to the A0 datapath. The captures on B0/B1 should
 * be identical (same sample values) while the captures on A0/A1 will not be.
 * 
 * The `adi_bmem_hsdin_delay_` functions can be replaced with those for other BMEM
 * locations, i.e. `adi_bmem_cddc_delay_` and `adi_bmem_fddc_delay_`
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
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
#include "adi_apollo_loopback.h"
#include "adi_apollo_bmem.h"
#include "adi_apollo_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_apollo_trigts.h"
#include "adi_fpga_apollo_core.h"

int32_t rx_bmem_delay(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;

    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;             /* min num samples per virt conv */
    // Setup

    adi_apollo_bmem_delay_sample_t config = {0};
    config.sample_delay = 512;
    config.parity_check_en = 1;

    err = adi_apollo_bmem_hsdin_delay_sample_config(device, ADI_APOLLO_BMEM_A0, &config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_bmem_hsdin_delay_start(device, ADI_APOLLO_BMEM_A0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, "rx_bmem_delay", true);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}
