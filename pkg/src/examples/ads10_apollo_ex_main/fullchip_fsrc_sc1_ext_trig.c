#if !defined(VERSAL_PLATFORM)
﻿/*!
 * \brief     ADS10 Apollo Fullchip Dynamic Reconfig w/ SC1 and ext Trig (FSRC, CDDC/FDDC, CDUC/FDUC)
 * 
 * 
 * This example expects the following configuration:
 * 
 *  ADC chans A0 & B0 inputs  are connected to SYSREF (square wave)
 *  DAC chans A0 & B0 outputs are connected to digital scope (pulse pattern)
 * 
 * For DL verification, the ADC captures and DAC outputs will have same delay from ext trigger from startup to startup. 
 * 
 * - Trigger the scope with the ext trigger going to Apollo's ext trigger input.
 * - For SC1, the trigger to edge delay should be the same from startup=to-startup.
 *   Note that different configurations (e.g. fsrc ratio, duc/ddc) will have 
 *   different delays due to filter delay characteristics. However, individually they will 
 *   be the consistent.
 * - Individual capture files (I chan only) for each dr_config and channel are created.
 *   File format is: "cap-drconfig-", drconfig #, ADC chan, cap # within startup (currently 2)
 *   For example: cap-drconfig-001_a0_0.txt_I 
 *      profile #1, ADC-A0, 0th (first of 2) capture
 * 
 * Channel array assignment used throughout example (MAX_CHANS = 4, assumes 4T4R device)
 *
 *  Chan    Index   Side    Side-Index
 *  ----    -----   ----    ----------
 *   A0       0       A        0
 *   A1       1       A        1
 *   B0       2       B        0
 *   B1       3       B        1
 *
 * A list of profiles contains different FSRC, DUC and DDC configurations. Individual channels may
 * have different values. By default, the examples sets them all the same. See 'rx_tx_dr_configs'.
 * 
 * Note:
 * - This example is dependent on MCS type FPGA images.
 *   The image has HW FSRC, SYSREF Seq, ext SYSREF, cap delay.
 * - ADC-A0 and ADC-B0 have SYSREF connected from sig gen.
 * - ADC-A1 and ADC-B1 are looped back to DAC-A1 and DAC-B1, respectively. 
 * - DDC/DUC are put in Zero-If mode (no NCO) in order to see edges for DL.
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
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_transmit.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_fpga_apollo_private.h"
#include "adi_apollo_bf_mcs_sync.h"

#define MAX_CHANS 4
#define MAX_DR_CONFIGS 16
#define SAMPLES_PER_VC (EX_VEC_DEFAULT_SAMPLES_PER_VC * 2)

typedef struct {
    char* name;
    uint32_t   fsrc_n;                 /* FSRC N val from cfg */
    uint32_t   fsrc_m;                 /* FSRC M val from cfg */
    adi_apollo_coarse_ddc_dcm_e cddc_dcm;   /* CDDC decimation select. (Not supported in Ax Si) */
    adi_apollo_fddc_ratio_e fddc_dcm;       /* FDDC decimation select */

    /* These fields will be calculated */
    double  bb_tone_freq;           /* Target baseband freq */
    double  rf_tone_freq;           /* Target RF input (or target 'bb_tone_freq' if set to 0.0) */
    double  fdata;                  /* Effective baseband FSRC data rate */
    double  fsrc_ratio;             /* Calculated FSRC ratio N/M (1.0 > N/M < 2.0) */
    uint32_t cddc_dcm_val;          /* CDDC decimation numeric val */
    uint32_t fddc_dcm_val;          /* FDDC decimation numeric val */
} rx_fsrc_dr_cfg_t;

typedef struct {
    char* name;
    uint32_t   fsrc_n;                 /* FSRC N val from cfg */
    uint32_t   fsrc_m;                 /* FSRC M val from cfg */
    adi_apollo_coarse_duc_dcm_e cduc_interp;    /* CDUC interpolation select (not supported in Ax Si) */
    adi_apollo_fduc_ratio_e fduc_interp;        /* FDUC interpolation select */

    /* These fields will be calculated */
    double  bb_tone_freq;           /* Target baseband freq */
    double  rf_tone_freq;           /* Target RF output (or target 'bb_tone_freq' if set to 0.0) */
    double  fdata;                  /* Effective baseband FSRC data rate */
    double  fsrc_ratio;             /* Calculated FSRC ratio N/M (1.0 > N/M < 2.0) */
    uint32_t cduc_interp_val;       /* CDUC interpolation numeric val */
    uint32_t fduc_interp_val;       /* FDUC interpolation numeric val */
} tx_fsrc_dr_cfg_t;

typedef struct {
    char name[80];     /* profile name */
    rx_fsrc_dr_cfg_t* rx_chan[MAX_CHANS];
    tx_fsrc_dr_cfg_t* tx_chan[MAX_CHANS];
} rx_tx_reconfig_profile_t;

typedef enum {
    PULSE_10,           // pulse
    TONE_0P40_FS,       // 0.40 x Fdata
    TONE_0P05_FS,       // 0.05 x Fdata
    TONE_0P015625_FS    // 0.015625 x Fdata (1/64 = 0.015625)
} vec_type_e;

typedef struct {
    double      fin;
    double      fdata;
    uint32_t    pk_loc;
    int16_t     pk_val;
    bool        pos_pk;     // Search for pos/neg peak. 
} rx_chan_data_t;

static __maybe_unused void print_trig_phase(adi_apollo_device_t* device);
static __maybe_unused void print_reconfig_count(adi_apollo_device_t* device, adi_apollo_terminal_e terminal);

static int32_t reconfig_setup(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, rx_tx_reconfig_profile_t* rec_profile, adi_ads10_apollo_dp_info_t *tx_dp_info);
static int32_t reconfig_trig(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, char* desc, rx_chan_data_t *chans, uint32_t n_chans);

static int32_t apollo_fsrc_config(adi_apollo_device_t* device, uint8_t subclass, double dev_clk_freq, double sysref_freq);
static int32_t fpga_fsrc_config(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t subclass,
                                adi_ads10_apollo_dp_info_t *rx_dp_info, adi_ads10_apollo_dp_info_t *tx_dp_info);

static void print_sysref_phase(adi_apollo_device_t* device);

