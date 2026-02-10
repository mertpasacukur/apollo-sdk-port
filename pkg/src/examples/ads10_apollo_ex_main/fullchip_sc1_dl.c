/*!
 * \brief     ADS10 Apollo fullchip Rx/Tx subclass 1 with deterministic latency
 *
 * This example demonstrates SC1 deterministic latency for Apollo 4T4R devices. It 
 * is compatible with ADS10 based Apollo evaluation boards.
 * 
 * Setup uses external clocking, but should apply to other clocking options.
 * 
 * 
 *                   |---------- Apollo SYSREF-P
 * (ext clk)         |
 * SYSREF-P----------|
 *                   |
 *                   |---------- ADS10 SYSREF
 * 
 * 
 *                                                  |---------- Apollo ADC-A0
 *                   |------------------------------|
 *                   |                              |---------- Apollo ADC-B0
 *                   |
 * SYSREF-N----------|
 *                   |
 *                   |---------- Apollo SYSREF-N
 * 
 * FMC Board - Apollo
 * ------------------
 *  FClk
 *  SYSREF
 *  ADC-A0 and ADC-B0 => SYSREF
 *  DAC-A0 and DAC-B0 => To Scope
 *
 * ADS10 FPGA Board
 * -------------------
 * J1 FPGA ref clock (in)
 * J2 SYSREF (in)
 * J3 Start of pattern trig (out)
 * 
 * 
 * Trigger the scope with SYSREF (can be from a secondary phase locked sig gen) or the start of pattern
 * trigger from the ADS10. Pattern vectors produce square wave DAC outputs at the SYSREF freq. Observe phase 
 * alignment from startup-to-startup. Also, DAC-A0 and DAC-B0 will be aligned to one another.
 * 
 * Capture data is contains edge transitions at the SYSREF intervals. Samples
 * from ADC-A0 and ADC-B0 will in phase from run to run. ADS10 capture triggers
 * are re-timed to SYSREF.
 * 
 * Device Profile   Fclk        FPGA Ref        SYSREF
 * ==============   ====        ========        ======
 * id00_uc06sc1     20000.0     156.25          4.8828125
 * id00_uc08sc1_f   20000.0     312.5           9.765625
 * id99_uc15_sc1    19200.0     300.0           9.375
 * id99_uc02c_sc1   19660.8     409.6           12.8
 * 
 *
 * The example can be run with command line option '-jrx_adj' which will calculate
 * a phase adjust value that can be set for subsequent runs. This ensures that DAC
 * outputs are aligned with minimum skew. 
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

static int32_t apollo_sc1_config(adi_apollo_device_t* device, uint8_t subclass, double dev_clk_freq, double sysref_freq);
static int32_t fpga_sc1_config(adi_fpga_apollo_device_t* fpga_device);
static void print_sysref_phase(adi_apollo_device_t* device);
static void print_link_phase(adi_apollo_device_t* device);
static int32_t print_jrx_status(adi_apollo_device_t* device, uint16_t link);
static uint16_t jrx_phase_adj_const_get(adi_apollo_top_t *profile);

int32_t fullchip_sc1_dl(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                        int argc, char *argv[], int argc_ofst)
{

    int32_t err;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    bool jrx_phase_adjust_calc_run = (argc > argc_ofst) && !strncmp("-jrx_adj", argv[argc_ofst], 7);  // default not run jrx phase adjust
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rxtx_dp_info;
    uint32_t fpga_image_ver;
    uint8_t subclass = profile->jtx[0].common_link_cfg.subclass == ADI_APOLLO_SUBCLASS_1;
    double sysref_freq = profile->jtx[0].common_link_cfg.lane_rate_kHz / (66 * 32 * 1000.0);        // Assume same sysref for all links, JESD204C
    const uint32_t adc_cap_buff_num= 4;
    uint16_t *adc_cap_buff[adc_cap_buff_num];
    uint32_t samples_per_conv;
    uint32_t sysref_per_div;
    uint16_t jrx_phase_adjust = 0; // Value can be obtained by running this example with -jrx_adj option

    printf("subclass: %d sysref: %f\n", subclass, sysref_freq);

    /*
     * print FPGA image version
     */
    err = adi_fpga_apollo_core_image_ver_get(fpga_device, &fpga_image_ver);
    ADI_CMS_ERROR_GOTO(err, end);
    printf("fpga_image_ver = 0x%08x\n", fpga_image_ver);

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

    /* Configure Apollo for SC1 */
    err = apollo_sc1_config(device, subclass, rxtx_dp_info.fclk, sysref_freq);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Run Clock Conditioning and ADC cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);
  
    /*
     * Run the SERDES calibration
     *
     * NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly
     */
    /* Create test vector and download to FPGA memory - all zeros */
    err = adi_ads10_apollo_ex_vec_constants_write(fpga_device, profile, ADI_APOLLO_SIDE_A | ADI_APOLLO_SIDE_B, 8192, 0);
    ADI_CMS_ERROR_GOTO(err, end);

    /* SYSREF alignment (HW method) */
    err = adi_apollo_clk_mcs_sync_hw_align_set(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Dynamic Sync Serdes Links gradually in a sequence */
    printf("Run Rx-TX SerDes Links Dyn Sync...\n");
    err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Allow clks and supplies to settle after sync */
    err = adi_apollo_hal_delay_us(device, 1000);
    ADI_CMS_ERROR_RETURN(err);
    
    /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Check if running this example just to obtain the phase adjust */
    if (jrx_phase_adjust_calc_run) {
        err = adi_apollo_jrx_phase_adjust_calc(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, ADI_APOLLO_JRX_PHASE_ADJ_MARGIN_DEFAULT, &jrx_phase_adjust);
        ADI_CMS_ERROR_GOTO(err, end);
        printf("jrx_phase_adjust calculated: %d\n", jrx_phase_adjust);
        goto end;
    }

    /* Run ADC BG cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Create test vector and download to FPGA memory - square at SYSREF freq */
    sysref_per_div = (rxtx_dp_info.fdata * 1e9) / (sysref_freq * 1e9);
    err = adi_ads10_apollo_ex_vec_square_write(fpga_device, profile, sysref_per_div, sysref_per_div, 1, 0.10, -3.0);
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
        printf("\ncap_cnt = %d\n", cap_cnt++);

        /* Read FPGA capture memory and write out i/q files. Used for HW FSRC and non-FSRC data */
        if (profile->jtx->tx_link_cfg[0].np_minus1 == 11) {
            /* Read FPGA capture memory and write out non-interleaved i/q files */
            err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, 1024 * 64, "fullchip_sc1_dl_data12", false);
            ADI_CMS_ERROR_GOTO(err, end);

            /* Capture data into chan I/Q arrays */
            err = adi_ads10_apollo_ex_fpga_capture_to_array_12b(device, fpga_device, &samples_per_conv, adc_cap_buff, adc_cap_buff_num);
            ADI_CMS_ERROR_GOTO(err, end);
        } else {
            /* Read FPGA capture memory and write out interleaved i/q files */
            err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, 1024 * 64, "fullchip_sc1_dl_data16", false);
            ADI_CMS_ERROR_GOTO(err, end);

            /* Capture data into chan I/Q arrays */
            err = adi_ads10_apollo_ex_fpga_capture_to_array(device, fpga_device, &samples_per_conv, adc_cap_buff, adc_cap_buff_num);
            ADI_CMS_ERROR_GOTO(err, end);
        }

        printf("samples_per_conv %d\n", samples_per_conv);

        /* free memory allocated by adi_ads10_apollo_ex_fpga_capture_to_array() */
        for (int k = 0; k < 4; k++) {
            free(adc_cap_buff[k]);
        }
    }

    printf("Done with capture\n");

