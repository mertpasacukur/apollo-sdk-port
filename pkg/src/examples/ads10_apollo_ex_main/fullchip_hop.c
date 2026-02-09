/*!
 * \brief     ADS10 Apollo Fullchip Hopping example code
 *
 * Demonstrates various hopping modes for PFILT and CFIR. Also contains example code for resetting timestamp counters.
 * 
 * This examples uses device profile id00_uc06 (20G device clock, 10.3125Gbps JESD, 1250 data rate)
 * For 8t8r, use id98_uc05 (8G device clock, 10.0Gbps JESD204B, 125MHz data rate)
 *  
 * Device Connections
 *      ADC-A0  1162.5MHz tone from sig gen, +3dBm / -3dBm 8t8r
 *      ADC-B0  1162.5MHz tone from sig gen, +3dBm 4t4r / -3dBm 8t8r
 *      
 *      DAC-A0  Spectrum Analyzer
 *      DAC-B0  Spectrum Analyzer
 *  
 * General operating mode
 *       - Load filter coefficients with gain only responses
 *          - PFILT: unity, -6.0dB, -12dB, -18dB
 *          - CFIR: unity, -6.0dB (additional -18dB from gain block for second profile only)
 *      - Enable filter block
 *          - PFILT Real N/2 mode
 *          - CFIR Straight complex (non-sparse)
 *      - Set Rx CNCO = 1000, Rx FNCO = 100
 *      - Set Tx CNCO = 1000, Tx FNCO = 100
 *      - Create and play a vector at 0.05 * data rate
 *      - Set Tx NCOs so vector is at 1162.5MHz
 *      - Set Rx NCOs so 1162.5MHz input is at 0.05 * data rate 
 *      - Device runs in fullchip mode (simultaneous Rx/Tx)
 *      - For each of the hopping modes, cycle through all hops (4 for PFIR, 2 for CFIR)
 *          - Each hop will activate the corresponding filter
 *          - A 6db change in the fundamental power (at 1162.5MHz) will be seen at each hop
 *          - This verifies a hop executed
 *          - Observe specified tone levels on PXA and FFT of capture data
 *      - Set FNCOs to 0
 *      - For each CNCO hopping mode, cycle through all hops (16)
 *          - CNCO profiles will be set equally spaced around 1162.5MHz
 *          - Auto-select function demonstrates muting trigger master after n hops
 *      - Set CNCOs to base value
 *      - For each FNCO hopping mode, cycle through hops (32)
 *          - FNCO profiles will be set equally spaced around 1162.5MHz
 *          - Auto-select function demonstrates muting trigger master after n hops
 *      
 *      
 * PFILT hopping modes
 *          pfilt_direct_regmap
 *          pfilt_direct_gpio
 *          pfilt_trig_by_spi_sel_by_regmap
 *          pfilt_trig_by_spi_sel_by_gpio
 *          pfilt_trig_by_tmaster_sel_by_regmap
 *          pfilt_trig_by_tmaster_sel_by_gpio
 *
 * CFIR hopping modes
 *          cfir_direct_regmap
 *          cfir_direct_gpio
 *          cfir_trig_by_spi_sel_by_regmap
 *          cfir_trig_by_spi_sel_by_gpio
 *          cfir_trig_by_tmaster_sel_by_regmap
 *          cfir_trig_by_tmaster_sel_by_gpio
 * 
 * CNCO hopping modes
 *          cnco_direct_regmap
 *          cnco_direct_gpio
 *          cnco_trig_by_spi_sel_by_regmap
 *          cnco_trig_by_spi_sel_by_gpio
 *          cnco_trig_by_tmaster_sel_by_auto
 *              auto-increment
 *              auto-decrement
 *              auto-flip
 *
 * FNCO hopping modes
 *          fnco_direct_regmap
 *          fnco_direct_gpio
 *          fnco_trig_by_spi_sel_by_regmap
 *          fnco_trig_by_spi_sel_by_gpio
 *          fnco_trig_by_tmaster_sel_by_auto
 *              auto-increment
 *              auto-decrement
 *              auto-flip
 * 
 * The timestamp reset example demonstrates how to reset the timestamp counter by SPI or SYSREF based.
 *
 * General Operating mode:
 *  - Configure timestamp reset mode (SPI or SYSREF)
 *  - Read arbitrary timestamp counter values
 *  - Reset the timestamp 
 *  - Readback the counters and note T0 time is consistent among all timestamps
 *  - Delay the reading of each timestamp by 100ms and note delays T1-T4
 *  
 * Tests
 *      trigts_timestamp_reset_via_spi_test
 *      trigts_timestamp_reset_via_syref_test
 *      
 * Example output
 * 
 *            TS  Pre-Reset Cnt     Post-Reset T0         T1         T2         T3         T4
 *           ---- ----------------- ---------- ---------- ---------- ---------- ---------- ----------
 *           TX-A     100962024063   21872288   0.001094   0.101624   0.101610   0.101639   0.101614 
 *           TX-B      90778124223   21804320   0.001090   0.201622   0.201616   0.201647   0.201619 
 *           RX-A      70596013631   21876288   0.001094   0.301622   0.301661   0.301625   0.301621 
 *           RX-B      40413443327   22144352   0.001107   0.401634   0.401625   0.401690   0.401629
 *           
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
#include "adi_apollo_gpio.h"
#include "adi_apollo_gpio_hop.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_ctl.h"

#define CHANNEL_SEL 1

adi_apollo_gpio_hop_profile_t ADI_APOLLO_GPIO_HOP_PROFILE_CUSTOM = {{25, 26, 27, 28, 29}};
adi_apollo_gpio_hop_block_t ADI_APOLLO_GPIO_HOP_BLOCK_CUSTOM = {{21, 22, 23, 24}};

#ifdef ADS10
    #define QUICK_CONFIG ADI_APOLLO_QUICK_CFG_PROFILE_1
#endif /* ADS10 */
#ifdef VCU128
    #define QUICK_CONFIG ADI_APOLLO_QUICK_CFG_DISABLE
#endif /* VCU128 */

#define ADC_FREQ_MHZ 1162.5
#define TONE_RATIO 0.05

/*!
  * \brief Fullchip Hop execution container
  */
typedef struct
{
    adi_apollo_device_t *device;
    adi_fpga_apollo_device_t *fpga_device;
    adi_apollo_top_t *profile;
    bool is_interactive;
} fullchip_hop_exec_t;

typedef struct
{
    uint64_t tx_ftw;
    uint64_t rx_ftw;
    double nco_freq;
    double tone_freq;
} fullchip_hop_nco_calc_t;

static fullchip_hop_exec_t *fullchip_hop_exec_obj_create(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, bool interactive);
static int32_t fullchip_hop_exec_setup(const fullchip_hop_exec_t* obj);
static int32_t capture(const fullchip_hop_exec_t *exec, char* cap_fname_base, bool bdir_start);
static int32_t play(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, double tone_ratio, bool bdir_start);
static int32_t profile_gpio_sel(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint8_t hop_number);
static int32_t block_gpio_sel(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_gpio_hop_block_e block);
static int32_t print_timestamp_reset_done_status(const fullchip_hop_exec_t *obj);
static int32_t print_timestamp_counters(const fullchip_hop_exec_t *obj);
static int32_t calculate_cnco_hop_freq(const fullchip_hop_exec_t *obj, uint32_t hop, fullchip_hop_nco_calc_t * calc);
static int32_t calculate_fnco_hop_freq(const fullchip_hop_exec_t *obj, uint32_t hop, fullchip_hop_nco_calc_t * calc);
static int32_t set_ncos(const fullchip_hop_exec_t *obj);
static void print_timestamp_reset_results(const fullchip_hop_exec_t *obj, uint8_t trigts[4][2], uint64_t initial_ts[4], uint64_t curr_ts[4][5]);
static int32_t gpio_hop_custom_configure(adi_apollo_device_t* device, adi_apollo_gpio_hop_profile_t *hop_config, adi_apollo_gpio_hop_block_t *block_config);

/***       ****/
/*** PFILT ***/
/***       ***/
static int32_t fullchip_hop_exec_pfilt_init(const fullchip_hop_exec_t* obj);
static int32_t fullchip_hop_exec_pfilt_cfg(const fullchip_hop_exec_t* obj);
static __maybe_unused int32_t pfilt_direct_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t pfilt_direct_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t pfilt_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t pfilt_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t pfilt_trig_by_tmaster_sel_by_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t pfilt_trig_by_tmaster_sel_by_gpio(const fullchip_hop_exec_t *obj);

/***       ****/
/*** CFIR  ***/
/***       ***/
static int32_t fullchip_hop_exec_cfir_init(const fullchip_hop_exec_t *obj);
static int32_t fullchip_hop_exec_cfir_cfg(const fullchip_hop_exec_t* obj);
static __maybe_unused int32_t cfir_direct_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cfir_direct_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cfir_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cfir_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cfir_trig_by_tmaster_sel_by_regmap(const fullchip_hop_exec_t *obj);

/***       ****/
/*** CNCO ***/
/***       ***/
static int32_t fullchip_hop_exec_cnco_cfg(const fullchip_hop_exec_t* obj);
static __maybe_unused int32_t cnco_direct_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cnco_direct_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cnco_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cnco_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t cnco_trig_by_tmaster_sel_by_auto(const fullchip_hop_exec_t *obj);

/***       ****/
/*** FNCO ***/
/***       ***/
static int32_t fullchip_hop_exec_fnco_cfg(const fullchip_hop_exec_t* obj);
static __maybe_unused int32_t fnco_direct_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t fnco_direct_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t fnco_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t fnco_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t fnco_trig_by_tmaster_sel_by_auto(const fullchip_hop_exec_t *obj);

/***                 ****/
/*** Timestamp Reset ***/
/***                 ***/
static __maybe_unused int32_t trigts_timestamp_reset_via_spi_test(const fullchip_hop_exec_t *obj);
static __maybe_unused int32_t trigts_timestamp_reset_via_sysref_test(const fullchip_hop_exec_t *obj);