static int32_t fpga_create_and_load_vec(adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, vec_type_e vec_type);

static char* rx_fsrc_dr_cfg_tostr(rx_fsrc_dr_cfg_t* fsrc_ratio_cfg, char* buff);
static char* tx_fsrc_dr_cfg_tostr(tx_fsrc_dr_cfg_t* fsrc_ratio_cfg, char* buff);
static void print_link_phase(adi_apollo_device_t *device);
static uint16_t jrx_phase_adj_const_get(adi_apollo_top_t *profile);

static int num_dyn_recs;
static int num_dr_configs;

/* Rx FSRC dynamic reconfig test cases */
static rx_fsrc_dr_cfg_t rx_fsrc_dr_cfgs[] = {
    /* Name                 FSRC-N      FSRC-M          CDDC                    FDDC                        BB tone   ADC input */
    {"c4_f2_n15625_m1228",  15625,      12288,          ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_2,    50.0,     0.0},
    {"c4_f4_n15625_m1228",  15625,      12288,          ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_4,    50.0,     0.0},
    {"c4_f8_n15625_m1228",  15625,      12288,          ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_8,    50.0,     0.0},
    {"c4_f16_n15625_m1228", 15625,      12288,          ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_16,   50.0,     0.0},

    {"c4_f2_n100_m99",      100,        99,             ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_2,    50.0,     0.0},
    {"c4_f4_n100_m99",      100,        99,             ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_4,    50.0,     0.0},
    {"c4_f8_n100_m99",      100,        99,             ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_8,    50.0,     0.0},
    {"c4_f16_n100_m99",     100,        99,             ADI_APOLLO_CDDC_DCM_4,  ADI_APOLLO_FDDC_RATIO_16,   50.0,     0.0}
};
static int rx_num_dyn_recs = sizeof(rx_fsrc_dr_cfgs) / sizeof(rx_fsrc_dr_cfgs[0]);

/* Tx FSRC dynamic reconfig test cases */
static tx_fsrc_dr_cfg_t tx_fsrc_dr_cfgs[] = {
    /* Name                 FSRC-N      FSRC-M          CDUC                       FDUC                        BB tone   DAC output */
    {"c4_f2_n15625_m1228",  15625,      12288,          ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_2,    50.0,     0.0},
    {"c4_f4_n15625_m1228",  15625,      12288,          ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_4,    50.0,     0.0},
    {"c4_f8_n15625_m1228",  15625,      12288,          ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_8,    50.0,     0.0},
    {"c4_f16_n15625_m1228", 15625,      12288,          ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_16,   50.0,     0.0},

    {"c4_f2_n100_m99",      100,        99,             ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_2,    50.0,     0.0},
    {"c4_f4_n100_m99",      100,        99,             ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_4,    50.0,     0.0},
    {"c4_f8_n100_m99",      100,        99,             ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_8,    50.0,     0.0},
    {"c4_f16_n100_m99",     100,        99,             ADI_APOLLO_CDUC_INTERP_4,  ADI_APOLLO_FDUC_RATIO_16,   50.0,     0.0}
};
static int tx_num_dyn_recs = sizeof(tx_fsrc_dr_cfgs) / sizeof(tx_fsrc_dr_cfgs[0]);
static bool is_valid_rate_change(adi_ads10_apollo_dp_info_t *rx_dp_info, adi_ads10_apollo_dp_info_t *tx_dp_info,
                                 rx_tx_reconfig_profile_t *rx_tx_dr_configs, int max_chans);

