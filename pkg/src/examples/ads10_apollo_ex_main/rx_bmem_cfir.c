/*!
 * \brief     ADS10 Apollo Rx CFIR data path test using BMEM-AWG as input source
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
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_ads10_apollo_ex_bmem.h"
#include "adi_fpga_apollo_core.h"

int32_t rx_bmem_cfir(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    char file_name_base[256];
    double fs_mhz;                              // ADC sampling rate
    double f_cfir_dr_mhz;                       // Filter data rate
    double f_cfir_mhz;                          // CIFR input tone freq
    double f_fnco_mhz;                          // Fine nco freq
    double f_cnco_mhz;                          // Coarse nco freq
    uint32_t fddc_dcm, cddc_dcm;                // Coarse and fine decimation
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rx_dp_info;
    uint16_t bmem_sel = ADI_APOLLO_BMEM_A0 | ADI_APOLLO_BMEM_A1 | ADI_APOLLO_BMEM_B0 | ADI_APOLLO_BMEM_B1;                              // BMEM-AWG selection, each instance represents an ADC
    double tone_ratio[] = { 0.1, 0.2, 0.3, 0.4 };   // Test input tones: pass, transition and stop bands. e.g. Freq = Fs*tone_ratio
    bool interleaved = 1;
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;

    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    err = adi_ads10_apollo_ex_bmem_awg_config(device, bmem_sel);
    ADI_CMS_ERROR_RETURN(err);

    /* Clock conditioning calibration */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /*
    * Transfer filter coefficients from selected bank to active filter
    *
    * For Device Profile: id00_uc06_C (w/ -18db gain)
    *
    * Side A                          ------- atten (db) @freq -------
    *               Filter Type       0.100*Fs    0.200*Fs    0.300*Fs    0.400*Fs
    *               ===========       ========    ========    ========    ========
    *  Profile 0:   Low Pass          -14.93      -23.8       -82.9       -80.0
    *  Profile 1:   High Pass         -46.7       -61.3       -18.0       -12.1
    *
    * Side B                          ------- atten (db) @freq -------
    *               Filter Type       0.100*Fs    0.200*Fs    0.300*Fs    0.400*Fs
    *               ===========       ========    ========    ========    ========
    *  Profile 0:   Low Pass          -14.93      -23.8       -82.9       -80.0
    *  Profile 1:   High Pass         -46.7       -61.3       -18.0       -12.1
    */

    // profile 0 - low pass
    err = adi_apollo_cfir_profile_sel(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_A0 | ADI_APOLLO_CFIR_A1, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_0);
    ADI_CMS_ERROR_RETURN(err);

    // profile 1 = high pass
    err = adi_apollo_cfir_profile_sel(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_B0 | ADI_APOLLO_CFIR_B1, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_1);
    ADI_CMS_ERROR_RETURN(err);

    /* Get CDDC and FDDC decimation value */
    err = adi_apollo_cddc_dcm_bf_to_val(device, profile->rx_path[0].rx_cddc[0].drc_ratio, &cddc_dcm);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fddc_dcm_bf_to_val(device, profile->rx_path[0].rx_fddc[0].drc_ratio, &fddc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    /* Get sampling freq from profile */
    fs_mhz = profile->clk_cfg.dev_clk_freq_kHz / 1e3;
    f_cnco_mhz = ((fs_mhz)*profile->rx_path[0].rx_cddc[0].nco[0].nco_phase_inc) / (1ull << 32);
    f_fnco_mhz = ((fs_mhz / cddc_dcm) * profile->rx_path[0].rx_fddc[0].nco[0].nco_phase_inc) / (1ull << 48);
    f_cfir_dr_mhz = fs_mhz / (cddc_dcm * fddc_dcm);

    /*
     * For each tone ratio input, generate tone
    */
    for (int i = 0; i < sizeof(tone_ratio) / sizeof(tone_ratio[0]); i++) {

        /*
         * Determine the ADC (BMEM) input signal freq necessary to test CFIR. Takes into account NCOs.
         */
        f_cfir_mhz = tone_ratio[i] * f_cfir_dr_mhz + f_cnco_mhz + f_fnco_mhz;

        /*
         * Use BMEM-AWG to inject a real tone into the Rx data path.
         */
        err = adi_ads10_apollo_ex_bmem_awg_tone_write(device, bmem_sel, f_cfir_mhz/fs_mhz, -18.0, false);
        ADI_CMS_ERROR_RETURN(err);

        /* Read FPGA capture memory and write out i/q files */
        sprintf(file_name_base, "%s_%02d", "rx_bmem_cfir", i);
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
        ADI_CMS_ERROR_RETURN(err);
    }

    /*
    * Disable (i.e. bypass) all CFIR
    */
    err = adi_apollo_cfir_mode_enable_set(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_BYPASS);
    ADI_CMS_ERROR_RETURN(err);

    /* Read FPGA capture memory and write out i/q files */
    sprintf(file_name_base, "%s_bypass", "rx_bmem_cfir");
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}