int32_t fullchip_hop(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool run_timestamp = true;
    bool run_pfilt = true;
    bool run_cfir = true;
    bool run_cnco = true;
    bool run_fnco = true;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive

    printf("fullchip_hop\n");

    if (device->dev_info.is_8t8r) {
        /*
        * Enable the fduc gain block for all FDUCs.
        * Set each gain to 0.5. (note gain value of 1.0 is 0x800, 0.5 is 0x400)
        * Must do DP reset after adi_apollo_fduc_subdp_gain_enable() (not necessary for subsequent calls to adi_apollo_fduc_subdp_gain_set())
        * 0.5 is a -6dB change
        *
        * If this FDUC attenuation is not set, then summing FDUCs together results in clipping/distortion. FFT will look really bad!)
        */ 
        adi_apollo_fduc_subdp_gain_enable(device, ADI_APOLLO_FDUC_ALL, 1);
        adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x800>>1); // 0.5 gain reduction if summing two fducs
        err |= adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 0);
        err |= adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 1);
        err |= adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 0);
        ADI_CMS_ERROR_RETURN(err);
    }


    EXCTL_SIGGEN_FREQ(ADC_FREQ_MHZ, false);
    EXCTL_SIGGEN_ON(CHANNEL_SEL, interactive);

    fullchip_hop_exec_t *obj = fullchip_hop_exec_obj_create(device, fpga_device, profile, interactive);

    //Enable necessary blocks
    if (run_pfilt) {
        err = fullchip_hop_exec_pfilt_init(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }
    if (run_cfir) {
        err = fullchip_hop_exec_cfir_init(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    err = fullchip_hop_exec_setup(obj);
    ADI_CMS_ERROR_GOTO(err, end);

    if (run_pfilt) {
        err = fullchip_hop_exec_pfilt_cfg(obj);
        ADI_CMS_ERROR_GOTO(err, end);
        err |= pfilt_direct_regmap(obj);
        err |= pfilt_direct_gpio(obj);
        err |= pfilt_trig_by_spi_sel_by_regmap(obj);
        err |= pfilt_trig_by_spi_sel_by_gpio(obj);
        err |= pfilt_trig_by_tmaster_sel_by_regmap(obj);
        err |= pfilt_trig_by_tmaster_sel_by_gpio(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    if (run_cfir) {
        err = fullchip_hop_exec_cfir_cfg(obj);
        ADI_CMS_ERROR_GOTO(err, end);
        err |= cfir_direct_regmap(obj);
        err |= cfir_direct_gpio(obj);
        err |= cfir_trig_by_spi_sel_by_regmap(obj);
        err |= cfir_trig_by_spi_sel_by_gpio(obj);
        err |= cfir_trig_by_tmaster_sel_by_regmap(obj);
        //err |= cfir_trig_by_tmaster_sel_by_gpio(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    if (run_cnco) {
        err = fullchip_hop_exec_cnco_cfg(obj);
        ADI_CMS_ERROR_GOTO(err, end);
        err |= cnco_direct_regmap(obj);
        err |= cnco_direct_gpio(obj);
        err |= cnco_trig_by_spi_sel_by_regmap(obj);
        err |= cnco_trig_by_spi_sel_by_gpio(obj);
        err |= cnco_trig_by_tmaster_sel_by_auto(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    if (run_fnco) {
        err = fullchip_hop_exec_fnco_cfg(obj);
        ADI_CMS_ERROR_GOTO(err, end);
        err |= fnco_direct_regmap(obj);
        err |= fnco_direct_gpio(obj);
        err |= fnco_trig_by_spi_sel_by_regmap(obj);
        err |= fnco_trig_by_spi_sel_by_gpio(obj);
        err |= fnco_trig_by_tmaster_sel_by_auto(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    if (run_timestamp) {
        err = trigts_timestamp_reset_via_spi_test(obj);
        err |= trigts_timestamp_reset_via_sysref_test(obj);
        ADI_CMS_ERROR_GOTO(err, end);
    }

end:
    ADI_CMS_MEM_ALLOC_FREE(obj);
    return err;
}

/* 
 * Creates a container object for commonly used members 
 * 
 **/
fullchip_hop_exec_t* fullchip_hop_exec_obj_create(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, bool interactive)
{
    fullchip_hop_exec_t* obj = (fullchip_hop_exec_t *) malloc(sizeof(fullchip_hop_exec_t));

    obj->device = device;
    obj->fpga_device = fpga_device;
    obj->profile = profile;

    obj->is_interactive = interactive;

    return obj;
}

static int32_t fullchip_hop_exec_setup(const fullchip_hop_exec_t* obj)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_ads10_apollo_dp_info_t tx_dp_info, rx_dp_info;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    
    err = adi_ads10_apollo_ex_cals_run(obj->device, obj->profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    /* Get Tx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Rx Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Base Tx Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);
 
    /* Override Device Profile - Set RX FNCO/CNCO to match TX - Tone will land at baseband freq in FFT when ADC-DAC are loop backed */
    set_ncos(obj);
 
    /* Select the GPIO quick config 1. Assigns GPIO pin functions. Refer to `GPIO Quick Config Profiles' in user guide. */
    adi_apollo_gpio_quick_config_mode_set(obj->device, QUICK_CONFIG);
 
    if (!QUICK_CONFIG) {
        gpio_hop_custom_configure(obj->device, &ADI_APOLLO_GPIO_HOP_PROFILE_CUSTOM, &ADI_APOLLO_GPIO_HOP_BLOCK_CUSTOM);
    } 

    // Dynamic Sync Serdes Links gradually in a sequence
    err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(obj->device);
    ADI_CMS_ERROR_RETURN(err);

    err = play(obj->device, obj->fpga_device, obj->profile, TONE_RATIO, true);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(obj->device, obj->profile, ADI_ADS10_APOLLO_CAL_ADC_BG | ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

/***       ****/
/*** PFILT ***/
/***       ***/
static int32_t fullchip_hop_exec_pfilt_init(const fullchip_hop_exec_t* obj)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_apollo_terminal_e terminals[] = { ADI_APOLLO_RX, ADI_APOLLO_TX };
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;

    printf("@%s()\n", __func__);

    /* Load the filter gain and delay settings into the 4 banks */
    adi_apollo_pfilt_gain_dly_pgm_t filt_gain_dly_config = {
        ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, // Gain for A0, A2, A1, A3
        63, 63, 63, 63,     // x/64 6-bit gain multiplier (32 = ~6dB)
        0
    };

    /* Load the PFILT coefficients from file into the 4 banks */
    for (int i = 0; i < sizeof(terminals)/sizeof(terminals[0]); i++) {
        err =  adi_ads10_ex_pfilt_coeff_file_load(obj->device, terminals[i], active_pfilts,
                    ADI_APOLLO_PFILT_BANK0, "./filters/pfilt_coeffs_16_unity.txt");
        err |= adi_ads10_ex_pfilt_coeff_file_load(obj->device, terminals[i], active_pfilts,
                    ADI_APOLLO_PFILT_BANK1, "./filters/pfilt_coeffs_16_neg6db.txt");
        err |= adi_ads10_ex_pfilt_coeff_file_load(obj->device, terminals[i], active_pfilts,
                    ADI_APOLLO_PFILT_BANK2, "./filters/pfilt_coeffs_16_neg12db.txt");
        err |= adi_ads10_ex_pfilt_coeff_file_load(obj->device, terminals[i], active_pfilts,
                    ADI_APOLLO_PFILT_BANK3, "./filters/pfilt_coeffs_16_neg18db.txt");
        err |= adi_apollo_pfilt_gain_dly_pgm(obj->device, terminals[i], active_pfilts, ADI_APOLLO_PFILT_BANK_ALL, &filt_gain_dly_config);

        ADI_CMS_ERROR_RETURN(err);
    }

    /* Default to filter to BANK0 */
    err |= adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_BANK0);
    err |= adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_BANK0);
    ADI_CMS_ERROR_RETURN(err);

    /* Use N/2 tap real mode. 16-tap filters */
    err |= adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL);
    err |= adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t fullchip_hop_exec_pfilt_cfg(const fullchip_hop_exec_t* obj)
{
    int32_t err = API_CMS_ERROR_OK;

    printf("@%s()\n", __func__);

    /*
    * Configure the GPIO pinmux for PFILT profile hop selection. Only the profile GPIOs
    * need to be driven (from FPGA) to select the profile hop. Other pins are set via registers.
    * 
    * The 'fcn' bits are set for PFILT.
    * The PFILT 'slice selects' are set to enable all slices.
    *
    * Alternatively, these GPIOs can be programmed entirely through device GPIO pins.
    */
    adi_apollo_gpio_hop_block_select_set(obj->device, ADI_APOLLO_GPIO_HOP_SELECT_GPIO);
    block_gpio_sel(obj->device, obj->fpga_device, ADI_APOLLO_GPIO_BLOCK_PFILT);

    adi_apollo_gpio_hop_slice_select_set(obj->device, ADI_APOLLO_GPIO_BLOCK_PFILT, ADI_APOLLO_GPIO_HOP_SELECT_SPI);
    adi_apollo_gpio_hop_pfilt_enable_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_gpio_hop_pfilt_enable_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);
    
    /* Real data is input to the PFILT */
    #include "adi_apollo_bf_txrx_pfilt_top.h"
    adi_apollo_hal_bf_set(obj->device, BF_REAL_DATA_INFO(TX_PFILT_TOP_TX_SLICE_0_TX_DIGITAL0), 1);       // Real Data
    adi_apollo_hal_bf_set(obj->device, BF_REAL_DATA_INFO(TX_PFILT_TOP_TX_SLICE_0_TX_DIGITAL1), 1);       // Real Data
    adi_apollo_hal_bf_set(obj->device, BF_REAL_DATA_INFO(RX_PFILT_TOP_RX_SLICE_0_RX_DIGITAL0), 1);       // Real Data
    adi_apollo_hal_bf_set(obj->device, BF_REAL_DATA_INFO(RX_PFILT_TOP_RX_SLICE_0_RX_DIGITAL1), 1);       // Real Data

    return err;
}

static int32_t pfilt_direct_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;

    // For Direct SPI:
    // - Set the 'pfilt_trigger_en' to 0. Disable trigger for coefficient switching.
    // - Set 'eq_gpio_en' to 0. Use 'rd_coeff_page_sel' to select one of the 4 banks.       {call adi_apollo_pfilt_profile_sel_mode_set()}
    // - Set 'rd_coeff_page_sel' to [0->3] for bank0 to bank3.                              {call adi_apollo_pfilt_profile_sel_mode_set()}
    //
    printf("@%s()\n", __func__);

    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_DIRECT_REGMAP);
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_DIRECT_REGMAP);

    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_BANK0);
    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_BANK0);
    capture(obj, "fullchip_hop_pfilt_dir_regmap_h00", false);

    EXCTL_RX_REF(CHANNEL_SEL, "fullchip_hop_pfilt_dir_regmap_h00");
    EXCTL_TX_REF(CHANNEL_SEL, ADC_FREQ_MHZ, false);
    EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, ADC_FREQ_MHZ, 10, obj->is_interactive);

    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_BANK1);
    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_BANK1);
    capture(obj, "fullchip_hop_pfilt_dir_regmap_h01", false);
    
    EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, "fullchip_hop_pfilt_dir_regmap_h01", -6, 1);
    EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
    EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, -6, 1, obj->is_interactive);

    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_BANK2);
    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_BANK2);
    capture(obj, "fullchip_hop_pfilt_dir_regmap_h02", false);
    
    EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, "fullchip_hop_pfilt_dir_regmap_h02", -12, 1);
    EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
    EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, -12, 1, obj->is_interactive);

    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_BANK3);
    adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_BANK3);
    capture(obj, "fullchip_hop_pfilt_dir_regmap_h03", false);
    
    EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, "fullchip_hop_pfilt_dir_regmap_h03", -18, 1);
    EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
    EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, -18, 1, obj->is_interactive);

    return err;
}

