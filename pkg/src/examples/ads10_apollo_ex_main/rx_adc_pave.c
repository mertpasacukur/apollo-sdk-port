/*!
 * \brief     ADS10 Apollo Rx PFILT ADC averaging
 * 
 * The rx_pave example takes two ADC inputs and averages them together with a noise improvement of up to 3dB. The
 * PFILT ADC averaging block is used to average two input signals with an option for adding or subtracting. The
 * filter may be bypassed or engaged.
 * The ADC inputs are routed through the data path and out JTx to the FPGA where capture data is available.
 * Three files are used for results comparison. One contains the averaged data while the other two contain 
 * un-averaged data from the individual input channels. Compare the averaged vs un-averaged FFTs to see the noise improvement.
 * 
 * 
 * Setup Configuration (4T4R)
 * ==========================
 *   Device Profile: id00_uc06
 *   Fclk:      20000MHz
 *   FPGA Ref:  156.25MHz
 *   ADC Inputs
 *      Side A: ADC-A0 & ADC-A1     (set for -7dBFs)
 *      Side B: ADC-B0 & ADC-B1     (set for -7dBFs)
 *
 *  SigGen #1 -> split to A0/A1
 *  SigGen #2 -> split to B0/B1
 *
 *
 *  Capture File (I/Q interleaved)                                Contents
 *  ---------------------------------------------------------    ------------------------
 *  rx_adc_pave_L0_IQ0_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         Average ADC-A0 + ADC-A1
 *  rx_adc_pave_L0_IQ1_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         ADC-A1 un-averaged
 *  rx_adc_pave_L2_IQ0_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         Average ADC-B0 + ADC-B1
 *  rx_adc_pave_L2_IQ1_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         ADC-B1 un-averaged
 *
 *
 * Setup Configuration (8T8R)
 * ==========================
 *   Device Profile: id98_uc05
 *   Fclk:      8000MHz
 *   FPGA Ref:  250MHz
 *   ADC Inputs
 *      Side A: ADC-A0 & ADC-A2     (set for -7dBFs)
 *      Side B: ADC-B1 & ADC-B3     (set for -7dBFs)
 * 
 *  SigGen #1 -> split to A0/A2
 *  SigGen #2 -> split to B1/B3
 *  
 *
 *  Capture File (I/Q interleaved)                                Contents
 *  ---------------------------------------------------------    ------------------------
 *  rx_adc_pave_L0_IQ0_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         Average ADC-A0 + ADC-A2
 *  rx_adc_pave_L0_IQ4_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         ADC-A2 un-averaged
 *  rx_adc_pave_L0_IQ6_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         ADC-A0 un-averaged
 *  rx_adc_pave_L2_IQ0_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         Average ADC-B1 + ADC-B3
 *  rx_adc_pave_L2_IQ4_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         ADC-B3 un-averaged
 *  rx_adc_pave_L2_IQ6_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt         ADC-B1 un-averaged
 *
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_apollo.h"
#include "adi_fpga_apollo_core.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int32_t rx_mux1_config(adi_apollo_device_t *device);
static double coherent_freq(double target, double fdata, uint32_t n_samples);

int32_t rx_adc_pave(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    char file_name_base[256];
    double f_final_mhz;                     // Use CNCO and FNCO to shift input signal to this freq
    double fs_mhz;                          // ADC sampling rate
    double f_tone_mhz;                      // ADC input tone freq (real)
    double f_fnco_mhz;                      // Fine nco freq
    double f_cnco_mhz;                      // Coarse nco freq
    uint32_t cddc_dcm;                      // Coarse and fine decimation
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rx_dp_info;  // Rx datapath key info from profile
    uint32_t bgcal_dly = 15 * 1000000;      // Delay after starting bg cal (us)

    printf("\n*** PFILT ADC Averaging ***\n");

    /* Device block selects for 4t4r or 8t8r device */
    uint16_t cncos = (device->dev_info.is_8t8r) ? ADI_APOLLO_CNCO_ALL : ADI_APOLLO_CNCO_ALL_4T4R;
    uint16_t fncos = (device->dev_info.is_8t8r) ? ADI_APOLLO_FNCO_ALL : ADI_APOLLO_FNCO_ALL_4T4R;
    uint16_t pfilts = (device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;

    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Rx Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    /* Inspect and print the Apollo JTx link config */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    /* Get ADC sampling freq from profile */
    fs_mhz = rx_dp_info.adc_sample_rate;

    /* Determine the coarse and fine freq shifts from profile. This test assumes all chans are equal (i.e. use first) */
    cddc_dcm = rx_dp_info.cdrc;
    f_cnco_mhz = rx_dp_info.cnco_freq;
    f_fnco_mhz = rx_dp_info.fnco_freq;
    
    /*
     * Configure the CNCO/FNCO such that the resulting tone ends up at f_final_mhz. This is to keep it within the
     * band of the data rate.
     */
    f_final_mhz = -0.1 * rx_dp_info.fdata;
    f_final_mhz = coherent_freq(f_final_mhz, rx_dp_info.fdata, 64 * 1024);
    f_tone_mhz = f_cnco_mhz + f_fnco_mhz + f_final_mhz;
    printf("Ftone=%f, CNCO=%f, FNCO=%f\n", f_tone_mhz, f_cnco_mhz, f_fnco_mhz);

    /* Set the Rx CNCOs and FNCOs */
    adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_RX, cncos, fs_mhz, f_cnco_mhz);
    adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_RX, fncos, fs_mhz / cddc_dcm, f_fnco_mhz);

    /* Set the ADC analog input frequency */
    printf("@CMD : SET_ADC_INPUT_FREQ : %fMHz\n", f_tone_mhz);
    EXCTL_PAUSE(interactive);

    printf("Expect FFT Fundamental at: %fMHz\n", f_final_mhz);

    /* RX crossbar1, ADC to CB outs */
    err = rx_mux1_config(device);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Bypass the Rx filter (no FIR) */
    adi_apollo_pfilt_mode_enable_set(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);

    /* Set PFILT data type to real */
    adi_apollo_pfilt_data_type_set(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_REAL_DATA);

    /* Enable the PFILT ADC averaging block. Use subtraction if two ADC inputs have opposite polarity.
     * Note:
     *  ADI FMCA,B,C (4T4R) CE board has opposite input polarity for ADC-A0/A1 and ADC-B0/B1 (i.e. use SUB)
     *  ADI FMCC     (8T8R) CE board has same input polarity for all ADCs (i.e. use ADD)
     */
    adi_apollo_pfilt_ave_mode_set(device, pfilts,
                                  device->dev_info.is_8t8r ? ADI_APOLLO_PFILT_AVE_ENABLE_ADD : ADI_APOLLO_PFILT_AVE_ENABLE_SUB);

    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Let ADC background cal run a bit to lower noise floor */
    printf("Let ADC bgcal run for %4.1f secs...\n", (bgcal_dly / 1000000.0));
    adi_apollo_hal_delay_us(device, bgcal_dly);

    /* Read FPGA capture memory and write out i/q files - Capture data w/ averaging */
    sprintf(file_name_base, "%s", "rx_adc_pave");
    printf("Writing captures to files: %s_*\n", file_name_base);
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, 64 * 1024, file_name_base, true, true);
    ADI_CMS_ERROR_GOTO(err, end);