int32_t fullchip_fsrc_sc1_ext_trig(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                     int argc, char *argv[], int argc_ofst) {

    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2);  // default not interactive
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    char str_buff2[64];
    uint16_t link_status_a0, link_status_b0;

    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl
    adi_ads10_apollo_dp_info_t rx_dp_info[MAX_CHANS];   // Device profile data path info (RX)
    adi_ads10_apollo_dp_info_t tx_dp_info[MAX_CHANS];   // Device profile data path info (TX)
    uint32_t fpga_image_ver;
    uint8_t subclass = (profile->jtx[0].common_link_cfg.subclass == ADI_APOLLO_SUBCLASS_1) ? 1 : 0;         // Assume entire profile is SC1 or not

    double sysref_freq = profile->jtx[0].common_link_cfg.lane_rate_kHz / (66 * 32 * 1000.0);        // Assume same sysref for all links, JESD204C

    rx_chan_data_t rx_chan_dat[MAX_CHANS];
    rx_tx_reconfig_profile_t rx_tx_dr_configs[MAX_DR_CONFIGS];       // List of reconfig dr_configs. Each dr_config defines a config per chan.
    uint32_t pause_us = 1000000;

    /* Check that the number of Rx and Tx test cases match */
    if (tx_num_dyn_recs != rx_num_dyn_recs) {
        printf("This example requires Rx and Tx test cases to be of same length\n");
        ADI_CMS_ERROR_GOTO(API_CMS_ERROR_ERROR, end);
    }

    num_dyn_recs = rx_num_dyn_recs;

    printf("subclass: %d sysref: %f\n", subclass, sysref_freq);

    /*
     * Check FPGA image version is compatible with FSRC w/ SC1
     *
     * Note: This example only supports ADS10 FPGA MCS (multi-chip-sync) designs 
     */
    adi_fpga_apollo_core_image_ver_get(fpga_device, &fpga_image_ver);
    printf("FPGA image ver: 0x%08x\n", fpga_image_ver);
    
    if (fpga_device->state_info.design_id != ADI_FPGA_APOLLO_DESIGN_MCS) {
        printf("This example only runs with MCS typw FPGA images.\n");
        ADI_CMS_ERROR_GOTO(API_CMS_ERROR_NOT_SUPPORTED, end);
    }

    /*
     * Get the FPGA features supported (such as HW transport layer and FSRC)
     */
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_GOTO(err, end);

    if (!fpga_feature_flags.tx_hw_fsrc) {
        printf("This example only runs with FPGA images supporting HW FSRC\n");
        ADI_CMS_ERROR_GOTO(API_CMS_ERROR_NOT_SUPPORTED, end);
    }
    
    /* 
     * Inspect the JRx/JTx link states and print to console
     */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_GOTO(err, end);

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_GOTO(err, end);
    
    /* 
     * Get Rx and Tx datapath info from device profile and populate utility struct
     */
    for (int k = 0; k < MAX_CHANS; k++) {
        adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A + k / 2, k % 2, &rx_dp_info[k]);
        snprintf(str_buff2, sizeof(str_buff2), "RX %c%d", 'A' + k / 2, k % 2);
        adi_ads10_ex_dp_info_to_str(&rx_dp_info[k], str_buff2, str_buff, str_buff_len);
        printf("\n%s\n", str_buff);
    }
    printf("\n");
    for (int k = 0; k < MAX_CHANS; k++) {
        adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A + k / 2, k % 2, &tx_dp_info[k]);
        snprintf(str_buff2, sizeof(str_buff2), "TX %c%d", 'A' + k / 2, k % 2);
        adi_ads10_ex_dp_info_to_str(&tx_dp_info[k], str_buff2, str_buff, str_buff_len);
        printf("\n%s\n", str_buff);
    }
    /* Run Clock Conditioning and ADC cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the jrx phase adjust */
    err = adi_apollo_jrx_phase_adjust_set(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, jrx_phase_adj_const_get(profile));
    ADI_CMS_ERROR_RETURN(err);

    /* Set the jtx phase adjust */
    err = adi_apollo_jtx_phase_adjust_set(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, 0);
    ADI_CMS_ERROR_GOTO(err, end);

    /* 
     * Put CNCO and FNCO in Zero-IF mode. This bypasses the NCOs.
     *
     * Note: Pulse patterns are used to measure deterministic latency. Therefore, the mixers are bypassed
     *       so that waveform edges can be detected.
     */
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);
    err = adi_apollo_fnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);
    err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);
    err = adi_apollo_fnco_mode_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_MXR_ZERO_IF_MODE);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     *  Configure Apollo for FSRC (some device profile settings are overridden)
     */
    err = apollo_fsrc_config(device, subclass, rx_dp_info[0].fclk, sysref_freq);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     * Configure FPGA for FSRC
     */
    err = fpga_fsrc_config(device, fpga_device, subclass, rx_dp_info, tx_dp_info);
    ADI_CMS_ERROR_GOTO(err, end);

    
    /*
     * Create test vector and download to FPGA memory - all zeros
     */
    err = adi_ads10_apollo_ex_vec_constants_write(fpga_device, profile, ADI_APOLLO_SIDE_A | ADI_APOLLO_SIDE_B, 8192, 0);
    ADI_CMS_ERROR_GOTO(err, end);

    /* SYSREF alignment (HW method) */
    err = adi_apollo_clk_mcs_sync_hw_align_set(device);
    ADI_CMS_ERROR_GOTO(err, end);

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

    err = adi_apollo_hal_delay_us(device, 10000);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     * Run the SERDES calibration
     *
     * NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly
     */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     * Create test vector and download to FPGA memory
     *
     * PULSE_10         - 10 sample wide pulse. Vec len is SAMPLES_PER_VC.
     * TONE_0P40_FS     - Single tone at 0.40 x Fdata. Vec len is SAMPLES_PER_VC.
     * TONE_0P05_FS     - Single tone at 0.05 x Fdata. Vec len is SAMPLES_PER_VC.
     * TONE_0P015625_FS - Single tone at 0.015625 x Fdata, Vec len is SAMPLES_PER_VC.
     */
    err = fpga_create_and_load_vec(fpga_device, profile, PULSE_10);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Run ADC BG cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    printf("Starting FPGA BDIR sequence...\n");
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Reset the rm fifo - will clean up DAC tone output */
    err = adi_apollo_jrx_rm_fifo_reset(device, ADI_APOLLO_LINK_SIDE_ALL);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Print phase adjust settings */
    print_link_phase(device);

    /* Ext sysref to internal sysref phase delta (should be ~0 from previous one-shot sync alignment alignment phase) */
    print_sysref_phase(device);     // Print ext sysref to internal sysref phase delta

    /* Print trigger_phase w.r.t. internal sysref (should be the same startup-to-startup for repeatable deterministic latency */
    print_trig_phase(device);

    if (interactive) {
        printf("*** Configure scope for Tx DL measurement ***\n");
        printf("(<CR> to continue)\n");
        getchar();
    }

    /* Set nominal gain, resume DAC output */
    err = adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x800);
    ADI_CMS_ERROR_GOTO(err, end);

    /*
     * Resync Rx and Tx dig ONLY on ext trigger. Other blocks are masked off.
    */
    err = adi_apollo_clk_mcs_trig_reset_dsp_enable(device);   // Enable sync on Rx and Tx dig only
    ADI_CMS_ERROR_GOTO(err, end);

    for (int k = 0; k < MAX_CHANS; k++) {
        rx_chan_dat[k].fdata = rx_dp_info[k].fsrc_data_rate;
        rx_chan_dat[k].fin = ((k % 2) == 0) ? sysref_freq : ((SAMPLES_PER_VC) / rx_chan_dat[k].fdata);      // A0/B0 = sysref from sig gen, A1/B1 = loop-back
        rx_chan_dat[k].pk_loc = 0;
        rx_chan_dat[k].pk_val = 0;
        rx_chan_dat[k].pos_pk = (k != 2);      // For config A0/B0 have same input, A1/B1 in loop-back.
    }

    /* 
     * Make all chans the same for each dr_config
     * 
     * Each channel in profile can be assigned a different configuration from the table. Here we are
     * setting all chans in a dr_config to the same config.
    */
    num_dr_configs = 0;
    for (int i = 0; i < num_dyn_recs; i++) {
        sprintf(rx_tx_dr_configs[i].name, "drconfig-%03d", i);
        for (int k = 0; k < MAX_CHANS; k++) {
            (rx_tx_dr_configs + i)->rx_chan[k] = &rx_fsrc_dr_cfgs[i];
            (rx_tx_dr_configs + i)->tx_chan[k] = &tx_fsrc_dr_cfgs[i];
        }
        num_dr_configs++;
    }

    /* 
     * Example of adding a dr_config and assigning channels to a config
     * (uncomment to try)
    */
    // sprintf(rx_tx_dr_configs[num_dr_configs].name, "drconfig-%03d", num_dr_configs);
    // rx_tx_dr_configs[num_dr_configs].tx_chan[0] = &tx_fsrc_dr_cfgs[2];
    // rx_tx_dr_configs[num_dr_configs].tx_chan[1] = &tx_fsrc_dr_cfgs[2];
    // rx_tx_dr_configs[num_dr_configs].tx_chan[2] = &tx_fsrc_dr_cfgs[1];
    // rx_tx_dr_configs[num_dr_configs].tx_chan[3] = &tx_fsrc_dr_cfgs[1];

    // rx_tx_dr_configs[num_dr_configs].rx_chan[0] = &rx_fsrc_dr_cfgs[2];
    // rx_tx_dr_configs[num_dr_configs].rx_chan[1] = &rx_fsrc_dr_cfgs[2];
    // rx_tx_dr_configs[num_dr_configs].rx_chan[2] = &rx_fsrc_dr_cfgs[1];
    // rx_tx_dr_configs[num_dr_configs].rx_chan[3] = &rx_fsrc_dr_cfgs[1];
    // num_dr_configs++;

    /* Determine Rx baseband or RF tone values for each dynamic config */
    for (int i = 0; i < num_dyn_recs; i++) {
        if (rx_fsrc_dr_cfgs[i].rf_tone_freq == 0.0) {
            rx_fsrc_dr_cfgs[i].rf_tone_freq = rx_fsrc_dr_cfgs[i].bb_tone_freq + rx_dp_info[0].cnco_freq + rx_dp_info[0].fnco_freq;    /* Vary RF input to get baseband freq */
        } else {
            rx_fsrc_dr_cfgs[i].bb_tone_freq = rx_fsrc_dr_cfgs[i].rf_tone_freq - (rx_dp_info[0].cnco_freq + rx_dp_info[0].fnco_freq);  /* Vary baseband freq to get RF input */
        }

        rx_fsrc_dr_cfgs[i].fsrc_ratio = (double)rx_fsrc_dr_cfgs[i].fsrc_n / (double)rx_fsrc_dr_cfgs[i].fsrc_m;
        adi_apollo_cddc_dcm_bf_to_val(device, rx_fsrc_dr_cfgs[i].cddc_dcm, &rx_fsrc_dr_cfgs[i].cddc_dcm_val);
        adi_apollo_fddc_dcm_bf_to_val(device, rx_fsrc_dr_cfgs[i].fddc_dcm, &rx_fsrc_dr_cfgs[i].fddc_dcm_val);
        rx_fsrc_dr_cfgs[i].fdata = rx_dp_info[0].adc_sample_rate / (rx_fsrc_dr_cfgs[i].cddc_dcm_val * rx_fsrc_dr_cfgs[i].fddc_dcm_val * rx_fsrc_dr_cfgs[i].fsrc_ratio);
    }

    printf("Rx FSRC dyn rec test case        ADC input tone    Baseband tone     Fdata        CDDC    FDDC    FSRC m/n\n");
    printf("=========================        ===============   ==============    =========    ====    ====    ========\n");
    for (int i = 0; i < num_dr_configs; i++) {
        printf("\n");
        for (int k = 0; k < MAX_CHANS; k++) {
            printf("%c%d:  %s\n", 'A' + (k / 2), k % 2, rx_fsrc_dr_cfg_tostr((rx_tx_dr_configs + i)->rx_chan[k], str_buff));
        }
    }
    printf("\n");

    /* Determine Tx baseband or RF tone values for each dynamic config */
    for (int i = 0; i < tx_num_dyn_recs; i++) {
        if (tx_fsrc_dr_cfgs[i].rf_tone_freq == 0.0) {
            tx_fsrc_dr_cfgs[i].rf_tone_freq = tx_fsrc_dr_cfgs[i].bb_tone_freq + tx_dp_info[0].cnco_freq + tx_dp_info[0].fnco_freq;    /* RF tone based on baseband and NCOs */
        }
        else {
            tx_fsrc_dr_cfgs[i].bb_tone_freq = tx_fsrc_dr_cfgs[i].rf_tone_freq - (tx_dp_info[0].cnco_freq + tx_dp_info[0].fnco_freq);  /* Baseband tone base on RF freq and NCOs */
        }

        tx_fsrc_dr_cfgs[i].fsrc_ratio = (double)tx_fsrc_dr_cfgs[i].fsrc_n / (double)tx_fsrc_dr_cfgs[i].fsrc_m;
        adi_apollo_cduc_interp_bf_to_val(device, tx_fsrc_dr_cfgs[i].cduc_interp, &tx_fsrc_dr_cfgs[i].cduc_interp_val);
        adi_apollo_fduc_interp_bf_to_val(device, tx_fsrc_dr_cfgs[i].fduc_interp, &tx_fsrc_dr_cfgs[i].fduc_interp_val);
        tx_fsrc_dr_cfgs[i].fdata = tx_dp_info[0].fclk / (tx_fsrc_dr_cfgs[i].cduc_interp_val * tx_fsrc_dr_cfgs[i].fduc_interp_val * tx_fsrc_dr_cfgs[i].fsrc_ratio);
    }

    printf("Tx FSRC dyn rec test case        DAC output tone   Baseband tone     Fdata        CDUC    FDUC    FSRC n/m\n");
    printf("=========================        ===============   ==============    =========    ====    ====    ========\n");
    for (int i = 0; i < num_dr_configs; i++) {
        printf("\n");
        for (int k = 0; k < MAX_CHANS; k++) {
            printf("%c%d:  %s\n", 'A' + (k / 2), k % 2, tx_fsrc_dr_cfg_tostr((rx_tx_dr_configs + i)->tx_chan[k], str_buff));
        }       
    }
    printf("\n");


    /* Iterate over the fsrc cfg list, each with a different FSRC/FDUC ratio */
    for (int i = 0; i < num_dr_configs; i++) {
        
        if (!is_valid_rate_change(rx_dp_info, tx_dp_info, &rx_tx_dr_configs[i], MAX_CHANS)) {
            printf("Skipping %s\n", rx_tx_dr_configs[i].name);
            continue;
        }

        printf("\nStart dr_config: %s\n", rx_tx_dr_configs[i].name);
        for (int k = 0; k < MAX_CHANS; k++) {
            printf("%c%d-rx:  %s\n", 'A' + (k / 2), k % 2, rx_fsrc_dr_cfg_tostr((rx_tx_dr_configs + i)->rx_chan[k], str_buff));
            printf("%c%d-tx:  %s\n", 'A' + (k / 2), k % 2, tx_fsrc_dr_cfg_tostr((rx_tx_dr_configs + i)->tx_chan[k], str_buff));
        }

        // /* Request ADC input freq change if necessary */
        // if (prev_input_freq != rx_fsrc_ratio_cfg->rf_tone_freq) {
        //     printf("@CMD:SOURCE:FREQ %f\n", rx_fsrc_ratio_cfg->rf_tone_freq);
        //     prev_input_freq = rx_fsrc_ratio_cfg->rf_tone_freq;
        //     printf("(<CR> to continue)\n");
        //     getchar();
        // }

        /*
         * Set the FSRC/DDC/DUC configuration via SPI (opposed to GPIO)
         */
        err = reconfig_setup(device, fpga_device, &rx_tx_dr_configs[i], tx_dp_info);
        ADI_CMS_ERROR_GOTO(err, end);

        for (int k = 0; k < MAX_CHANS; k++) {
            rx_chan_dat[k].fdata = (rx_tx_dr_configs + i)->rx_chan[k]->fdata;
            rx_chan_dat[k].fin = ((k % 2) == 0) ? sysref_freq : (tx_dp_info[k].fsrc_data_rate / (SAMPLES_PER_VC));        // A0/B0 = sysref, A1/B1 = loop-back 10 sample pulse
        }

        /*
         * Generate the ext trigger for reconfig and capture. All sync'd to SYSREF, SC1.
         */
        err = reconfig_trig(device, fpga_device, (rx_tx_dr_configs + i)->name, rx_chan_dat, MAX_CHANS);
        ADI_CMS_ERROR_GOTO(err, end);

        /* Check rate match FIFO overflow/underflow */
        err = adi_apollo_jrx_rm_fifo_status(device, ADI_APOLLO_LINK_A0, &link_status_a0);
        ADI_CMS_ERROR_GOTO(err, end);
        err = adi_apollo_jrx_rm_fifo_status(device, ADI_APOLLO_LINK_B0, &link_status_b0);
        ADI_CMS_ERROR_GOTO(err, end);

        if ((link_status_a0 & 0x3) || (link_status_b0 & 0x3)) {
            printf("RM FIFO overflow or underflow\n");
        }

        adi_apollo_hal_delay_us(device, pause_us);

        /*
         * Print the Rx/Tx reconfig counters. Increments with each trigger (i.e. reconfig)
         * These are only 4-bit and rollover.
         */
        print_reconfig_count(device, ADI_APOLLO_RX);
        print_reconfig_count(device, ADI_APOLLO_TX);

        printf("End dr_config: %s\n", rx_tx_dr_configs[i].name);
    }

    /* Attenuate DAC output */
    err = adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x000);
    ADI_CMS_ERROR_GOTO(err, end);

