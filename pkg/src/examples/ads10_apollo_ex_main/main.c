#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo example program main
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
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_clk.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "apollo_examples.h"
#include "ads10_fpga.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vga.h"

#ifdef ADI_EMB_TESTS
#include "apollo_emb_tests.h"
#endif /*ADI_EMB_TESTS*/

#include "id00_uc04_f.h"        /* Sample Repeat for lane rate matching */
#include "id00_uc06a.h"         /* 10Gsps, jesd: 10.3125Gbps */
#include "id00_uc06.h"          /* 20Gsps, jesd: 10.3125Gbps */
#include "id00_uc06_U.h"        /* 20Gsps, jesd: 10.3125Gbps */
#include "id00_uc06sc1_lb.h"    /* 20Gsps, jesd: 10.3125Gbps, subclass 1, FPGA JESD loopback */
#include "id00_uc06sc1.h"       /* 20Gsps, jesd: 10.3125Gbps, subclass 1 */
#include "id00_uc06lb.h"        /* 20Gsps, jesd: 10.3125Gbps, loopback 3 */
#include "id00_uc06_P.h"        /* 20Gsps, jesd: 10.3125Gbps, PFILT */
#include "id00_uc06_smon.h"     /* 20Gsps, jesd: 10.3125Gbps, SMON 5-bit framer */
#include "id00_uc06_smon_10b.h" /* 20Gsps, jesd: 10.3125Gbps, SMON 10-bit framer */
#include "id00_uc06a_T.h"       /* 10Gsps, NCO test */
#include "id00_uc06_T.h"        /* 20Gsps, NCO test */
#include "id00_uc06a_PT.h"      /* 10Gsps, NCO test, PFILT */
#include "id00_uc06a_Q.h"       /* 10Gsps, Clk PLL 200MHz, for ext ref */
#include "id00_uc06a_FQ250.h"   /* 10Gsps, Clk PLL 250MHz, for hmc7044 ref, FSRC N = 15625 M = 12288 */
#include "id00_uc06a_Q250.h"    /* 10Gsps, Clk PLL 250MHz, for hmc7044 ref */
#include "id00_uc06_PT.h"       /* 20Gsps, NCO test, PFILT */
#include "id00_uc06_C.h"        /* 20Gsps, jesd: 10.3125Gbps, CFIR */
#include "id00_uc06xA2.h"       /* 20Gsps, jesd: 10.3125Gbps */
#include "id00_uc06_204B.h"     /* 20Gsps, jesd: 12.5000Gbps, JESD 204B mode*/
#include "id00_uc08_f.h"        /* 20Gsps, jesd: 20.625Gbps */
#include "id00_uc08sc1_f.h"     /* 20Gsps, jesd: 20.625Gbps, subclass 1 */
#include "id00_uc08a.h"         /* 20Gsps, jesd: 20.625Gbps, FSRC N = 15625 M = 12288 */
#include "id00_uc08a_sc1.h"     /* 20Gsps, jesd: 20.625Gbps, FSRC N = 15625 M = 12288, Subclass 1 */
#include "id00_uc08a1.h"        /* 20Gsps, jesd: 20.625Gbps, FSRC N = 3125  M = 1872 */
#include "id00_uc08a1_sc1.h"    /* 20Gsps, jesd: 20.625Gbps, FSRC N = 3125  M = 1872, Subclass 1 */
#include "id00_uc06_F.h"        /* 20Gsps, jesd: 10.3125Gbps, FSRC 15625/12288 */
#include "id00_uc06_F_sc1.h"    /* 20Gsps, jesd: 10.3125Gbps, FSRC 15625/12288, Subclass 1 */
#include "id00_uc06j1_F.h"      /* 20Gsps, jesd: 10.3125Gbps, FSRC 100/99 */
#include "id00_uc13.h"          /* 20Gsps, jesd: 20.635Gbps, Np12, 1x4x */
#include "id00_uc13_sc1.h"      /* 20Gsps, jesd: 20.635Gbps, Np12, 1x4x, Subclass 1 */
#include "id00_uc13_sc1_DTT.h"  /* 20Gsps, jesd: 20.635Gbps, Np12, 1x4x, Subclass 1. Dual Clock. Sysref Present at C: F, A: T, B: T */
#include "id00_uc14_sc1.h"      /* 19.6608Gsps, jesd: 20.2752Gbps, Np16, 2x4x, Subclass 1 */
#include "id00_uc15_sc1.h"      /* 19.2000Gsps, jesd: 19.8Gbps, Np12, 1x4x, Subclass 1 */
#include "id01_uc05.h"          /* 14/28Gsps, jesd: 14.4375Gbps, Np12, 1x8x */
#include "id01_uc05_Q.h"        /* 14/28Gsps, jesd: 14.4375Gbps, Np12, 1x8x, Clk PLL */
#include "id98_uc05.h"          /* 8/8GSPS, jesd: 10.000Gbps, 8t8r, JESD204B */
#include "id98_uc05lb.h"        /* 8/8GSPS, jesd: 10.000Gbps, 8t8r, JESD204B, mapping for loopback */
#include "id01_uc66.h"          /* 20Gsps, jesd: 10.3125Gbps, lane adapt: 1 */
#include "id99_uc00.h"          /* 10Gsps PRBS 20.6250 Gbps (24 lanes) */
#include "id99_uc02.h"          /* 20Gsps, jesd: 27.5Gbps, full bandwidth 1x1x mode. PRBS 27.5 Gbps (24 lanes)*/
#include "id99_uc02c_sc1.h"     /* 19660.0Gsps, jesd: 27.0336Gbps, full bandwidth 1x1x mode, SC1 */
#include "id99_uc02c_sc1_DTT.h" /* 19660.0Gsps, jesd: 27.0336Gbps, full bandwidth 1x1x mode, SC1. Dual Clock. Sysref Present at C: F, A: T, B: T */
#include "id81_uc28.h"          /* 8Gsps, jesd: 16.5Gbps, 8t8r, JESD204C */
#include "id81_uc28_sc1.h"      /* 8Gsps, jesd: 16.5Gbps, 8t8r, JESD204C, sc1 */
#include "id81_uc28_smon.h"     /* 8Gsps, jesd: 16.5Gbps, 8t8r, JESD204C, SMON 5-bit framer */
#include "id81_uc28_smon_10b.h" /* 8Gsps, jesd: 16.5Gbps, 8t8r, JESD204C, SMON 10-bit framer */
#include "id99_uc09.h"          /* 20Gsps, jesd: 27.5Gbps, Np8 (24 lanes) */
#include "id00_uc08.h"          /* 20Gsps, jesd: 25Gbps, TX FSRC N = 3125 M = 1872, RX FSRC N = 3125 M = 2496, tot_dcm != link_dcm */
#include "id02_uc34.h"          /* 20Gsps, jesd 20.635Gbps, 1T1R */
#include "id99_uc07.h"          /* Quad Link */

