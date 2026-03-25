#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples common types
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADS10_APOLLO_COMMON_EX_TYPES_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_TYPES_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/
#define FW_IMAGES_DIR           "../../examples/fw_images/b0/"

#define INDIRECT_REG_TEST_ADDR  (0x60366045)
#define ARM_REG_TEST_BASE_ADDR  (0x20000000U)

#define OUTPUT_DIR		                    "/home/analog/Apollo"
#define MAX_PATH_LEN	                    256
#define CAL_DATA_FILE_PATH                  OUTPUT_DIR"/calData"                /* Location where calData files will be written to and read from. */
#define BMEM_VEC_FILE_PATH                  OUTPUT_DIR"/bmem_vec/"              /* Location where BMEM debug vector file is stored */
#define BMEM_VEC_FILE                       BMEM_VEC_FILE_PATH"debug_bmem_vec.txt"
#define JRX_EYE_SWEEP_FILE_PATH OUTPUT_DIR  "/jrx_eye_sweep"                    /* Location where SERDES JRX Eye Sweep files will be written. */
#define FPGA_IMAGE_FILE_PATH                "/home/analog/platform/binaries"    /* Location of FPGA image files */
#define DEFAULT_FPGA_IMAGE_FILE             "ad9084_ads10v1_smapx8.bin"         /* Default FPGA image file name*/

#ifdef ADS10
    #define DEFAULT_NUM_SAMPLES_H                 1024*64
    #define DEFAULT_NUM_SAMPLES_L                 1024*16
#endif /* ADS10 */
#ifdef VCU128
    #define DEFAULT_NUM_SAMPLES_H                 1024*8
    #define DEFAULT_NUM_SAMPLES_L                 1024*2
#endif /* VCU128 */

#define ADI_ADS10_CHECK_ARG_MATCH(found)    if (!(found)) return API_CMS_ERROR_ERROR

#define DOUBLES_EQUAL_TOL(f1, f2, tol) (fabs((f1) - (f2)) <= (tol))         /* Compare two doubles for equality within a tolerance, return true if equal */

/*!
 * \typedef adi_apollo_ex_test_t
 * \brief Function pointer type for Apollo example test functions.
 */
typedef int32_t(*adi_apollo_ex_test_t)(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);

/*!
 * \struct adi_ads10_apollo_dp_info_t
 * \brief Structure containing core data path information.
 *
 * This structure holds various parameters describing the data path configuration,
 * including clock rates, decimation ratios, frequency tuning words, and filter enables.
 */
typedef struct {
    double fclk;                   /*!< Device clock frequency */
    double adc_sample_rate;        /*!< ADC sample rate */
    double dac_sample_rate;        /*!< DAC sample rate */
    uint32_t cdrc;                 /*!< Coarse decimation ratio */
    uint32_t fdrc;                 /*!< Fine decimation ratio */
    uint32_t total_drc;            /*!< Total decimation ratio */
    uint32_t coarse_ftw;           /*!< Coarse frequency tuning word */
    uint64_t fine_ftw;             /*!< Fine frequency tuning word */
    double cnco_freq;              /*!< Coarse NCO frequency */
    double fnco_freq;              /*!< Fine NCO frequency */
    double fdata;                  /*!< Data rate after all processing */

    bool fsrc_en;                  /*!< FSRC enable flag */
    double fsrc_ratio;             /*!< FSRC ratio */
    double fsrc_data_rate;         /*!< FSRC data rate */
    int fsrc_n;                    /*!< FSRC N value */
    int fsrc_m;                    /*!< FSRC M value */
    uint8_t fsrc_1x_en;            /*!< FSRC 1x mode enable */

    bool cfir_en;                  /*!< CFIR filter enable flag */
    bool pfilt_en;                 /*!< PFILT filter enable flag */

    adi_apollo_terminal_e terminal;/*!< Terminal type */
} adi_ads10_apollo_dp_info_t;

/*!
 * \enum adi_ads10_apollo_cal_e
 * \brief Enumeration of calibration options for Apollo.
 */
typedef enum {
    ADI_ADS10_APOLLO_CAL_NONE           = 0x00, /*!< Run no calibrations */
    ADI_ADS10_APOLLO_CAL_CC             = 0x01, /*!< Run clock conditioning calibration */
    ADI_ADS10_APOLLO_CAL_ADC            = 0x02, /*!< Run ADC Init (FG) calibration */
    ADI_ADS10_APOLLO_CAL_ADC_BG         = 0x04, /*!< Unfreeze ADC BG calibration */
    ADI_ADS10_APOLLO_CAL_ADC_BG_10S     = 0x08, /*!< Unfreeze ADC BG calibration, soak for 10 seconds */
    ADI_ADS10_APOLLO_CAL_RXTX_SYNC      = 0x10, /*!< Sync clocks including JTx/JRx SERDES  */
    ADI_ADS10_APOLLO_CAL_SERDES         = 0x20, /*!< Run SERDES JRx Init (FG) calibration, lane rate > 8.0Gbps */
    ADI_ADS10_APOLLO_CAL_SERDES_BG      = 0x40, /*!< Run SERDES JRx Track (BG) calibration, lane rate > 16.0Gbps */
} adi_ads10_apollo_cal_e;

/*!
 * \enum adi_ads10_apollo_clk_src_e
 * \brief Enumeration of Apollo clock source options.
 */