end:

    return err;
}

static int32_t apollo_fsrc_config(adi_apollo_device_t* device, uint8_t subclass, double dev_clk_freq, double sysref_freq)
{
    int32_t err = API_CMS_ERROR_OK;

    /* Dynamic reconfig configuration */
    static adi_apollo_reconfig_ctrl_pgm_t tx_reconfig = {
        .trig_reconfig_mode = 0,    // 1 = internal triggers, else external
        .cnco_reset = 1,
        .timestamp_reset_en = 1,
        .resync_en = 0,             // 0 for ext trig, 1 for internal
        .fnco_reset_en = 1,
        .tzero_coherence_en = 0,    // T0 Coherence with FSRC enabled is not supported
        .prefsrc_lcm = 0x5f,        // 0x05F if (fduc*cduc) < 128 else 0x17F
        .postfsrc_lcm = 0x17F       // 0x17F if (fduc*cduc) < 128 else 0xBFF
    };

    /* Dynamic reconfig configuration */
    static adi_apollo_reconfig_ctrl_pgm_t rx_reconfig = {
        .trig_reconfig_mode = 0,    // 0 = ext trig, 1 = internal trig
        .cnco_reset = 1,
        .timestamp_reset_en = 1,
        .resync_en = 0,             // 0 = ext trig, 1 = internal trig
        .fnco_reset_en = 1,
        .tzero_coherence_en = 0,    // T0 Coherence with FSRC enabled is not supported
        .prefsrc_lcm = 0x02F,       // 0x02F if (fddc*cddc) < 128 else 0x0BF
        .postfsrc_lcm = 0x17F       // 0x17F if (fddc*cddc) < 128 else 0x5FF 
    };

    /* Don't respond to triggers until system config is stable, later */
    adi_apollo_clk_mcs_trig_sync_enable(device, 0);         // Disable trig sync, will get enabled later
    adi_apollo_clk_mcs_trig_reset_disable(device);

    /* Enable the sub datapath gain. Must be done prior to oneshot-sync (Tx/DAC) */
    adi_apollo_fduc_subdp_gain_enable(device, ADI_APOLLO_FDUC_ALL, 1);

    /* Attenuate DAC output while startup up to prevent random data */
    adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x000);

