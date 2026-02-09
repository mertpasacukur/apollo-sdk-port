/*!
 * \brief     ADS10 Apollo Tx data path Fast-Frequency Hopping
 *  
 * Demonstrates different methods for CNCO frequency hopping. First is direct regmap,
 * then for SPI triggered and trigger-master the following methods are shown:
 *   -- scheduled SPI
 *   -- Auto-flip
 *   -- Auto-decrement
 *   -- Auto-increment
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
#include "adi_apollo_gpio.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"

static int32_t apollo_cnco_init_profile_load(adi_apollo_device_t *device, uint32_t nco_phase_inc_words[], uint32_t nco_phase_offset_words[], uint32_t length);
static int32_t apollo_cnco_direct_spi_hop(adi_apollo_device_t *device, uint8_t nco_chan_sel);
static int32_t apollo_cnco_scheduled_spi_hop(adi_apollo_device_t *device, uint8_t init_nco_hop_config, uint8_t next_hop, uint8_t trig_type);
static int32_t apollo_cnco_auto_hop(adi_apollo_device_t *device, adi_apollo_coarse_nco_hop_t *config, uint8_t init_nco_hop_config, uint8_t trig_type);

int32_t tx_nco_ffh(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[])
{
    int32_t err = API_CMS_ERROR_OK;
    int32_t i;
    bool direct_regmap = true;
    bool direct_regmap_triggered = true;
    bool auto_decrement = true;
    bool auto_increment = true;
    bool auto_flip = true;

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Set NCO dc test values */
    adi_apollo_cnco_test_mode_val_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_CNCO_MXR_TEST_TX_FS);

    /* INVSINC is configured by device profile, but can be enabled/disabled on demand. */
    adi_apollo_invsinc_enable(device, ADI_APOLLO_TX_INVSINC_ALL, 1);

    // Configure trigger master before oneshot
    adi_apollo_trig_mst_config_t trig_config = {
        .trig_enable = 1,
        .trig_offset = 31,
        .trig_period = 20000000000, // 1s
    };

    adi_apollo_coarse_nco_hop_t nco_hop_config;
    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_AUTO;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DECR;

    adi_apollo_cnco_hop_enable(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
    adi_apollo_trigts_mst_config(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_A, ADI_APOLLO_TRIG_MST_CNCO_0 | ADI_APOLLO_TRIG_MST_CNCO_1, &trig_config);
    adi_apollo_trigts_mst_mute(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_A, ADI_APOLLO_TRIG_MST_CNCO_0 | ADI_APOLLO_TRIG_MST_CNCO_1, 1);
    adi_apollo_trigts_cdrc_trig_sel_mux_set(device, ADI_APOLLO_TX, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_TRIG_MASTER);       // Trigger CDDC

    /* Configure timestamp reset for SYSREF based mode and execute a sync reset (required for timestamp triggered hopping) */
    adi_apollo_trigts_ts_reset_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_A, ADI_APOLLO_TRIG_TS_RESET_MODE_SYSREF);
    adi_apollo_trigts_ts_reset(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_A, 1);

    adi_apollo_hal_delay_us(device, 3000000);  // 3 sec pause to see tone

    /* Inspect the Inverse Sinc states */
    err = adi_ads10_apollo_ex_inspect_invsinc_all(device);
    ADI_CMS_ERROR_RETURN(err);


    /* Load NCO FTW values for all CNCO */
    uint32_t nco_phase_inc_words[] = {0x20000000, 0x24000000, 0x28000000, 0x2C000000,
                                      0x30000000, 0x34000000, 0x38000000, 0x3C000000,
                                      0x40000000, 0x44000000, 0x48000000, 0x4C000000,
                                      0x50000000, 0x54000000, 0x58000000, 0x5C000000};

    uint32_t nco_phase_offset_words[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t length = ADI_APOLLO_CNCO_PROFILE_NUM;

    apollo_cnco_init_profile_load(device, nco_phase_inc_words, nco_phase_offset_words, length);

    /* Configure Direct Regmap Mode Hopping and nco profile number to hop */
    if (direct_regmap) {
        printf("\n\nDirect Regmap Mode Hopping\n");
        for (i = 0; i < 2 * ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
            err = apollo_cnco_direct_spi_hop(device, i % ADI_APOLLO_CNCO_PROFILE_NUM);
            ADI_CMS_ERROR_RETURN(err);
            printf("\tHopped to profile %d\n", i % ADI_APOLLO_CNCO_PROFILE_NUM);
            adi_apollo_hal_delay_us(device, 500000);
        }
    }

    for (int trigger_type = ADI_APOLLO_TRIG_SPI; trigger_type < ADI_APOLLO_TRIG_MASTER + 1; trigger_type++) {

        /* Select SPI trigger for the trig selection mux */
        adi_apollo_trigts_reconfig_trig_sel_mux_set(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, trigger_type);   // Trigger Reconfig
        adi_apollo_trigts_cdrc_trig_sel_mux_set(device, ADI_APOLLO_TX, ADI_APOLLO_CDDC_ALL, trigger_type);       // Trigger CDDC

        if (trigger_type == ADI_APOLLO_TRIG_INPUT) {
            printf("\nGPIO Triggering\n");
            adi_apollo_gpio_quick_config_mode_set(device, 0);
            //adi_apollo_gpio_cmos_func_mode_set(device, GPIO_PIN, );
        } else if (trigger_type == ADI_APOLLO_TRIG_MASTER) {
            printf("\nMaster Triggering\n");
            adi_apollo_trigts_mst_mute(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MST_CNCO_0 | ADI_APOLLO_TRIG_MST_CNCO_1, 0);
        } else {
            printf("\nSPI Triggering\n");
        }

        if (direct_regmap_triggered) {
            /* Configure Scheduled SPI based hopping with trigger and nco profile number to hop */
            printf("\n\nScheduled SPI Hopping\n");
            for (i = 0; i < 2 * ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
                err = apollo_cnco_scheduled_spi_hop(device, i==0, i % ADI_APOLLO_CNCO_PROFILE_NUM, trigger_type);
                ADI_CMS_ERROR_RETURN(err);
                if (trigger_type != ADI_APOLLO_TRIG_MASTER) {
                    printf("\tHopped to profile %d\n", i % ADI_APOLLO_CNCO_PROFILE_NUM);
                }
                adi_apollo_hal_delay_us(device, 500000);
            }
        }

        nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_AUTO;

        if (auto_flip) {
            /* Configure SPI triggered, Auto Decrement hopping */
            printf("\n\nAuto Flip Hopping\n");
            nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_FLIP;
            nco_hop_config.high_limit = ADI_APOLLO_CNCO_PROFILE_NUM/2;
            nco_hop_config.low_limit = 0;
            nco_hop_config.next_hop_number_wr_en = 0;
            nco_hop_config.hop_ctrl_init = 0;
            for (i = 0; i<ADI_APOLLO_CNCO_PROFILE_NUM; ++i) {
                err = apollo_cnco_auto_hop(device, &nco_hop_config, i==0, trigger_type);
                ADI_CMS_ERROR_RETURN(err);
                if (trigger_type != ADI_APOLLO_TRIG_MASTER) {
                    printf("\tProfile hop %d\n", i);
                }
                adi_apollo_hal_delay_us(device, 500000);
            }        
        }

        if (auto_decrement) {
            /* Configure SPI triggered, Auto Decrement hopping */
            printf("\n\nAuto Decrement Hopping\n");
            nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DECR;
            for (i = 0; i<ADI_APOLLO_CNCO_PROFILE_NUM * 2; ++i) {
                err = apollo_cnco_auto_hop(device, &nco_hop_config, i==0, trigger_type);
                ADI_CMS_ERROR_RETURN(err);
                if (trigger_type != ADI_APOLLO_TRIG_MASTER) {
                    printf("\tProfile hop %d\n", i);
                }
                adi_apollo_hal_delay_us(device, 500000);
            }        
        }

        if (auto_increment) {
            /* Configure SPI triggered, Auto Decrement hopping */
            printf("\n\nAuto Increment Hopping\n");
            nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_INCR;
            for (i = 0; i<ADI_APOLLO_CNCO_PROFILE_NUM * 2; ++i) {
                err = apollo_cnco_auto_hop(device, &nco_hop_config, i==0, trigger_type);
                ADI_CMS_ERROR_RETURN(err);
                if (trigger_type != ADI_APOLLO_TRIG_MASTER) {
                    printf("\tProfile hop %d\n", i);
                }
                adi_apollo_hal_delay_us(device, 500000);
            }        
        }
    }

    return err;
}



