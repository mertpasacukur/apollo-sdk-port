/*!
 * \brief     ADS10 Apollo Sparse CFIR example
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 *
 * Enables CFIR sparse mode for Rx and Tx.
 * Cycles through:
 *  - cfir enabled, pass band (~.3dB attenuation)
 *  - cfir enabled, transition band (~10dB attenuation)
 *  - cfir enabled, stop band (~22dB attenuation)
 *  - cfir bypassed
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
#include "adi_fpga_apollo_transmit.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t fullchip_sparse_cfir(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{

    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    double tone_ratio[] = { 0.024, .065, 0.13 };   // Input to CFIR should be Passband, transition band, stop band
    double fs_mhz = profile->clk_cfg.dev_clk_freq_kHz / 1e3;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t tx_dp_info;
    adi_fpga_feature_flag_t fpga_feature_flags;
    uint16_t i_coeffs[16] = {0xF726, 0xF29B, 0x0589, 0x0676, 0x011D, 0x047D, 0x247D, 0x4AAD, 0x4AAD, 0x247D, 0x047D, 0x011D, 0x0676, 0x0589, 0xF29B, 0xF726};
    uint16_t q_coeffs[16] = {0x03AA, 0xFEF1, 0x0365, 0x0A88, 0x0E33, 0xF52C, 0xE0D5, 0xEE12, 0x11ED, 0x1F2A, 0x0AD3, 0xF1CC, 0xF577, 0xFC9A, 0x010E, 0xFC55};
    uint16_t coeff_sel[16] = {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C};
    adi_apollo_cfir_pgm_t cfir_config = {
        .cfir_32taps_en = 0,
        .cfir_bypass = 0,
        .cfir_coeff_transfer = 0,
        .cfir_sparse_filt_en = 1,
    };
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;
    uint32_t cddc_dcm;

    /* Inspect the JRx link states */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Get flag indicating FPGA HW transport support */
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    printf("FPGA DAC TL: %s\n", fpga_feature_flags.tx_hw_tl ? "HW" : "SW");

    /* Get Tx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);
    adi_apollo_cddc_dcm_bf_to_val(device, profile->rx_path[0].rx_cddc[0].drc_ratio, &cddc_dcm);

    err = adi_apollo_cfir_coeff_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_PROFILE_ALL, ADI_APOLLO_CFIR_DP_ALL, i_coeffs, q_coeffs, 16);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_gain_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_PROFILE_ALL, ADI_APOLLO_CFIR_DP_ALL, ADI_APOLLO_CFIR_GAIN_MINUS6_DB); // Set gain to -6dB
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_sparse_coeff_sel_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_PROFILE_ALL, ADI_APOLLO_CFIR_DP_ALL, coeff_sel, 16);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, &cfir_config);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_cfir_coeff_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_PROFILE_ALL, ADI_APOLLO_CFIR_DP_ALL, i_coeffs, q_coeffs, 16);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_gain_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_PROFILE_ALL, ADI_APOLLO_CFIR_DP_ALL, ADI_APOLLO_CFIR_GAIN_MINUS6_DB); // Set gain to -6dB
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_sparse_coeff_sel_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_PROFILE_ALL, ADI_APOLLO_CFIR_DP_ALL, coeff_sel, 16);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, &cfir_config);
    ADI_CMS_ERROR_RETURN(err);

    // Rx FNCO tone into CFIR
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);     /* ZIF mode bypasses CNCO */
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cnco_mixer_set(device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_DRC_MIXER_COMPLEX);   /* Should be in profile */
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_fnco_hop_enable(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, 0);                             /* Update FNCO through main, non-hopping nco control */
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fnco_mixer_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_DRC_MIXER_COMPLEX);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fnco_mode_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_MXR_TEST_MODE);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fnco_test_mode_val_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_FNCO_MXR_TEST_RX_FS_BY2);
    ADI_CMS_ERROR_RETURN(err);

    cfir_config.cfir_coeff_transfer = 1;
    err = adi_apollo_cfir_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, &cfir_config);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, &cfir_config);
    ADI_CMS_ERROR_RETURN(err);

    /* Clock conditioning calibration */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    printf("\n        PASSBAND    TRANSITION    STOPBAND\n");
    printf("Atten      0.3dB          10dB        22dB\n\n");

    for (int i = 0; i < 3; i++) {
        // Transmit
        /* Load FPGA transmit memory */
        err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, num_samples, tone_ratio[i], -1.0);
        ADI_CMS_ERROR_RETURN(err);

        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        if (i == 0) {
            err = adi_fpga_apollo_core_bidir_init(fpga_device);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG| ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
            ADI_CMS_ERROR_RETURN(err);
        } else {
            err = adi_fpga_apollo_core_ptn_play_start(fpga_device);
            ADI_CMS_ERROR_RETURN(err);
        }

        // Capture
        err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, 1.0, -tone_ratio[i]/cddc_dcm);
        ADI_CMS_ERROR_RETURN(err);
        printf("Freq %f\n", fs_mhz/cddc_dcm);
        sprintf(str_buff, "cfir_enabled_%s", i == 0 ? "PASSBAND" : (i==1) ? "TRANSITION" : "STOPBAND");
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, str_buff, 1);
        ADI_CMS_ERROR_RETURN(err);

        printf("Expect signal in %s\n", i == 0 ? "PASSBAND" : (i==1) ? "TRANSITION" : "STOPBAND");
        EXCTL_PAUSE(interactive);
    }

    /*
    * Disable (i.e. bypass) all CFIR
    */
    printf("CFIRs bypassed\n");
    err = adi_apollo_cfir_mode_enable_set(device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_BYPASS);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cfir_mode_enable_set(device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_BYPASS);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, "cfir_bypassed", 1);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}