#include "adi_apollo_bf_jtx_qbf_txfe.h"
    adi_apollo_hal_bf_set(device, BF_JTX_NS_OVERRIDE_INFO(JTX_QBF_TXFE_0_JTX_TOP_RX_DIGITAL0), 1);
    adi_apollo_hal_bf_set(device, BF_JTX_NS_OVERRIDE_INFO(JTX_QBF_TXFE_1_JTX_TOP_RX_DIGITAL0), 1);
    adi_apollo_hal_bf_set(device, BF_JTX_NS_OVERRIDE_INFO(JTX_QBF_TXFE_0_JTX_TOP_RX_DIGITAL1), 1);
    adi_apollo_hal_bf_set(device, BF_JTX_NS_OVERRIDE_INFO(JTX_QBF_TXFE_1_JTX_TOP_RX_DIGITAL1), 1);

#include "adi_apollo_bf_jtx_dual_link.h"
    adi_apollo_hal_bf_set(device, BF_JTX_NS_CFG_INFO(JTX_DUAL_LINK_0_JTX_TOP_RX_DIGITAL0), 8 - 1);
    adi_apollo_hal_bf_set(device, BF_JTX_NS_CFG_INFO(JTX_DUAL_LINK_1_JTX_TOP_RX_DIGITAL0), 8 - 1);
    adi_apollo_hal_bf_set(device, BF_JTX_NS_CFG_INFO(JTX_DUAL_LINK_0_JTX_TOP_RX_DIGITAL1), 8 - 1);
    adi_apollo_hal_bf_set(device, BF_JTX_NS_CFG_INFO(JTX_DUAL_LINK_1_JTX_TOP_RX_DIGITAL1), 8 - 1);

    /* Configure the Tx recofig controller */
    adi_apollo_reconfig_ctrl_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, &tx_reconfig);

    /* Configure the Rx recofig controller */
    adi_apollo_reconfig_ctrl_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, &rx_reconfig);

    /* Use Trigger pin A0 to sync Rx and Tx */
    adi_apollo_clk_mcs_sync_trig_map(device, ADI_APOLLO_RX_TX_ALL, ADI_APOLLO_TRIG_PIN_A0);

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