static int32_t apollo_cnco_init_profile_load(adi_apollo_device_t *device, uint32_t nco_phase_inc_words[], uint32_t nco_phase_offset_words[], uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;

    err = adi_apollo_cnco_profile_load(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL,
                                           ADI_APOLLO_NCO_PROFILE_PHASE_INCREMENT, 0,
                                           nco_phase_inc_words, length);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t apollo_cnco_direct_spi_hop(adi_apollo_device_t *device, uint8_t nco_chan_sel)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_apollo_nco_profile_sel_mode_e profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP;

    err = adi_apollo_cnco_profile_sel_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, profile_sel_mode);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_cnco_active_profile_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, nco_chan_sel);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t apollo_cnco_scheduled_spi_hop(adi_apollo_device_t *device, uint8_t init_nco_hop_config, uint8_t next_hop, uint8_t trig_type)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_apollo_coarse_nco_hop_t nco_hop_config;

    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_REGMAP;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_INCR;    // No actual auto increment.
    nco_hop_config.hop_ctrl_init = 1;
    nco_hop_config.next_hop_number_wr_en = 1;

    if (init_nco_hop_config == 1) {
        err = adi_apollo_cnco_hop_enable(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_apollo_cnco_next_hop_num_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, next_hop);
    ADI_CMS_ERROR_RETURN(err);

    if (trig_type == ADI_APOLLO_TRIG_SPI) {
        err = adi_apollo_trigts_trig_now(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

static int32_t apollo_cnco_auto_hop(adi_apollo_device_t *device, adi_apollo_coarse_nco_hop_t *config, uint8_t init_nco_hop_config, uint8_t trig_type)
{
    int32_t err = API_CMS_ERROR_OK;

    if (init_nco_hop_config == 1) {
        err = adi_apollo_cnco_hop_enable(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, config);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (trig_type == ADI_APOLLO_TRIG_SPI) {
        err = adi_apollo_trigts_trig_now(device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}