end:

    return err;

}

static int32_t apollo_sc1_config(adi_apollo_device_t* device, uint8_t subclass, double dev_clk_freq, double sysref_freq)
{
    int32_t err = API_CMS_ERROR_OK;

    /*
     * Set the subclass mode for JESD JRx and JTx links
     */
    adi_apollo_jrx_subclass_set(device, ADI_APOLLO_LINK_ALL, subclass);
    adi_apollo_jtx_subclass_set(device, ADI_APOLLO_LINK_ALL, subclass);

    /*
     * Set the subclass mode for Apollo MCS
     */
    adi_apollo_clk_mcs_subclass_set(device, subclass);

    /*
    * Set the MCS internal SYSREF period
    *
    * internal_sysref_per = (Fclk/Fsysref)/8
    *
    * For example:
    * Fclk = 20000, Fsysref = 4.8828125
    *
    * internal_sysref_per = (20000/4.8828125)/8 = 512;
    *
    */
    adi_apollo_clk_mcs_internal_sysref_per_set(device, (uint16_t) ((dev_clk_freq / sysref_freq / 8) + 0.5));

    /*
     * Enable the MCS SYSREF receiver if subclass 1
     */
    adi_apollo_clk_mcs_sysref_en_set(device, (subclass == 1) ? ADI_APOLLO_ENABLE : ADI_APOLLO_DISABLE);

    return err;
}

static int32_t fpga_sc1_config(adi_fpga_apollo_device_t* fpga_device)
{
    int32_t err = API_CMS_ERROR_OK;

    /*
     * Enable ADS10 pattern start output trigger (0 to 1) output on SMA J3.
     */
    err = adi_fpga_apollo_core_sys_indicator_set(fpga_device, ADI_FPGA_APOLLO_TX_PAT_START_INDICATOR);
    ADI_CMS_ERROR_RETURN(err);

    /*
     * Select ADS10 external SYSREF source, input on SMA J5
     */
    err = adi_fpga_apollo_core_sysref_src_set(fpga_device, ADI_FPGA_APOLLO_SYSREF_SRC_EXT);
    ADI_CMS_ERROR_RETURN(err);

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