static int32_t fpga_fsrc_config(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint8_t subclass,
    adi_ads10_apollo_dp_info_t* rx_dp_info, adi_ads10_apollo_dp_info_t* tx_dp_info)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t fpga_link_count;

    /*
     * Some images support 2 links, others 4. This example assumes HW FSRC which has two links.
     */
    adi_fpga_apollo_core_jtx_link_cnt_get(fpga_device, &fpga_link_count);
    if (fpga_link_count != 2) {
        printf("This example only supports FPGA images with HW FSRC (2 links)\n");
        return API_CMS_ERROR_NOT_SUPPORTED;
    }

    /*
     * Set FPGA the subclass
     */
    adi_fpga_apollo_private_write32_bitfield(fpga_device, JESD204B_RX_CONFIG, JESD204B_RX_SUBCLASS_MASK, subclass);
    adi_fpga_apollo_private_write32_bitfield(fpga_device, JESD204B_TX_CONFIG, JESD204B_TX_SUBCLASS_MASK, subclass);

    /*
     * Enable ADS10 pattern start output trigger (0 to 1) output on SMA J3.
     */
    adi_fpga_apollo_core_sys_indicator_set(fpga_device, ADI_FPGA_APOLLO_TX_PAT_START_INDICATOR);
    ADI_CMS_ERROR_RETURN(err);

    /*
     * Select ADS10 external SYSREF source, input on SMA J5
     */
    err = adi_fpga_apollo_core_sysref_src_set(fpga_device, ADI_FPGA_APOLLO_SYSREF_SRC_EXT);
    ADI_CMS_ERROR_RETURN(err);

    /*
     * Setup FPGA SYSREF clocking
     */
    adi_fpga_apollo_core_sysref_setup(fpga_device);

    /*
     * GPIO
     */
    adi_fpga_apollo_private_write32_bitfield(fpga_device, GPIO_D_1, DUT_GPIO_D_1_MASK, 0);
    adi_fpga_apollo_private_write32_bitfield(fpga_device, GPIO_D_2, DUT_GPIO_D_2_MASK, 0);
    adi_fpga_apollo_private_write32_bitfield(fpga_device, GPIO_W_1, DUT_GPIO_W_1_MASK, 0);
    adi_fpga_apollo_private_write32_bitfield(fpga_device, GPIO_W_2, DUT_GPIO_W_2_MASK, 0);


    /*
    * FPGA SYSREF sequencer programming
    *
    * gpio_change_cnt          GPIO change count. Updates GPIOs that set Apollo reconfig profile (not used in this example)
    * first_trig_cnt           First trigger count (starts an Apollo reconfig)
    * second_trig_cnt          Second Trigger count (set equal to first_trig_cnt for single pulse)
    * fsrc_accum_reset_cnt     FSRC accum reset count (FPGA starts sending valid FSRC data)
    * rx_delay_cnt             Rx capture count. The SYSREF edge to capture.
    *
    * Note:
    * - trigger count values are absolute from trigger
    * - first_trig_cnt initiates an Apollo reconfig
    */
    adi_fpga_apollo_hw_fsrc_count_t sysref_seq_cnt = { 1, 1002, 1002, 1102, 2102 };
    adi_fpga_apollo_hw_fsrc_tx_sequencer_config(fpga_device, &sysref_seq_cnt);

    return err;
}