static int32_t pfilt_direct_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    char cap_fname_base[256];

    // For Direct GPIO
    // - Set the pin mux quick config mode to profile 1
    // - Set the 'pfilt_trigger_en' to 0. Disable trigger for coefficient switching. Use regmap or GPIO profile pins.
    // - Set 'eq_gpio_en' to 1. Use the GPIO pins to select between the four sets of coeffs. {call adi_apollo_pfilt_setup_gpio_transfer()}
    // -- 'gpo_config1'
    // ---- Set to 0 (Configuration 0): to use i_eq_gpio[1:0] for selecting 1 of the 4 coeffs
    // ---- Set to 1 (Configuration 1):
    // ------ 4T4R & N/2 mode GPIO[0] for coeff sets 0/1 for stream 0. GPIO[1] for coeff sets 2/3 for stream 1. Other modes GPIO[0] for coeff sets 0/1.
    // ------ 8T8R GPIO[0] Switch between coefficient sets 0 and 1 for Filter 0. GPIO[1] coefficient sets 2 and 3 for Filter 1.

    printf("@%s()\n", __func__);

    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_DIRECT_GPIO);
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_DIRECT_GPIO);

    uint8_t profile[] = {0, 1, 2, 3, 3, 2, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {
        profile_gpio_sel(obj->device, obj->fpga_device, profile[i]); // 2-bits for PFILT profile select (0-3), start at GPIO #26(lsb) based on quick config 1.

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);

        sprintf(cap_fname_base, "fullchip_hop_pfilt_dir_gpio_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] * 6.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] * 6.0), 1, obj->is_interactive);
    }

    return err;
}

static int32_t pfilt_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    char cap_fname_base[256];
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for PFILT). Will hop to selected profile when triggered.
    */
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_REGMAP);
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_REGMAP);

    uint8_t profile[] = {0, 1, 2, 3, 3, 2, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {

        adi_apollo_pfilt_next_hop_num_set(obj->device, ADI_APOLLO_TX, active_pfilts, profile[i]); /* Select the profile to hop to (via regmap) on trigger event */
        adi_apollo_pfilt_next_hop_num_set(obj->device, ADI_APOLLO_RX, active_pfilts, profile[i]); /* Select the profile to hop to (via regmap) on trigger event */

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);

        sprintf(cap_fname_base, "fullchip_hop_pfilt_trig_by_spi_sel_by_regmap_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] * 6.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] * 6.0), 1, obj->is_interactive);
        
    }

    return err;
}

static int32_t pfilt_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    char cap_fname_base[256];

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for PFILT). Will hop to selected profile when triggered.
    */
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_GPIO);
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_GPIO);

    uint8_t profile[] = {0, 1, 2, 3, 3, 2, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {

        profile_gpio_sel(obj->device, obj->fpga_device, profile[i]); // 2-bits for PFILT profile select (0-3), start at GPIO #26(lsb) based on quick config 1.

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);

        sprintf(cap_fname_base, "fullchip_hop_pfilt_trig_by_spi_sel_by_gpio_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] * 6.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] * 6.0), 1, obj->is_interactive);
    }

    return err;    
}

static int32_t pfilt_trig_by_tmaster_sel_by_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    char cap_fname_base[256];
    uint64_t curr_timestamp_count;
    uint64_t prev_trig_cnt = 0;
    uint64_t curr_trig_cnt = 0;
    uint64_t trig_per = obj->device->dev_info.dev_freq_hz * 3;  // Gen trigger every 3s
    adi_apollo_trig_mst_config_t trig_mst_config;
    
    /*
    * Trigger mode mux for PFILT: SPI or Trig Master (timestamp) or External (dynamic reconfig)
    */
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_TRIG_MASTER);
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_TRIG_MASTER);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for PFILT). Will hop to selected profile when triggered.
    */
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_REGMAP);
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_REGMAP);

    /*
     * Reset the timestamp counter
     */
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);

    trig_mst_config.trig_period = trig_per;                 // Trig Period unit is Fs
    trig_mst_config.trig_offset = 32;                       // Min for 4t4r per userguide
    trig_mst_config.trig_enable = ADI_APOLLO_TRIG_ENABLE;   //
    adi_apollo_trigts_pfilt_trig_mst_config(obj->device, ADI_APOLLO_TX, active_pfilts, &trig_mst_config);
    adi_apollo_trigts_pfilt_trig_mst_config(obj->device, ADI_APOLLO_RX, active_pfilts, &trig_mst_config);

    /* Set initial hop value applied at timestamp reset */
    adi_apollo_pfilt_next_hop_num_set(obj->device, ADI_APOLLO_TX, active_pfilts, 0);
    adi_apollo_pfilt_next_hop_num_set(obj->device, ADI_APOLLO_RX, active_pfilts, 0);
    
    /* Timestamp reset - will apply next hop set previously */
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);

    print_timestamp_reset_done_status(obj);
    print_timestamp_counters(obj);

    uint8_t profile[] = {0, 1, 2, 3, 2, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {

        adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count);    // Using Rx/B since last ts reset async (sloweset)
        prev_trig_cnt = curr_timestamp_count / trig_per;

        adi_apollo_pfilt_next_hop_num_set(obj->device, ADI_APOLLO_TX, active_pfilts, profile[i]);
        adi_apollo_pfilt_next_hop_num_set(obj->device, ADI_APOLLO_RX, active_pfilts, profile[i]);
        
        /* Wait for trigger - new profile hop will be applied upon trigger event */
        for (;;) {
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
            curr_trig_cnt = curr_timestamp_count / trig_per;

            if (curr_trig_cnt > prev_trig_cnt) {
                break;
            }

            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
        }

        printf("Trigger: %llu    Hop: %d    Time: %lf\n", curr_trig_cnt, profile[i], curr_timestamp_count / (double)obj->device->dev_info.dev_freq_hz);

        sprintf(cap_fname_base, "fullchip_hop_pfilt_trig_by_tmaster_sel_by_regmap_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] * 6.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] * 6.0), 1, obj->is_interactive);
    }

    return err;
}

static int32_t pfilt_trig_by_tmaster_sel_by_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    char cap_fname_base[256];
    uint64_t curr_timestamp_count;
    uint64_t prev_trig_cnt = 0;
    uint64_t curr_trig_cnt = 0;
    uint64_t trig_per = obj->device->dev_info.dev_freq_hz * 3; // Gen trigger every 3s
    adi_apollo_trig_mst_config_t trig_mst_config;

    /*
    * Trigger mode mux for PFILT: SPI or Trig Master (timestamp) or External (dynamic reconfig)
    */
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_TRIG_MASTER);
    adi_apollo_trigts_pfilt_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_TRIG_MASTER);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for PFILT). Will hop to selected profile when triggered.
    */
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_GPIO);
    adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_CHAN_SEL_TRIG_GPIO);

    /*
     * Reset the timestamp counter
     */
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);

    trig_mst_config.trig_period = trig_per;               // Trig Period unit is Fs
    trig_mst_config.trig_offset = 32;                     // Min for 4t4r per userguide
    trig_mst_config.trig_enable = ADI_APOLLO_TRIG_ENABLE; //
    adi_apollo_trigts_pfilt_trig_mst_config(obj->device, ADI_APOLLO_TX, active_pfilts, &trig_mst_config);
    adi_apollo_trigts_pfilt_trig_mst_config(obj->device, ADI_APOLLO_RX, active_pfilts, &trig_mst_config);

    /* Set initial hop value applied at timestamp reset */
    profile_gpio_sel(obj->device, obj->fpga_device, 2);

    /* Timestamp reset - will apply next hop set previously */
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);

    print_timestamp_reset_done_status(obj);
    print_timestamp_counters(obj);

    uint8_t profile[] = {0, 1, 2, 3, 2, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {

        adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); // Using Rx/B since last ts reset async (sloweset)
        prev_trig_cnt = curr_timestamp_count / trig_per;

        /* GPIOs 26 & 27, val=profile. Selects the chan to hop (via GPIO) to on trigger event */
        profile_gpio_sel(obj->device, obj->fpga_device, profile[i]);

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        for (;;) {
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
            curr_trig_cnt = curr_timestamp_count / trig_per;

            if (curr_trig_cnt > prev_trig_cnt) {
                break;
            }

            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
        }

        printf("Trigger: %llu    Hop: %d    Time: %lf\n", curr_trig_cnt, profile[i], curr_timestamp_count / (double)obj->device->dev_info.dev_freq_hz);

        sprintf(cap_fname_base, "fullchip_hop_pfilt_trig_by_tmaster_sel_by_gpio_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] * 6.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] * 6.0), 1, obj->is_interactive);
    }

    return err;
}

