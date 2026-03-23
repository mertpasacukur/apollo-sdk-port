/*!
 * \brief     ADS10 Apollo Tx data path NCO test with PFILT
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
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t tx_nco_pfilt(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    double tone_ratio[] = {0.115, 0.3, 0.4 };       // Test input tones: pass, transition and stop bands. e.g. Freq = Fs*tone_ratio
    double fs_mhz;                                  // DAC sampling rate
    double f_tone_mhz;                              // DAC output freq
    double f_cnco_mhz;                              // Coarse nco freq
    uint8_t first_tone = 0;                         // Flag indicating first tone in sweep
    adi_apollo_pfilt_inspect_t pfilt_inspect_a0;    // PFILT inspect struct for PFILT A0
    adi_apollo_pfilt_inspect_t pfilt_inspect_b0;    // PFILT inspect struct for PFILT B0
    char* fname;                                    // Name of filter coeff file to load

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Set Tx CNCO dc test values */
    err = adi_apollo_cnco_test_mode_val_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, profile->tx_path[0].tx_cduc[0].nco[0].dc_testmode_value);
    ADI_CMS_ERROR_RETURN(err);

    /* INVSINC is configured by device profile, but can be enabled/disabled on demand. */
    err = adi_apollo_invsinc_enable(device, ADI_APOLLO_TX_INVSINC_ALL, 1);
    ADI_CMS_ERROR_RETURN(err);

    /* Inspect the INVSINC state */
    err = adi_ads10_apollo_ex_inspect_invsinc_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Put the CNCO in DC test mode. This is used as input tone source for filter.
    *
    *  Note: because the Tx CNCO is in DC test mode, the FNCO path is blocked.
    */
    adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_TEST_MODE);        /* Should be in profile */

    /* Get DAC sampling freq from profile */
    fs_mhz = profile->clk_cfg.dev_clk_freq_kHz/1e3;

    /*
    * For each tone ratio input tone in sweep, use the CNCO to generate the signal
    */
    for (int i = 0; i < sizeof(tone_ratio) / sizeof(tone_ratio[0]); i++) {
        first_tone = (0 == i);

        f_tone_mhz = tone_ratio[i] * fs_mhz;                // Tone freq going into the filter
        f_cnco_mhz = f_tone_mhz;

        /*
        * On first tone load the active filter coefficients.
        */
        if (first_tone) {
            /*
            * Transfer filter coefficients from selected bank to active filter
            *
            * For Device Profile: id00_uc_06_PT
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
            err = adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0, ADI_APOLLO_PFILT_BANK0);        // Side A, make Bank 0 active
            ADI_CMS_ERROR_RETURN(err);

            err = adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_B0, ADI_APOLLO_PFILT_BANK2);        // Side B, make Bank 2 active
            ADI_CMS_ERROR_RETURN(err);

        }

        printf("ToneRatio=%f, Ftone=%f, CNCO=%f, FNCO=n/a\n", tone_ratio[i], f_tone_mhz, f_cnco_mhz);
        adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_A0 | ADI_APOLLO_CNCO_A1 | ADI_APOLLO_CNCO_B0 | ADI_APOLLO_CNCO_B1, fs_mhz, f_cnco_mhz);

        printf("@CMD : MEAS_DAC_OUTPUT : EXPECT: %fMHz\n", tone_ratio[i] * fs_mhz);
        EXCTL_PAUSE(interactive);

        printf("*** Begin PFILT modification ***\n");
        EXCTL_PAUSE(interactive);

        /*
        * Inspect the PFILT blocks
        *
        * This captures the state of a PFILT block by reading device registers.
        */

        adi_apollo_pfilt_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0, &pfilt_inspect_a0);
        adi_apollo_pfilt_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_B0, &pfilt_inspect_b0);
        printf("Inspecting PFILT status\n");
        printf("PFILT A0: i_mode=%d, q_mode=%d\n", pfilt_inspect_a0.dp_cfg.i_mode, pfilt_inspect_a0.dp_cfg.q_mode);     // note: 2 = real n/2 mode, 0 = bypassed
        printf("PFILT B0: i_mode=%d, q_mode=%d\n", pfilt_inspect_b0.dp_cfg.i_mode, pfilt_inspect_a0.dp_cfg.q_mode);
        EXCTL_PAUSE(interactive);

        /*
        * Disable (i.e. bypass) all PFILT streams
        */
        adi_apollo_pfilt_mode_enable_set(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0 | ADI_APOLLO_PFILT_B0, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);
        printf("All PFILT streams bypassed\n");
        EXCTL_PAUSE(interactive);

        /*
        * Re-enable all PFILT streams by setting the filter mode back to real n/2 mode
        */
        adi_apollo_pfilt_mode_enable_set(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0 | ADI_APOLLO_PFILT_B0, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL);
        printf("All PFILT streams re-enabled to real n/2 mode\n");
        EXCTL_PAUSE(interactive);

        /*
        * Load new coefficients, from a text file, into PFILT A0-Bank1 and PFILT B0-Bank1
        *
        * Make Bank1 active for all streams by transferring the coeffs into the filter
        *
        */
        fname = "./filters/pfilt_coeffs_16_zeros.txt";
        printf("Loading filter coeffs from file %s into BANK1\n", fname);
        adi_ads10_ex_pfilt_coeff_file_load(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0 | ADI_APOLLO_PFILT_B0, ADI_APOLLO_PFILT_BANK1, fname);
        adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0 | ADI_APOLLO_PFILT_B0, ADI_APOLLO_PFILT_BANK1);
        printf("Effects of new filter are new applied\n");
        EXCTL_PAUSE(interactive);

        /*
        * Restore original PFILT to state prior to filter modes.
        */
        adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_A0, ADI_APOLLO_PFILT_BANK0);        // Side A, make Bank 0 active
        adi_apollo_pfilt_coeff_transfer(device, ADI_APOLLO_TX, ADI_APOLLO_PFILT_B0, ADI_APOLLO_PFILT_BANK2);        // Side B, make Bank 2 active

        printf("*** End PFILT modification, state restored ***\n");
        EXCTL_PAUSE(interactive);
    }

    return err;
}
