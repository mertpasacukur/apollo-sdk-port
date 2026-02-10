#if defined(__linux__)

/*!
 * \brief     ADS10 Apollo Rx/Tx Sample Repeat Dynamic Reconfig (DR) using manual (SPI) trigger.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 *            
 * This example demonstrates Apollo dynamic reconfiguration (DR) initiated by manual (SPI) trigger.
 * 
 * Two tables define a list of configuration parameters, one for Rx and one for Tx. Each row 
 * specifies the CDDC/CDUC and FDDC/FDUC. The program will iterate through the table setting 
 * up and performing a dynamic reconfiguration of the data path. Although Apollo's data path 
 * data rate changes, the JESD data rate remains fixed.
 * 
 * This example works for two versions of ADS10 FPGA images. One is a SW based Sample Repeat implementation
 * and the other is HW based. For the SW version, the Tx vector must be modified and reloaded into FPGA 
 * memory for each reconfiguration. The modification inserts repeated samples into the vector in order
 * to achieve the desired data rate. They are removed by Apollo's rate-match FIFO. Note that this 
 * will drop the JESD link momentarily due to the FPGA's logic. 
 * 
 * For the HW FPGA version, the vector is loaded once and invalid samples are inserted automatically. 
 * A DR sequence is orchestrated between Apollo and the FPGA resulting in a smooth transition from one config to another.
 * The JESD link remains up between DR events.
 * 
 * This example uses JRx and RMFIFO IRQs to ensure proper DR execution. For example, it checks that the JRx JESD
 * link remains up and no RMFIFO empty/full events occur. 
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "adi_apollo.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_fpga_apollo_capture.h"
#include "adi_fpga_apollo_transmit.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_ctl.h"

/* Dynamic reconfig configuration (Rx) */
static adi_apollo_reconfig_ctrl_pgm_t rx_reconfig = {
    .trig_reconfig_mode = 0,    // 0: ext or manual trig sync; 1 = internal trig master
    .cnco_reset = 1,
    .timestamp_reset_en = 1,
    .resync_en = 0,
    .fnco_reset_en = 1,
    .tzero_coherence_en = 0,    // T0 Coherence with FSRC enabled is not supported
    .prefsrc_lcm = 0x02F,       // 0x02F if (fddc*cddc) < 128 else 0x0BF
    .postfsrc_lcm = 0x17F       // 0x17F if (fddc*cddc) < 128 else 0x5FF
};

/* Dynamic reconfig configuration (Tx) */
static adi_apollo_reconfig_ctrl_pgm_t tx_reconfig = {
    .trig_reconfig_mode = 0,    // 0: ext or manual trig sync; 1 = internal trig master
    .cnco_reset = 1,
    .timestamp_reset_en = 1,
    .resync_en = 0,
    .fnco_reset_en = 1,
    .tzero_coherence_en = 0,    // T0 Coherence with FSRC enabled is not supported
    .prefsrc_lcm = 0x5f,        // 0x05F if (fduc*cduc) < 128 else 0x17F
    .postfsrc_lcm = 0x17F       // 0x17F if (fduc*cduc) < 128 else 0xBFF
};

static int32_t rx_cfg_table_setup(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t rx_sr_dr_cfgs[], 
                               uint32_t num_dyn_recs, adi_ads10_apollo_dp_info_t *rx_dp_info, adi_apollo_top_t *profile);
static int32_t tx_cfg_table_setup(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t tx_sr_dr_cfgs[], uint32_t num_dyn_recs, 
                               adi_ads10_apollo_dp_info_t *tx_dp_info, adi_apollo_top_t *profile);

static int32_t apollo_rx_sr_configure(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_ads10_apollo_sr_dr_cfg_t *sr_cfg);
static int32_t apollo_tx_sr_configure(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_ads10_apollo_sr_dr_cfg_t* sr_cfg);
static void print_rx_cfg_table(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t rx_sr_dr_cfgs[], uint32_t num_dyn_recs);
static void print_tx_cfg_table(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t tx_sr_dr_cfgs[], uint32_t num_dyn_recs);

