#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo Rx ADC SMON example
 *
 * 
 * This example demonstrates the SMON feature of Apollo. The SMON block measures an ADC’s peak sample value over a
 * time interval. The result, an 11-bit absolute val, is obtained from three methods.
 *  1. Register read
 *  2. GPIO (2-bits) representing low, high and mid levels relative to programmed low and high threshold values.
 *  3. Capture data via JESD JTx control bits. The SMON peak value is serialized. Two framers are available (5-bit, 10-bit)
 * 
 * There are 2 SMONs per side on 4t4R devices and 4 SMONs per side on 8T8R devices. Essentially, one for each ADC.
 * The SMON blocks are located after the RxMux-1 crossbar. It is possible to route SMONs to any ADC on a side.
 * 
 * The SMON block is configured from device profiles during Rx data path initialization. If this example is run with a 
 * profile that doesn't enable SMONs, then it will configure them programmatically. However, JESD serialization cannot 
 * be used in this case because JESD initialization (e.g. setting N, and CS values) is handle by FW only.
 * 
 * This example supports 3 command line options:
 *    -c      Continuous mode. Will loop continuously reading and display SMON data (^C to break.)
 *    -i      Interactive mode. Prompts user for setting ADC input freq/levels.
 *    -v      Validation mode. Verifies that GPIO and JESD SMON values match register values. 
 *            For GPIO validation, the ADC input level remains constant and the low/high thresholds
 *            are changed to produce low, mid and high outputs.
 *            For JESD SMON validation, the deserialized value is compared against the register readback.
 *            Returns error if validation fails.
 * 
 * These options my be combined.
 *
 *
 *                 High/Low Threshold    Period
 *                                 | |   |
 *    adi_apollo_smon_thresh_set() | |   |
 *                                 | |   |
 *                                 v v   v
 *             +-----+          +-----------+
 *  +------+   |     |CBout-0   |           |-----------> GPIO-0
 *  |ADC A0|-->|     |--------->|  SMON-A0  |-----------> GPIO-1
 *  +------+   |     |          |           |
 *             |RxMux|          |  PK Det   |-----------> Reg Val (pk)   adi_apollo_smon_peak_val_get()
 *  +------+   |  1  |CBout-1   |           |
 *  |ADC A1|-->|     |------    |           |        +----+
 *  +------+   |     |     |    |  SFramer  |------->|    |  <-----------------------|
 *             +-----+     |    +-----------+        |FDDC|         adi_apollo_dformat_smon_fd_fddc_set()
 *                         |                         |I/Q |---|                      |
 *                         |                         |SMON|   |                      |
 *                         |                     |-->|Sel |   |                      |
 *                         |    +-----------+    |   +----+   |                      |
 *                         |--->|           |    |            |    +-----+           |
 *                              |  SMON-A1  |    |       SMON |--->|     |           |           +----+
 *                              |           |    |    FD --------->| JTx |           v           |    |
 *                              |  PK Det   |    |   LOW --------->| Ctrl|    +-------------+    | VC |   Capture Files: _Lx_My.txt
 *                              |           |    |   OVR --------->| Bit |--->|FDDC I/Q Path|--->|FDDC|----->>
 *                              |           |    |                 | Sel |    +-------------+    |CONV|   L=Link [0:3], M=VirtConv [0..15]
 *                              |  SFramer  |----|                 |     |                       | MAP|
 *                              +-----------+                      +-----+                       |    |
 *                                                                    ^                          +----+
 *                                                                    | 
 *                                                                    | adi_apollo_dformat_ctrl_bit_sel_set()
 *
 * Usage:
 * rx_adc_smon <profile> [-c][-i][-v]
 * 
 * Spme example profiles:
 * id00_uc06_smon       (4T4R, 5-bit framer)
 * id00_uc06_smon_10b   (4T4R, 10-bit framer)
 * id81_uc28_smon       (8T8R, 5-bit framer)
 * id81_uc28_smon_10b   (8T8R, 10-bit framer)
 *
 * Example output for an SMON reading (4T4R):
 *  SMON  |  FCNT PK   |        REG             |          JESD          |              GPIO
 * -------|------------|------------------------|------------------------|-------------------------------
 * [0x01] | 235 0x0318 | 0x031e  -8.2 dBFS      | 0x031a  -8.2 dBFS      | 0x01 MID **  [L:-15.0 H: -3.0]
 * [0x02] | 161 0x0402 | 0x03ff  -6.0 dBFS      | 0x0409  -5.9 dBFS      | 0x01 MID **  [L:-15.0 H: -3.0]
 * [0x10] | 172 0x042f | 0x0433  -5.6 dBFS      | 0x042c  -5.7 dBFS      | 0x01 MID **  [L:-15.0 H: -3.0]
 * [0x20] |  94 0x041f | 0x041b  -5.8 dBFS      | 0x041a  -5.8 dBFS      | 0x01 MID **  [L:-15.0 H: -3.0]
 *
 *
 * Header Definitions:
 * SMON     SMON bit-wise selector (0x01=A0, 0x02=A1, 0x10=B0, 0x20=B1)
 * FCNT PK  Frame count (8-bit frame update counter), PK value from adi_apollo_smon_read()
 * REG      PK value from adi_apollo_smon_peak_val_get()
 * JESD     PK value from JESD control bits. Value is de-serialized from capture data
 * GPIO     2-bit status from low and high threshold settings. (0x00=low, 0x01=mid, 0x33=high)
 * 
 * Converting an 11-bit peak value to dBFS: 20*log10(pk_val/2^11)
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
#include "adi_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_fpga_apollo_gpio.h"

