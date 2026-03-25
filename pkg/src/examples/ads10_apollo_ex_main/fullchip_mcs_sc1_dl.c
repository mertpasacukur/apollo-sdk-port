#if !defined(VERSAL_PLATFORM)
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
 *        |  J17/  |                                                  +-<*>----<*>-+  |
 *        |  XTAL  |  125MHz   +-----------+       DEVICE_CLK         |            |  |                      DAC_B0
 *        |   <*>--|-----+---->|  ADF4382  +------------------------> |           <*>------------------------------+
 *        |        |     |     +-----------+                          |   APOLLO  <*>--------------------------+   |
 *        |        |     |                               +----------> |            |  |                 DAC_A0 |   |
 *        |        |     |     +-----------+             |BSYNC_n*    |            |  |                        |   |
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
 * It is recommended to first run the example multiple times using the -jrx_adj command-line option and record the calculated jrx_phase_adjust values.
 * After obtaining an average phase adjustment value for a given profile across several runs,
 * use that value in subsequent executions by setting it in jrx_phase_adj_const_get(). This approach helps ensure that the DAC outputs remain aligned with minimal skew.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
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
static int32_t print_jrx_status(adi_apollo_device_t* device, uint16_t link);
static uint16_t jrx_phase_adj_const_get(adi_apollo_top_t *profile);

int32_t fullchip_mcs_sc1_dl(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                        int argc, char *argv[], int argc_ofst)
{
    int32_t err;
    char cal_data_file_name[MAX_PATH_LEN];      /* Filename for calData */
    bool interactive = false;   // cmd arg: -i. default not interactive
    bool jrx_phase_adjust_calc_run = false;   // cmd arg: -jrx_adj. default not run jrx phase adjust
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rxtx_dp_info;
    uint32_t sysref_per_div;
    uint16_t jrx_phase_adjust = 0; // Value can be obtained by running this example with -jrx_adj option
    uint64_t dev_clk_hz = (uint64_t) profile->clk_cfg.dev_clk_freq_kHz * 1e3;
    uint8_t divg_modulus = (profile->clk_cfg.clocking_mode == 0) ? 8 : 4;   // divg is global digital divider
    uint32_t sysref_prd_digclk_cycles = profile->mcs_cfg.internal_sysref_prd_digclk_cycles_center;
    double int_sysref_freq_hz = (double) (dev_clk_hz) / (divg_modulus * sysref_prd_digclk_cycles);

    // Use strlen(flag) + 1 in strncmp() to include the null terminator for exact string matching
    for (uint8_t i = argc_ofst; i < argc; ++i) {
        if (strncmp(argv[i], "-i", 3) == 0) {
            interactive = true;
        } else if (strncmp(argv[i], "-jrx_adj", 9) == 0) {
            jrx_phase_adjust_calc_run = true;
        }
    }

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

    /* Run Clock Conditioning cal */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Top level function to generate SYSREF from on-board clock chip and perform internal-external SYSREF alignment */
    printf("Using ADF4030 for SYSREF generation and MCS Cal(SW) for it's alignment.\n");
    err = adi_ads10_apollo_ex_mcs_sysref_gen_align_cal_setup(device, fpga_device, profile);
    ADI_CMS_ERROR_RETURN(err);

    /* Run ADC FG cal */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the jrx phase adjust */
    if (jrx_phase_adjust_calc_run) {
        jrx_phase_adjust = 0;               // Set to 0 if running with -jrx_adj option to obtain adjust value
    } else {
        jrx_phase_adjust = jrx_phase_adj_const_get(profile);
        printf("jrx_phase_adjust: %d.\n", jrx_phase_adjust);
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
        err = adi_ads10_apollo_ex_vec_constants_incr_write(fpga_device, profile, NULL, ADI_APOLLO_LINK_ALL, 8192, 0, 0);
        ADI_CMS_ERROR_GOTO(err, end);

        err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(device);
        ADI_CMS_ERROR_GOTO(err, end);

        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        err = adi_fpga_apollo_core_bidir_init(fpga_device);
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
    err = adi_ads10_apollo_ex_vec_square_write(fpga_device, profile, NULL, ADI_APOLLO_LINK_ALL, EX_VEC_DEFAULT_SAMPLES_PER_VC, sysref_per_div, 1, 0.10, -3.0);
    ADI_CMS_ERROR_GOTO(err, end);

    if (interactive) {
        printf("*** Configure scope for Tx DL measurement ***\n");
        printf("(<CR> to continue)\n");
        getchar();
    }

    /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_apollo_ex_mcs_sysref_phase_print(device);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_mcs_link_phase_print(device);
    ADI_CMS_ERROR_RETURN(err);

    printf("ADI_APOLLO_LINK_A0 UP: %s.\n", (print_jrx_status(device, ADI_APOLLO_LINK_A0) == 1) ? "True" : "False");
    printf("ADI_APOLLO_LINK_B0 UP: %s.\n", (print_jrx_status(device, ADI_APOLLO_LINK_B0) == 1) ? "True" : "False");

    /* Take several ADC captures while DAC is transmitting */
    uint32_t cap_cnt = 0;
    while (cap_cnt < 1) {
        if (profile->jtx->tx_link_cfg[0].np_minus1 == 11) {
            snprintf(cal_data_file_name, MAX_PATH_LEN, "%s_cap_cnt_%02d", "fullchip_mcs_sc1_dl_data12", cap_cnt);
        } else {
            snprintf(cal_data_file_name, MAX_PATH_LEN, "%s_cap_cnt_%02d", "fullchip_mcs_sc1_dl_data16", cap_cnt);
        }

        printf("\ncap_cnt = %d\n", cap_cnt++);
        /* Read FPGA capture memory and write out non-interleaved i/q files */
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, DEFAULT_NUM_SAMPLES_H, cal_data_file_name, true, false);
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
    printf("jrx_slr: %d.\n", jrx_slr);

    // phase adjust vals from running this example with -jrx_adj option over several profiles
    // From ADS10 w/ FMCB
    if (jrx_slr <= 10312500) {
        return 6;
    } else if (jrx_slr <= 20625000) {
        return 11;
    } else if (jrx_slr <= 27033600) {
        return 21;
    } else {
        return 33;
    }
}

#endif /* !defined(VERSAL_PLATFORM) */
