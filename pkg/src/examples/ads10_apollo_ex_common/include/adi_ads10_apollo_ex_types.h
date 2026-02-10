#if defined(__linux__)

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

typedef int32_t(*adi_apollo_ex_test_t)(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);

/*!
* \brief Structure containing core data path info
*/
typedef struct {
    double fclk;
    double adc_sample_rate;
    double dac_sample_rate;
    uint32_t cdrc;
    uint32_t fdrc;
    uint32_t total_drc;
    uint32_t coarse_ftw;
    uint64_t fine_ftw;
    double cnco_freq;
    double fnco_freq;
    double fdata;

    bool fsrc_en;
    double fsrc_ratio;
    double fsrc_data_rate;
    int fsrc_n;
    int fsrc_m;
    uint8_t fsrc_1x_en;

    bool cfir_en;
    bool pfilt_en;

    adi_apollo_terminal_e terminal;

} adi_ads10_apollo_dp_info_t;

typedef enum {
    ADI_ADS10_APOLLO_CAL_NONE           = 0x00,             /*!< Run no cals */
    ADI_ADS10_APOLLO_CAL_CC             = 0x01,             /*!< Run clock conditioning cal */
    ADI_ADS10_APOLLO_CAL_ADC            = 0x02,             /*!< Run ADC Init (FG) cal */
    ADI_ADS10_APOLLO_CAL_ADC_BG         = 0x04,             /*!< Unfreeze ADC BG cal */
    ADI_ADS10_APOLLO_CAL_ADC_BG_10S     = 0x08,             /*!< Unfreeze ADC BG cal, soak for 10secs */
    ADI_ADS10_APOLLO_CAL_ONESHOT        = 0x10,             /*!< Sync clocks (one-shot-sync) */
    ADI_ADS10_APOLLO_CAL_SERDES         = 0x20,             /*!< Run SERDES JRx Init (FG) cal, lane rate > 8.0Gbps */
    ADI_ADS10_APOLLO_CAL_SERDES_BG      = 0x40,             /*!< Run SERDES JRx Track (BG) cal, lane rate > 16.0Gbps */
} adi_ads10_apollo_cal_e;

typedef enum {
    ADI_ADS10_APOLLO_CLK_SRC_EXTERNAL,
    ADI_ADS10_APOLLO_CLK_SRC_ADF4382
} adi_ads10_apollo_clk_src_e;

/* Struct defining Tx DR table entries */
typedef struct {
    char *name;
    uint32_t fsrc_n;                /* FSRC N val from cfg */
    uint32_t fsrc_m;                /* FSRC M val from cfg */
    uint16_t cdrc_ratio;            /* CDRC bit select */
    uint16_t fdrc_ratio;            /* FDRC bit select */
    double  cnco_freq;              /* CNCO freq (MHz) */
    double  fnco_freq;              /* FNCO freq (MHz) - adjusted for cdrc changes (fnco clk is cduc/adc sample rate) */

    /* These fields will be calculated */
    double  fdata;                  /* Effective baseband FSRC data rate (MHz) */
    double  fsrc_ratio;             /* Calculated FSRC ratio N/M (1.0 > N/M < 2.0) */
    uint32_t cdrc_ratio_val;        /* CDRC ratio numeric val */
    uint32_t fdrc_ratio_val;        /* FDRC ratio numeric val */
    uint32_t link_xdrc;             /* Total ratio numeric val for a given link (CDRC * FDRC) */
    uint16_t base_xdrc;             /* Original total drc */
    uint8_t mode_1x;                /* if fsrc_n == fsrc_m then use FSRC 1x mode*/

} adi_ads10_apollo_fsrc_dr_cfg_t;

typedef struct {
    char *name;
    uint16_t cdrc_ratio;            /* CDRC bit select */
    uint16_t fdrc_ratio;            /* FDRC bit select */
    double  cnco_freq;              /* CNCO freq (MHz) */
    double  fnco_freq;              /* FNCO freq (MHz) - adjusted for cdrc changes (fnco clk is cduc/adc sample rate) */

    /* These fields will be calculated */
    double  fdata;                  /* Effective baseband FSRC data rate (MHz) */
    uint32_t cdrc_ratio_val;        /* CDRC ratio numeric val */
    uint32_t fdrc_ratio_val;        /* FDRC ratio numeric val */
    uint32_t link_xdrc;             /* Total ratio numeric val for a given link (CDRC * FDRC) */
    uint16_t base_xdrc;             /* Original total drc */
} adi_ads10_apollo_sr_dr_cfg_t;
#endif /* __ADI_ADS10_APOLLO_COMMON_EX_TYPES_H__ */

#endif /* defined(__linux__) */