int32_t fullchip_sr_dr(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
	int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    adi_ads10_apollo_dp_info_t rx_dp_info;
    adi_ads10_apollo_dp_info_t tx_dp_info;
    adi_ads10_apollo_sr_dr_cfg_t* rx_sr_ratio_cfg;
    adi_ads10_apollo_sr_dr_cfg_t* tx_sr_ratio_cfg;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    double prev_input_freq = -1.0;
    int num_dyn_recs;
    bool interleaved = 1;               /* 1 = interleave data, 0 = separate I/Q files */
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;       /* min num samples per virt conv */
    char cap_fname_base[128];
    uint32_t vec_len = DEFAULT_NUM_SAMPLES_L;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw sr and tpl
    double bb_tone_freq = 0;            // Baseband tone
    double tone_ratio = 0.2;            // Tone vector used for all DR configs (Ft = tone_ratio * data_rate)
    uint32_t irq_status[2];
    uint16_t active_links = ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0;
    uint32_t jrx_rmfifo_irqs = ADI_APOLLO_JRX_J204C_IRQ_RM_FIFO_EMPTY | ADI_APOLLO_JRX_J204C_IRQ_RM_FIFO_FULL;
    uint32_t jrx_link_irqs = ADI_APOLLO_JRX_J204C_IRQ_CRC |
                             ADI_APOLLO_JRX_J204C_IRQ_SH |
                             ADI_APOLLO_JRX_J204C_IRQ_MB |
                             ADI_APOLLO_JRX_J204C_IRQ_EMB |
                             ADI_APOLLO_JRX_J204C_IRQ_DATA_RDY_LOST;
    uint32_t jrx_all_irqs = jrx_rmfifo_irqs | jrx_link_irqs;

    /* Rx SR dynamic reconfig test cases */
    adi_ads10_apollo_sr_dr_cfg_t rx_sr_dr_cfgs[] = {
        /* Name        CDDC                        FDDC                        CNCO        FNCO */
        {"profile",    ADI_APOLLO_CDDC_DCM_1,      ADI_APOLLO_FDDC_RATIO_1,    2000.0,     10.0}, /* will be overwritten by profile */
        {"c4_f4",      ADI_APOLLO_CDDC_DCM_4,      ADI_APOLLO_FDDC_RATIO_4,    2000.0,     10.0},
        {"c4_f8",      ADI_APOLLO_CDDC_DCM_4,      ADI_APOLLO_FDDC_RATIO_8,    2000.0,     10.0},
        {"c4_f16",     ADI_APOLLO_CDDC_DCM_4,      ADI_APOLLO_FDDC_RATIO_16,   2000.0,     10.0},
        {"c4_f32",     ADI_APOLLO_CDDC_DCM_4,      ADI_APOLLO_FDDC_RATIO_32,   2000.0,     10.0}, 
        {"c4_f64",     ADI_APOLLO_CDDC_DCM_4,      ADI_APOLLO_FDDC_RATIO_64,   2000.0,     10.0},
        {"profile",    ADI_APOLLO_CDDC_DCM_1,      ADI_APOLLO_FDDC_RATIO_1,    2000.0,     10.0}, /* will be overwritten by profile */
    };
    int rx_num_dyn_recs = sizeof(rx_sr_dr_cfgs) / sizeof(rx_sr_dr_cfgs[0]);

    /* Tx SRC dynamic reconfig test cases */
    adi_ads10_apollo_sr_dr_cfg_t tx_sr_dr_cfgs[] = {
        /* Name        CDUC                           FDUC                        CNCO        FNCO */
        {"profile",    ADI_APOLLO_CDUC_INTERP_1,      ADI_APOLLO_FDUC_RATIO_1,    2000.0,     10.0}, /* will be overwritten by profile */
        {"c4_f4",      ADI_APOLLO_CDUC_INTERP_4,      ADI_APOLLO_FDUC_RATIO_4,    2000.0,     10.0},
        {"c4_f8",      ADI_APOLLO_CDUC_INTERP_4,      ADI_APOLLO_FDUC_RATIO_8,    2000.0,     10.0},
        {"c4_f16",     ADI_APOLLO_CDUC_INTERP_4,      ADI_APOLLO_FDUC_RATIO_16,   2000.0,     10.0},
        {"c4_f32",     ADI_APOLLO_CDUC_INTERP_4,      ADI_APOLLO_FDUC_RATIO_32,   2000.0,     10.0}, 
        {"c4_f64",     ADI_APOLLO_CDUC_INTERP_4,      ADI_APOLLO_FDUC_RATIO_64,   2000.0,     10.0},
        {"profile",    ADI_APOLLO_CDUC_INTERP_1,      ADI_APOLLO_FDUC_RATIO_1,    2000.0,     10.0}, /* will be overwritten by profile */
    };
    int tx_num_dyn_recs = sizeof(tx_sr_dr_cfgs) / sizeof(tx_sr_dr_cfgs[0]);

    /* Check that the number of Rx and Tx test cases match */
    ADI_CMS_CHECK(tx_num_dyn_recs != rx_num_dyn_recs, API_CMS_ERROR_ERROR);

    num_dyn_recs = rx_num_dyn_recs;
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);
    
    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    /* Get Tx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    /*
     * Align the Tx NCOs with Rx NCOs so that baseband signals match when looped-back
    */
    tx_dp_info.coarse_ftw = rx_dp_info.coarse_ftw;
    tx_dp_info.cnco_freq = rx_dp_info.cnco_freq;
    tx_dp_info.fine_ftw = rx_dp_info.fine_ftw;
    tx_dp_info.fnco_freq = rx_dp_info.fnco_freq;

    /* Change the Tx NCOs to match Rx */
    adi_apollo_cnco_ftw_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, 0, 1, tx_dp_info.coarse_ftw);
    adi_apollo_fnco_main_phase_inc_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, tx_dp_info.fine_ftw);

    /* Calculate data rates for each item in config tables */
    rx_cfg_table_setup(device, rx_sr_dr_cfgs, num_dyn_recs, &rx_dp_info, profile);
    tx_cfg_table_setup(device, tx_sr_dr_cfgs, num_dyn_recs, &tx_dp_info, profile);
    print_rx_cfg_table(device, rx_sr_dr_cfgs, num_dyn_recs);
    print_tx_cfg_table(device, tx_sr_dr_cfgs, num_dyn_recs);

    /* Inspect the JTx link states (Rx/ADC) */
    printf("\nApollo JTx (Rx/ADC) link info:\n");
    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Inspect the JRx link states (Tx/DAC) */
    printf("\nApollo JRx (Tx/DAC) link info:\n");
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Select SPI trigger for the trig selection mux (Rx/ADC) */
    err = adi_apollo_trigts_reconfig_trig_sel_mux_set(device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_SPI);   // SPI trigger Reconfig
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_trigts_cdrc_trig_sel_mux_set(device, ADI_APOLLO_RX, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_TRIG_SPI);       // SPI trigger CDDC
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_trigts_fdrc_trig_sel_mux_set(device, ADI_APOLLO_RX, ADI_APOLLO_FDDC_ALL, ADI_APOLLO_TRIG_SPI);       // SPI trigger FDDC
    ADI_CMS_ERROR_RETURN(err);

    /* Select SPI trigger for the trig selection mux (Tx/DAC)*/
    err = adi_apollo_trigts_reconfig_trig_sel_mux_set(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_SPI);   // SPI trigger Reconfig
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_trigts_cdrc_trig_sel_mux_set(device, ADI_APOLLO_TX, ADI_APOLLO_CDUC_ALL, ADI_APOLLO_TRIG_SPI);       // SPI trigger CDUC
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_trigts_fdrc_trig_sel_mux_set(device, ADI_APOLLO_TX, ADI_APOLLO_FDUC_ALL, ADI_APOLLO_TRIG_SPI);       // SPI trigger FDUC
    ADI_CMS_ERROR_RETURN(err);

    /* Program the Rx dynamic reconfig block */
    err = adi_apollo_reconfig_ctrl_pgm(device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, &rx_reconfig);
    ADI_CMS_ERROR_RETURN(err);

    /* Program the Tx dynamic reconfig block */
    err = adi_apollo_reconfig_ctrl_pgm(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, &tx_reconfig);
    ADI_CMS_ERROR_RETURN(err);

    /* Enable the sub datapath gain. Must be done prior to oneshot-sync (Tx/DAC) */
    err = adi_apollo_fduc_subdp_gain_enable(device, ADI_APOLLO_FDUC_ALL, 1);
    ADI_CMS_ERROR_RETURN(err);

    /* Attenuate DAC output while startup up to prevent random data */
    err = adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x000);
    ADI_CMS_ERROR_RETURN(err);

    /* Enable the JRx link and RM-FIFO IRQs */
    err = adi_apollo_jrx_j204c_irq_enable_set(device, active_links, ADI_APOLLO_JRX_J204C_IRQ_ALL, 0);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_jrx_j204c_irq_enable_set(device, active_links, jrx_all_irqs, 1);
    ADI_CMS_ERROR_RETURN(err);

    /*  Clock conditioning and ADC FG calibrations */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, vec_len, tone_ratio, -1.0);
    ADI_CMS_ERROR_RETURN(err);

    // Dynamic Sync Serdes Links gradually in a sequence
    printf("Run Rx-TX SerDes Links Dyn Sync...\n");
    err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(device);
    ADI_CMS_ERROR_RETURN(err);

    // Allow clks and supplies to settle after sync
    err = adi_apollo_hal_delay_us(device, 1000);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG | ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    EXCTL_SIGGEN_ON(0x33, false);
    EXCTL_SIGGEN_LEVEL(3.0, false);

    /* 
     * Iterate over the list of dynamic configurations
     */
    for (int i = 0; i < num_dyn_recs; i++) {
        printf("\n\niter: %02d.\n", i);

        /* Get config */
        rx_sr_ratio_cfg = &rx_sr_dr_cfgs[i];
        tx_sr_ratio_cfg = &tx_sr_dr_cfgs[i];
    
        adi_ads10_ex_sr_dr_cfg_to_str(rx_sr_ratio_cfg, str_buff, str_buff_len);
        printf("\nStart RX: %s\n", str_buff);
        adi_ads10_ex_sr_dr_cfg_to_str(tx_sr_ratio_cfg, str_buff, str_buff_len);
        printf("Start TX: %s\n", str_buff);

        /* TODO: Remove once FPGA supports xdrc > 2 */
        if (fpga_feature_flags.rx_hw_tl || fpga_feature_flags.tx_hw_tl) {
            if ((rx_sr_ratio_cfg->link_xdrc / rx_sr_ratio_cfg->base_xdrc) > 2 || (tx_sr_ratio_cfg->link_xdrc / tx_sr_ratio_cfg->base_xdrc) > 2) {
                printf("HW Sample Repeat does not support XDRC > 2, skipping\n");
                continue;
            }
        }

        /* Attenuate output to mask transition glitches during reconfig */
        err = adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0);
        ADI_CMS_ERROR_RETURN(err);

        bb_tone_freq = tone_ratio * tx_sr_ratio_cfg->fdata;
        printf("Tx bb_tone_freq = %f    NCO Shift %f MHz\n", bb_tone_freq, tx_sr_ratio_cfg->cnco_freq + tx_sr_ratio_cfg->fnco_freq);
        
        /* Request ADC input freq change if necessary. Capture FFT will show bb_tone_freq after nco freq subtracted out */
        if (prev_input_freq != bb_tone_freq + (rx_sr_ratio_cfg->cnco_freq + rx_sr_ratio_cfg->fnco_freq)) {
            EXCTL_SIGGEN_FREQ(bb_tone_freq + (rx_sr_ratio_cfg->cnco_freq + rx_sr_ratio_cfg->fnco_freq), interactive);
            prev_input_freq = bb_tone_freq + (rx_sr_ratio_cfg->cnco_freq + rx_sr_ratio_cfg->fnco_freq);
        }

        /* Adjust the FNCOs for CDDC/CDUC changes. The FNCO clock is sample_clk / cdrc val */
        err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, rx_dp_info.adc_sample_rate, rx_sr_ratio_cfg->cnco_freq);
        ADI_CMS_ERROR_RETURN(err);
        
        err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, rx_dp_info.adc_sample_rate / rx_sr_ratio_cfg->cdrc_ratio_val, rx_sr_ratio_cfg->fnco_freq);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, tx_dp_info.dac_sample_rate, tx_sr_ratio_cfg->cnco_freq);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, tx_dp_info.dac_sample_rate / tx_sr_ratio_cfg->cdrc_ratio_val, tx_sr_ratio_cfg->fnco_freq);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_fpga_sr_pgm(fpga_device, tx_sr_ratio_cfg, ADI_APOLLO_TX);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_fpga_sr_pgm(fpga_device, rx_sr_ratio_cfg, ADI_APOLLO_RX);
        ADI_CMS_ERROR_RETURN(err);
        
        /* Configure Apollo RX SR dynamic reconfig */
        err = apollo_rx_sr_configure(device, fpga_device, rx_sr_ratio_cfg);
        ADI_CMS_ERROR_RETURN(err);

        /* Configure Apollo TX SR dynamic reconfig */
        err = apollo_tx_sr_configure(device, fpga_device, tx_sr_ratio_cfg);
        ADI_CMS_ERROR_RETURN(err);

        /* 
         * Perform the reconfiguration.
         * For HW based FPGA SR, we only have to load vector once. The repeated samples are inserted by HW.
         * For SW based FPGA SR, the vector has to be modified to insert invalid samples.
         * JRx IRQs are used to detect if Jrx links drop.
         */
        if (!fpga_feature_flags.tx_hw_tl) {
            err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, vec_len, tone_ratio, -1.0);
            ADI_CMS_ERROR_RETURN(err);
            if (fpga_device->state_info.design_id != ADI_FPGA_APOLLO_DESIGN_STD_EVAL) {
                err = adi_fpga_apollo_core_ptn_play_start(fpga_device);
                ADI_CMS_ERROR_RETURN(err);
            } else {
                err = adi_fpga_apollo_core_transmit_start(fpga_device);
                ADI_CMS_ERROR_RETURN(err);
            }
        }

        /* Execute manual dynamic reconfig - new configuration applied */
        err = adi_apollo_clk_mcs_trig_reset_dsp_enable(device);   /* Resync Rx and Tx digital blocks only during reconfig */
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_clk_mcs_man_reconfig_sync(device);       /* Reconfig happens here, clears rmfifo status */
        ADI_CMS_ERROR_RETURN(err); 

        /* Set nominal gain to resume output */
        err = adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x800);
        ADI_CMS_ERROR_RETURN(err);
        
        /* Read and verify Apollo JRx status A-SIDE */
        err = adi_ads10_apollo_ex_jesd_rx_status(device, ADI_APOLLO_LINK_A0);
        ADI_CMS_ERROR_RETURN(err);

        /* Read and verify Apollo JRx status B-SIDE */
        err = adi_ads10_apollo_ex_jesd_rx_status(device, ADI_APOLLO_LINK_B0);
        ADI_CMS_ERROR_RETURN(err);

        /* 
         * Clear all IRQs when JRx link is first up; Otherwise, clear RMFIFO IRQs only.
         * Clearing only the RMFIFO IRQs allows validating the JESD JRx links remain up with after each DR.
         */
        err = adi_apollo_jrx_j204c_irq_clear(device, active_links, (i == 0) ? jrx_all_irqs : jrx_rmfifo_irqs);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_hal_delay_us(device, 1000 * 10);     // Let things run to trigger any IRQs (10ms)
        ADI_CMS_ERROR_RETURN(err);

        printf("\nJRx irq state after DR\n");
        adi_ads10_ex_jrx_irq_to_str(device, jrx_all_irqs, 0, str_buff, str_buff_len); 
        printf("\n%s\n\n", str_buff);

        /* Links should be running w/o any errors (i.e. no irqs set) */
        err = adi_apollo_jrx_j204c_irq_get(device, ADI_APOLLO_LINK_A0, jrx_all_irqs, 1, &irq_status[0]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_jrx_j204c_irq_get(device, ADI_APOLLO_LINK_B0, jrx_all_irqs, 1, &irq_status[1]);
        ADI_CMS_ERROR_RETURN(err);

        if (irq_status[0] || irq_status[1]) {
            EXCTL_FAIL("jrx-links-should-be-up");
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_LINK_DOWN);
        }
        /* Read FPGA capture memory and write out i/q files */
        sprintf(cap_fname_base, "%s_%s", "fullchip_sr_dr", rx_sr_ratio_cfg->name);
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, interleaved);
        ADI_CMS_ERROR_RETURN(err);