/***       ****/
/*** CFIR ***/
/***       ***/
fullchip_hop_exec_t *fullchip_hop_exec_cfir_create(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile)
{
    fullchip_hop_exec_t *obj = (fullchip_hop_exec_t *)malloc(sizeof(fullchip_hop_exec_t));

    obj->device = device;
    obj->fpga_device = fpga_device;
    obj->profile = profile;

    return obj;
}

static int32_t fullchip_hop_exec_cfir_init(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_apollo_cfir_pgm_t blk_mode_config;
    adi_apollo_terminal_e terminals[] = {ADI_APOLLO_RX, ADI_APOLLO_TX};

    /* Enable CFIR (i.e. don't bypass) */
    blk_mode_config.cfir_bypass = 0;
    blk_mode_config.cfir_sparse_filt_en = 0;
    blk_mode_config.cfir_32taps_en = 0;
    
    uint16_t coeffs_i[ADI_APOLLO_CFIR_NUM_PROFILES][ADI_APOLLO_CFIR_COEFF_SETS][ADI_APOLLO_CFIR_NUM_TAPS] =
        {
            {{32767 / 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Profile 0 (I) (unity)
             {32767 / 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {32767 / 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {32767 / 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
            {{32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Profile 1 (I) (-6dB)
             {32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}};

    uint16_t coeffs_q[ADI_APOLLO_CFIR_NUM_PROFILES][ADI_APOLLO_CFIR_COEFF_SETS][ADI_APOLLO_CFIR_NUM_TAPS] =
        {
            {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          // Profile 0 (Q)
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},         // Profile 1 (Q)
            {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}};

    adi_apollo_cfir_gain_e cfir_gain_dB[ADI_APOLLO_CFIR_NUM_PROFILES][ADI_APOLLO_CFIR_COEFF_SETS] =
        {
            {ADI_APOLLO_CFIR_GAIN_ZERO_DB,    ADI_APOLLO_CFIR_GAIN_ZERO_DB,    ADI_APOLLO_CFIR_GAIN_ZERO_DB,    ADI_APOLLO_CFIR_GAIN_ZERO_DB},      // Profile 0 (-0.0 dB)
            {ADI_APOLLO_CFIR_GAIN_MINUS18_DB, ADI_APOLLO_CFIR_GAIN_MINUS18_DB, ADI_APOLLO_CFIR_GAIN_MINUS18_DB, ADI_APOLLO_CFIR_GAIN_MINUS18_DB}};  // Profile 1 (-18.0 dB)

    uint16_t scalar_i[ADI_APOLLO_CFIR_NUM_PROFILES][ADI_APOLLO_CFIR_COEFF_SETS] = {
        {32767, 32767, 32767, 32767}, {32767, 32767, 32767, 32767}};

    uint16_t scalar_q[ADI_APOLLO_CFIR_NUM_PROFILES][ADI_APOLLO_CFIR_COEFF_SETS] = {
        {0, 0, 0, 0}, {0, 0, 0, 0}};

    printf("@%s()\n", __func__);

    /* 
     * Load the CFIR coefficients into the 2 banks, init the CFIR blocks
     *
     * Note: This can all be configured in a device profile file. Here we are being explicit for visibility.
     * 
     **/
    for (int i = 0; i < sizeof(terminals) / sizeof(terminals[0]); i++) {
        for (int j = 0; j < ADI_APOLLO_CFIR_NUM_PROFILES; j++) {
            for (int k = 0; k < ADI_APOLLO_CFIR_COEFF_SETS; k++) {
                err |= adi_apollo_cfir_coeff_pgm(obj->device, terminals[i], ADI_APOLLO_CFIR_ALL,
                                                 ADI_APOLLO_CFIR_PROFILE_0 << j,
                                                 ADI_APOLLO_CFIR_DP_0 << k,
                                                 coeffs_i[j][k], coeffs_q[j][k], ADI_APOLLO_CFIR_NUM_TAPS);

                err |= adi_apollo_cfir_scalar_pgm(obj->device, terminals[i], ADI_APOLLO_CFIR_ALL,
                                                  ADI_APOLLO_CFIR_PROFILE_0 << j,
                                                  ADI_APOLLO_CFIR_DP_0 << k,
                                                  scalar_i[j][k], scalar_q[j][k]);

                err |= adi_apollo_cfir_gain_pgm(obj->device, terminals[i], ADI_APOLLO_CFIR_ALL,
                                                ADI_APOLLO_CFIR_PROFILE_0 << j,
                                                ADI_APOLLO_CFIR_DP_0 << k,
                                                cfir_gain_dB[j][k]);
            }
        }

        err |= adi_apollo_cfir_pgm(obj->device, terminals[i], ADI_APOLLO_CFIR_ALL, &blk_mode_config);
    }
    err |= adi_apollo_cfir_profile_sel(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_0);
    err |= adi_apollo_cfir_profile_sel(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_0);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}
int32_t fullchip_hop_exec_cfir_cfg(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t active_pfilts = (obj->device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;

    /* Disable the PFILT - hop CFIR only */
    adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, active_pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);
    adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, active_pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);

    /*
     * Configure the GPIO pinmux for CFIR profile hop selection. Only the profile GPIOs
     * need to be driven (from FPGA) to select the profile hop. Other pins are set via registers.
     *
     * The 'fcn' bits are set for CFIR.
     * The CFIR 'slice selects' are set to enable all slices.
     *
     * Alternatively, these GPIOs can be programmed entirely through device GPIO pins.
    */
    adi_apollo_gpio_hop_block_select_set(obj->device, ADI_APOLLO_GPIO_HOP_SELECT_GPIO);
    block_gpio_sel(obj->device, obj->fpga_device, ADI_APOLLO_GPIO_BLOCK_CFIR);

    adi_apollo_gpio_hop_slice_select_set(obj->device, ADI_APOLLO_GPIO_BLOCK_CFIR, ADI_APOLLO_GPIO_HOP_SELECT_SPI);
    adi_apollo_gpio_hop_cfir_enable_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, 1);
    adi_apollo_gpio_hop_cfir_enable_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, 1);
    
    profile_gpio_sel(obj->device, obj->fpga_device, 0);   // Init select GPIO (when CFIR GPIO is enabled, will go to this set initially)
    
    return err;
}

static int32_t cfir_direct_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;

    printf("@%s()\n", __func__);

    /* Select Direct Regmap CFIR hopping */
    err |= adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_DIRECT_REGMAP);
    err |= adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_DIRECT_REGMAP);

    /* Select Profile 0 (hop 0) via direct regmap */
    err |= adi_apollo_cfir_profile_sel(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_0);
    err |= adi_apollo_cfir_profile_sel(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_0);
    capture(obj, "fullchip_hop_cfir_direct_regmap_h00", false);

    EXCTL_RX_REF(CHANNEL_SEL, "fullchip_hop_cfir_direct_regmap_h00");
    EXCTL_TX_REF(CHANNEL_SEL, ADC_FREQ_MHZ, false);
    EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, obj->is_interactive);

    /* Select Profile 1 (hop 1) via direct regmap */
    err |= adi_apollo_cfir_profile_sel(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_1);
    err |= adi_apollo_cfir_profile_sel(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_0 | ADI_APOLLO_CFIR_DP_1, ADI_APOLLO_CFIR_PROFILE_1);
    capture(obj, "fullchip_hop_cfir_direct_regmap_h01", false);

    EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, "fullchip_hop_cfir_direct_regmap_h01", -24, 1);
    EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
    EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - 24, 1, obj->is_interactive);

    return err;
}

static int32_t cfir_direct_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    char cap_fname_base[256];

    printf("@%s()\n", __func__);

    /* Select Direct Regmap CFIR hopping */
    err |= adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_DIRECT_GPIO);
    err |= adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_DIRECT_GPIO);

    uint8_t profile[] = {0, 1, 0, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {
        profile_gpio_sel(obj->device, obj->fpga_device, profile[i]);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);

        sprintf(cap_fname_base, "fullchip_hop_cfir_direct_gpio_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1, obj->is_interactive);
    }

    return err;
}

static int32_t cfir_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    char cap_fname_base[256];

    printf("@%s()\n", __func__);

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_cfir_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_cfir_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for CFIR). Will hop to selected profile when triggered.
    */
    adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_TRIG_REGMAP);
    adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_TRIG_REGMAP);

    uint8_t profile[] = {0, 1, 0, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {
        adi_apollo_cfir_next_hop_num_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, profile[i]); /* Select the profile to hop to (via regmap) on trigger event */
        adi_apollo_cfir_next_hop_num_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, profile[i]); /* Select the profile to hop to (via regmap) on trigger event */

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);

        sprintf(cap_fname_base, "fullchip_hop_cfir_trig_by_spi_sel_by_regmap_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1, obj->is_interactive);
    }

    return err;
}

static int32_t cfir_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    char cap_fname_base[256];

    printf("@%s()\n", __func__);

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_cfir_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_cfir_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for CFIR). Will hop to selected profile when triggered.
    */
    adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_TRIG_GPIO);
    adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_TRIG_GPIO);

    uint8_t profile[] = {0, 1, 0, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {
        profile_gpio_sel(obj->device, obj->fpga_device, profile[i]);

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);

        sprintf(cap_fname_base, "fullchip_hop_cfir_trig_by_spi_sel_by_gpio_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1, obj->is_interactive);
    }

    return err;
}