static int32_t parse_cli_args(int argc, char *argv[], adi_apollo_ex_test_t *ex_func, adi_apollo_top_t **profile, adi_ads10_apollo_clk_mode_e *clk_mode, int* argc_ofst);
static void print_usage();

int32_t main(int argc, char *argv[])
{
    int32_t err;
    int i;
    int argc_ofst;                                    /* argc idx offset where local ex cmd args begin in argv[] */
    adi_apollo_device_t device = { {0} };             /* Apollo device object */
    adi_fpga_apollo_device_t fpga_device = { {0} };   /* FPGA device object */
    adi_apollo_ex_test_t ex_func = NULL;              /* Ptr to example function to run */
    adi_apollo_top_t *profile = NULL;                 /* Profile to load */
    uint8_t uuid[ADI_APOLLO_UUID_NUM_BYTES];          /* Buffer for UUID raw bytes */
    char uuid_str[256];                               /* Buffer for UUID string */
    uint8_t die_id;
    uint8_t is_ads10_open = 0;                        /* Indicates if ads10 is opened */

    // Clocks to configure
    adi_ads10_apollo_clk_mode_e clk_mode = ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER | ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL;
    // Frequency going to the LTC6955 (J17 Frequency or 125MHz for onboard crystal).
    uint32_t ltc6955_clk_khz = 125e3;
    adi_apollo_hal_protocol_e protocol;
    bool enable_hsci = false;
    bool enable_rmw = true;
    bool is_fmcb_eval = false;      // Set 'true' when using an FMCB Eval Brd

    /* Command line */
    for (i = 0; i < argc; i++) {
        printf("%s%s", argv[i], i == argc - 1 ? "\n" : " ");
    }

    if (argc < 3) {
        printf("\nMust specify example type and profile.\n\n");
        print_usage();
        err = API_CMS_ERROR_ERROR;
        goto end;
    } else {
        err = parse_cli_args(argc, argv, &ex_func, &profile, &clk_mode, &argc_ofst);
        if (err != API_CMS_ERROR_OK) {
            printf("Unknown test or profile. err=%d\n\n", err);
            print_usage();
            goto end;
        }
    }

    // Use strlen(flag) + 1 in strncmp() to include the null terminator for exact string matching
    for (uint8_t i = argc_ofst; i < argc; ++i) {
        if (strncmp(argv[i], "-hsci", 6) == 0) {
            enable_hsci = true;
        }
    }

    /* Make sure OUTPUT_DIR exists */
    if (err = adi_apollo_utilities_mkdir(&device, true, OUTPUT_DIR), err != API_CMS_ERROR_OK) {
        printf("Failed to open %s\n", OUTPUT_DIR);
        return err;
    }

    bool is_rx_cfir = profile->rx_path[0].rx_cfir[0].enable || profile->rx_path[1].rx_cfir[0].enable; /* Determine if profile has rx-cfir enabled */
    bool is_tx_cfir = profile->tx_path[0].tx_cfir[0].enable || profile->tx_path[1].tx_cfir[0].enable; /* Determine if profile has tx-cfir enabled */

    bool is_rx_pfilt = profile->rx_path[0].rx_pfilt[0].enable || profile->rx_path[1].rx_pfilt[0].enable; /* Determine if profile has rx-pfilt enabled */
    bool is_tx_pfilt = profile->tx_path[0].tx_pfilt[0].enable || profile->tx_path[1].tx_pfilt[0].enable; /* Determine if profile has tx-pfilt enabled */
    bool is_rx_fsrc = profile->rx_path[0].rx_fsrc[0].enable || profile->rx_path[1].rx_fsrc[0].enable;    /* Determine if profile has rx-fsrc enabled */
    bool is_tx_fsrc = profile->tx_path[0].tx_fsrc[0].enable || profile->tx_path[1].tx_fsrc[0].enable;    /* Determine if profile has tx-fsrc enabled */

    /* Set FPGA Ref clk divider to 64 for JRx Eye Sweep and PRBS example */
    if ((ex_func == &jrx_eye_sweep) || (ex_func == &prbs)) {
        fpga_device.state_info.clk_info.is_prbs = 1;
    } else {
        fpga_device.state_info.clk_info.is_prbs = 0;
    }

    printf("ADI Apollo Example Code on ADS10 platform\n");
    printf("Example: %s, Device Profile: %s\n", argv[1], argv[2]);
    adi_ads10_apollo_ex_inspect_lane_rates(&fpga_device, profile, clk_mode);
    printf("RX CFIR: %s\n", is_rx_cfir ? "enabled" : "disabled");
    printf("TX CFIR: %s\n", is_tx_cfir ? "enabled" : "disabled");
    printf("RX PFILT: %s\n", is_rx_pfilt ? "enabled" : "disabled");
    printf("TX PFILT: %s\n", is_tx_pfilt ? "enabled" : "disabled");
    printf("RX FSRC:  %s\n", is_rx_fsrc ? "enabled" : "disabled");
    printf("TX FSRC:  %s\n", is_tx_fsrc ? "enabled" : "disabled");

    printf("*** Assume ADS10 FPGA image has been configured, power supplies up and ext clock on ***\n");

    /* Open ADS10 platform - This will init the FPGA SPI Master */
    if (err = ads10_hw_open("apollo_app.log"), err != API_CMS_ERROR_OK) {
        printf("ADS10 failed to open. Exiting...\n");
        goto end;
    }
    is_ads10_open = 1;

    /* Create an ADS10 platform instance. This provides FPGA register access. */
    adi_fpga_apollo_hal_config_t *ads10_platform = ads10_apollo_hal_instance(); // Allocates memory for the platform HAL data structure.

    /* Wire up the platform specific FPGA HAL for the FPGA API */
    adi_ads10_apollo_ex_configure_fpga_hal(&fpga_device, ads10_platform);

    /* Wire up the platform specific APOLLO HAL for the Apollo API */
    adi_ads10_apollo_ex_configure_hal(&device, ads10_platform, (uint8_t)enable_hsci);

    /* Setup the startup sequence info used by adi_apollo_startup_execute()  */
    adi_ads10_apollo_ex_configure_startup(&device);

    /* Perform any FPGA init before powering up Apollo (e.g. stop play/capture to reduce pwr) */
    adi_ads10_apollo_ex_fpga_pre_reset(&fpga_device);

    /* Open the Apollo API. This will execute required API initialization. */
    if (err = adi_apollo_device_hw_open(&device, ADI_APOLLO_HARD_RESET_AND_INIT), err != API_CMS_ERROR_OK) {
        printf("Error occurred opening device(). err=%d\n", err);
        goto end;
    }

    /* Print device type 4T4R or 8T8R */
    printf("Device is: %s\n", device.dev_info.is_8t8r ? "8T8R" : "4T4R");

    /* Set the active driver to SPI0 */
    adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0);

    /* Disable the read-modify-write bitfield setting */
    adi_apollo_hal_rmw_enable_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0, 0);

    /* Print spi config reg (e.g. msb first, sdo, etc.) */
    adi_ads10_apollo_ex_print_regs(&device, 0x47000000, 1);

    /* Execute optional ADS10 FPGA register access test */
    err = adi_ads10_apollo_ex_fpga_reg_test(&fpga_device);
    printf("FPGA register access test: %s\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    ADI_CMS_ERROR_GOTO(err, end);

    /* Execute optional Apollo register access test */
    err = adi_ads10_apollo_ex_reg_test(&device);
    printf("Apollo register access test: %s\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    ADI_CMS_ERROR_GOTO(err, end);

    if (enable_hsci) {
        /* Set the active protocol to HSCI for remainder of execution */
        adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_HSCI);

        /*  Execute an optional Apollo register access test using HSCI */
        err = adi_ads10_apollo_ex_reg_test(&device);
        printf("HSCI reg test: %s\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    adi_apollo_hal_active_protocol_get(&device, &protocol);
    adi_apollo_hal_rmw_enable_set(&device, protocol, enable_rmw);
    printf("Active Apollo HAL Protocol: %s, RMW: %s\n", (protocol == 0) ? "SPI0" : "HSCI", enable_rmw ? "enabled" : "disabled");

    /* Configure clks */
    err = adi_ads10_apollo_ex_configure_profile_clks(&fpga_device, ltc6955_clk_khz, profile, clk_mode);
    ADI_CMS_ERROR_GOTO(err, end);

    if (ex_func == &rx_adc_ms) {
        printf("Enabling ADC mode switch...\n");
        err = adi_apollo_adc_mode_switch_enable_set(&device, 1);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    /* Load firmware image, device profile and configure digital data path */
    printf("Running Apollo startup sequence...\n");
    err = adi_ads10_apollo_ex_startup(&device, profile);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Print the uuid of device */
    adi_apollo_device_uuid_get(&device, uuid, ADI_APOLLO_UUID_NUM_BYTES);
    adi_ads10_ex_uuid_to_str(uuid, ADI_APOLLO_UUID_NUM_BYTES, uuid_str, 256);
    adi_apollo_hal_log_write(&device, ADI_CMS_LOG_MSG, "UUID: %s", uuid_str);
    printf("UUID: %s\n", uuid_str);

    /* Print die id (e.g. A2 Si = 2)*/
    adi_apollo_device_die_id_get(&device, &die_id);
    adi_apollo_hal_log_write(&device, ADI_CMS_LOG_MSG, "dev id: %d", die_id);
    printf("Die ID: %d\n", die_id);

    /* Cal clk power level */
    err = adi_ads10_apollo_ex_clk_power_cal(&device,
                                            clk_mode,
                                            (uint64_t)(profile->clk_cfg.dev_clk_freq_kHz * 1e3),
                                            (uint64_t)(ltc6955_clk_khz * 1e3));
    ADI_CMS_ERROR_GOTO(err, end);

    /* Initialize FPGA configuration from device profile (jesd, fsrc) */
    err = adi_ads10_apollo_ex_fpga_profile_init(&fpga_device, profile);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Initialize Tx and Rx VGAs populated on FMCB Eval Brd */
    if (is_fmcb_eval) {
        err = adi_ads10_apollo_ex_tx_rx_vga_initialize(&fpga_device);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    /********** execute example function here **********/
    err = ex_func(&device, &fpga_device, profile, argc, argv, argc_ofst);
    printf("Example code exec returned: %s\n", err == API_CMS_ERROR_OK ? "OK" : "*ERROR*");
    adi_apollo_hal_log_write(&device, ADI_CMS_LOG_MSG, "Example code exec returned: %s\n", err == API_CMS_ERROR_OK ? "OK" : "*ERROR*");

end:

    if (is_ads10_open) {
        ads10_hw_close();
    }

    printf("Goodbye\n");
    EXCTL_END(err);

    return err;
}

/*
* Return the example function ptr, device profile, and clock source from command line args
*/
static int32_t parse_cli_args(int argc, char *argv[], adi_apollo_ex_test_t *ex_func, adi_apollo_top_t **profile, adi_ads10_apollo_clk_mode_e *clk_mode, int *argc_ofst)
{
    bool arg_match_found = false;

    if (argc < 3) {
        printf("Insufficient number of parameters specified.\n");
        print_usage();
        return API_CMS_ERROR_ERROR;
    }

    typedef struct {
        const char *name;
        adi_apollo_ex_test_t func;
    } name_ex_func_t;

    typedef struct {
        const char *name;
        adi_apollo_top_t *profile;
    } name_uc_t;

    typedef struct {
        const char *name;
        adi_ads10_apollo_clk_mode_e clk_mode;
    } name_clk_t;

    /* Example function name map */
    name_ex_func_t name_funcs[] = {
        {"dp_load",                     dp_load},
        {"prbs",                        prbs},
        {"gpio_toggle",                 gpio_toggle},
        {"fullchip",                    fullchip},
        {"fullchip_8t8r",               fullchip_8t8r},
        {"fullchip_fsrc_dr",            fullchip_fsrc_dr},
        {"fullchip_fsrc_sc1_ext_trig",  fullchip_fsrc_sc1_ext_trig},
        {"fullchip_hop",                fullchip_hop},
        {"fullchip_pfilt",              fullchip_pfilt},
        {"fullchip_sc1_dl",             fullchip_sc1_dl},
        {"fullchip_mcs_sc1_dl",         fullchip_mcs_sc1_dl},
        {"fullchip_sr_dr",              fullchip_sr_dr},
        {"tx_nco",                      tx_nco},
        {"tx_nco_ffh",                  tx_nco_ffh},
        {"tx_nco_pfilt",                tx_nco_pfilt},
        {"rx_adc_cc",                   rx_adc_cc},
        {"rx_adc_nz",                   rx_adc_nz},
        {"rx_adc_ms",                   rx_adc_ms},
        {"rx_jesd",                     rx_jesd},
        {"rx_adc_mux2",                 rx_adc_mux2},
        {"rx_adc_pave",                 rx_adc_pave},
        {"rx_adc_pfilt",                rx_adc_pfilt},
        {"rx_bmem_pfilt",               rx_bmem_pfilt},
        {"tx_jesd",                     tx_jesd},
        {"rx_bmem_cfir",                rx_bmem_cfir},
        {"tx_jesd_cfir",                tx_jesd_cfir},
        {"rx_bmem_ddc",                 rx_bmem_ddc},
        {"tx_jesd_file",                tx_jesd_file},
        {"rx_adc_deep",                 rx_adc_deep},
        {"rx_sniffer",                  rx_sniffer},
        {"loopback0",                   loopback0},
        {"loopback1_2",                 loopback1_2},
        {"rx_bmem_delay",               rx_bmem_delay},
        {"lb0_bmem_delay_hop",          lb0_bmem_delay_hop},
        {"tx_nco_mod",                  tx_nco_mod},
        {"rx_adc_fd",                   rx_adc_fd},
        {"rx_adc_smon",                 rx_adc_smon},
        {"jesd_loopback",               jesd_loopback},
        {"rx_adc_bmem",                 rx_adc_bmem},
        {"fullchip_sparse_cfir",        fullchip_sparse_cfir},
        {"mcs_cal",                     mcs_cal},
        {"fmcb_aux",                    fmcb_aux},
        {"bsync_tof",                   bsync_tof},
        {"jrx_eye_sweep",               jrx_eye_sweep},
        {"power_readback",              power_readback}
    };
    int n_funcs = sizeof(name_funcs)/sizeof(name_funcs[0]);

    arg_match_found = false;
    for (int i=0; i<n_funcs; i++) {
        if (!strcmp(name_funcs[i].name, argv[1])) {
            // Found match
            *ex_func = name_funcs[i].func;
            arg_match_found = true;
            break;
        }
    }

#ifdef ADI_EMB_TESTS
    if (!arg_match_found) {
        *ex_func = emb_test_func_get(argv[1]);
        arg_match_found = (*ex_func != NULL);
    }
#endif /*ADI_EMB_TESTS*/

    ADI_ADS10_CHECK_ARG_MATCH(arg_match_found);

    /* Device profile name map */
    name_uc_t name_ucs[] = {
        {"id00_uc04_f",         &id00_uc04_f_0},
        {"id00_uc06a",          &id00_uc06a_0},
        {"id00_uc06",           &id00_uc06_0},
        {"id00_uc06_U",         &id00_uc06_U_0},
        {"id00_uc06sc1",        &id00_uc06sc1_0},
        {"id00_uc06sc1_lb",     &id00_uc06sc1_lb_0},
        {"id00_uc06_P",         &id00_uc06_P_0},
        {"id00_uc06_smon",      &id00_uc06_smon_0},
        {"id00_uc06_smon_10b",  &id00_uc06_smon_10b_0},
        {"id00_uc06a_T",        &id00_uc06a_T_0},
        {"id00_uc06_T",         &id00_uc06_T_0},
        {"id00_uc06a_PT",       &id00_uc06a_PT_0},
        {"id00_uc06a_Q",        &id00_uc06a_Q_0},
        {"id00_uc06a_FQ250",    &id00_uc06a_FQ250_0},
        {"id00_uc06a_Q250",     &id00_uc06a_Q250_0},
        {"id00_uc06_PT",        &id00_uc06_PT_0},
        {"id00_uc06_C",         &id00_uc06_C_0},
        {"id00_uc06xA2",        &id00_uc06xA2_0},
        {"id00_uc06lb",         &id00_uc06lb_0},
        {"id00_uc06_204B",      &id00_uc06_204B_0},
        {"id00_uc08_f",         &id00_uc08_f_0},
        {"id00_uc08sc1_f",      &id00_uc08sc1_f_0},
        {"id00_uc08a",          &id00_uc08a_0},
        {"id00_uc08a_sc1",      &id00_uc08a_sc1_0},
        {"id00_uc08a1",         &id00_uc08a1_0},
        {"id00_uc08a1_sc1",     &id00_uc08a1_sc1_0},
        {"id00_uc06_F",         &id00_uc06_F_0},
        {"id00_uc06_F_sc1",     &id00_uc06_F_sc1_0},
        {"id00_uc06j1_F",       &id00_uc06j1_F_0},
        {"id00_uc13",           &id00_uc13_0},
        {"id00_uc13_sc1",       &id00_uc13_sc1_0},
        {"id00_uc13_sc1_DTT",   &id00_uc13_sc1_DTT_0},
        {"id00_uc14_sc1",       &id00_uc14_sc1_0},
        {"id00_uc15_sc1",       &id00_uc15_sc1_0},
        {"id01_uc05",           &id01_uc05_0},
        {"id01_uc05_Q",         &id01_uc05_Q_0},
        {"id01_uc66",           &id01_uc66_0},
        {"id98_uc05lb",         &id98_uc05lb_0},
        {"id98_uc05",           &id98_uc05_0},
        {"id99_uc00",           &id99_uc00_0},
        {"id99_uc02",           &id99_uc02_0},
        {"id99_uc02c_sc1",      &id99_uc02c_sc1_0},
        {"id99_uc02c_sc1_DTT",  &id99_uc02c_sc1_DTT_0},
        {"id81_uc28",           &id81_uc28_0},
        {"id81_uc28_sc1",       &id81_uc28_sc1_0},
        {"id81_uc28_smon",      &id81_uc28_smon_0},
        {"id81_uc28_smon_10b",  &id81_uc28_smon_10b_0},
        {"id99_uc09",           &id99_uc09_0},
        {"id00_uc08",           &id00_uc08_0},
        {"id02_uc34",           &id02_uc34_0},
        {"id99_uc07",           &id99_uc07_0},
    };
    int n_profiles = sizeof(name_ucs)/sizeof(name_ucs[0]);

    arg_match_found = false;
    for (int i=0; i<n_profiles; i++) {
        if (!strcmp(name_ucs[i].name, argv[2])) {
            // Found match
            *profile = name_ucs[i].profile;
            arg_match_found = true;
            break;
        }
    }
    ADI_ADS10_CHECK_ARG_MATCH(arg_match_found);

    /* Device clock source name map */
    name_clk_t name_dev_clks[] = {
        {"-devclk=ext_center",   ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER},
        {"-devclk=ext_dual",     ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_DUAL},
        {"-devclk=adf4382",      ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382},
        {"-devclk=ext_pll",      ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_PLL},
        {"-devclk=hmc7044_pll",  ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_HMC7044_PLL},
    };

    name_clk_t name_fpga_clks[] = {
        {"-fpgaclk=ext",         ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL},
        {"-fpgaclk=fmc",         ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_FMC},
    };
    int n_dev_clk_srcs = sizeof(name_dev_clks)/sizeof(name_dev_clks[0]);
    int n_fpga_clk_srcs = sizeof(name_fpga_clks)/sizeof(name_fpga_clks[0]);
    adi_ads10_apollo_clk_mode_e dev_clk_mode = 0;
    adi_ads10_apollo_clk_mode_e fpga_clk_mode = 0;

    *argc_ofst = 3;  // index of first optional param. increment as options found
    if (argc == 3) {
        *clk_mode = ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER
                  | ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL;
    } else {
        for (int i = 3; i < argc; i++) {
            if (strstr(argv[i], "devclk") != NULL) {
                for (int ii = 0; ii < n_dev_clk_srcs; ii++) {
                    if (!strcmp(name_dev_clks[ii].name, argv[i])) {
                        dev_clk_mode = name_dev_clks[ii].clk_mode;
                        (*argc_ofst)++;
                        break;
                    }
                }
            }

            if (strstr(argv[i], "fpgaclk") != NULL) {
                for (int ii = 0; ii < n_fpga_clk_srcs; ii++) {
                    if (!strcmp(name_fpga_clks[ii].name, argv[i])) {
                        fpga_clk_mode = name_fpga_clks[ii].clk_mode;
                        (*argc_ofst)++;
                        break;
                    }
                }
            }
        }

        *clk_mode = (dev_clk_mode ? dev_clk_mode : ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER)
                  | (fpga_clk_mode ? fpga_clk_mode : ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL);
    }

    return API_CMS_ERROR_OK;
}

void print_usage()
{
    printf("Usage: apollo_main EXAMPLE_TYPE DEVICE_PROFILE [-devclk=DEV_CLK_SRC] [-fpgaclk=FPGA_CLK_SRC] [CMD_SPECIFIC_ARGS]\n");
    printf("\n");
    printf("EXAMPLE_TYPE summary:\n");
    printf("    dp_load                    id00_uc06 | id00_uc06_T | <any profile>\n");
    printf("    fullchip                   id00_uc06 | id00_uc06_F | id00_uc06j1_F | id00_uc08a | id00_uc08a1 | id00_uc06_204B\n");
    printf("    fullchip_8t8r              id98_uc05\n");
    printf("    fullchip_fsrc_dr           id00_uc06_F | id00_uc06j1_F | id00_uc08a | id00_uc08a1 [-i]\n");
    printf("    fullchip_fsrc_sc1_ext_trig id00_uc06_F_sc1 | id00_uc08a_sc1 | id00_uc08a1_sc1\n");
    printf("    fullchip_hop               id00_uc06 [-i]\n");
    printf("    fullchip_sparse_cfir       id00_uc06_C [-i]\n");
    printf("    fullchip_pfilt             id00_uc06 | id98_uc05 [-i]\n");
    printf("    fullchip_sc1_dl            id00_uc06sc1 | id00_uc13_sc1 | id00_uc14_sc1 | id00_uc15_sc1 | id00_uc08sc1_f\n");
    printf("    fullchip_mcs_sc1_dl        id00_uc08sc1_f | id00_uc13_sc1 | id00_uc14_sc1 | id00_uc15_sc1 | id99_uc02c_sc1\n");
    printf("    fullchip_sr_dr             id00_uc06 | id00_uc08_f [-i]\n");
    printf("    lb0_bmem_delay_hop         id00_uc06 [-i]\n");
    printf("    loopback0                  id00_uc06 | id00_uc08a [-i]\n");
    printf("    loopback1_2                id00_uc06lb | id98_uc05lb [-i]\n");
    printf("    jesd_loopback              id00_uc06lb\n");
    printf("    rx_adc_cc                  id00_uc06\n");
    printf("    rx_adc_deep                id00_uc06 | id01_uc05\n");
    printf("    rx_adc_bmem                id00_uc06 | id98_uc05\n");
    printf("    rx_adc_fd                  id00_uc06 [-i]\n");
    printf("    rx_adc_ms                  id00_uc06\n");
    printf("    rx_adc_nz                  id00_uc06\n");
    printf("    rx_adc_mux2                id00_uc06 | id98_uc05\n");
    printf("    rx_adc_pave                id00_uc06, id98_uc05\n");
    printf("    rx_adc_pfilt               id00_uc06_P | id98_uc05\n");
    printf("    rx_adc_smon                id00_uc06_smon | id00_uc06_smon_10b | id81_uc28_smon | id81_uc28_smon_10b | ... [-i][-c][-v]\n");
    printf("    rx_bmem_cfir               id00_uc06_C\n");
    printf("    rx_bmem_ddc                id00_uc06\n");
    printf("    rx_jesd {-b for BMEM AWG}  id00_uc06_F | id00_uc08a | id00_uc08a1\n");
    printf("    rx_bmem_pfilt              id00_uc06_P\n");
    printf("    rx_bmem_delay              id00_uc06xA2\n");
    printf("    rx_sniffer                 id00_uc06 [-i]\n");
    printf("    tx_jesd_cfir               id00_uc06_C [-i]\n");
    printf("    tx_jesd                    id00_uc06_F | id00_uc08a | id00_uc08a1\n");
    printf("    tx_nco                     id00_uc06_T | id00_uc06a_T [-i]\n");
    printf("    tx_nco_ffh                 id00_uc06_T\n");
    printf("    tx_nco_mod                 id00_uc06_T\n");
    printf("    tx_nco_pfilt               id00_uc06_PT | id00_uc06a_PT\n");
    printf("    tx_jesd_file               id00_uc06 | id00_uc13 {-a | -al | -f i_file q_file [cnco]}\n");
    printf("    mcs_cal                    id00_uc13_sc1 | id99_uc02c_sc1\n");
    printf("    prbs                       id00_uc06a_T | id99_uc00 | id99_uc02\n");
    printf("    gpio_toggle                id00_uc06\n");
    printf("    fmcb_aux                   id00_uc06\n");
    printf("    bsync_tof                  id00_uc13_sc1 | id99_uc02c_sc1\n");
    printf("    jrx_eye_sweep              id00_uc06a_T | id99_uc00 | id99_uc02\n");
    printf("    power_readback             id00_uc06 | <any profile>\n");

    printf("\n");
    printf("DEVICE_PROFILE summary:\n");
    printf("    Dev Profile         Fclk (MHz)  FPGA (MHz)  DR (MHz)  LR (Gbps)  FSRC                     PFILT    CFIR        Note\n");
    printf("    ===========         ==========  ==========  ========  ========   =======================  =======  ==========  ================================================\n");
    printf("    id00_uc06           20000.0     156.25      1250      10.3125    no                       bypass                                                               \n");
    printf("    id00_uc06_U         20000.0     156.25      1250      10.3125    no                       bypass               Dual-Clk scheme                                 \n");
    printf("    id00_uc06_C         20000.0     156.25      1250      10.3125    no                       bypass   non-sparse                                                  \n");
    printf("    id00_uc06_F         20000.0     156.25      983.04    10.3125    15625/12288              bypass                                                               \n");
    printf("    id00_uc06_F_sc1     20000.0     156.25      983.04    10.3125    15625/12288              bypass               FSRC w/ SC1, trig sync                          \n");
    printf("    id00_uc06lb         20000.0     156.25      1250      10.3125    no                       bypass               Muxing for loopback                             \n");
    printf("    id00_uc06j1_F       20000.0     156.25      1237.5    10.3125    100/99                   bypass                                                               \n");
    printf("    id00_uc06sc1        20000.0     156.25      1250      10.3125    no                       bypass               Subclass 1 testing                              \n");
    printf("    id00_uc06sc1_lb     20000.0     161.1328125 1250      10.3125    no                       bypass               Subclass 1 testing. Requires loopback fpga image\n");
    printf("    id00_uc06_P         20000.0     156.25      1250      10.3125    no                       real n/2                                                             \n");
    printf("    id00_uc06_PT        20000.0     n/a         n/a       n/a        no                       real n/2             NCO test mode                                   \n");
    printf("    id00_uc06_smon      20000.0     156.25      1250      10.3125    no                       bypass               SMON 5-bit framer                               \n");
    printf("    id00_uc06_smon_10b  20000.0     156.25      1250      10.3125    no                       bypass               SMON 10-bit framer                              \n");
    printf("    id00_uc06_T         20000.0     n/a         n/a       n/a        no                       bypass               NCO test mode                                   \n");
    printf("    id00_uc06xA2        20000.0     156.25      1250      10.3125    no                       bypass               ADC 0/1, DAC 0/1 through same datapath          \n");
    printf("    id00_uc06a          10000.0     156.25      1250      10.3125    no                       bypass                                                               \n");
    printf("    id00_uc06a_PT       10000.0     n/a         n/a       n/a        no                       real n/2             NCO test mode                                   \n");
    printf("    id00_uc06a_Q        10000.0     156.25      1250      10.3125    no                       bypass               Clk PLL 200MHz ext ref, 10/10                   \n");
    printf("    id00_uc06a_FQ250    10000.0     156.25      1250      10.3125    no                       bypass               Clk PLL 250MHz hmc7044 ref, 10/10, FSRC         \n");
    printf("    id00_uc06a_Q250     10000.0     156.25      1250      10.3125    no                       bypass               Clk PLL 250MHz hmc7044 ref, 10/10               \n");
    printf("    id00_uc06a_T        10000.0     n/a         n/a       n/a        no                       bypass               NCO test mode                                   \n");
    printf("    id00_uc06_204B      20000.0     312.5       1250      12.500     no                       bypass               4T4R, JESD204B                                  \n");
    printf("    id00_uc08_f         20000.0     312.5       2500      20.625     no                       bypass                                                               \n");
    printf("    id00_uc08sc1_f      20000.0     312.5       2500      20.625     no                       bypass               Subclass 1                                      \n");
    printf("    id00_uc08a          20000.0     312.5       1966.08   20.625     15625/12288              bypass                                                               \n");
    printf("    id00_uc08a_sc1      20000.0     312.5       1966.08   20.625     15625/12288              bypass               FSRC w/ SC1, trig sync                          \n");
    printf("    id00_uc08a1         20000.0     312.5       1497.6    20.625     3125/1872                bypass                                                               \n");
    printf("    id00_uc08a1_sc1     20000.0     312.5       1497.6    20.625     3125/1872                bypass               FSRC w/ SC1, trig sync                          \n");
    printf("    id00_uc13           20000.0     312.5       5000      20.625     no                       bypass               Np=12, 4G IBW                                   \n");
    printf("    id00_uc13_sc1       20000.0     312.5       5000      20.625     no                       bypass               Np=12, 4G IBW, SC1                              \n");
    printf("    id00_uc13_sc1_DTT   20000.0     312.5       5000      20.625     no                       bypass               Np=12, 4G IBW, SC1. Dual-Clk A/B SYSREF Present \n");
    printf("    id00_uc14_sc1       19660.8     307.2       2457.6    20.2752    no                       bypass               Np=16, SC1                                      \n");
    printf("    id00_uc15_sc1       19200.0     300.0       4800      19.800     no                       bypass               Np=12, L=12, SC1                                \n");
    printf("    id01_uc05           14000.0     218.75      3500      10.3125    no                       bypass               Np=12, 14/28                                    \n");
    printf("    id01_uc05_Q         14000.0     218.75      3500      10.3125    no                       bypass               Clk PLL 250MHz, Np=12, 14/28                    \n");
    printf("    id98_uc05           8000.0      250.0       125.0     10.000     no                       bypass               8T8R, JESD204B                                  \n");
    printf("    id98_uc05lb         8000.0      250.0       125.0     10.000     no                       bypass               8T8R, JESD204B, loopback muxes                  \n");
    printf("    id01_uc66           20000.0     156.25      625       10.3125    no                       bypass               Lane_Adapt=1                                    \n");
    printf("    id99_uc00           10000.0     312.5       2500      20.625     no                       bypass               PRBS                                            \n");
    printf("    id99_uc02           20000.0     416.667     20000.0   27.5       no                       bypass               PRBS, Full BW, 1x1x mode, L=12, Np=16           \n");
    printf("    id99_uc02c_sc1      19660.8     409.6       19660.8   27.0336    no                       bypass               1x1x mode, L=12, Np=16, SC1                     \n");
    printf("    id99_uc02c_sc1_DTT  19660.8     409.6       19660.8   27.0336    no                       bypass               1x1x mode, L=12, Np=16, SC1. Dual-Clk A/B SYSREF Present \n");
    printf("    id81_uc28           8000.0      250.0       250       16.5       no                       bypass               8T8R, JESD204C                                  \n");
    printf("    id81_uc28_sc1       8000.0      250.0       250       16.5       no                       bypass               8T8R, JESD204C SC1                              \n");
    printf("    id81_uc28_smon      8000.0      250.0       250       16.5       no                       bypass               8T8R, JESD204C, SMON 5-bit framer               \n");
    printf("    id81_uc28_smon_10b  8000.0      250.0       250       16.5       no                       bypass               8T8R, JESD204C, SMON 10-bit framer              \n");
    printf("    id99_uc09           20000.0     416.667     20000.0   27.5       no                       bypass               NP=8                                            \n");
    printf("    id00_uc08           20000.0     250         2500      16.5       (3125/1872):(3125/2496)  bypass               Np=16, JESD204C, tot_dcm != link_dcm            \n");
    printf("    id02_uc34           20000.0     312.5       2500      20.625     no                       bypass               Np=16, 1T1R - Low Power                         \n");
    printf("\n");

    printf("\n");
    printf("DEV_CLK_SRC summary:\n");
    printf("    ext_center          Use external sig gen to clock Apollo -- CLK_C (default)\n");
    printf("    ext_dual            Use external sig gen to clock Apollo -- CLK_A + CLK_B\n");
    printf("    adf4382             Use ADF4382 to clock Apollo\n");
    printf("    ext_pll             Use external sig gen for Apollo clock PLL ref\n");
    printf("    hmc7044_pll         Use HMC7044 for Apollo clock PLL ref\n");

    printf("\n");
    printf("FPGA_CLK_SRC summary:\n");
    printf("    ext            Use external sig gen to clock the ADS10 (default)\n");
    printf("    fmc            Use HMC7044 to clock the ADS10\n");

#ifdef ADI_EMB_TESTS
    emb_test_usage_print();
#endif /*ADI_EMB_TESTS*/
}

#endif /* !defined(VERSAL_PLATFORM) */
