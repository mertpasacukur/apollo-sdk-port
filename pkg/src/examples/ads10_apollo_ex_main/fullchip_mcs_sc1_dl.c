/*!
 * \brief     ADS10 Apollo fullchip Rx/Tx subclass 1 with deterministic latency
 *            using internal clocking scheme and multi-chip sync.
 *
 * This example demonstrates SC1 deterministic latency for Apollo 4T4R devices.
 * It is compatible with ADS10 based Apollo evaluation boards.
 *
 * Setup requires internal (on-board) clocking scheme.
 *
 *     +-----+
 *     |    <*>-----------------------------------------------------------------+
 *     | 1:2 |        BSYNC_1_P                                          ADC_B0 |
 *     |    <*>----------------------------------------------------------+      |
 *     +-<*>-+                                                    ADC_A0 |      |
 *        ^        +-----------------------------------------------------|------|-----+
 *        |        |                                                     v      v     |
 *        |        |                                                  +-<*>----<*>-+  |
 *        |        |  125MHz   +-----------+       DEVICE_CLK         |            |  |                      DAC_B0
 *        |   <*>--|-----+---->|  ADF4382  +------------------------> |           <*>------------------------------+
 *        |        |     |     +-----------+                          |   APOLLO  <*>--------------------------+   |
 *        |        |     |                               +----------> |            |  |                 DAC_A0 |   |
 *        |        |     |     +-----------+             |BSYNC_5     |            |  |                        |   |
 *        |        |     +---->|  HMC7044  +-------+     |(SYSREF)    +------------+  |                        |   |
 *        |        |           +---+---+---+       |     |                            |  +-------------+       |   |
 *        |        |           Ref |   |           |     |                 +-------+  |  |             |       |   |
 *        |        |           Clk |   |           +-----<---------------->|       +---->|             |       |   |
 *        |        |               |   |BSYNC_0          |    FPGA Ref     | FMC   |  |  |    FPGA     |       |   |
 *        |        |               v   v(Ref)            |                 | Conn. |  |  |             |       |   |
 *        |        |           +-----------+             |    +----------->|       |<--->|      	   |	   |   |
 *        |        |           |           |             |    | BSYNC_8    +-------+  |  +-----<*>-----+       |   |
 *        |        |           |           |<------------+    | (SYSREF)              |         |              |   |
 *  +----<*>----+  |           |           |                  |                       |         | TRIG         |   |
 *  |    P/N    |<------------>|  ADF4030  |<-----------------+                       |         |              |   |
 *  +----<*>----+  | BSYNC_1   |           |                                          |         |              |   |
 *        |        | (SYSREF)  |           |                                          |         |              |   |
 *        |        |           |           |                                          |         |              v   v
 *        |        |           +-----------+                                          |         |       +-----<*>-<*>---+
 *        |        |                                  FMCA-REVC EVAL BRD - INT CLK'D  |         +-----><*>              |
 *        |        +------------------------------------------------------------------+                 |     SCOPE     |
 *        |                                                                                             |               |
 *	      +-------------------------------------------------------------------------------------------><*>              |
 *                    BSYNC_1_N                                                                         +---------------+
 *
 *
 * Supported Device Profiles are list below along with corresponding clocking values. All Clk freq are in MHz.
 * +=================+============+================+=================+=============+=============+
 * | Device Profile  | Device Clk |  FPGA Ref  Clk | ADF4030 Ref Clk | ADF4030 VCO | SYSREF Clk  |
 * +=================+============+================+=================+=============+=============+
 * | id00_uc08sc1_f  |   20000    |     312.5      |     156.25      |    2500     |  9.765625   |
 * |  id00_uc13_sc1  |   20000    |     312.5      |     156.25      |    2500     |  9.765625   |
 * |  id00_uc14_sc1  |  19660.8   |     307.2      |      153.6      |   2380.8    |     9.6     |
 * |  id00_uc15_sc1  |   19200    |      300       |       150       |   2381.25   |    9.375    |
 * | id99_uc02c_sc1  |  19660.8   |     409.6      |      204.8      |   2457.6    |    12.8     |
 * +-----------------+------------+----------------+-----------------+-------------+-------------+
 *
 * First half of the example configures ADF4030 for SYSREF generation and Syncronization by measuring
 * Time-of-Flight between the SYSREF Source (ADF4030) and the devices receiving SYSREF (i.e Apollo and FPGA).
 * Additionally an extra SYSREF signal is also generated which can be used for observation.
 *
 * Following SYSREF config, we perform MCS init and tracking cal to align and maintain the sync between
 * Apollo's internal SYSREF and the external SYSREF. This involves setting up Apollo MCS FW and ADF4382 (Dev Clk Src).
 *
 * On the FPGA side, we configure the trigger pattern and set FMC as External SYSREF source. The Links are configured
 * and brought up after loading pattern vectors into FPGA producing square wave DAC outputs at the SYSREF freq.
 *
 * Trigger the scope with additional SYSREF output port or the start of pattern trigger from the ADS10.
 * Observe phase alignment from startup-to-startup. Also, DAC-A0 and DAC-B0 will be aligned to one another.
 * Captured ADC data contains edge transitions at the SYSREF intervals.
 * Samples from ADC-A0 and ADC-B0 will in phase from run to run. ADS10 capture triggers are re-timed to SYSREF.
 *
 * The example can be run with command line option '-jrx_adj' which will calculate
 * a phase adjust value that can be set for subsequent runs. This ensures that DAC
 * outputs are aligned with minimum skew.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
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
#include "adi_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_fpga_apollo_transmit.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_fpga_apollo_private.h"
#include "adi_apollo_bf_mcs_sync.h"
#include "adi_ads10_apollo_ex_mcs.h"
#include "ads10_fpga.h"

static int32_t fpga_sc1_config(adi_fpga_apollo_device_t* fpga_device);
static void print_sysref_phase(adi_apollo_device_t* device);
static void print_link_phase(adi_apollo_device_t* device);
static int32_t print_jrx_status(adi_apollo_device_t* device, uint16_t link);
static uint16_t jrx_phase_adj_const_get(adi_apollo_top_t *profile);

int32_t fullchip_mcs_sc1_dl(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                        int argc, char *argv[], int argc_ofst)
{
    int32_t err;
    char cal_data_file_name[MAX_PATH_LEN];      /* Filename for calData */
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    bool jrx_phase_adjust_calc_run = (argc > argc_ofst) && !strncmp("-jrx_adj", argv[argc_ofst], 7);  // default not run jrx phase adjust
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rxtx_dp_info;
    uint8_t subclass = profile->jtx[0].common_link_cfg.subclass == ADI_APOLLO_SUBCLASS_1;
    uint32_t sysref_per_div;
    uint16_t jrx_phase_adjust = 0; // Value can be obtained by running this example with -jrx_adj option

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
    uint32_t sysref_glblclk_ratio = (uint32_t) (fpga_ref_freq_hz / ext_sysref_freq_hz);

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

    printf("subclass: %d.\n", subclass);
    printf("dev_clk_hz: %lld.\n", dev_clk_hz);
    printf("divg_modulus: %d.\t sysref_prd_digclk_cycles: %d.\n", divg_modulus, sysref_prd_digclk_cycles);
    printf("int_sysref_freq_hz: %f.\n", int_sysref_freq_hz);
    printf("ext_sysref_freq_hz: %lld.\n", ext_sysref_freq_hz);
    printf("bsync_divider: %d.\n", mcs_tof.bsync_divider);
    printf("sysref_glblclk_ratio: %d.\n", sysref_glblclk_ratio);
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


    /* Run Clock Conditioning and ADC cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the jrx phase adjust */
    if (jrx_phase_adjust_calc_run) {
        jrx_phase_adjust = 0;               // Set to 0 if running with -jrx_adj option to obtain adjust value
    } else {
        jrx_phase_adjust = jrx_phase_adj_const_get(profile);
    }
    err = adi_apollo_jrx_phase_adjust_set(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, jrx_phase_adjust); // Obtained by running this with jrx_phase_adjust_calc_run = true
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_apollo_jtx_phase_adjust_set(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, 0);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Get datapath info. Assume Rx/Tx paths setup the same */
    err = adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rxtx_dp_info);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_ex_dp_info_to_str(&rxtx_dp_info, "Profile RxTx", str_buff, str_buff_len);
    ADI_CMS_ERROR_GOTO(err, end);
    printf("\n%s\n\n", str_buff);

    /*
     * Put CNCO and FNCO in Zero-IF mode. This will bypass the NCOs.
     * The pattern used to demonstrate SYSREF-to-DAC out is a square wave.
     */
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);
    err = adi_apollo_fnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);
    err = adi_apollo_fnco_mode_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Inspect the JRx/JTx link states */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Configure FPGA for SC1 */
    err = fpga_sc1_config(fpga_device);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     * Run the SERDES calibration
     *
     * NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly
     */
    if (adi_ads10_apollo_ex_run_serdes_init_cal_get(profile)) {
        /*
         * Create test vector and download to FPGA memory - all zeros
         */
        err = adi_ads10_apollo_ex_vec_constants_write(fpga_device, profile, ADI_APOLLO_SIDE_A | ADI_APOLLO_SIDE_B, 8192, 0);
        ADI_CMS_ERROR_GOTO(err, end);

        err = adi_apollo_clk_mcs_dyn_sync_sequence_run(device);
        ADI_CMS_ERROR_GOTO(err, end);

        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        err = adi_fpga_apollo_core_bidir_init(fpga_device);
        ADI_CMS_ERROR_GOTO(err, end);

        err = adi_apollo_hal_delay_us(device, 10000);
        ADI_CMS_ERROR_GOTO(err, end);

        err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    /* Check if running this example just to obtain the phase adjust */
    if (jrx_phase_adjust_calc_run) {
        err = adi_apollo_jrx_phase_adjust_calc(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, ADI_APOLLO_JRX_PHASE_ADJ_MARGIN_DEFAULT, &jrx_phase_adjust);
        ADI_CMS_ERROR_GOTO(err, end);
        printf("jrx_phase_adjust calculated: %d\n", jrx_phase_adjust);
        goto end;
    }

    /* Run ADC BG cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    /* Create test vector and download to FPGA memory - square at SYSREF freq */
    sysref_per_div = (rxtx_dp_info.fdata * 1e6) / (int_sysref_freq_hz);
    err = adi_ads10_apollo_ex_vec_square_write(fpga_device, profile, sysref_per_div, sysref_per_div, 1, 0.10, -3.0);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_apollo_clk_mcs_dyn_sync_sequence_run(device);
    ADI_CMS_ERROR_GOTO(err, end);

    if (interactive) {
        printf("*** Configure scope for Tx DL measurement ***\n");
        printf("(<CR> to continue)\n");
        getchar();
    }

    /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_GOTO(err, end);

    print_sysref_phase(device);
    print_link_phase(device);
    print_jrx_status(device, ADI_APOLLO_LINK_A0);
    print_jrx_status(device, ADI_APOLLO_LINK_B0);

    /* Take several ADC captures while DAC is transmitting */
    uint32_t cap_cnt = 0;
    while (cap_cnt < 1) {
        if (profile->jtx->tx_link_cfg[0].np_minus1 == 11) {
            snprintf(cal_data_file_name, MAX_PATH_LEN, "%s_cap_cnt_%02d", "fullchip_sc1_dl_data12", cap_cnt);
        } else {
            snprintf(cal_data_file_name, MAX_PATH_LEN, "%s_cap_cnt_%02d", "fullchip_sc1_dl_data16", cap_cnt);
        }

        printf("\ncap_cnt = %d\n", cap_cnt++);
        /* Read FPGA capture memory and write out non-interleaved i/q files */
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, DEFAULT_NUM_SAMPLES_H, cal_data_file_name, false);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    printf("Done with capture\n");

end:
    return err;
}