end:
    adi_apollo_device_reset(device, ADI_APOLLO_HARD_RESET);
    return err;
}

static double coherent_freq(double target, double fdata, uint32_t n_samples)
{
    int32_t  m_cycles = (target * n_samples) / fdata;
    if (m_cycles % 2 == 0) {
        m_cycles++;
    }
    return ((m_cycles * fdata) / n_samples);
}

/*
 * This function configures Rx MUX1 for averaging. Overrides device profile settings.
 * 
 * 4T4R
 *  Average: ADC-A0 & ADC-A1
 *  Average: ADC-B0 & ADC-B1
 *  
 * 8T8R
 *  Average: ADC-A0 & ADC-A2
 *  Average: ADC-B1 & ADC-B3
 *  
 * Note: 
 *  4T4R averages ADCs routed to CB0 & CB1
 *  8T8R averages ADCs routed to CB0 & CB1, CB2 & CB3
 *  
 *  CBx is crossbar out for side A or B
 */
static int32_t rx_mux1_config(adi_apollo_device_t *device)
{
    int32_t err = API_CMS_ERROR_OK;

    adi_apollo_rx_mux0_sel_e cbout_to_adc_4t4r_a[] = {ADI_APOLLO_4T4R_CB_OUT_0_FROM_ADC0, ADI_APOLLO_4T4R_CB_OUT_1_FROM_ADC1};  // Ave Pair: ADCA0->CB0 + ADCA1->CB1 (Side A)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_4t4r_b[] = {ADI_APOLLO_4T4R_CB_OUT_0_FROM_ADC0, ADI_APOLLO_4T4R_CB_OUT_1_FROM_ADC1};  // Ave Pair: ADCB0->CB0 + ADCB1->CB1 (Side B)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_8t8r_a[] = {ADI_APOLLO_8T8R_CB_OUT_FROM_ADC0, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC2,       // Ave Pair: ADCA0->CB0 + ADCA2->CB1
                                                      ADI_APOLLO_8T8R_CB_OUT_FROM_ADC2, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC0};      // Ave Pair: ADCA2->CB2 + ADCA0->CB3 (Side A)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_8t8r_b[] = {ADI_APOLLO_8T8R_CB_OUT_FROM_ADC1, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC3,       // Ave Pair: ADCB1->CB0 + ADCB3->CB1
                                                      ADI_APOLLO_8T8R_CB_OUT_FROM_ADC3, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC1};      // Ave Pair: ADCB3->CB2 + ADCB1->CB3 (Side B)

    if (device->dev_info.is_8t8r) {
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_A, cbout_to_adc_8t8r_a, ADI_APOLLO_RX_MUX0_NUM_8T8R);
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_B, cbout_to_adc_8t8r_b, ADI_APOLLO_RX_MUX0_NUM_8T8R);
    } else {
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_A, cbout_to_adc_4t4r_a, ADI_APOLLO_RX_MUX0_NUM_4T4R);
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_B, cbout_to_adc_4t4r_b, ADI_APOLLO_RX_MUX0_NUM_4T4R);
    }

    return (err == API_CMS_ERROR_OK ? API_CMS_ERROR_OK : API_CMS_ERROR_ERROR);
}