static int32_t cfir_trig_by_tmaster_sel_by_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    char cap_fname_base[256];
    uint64_t curr_timestamp_count;
    uint64_t prev_trig_cnt = 0;
    uint64_t curr_trig_cnt = 0;
    uint64_t trig_per = obj->device->dev_info.dev_freq_hz * 3; // Gen trigger every 3s
    adi_apollo_trig_mst_config_t trig_mst_config;
    
    printf("@%s()\n", __func__);

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_cfir_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_TRIG_MASTER);
    adi_apollo_trigts_cfir_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_TRIG_MASTER);

    /*
    *  Profile selection method (REGMAP or GPIO, no auto mode for CFIR). Will hop to selected profile when triggered.
    */
    adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_TRIG_REGMAP);
    adi_apollo_cfir_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_CHAN_SEL_TRIG_REGMAP);

    /*
     * Reset the timestamp counter
     */
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);

    trig_mst_config.trig_period = trig_per;               // Trig Period unit is Fs
    trig_mst_config.trig_offset = 32;                     // Min for 4t4r per userguide
    trig_mst_config.trig_enable = ADI_APOLLO_TRIG_ENABLE; //
    adi_apollo_trigts_cfir_trig_mst_config(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, &trig_mst_config);
    adi_apollo_trigts_cfir_trig_mst_config(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, &trig_mst_config);

    /* Set initial hop value applied at timestamp reset */
    adi_apollo_cfir_next_hop_num_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, 0);
    adi_apollo_cfir_next_hop_num_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, 0);

    /* Timestamp reset - will apply next hop set previously */
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 1);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 1);

    print_timestamp_reset_done_status(obj);
    print_timestamp_counters(obj);

    uint8_t profile[] = {0, 1, 0, 1, 0};
    for (int i = 0; i < sizeof(profile) / sizeof(profile[0]); i++) {

        adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); // Using Rx/B since last ts reset async (sloweset)
        prev_trig_cnt = curr_timestamp_count / trig_per;

        adi_apollo_cfir_next_hop_num_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, profile[i]);
        adi_apollo_cfir_next_hop_num_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CFIR_ALL, ADI_APOLLO_CFIR_DP_ALL, profile[i]);

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        for (;;) {
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
            curr_trig_cnt = curr_timestamp_count / trig_per;

            if (curr_trig_cnt > prev_trig_cnt) {
                break;
            }

            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
        }

        printf("Trigger: %llu    Hop: %d    Time: %lf\n", curr_trig_cnt, profile[i], curr_timestamp_count / (double)obj->device->dev_info.dev_freq_hz);

        sprintf(cap_fname_base, "fullchip_hop_cfir_trig_by_tmaster_sel_by_regmap_h%02d", profile[i]);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_LEVEL(CHANNEL_SEL, cap_fname_base, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL,ADC_FREQ_MHZ, 10, false);
        EXCTL_TX_MEAS_LEVEL(CHANNEL_SEL, ADC_FREQ_MHZ, - (profile[i] == 1 ? (6.0 + 18.0) : 0.0), 1, obj->is_interactive);
    }

    return err;
}

/***                 ***/
/*** TIMESTAMP Reset ***/
/***                 ***/

/*
 * Test the timestamp reset using the SPI mode
 * 
 * Flow:
 *  - Configure timestamp reset mode (SPI)
 *  - Read arbitrary timestamp counter values
 *  - Reset the timestamp 
 *  - Readback the counters and note T0 time is consistent among all timestamps
 *  - Delay the reading of each timestamp by 100ms and note delays T1-T4
 *  
 * Example output
 *         
 *            TS  Pre-Reset Cnt     Post-Reset T0         T1         T2         T3         T4
 *           ---- ----------------- ---------- ---------- ---------- ---------- ---------- ----------
 *           TX-A     100962024063   21872288   0.001094   0.101624   0.101610   0.101639   0.101614 
 *           TX-B      90778124223   21804320   0.001090   0.201622   0.201616   0.201647   0.201619 
 *           RX-A      70596013631   21876288   0.001094   0.301622   0.301661   0.301625   0.301621 
 *           RX-B      40413443327   22144352   0.001107   0.401634   0.401625   0.401690   0.401629  
 *
 **/
static int32_t trigts_timestamp_reset_via_spi_test(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t is_reset_done;
    uint64_t initial_ts[4];
    uint64_t curr_ts[4][5];
    uint8_t trigts[4][2] = {
        {ADI_APOLLO_TX, ADI_APOLLO_SIDE_A},
        {ADI_APOLLO_TX, ADI_APOLLO_SIDE_B},
        {ADI_APOLLO_RX, ADI_APOLLO_SIDE_A},
        {ADI_APOLLO_RX, ADI_APOLLO_SIDE_B}};

    printf("@%s()\n", __func__);

    /* Configure each timestamp for SPI reset mode */
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);

    for (int k = 0; k < 2; k++) {
     
        /* Read initial (random) ts counters for later reset comparison */
        for (int i = 0; i < 4; i++) {
            adi_apollo_trigts_counter_get(obj->device, trigts[i][0], trigts[i][1], &initial_ts[i]);
        }
        
        for (int i = 0; i < 4; i++) {
            adi_apollo_trigts_ts_reset(obj->device, trigts[i][0], trigts[i][1], 1);
            adi_apollo_trigts_reset_done_get(obj->device, trigts[i][0], trigts[i][1], &is_reset_done);
            printf("Reset Done: %d\n", is_reset_done);

            /*
             * Take 5 readings with a delta of (100ms * i) between.
             * This will result in TX-A=100ms, TX-B=200ms, RX-A=300ms, RX-B=400ms
             **/
            for (int j = 0; j < 5; j++) {
                adi_apollo_trigts_counter_get(obj->device, trigts[i][0], trigts[i][1], &curr_ts[i][j]);
                adi_apollo_hal_delay_us(obj->device, 100000 * (1 + i)); 
            }
        }
        
        /* Display time stamp results */
        printf("\n\nResults from trigts_timestamp_reset_via_spi_test\n");
        print_timestamp_reset_results(obj, trigts, initial_ts, curr_ts);
    }

    return err;
}

/*
 * Test the timestamp reset using the SYSREF mode
 * 
 * Flow:
 *  - Configure timestamp reset mode (SYSREF)
 *  - Read arbitrary timestamp counter values
 *  - Reset the timestamp 
 *  - Readback the counters and note T0 time is consistent among all timestamps
 *  - Delay the reading of each timestamp by 100ms and note delays T1-T4
 *  
 * Example output
 *         
 *            TS  Pre-Reset Cnt     Post-Reset T0         T1         T2         T3         T4
 *           ---- ----------------- ---------- ---------- ---------- ---------- ---------- ----------
 *           TX-A     100962024063   21872288   0.001094   0.101624   0.101610   0.101639   0.101614 
 *           TX-B      90778124223   21804320   0.001090   0.201622   0.201616   0.201647   0.201619 
 *           RX-A      70596013631   21876288   0.001094   0.301622   0.301661   0.301625   0.301621 
 *           RX-B      40413443327   22144352   0.001107   0.401634   0.401625   0.401690   0.401629  
 *
 **/
static int32_t trigts_timestamp_reset_via_sysref_test(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t is_reset_done;

    uint64_t initial_ts[4];
    uint64_t curr_ts[4][5];
    uint8_t trigts[4][2] = {
        {ADI_APOLLO_TX, ADI_APOLLO_SIDE_A},
        {ADI_APOLLO_TX, ADI_APOLLO_SIDE_B},
        {ADI_APOLLO_RX, ADI_APOLLO_SIDE_A},
        {ADI_APOLLO_RX, ADI_APOLLO_SIDE_B}};

    printf("@%s()\n", __func__);

    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SYSREF);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SYSREF);

    for (int k = 0; k < 2; k++) {
        /* Read initial (random) ts counters for later reset comparison */
        for (int i = 0; i < 4; i++) {
            adi_apollo_trigts_counter_get(obj->device, trigts[i][0], trigts[i][1], &initial_ts[i]);
        }
    
        for (int i = 0; i < 4; i++) {
            adi_apollo_trigts_ts_reset(obj->device, trigts[i][0], trigts[i][1], 1);  // reset timestamp instance one at a time, sync
            adi_apollo_trigts_reset_done_get(obj->device, trigts[i][0], trigts[i][1], &is_reset_done);
            printf("Reset Done: %d\n", is_reset_done);

            /*
             * Take 5 readings with a delta of (100ms * i) between.
             * This will result in TX-A=100ms, TX-B=200ms, RX-A=300ms, RX-B=400ms
             **/
            for (int j = 0; j < 5; j++) {
                adi_apollo_trigts_counter_get(obj->device, trigts[i][0], trigts[i][1], &curr_ts[i][j]);
                adi_apollo_hal_delay_us(obj->device, 100000 * (1 + i));
            }
        }

        /* Display time stamp results */
        printf("\n\nResults from trigts_timestamp_reset_via_sysref_test\n");
        print_timestamp_reset_results(obj, trigts, initial_ts, curr_ts);      
    }

    return err;
}