static __maybe_unused int32_t fpga_sc1_config(adi_fpga_apollo_device_t* fpga_device)
{
    int32_t err = API_CMS_ERROR_OK;

    /*
     * Enable ADS10 pattern start output trigger (0 to 1) output on SMA J3.
     */
    err = adi_fpga_apollo_core_sys_indicator_set(fpga_device, ADI_FPGA_APOLLO_TX_PAT_START_INDICATOR);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     * Select ADS10 external SYSREF source, input from FMC
     */
    err = adi_fpga_apollo_core_sysref_src_set(fpga_device, ADI_FPGA_APOLLO_SYSREF_SRC_FMC);
    ADI_CMS_ERROR_GOTO(err, end);

end:
    return err;
}

static __maybe_unused void print_link_phase(adi_apollo_device_t* device) {

    uint16_t jrx_phase_diff0, jrx_phase_diff1;
    uint16_t jrx_phase_adjust0, jrx_phase_adjust1;

    adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_diff0);
    adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_diff1);
    printf("jrx_phase_diff0/jrx_phase_diff1 = %d %d\n", jrx_phase_diff0, jrx_phase_diff1);

    adi_apollo_jrx_phase_adjust_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_adjust0);
    adi_apollo_jrx_phase_adjust_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_adjust1);
    printf("jrx_phase_adjust0/jrx_phase_adjust1 = %d %d\n", jrx_phase_adjust0, jrx_phase_adjust1);
}

