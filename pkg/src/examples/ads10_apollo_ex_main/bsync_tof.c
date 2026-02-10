#if defined(__linux__)

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
 *    |                                                         |         |          |          |         BSYNC_5(SYSREF_OUT)          |        |
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
 *    |                                                         |         |          |          |         BSYNC_5(SYSREF_OUT)          |        |
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
#include "ads10_fpga.h"

// FOR DEBUG ONLY
static __maybe_unused int32_t check_sysref(adi_apollo_device_t *device);
static __maybe_unused void print_sysref_phase(adi_apollo_device_t* device);
static __maybe_unused void print_link_phase(adi_apollo_device_t* device);

int32_t bsync_tof(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err;

    // MCS Calibration Data Variables
    uint64_t dev_clk_hz = (uint64_t) profile->clk_cfg.dev_clk_freq_kHz * 1e3;
    uint64_t adf4382_ref_freq_hz = 125e6;
    uint64_t adf4382_rfout_freq_hz = dev_clk_hz;
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
    uint16_t bsync_out_ch_sel = ((1 << ADI_ADF4030_CHANNEL_ID_1) | (1 << ADI_ADF4030_CHANNEL_ID_5) | (1 << ADI_ADF4030_CHANNEL_ID_8));   // CH_1: Scope, CH_5: Apollo SYSREF, CH_8: FPGA SYSREF
    uint16_t bsync_in_ch_sel = (1 << ADI_ADF4030_CHANNEL_ID_0);
    uint8_t align_cycles_iters = 5;
    int64_t adf4030_apollo_path_delay = 0;
    int64_t adf4030_fpga_path_delay = 0;
    uint16_t die_temp = 0;

    // MCS Config Struct
    mcs_tof_config_t mcs_tof = {
        .bsync_divider = divg_modulus * sysref_prd_digclk_cycles,
        .fpga_ref_freq_hz = fpga_ref_freq_hz,
        .adf4030_ref_freq_hz = (uint64_t) (fpga_ref_freq_hz < ADI_ADF4030_REF_FREQ_MAX) ? fpga_ref_freq_hz : (fpga_ref_freq_hz / 2),    // Generated by HMC7044. To-Do: HMC7044 to generate fixed ref clk.
        .vco_out_freq_hz = 0,
        .bsync_out_sysref_freq_hz = ext_sysref_freq_hz,
        .bsync_in_ref_ch = ADI_ADF4030_CHANNEL_ID_0,
        .bsync_out_apollo_sysref_ch = ADI_ADF4030_CHANNEL_ID_5,
        .bsync_out_fpga_sysref_ch = ADI_ADF4030_CHANNEL_ID_8,
    };
    err = adi_ads10_apollo_ex_adf4030_vco_freq_calc(mcs_tof.adf4030_ref_freq_hz, mcs_tof.bsync_out_sysref_freq_hz, &mcs_tof.vco_out_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // MCS Init Cal
    adi_apollo_mcs_cal_init_status_t init_cal_status = {{0}};

    // MCS Tracking cal
    uint16_t mcs_tracking_decimation = 1023;

    // ADF4382 Device Struct
    adi_adf4382_device_t adf4382 = {{0}};

    printf("dev_clk_hz: %lld.\n", dev_clk_hz);
    printf("divg_modulus: %d.\t sysref_prd_digclk_cycles: %d.\n", divg_modulus, sysref_prd_digclk_cycles);
    printf("int_sysref_freq_hz: %f.\n", int_sysref_freq_hz);
    printf("ext_sysref_freq_hz: %lld.\n", ext_sysref_freq_hz);
    printf("bsync_divider: %d.\n", mcs_tof.bsync_divider);
    printf("bsync_out_sysref_freq_hz: %lld.\n", mcs_tof.bsync_out_sysref_freq_hz);
    printf("adf4030_ref_freq_hz: %lld.\n", mcs_tof.adf4030_ref_freq_hz);
    printf("vco_out_freq_hz: %lld.\n\n", mcs_tof.vco_out_freq_hz);

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
                                                       mcs_tof.bsync_out_sysref_freq_hz, 1);
    ADI_CMS_ERROR_RETURN(err);

    // SYNC all BSYNC_OUT channels with BSYNC_IN Ref channel.
    err = adi_ads10_apollo_ex_adf4030_align_bsync_out(&adf4030,
                                                      bsync_in_ch_sel,
                                                      bsync_out_ch_sel,
                                                      mcs_tof.bsync_out_sysref_freq_hz);
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
                                                                        &adf4030_apollo_path_delay);
    ADI_CMS_ERROR_RETURN(err);

    // Offset path delay from BSYNC 5 output.
    err = adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_offset(&adf4030,
                                                                   &mcs_tof,
                                                                   align_cycles_iters,
                                                                   adf4030_apollo_path_delay);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 04. ADF4030 - FPGA Time-of-Flight Measurement and offset. */

    err = adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_measurement(&adf4030,
                                                                      fpga_device,
                                                                      &mcs_tof,
                                                                      &adf4030_fpga_path_delay);
    ADI_CMS_ERROR_RETURN(err);

    // Offset path delay from BSYNC 8 output.
    err = adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_offset(&adf4030,
                                                                 &mcs_tof,
                                                                 align_cycles_iters,
                                                                 adf4030_fpga_path_delay);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 05. SYNC all BSYNC_OUT channels with BSYNC_IN Ref channel. */

    err = adi_ads10_apollo_ex_adf4030_align_bsync_out(&adf4030,
                                                      bsync_in_ch_sel,
                                                      bsync_out_ch_sel,
                                                      mcs_tof.bsync_out_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 06. Perform MCS Init Cal for Sysref Alignment.
              Read back MCS init cal status data and validate init cal success. */

    printf("Running MCS Init Calibration...\n");
    err = adi_apollo_mcs_cal_init_run(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_mcs_cal_init_status_get(device, &init_cal_status);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_mcs_init_cal_validate(device, &init_cal_status, dev_clk_hz);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 07. Optional. Perform MCS Tracking Cal for maintaining alignment.*/

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


    // Measure ADF4030 DIE Temperature.
    err = adi_adf4030_core_die_temp_get(&adf4030, &die_temp);
    ADI_CMS_ERROR_RETURN(err);

    printf("ADF4030 Die Temp: %d *C.\n", die_temp);

    print_sysref_phase(device);
    print_link_phase(device);

    return API_CMS_ERROR_OK;
}


static int32_t check_sysref(adi_apollo_device_t *device)
{
#include <adi_apollo_bf_mcs_tdc.h>
#include <adi_apollo_bf_mcs_sync.h>

    uint8_t sysref_en = 0;
    uint8_t sysref_lvl = 0;
    uint8_t int_sysref_period[2];

    printf("\nBF_SYSREF_LEVEL_INFO(MCS_TDC_MCSTOP0):");
    for (uint16_t num = 0; num<32; num++) {
        adi_apollo_hal_bf_get(device, BF_SYSREF_LEVEL_INFO(MCS_TDC_MCSTOP0), &sysref_lvl, 1);
        if ((num % 16) == 0) {
            printf("\n");
        }
        printf("%d.  ", sysref_lvl);
        sysref_lvl = 0;
    }
    printf("\n");

    adi_apollo_hal_bf_get(device, BF_SYSREF_EN_INFO(MCS_TDC_MCSTOP0), &sysref_en, 1);
    printf("BF_SYSREF_EN_INFO(MCS_TDC_MCSTOP0): %d.\n", sysref_en);

    adi_apollo_hal_bf_get(device, BF_INT_SYSREF_PERIOD_INFO(MCS_SYNC_MCSTOP0), int_sysref_period, 2);
    printf("BF_INT_SYSREF_PERIOD_INFO(MCS_SYNC_MCSTOP0): %d.\n", (int_sysref_period[1] << 8) | int_sysref_period[0]);
    printf("\n");

    return 0;
}


static __maybe_unused void print_sysref_phase(adi_apollo_device_t* device) {
    uint32_t sysref_phase;
    for (int x = 0; x < 1; x++) {
        adi_apollo_clk_mcs_sysref_phase_get(device, &sysref_phase);
        printf("Apollo sysref_phase = 0x%x  %d\n", sysref_phase, sysref_phase);
    }
}

static __maybe_unused void print_link_phase(adi_apollo_device_t* device) {

    uint16_t jrx_phase_diff0, jrx_phase_diff1;
    adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_diff0);
    adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_diff1);
    printf("jrx_phase_diff0/jrx_phase_diff1 = %d %d\n", jrx_phase_diff0, jrx_phase_diff1);
}
#endif /* defined(__linux__) */