static int32_t capture(const fullchip_hop_exec_t *exec, char *cap_fname_base, bool bdir_start)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interleaved = true;
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;

    if (bdir_start) {
        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        err = adi_fpga_apollo_core_bidir_init(exec->fpga_device);
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(exec->device, exec->profile, exec->fpga_device, num_samples, cap_fname_base, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    printf("Done with capture. File base name: %s\n", cap_fname_base);

    return err;
}

static int32_t play(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, double tone_ratio, bool bdir_start)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl

    /*
     * Get the FPGA features supported (such as HW transport layer and FSRC)
     */
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, EX_VEC_DEFAULT_SAMPLES_PER_VC, tone_ratio, -1.0);
    ADI_CMS_ERROR_RETURN(err);

    if (bdir_start) {
        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        err = adi_fpga_apollo_core_bidir_init(fpga_device);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

static int32_t profile_gpio_sel(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint8_t hop_number) {
    int32_t err;
    uint64_t gpio, val;

    if (!QUICK_CONFIG) {
        err = adi_apollo_gpio_hop_profile_calc(device, &ADI_APOLLO_GPIO_HOP_PROFILE_CUSTOM, hop_number, &gpio, &val);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_apollo_gpio_hop_profile_qc_calc(device, QUICK_CONFIG, hop_number, &gpio, &val);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_fpga_apollo_gpio_output_word_set(fpga_device, gpio, val);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t block_gpio_sel(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_gpio_hop_block_e block) {
    int32_t err;
    uint64_t gpio, val;

    if (!QUICK_CONFIG) {
        err = adi_apollo_gpio_hop_block_calc(device, &ADI_APOLLO_GPIO_HOP_BLOCK_CUSTOM, block, &gpio, &val);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_apollo_gpio_hop_block_qc_calc(device, QUICK_CONFIG, block, &gpio, &val);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_fpga_apollo_gpio_output_word_set(fpga_device, gpio, val);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t  print_timestamp_reset_done_status(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;

    uint8_t is_reset_done[4];
    err |= adi_apollo_trigts_reset_done_get(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_A, &is_reset_done[0]);
    err |= adi_apollo_trigts_reset_done_get(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_B, &is_reset_done[1]);
    err |= adi_apollo_trigts_reset_done_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_A, &is_reset_done[2]);
    err |= adi_apollo_trigts_reset_done_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &is_reset_done[3]);

    if (err == API_CMS_ERROR_OK) {
        printf("Timestamp reset done status: %d %d %d %d\n", is_reset_done[0], is_reset_done[1], is_reset_done[2], is_reset_done[3]);
    } else {
        printf("Error from adi_apollo_trigts_reset_done_get()");
    }

    return err;
}

static int32_t print_timestamp_counters(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;

    uint64_t timestamp_count[4];
    err |= adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_A, &timestamp_count[0]);
    err |= adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_B, &timestamp_count[1]);
    err |= adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_A, &timestamp_count[2]);
    err |= adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &timestamp_count[3]);

    if (err == API_CMS_ERROR_OK) {
        printf("Timestamp counters: %lld %lld %lld %lld\n", timestamp_count[0], timestamp_count[1], timestamp_count[2], timestamp_count[3]);
    } else {
        printf("Error from adi_apollo_trigts_counter_get()");
    }

    return err;
}

/*
 * Print results from timestamp reset tests 
 **/
static void print_timestamp_reset_results(const fullchip_hop_exec_t *obj, uint8_t trigts[4][2], uint64_t initial_ts[4], uint64_t curr_ts[4][5])
{

    printf("\n");
    printf(" TS  Pre-Reset Cnt     Post-Reset T0         T1         T2         T3         T4\n");
    printf("---- ----------------- ---------- ---------- ---------- ---------- ---------- ----------\n");
    for (int i = 0; i < 4; i++) {
        printf("%s-%c ", trigts[i][0] == ADI_APOLLO_RX ? "RX" : "TX", 'A' + (trigts[i][1] >> 1));
        printf("%16llu %10llu   %lf ", initial_ts[i], curr_ts[i][0], curr_ts[i][0] / (double)obj->device->dev_info.dev_freq_hz);
        for (int j = 1; j < 5; j++) {
            printf("%10lf ", (curr_ts[i][j] - curr_ts[i][j - 1]) / (double)obj->device->dev_info.dev_freq_hz);
        }
        printf("\n");
    }
    printf("\n");
}

/***       ****/
/*** CNCO ***/
/***       ***/

int32_t fullchip_hop_exec_cnco_cfg(const fullchip_hop_exec_t* obj)
{
    int32_t err = API_CMS_ERROR_OK;

    printf("@%s()\n", __func__);

    // Put CNCO in DC test mode (bypass FNCO)
    adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, 10, 0);
    adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 10, 0);

    /* Load NCO FTW values for all CNCO */
    uint32_t nco_phase_inc_words[ADI_APOLLO_CNCO_PROFILE_NUM];
    uint32_t length = ADI_APOLLO_CNCO_PROFILE_NUM;

    fullchip_hop_nco_calc_t nco_calc;

    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
        calculate_cnco_hop_freq(obj, i, &nco_calc);
        nco_phase_inc_words[i] = nco_calc.tx_ftw;
    }

    adi_apollo_cnco_profile_load(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_NCO_PROFILE_PHASE_INCREMENT, 0, nco_phase_inc_words, length);
    adi_apollo_cnco_profile_load(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_NCO_PROFILE_PHASE_INCREMENT, 0, nco_phase_inc_words, length);

    /*
    * Configure the GPIO pinmux for CNCO profile hop selection. Only the profile GPIOs
    * need to be driven (from FPGA) to select the profile hop. Other pins are set via registers.
    * 
    * The 'fcn' bits are set for CNCO.
    * The CNCO 'slice selects' are set to enable all slices.
    *
    * Alternatively, these GPIOs can be programmed entirely through device GPIO pins.
    */
    adi_apollo_gpio_hop_block_select_set(obj->device, ADI_APOLLO_GPIO_HOP_SELECT_GPIO);
    block_gpio_sel(obj->device, obj->fpga_device, ADI_APOLLO_GPIO_BLOCK_CNCO);

    adi_apollo_gpio_hop_slice_select_set(obj->device, ADI_APOLLO_GPIO_BLOCK_CNCO, ADI_APOLLO_GPIO_HOP_SELECT_SPI);
    adi_apollo_gpio_hop_cnco_enable_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, 1);
    adi_apollo_gpio_hop_cnco_enable_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, 1);

    /*
    *  Configure triggers to mute after specified value
    */
    adi_apollo_trigts_mst_mute_mask_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0xF, ADI_APOLLO_TRIG_MUTE_MASK_0);
    adi_apollo_trigts_mst_mute_mask_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0xF, ADI_APOLLO_TRIG_MUTE_MASK_0);

    return err;
}

static int32_t cnco_direct_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    adi_apollo_nco_profile_sel_mode_e profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP;
    char cap_name[MAX_PATH_LEN];
    printf("@%s()\n", __func__);

    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, profile_sel_mode);
    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, profile_sel_mode);

    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
        calculate_cnco_hop_freq(obj, i, &nco_calc);
        adi_apollo_cnco_active_profile_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, i);
        adi_apollo_cnco_active_profile_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, i);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_cnco_dir_regmap_h",i);
        capture(obj, cap_name, false);

        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;
}

static int32_t cnco_direct_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    adi_apollo_nco_profile_sel_mode_e profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_DIRECT_GPIO;
    char cap_name[MAX_PATH_LEN];
    printf("@%s()\n", __func__);

    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, profile_sel_mode);
    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, profile_sel_mode);

    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
        calculate_cnco_hop_freq(obj, i, &nco_calc);
        profile_gpio_sel(obj->device, obj->fpga_device, i);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_cnco_dir_gpio_h",i);
        capture(obj, cap_name, false);
        
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;
}

static int32_t cnco_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_name[256];

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_cdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_cdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO). Will hop to selected profile when triggered.
    */
    adi_apollo_coarse_nco_hop_t nco_hop_config;

    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_REGMAP;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_INCR;    // No actual auto increment.
    nco_hop_config.hop_ctrl_init = 1;
    nco_hop_config.next_hop_number_wr_en = 1;
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);

    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
        calculate_cnco_hop_freq(obj, i, &nco_calc);
        adi_apollo_cnco_next_hop_num_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, i);
        adi_apollo_cnco_next_hop_num_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, i);

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_cnco_trig_by_spi_sel_by_regmap_h",i);
        capture(obj, cap_name, false);

        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;
}

static int32_t cnco_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_name[256];

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_cdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_cdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO). Will hop to selected profile when triggered.
    */
    adi_apollo_coarse_nco_hop_t nco_hop_config;

    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_GPIO;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_INCR;    // No actual auto increment.
    nco_hop_config.hop_ctrl_init = 1;
    nco_hop_config.next_hop_number_wr_en = 1;
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);

    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM; i++) {
        calculate_cnco_hop_freq(obj, i, &nco_calc);
        profile_gpio_sel(obj->device, obj->fpga_device, i);

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_cnco_trig_by_spi_sel_by_gpio_h",i);
        capture(obj, cap_name, false);
        
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;  
}

