#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo Tx data path NCO test
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

int32_t tx_nco(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    uint32_t cdrc_dcm;
    adi_apollo_cduc_inspect_t cduc_inspect;
    double cnco_tone_mhz = 2500;
    double fnco_tone_mhz = 500;

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Get CDUC interpolation value - this is used to determine the clock rate to the FNCO */
    err = adi_apollo_cduc_inspect(device, ADI_APOLLO_CDUC_A0, &cduc_inspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cduc_interp_bf_to_val(device, cduc_inspect.dp_cfg.drc_ratio, &cdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the CNCO and FNCO frequencies */
    err = adi_ads10_apollo_ex_cnco_freq_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, false, profile->clk_cfg.dev_clk_freq_kHz, cnco_tone_mhz * 1e3);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, profile->clk_cfg.dev_clk_freq_kHz / cdrc_dcm, fnco_tone_mhz * 1e3);
    ADI_CMS_ERROR_RETURN(err);

    adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_VAR_IF_MODE);      /* VAR_IF mixer and nco enabled */
    adi_apollo_cnco_mixer_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_DRC_MIXER_COMPLEX);   /* Input to CNCO is complex */

    adi_apollo_fnco_hop_enable(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 0);                             /* Update FNCO through main, non-hopping nco control */
    adi_apollo_fnco_mixer_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_DRC_MIXER_COMPLEX);   /* Should be in profile */
    adi_apollo_fnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_MXR_TEST_MODE);        /* FNCO in test mode */
    adi_apollo_fnco_test_mode_val_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 0x5a85);                 /* Full scale backed off ~3db (0x7fff/1.414) */
    adi_apollo_fnco_main_phase_offset_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 0);

    EXCTL_TX_MEAS_FREQ(0x33, cnco_tone_mhz + fnco_tone_mhz, 10, interactive);

    /* FNCO test - CNCO in ZIF mode bypassing it. (Tone at 500MHz (Fs=20G) */
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);     /* ZIF mode bypasses CNCO */
    ADI_CMS_ERROR_RETURN(err);
    EXCTL_TX_MEAS_FREQ(0x33, fnco_tone_mhz, 10, interactive);

    /* FNCO + CNCO - CNCO in VAR mode including it. Output freq is sum of FNCO and CNCO. (Tone at 3000MHz (Fs=20G) */
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_VAR_IF_MODE);      /* VAR IF mode allows FNCO out to CNCO input */
    ADI_CMS_ERROR_RETURN(err);
    EXCTL_TX_MEAS_FREQ(0x33, cnco_tone_mhz + fnco_tone_mhz, 10, interactive);

    /* CNCO only - CNCO back to test mode. (Tone at 2500MHz (Fs=20G) */
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_TEST_MODE);        /* CNCO to test mode, blocks FNCO data */
    ADI_CMS_ERROR_RETURN(err);
    EXCTL_TX_MEAS_FREQ(0x33, cnco_tone_mhz, 10, interactive);

    return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