#define PK_VAL_TO_DBFS(peak_val) (20.0 * log10(((double)peak_val) / 2048.0))
#define DBFS_TO_PK_VAL(dbfs) (uint16_t)(pow(10.0, ((dbfs) / 20.0)) * 2048.0)
#define CAP_FNAME_BASE "rx_adc_smon"

/* Contains which SMON block features are enabled */
typedef struct {
    bool reg_en;
    bool jesd_en;
    bool gpio_en;
} smon_blk_en_t;

static int32_t smon_status_get(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], adi_apollo_smon_read_t results[], uint16_t smons_len);
static int32_t smon_pk_val_reg_get(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t results[], uint16_t smons_len);
static int32_t smon_pk_val_jesd_get(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t results[], uint16_t smons_len);
static int32_t smon_pk_val_gpio_get(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t results[], uint16_t smons_len);
static int32_t peak_val_from_cap_file_get(adi_apollo_device_t *device, char *fname, adi_apollo_blk_sel_t smon, uint16_t *peak_val);
static int32_t smon_configure(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t smons_len);
static int32_t smon_gpios_configure(adi_apollo_device_t *device);
static int32_t smon_inspect(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], int smons_len);
static int32_t smon_hi_lo_thresh_set(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], double input_ofst_db[], int smons_len, int iteration);
static int32_t smon_gpio_validate(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t pk_val_gpio_results[], int smons_len, int iteration);
static int32_t smon_jesd_validate(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t pk_val_reg_results[], uint16_t pk_val_jesd_results[], int smons_len);
static int32_t smon_input_offset_set(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], double input_ofst_db[], int smons_len, double *avg_ofst);
static int32_t adc_input_level_set(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], double input_ofst_db[], int smons_len, double target_input_db, double rf_src_level_db, bool interactive);

static char *status_to_string(const adi_apollo_smon_read_t *result, bool en, char *str);
static char *result_to_string(uint16_t result, bool en, char *str);
static char *gpio_to_string(adi_apollo_device_t *device, adi_apollo_blk_sel_t smon, uint16_t result, bool en, char *str);

static uint8_t gpio_num_to_smon_map[ADI_APOLLO_SMON_NUM][2] = {
    /* 4T4R and 8T8R */
    {17, 18}, // SMON-A0
    {19, 20}, // SMON-A1
    {21, 22}, // SMON-B0
    {23, 34}, // SMON-B1

    /* 8T8R only */
    {25, 26}, // SMON-A2
    {27, 28}, // SMON-A3
    {29, 30}, // SMON-B2
    {31, 32}  // SMON-B3
};