static int32_t cnco_trig_by_tmaster_sel_by_auto(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_fname_base[256];
    uint64_t curr_timestamp_count;
    uint64_t trig_per = obj->device->dev_info.dev_freq_hz * 3;  // Gen trigger every 5s
    adi_apollo_trig_mst_config_t trig_mst_config;
    adi_apollo_coarse_nco_hop_t nco_hop_config;
    int frequency;

    // Initialize to profile 0
    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP);
    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP);
    adi_apollo_cnco_active_profile_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, 0);
    adi_apollo_cnco_active_profile_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, 0);

    /*
    * Trigger mode mux for CNCO: SPI or Trig Master (timestamp) or External (dynamic reconfig)
    */
    adi_apollo_trigts_cdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_TRIG_MASTER);
    adi_apollo_trigts_cdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_TRIG_MASTER);

    /*
     * Select timestamp counter reset by SPI
     */
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);

    trig_mst_config.trig_period = trig_per;                 // Trig Period unit is Fs
    trig_mst_config.trig_offset = 32;                       // Min for 4t4r per userguide
    trig_mst_config.trig_enable = ADI_APOLLO_TRIG_ENABLE;   //
    adi_apollo_trigts_cnco_trig_mst_config(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &trig_mst_config);
    adi_apollo_trigts_cnco_trig_mst_config(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, &trig_mst_config);

    /*
    *  Reset timestamp before auto trigger or else the reset will cause a hop from initial profile
    */

    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0);

    /*
    *  Profile selection method (REGMAP or GPIO or AUTO). Will hop to selected profile when triggered.
    */
    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_AUTO;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_INCR;    // No actual auto increment.
    nco_hop_config.hop_ctrl_init = 1;
    nco_hop_config.next_hop_number_wr_en = 1;
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);

    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_0, ADI_APOLLO_TRIG_MST_CNCO_0, ADI_APOLLO_CNCO_PROFILE_NUM - 1);
    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_0, ADI_APOLLO_TRIG_MST_CNCO_0, ADI_APOLLO_CNCO_PROFILE_NUM - 1);
    
    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM; i++) {

        sprintf(cap_fname_base, "fullchip_hop_cnco_trig_by_tmaster_sel_by_auto_incr_h%02d", i);
        capture(obj, cap_fname_base, false);
        calculate_cnco_hop_freq(obj, i, &nco_calc);

        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_fname_base, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, false);

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        do {
            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
        } while (curr_timestamp_count < trig_per * (i+1));

        printf("Timestamp %lld is greater than trigger value %lld\n", curr_timestamp_count, trig_per * (i+1));
    }

    /*
    *  Profile selection method (REGMAP or GPIO or AUTO). Will hop to selected profile when triggered.
    */
    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_AUTO;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DECR;    // No actual auto increment.
    nco_hop_config.hop_ctrl_init = 1;
    nco_hop_config.next_hop_number_wr_en = 1;
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);

    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_0, ADI_APOLLO_TRIG_MST_CNCO_0, ADI_APOLLO_CNCO_PROFILE_NUM - 1);
    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_0, ADI_APOLLO_TRIG_MST_CNCO_0, ADI_APOLLO_CNCO_PROFILE_NUM - 1);

    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0);

    for (int i = 0; i < ADI_APOLLO_CNCO_PROFILE_NUM-1; i++) {

        sprintf(cap_fname_base, "fullchip_hop_cnco_trig_by_tmaster_sel_by_auto_decr_h%02d", i);
        capture(obj, cap_fname_base, false);
        calculate_cnco_hop_freq(obj, ADI_APOLLO_CNCO_PROFILE_NUM - i - 2, &nco_calc);

        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_fname_base, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, false);

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        do {
            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
        } while (curr_timestamp_count < trig_per * (i+1));
        printf("Timestamp %lld is greater than trigger value %lld\n", curr_timestamp_count, trig_per * (i+1));
    }

    /*
    *  Profile selection method (REGMAP or GPIO or AUTO). Will hop to selected profile when triggered.
    */
    nco_hop_config.profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_AUTO;
    nco_hop_config.auto_mode = ADI_APOLLO_NCO_AUTO_HOP_FLIP;    // No actual auto increment.
    nco_hop_config.hop_ctrl_init = 0;
    nco_hop_config.next_hop_number_wr_en = 0;
    nco_hop_config.high_limit = 5;
    nco_hop_config.low_limit = 0;
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);
    adi_apollo_cnco_hop_enable(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, &nco_hop_config);

    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MST_CNCO_0, ADI_APOLLO_TRIG_MUTE_MASK_0, 8);
    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MST_CNCO_0, ADI_APOLLO_TRIG_MUTE_MASK_0, 8);

    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0);

    frequency = 1;
    for (int i = 0; i < 8; i++) {
        calculate_cnco_hop_freq(obj, frequency, &nco_calc);

        sprintf(cap_fname_base, "fullchip_hop_cnco_trig_by_tmaster_sel_by_auto_flip_h%02d", i);
        capture(obj, cap_fname_base, false);

        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_fname_base, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, false);

        frequency += (i < 8/2) ? 1 : -1;

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        do {
            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
        } while (curr_timestamp_count < trig_per * (i+1));
        printf("Timestamp %lld is greater than trigger value %lld\n", curr_timestamp_count, trig_per * (i+1));
    }

    return err;
}

/***       ****/
/*** FNCO ***/
/***       ***/

int32_t fullchip_hop_exec_fnco_cfg(const fullchip_hop_exec_t* obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;

    adi_apollo_fine_nco_chan_pgm_t chan_cfg = {
        .drc_phase_inc = 0,
        .drc_phase_offset = 0,
    };

    printf("@%s()\n", __func__);

    // Set CNCO frequency to base value
    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP);
    adi_apollo_cnco_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP);
    calculate_cnco_hop_freq(obj, 0, &nco_calc);
    adi_apollo_cnco_ftw_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, 0, 1, nco_calc.rx_ftw);
    adi_apollo_cnco_ftw_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, 0, 1, nco_calc.tx_ftw);

    /*
     * Enable hopping
     */
    adi_apollo_fnco_hop_enable(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 1);
    adi_apollo_fnco_hop_enable(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, 1);

    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM; i++) {
        calculate_fnco_hop_freq(obj, i, &nco_calc);
        chan_cfg.drc_phase_inc = nco_calc.tx_ftw;
        adi_apollo_fnco_chan_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, i, &chan_cfg);
        chan_cfg.drc_phase_inc = nco_calc.rx_ftw;
        adi_apollo_fnco_chan_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, i, &chan_cfg);
    }

    /*
    * Configure the GPIO pinmux for FNCO profile hop selection. Only the profile GPIOs
    * need to be driven (from FPGA) to select the profile hop. Other pins are set via registers.
    * 
    * The 'fcn' bits are set for FNCO.
    * The FNCO 'slice selects' are set to enable all slices.
    *
    * Alternatively, these GPIOs can be programmed entirely through device GPIO pins.
    */
    adi_apollo_gpio_hop_block_select_set(obj->device, ADI_APOLLO_GPIO_HOP_SELECT_GPIO);
    block_gpio_sel(obj->device, obj->fpga_device, ADI_APOLLO_GPIO_BLOCK_FNCO);

    adi_apollo_gpio_hop_slice_select_set(obj->device, ADI_APOLLO_GPIO_BLOCK_FNCO, ADI_APOLLO_GPIO_HOP_SELECT_SPI);
    adi_apollo_gpio_hop_fnco_enable_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 1);
    adi_apollo_gpio_hop_fnco_enable_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, 1);

    /*
    *  Configure triggers to mute after specified value
    */
    adi_apollo_trigts_mst_mute_mask_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0xFF0, ADI_APOLLO_TRIG_MUTE_MASK_1);
    adi_apollo_trigts_mst_mute_mask_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0xFF0, ADI_APOLLO_TRIG_MUTE_MASK_1);

    return err;
}

static int32_t fnco_direct_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_name[MAX_PATH_LEN];
    printf("@%s()\n", __func__);

    adi_apollo_fine_nco_hop_t hop_cfg = {
        .nco_trig_hop_sel = ADI_APOLLO_FNCO_TRIG_HOP_FREQ,
        .profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP,
        .phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DIR_INCR,
        .phase_inc_high_limit = 31,
        .phase_inc_low_limit = 0,
    };

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM; i++) {
        adi_apollo_fnco_active_profile_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, i);
        adi_apollo_fnco_active_profile_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, i);

        calculate_fnco_hop_freq(obj, i, &nco_calc);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_fnco_dir_regmap_h",i);
        capture(obj, cap_name, false);
        
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;
}

static int32_t fnco_direct_gpio(const fullchip_hop_exec_t *obj)
{

    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_name[MAX_PATH_LEN];
    printf("@%s()\n", __func__);

    profile_gpio_sel(obj->device, obj->fpga_device, 0);

    adi_apollo_fine_nco_hop_t hop_cfg = {
        .nco_trig_hop_sel = ADI_APOLLO_FNCO_TRIG_HOP_FREQ,
        .profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_DIRECT_GPIO,
        .phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DIR_INCR,
        .phase_inc_high_limit = 31,
        .phase_inc_low_limit = 0,
    };

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM; i++) {
        profile_gpio_sel(obj->device, obj->fpga_device, i);
        calculate_fnco_hop_freq(obj, i, &nco_calc);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_fnco_dir_gpio_h",i);
        capture(obj, cap_name, false);
        
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;
}

static int32_t fnco_trig_by_spi_sel_by_regmap(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_name[256];

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_fdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_fdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO). Will hop to selected profile when triggered.
    */
    adi_apollo_fine_nco_hop_t hop_cfg = {
        .nco_trig_hop_sel = ADI_APOLLO_FNCO_TRIG_HOP_FREQ,
        .profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_REGMAP,
        .phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DIR_INCR,
        .phase_inc_high_limit = 31,
        .phase_inc_low_limit = 0,
    };

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM; i++) {

        adi_apollo_fnco_next_hop_num_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, i, 0);
        adi_apollo_fnco_next_hop_num_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, i, 0);

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);
        calculate_fnco_hop_freq(obj, i, &nco_calc);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_fnco_trig_by_spi_sel_by_regmap_h",i);
        capture(obj, cap_name, false);
        
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;
}

static int32_t fnco_trig_by_spi_sel_by_gpio(const fullchip_hop_exec_t *obj)
{

    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_name[256];

    profile_gpio_sel(obj->device, obj->fpga_device, 0);

    /*
    * Trigger mode (SPI, Trig Master (timestamp), External (dynamic reconfig)
    */
    adi_apollo_trigts_fdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_TRIG_SPI);
    adi_apollo_trigts_fdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_TRIG_SPI);

    /*
    *  Profile selection method (REGMAP or GPIO). Will hop to selected profile when triggered.
    */
    adi_apollo_fine_nco_hop_t hop_cfg = {
        .nco_trig_hop_sel = ADI_APOLLO_FNCO_TRIG_HOP_FREQ,
        .profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_GPIO,
        .phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DIR_INCR,
        .phase_inc_high_limit = 31,
        .phase_inc_low_limit = 0,
    };

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM; i++) {

        profile_gpio_sel(obj->device, obj->fpga_device, i);

        /* Hop not applied until AFTER these SPI initiated trigger */
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL);
        adi_apollo_trigts_trig_now(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL);

        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);
        calculate_fnco_hop_freq(obj, i, &nco_calc);
        
        snprintf(cap_name, MAX_PATH_LEN, "%s%02d","fullchip_hop_fnco_trig_by_spi_sel_by_gpio_h",i);
        capture(obj, cap_name, false);
        
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_name, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, obj->is_interactive);
    }

    return err;  
}