/*
 * Setup the reconfigure parameters (FSRC, DDC, DUC)
*/
static int32_t reconfig_setup(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, 
    rx_tx_reconfig_profile_t* rec_profile,
    adi_ads10_apollo_dp_info_t* tx_dp_info)
{

    /*
     * Channel array assignment
     *
     *  Chan    Index   Side    Side-Index
     *  ----    -----   ----    ----------
     *   A0       0       A        0
     *   A1       1       A        1
     *   B0       2       B        0
     *   B1       3       B        1
    */
    /* CDDC/FDDC CDUC/FDUC */
    for (int k = 0; k < MAX_CHANS; k++) {
        adi_apollo_cddc_dcm_set(device, ADI_APOLLO_CDDC_IDX2B(k / 2, k % 2), rec_profile->rx_chan[k]->cddc_dcm);
        adi_apollo_fddc_dcm_set(device, ADI_APOLLO_FDDC_IDX2B(k / 2, k*2 % 4), rec_profile->rx_chan[k]->fddc_dcm);
        adi_apollo_cduc_interp_set(device, ADI_APOLLO_CDUC_IDX2B(k / 2, k % 2), rec_profile->tx_chan[k]->cduc_interp);
        adi_apollo_fduc_interp_set(device, ADI_APOLLO_FDUC_IDX2B(k / 2, k*2 % 4), rec_profile->tx_chan[k]->fduc_interp);
    }

    /* Rx FSRC */
    adi_apollo_fsrc_ratio_set(device, ADI_APOLLO_RX, ADI_APOLLO_FSRC_A0, rec_profile->rx_chan[0]->fsrc_n, rec_profile->rx_chan[0]->fsrc_m);
    adi_apollo_fsrc_ratio_set(device, ADI_APOLLO_RX, ADI_APOLLO_FSRC_B0, rec_profile->rx_chan[2]->fsrc_n, rec_profile->rx_chan[2]->fsrc_m);

    /* Tx FSRC */
    adi_apollo_fsrc_ratio_set(device, ADI_APOLLO_TX, ADI_APOLLO_FSRC_A0, rec_profile->tx_chan[0]->fsrc_n, rec_profile->tx_chan[0]->fsrc_m);
    adi_apollo_fsrc_ratio_set(device, ADI_APOLLO_TX, ADI_APOLLO_FSRC_B0, rec_profile->tx_chan[2]->fsrc_n, rec_profile->tx_chan[2]->fsrc_m);

    /* FPGA - adjust the # of valid samples sent (FSRC 'n') to compensate for datapath interpolation change */
    adi_fpga_apollo_hw_fsrc_ratio_set(fpga_device, ADI_APOLLO_LINK_A0,
        (rec_profile->tx_chan[0]->fsrc_n * rec_profile->tx_chan[0]->cduc_interp_val * rec_profile->tx_chan[0]->fduc_interp_val) / tx_dp_info[0].total_drc, rec_profile->tx_chan[0]->fsrc_m);
    adi_fpga_apollo_hw_fsrc_ratio_set(fpga_device, ADI_APOLLO_LINK_B0,
        (rec_profile->tx_chan[2]->fsrc_n * rec_profile->tx_chan[2]->cduc_interp_val * rec_profile->tx_chan[2]->fduc_interp_val) / tx_dp_info[2].total_drc, rec_profile->tx_chan[2]->fsrc_m);

    /*
    * FPGA SYSREF sequencer programming
    *
    * gpio_change_cnt          GPIO change count. Updates GPIOs that set Apollo reconfig profile (not used in this example)
    * first_trig_cnt           First trigger count (starts an Apollo reconfig)
    * second_trig_cnt          Second Trigger count (set equal to first_trig_cnt for single pulse)
    * fsrc_accum_reset_cnt     FSRC accum reset count (FPGA starts sending valid FSRC data)
    * rx_delay_cnt             Rx capture count. The SYSREF edge to capture.
    *
    * Note:
    * - trigger count values are absolute from trigger
    * - first_trig_cnt initiates an Apollo reconfig
    */
    adi_fpga_apollo_hw_fsrc_count_t sysref_seq_cnt_post = { 1, 1002, 1002, 1102, 2102 };
    adi_fpga_apollo_hw_fsrc_tx_sequencer_config(fpga_device, &sysref_seq_cnt_post);

    return API_CMS_ERROR_OK;
}

/*
 * Execute a dynamic reconfig sequence using the FPGA's SYSREF event sequencer
 * 
 * Expected configuration
 * 
 * Chan     ADC Input                       DAC Output
 * ----     ----------------------------    ------------------------------
 * A0       Ext SYREF                       Pulse waveform. Aligned to trigger.
 * A1       Loopback DAC_A0-to-ADC_A0   
 * B0       Ext SYREF                       Pulse waveform (inverted due to CE board balun). Aligned to trigger.
 * B1       Loopback DAC_B1-to-ADC_B1   
 * 
 * - The loopback vector is 16K long, single pulse (10 samples wide). May see aliasing effects due to pulse.
 * - Side A and B data rates may be the same or different. If same, then chan-to-chan alignment across side A & B can be checked.
 */
static int32_t reconfig_trig(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, char *desc, rx_chan_data_t *chans, uint32_t n_chans)
{
    int i;
    const int max_chans = MAX_CHANS;
    const uint32_t adc_cap_buff_num = max_chans;
    int32_t err = API_CMS_ERROR_OK;
    int n_caps = 2;
    int16_t* adc_cap_buff[adc_cap_buff_num];
    uint32_t samples_per_conv;
    char cap_filename[256];

    if (n_chans > max_chans) {
        printf("Input param max_chans > MAX_CHANS\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    /* Enable the FPGA ext sysref seq trig */
    adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set(fpga_device, 1);
    
    for (i = 0; i < n_caps; i++) {
        printf("cap # = %d\n", i);

        /* FPGA - Force invalids on JTx.  Valid samples resume after trigger sequence */
        adi_fpga_apollo_core_ptn_play_stop(fpga_device);            // Stop playing vec data, send zeros. Link stays up
        adi_fpga_apollo_core_tx_fsrc_stop(fpga_device);             // Stop sending valid samples, send invalids only.
        adi_apollo_hal_delay_us(device, 10);
     
        adi_fpga_apollo_core_ptn_play_start(fpga_device);           // Arm FPGA playback. Resume playing vec data when re-synced.
        adi_apollo_hal_delay_us(device, 1000);

        /* Apollo - Force invalids on JTx. Upon receiving ext trigger from FPGA, valid samples will be sent. */
        printf("Forcing FSRC JTx invalids\n");
        adi_apollo_jtx_force_invalids_set(device, ADI_APOLLO_LINK_ALL, 1);

        // Allow invalids to flow
        adi_apollo_hal_delay_us(device, 1000000);

        /*
         * Set trig_syn to 1. Apollo will wait for a trigger from the FPGA. When
         * received, the FSRC will be reset.
         *
         * trig_sync is not self-clearing
         */
        adi_apollo_clk_mcs_trig_sync_enable(device, 1);

        adi_fpga_apollo_core_tx_fsrc_change_rate(fpga_device);
        
        /* Capture data into separate I/Q arrays. *** This will start the FPGA SYSREF sequencer *** */
        err = adi_ads10_apollo_ex_fpga_capture_to_array(device, fpga_device, &samples_per_conv, (uint16_t**)adc_cap_buff, adc_cap_buff_num);
        ADI_CMS_ERROR_GOTO(err, end);
        
        /* Save off capture to individual converter files */
        for (int k = 0; k < n_chans; k++) {
            sprintf(cap_filename, "cap-%s_%c%d_%d", desc, 'a' + k / 2, k % 2, i);
            adi_ads10_apollo_ex_samples_to_file(cap_filename, adc_cap_buff[k], samples_per_conv, true);
        }

        /* free memory allocated by adi_ads10_apollo_ex_fpga_capture_to_array() */
        for (int k = 0; k < n_chans; k++) {
            free(adc_cap_buff[k]);
            adc_cap_buff[k] = NULL;
        }

        /* Clear the trig sync (not a self clearing bit) */
        adi_apollo_clk_mcs_trig_sync_enable(device, 0);
    }

end:
    /*
     * Print the Rx/Tx reconfig count (4-bit counter)
    */
    print_reconfig_count(device, ADI_APOLLO_RX);
    print_reconfig_count(device, ADI_APOLLO_TX);

    /* Disable the FPGA ext sysref seq trig */
    adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set(fpga_device, 0);

    return err;
 }

static __maybe_unused int32_t fpga_create_and_load_vec(adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t *profile, vec_type_e vec_type)
{
    int32_t err = API_CMS_ERROR_OK;

    /* 
     * Create and populate transmit buffer. Vector is a single pulse pattern for scope observation.
     */
    if (vec_type == PULSE_10) {
        err = adi_ads10_apollo_ex_vec_pulse_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, 8192, 10.0/8192, 0.5, 0.5);
    } else if (vec_type == TONE_0P40_FS) {
        err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, SAMPLES_PER_VC, 0.4, -9.0);
    } else if (vec_type == TONE_0P05_FS) {
        err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, SAMPLES_PER_VC, 0.05, -9.0);
    } else if (vec_type == TONE_0P015625_FS) {
        err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, SAMPLES_PER_VC, 0.015625, -9.0);
    } else {
        printf("Unknown vec_type_e type\n");
        return API_CMS_ERROR_ERROR;
    }

    return err;
}