int32_t rx_adc_smon(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = false;
    bool validate = false;
    bool continuous = false;
    uint16_t i, j;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    char str_buff2[4][80];
    adi_ads10_apollo_dp_info_t rx_dp_info;
    uint16_t smons_len = device->dev_info.is_8t8r ? ADI_APOLLO_SMON_NUM : ADI_APOLLO_SMON_NUM / 2;
    smon_blk_en_t smons_blk_en[ADI_APOLLO_SMON_NUM] = {{false}};
    adi_apollo_smon_read_t smon_status_results[ADI_APOLLO_SMON_NUM] = {{0}};
    uint16_t pk_val_reg_results[ADI_APOLLO_SMON_NUM] = {0};
    uint16_t pk_val_jesd_results[ADI_APOLLO_SMON_NUM] = {0};
    uint16_t pk_val_gpio_results[ADI_APOLLO_SMON_NUM] = {0};
    double input_ofst_db[ADI_APOLLO_SMON_NUM] = {0};
    double adc_input_freq = 2300.0;
    double rf_src_level_db = device->dev_info.is_8t8r ? 3.0 : 6.0;          // Initial RF soruce input level
    double target_input_db = -6.0;                                          // Target SMON mid level

    adi_apollo_blk_sel_t smons[ADI_APOLLO_SMON_NUM] = {
        ADI_APOLLO_SMON_A0, ADI_APOLLO_SMON_A1, ADI_APOLLO_SMON_B0, ADI_APOLLO_SMON_B1, /* 4T4R and 8T8R */
        ADI_APOLLO_SMON_A2, ADI_APOLLO_SMON_A3, ADI_APOLLO_SMON_B2, ADI_APOLLO_SMON_B3  /* 8T8R only */
    };

    /* 
     * Process command line options 
     * -i: interactive mode 
     * -v: validate mode. Verifies GPIO threshold settings and JESD framer
     **/
    for (i = argc_ofst; i < argc; ++i) {
        if (strcmp(argv[i], "-i") == 0) {
            interactive = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            validate = true;
        } else if (strcmp(argv[i], "-c") == 0) {
            continuous = true;
        }
    }

    /* Set RF Source to for ADC inputs */
    EXCTL_SIGGEN_ON(0x33, false);
    EXCTL_SIGGEN_FREQ(adc_input_freq, interactive);
    EXCTL_SIGGEN_LEVEL(rf_src_level_db, interactive);

    /* Print some Rx data path info */
    err = adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Profile Rx", str_buff, str_buff_len);
    ADI_CMS_ERROR_RETURN(err);
    printf("\n%s\n\n", str_buff);

    /* Inspect the Apollo JTx link config */
    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* 
     * Configure SMON from the profile or programmatically if SMON not enabled.
     * 'smons_blk_en[]' keeps track on individual smon feature enables (e.g. reg, gpio or jesd)
     */
    err = smon_configure(device, profile, smons, smons_blk_en, smons_len);
    ADI_CMS_ERROR_RETURN(err);

    /* Map the SMON GPIOs */
    err = smon_gpios_configure(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Inspect SMON block configs */
    err = smon_inspect(device, smons, smons_len);
    ADI_CMS_ERROR_RETURN(err);

    /* Run clock conditioning and ADC init cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    /* Init FPGA JRx links */
    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /* Start ADC background cal */
     err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
     ADI_CMS_ERROR_RETURN(err);

    // If running example in validate mode, adjust input level for proper low-mid-hi threshold testing.
    if (validate) {
        err = adc_input_level_set(device, smons, input_ofst_db, smons_len, target_input_db, rf_src_level_db, interactive);
        ADI_CMS_ERROR_RETURN(err);
    }

    for (i = 0; (i < 6) || continuous; i++) {

        if (validate) {
            /* Set the Low and High GPIO thresholds. */
            err = smon_hi_lo_thresh_set(device, smons, smons_blk_en, input_ofst_db, smons_len, i);
            ADI_CMS_ERROR_RETURN(err);
        }

        /* Get SMON status - frame count and pk val */
        err = smon_status_get(device, smons, smons_blk_en, smon_status_results, smons_len);
        ADI_CMS_ERROR_RETURN(err);

        /* Get peak val only from reading register */
        err = smon_pk_val_reg_get(device, smons, smons_blk_en, pk_val_reg_results, smons_len);
        ADI_CMS_ERROR_RETURN(err);

        /* Get peak val from JESD */
        err = smon_pk_val_jesd_get(device, fpga_device, profile, smons, smons_blk_en, pk_val_jesd_results, smons_len);
        ADI_CMS_ERROR_RETURN(err);

        /* Get the 2-bit GPIO low-mid-high val */
        err = smon_pk_val_gpio_get(device, fpga_device, smons, smons_blk_en, pk_val_gpio_results, smons_len);
        ADI_CMS_ERROR_RETURN(err);

        printf("\n");
        printf(" SMON  |  FCNT PK   |        REG             |          JESD          |              GPIO             \n");
        printf("-------|------------|------------------------|------------------------|-------------------------------\n");
        for (j = 0; j < smons_len; j++) {
            printf("[0x%02X] | %-10s | %-22s | %-22s | %-30s\n",
                   smons[j],
                   status_to_string(&smon_status_results[j], smons_blk_en[j].reg_en, str_buff2[0]),
                   result_to_string(pk_val_reg_results[j], smons_blk_en[j].reg_en, str_buff2[1]),
                   result_to_string(pk_val_jesd_results[j], smons_blk_en[j].jesd_en, str_buff2[2]),
                   gpio_to_string(device, smons[j], pk_val_gpio_results[j], smons_blk_en[j].gpio_en, str_buff2[3]));
        }
        printf("\n");

        if (validate) {
            /* Validate the 2-bit GPIO outputs match set thresholds. Note, cycles through low, mid and hi (i % 3) */
            err = smon_gpio_validate(device, smons, smons_blk_en, pk_val_gpio_results, smons_len, i);
            ADI_CMS_ERROR_RETURN(err);

            /* Validate JESD de-serialized peak val aligns with val from register */
            err = smon_jesd_validate(device, smons, smons_blk_en, pk_val_reg_results, pk_val_jesd_results, smons_len);
            ADI_CMS_ERROR_RETURN(err);
        }

        if (interactive) {
            adi_apollo_hal_delay_us(device, 1000000 * 1.0);     // Delay for console observation
        }
    }

    return err;
}

int32_t smon_status_get(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], adi_apollo_smon_read_t results[], uint16_t smons_len)
{
    int32_t err;
    uint16_t i;

    /*                                               */
    /* SMON status - frame count and pk val from reg */
    /*                                               */
    for (i = 0; i < smons_len; i++) {

        if (!smons_blk_en[i].reg_en) {
            results[i].status_fcnt = 0x0000;
            results[i].status = 0x0000;
            continue;
        }

        /* Initiate a status update by toggling bit */
        err = adi_apollo_smon_status_update(device, smons[i], 0);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_smon_status_update(device, smons[i], 1);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_smon_read(device, smons[i], &results[i]);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_pk_val_reg_get(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t results[], uint16_t smons_len)
{
    int32_t err;
    uint16_t i;

    /*                           */
    /* Peak Mag from reg map     */
    /*                           */
    for (i = 0; i < smons_len; i++) {

        if (!smons_blk_en[i].reg_en) {
            results[i] = 0x0000;
            continue;
        }

        err = adi_apollo_smon_peak_val_get(device, smons[i], &results[i]);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_pk_val_jesd_get(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t results[], uint16_t smons_len)
{
    int32_t err;
    uint16_t i;
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H; /* min num samples per virt conv */

    // A - side get SMON serialized data from FDDC I samples
    // B - side get SMON serialized data from FDDC Q samples
    char *cap_fnames[ADI_APOLLO_SMON_NUM] = {
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L0_m0.txt", // ADC-A0  I
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L0_m2.txt", // ADC-A1  I
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L2_m1.txt", // ADC-B0  Q
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L2_m3.txt", // ADC-B1  Q

        /* 8T8R only */
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L0_m4.txt", // ADC-A2  I
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L0_m6.txt", // ADC-A3  I
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L2_m5.txt", // ADC-B2  Q
        OUTPUT_DIR "/" CAP_FNAME_BASE "_L2_m7.txt", // ADC-B3  Q
    };

    /* Capture ADC samples */
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, CAP_FNAME_BASE, false, false);
    ADI_CMS_ERROR_RETURN(err);

    /* Deserialize SMON peak val data from capture file */
    for (i = 0; i < smons_len; i++) {

        if (!smons_blk_en[i].jesd_en) {
            results[i] = 0xffff;
            continue;
        }

        err = peak_val_from_cap_file_get(device, cap_fnames[i], smons[i], &results[i]);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_pk_val_gpio_get(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t results[], uint16_t smons_len)
{
    int32_t err;
    uint16_t i;
    uint8_t gpio_state[ADI_APOLLO_SMON_NUM][2] = {{0}}; // FPGA readback of GPIO

    for (i = 0; i < smons_len; i++) {

        if (!smons_blk_en[i].gpio_en) {
            results[i] = 0x0002;
            continue;
        }

        /* Read the GPIO states */
        err = adi_fpga_apollo_gpio_input_get(fpga_device, gpio_num_to_smon_map[i][0], &gpio_state[i][0]);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_gpio_input_get(fpga_device, gpio_num_to_smon_map[i][1], &gpio_state[i][1]);
        ADI_CMS_ERROR_RETURN(err);

        results[i] = gpio_state[i][0] | (gpio_state[i][1] << 1);
    }

    return API_CMS_ERROR_OK;
}

int32_t peak_val_from_cap_file_get(adi_apollo_device_t *device, char *fname, adi_apollo_blk_sel_t smon, uint16_t *peak_val)
{
    int32_t err;
    uint16_t i;
    uint32_t sof_idx;
    uint16_t n_avg = 2;
    uint16_t pk_val = 0;
    uint32_t pk_val_avg = 0;
    int16_t *capture_buf;
    uint32_t num_samples_file = 0;
    int32_t ofst = 0;                   // Sample num in capture to start looking for frame.
    uint16_t ctrl_bit_mask = 0x0001;    // Locaton of SMON serialized bit in sample. Assumes JTx N=15, CS=1 and SMON is ctrl bit 2 (msb)

    err = adi_apollo_utilities_file_to_16b_samples_arr(device, fname, &capture_buf, &num_samples_file, 0);
    ADI_CMS_ERROR_RETURN(err);

    // Extract 'n_avg' frames from capture. Returned val 'sof_idx' can be used as an offset into capture to get the next frame.
    for (i = 0; i < n_avg; i++) {
        err = adi_apollo_smon_peak_val_from_cap_get(device, smon, capture_buf, num_samples_file, ofst, ctrl_bit_mask, &sof_idx, &pk_val);
        ADI_CMS_ERROR_GOTO(err, end);

        pk_val_avg += pk_val;

        ofst += sof_idx;
    }

    *peak_val = pk_val_avg / n_avg;
end:
    free(capture_buf);

    return err;
}

int32_t smon_configure(adi_apollo_device_t *device, adi_apollo_top_t *profile,
                       adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t smons_len)
{
    int32_t err;
    uint16_t i;
    bool map_fddc = false;
    uint16_t side;
    uint16_t idx;

    // SMON config - enable peak and gpio, disable JESD framer.
    adi_apollo_smon_pgm_t smon_pgm = {
        .sframer_mode_en = ADI_APOLLO_SFRAMER_FIVE_BIT_DISABLE,
        .smon_period = 1024,                              // Measurement duration counter
        .thresh_low = DBFS_TO_PK_VAL(-15.0),              // GPIO low threshold (in ADC sample val)
        .thresh_high = DBFS_TO_PK_VAL(-3.0),              // GPIO hi threshold (in ADC sample val)
        .sync_en = 0,                                     // 0: disable sync mode, 1: sync SMON on next SYSREF, ignore subsequent edges.
        .sync_next = 0,                                   // 0: continuous mode, 1: Next Sync mode (only applicable if sync_en = 1)
        .sframer_en = 0,                                  // 0: disable serial framer, 1: enable
        .sframer_mode = ADI_APOLLO_SMON_SFRAMER_FIVE_BIT, // 5-bit framer
        .sframer_insel = 2,                               // Peak detector data in serial frame
        .peak_en = 1,                                     // Peak detector enable
        .status_rdsel = 1,                                // Peak detector placed on status read-back
        .jlink_sel = 0,                                   // 0: jesd ink 0, 1: jesd link 1
        .gpio_en = 1                                      // 1: Enable GPIO peak output indicator, 0: disable
    };

    adi_apollo_dformat_smon_fd_map_t smon_fddc_4t4r_map[] = {
        // 4T4R - id00_uc06_smon (JTx m=4)
        // Assume 1-to-1 ADC-to-CBout RxMux1 mapping
        // Assume JTX FDDC-to-VC FDDC mapping [0, 1, 4, 5] => [A0-I, A0-Q, A2-I, A2-Q]
        // ADC   CBout    FDDC    I/Q    Link    VC       File
        // A0    A0       A0      I      A0      m0       _L0_m0.txt
        // A1    A1       A2      I      A0      m2       _L0_m2.txt
        // B0    B0       B0      Q      B0      m1       _L2_m1.txt
        // B1    B1       B2      Q      B0      m3       _L2_m3.txt

        /* FDDC-I paths (Side A) */
        {ADI_APOLLO_RXMUX_CBOUT_0, ADI_APOLLO_FDDC_A0, ADI_APOLLO_DFORMAT_FDDC_SMON_I_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_1, ADI_APOLLO_FDDC_A2, ADI_APOLLO_DFORMAT_FDDC_SMON_I_PATH},

        /* FDDC-Q paths (Side B) */
        {ADI_APOLLO_RXMUX_CBOUT_0, ADI_APOLLO_FDDC_B0, ADI_APOLLO_DFORMAT_FDDC_SMON_Q_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_1, ADI_APOLLO_FDDC_B2, ADI_APOLLO_DFORMAT_FDDC_SMON_Q_PATH},
    };

    adi_apollo_dformat_smon_fd_map_t smon_fddc_8t8r_map[] = {
        // 8T8R - id81_uc28_smon (JTx m=8)
        // Assume 1-to-1 ADC-to-CBout RxMux1 mapping
        // Assume JTX FDDC-to-VC mapping [0, 2, 4, 6, 8, A, C, E] => [A0-I, A0-Q, A1-I, A1-Q, A2-I, A2-Q, A3-I, A3-Q]
        // ADC   CBout    FDDC    I/Q    Link    VC       File
        // A0    0        A0      I      A0      m0       _L0_m0.txt
        // A1    1        A1      I      A0      m2       _L0_m2.txt
        // A2    2        A2      I      A0      m4       _L0_m4.txt
        // A3    3        A3      I      A0      m6       _L0_m6.txt
        // B0    0        B0      Q      B0      m1       _L2_m1.txt
        // B1    1        B1      Q      B0      m3       _L2_m3.txt
        // B2    2        B2      Q      B0      m5       _L2_m5.txt
        // B3    3        B3      Q      B0      m7       _L2_m7.txt

        /* FDDC-I paths (Side A) */
        {ADI_APOLLO_RXMUX_CBOUT_0, ADI_APOLLO_FDDC_A0, ADI_APOLLO_DFORMAT_FDDC_SMON_I_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_1, ADI_APOLLO_FDDC_A1, ADI_APOLLO_DFORMAT_FDDC_SMON_I_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_2, ADI_APOLLO_FDDC_A2, ADI_APOLLO_DFORMAT_FDDC_SMON_I_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_3, ADI_APOLLO_FDDC_A3, ADI_APOLLO_DFORMAT_FDDC_SMON_I_PATH},

        /* FDDC-Q paths (Side B) */
        {ADI_APOLLO_RXMUX_CBOUT_0, ADI_APOLLO_FDDC_B0, ADI_APOLLO_DFORMAT_FDDC_SMON_Q_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_1, ADI_APOLLO_FDDC_B1, ADI_APOLLO_DFORMAT_FDDC_SMON_Q_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_2, ADI_APOLLO_FDDC_B2, ADI_APOLLO_DFORMAT_FDDC_SMON_Q_PATH},
        {ADI_APOLLO_RXMUX_CBOUT_3, ADI_APOLLO_FDDC_B3, ADI_APOLLO_DFORMAT_FDDC_SMON_Q_PATH},
    };
    
    // If the profile doesn't enable SMON (i.e. peak_en = false) then configure SMON programmatically, less the JESD framer option.
    // The SMON framer requires proper JESD link configuration (e.g. CS=1, N=15) which is done by FW during profile loading.
    for (i = 0; i < smons_len; i++) {
        side = (smons[i] >> ADI_APOLLO_SMON_PER_SIDE_NUM) == 0 ? 0 : 1;
        idx = adi_api_utils_select_lsb_get(smons[i]) % ADI_APOLLO_SMON_PER_SIDE_NUM;

        if (profile->rx_path[side].rx_smon[idx].peak_en) {
            // configured from profile
            (smons_blk_en + i)->reg_en = profile->rx_path[side].rx_smon[idx].peak_en;
            (smons_blk_en + i)->jesd_en = profile->rx_path[side].rx_smon[idx].sframer_en;
            (smons_blk_en + i)->gpio_en = profile->rx_path[side].rx_smon[idx].gpio_en;

            map_fddc = true;
        } else {
            // configure programmatically
            (smons_blk_en + i)->reg_en = true;
            (smons_blk_en + i)->jesd_en = false;
            (smons_blk_en + i)->gpio_en = true;

            err = adi_apollo_smon_pgm(device, smons[i], &smon_pgm);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    // Map the smon ctrl bits to FDDCs if any SMON framer is enabled
    if (map_fddc) {
        /* Select SMON for JESD control bit #2 for all links */
        err = adi_apollo_dformat_ctrl_bit_sel_set(device, ADI_APOLLO_LINK_ALL, ADI_APOLLO_DFOR_CTRL_LOW, ADI_APOLLO_DFOR_CTRL_LOW, ADI_APOLLO_DFOR_CTRL_SMON);
        ADI_CMS_ERROR_RETURN(err);

        if (device->dev_info.is_8t8r) {
            err = adi_apollo_dformat_smon_fd_fddc_set(device, smon_fddc_8t8r_map, ADI_UTILS_ARRAY_SIZE(smon_fddc_8t8r_map));
            ADI_CMS_ERROR_RETURN(err);
        } else {
            err = adi_apollo_dformat_smon_fd_fddc_set(device, smon_fddc_4t4r_map, ADI_UTILS_ARRAY_SIZE(smon_fddc_4t4r_map));
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_gpios_configure(adi_apollo_device_t *device)
{
    int32_t err;

    err = adi_apollo_gpio_quick_config_mode_set(device, ADI_APOLLO_QUICK_CFG_DISABLE);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[0][0], ADI_APOLLO_FUNC_SMON_A_0); // SMON-A0
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[0][1], ADI_APOLLO_FUNC_SMON_A_1); //
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[1][0], ADI_APOLLO_FUNC_SMON_A_4); // SMON-A1
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[1][1], ADI_APOLLO_FUNC_SMON_A_5); //
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[2][0], ADI_APOLLO_FUNC_SMON_B_0); // SMON-B0
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[2][1], ADI_APOLLO_FUNC_SMON_B_1); //
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[3][0], ADI_APOLLO_FUNC_SMON_B_4); // SMON-B1
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[3][1], ADI_APOLLO_FUNC_SMON_B_5); //
    ADI_CMS_ERROR_RETURN(err);

    if (device->dev_info.is_8t8r) {
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[4][0], ADI_APOLLO_FUNC_SMON_A_2); // SMON-A2
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[4][1], ADI_APOLLO_FUNC_SMON_A_3); //
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[5][0], ADI_APOLLO_FUNC_SMON_A_6); // SMON-A3
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[5][1], ADI_APOLLO_FUNC_SMON_A_7); //
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[6][0], ADI_APOLLO_FUNC_SMON_B_2); // SMON-B2
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[6][1], ADI_APOLLO_FUNC_SMON_B_3); //
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[7][0], ADI_APOLLO_FUNC_SMON_B_6); // SMON-B3
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_gpio_cmos_func_mode_set(device, gpio_num_to_smon_map[7][1], ADI_APOLLO_FUNC_SMON_B_7); //
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_inspect(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], int smons_len)
{
    int32_t err;
    uint16_t i;
    adi_apollo_smon_inspect_t smon_inspect;

    printf("\nSMON HW state inspect\n");
    for (i = 0; i < smons_len; i++) {
        err = adi_apollo_smon_inspect(device, smons[i], &smon_inspect);
        ADI_CMS_ERROR_RETURN(err);

        printf("SMON [0x%02x]  period: %-7d", smons[i], smon_inspect.dp_cfg.period);
        printf("  tlow: 0x%04x, %5.1f dBFS  thigh: 0x%04x, %5.1f dBFS",
               smon_inspect.dp_cfg.thresh_low, PK_VAL_TO_DBFS(smon_inspect.dp_cfg.thresh_low),
               smon_inspect.dp_cfg.thresh_high, PK_VAL_TO_DBFS(smon_inspect.dp_cfg.thresh_high));
        printf("  pk en: %1d  gpio en: %1d  sframe en: %1d  sframe: %s\n",
               smon_inspect.dp_cfg.peak_en, smon_inspect.dp_cfg.gpio_en, smon_inspect.dp_cfg.sframer_en,
               smon_inspect.dp_cfg.sframer_mode == (adi_apollo_rx_smon_framer_mode_e)ADI_APOLLO_SMON_SFRAMER_FIVE_BIT ? "5-bit" : "10-bit");
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}

int32_t smon_hi_lo_thresh_set(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], double input_ofst_db[], int smons_len, int iteration)
{
    int32_t err;
    uint16_t i;
    double ltdb;
    double htdb;

    for (i = 0; i < smons_len; i++) {

        if (!smons_blk_en[i].gpio_en) {
            continue;
        }

        ltdb = input_ofst_db[i] - 1.5;
        htdb = input_ofst_db[i] + 1.5;

        if (iteration % 3 == 0) {
            err = adi_apollo_smon_thresh_set(device, smons[i], DBFS_TO_PK_VAL(ltdb + 3.0), DBFS_TO_PK_VAL(htdb + 3.0)); // LOW
        } else if (iteration % 3 == 1) {
            err = adi_apollo_smon_thresh_set(device, smons[i], DBFS_TO_PK_VAL(ltdb + 0.0), DBFS_TO_PK_VAL(htdb + 0.0)); // MID
        } else {
            err = adi_apollo_smon_thresh_set(device, smons[i], DBFS_TO_PK_VAL(ltdb - 3.0), DBFS_TO_PK_VAL(htdb - 3.0)); // HIGH
        }
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_gpio_validate(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t pk_val_gpio_results[], int smons_len, int iteration)
{
    uint16_t i;
    char str_buff[256];

    for (i = 0; i < smons_len; i++) {

        if (!smons_blk_en[i].gpio_en) {
            continue;
        }

        if (iteration % 3 == 0) { // LOW
            if (pk_val_gpio_results[i] != 0x00) {
                sprintf(str_buff, "SMON [0x%02x] GPIO expected 0x%02x but was 0x%02x", smons[i], 0x00, pk_val_gpio_results[i]);
                EXCTL_FAIL(str_buff);
                ADI_CMS_ERROR_RETURN(API_CMS_ERROR_UNEXPECTED_RESULT);
            }
        } else if (iteration % 3 == 1) { // MID
            if (pk_val_gpio_results[i] != 0x01) {
                sprintf(str_buff, "SMON [0x%02x] GPIO expected 0x%02x but was 0x%02x", smons[i], 0x01, pk_val_gpio_results[i]);
                EXCTL_FAIL(str_buff);
                ADI_CMS_ERROR_RETURN(API_CMS_ERROR_UNEXPECTED_RESULT);
            }
        } else { // HIGH
            if (pk_val_gpio_results[i] != 0x03) {
                sprintf(str_buff, "SMON [0x%02x] GPIO expected 0x%02x but was 0x%02x", smons[i], 0x03, pk_val_gpio_results[i]);
                EXCTL_FAIL(str_buff);
                ADI_CMS_ERROR_RETURN(API_CMS_ERROR_UNEXPECTED_RESULT);
            }
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_jesd_validate(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], smon_blk_en_t smons_blk_en[], uint16_t pk_val_reg_results[], uint16_t pk_val_jesd_results[], int smons_len)
{
    uint16_t i;
    char str_buff[256];
    double tolerance = 1.0;

    for (i = 0; i < smons_len; i++) {
        // Validate the REG vs JESD serialized values
        if (smons_blk_en[i].jesd_en) {
            double delta = fabs(PK_VAL_TO_DBFS(pk_val_reg_results[i]) - PK_VAL_TO_DBFS(pk_val_jesd_results[i]));
            printf("Delta: %6.3f   %6.1f, %6.1f\n", delta, PK_VAL_TO_DBFS(pk_val_reg_results[i]), PK_VAL_TO_DBFS(pk_val_jesd_results[i]));
            if (!DOUBLES_EQUAL_TOL(PK_VAL_TO_DBFS(pk_val_reg_results[i]), PK_VAL_TO_DBFS(pk_val_jesd_results[i]), 1.0)) {
                sprintf(str_buff, "SMON [0x%02x] peak val from REG %6.1f differs from JESD %6.1f by more than %4.1fdBFS",
                        smons[i], PK_VAL_TO_DBFS(pk_val_reg_results[i]), PK_VAL_TO_DBFS(pk_val_jesd_results[i]), tolerance);
                EXCTL_FAIL(str_buff);
                ADI_CMS_ERROR_RETURN(API_CMS_ERROR_UNEXPECTED_RESULT);
            }
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t smon_input_offset_set(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], double input_ofst_db[], int smons_len, double *avg_ofst)
{
    int32_t err;
    uint16_t i;
    uint16_t input_ofst;
    double avg_ofst_sum = 0.0;
    
    *avg_ofst = 0;
    for (i = 0; i < smons_len; i++) {
        err = adi_apollo_smon_peak_val_get(device, smons[i], &input_ofst);
        ADI_CMS_ERROR_RETURN(err);
        
        input_ofst_db[i] = PK_VAL_TO_DBFS(input_ofst);
        avg_ofst_sum += input_ofst_db[i];
        
        printf("Input level: %6.1fdBFS\n", input_ofst_db[i]);

        ADI_APOLLO_LOG_MSG_VAR("SMON ADC input: [0x%02x] %6.1fdBFS", smons[i], input_ofst_db[i]);
    }

    *avg_ofst = avg_ofst_sum / (double)smons_len;

    ADI_APOLLO_LOG_MSG_VAR("SMON ADC input ave: %6.1fdBFS", *avg_ofst);

    return API_CMS_ERROR_OK;
}

int32_t adc_input_level_set(adi_apollo_device_t *device, adi_apollo_blk_sel_t smons[], double input_ofst_db[], int smons_len, 
                            double target_input_db, double rf_src_level_db, bool interactive)
{
    int32_t err;
    double input_ofst_avg_db;

    /* Take initial SMON peak average */
    err = smon_input_offset_set(device, smons, input_ofst_db, smons_len, &input_ofst_avg_db);
    ADI_CMS_ERROR_RETURN(err);

    /* Adjust RF source for target level */
    EXCTL_SIGGEN_LEVEL(rf_src_level_db + (target_input_db - input_ofst_avg_db), interactive);

    adi_apollo_hal_delay_us(device, 1000000 * .5);

    ADI_APOLLO_LOG_MSG_VAR("Delaying 0.5s for RF SIGGEN to settle", "");

    /* Save adjusted offsets for each SMON */
    err = smon_input_offset_set(device, smons, input_ofst_db, smons_len, &input_ofst_avg_db);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

char *status_to_string(const adi_apollo_smon_read_t *result, bool en, char *str)
{
    uint16_t pk_val;
    
    if (en) {
        pk_val = ((result->status & 0x000ffe00) >> 9u);   /* Extract 12-bit pk val */
        sprintf(str, "%3d 0x%04x", result->status_fcnt, pk_val);
    } else {
        sprintf(str, "n/a");
    }

    return str;
}

char *result_to_string(uint16_t result, bool en, char *str)
{
    if (en) {
        sprintf(str, "0x%04x %5.1f dBFS", result, PK_VAL_TO_DBFS(result));
    } else {
        sprintf(str, "not enabled");
    }

    return str;
}

char *gpio_to_string(adi_apollo_device_t *device, adi_apollo_blk_sel_t smon, uint16_t result, bool en, char *str)
{
    int32_t err;
    uint16_t lt;
    uint16_t ht;

    if (en) {
        err = adi_apollo_smon_thresh_get(device, smon, &lt, &ht);
        if (err != API_CMS_ERROR_OK) {
            return "error";
        }

        sprintf(str, "0x%02x %3s %-3s [L:%5.1f H:%5.1f]",
                result,
                (result == 0) ? "LOW" : (result == 1) ? "MID"
                                    : (result == 3)   ? "HI "
                                                      : "?",
                (result == 0) ? "*" : (result == 1) ? "**"
                                  : (result == 3)   ? "***"
                                                    : "?",
                PK_VAL_TO_DBFS(lt), PK_VAL_TO_DBFS(ht));
    } else {
        sprintf(str, "not enabled");
    }

    return str;
}
#endif /* !defined(VERSAL_PLATFORM) */
