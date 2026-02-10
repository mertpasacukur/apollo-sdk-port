/*!
 * \brief     ADS10 Apollo Rx raw ADC BMEM capture
 *
 * The rx_adc_bmem example simply writes ADC samples to real capture files using BMEM. No JESD or
 * Apollo datapath blocks are used. Both 4T4R and 8T8R devices are supported.
 * 
 * Connect the ADC inputs to a signal generator, set level for aprox -7.0dBFS
 * 
 * Device profiles
 *     4T4R: id00_uc06      (Fadc = 20G)
 *     4T4R: id00_uc02_8G   (Fadc = 8G)
 * 
 * Output Files:
 *      rx_adc_bmem_a0_00.txt
 *      rx_adc_bmem_a1_00.txt
 *      rx_adc_bmem_a2_00.txt   (8T8R only)
 *      rx_adc_bmem_a3_00.txt   (8T8R only)
 *      rx_adc_bmem_b0_00.txt
 *      rx_adc_bmem_b1_00.txt
 *      rx_adc_bmem_b2_00.txt   (8T8R only)
 *      rx_adc_bmem_b3_00.txt   (8T8R only)
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
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
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_types.h"
#include "ads10_fpga.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_bmem.h"
#include "adi_ads10_apollo_ex_ctl.h"

#define INPUT_FREQ_MHZ 2300

static char *compose_fname(char *base, int n, char *buff);
static int32_t rx_mux1_config(adi_apollo_device_t *device);

int32_t rx_adc_bmem(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rx_dp_info;
    uint32_t cap_dly = 10 * 1000000;   /* Delay between captures (us) */
    uint32_t n_cap_loops = 3;         /* Number of captures to acquire. New files per capture iteration */
    uint32_t nyquist_zone = 1;         /* Nyquist zone for input tone based on sample rate 1:odd, 2:even */

    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, 0, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Rx Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    /*
     * Set Mux1 for standard mapping
     * 
     *  4T4R
     *      ADC-A0 to CBout0 -> ADI_APOLLO_BMEM_A0
     *      ADC-A1 to CBout1 -> ADI_APOLLO_BMEM_A1
     *      
     *      ADC-B0 to CBout0 -> ADI_APOLLO_BMEM_B0
     *      ADC-B1 to CBout0 -> ADI_APOLLO_BMEM_B1
     *      
     *  8T8R
     *      ADC-A0 to CBout0 -> ADI_APOLLO_BMEM_A0
     *      ADC-A2 to CBout2 -> ADI_APOLLO_BMEM_A2
     *      ADC-A1 to CBout1 -> ADI_APOLLO_BMEM_A1
     *      ADC-A3 to CBout3 -> ADI_APOLLO_BMEM_A3
     *      
     *      ADC-B0 to CBout0 -> ADI_APOLLO_BMEM_B0
     *      ADC-B2 to CBout2 -> ADI_APOLLO_BMEM_B2
     *      ADC-B1 to CBout1 -> ADI_APOLLO_BMEM_B1
     *      ADC-B3 to CBout3 -> ADI_APOLLO_BMEM_B3
     */
    err = rx_mux1_config(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the Nyquist zone for the input tone */
    err = adi_apollo_adc_nyquist_zone_set(device, device->dev_info.is_8t8r ? ADI_APOLLO_ADC_ALL : ADI_APOLLO_ADC_ALL_4T4R, nyquist_zone);
    ADI_CMS_ERROR_RETURN(err);

    /*
     * Configure BMEM capture
    */
    adi_apollo_bmem_capture_t bmem_config = {
        .sample_size = 0,
        .ramclk_ph_dis = 0,
        .st_addr_cpt = 0,
        .end_addr_cpt = ADI_APOLLO_BMEM_HSDIN_MEM_SIZE_WORDS - 1,
        .parity_check_en = 1
    };
    err = adi_apollo_bmem_hsdin_capture_config(device, ADI_APOLLO_BMEM_ALL, &bmem_config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    EXCTL_SIGGEN_FREQ(INPUT_FREQ_MHZ, interactive);

    for (int i = 0; i < n_cap_loops; i++) {
        printf("Delay %4.1f secs\n", cap_dly / 1000000.0);
        adi_apollo_hal_delay_us(device, cap_dly);

        err = adi_ads10_apollo_ex_bmem_capture(device, ADI_APOLLO_BMEM_ALL, compose_fname("rx_adc_bmem", i, str_buff));
        ADI_CMS_ERROR_RETURN(err);

        EXCTL_RX_MEAS_FREQ(0x33, str_buff, INPUT_FREQ_MHZ, 10);
    }

    return err;
}

static char *compose_fname(char *base, int n, char *buff)
{
    sprintf(buff, "%s_%02d", base, n);
    return buff;
}

/*
 * This function configures Rx MUX1 for standard ADC to BMEM mapping
*/
static int32_t rx_mux1_config(adi_apollo_device_t *device)
{
    int32_t err = API_CMS_ERROR_OK;

    adi_apollo_rx_mux0_sel_e cbout_to_adc_4t4r_a[] = {ADI_APOLLO_4T4R_CB_OUT_0_FROM_ADC0, ADI_APOLLO_4T4R_CB_OUT_1_FROM_ADC1}; // Ave Pair: ADCA0->CB0 + ADCA1->CB1 (Side A)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_4t4r_b[] = {ADI_APOLLO_4T4R_CB_OUT_0_FROM_ADC0, ADI_APOLLO_4T4R_CB_OUT_1_FROM_ADC1}; // Ave Pair: ADCB0->CB0 + ADCB1->CB1 (Side B)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_8t8r_a[] = {ADI_APOLLO_8T8R_CB_OUT_FROM_ADC0, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC1,      // Ave Pair: ADCA0->CB0 + ADCA1->CB1
                                                      ADI_APOLLO_8T8R_CB_OUT_FROM_ADC2, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC3};     // Ave Pair: ADCA2->CB2 + ADCA3->CB3 (Side A)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_8t8r_b[] = {ADI_APOLLO_8T8R_CB_OUT_FROM_ADC0, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC1,      // Ave Pair: ADCB0->CB0 + ADCB1->CB1
                                                      ADI_APOLLO_8T8R_CB_OUT_FROM_ADC2, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC3};     // Ave Pair: ADCB2->CB2 + ADCB3->CB3 (Side B)

    if (device->dev_info.is_8t8r) {
        err = adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_A, cbout_to_adc_8t8r_a, ADI_APOLLO_RX_MUX0_NUM_8T8R);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_B, cbout_to_adc_8t8r_b, ADI_APOLLO_RX_MUX0_NUM_8T8R);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_A, cbout_to_adc_4t4r_a, ADI_APOLLO_RX_MUX0_NUM_4T4R);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_B, cbout_to_adc_4t4r_b, ADI_APOLLO_RX_MUX0_NUM_4T4R);
        ADI_CMS_ERROR_RETURN(err);
    }
    
    return err;
}
