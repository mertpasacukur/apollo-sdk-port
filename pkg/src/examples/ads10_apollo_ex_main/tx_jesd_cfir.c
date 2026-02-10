#if defined(__linux__)

/*!
 * \brief     ADS10 Apollo JRx data path with CFIR
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
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
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t tx_jesd_cfir(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{

    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    double tone_ratio[] = { 0.1, 0.2, 0.3, 0.4 };   // Test input tones: pass, transition and stop bands. e.g. Freq = Fs*tone_ratio
    double fs_mhz;                                  // DAC sampling rate
    double f_tone_mhz;                              // DAC output freq
    double f_fnco_mhz;                              // Fine nco freq
    double f_cnco_mhz;                              // Coarse nco freq
    double f_data_rate_mhz;                         // JESD data rate = Fdac/(CDUC*FDUC)
    uint32_t fduc_interp, cduc_interp;              // Coarse and fine decimation
    uint8_t first_tone = 0;                         // Flag indicating first tone in sweep
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t tx_dp_info;
    adi_fpga_feature_flag_t fpga_feature_flags;

    /* Inspect the JRx link states */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Get sampling freq from profile */
    fs_mhz = profile->clk_cfg.dev_clk_freq_kHz / 1e3;

    /* Determine the coarse and fine freq shifts from profile. This test assumes chans the same (i.e. use first) */
    adi_apollo_cduc_interp_bf_to_val(device, profile->tx_path[0].tx_cduc[0].drc_ratio, &cduc_interp);
    adi_apollo_fduc_interp_bf_to_val(device, profile->tx_path[0].tx_fduc[0].drc_ratio, &fduc_interp);
    f_cnco_mhz = ((fs_mhz)*profile->tx_path[0].tx_cduc[0].nco[0].nco_phase_inc) / (1ull << 32);
    f_fnco_mhz = ((fs_mhz / cduc_interp) * profile->tx_path[0].tx_fduc[0].nco[0].nco_phase_inc) / (1ull << 48);
    f_data_rate_mhz = fs_mhz / (cduc_interp * fduc_interp);     // JESD data rate

    /* Get flag indicating FPGA HW transport support */
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    printf("FPGA DAC TL: %s\n", fpga_feature_flags.tx_hw_tl ? "HW" : "SW");

    /* Get Tx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    printf("Create tone (A/B): %7.3fMHz\n", tone_ratio[0] * tx_dp_info.fdata);
    /* Create and populate transmit buffer with baseband tone at 0.1*Fdata, = 125MHz @1250Gsps, 250MHz @2500 */

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, EX_VEC_DEFAULT_SAMPLES_PER_VC, tone_ratio[0], -1.0);
    ADI_CMS_ERROR_RETURN(err);

    /* Transmit data out from FPGA JTx */
    err = adi_fpga_apollo_core_tx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /*
    * Run the serdes calibration
    *
    * NOTE: The FPGA must be transmitting (any data) for serdes cal to function properly
    */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    /*
     * For each tone ratio input, generate tone
    */
    for (int i = 0; i < sizeof(tone_ratio) / sizeof(tone_ratio[0]); i++) {
        first_tone = (0 == i);

        f_tone_mhz = tone_ratio[i] * f_data_rate_mhz;                // Tone freq going into the filter

        /*
        * On first tone load the filter coefficients for profile
        */
        if (first_tone) {

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
            err = adi_apollo_cfir_profile_sel(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_A0 | ADI_APOLLO_CFIR_A1, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_0);  // profile 0 - low pass
            ADI_CMS_ERROR_RETURN(err);

            err = adi_apollo_cfir_profile_sel(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_B0 | ADI_APOLLO_CFIR_B1, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_1); // profile 1 = high pass
            ADI_CMS_ERROR_RETURN(err);

        }
    
        /* Will assume all chans match A0 (side-A, chan 0) */
        printf("Create tone (A/B): %7.3fMHz\n", tone_ratio[i] * tx_dp_info.fdata);
        err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, EX_VEC_DEFAULT_SAMPLES_PER_VC, tone_ratio[i], -1.0);
        ADI_CMS_ERROR_RETURN(err);

        /* Start transmit from FPGA JTx */
        err = adi_fpga_apollo_core_ptn_play_start(fpga_device);
        ADI_CMS_ERROR_RETURN(err);

        printf("ToneRatio=%f, Ftone=%f, CNCO=%f, FNCO=n/a\n", tone_ratio[i], f_tone_mhz, f_cnco_mhz);

        // printf("@CMD : MEAS_DAC_OUTPUT : EXPECT: %fMHz\n", f_tone_mhz + f_cnco_mhz + f_fnco_mhz);
        // printf("<cr> to continue\n");
        // getchar();
        EXCTL_TX_MEAS_FREQ(0x33, f_tone_mhz + f_cnco_mhz + f_fnco_mhz, 10, interactive);

        printf("\n");
    }


    /*
    * Disable (i.e. bypass) all CFIR
    */
    adi_apollo_cfir_mode_enable_set(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_BYPASS);
    printf("All CFIRs bypassed\n");
    // printf("@CMD : MEAS_DAC_OUTPUT : EXPECT: %fMHz\n", f_tone_mhz + f_cnco_mhz + f_fnco_mhz);
    // printf("<cr> to continue\n");
    // getchar();

    EXCTL_TX_MEAS_FREQ(0x33, f_tone_mhz + f_cnco_mhz + f_fnco_mhz, 10, interactive);

    return err;
}

#endif /* defined(__linux__) */