static __maybe_unused void print_trig_phase(adi_apollo_device_t* device) {
    uint16_t trig_phase, trig_phase_unused;
    adi_apollo_trig_pin_e trig = ADI_APOLLO_TRIG_PIN_A0;
    adi_apollo_clk_mcs_trig_phase_get(device, trig, &trig_phase, &trig_phase_unused);     // A0 trig phase
    printf("[*Trigger phase %c%d: %d*]\n", 'A' + trig / 2, trig % 2, trig_phase);
}

static __maybe_unused void print_sysref_phase(adi_apollo_device_t* device) {
    uint32_t sysref_phase;
    for (int x = 0; x < 1; x++) {
        adi_apollo_clk_mcs_sysref_phase_get(device, &sysref_phase);
        printf("Apollo sysref_phase = 0x%x  %d\n", sysref_phase, sysref_phase);
    }
}

static __maybe_unused void print_reconfig_count(adi_apollo_device_t* device, adi_apollo_terminal_e terminal)
{
    uint16_t sync_count[2];

    adi_apollo_reconfig_trig_evt_cnt_get(device, terminal, ADI_APOLLO_SIDE_A, ADI_APOLLO_RECONFIG_EXTERNAL, &sync_count[0]);
    adi_apollo_reconfig_trig_evt_cnt_get(device, terminal, ADI_APOLLO_SIDE_B, ADI_APOLLO_RECONFIG_EXTERNAL, &sync_count[1]);

    printf("%s ext trig event count A=%d B=%d\n", (terminal == ADI_APOLLO_RX) ? "Rx" : "Tx", sync_count[0], sync_count[1]);
}

static char* rx_fsrc_dr_cfg_tostr(rx_fsrc_dr_cfg_t* fsrc_ratio_cfg, char* buff)
{
    sprintf(buff, "%-25s    %14.3f    %14.3f    %9.3f   %4d    %4d     %6d/%-6d",
        fsrc_ratio_cfg->name, fsrc_ratio_cfg->rf_tone_freq, fsrc_ratio_cfg->bb_tone_freq, fsrc_ratio_cfg->fdata,
        fsrc_ratio_cfg->cddc_dcm_val, fsrc_ratio_cfg->fddc_dcm_val,
        fsrc_ratio_cfg->fsrc_n, fsrc_ratio_cfg->fsrc_m);

    return buff;
}

static char * tx_fsrc_dr_cfg_tostr(tx_fsrc_dr_cfg_t *fsrc_ratio_cfg, char *buff)
{
    sprintf(buff, "%-25s    %14.3f    %14.3f    %9.3f   %4d    %4d     %6d/%-6d",
        fsrc_ratio_cfg->name, fsrc_ratio_cfg->rf_tone_freq, fsrc_ratio_cfg->bb_tone_freq, fsrc_ratio_cfg->fdata,
        fsrc_ratio_cfg->cduc_interp_val, fsrc_ratio_cfg->fduc_interp_val,
        fsrc_ratio_cfg->fsrc_n, fsrc_ratio_cfg->fsrc_m);

    return buff;
}

static bool is_valid_rate_change(adi_ads10_apollo_dp_info_t *rx_dp_info, adi_ads10_apollo_dp_info_t *tx_dp_info,
                                 rx_tx_reconfig_profile_t *rx_tx_dr_configs, int max_chans)
{
    rx_fsrc_dr_cfg_t *rx;
    tx_fsrc_dr_cfg_t *tx;

    for (int i = 0; i < max_chans; i++) {

        rx = rx_tx_dr_configs->rx_chan[i];
        tx = rx_tx_dr_configs->tx_chan[i];

        /* Link rate must be faster than reconfig rate */
        if ((rx->cddc_dcm_val * rx->fddc_dcm_val < rx_dp_info[i].total_drc) ||
            (tx->cduc_interp_val * tx->fduc_interp_val < tx_dp_info[i].total_drc)) {
            return false;
        }
    }
    return true;
}

static void print_link_phase(adi_apollo_device_t *device)
{

    uint16_t jrx_phase_diff0, jrx_phase_diff1;
    uint16_t jrx_phase_adjust0, jrx_phase_adjust1;

    adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_diff0);
    adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_diff1);
    printf("jrx_phase_diff0/jrx_phase_diff1 = %d %d\n", jrx_phase_diff0, jrx_phase_diff1);

    adi_apollo_jrx_phase_adjust_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_adjust0);
    adi_apollo_jrx_phase_adjust_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_adjust1);
    printf("jrx_phase_adjust0/jrx_phase_adjust1 = %d %d\n", jrx_phase_adjust0, jrx_phase_adjust1);
}

static uint16_t jrx_phase_adj_const_get(adi_apollo_top_t *profile)
{
    uint32_t jrx_slr = profile->jrx[0].common_link_cfg.lane_rate_kHz;

    // From ADS10 w/ FMCB
    // Can be obtained from adi_apollo_jrx_phase_adjust_calc()
    // Margin may vary  depending on configuration and FPGA image
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

#endif /* !defined(VERSAL_PLATFORM) */