typedef enum {
    ADI_ADS10_APOLLO_CLK_SRC_EXTERNAL,   /*!< External clock source */
    ADI_ADS10_APOLLO_CLK_SRC_ADF4382     /*!< ADF4382 clock source */
} adi_ads10_apollo_clk_src_e;

/*!
 * \brief Enumerates IQ pair capture selection
 *
 * This enumeration defines the possible selections for capturing IQ (In-phase and Quadrature) data pairs in the Apollo device.
 * Each value represents a specific IQ pair or a combination of pairs that can be selected for data capture operations.
 */
typedef enum {
    ADI_APOLLO_CAPTURE_IQ_PAIR_NONE         = 0x00,   /*!< No IQ pair selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_A0           = 0x01,   /*!< Side A IQ pair 0 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_A1           = 0x02,   /*!< Side A IQ pair 1 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_A2           = 0x04,   /*!< Side A IQ pair 2 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_A3           = 0x08,   /*!< Side A IQ pair 3 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_B0           = 0x10,   /*!< Side B IQ pair 0 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_B1           = 0x20,   /*!< Side B IQ pair 1 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_B2           = 0x40,   /*!< Side B IQ pair 2 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_B3           = 0x80,   /*!< Side B IQ pair 3 selected */
    ADI_APOLLO_CAPTURE_IQ_PAIR_4T4R_ALL     = 0x33,   /*!< All IQ pairs selected for 4t4r */
    ADI_APOLLO_CAPTURE_IQ_PAIR_8T8R_ALL     = 0xFF    /*!< All IQ pairs selected for 8t8r */
} adi_apollo_capture_iq_pair_select_e;

/*!
 * \struct adi_ads10_apollo_fsrc_dr_cfg_t
 * \brief Structure defining Tx Data Rate (DR) table entries for FSRC.
 *
 * This structure contains configuration and calculated values for FSRC data rate settings.
 */
typedef struct {
    char *name;                 /*!< Configuration name */
    uint32_t fsrc_n;            /*!< FSRC N value from configuration */
    uint32_t fsrc_m;            /*!< FSRC M value from configuration */
    uint16_t cdrc_ratio;        /*!< CDRC bit select */
    uint16_t fdrc_ratio;        /*!< FDRC bit select */
    double  cnco_freq;          /*!< CNCO frequency (MHz) */
    double  fnco_freq;          /*!< FNCO frequency (MHz), adjusted for cdrc changes (fnco clk is cduc/adc sample rate)*/

    /* Calculated fields */
    double  fdata;              /*!< Effective baseband FSRC data rate (MHz) */
    double  fsrc_ratio;         /*!< Calculated FSRC ratio N/M (1.0 > N/M < 2.0) */
    uint32_t cdrc_ratio_val;    /*!< CDRC ratio numeric value */
    uint32_t fdrc_ratio_val;    /*!< FDRC ratio numeric value */
    uint32_t link_xdrc;         /*!< Total ratio numeric value for a given link (CDRC * FDRC) */
    uint16_t base_xdrc;         /*!< Original total DRC */
    uint8_t mode_1x;            /*!< Use FSRC 1x mode if fsrc_n == fsrc_m */
} adi_ads10_apollo_fsrc_dr_cfg_t;

/*!
 * \struct adi_ads10_apollo_sr_dr_cfg_t
 * \brief Structure defining Tx Data Rate (DR) table entries for SR.
 *
 * This structure contains configuration and calculated values for SR data rate settings.
 */
typedef struct {
    char *name;                 /*!< Configuration name */
    uint16_t cdrc_ratio;        /*!< CDRC bit select */
    uint16_t fdrc_ratio;        /*!< FDRC bit select */
    double  cnco_freq;          /*!< CNCO frequency (MHz) */
    double  fnco_freq;          /*!< FNCO frequency (MHz), adjusted for cdrc changes (fnco clk is cduc/adc sample rate) */

    /* Calculated fields */
    double  fdata;              /*!< Effective baseband FSRC data rate (MHz) */
    uint32_t cdrc_ratio_val;    /*!< CDRC ratio numeric value */
    uint32_t fdrc_ratio_val;    /*!< FDRC ratio numeric value */
    uint32_t link_xdrc;         /*!< Total ratio numeric value for a given link (CDRC * FDRC) */
    uint16_t base_xdrc;         /*!< Original total DRC */
} adi_ads10_apollo_sr_dr_cfg_t;

/*!
 * \struct adi_ads10_apollo_filter_param_t
 * \brief Structure for filter test parameters.
 *
 * This structure defines expected and tolerance values for filter attenuation and frequency.
 */
typedef struct {
    uint8_t     channels;           /*!< Number of channels with the same filter. Recommended to use \ref adi_apollo_jesd_link_select_e to select desired ones. */
    double      atten_expect;       /*!< Expected attenuation value */
    double      atten_tolerance;    /*!< Allowed tolerance for attenuation measurement */
    double      freq_tolerance;     /*!< Tolerance for frequency deviation in Hz */
} adi_ads10_apollo_filter_param_t;

/*!
 * \struct adi_ads10_apollo_filter_test_t
 * \brief Structure for filter test configuration.
 *
 * This structure defines a test tone ratio and associated filter parameters.
 */
typedef struct {
    double tone_ratio;                          /*!< Ratio of the tone frequency to the sampling frequency */
    adi_ads10_apollo_filter_param_t filter;     /*!< Filter parameters used for testing */
} adi_ads10_apollo_filter_test_t;

#endif /* __ADI_ADS10_APOLLO_EX_TYPES_H__ */

#endif /* !defined(VERSAL_PLATFORM) */
