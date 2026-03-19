/*!
 * \brief       Measure the time-of-flight (tof) of BSYNC signal (like SYSREF Clock)
                from its source, ADF4030, to the Apollo and FPGA SYSREF pin.
 *
 * The example demonstrates configuration of ADF4030 as an in-system SYSREF source and
 * how to utilize its bi-directions pads for aligning SYSREF going to multiple devices.
 *
 *  +----+                                                     +---+  +-------+  +-------+  +-------+                               +------+  +----+
 *  |STEP|                                                     |EXE|  |HMC7044|  |ADF4382|  |ADF4030|                               |APOLLO|  |FPGA|
 *  +----+                                                     +---+  +-------+  +-------+  +-------+                               +------+  +----+
 *    |                                                         |         |          |          |                                      |        |
 *    |                                                         |         |          |          |         DEVICE_CLK                   |        |
 *    |                                                         |         |          |------------------------------------------------>|        |
 *    |                                                         |         |          |          |         REF_CLK                      |        |
 *    |                                                         |         |-------------------------------------------------------------------->|
 *    |                                                         |         |       REF_CLK       |                                      |        |
 *    |                                                         |         |-------------------->|                                      |        |
 *    |   1. Setup ADF4030 for SYSREF and SYNC all SYSREF_OUT   |         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         | BSYNC_0(SYSREF_IN)  |                                      |        |
 *    |                                                         |         |-------------------->|                                      |        |
 *    |                                                         |         |          |          |         BSYNC_n*(SYSREF_OUT)         |        |
 *    |                                                         |         |          |          |------------------------------------->|        |
 *    |                                                         |         |          |          |             BSYNC_8(SYSREF_OUT)      |        |
 *    |                                                         |         |          |          |---------------------------------------------->|
 *    |                 2. Setup MCS Cal Config                 |         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         |          |          |                                      |        |
 *    |3. ADF4030 - Apollo Time-of-Flight Measurement and offset|         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         |          |          |       SYSREF Path Delay offset       |        |
 *    |                                                         |         |          |          |<------------------------------------>|        |
 *    | 4. ADF4030 - FPGA Time-of-Flight Measurement and offset |         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         |          |          |          SYSREF Path Delay offset    |        |
 *    |                                                         |         |          |          |<--------------------------------------------->|
 *    |         5. Sync all In and Out SYSREF w/ offset         |         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         | BSYNC_0(SYSREF_IN)  |                                      |        |
 *    |                                                         |         |-------------------->|                                      |        |
 *    |                                                         |         |          |          |         BSYNC_n*(SYSREF_OUT)         |        |
 *    |                                                         |         |          |          |------------------------------------->|        |
 *    |                                                         |         |          |          |             BSYNC_8(SYSREF_OUT)      |        |
 *    |                                                         |         |          |          |---------------------------------------------->|
 *    |                 6. Perform MCS Init Cal                 |         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         |          |          |SYNC Apollo Int SYSREF with Ext SYSREF|        |
 *    |                                                         |         |          |          |------------------------------------->|        |
 *    |          7. Perform MCS FG and BG Tracking cal          |         |          |          |                                      |        |
 *    |-------------------------------------------------------->|         |          |          |                                      |        |
 *    |                                                         |         |          | Track and SYNC Apollo Int SYSREF with Ext SYSREF|        |
 *    |                                                         |         |          |<------------------------------------------------|        |
 *    |                                                         |         |          |          |                                      |        |
 *  +----+                                                     +---+  +-------+  +-------+  +-------+                               +------+  +----+
 *  |STEP|                                                     |EXE|  |HMC7044|  |ADF4382|  |ADF4030|                               |APOLLO|  |FPGA|
 *  +----+                                                     +---+  +-------+  +-------+  +-------+                               +------+  +----+
 *
 * \note        BSYNC_n*:
 *              Apollo can have external sysref clock provided to 3 possible inputs, i.e Center, A-side, B-side.
 *              This configuration is called central/single clock when Center input is selected and
 *              Dual clock when sides A and B are selected. This selection is configured with device profile.
 *
 *              ADF4030 BSYNC channel 5 is configured as external SYSREF source for Apollo for single clock scheme and
 *              for Dual clock, BSYNC channel 6 and channel 7 are configured for sides A and B respectively.
 *
 *              Since the Apollo Eval Brd has single ADF4382 clock chip on broad, for dual clocking scheme the device clk for individual sides
 *              needs to be provided externally using a sig-gen along with an external 125MHz ref clk via J17.
 *              Also due to this the MCS Tracking calibration cannot be performed as individual side device clk phase adjustment isn't possible.
 *
 * \copyright   copyright(c) 2024 analog devices, inc. all rights reserved.
 *              This software is proprietary to Analog Devices, Inc. and its
 *              licensor. By using this software you agree to the terms of the
 *              associated analog devices software license agreement.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "adi_apollo.h"
#include "adi_utils.h"
#include "adi_fpga_apollo_types.h"
#include "apollo_cpu_device_profile_types.h"
#include "adi_fpga_apollo_private.h"
#include "adi_ads10_apollo_ex_mcs.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "ads10_fpga.h"

int32_t bsync_tof(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err;

    // MCS Calibration Data Variables
    uint64_t dev_clk_hz = (uint64_t) profile->clk_cfg.dev_clk_freq_kHz * 1e3;
    uint64_t adf4382_ref_freq_hz = 125e6;
    uint64_t adf4382_rfout_freq_hz = dev_clk_hz;

    /* NOTE: Assuming SYSREF settings for Center, A-side and B-side paths are the same, will use center for freq calculation */
    uint8_t divg_modulus = (profile->clk_cfg.clocking_mode == 0) ? 8 : 4;   // divg is global digital divider
    uint32_t sysref_prd_digclk_cycles = profile->mcs_cfg.internal_sysref_prd_digclk_cycles_center;
    double int_sysref_freq_hz = (double) (dev_clk_hz) / (divg_modulus * sysref_prd_digclk_cycles);
    uint64_t ext_sysref_freq_hz = (uint64_t) (int_sysref_freq_hz);
    printf("Sysref Freq (%f Hz) Supported: %s.\n", int_sysref_freq_hz, ((int_sysref_freq_hz == ext_sysref_freq_hz) ? "True" : "False"));

    // Currently supporting SYSREF Freq with minimum resolution of Hz.
    ADI_CMS_CHECK((int_sysref_freq_hz != ext_sysref_freq_hz), API_CMS_ERROR_NOT_SUPPORTED);

    uint8_t fpga_ref_div = (profile->jtx[0].common_link_cfg.ver == ADI_APOLLO_JESD_204C) ? 66 : 40;
    uint64_t fpga_ref_freq_hz = profile->jtx[0].common_link_cfg.lane_rate_kHz / fpga_ref_div * 1e3;

    // ADF4030 Device Struct and Config
    adi_adf4030_device_t adf4030 = {{0}};
    uint8_t adf4030_dev_id = ADF4030_0;
    uint16_t apollo_bsync_out_ch_sel = 0;
    uint16_t bsync_out_ch_sel = 0;
    uint16_t bsync_in_ch_sel = (1 << ADI_ADF4030_CHANNEL_ID_0);
    uint8_t align_cycles_iters = 5;
    const uint8_t num_apollo_sysref_path = 3;
    // Arr[Central, A-side, B-side]
    int64_t adf4030_apollo_path_delay[3] = {0};
    int64_t adf4030_fpga_path_delay = 0;
    uint16_t die_temp = 0;

    // MCS Config Struct
    mcs_tof_config_t mcs_tof = {
        .bsync_divider = divg_modulus * sysref_prd_digclk_cycles,
        .fpga_ref_freq_hz = fpga_ref_freq_hz,
        .adf4030_ref_freq_hz = (uint64_t) (fpga_ref_freq_hz < ADI_ADF4030_REF_FREQ_MAX) ? fpga_ref_freq_hz : (fpga_ref_freq_hz / 2),    // Generated by HMC7044. To-Do: HMC7044 to generate fixed ref clk.
        .vco_out_freq_hz = 0,
        .bsync_in_ref_ch = ADI_ADF4030_CHANNEL_ID_0,
        .center = {
            .sysref_present = profile->mcs_cfg.center_sysref.sysref_present,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_5,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        },
        .aside = {
            .sysref_present = profile->mcs_cfg.aside_sysref.sysref_present,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_6,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        },
        .bside = {
            .sysref_present = profile->mcs_cfg.bside_sysref.sysref_present,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_7,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        },
        .fpga = {
            .sysref_present = true,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_8,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        }
    };

    /* Determine Apollo SYSREF BSYNC Source based on profile clk config */
    err = adi_ads10_apollo_ex_adf4030_apollo_bsync_out_ch_get(profile, &apollo_bsync_out_ch_sel);
    ADI_CMS_ERROR_RETURN(err);

    // CH_1: Observe SYSREF, CH_8: FPGA SYSREF, Profile based Apollo SYSREF Channel
    bsync_out_ch_sel = ((1 << ADI_ADF4030_CHANNEL_ID_1) | (1 << ADI_ADF4030_CHANNEL_ID_8) | apollo_bsync_out_ch_sel);

    err = adi_ads10_apollo_ex_adf4030_vco_freq_calc(mcs_tof.adf4030_ref_freq_hz, mcs_tof.center.bsync_out_sysref_freq_hz, &mcs_tof.vco_out_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // MCS Init Cal
    adi_apollo_mcs_cal_init_status_t init_cal_status = {{0}};

    // MCS Tracking cal
    uint16_t mcs_tracking_decimation = 1023;

    // ADF4382 Device Struct
    adi_adf4382_device_t adf4382 = {{0}};

    printf("\n");
    printf("dev_clk_hz: %lld.\t", dev_clk_hz);
    printf("divg_modulus: %d.\t sysref_prd_digclk_cycles: %d.\n", divg_modulus, sysref_prd_digclk_cycles);
    printf("int_sysref_freq_hz: %f.\n", int_sysref_freq_hz);
    printf("ext_sysref_freq_hz: %lld.\n", ext_sysref_freq_hz);
    printf("bsync_divider: %d.\n", mcs_tof.bsync_divider);
    printf("bsync_out_sysref_freq_hz: %lld.\n", mcs_tof.center.bsync_out_sysref_freq_hz);
    printf("adf4030_ref_freq_hz: %lld.\n", mcs_tof.adf4030_ref_freq_hz);
    printf("vco_out_freq_hz: %lld.\n\n", mcs_tof.vco_out_freq_hz);

    /* Run Clock Conditioning cal */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 01. Configure device struct and HAL settings for ADF4382 and ADF4030.
              Perform ADF4030 startup and configure BSYNC input/output settings. */

    // Configure ADF4382 HAL
    err = adi_ads10_apollo_ex_adf4382_hal_config(&adf4382,
                                                 NULL,
                                                 &ads10_fpga_fmcb_aux_gpio_write);
    ADI_CMS_ERROR_RETURN(err);

    // Configure ADF4030 HAL.
    err = adi_ads10_apollo_ex_adf4030_configure_hal(&adf4030, adf4030_dev_id);
    ADI_CMS_ERROR_RETURN(err);

    // Perform ADF4030 power up and initialization.
    err = adi_ads10_apollo_ex_adf4030_startup(&adf4030, mcs_tof.adf4030_ref_freq_hz, mcs_tof.vco_out_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // BSYNC setup for sysref generation.
    err = adi_ads10_apollo_ex_adf4030_bsync_input_set(&adf4030, bsync_in_ch_sel);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_adf4030_bsync_output_set(&adf4030,
                                                       bsync_out_ch_sel,
                                                       mcs_tof.vco_out_freq_hz,
                                                       mcs_tof.center.bsync_out_sysref_freq_hz, 1);
    ADI_CMS_ERROR_RETURN(err);

    // SYNC all BSYNC_OUT channels with BSYNC_IN Ref channel.
    printf("Align all BSYNC_OUT Channels with ref BSYNC_IN Channel...\n");
    err = adi_ads10_apollo_ex_adf4030_align_bsync_out(&adf4030,
                                                      bsync_in_ch_sel,
                                                      bsync_out_ch_sel,
                                                      mcs_tof.center.bsync_out_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // Measure ADF4030 DIE Temperature.
    err = adi_adf4030_core_die_temp_get(&adf4030, &die_temp);
    ADI_CMS_ERROR_RETURN(err);

    printf("ADF4030 Die Temp: %d *C.\n", die_temp);


/*  Step: 02. Program MCS Init Cal, including most of settings for ADF4382 Tracking.
              And configure ADF4382 for MCS init cal. */

    err = adi_ads10_apollo_ex_mcs_init_cal_setup(device,
                                                 &adf4382,
                                                 adf4382_ref_freq_hz,
                                                 adf4382_rfout_freq_hz,
                                                 ext_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 03. ADF4030 - Apollo Time-of-Flight Measurement and offset. */

    // Measure sysref path delay.
    err = adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_measurement(device,
                                                                        &adf4030,
                                                                        &mcs_tof,
                                                                        adf4030_apollo_path_delay,
                                                                        num_apollo_sysref_path);
    ADI_CMS_ERROR_RETURN(err);

    // Offset path delay from BSYNC output.
    err = adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_offset(&adf4030,
                                                                   &mcs_tof,
                                                                   align_cycles_iters,
                                                                   adf4030_apollo_path_delay,
                                                                   num_apollo_sysref_path);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 04. ADF4030 - FPGA Time-of-Flight Measurement and offset. */

    err = adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_measurement(&adf4030,
                                                                      fpga_device,
                                                                      &mcs_tof,
                                                                      &adf4030_fpga_path_delay);
    ADI_CMS_ERROR_RETURN(err);

    // Offset path delay from BSYNC output.
    err = adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_offset(&adf4030,
                                                                 &mcs_tof,
                                                                 align_cycles_iters,
                                                                 adf4030_fpga_path_delay);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 05. SYNC all BSYNC_OUT channels with BSYNC_IN Ref channel. */

    printf("Align all BSYNC_OUT Channels with ref BSYNC_IN Channel...\n");
    err = adi_ads10_apollo_ex_adf4030_align_bsync_out(&adf4030,
                                                      bsync_in_ch_sel,
                                                      bsync_out_ch_sel,
                                                      mcs_tof.center.bsync_out_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 06. Perform MCS Init Cal for Sysref Alignment.
              Read back MCS init cal status data and validate init cal success. */

    printf("Running MCS Init Calibration...\n");
    err = adi_apollo_mcs_cal_init_run(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_mcs_cal_init_status_get(device, &init_cal_status);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_mcs_init_cal_validate(device, profile, &init_cal_status);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 07. Optional. Perform MCS Tracking Cal for maintaining alignment.
              MCS Tracking Calibration restricted to Single Clock scheme due to HW limitation. */
    if (profile->clk_cfg.single_dual_clk_sel == false) {
        err = adi_ads10_apollo_ex_mcs_tracking_cal_setup(device, mcs_tracking_decimation, 1);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4382_phase_adjust_auto_align_enable(&adf4382, 1);
        ADI_CMS_ERROR_RETURN(err);

        /* Execute MCS Foreground Tracking cal for faster SysRef Alignment.
           Performs TDC measurements and multi phase correction strobes for clock calibration. */
        printf("Running MCS FG Tracking Calibration...\n");
        err = adi_apollo_mcs_cal_fg_tracking_run(device);
        ADI_CMS_ERROR_RETURN(err);

        /* Trigger MCS Background Tracking cal for maintaining alignment.
           Executes in background periodically, taking TDC measurements and phase correction if required. */
        printf("Running MCS BG Tracking Calibration...\n");
        err = adi_apollo_mcs_cal_bg_tracking_run(device);
        ADI_CMS_ERROR_RETURN(err);
    }


    // Measure ADF4030 DIE Temperature.
    err = adi_adf4030_core_die_temp_get(&adf4030, &die_temp);
    ADI_CMS_ERROR_RETURN(err);

    printf("ADF4030 Die Temp: %d *C.\n", die_temp);

    // err = adi_ads10_apollo_ex_mcs_sysref_info_print(device);
    // ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_mcs_sysref_phase_print(device);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_mcs_link_phase_print(device);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}