static int32_t fnco_trig_by_tmaster_sel_by_auto(const fullchip_hop_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    fullchip_hop_nco_calc_t nco_calc;
    char cap_fname_base[256];
    uint64_t curr_timestamp_count;
    uint64_t trig_per = obj->device->dev_info.dev_freq_hz * 3;  // Gen trigger every 5s
    adi_apollo_trig_mst_config_t trig_mst_config;
    int frequency;

    // Initialize to profile 0
    adi_apollo_fnco_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP);
    adi_apollo_fnco_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP);
    adi_apollo_fnco_active_profile_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, 0);
    adi_apollo_fnco_active_profile_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, 0);

    /*
    * Trigger mode mux for FNCO: SPI or Trig Master (timestamp) or External (dynamic reconfig)
    */
    adi_apollo_trigts_fdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_TRIG_MASTER);
    adi_apollo_trigts_fdrc_trig_sel_mux_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, ADI_APOLLO_TRIG_MASTER);

    /*
     * Select timestamp counter reset by SPI
     */
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);
    adi_apollo_trigts_ts_reset_mode_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_TS_RESET_MODE_SPI);

    trig_mst_config.trig_period = trig_per;                 // Trig Period unit is Fs
    trig_mst_config.trig_offset = 32;                       // Min for 4t4r per userguide
    trig_mst_config.trig_enable = ADI_APOLLO_TRIG_ENABLE;   //
    adi_apollo_trigts_fnco_trig_mst_config(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &trig_mst_config);
    adi_apollo_trigts_fnco_trig_mst_config(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &trig_mst_config);

    /*
    *  Reset timestamp before auto trigger or else the reset will cause a hop from initial profile
    */

    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0);

    /*
    *  Profile selection method (REGMAP or GPIO or AUTO). Will hop to selected profile when triggered.
    */
    adi_apollo_fine_nco_hop_t hop_cfg = {
        .nco_trig_hop_sel = ADI_APOLLO_FNCO_TRIG_HOP_FREQ,
        .profile_sel_mode = ADI_APOLLO_NCO_CHAN_SEL_TRIG_AUTO,
        .phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DIR_INCR,
        .phase_inc_high_limit = 31,
        .phase_inc_low_limit = 0,
    };

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_1, ADI_APOLLO_TRIG_MST_FNCO_0, ADI_APOLLO_FNCO_PROFILE_NUM - 1);
    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_1, ADI_APOLLO_TRIG_MST_FNCO_0, ADI_APOLLO_FNCO_PROFILE_NUM - 1);
    
    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM; i++) {

        sprintf(cap_fname_base, "fullchip_hop_fnco_trig_by_tmaster_sel_by_auto_incr_h%02d", i);
        capture(obj, cap_fname_base, false);

        calculate_fnco_hop_freq(obj, i, &nco_calc);

        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_fname_base, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, false);

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        do {
            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
        } while (curr_timestamp_count < trig_per * (i+1));

        printf("Timestamp %lld is greater than trigger value %lld\n", curr_timestamp_count, trig_per * (i+1));
    }

    /*
    *  Profile selection method (REGMAP or GPIO or AUTO). Will hop to selected profile when triggered.
    */
    hop_cfg.phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_DIR_DECR;

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_1, ADI_APOLLO_TRIG_MST_FNCO_0, ADI_APOLLO_FNCO_PROFILE_NUM - 1);
    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_1, ADI_APOLLO_TRIG_MST_FNCO_0, ADI_APOLLO_FNCO_PROFILE_NUM - 1);

    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0);

    for (int i = 0; i < ADI_APOLLO_FNCO_PROFILE_NUM-1; i++) {

        sprintf(cap_fname_base, "fullchip_hop_fnco_trig_by_tmaster_sel_by_auto_decr_h%02d", i);
        capture(obj, cap_fname_base, false);

        calculate_fnco_hop_freq(obj, ADI_APOLLO_FNCO_PROFILE_NUM - 1 - i, &nco_calc);
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_fname_base, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, false);

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        do {
            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
        } while (curr_timestamp_count < trig_per * (i+1));
        printf("Timestamp %lld is greater than trigger value %lld\n", curr_timestamp_count, trig_per * (i+1));
    }

    /*
    *  Profile selection method (REGMAP or GPIO or AUTO). Will hop to selected profile when triggered.
    */
    hop_cfg.phase_inc_auto_mode = ADI_APOLLO_NCO_AUTO_HOP_FLIP;
    hop_cfg.phase_inc_high_limit = 9;

    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, &hop_cfg);
    adi_apollo_fnco_hop_pgm(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, &hop_cfg);

    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_1, ADI_APOLLO_TRIG_MST_FNCO_0, 15);
    adi_apollo_trigts_mst_mute_mask_count_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, ADI_APOLLO_TRIG_MUTE_MASK_1, ADI_APOLLO_TRIG_MST_FNCO_0, 15);

    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_TX, ADI_APOLLO_SIDE_ALL, 0);
    adi_apollo_trigts_ts_reset(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_ALL, 0);

    frequency = 1;
    for (int i = 0; i < 15; i++) {

        sprintf(cap_fname_base, "fullchip_hop_fnco_trig_by_tmaster_sel_by_auto_flip_h%02d", i);
        capture(obj, cap_fname_base, false);

        calculate_fnco_hop_freq(obj, frequency, &nco_calc);
        EXCTL_RX_MEAS_FREQ(CHANNEL_SEL, cap_fname_base, ADC_FREQ_MHZ, 10);
        EXCTL_TX_MEAS_FREQ(CHANNEL_SEL, nco_calc.tone_freq, 10, false);
        frequency += (i < 8) ? 1 : -1;

        /* Wait for trigger - new profile hop will be applied upon trigger event */
        do {
            adi_apollo_hal_delay_us(obj->device, 10000); // poll delay
            adi_apollo_trigts_counter_get(obj->device, ADI_APOLLO_RX, ADI_APOLLO_SIDE_B, &curr_timestamp_count); //  Using Rx/B since last ts reset async (slowest)
        } while (curr_timestamp_count < trig_per * (i+1));
        printf("Timestamp %lld is greater than trigger value %lld\n", curr_timestamp_count, trig_per * (i+1));
    }


    return err;
}

static int32_t calculate_cnco_hop_freq(const fullchip_hop_exec_t *obj, uint32_t hop, fullchip_hop_nco_calc_t * calc)
{
    adi_ads10_apollo_dp_info_t dp_info;
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &dp_info);
    double nco_spacing = dp_info.fdata / (ADI_APOLLO_CNCO_PROFILE_NUM + 1); // evenly space NCO across Fdata
    calc->nco_freq = ADC_FREQ_MHZ + dp_info.fdata/2 - ((ADI_APOLLO_CNCO_PROFILE_NUM - hop) * nco_spacing); // Set CNCO freqs to mirror across adc freq
    calc->tone_freq = calc->nco_freq + dp_info.fdata * TONE_RATIO; // Output tone is offset based on tone ratio
    calc->tx_ftw = (calc->nco_freq)/(dp_info.adc_sample_rate)*(1ull<<32); // Assumes dac and adc sample rates are the same
    calc->rx_ftw = calc->tx_ftw;
    return API_CMS_ERROR_OK;
}

static int32_t calculate_fnco_hop_freq(const fullchip_hop_exec_t *obj, uint32_t hop, fullchip_hop_nco_calc_t * calc)
{
    adi_ads10_apollo_dp_info_t dp_info;
    calculate_cnco_hop_freq(obj, 0, calc);
    double cnco_freq = calc->nco_freq; // CNCO should be set to baseline, FNCO hops start at 0
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &dp_info);
    double nco_spacing = dp_info.fdata / (ADI_APOLLO_FNCO_PROFILE_NUM + 1); // Evenly space nco across Fdata
    calc->nco_freq = hop * nco_spacing;
    calc->tone_freq = calc->nco_freq + dp_info.fdata * TONE_RATIO + cnco_freq; // Output tone is offset by tone ratio and CNCO
    // Assume adc/dac clocks the same, different CDRC
    calc->rx_ftw = (calc->nco_freq)/(dp_info.adc_sample_rate/dp_info.cdrc)*(1ull<<32);
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &dp_info);
    calc->tx_ftw = (calc->nco_freq)/(dp_info.adc_sample_rate/dp_info.cdrc)*(1ull<<32);
    return API_CMS_ERROR_OK;
}

static int32_t set_ncos(const fullchip_hop_exec_t *obj)
{
    int32_t err;
    adi_ads10_apollo_dp_info_t dp_info;
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &dp_info);
    double combined_nco_freq = ADC_FREQ_MHZ - (dp_info.fdata * TONE_RATIO);
    double coarse_nco_freq = 10 * combined_nco_freq / 11;
    double fine_nco_freq = combined_nco_freq - coarse_nco_freq;
    err = adi_ads10_apollo_ex_cnco_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, dp_info.adc_sample_rate, coarse_nco_freq);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_cnco_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_ALL, dp_info.adc_sample_rate, coarse_nco_freq);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_ALL, dp_info.adc_sample_rate/dp_info.cdrc, fine_nco_freq);
	ADI_CMS_ERROR_RETURN(err);
    adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &dp_info);
    err = adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, dp_info.adc_sample_rate/dp_info.cdrc, fine_nco_freq);
	ADI_CMS_ERROR_RETURN(err);
    return err;
}

static int32_t gpio_hop_custom_configure(adi_apollo_device_t* device, adi_apollo_gpio_hop_profile_t *hop_config, adi_apollo_gpio_hop_block_t *block_config)
{
    int32_t err;

    err = adi_apollo_gpio_hop_profile_configure(device, hop_config);
    ADI_APOLLO_ERROR_RETURN(err);
    err = adi_apollo_gpio_hop_block_configure(device, block_config);
    ADI_APOLLO_ERROR_RETURN(err);

    return err;
}