#ifdef EXCTL_AUTOMATION
        EXCTL_RX_MEAS_FREQ(0x33, cap_fname_base, prev_input_freq,  10);
#else
        EXCTL_RX_MEAS_FREQ(0x33, cap_fname_base, (prev_input_freq - (rx_sr_ratio_cfg->cnco_freq + rx_sr_ratio_cfg->fnco_freq)), 10);
#endif
        
        EXCTL_TX_MEAS_FREQ(0x33, bb_tone_freq + tx_sr_ratio_cfg->cnco_freq + tx_sr_ratio_cfg->fnco_freq, 10, interactive);
        
        adi_ads10_ex_sr_dr_cfg_to_str(rx_sr_ratio_cfg, str_buff, str_buff_len);
        printf("\nEnd RX: %s\n", str_buff);
        adi_ads10_ex_sr_dr_cfg_to_str(tx_sr_ratio_cfg, str_buff, str_buff_len);
        printf("End TX: %s\n", str_buff);
    }

    return err;
}

/**
 * \brief  Update the dynamic reconfig blocks (FDDC, CDDC)
 */
static int32_t apollo_rx_sr_configure(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_ads10_apollo_sr_dr_cfg_t* sr_cfg) {
    int32_t err = API_CMS_ERROR_OK;

    /* Set the CDDC decimation to be applied on trigger (reconfig) */
    err = adi_apollo_cddc_dcm_set(device, ADI_APOLLO_CDDC_ALL, sr_cfg->cdrc_ratio);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the FDDC decimation to be applied on trigger (reconfig) */
    err = adi_apollo_fddc_dcm_set(device, ADI_APOLLO_FDDC_ALL, sr_cfg->fdrc_ratio);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

/**
 * \brief  Update the dynamic reconfig blocks (FDUC, CDUC)
 */
static int32_t apollo_tx_sr_configure(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_ads10_apollo_sr_dr_cfg_t* sr_cfg) {
    int32_t err = API_CMS_ERROR_OK;

    /* Set the CDUC interpolation to be applied on trigger (reconfig) */
    err = adi_apollo_cduc_interp_set(device, ADI_APOLLO_CDUC_ALL, sr_cfg->cdrc_ratio);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the FDUC interpolation to be applied on trigger (reconfig) */
    err = adi_apollo_fduc_interp_set(device, ADI_APOLLO_FDUC_ALL, sr_cfg->fdrc_ratio);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static void print_rx_cfg_table(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t rx_sr_dr_cfgs[], uint32_t num_dyn_recs)
{
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_sr_dr_cfg_t *rx_sr_ratio_cfg;

    printf("Rx SR Dyn Rec Test Case        Fdata           CDDC  FDDC  CNCO          FNCO\n");
    printf("=========================      ============    ====  ====  ============  ============\n");
    for (int i = 0; i < num_dyn_recs; i++) {
        rx_sr_ratio_cfg = &rx_sr_dr_cfgs[i];
        adi_ads10_ex_sr_dr_cfg_to_str(rx_sr_ratio_cfg, str_buff, str_buff_len);
        printf("%s\n", str_buff);
    }
    printf("\n");
}

static void print_tx_cfg_table(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t tx_sr_dr_cfgs[], uint32_t num_dyn_recs)
{
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_sr_dr_cfg_t *tx_sr_ratio_cfg;

    printf("Tx SR Dyn Rec Test Case        Fdata           CDDC  FDDC  CNCO          FNCO\n");
    printf("=========================      ============    ====  ====  ============  ============\n");
    for (int i = 0; i < num_dyn_recs; i++) {
        tx_sr_ratio_cfg = &tx_sr_dr_cfgs[i];
        adi_ads10_ex_sr_dr_cfg_to_str(tx_sr_ratio_cfg, str_buff, str_buff_len);
        printf("%s\n", str_buff);
    }
    printf("\n");
}

static int32_t rx_cfg_table_setup(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t rx_sr_dr_cfgs[], uint32_t num_dyn_recs, adi_ads10_apollo_dp_info_t *rx_dp_info, adi_apollo_top_t *profile)
{
    int32_t err = API_CMS_ERROR_OK;

    adi_ads10_apollo_sr_dr_cfg_t *rx_sr_ratio_cfg;

    /* Determine Rx Fdata rates for each test case. Set first to link dcm. */
    rx_sr_dr_cfgs[0].cdrc_ratio = profile->rx_path[0].rx_cddc[0].drc_ratio;
    rx_sr_dr_cfgs[0].fdrc_ratio = profile->rx_path[0].rx_fddc[0].drc_ratio;
    for (int i = 0; i < num_dyn_recs - 1; i++) {
        rx_sr_ratio_cfg = &rx_sr_dr_cfgs[i];

        adi_apollo_cddc_dcm_bf_to_val(device, rx_sr_ratio_cfg->cdrc_ratio, &rx_sr_ratio_cfg->cdrc_ratio_val);
        adi_apollo_fddc_dcm_bf_to_val(device, rx_sr_ratio_cfg->fdrc_ratio, &rx_sr_ratio_cfg->fdrc_ratio_val);
        rx_sr_ratio_cfg->link_xdrc = rx_sr_ratio_cfg->cdrc_ratio_val * rx_sr_ratio_cfg->fdrc_ratio_val;
        rx_sr_ratio_cfg->fdata = rx_dp_info->fclk / (rx_sr_ratio_cfg->cdrc_ratio_val * rx_sr_ratio_cfg->fdrc_ratio_val);
        rx_sr_ratio_cfg->base_xdrc = profile->jtx[0].tx_link_cfg[0].link_total_ratio;
    }
    rx_sr_dr_cfgs[num_dyn_recs - 1] = rx_sr_dr_cfgs[0]; /* Copy the first entry to the last */

    return err;
}

static int32_t tx_cfg_table_setup(adi_apollo_device_t *device, adi_ads10_apollo_sr_dr_cfg_t tx_sr_dr_cfgs[], uint32_t num_dyn_recs, adi_ads10_apollo_dp_info_t *tx_dp_info, adi_apollo_top_t *profile)
{
    int32_t err = API_CMS_ERROR_OK;

    adi_ads10_apollo_sr_dr_cfg_t *tx_sr_ratio_cfg;

    /* Determine Tx Fdata rates for each test case. Set first to link interp. */
    tx_sr_dr_cfgs[0].cdrc_ratio = profile->tx_path[0].tx_cduc[0].drc_ratio;
    tx_sr_dr_cfgs[0].fdrc_ratio = profile->tx_path[0].tx_fduc[0].drc_ratio;
    for (int i = 0; i < num_dyn_recs - 1; i++) {
        tx_sr_ratio_cfg = &tx_sr_dr_cfgs[i];

        adi_apollo_cduc_interp_bf_to_val(device, tx_sr_ratio_cfg->cdrc_ratio, &tx_sr_ratio_cfg->cdrc_ratio_val);
        adi_apollo_fduc_interp_bf_to_val(device, tx_sr_ratio_cfg->fdrc_ratio, &tx_sr_ratio_cfg->fdrc_ratio_val);
        tx_sr_ratio_cfg->link_xdrc = tx_sr_ratio_cfg->cdrc_ratio_val * tx_sr_ratio_cfg->fdrc_ratio_val;
        tx_sr_ratio_cfg->fdata = tx_dp_info->fclk / (tx_sr_ratio_cfg->cdrc_ratio_val * tx_sr_ratio_cfg->fdrc_ratio_val);
        tx_sr_ratio_cfg->base_xdrc = profile->jrx[0].rx_link_cfg[0].link_total_ratio;
    }
    tx_sr_dr_cfgs[num_dyn_recs - 1] = tx_sr_dr_cfgs[0]; /* Copy the first entry to the last */

    return err;
}
#endif /* defined(__linux__) */
