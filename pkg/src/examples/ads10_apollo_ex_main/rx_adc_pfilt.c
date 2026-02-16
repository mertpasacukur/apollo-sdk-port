#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo Rx PFILT data path test using ADCs as input source
 *
 *  This example uses ADCs to exercise the PFILT block.
 *  Side A is configured for a low pass filter while side B is configured for high pass.
 *  Tones are generated at 0.115, 0.3 and 0.399 Fs (pass/transition/stop bands). Data is 
 *  captured for all channels and written out to interleaved I/Q files. Use FFT analysis 
 *  and compare fundamental magnitudes to verify filter response.
 *  
 * This example supports 4T4R and 8TR devices.
 *  
 * Example filter response for N/2 real filter config
 *  Fin	    Low Pass    Hi Pass
 *  ---     --------    -------
 *  0.115   0           <-55
 *  0.3     -8.5        -8.5
 *  0.39    <-55        0
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
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_types.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_ctl.h"


static double coherent_freq(double target, double fdata, uint32_t n_samples);

int32_t rx_adc_pfilt(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    char file_name_base[256];
    double tone_ratio[] = {0.115, 0.3, 0.399};  // Test input tones: pass, transition and stop bands. e.g. Freq = Fs*tone_ratio
    double final_tone_ratio = 0.16390625;       // Location after decimation and shifting
    double f_final_mhz;                         // Use CNCO and FNCO to shift input signal to this freq
    double fs_mhz;                              // ADC sampling rate
    double f_tone_mhz;                          // ADC input tone freq (real)
    double f_fnco_mhz;                          // Fine nco freq
    double f_cnco_mhz;                          // Coarse nco freq
    uint32_t fddc_dcm, cddc_dcm;                // Coarse and fine decimation
    uint8_t first_tone = 0;                     // Flag indicating first tone in sweep
    bool interleaved = 1;
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;
    bool coeffs_from_file = false;              // true to load coeffs from file. false to use coeffs from profile
    adi_ads10_apollo_dp_info_t rx_dp_info;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
 
    /* Device block selects for 4t4r or 8t8r device */
    uint16_t cncos = (device->dev_info.is_8t8r) ? ADI_APOLLO_CNCO_ALL : ADI_APOLLO_CNCO_ALL_4T4R;
    uint16_t fncos = (device->dev_info.is_8t8r) ? ADI_APOLLO_FNCO_ALL : ADI_APOLLO_FNCO_ALL_4T4R;
    uint16_t pfilts = (device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    uint16_t pfilts_a = (device->dev_info.is_8t8r) ? (ADI_APOLLO_PFILT_A0 | ADI_APOLLO_PFILT_A1) : ADI_APOLLO_PFILT_A0;
    uint16_t pfilts_b = (device->dev_info.is_8t8r) ? (ADI_APOLLO_PFILT_B0 | ADI_APOLLO_PFILT_B1) : ADI_APOLLO_PFILT_B0;

    /* Assign the filter gain and delay settings for the four 4 PFILT banks */
    adi_apollo_pfilt_gain_dly_pgm_t filt_gain_dly_config = {
        ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, // Gain for A0, A2, A1, A3
        63, 63, 63, 63,                                                                                                             // x/64 6-bit gain multiplier (32 = ~6dB)
        0};

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);
    
    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Rx A0 Profile", str_buff, str_buff_len);
    printf("\n%s\n", str_buff);
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 1, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Rx A1 Profile", str_buff, str_buff_len);
    printf("%s\n", str_buff);
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_B, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Rx B0 Profile", str_buff, str_buff_len);
    printf("%s\n", str_buff);
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_B, 1, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Rx B1 Profile", str_buff, str_buff_len);
    printf("%s\n\n", str_buff);

    adi_apollo_pfilt_mode_enable_set(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL);

    coeffs_from_file = !profile->rx_path[0].rx_pfilt[0].enable;     // Assume profile contains PFILT coeffs if enable set

    /* Load filter coeffs from file if not part of device profile, will use coeffs from profile otherwise */
    if (coeffs_from_file) {
        err |= adi_ads10_ex_pfilt_coeff_file_load(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0, "./filters/pfilt_coeffs_16_lp0.txt");
        err |= adi_ads10_ex_pfilt_coeff_file_load(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK1, "./filters/pfilt_coeffs_16_lp1.txt");
        err |= adi_ads10_ex_pfilt_coeff_file_load(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK2, "./filters/pfilt_coeffs_16_hp0.txt");
        err |= adi_ads10_ex_pfilt_coeff_file_load(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK3, "./filters/pfilt_coeffs_16_hp1.txt");
        err |= adi_apollo_pfilt_gain_dly_pgm(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK_ALL, &filt_gain_dly_config);
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    /* Get sampling freq, determine final tone location after decimation and shifting */
    fs_mhz = rx_dp_info.adc_sample_rate;
    f_final_mhz = -coherent_freq(rx_dp_info.fdata * final_tone_ratio, rx_dp_info.fdata, num_samples);

    /* Get the coarse and fine freq shifts from profile. This test assumes chans are the same */
    err = adi_apollo_cddc_dcm_bf_to_val(device, profile->rx_path[0].rx_cddc[0].drc_ratio, &cddc_dcm);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fddc_dcm_bf_to_val(device, profile->rx_path[0].rx_fddc[0].drc_ratio, &fddc_dcm);
    ADI_CMS_ERROR_RETURN(err);
    f_cnco_mhz = ((fs_mhz)*profile->rx_path[0].rx_cddc[0].nco[0].nco_phase_inc) / (1ull<<32);
    f_fnco_mhz = ((fs_mhz/cddc_dcm)*profile->rx_path[0].rx_fddc[0].nco[0].nco_phase_inc) / (1ull<<48);

    /*
    * Sweep the ADC input tone through the filter and capture. Each capture FFT
    * can be used to measure the filter response.
    */
    for (int i = 0; i < sizeof(tone_ratio) / sizeof(tone_ratio[0]); i++) {
        first_tone = (0 == i);
        f_tone_mhz = coherent_freq(tone_ratio[i] * fs_mhz, fs_mhz, 32 * 1024); // ADC input tone freq

        /*
         * Configure the CNCO/FNCO such that the resulting tone ends up at f_final_mhz. This is to keep it within the
         * band of the data rate.
         */
        f_cnco_mhz = f_tone_mhz - f_fnco_mhz - f_final_mhz;

        printf("ToneRatio=%f, Ftone=%f, CNCO=%f, FNCO=%f\n", tone_ratio[i], f_tone_mhz, f_cnco_mhz, f_fnco_mhz);
        err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_RX, cncos, fs_mhz, f_cnco_mhz);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_RX, fncos, fs_mhz / cddc_dcm, f_fnco_mhz);
        ADI_CMS_ERROR_RETURN(err);

        printf("Expected input tone location: %fMHz\n", f_final_mhz);

        /* Manually set the ADC analog input frequency */
        printf("@CMD : SET_ADC_INPUT_FREQ : %fMHz\n", f_tone_mhz);
        EXCTL_PAUSE(interactive);

        if (first_tone) {
            /*
            * Transfer filter coefficients from selected bank to active filter
            *
            * For Device Profile: id00_uc06_P
            *
            * Side A                                               ------- atten (db) @freq -------
            *         Filter Mode (Stream 0&1)   Filter Type       0.115*Fs    0.300*Fs    0.400*Fs
            *         ========================   ===========       ========    ========    ========
            *  Bank0: Real N/2 configuration      Low Pass         -0.0623     -8.667      -60.46
            *  Bank1: Real N/2 configuration      Low Pass         -0.0563     -67.87      -63.80
            *  Bank2: Real N/2 configuration      High Pass        -63.842     -8.604      +0.115
            *  Bank3: Real N/2 configuration      High Pass        -55.310     +0.174      +0.060
            *
            * Side B
            *         Filter Mode (Stream 0&1)   Filter Type       0.115*Fs    0.300*Fs    0.400*Fs
            *         ========================   ===========       ========    ========    ========
            *  Bank0: Real N/2 configuration      Low Pass         -0.0623     -8.667      -60.46
            *  Bank1: Real N/2 configuration      Low Pass         -0.0563     -67.87      -63.80
            *  Bank2: Real N/2 configuration      High Pass        -63.842     -8.604      +0.115
            *  Bank3: Real N/2 configuration      High Pass        -55.310     +0.174      +0.060
            *
            */
            err = adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_RX, pfilts_a, ADI_APOLLO_PFILT_BANK0); // Side A, make Bank 0 active
            ADI_CMS_ERROR_RETURN(err);
            err = adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_RX, pfilts_b, ADI_APOLLO_PFILT_BANK2); // Side B, make Bank 2 active
            ADI_CMS_ERROR_RETURN(err);
        }
     
        /* Read FPGA capture memory and write out i/q files */
        sprintf(file_name_base, "%s_%02d", "rx_adc_pfilt", i);
        printf("Writing captures to files: %s_*\n", file_name_base);
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

static double coherent_freq(double target, double fdata, uint32_t n_samples)
{
    int32_t m_cycles = (target * n_samples) / fdata;
    if (m_cycles % 2 == 0) {
        m_cycles++;
    }
    return ((m_cycles * fdata) / n_samples);
}

#endif /* !defined(VERSAL_PLATFORM) */