static __maybe_unused void print_sysref_phase(adi_apollo_device_t* device) {
    uint32_t sysref_phase;
    for (int x = 0; x < 1; x++) {
        adi_apollo_clk_mcs_sysref_phase_get(device, &sysref_phase);
        printf("Apollo sysref_phase = 0x%x  %d\n", sysref_phase, sysref_phase);
    }
}

static __maybe_unused int32_t print_jrx_status(adi_apollo_device_t* device, uint16_t link)
{
    int32_t err;
    uint16_t link_status;
    err = adi_apollo_jrx_link_status_get(device, link, &link_status);
    ADI_CMS_ERROR_RETURN(err);

    printf("JRx ready: 0x%x  SYSREF phase lock: 0x%x\n", (link_status & 0x20) >> 5, (link_status & 0x40) >> 6);

    // return 1 if link up, else 0
    return (link_status & ADI_APOLLO_JRX_LINK_STAT_USR_DAT_RDY) && (link_status & ADI_APOLLO_JRX_LINK_STAT_SYSREF_RECV);
}

static uint16_t jrx_phase_adj_const_get(adi_apollo_top_t *profile)
{
    uint32_t jrx_slr = profile->jrx[0].common_link_cfg.lane_rate_kHz;

    // phase adjust vals from running this example with -jrx_adj option over several profiles
    // From ADS10 w/ FMCB
    if (jrx_slr <= 10312500) {
        return 6;
    } else if (jrx_slr <= 20625000) {
        return 14;
    } else if (jrx_slr <= 27033600) {
        return 27;
    } else {
        return 33;
    }
}
