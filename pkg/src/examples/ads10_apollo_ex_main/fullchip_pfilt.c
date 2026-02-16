#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo Fullchip PFILT example code
 *
 * Demonstrates various PFILT configurations for Rx and Tx paths
 *
 * This examples uses device profiles
 *  
 *  id98_uc05       (8T8R Fadc/Fdac= 8G, 10.0000Gbps JESD,  125Msps data rate)
 *  id00_uc06       (4T4R Fadc/Fdac= 20G 10.3125Gbps JESD, 1250Msps data rate)
 *
 * Setup Configuration (4T4R)
 * ==========================
 *   Device Profile: id00_uc06
 *   Fclk:      20000MHz
 *   FPGA Ref:  156.25MHz
 *   ADC Inputs
 *      Side A: ADC-A0 & ADC-A1
 *      Side B: ADC-B0 & ADC-B1
 *
 *  SigGen #1 -> split to A0/A1
 *  SigGen #2 -> split to B0/B1
 *
 *
 * Setup Configuration (8T8R)
 * ==========================
 *   Device Profile: id00_uc02_8G
 *   Fclk:      8000MHz
 *   FPGA Ref:  250MHz
 *   ADC Inputs
 *      Side A: ADC-A0 & ADC-A2
 *      Side B: ADC-B1 & ADC-B3
 * 
 *  SigGen #1 -> split to A0/A2
 *  SigGen #2 -> split to B0/B2
 *  Loopback DAC-A1 -> ADC-A1
 *  Loopback DAC-A3 -> ADC-A3
 *  Loopback DAC-B1 -> ADC-B1
 *  Loopback DAC-B3 -> ADC-B3
 *
 *
 * Capture file names reflect the filter mode and type. For example, this capture file
 * contains interleaved I/Q capture data from a filter with both streams bypassed.
 * 
 *  fullchip_pfilt_bypass_bypass_f00_L0_IQ0_NP16_CFXXXXXX_DRXXXXX.txt
 *  -------------- ------ ------ --- -- -------------- Complex data interleaved
 *  |              |      |      |   |   |------------ Virtual conv pair (e.g. m0/m1, m2/m3, ...)
 *  |              |      |      |   |---------------- Link idx
 *  |              |      |      |-------------------- Freq #0 if sweeping
 *  |              |      |--------------------------- Stream 1 filter mode
 *  |              |---------------------------------- Stream 0 filter mode
 *  |------------------------------------------------- Example name
 *
 *  fullchip_pfilt_realn2_hcplx_pos0p5_pos0p5_f00_L0_IQ0_NP16_DRXXXXXXXXXX_CFXXXXXXXXXX.txt
 *  -------------- ------ ----- --- -- ------------- ------------- Complex data interleaved
 *  |              |      |     |      |      |   |   |----------- Virtual conv pair (e.g. m0/m1, m2/m3, ...)
 *  |              |      |     |      |      |   |--------------- Link idx
 *  |              |      |     |      |      |------------------- Freq #0 if sweeping
 *  |              |      |     |      |-------------------------- Stream 1 coeffs are 0.5x multiply
 *  |              |      |     |--------------------------------- Stream 0 coeffs are 0.5x multiply
 *  |              |      |--------------------------------------- Stream 1 Half-complex (delay stream)
 *  |              |---------------------------------------------- Stream 0 Real N/2 mode
 *  |------------------------------------------------------------- Example name
 * 
 * File-ADC associations for this example using id98_uc05 device profile (8T8R):
 *  A0/A1 => ADC-A0      B0/B1 => ADC-B0
 *  A2/A3 => ADC-A2      B2/B3 => ADC-B2
 *  A4/A5 => ADC-A1      B4/B5 => ADC-B1
 *  A6/A7 => ADC-A3      B6/B7 => ADC-B3
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
#include "adi_apollo_gpio.h"
#include "adi_apollo_gpio_hop.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_ctl.h"

/*!
 * \brief Fullchip Hop execution container
 */
typedef struct
{
    adi_apollo_device_t *device;
    adi_fpga_apollo_device_t *fpga_device;
    adi_apollo_top_t *profile;
    bool is_interactive;
    uint8_t pfilt_all;
    uint8_t cnco_all;
    uint16_t fnco_all;
    adi_ads10_apollo_dp_info_t rx_dp_info;
    adi_ads10_apollo_dp_info_t tx_dp_info;
    double baseband_tone_ratio;    // Sets the baseband freq as a fraction of the data rate

} fullchip_pfilt_exec_t;


static fullchip_pfilt_exec_t *fullchip_pfilt_exec_obj_create(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, bool interactive);
static int32_t capture(const fullchip_pfilt_exec_t *obj, char *cap_fname_base, bool bdir_start);
static int32_t play(const fullchip_pfilt_exec_t *obj, double tone_ratio, bool bdir_start);
static int32_t tx_dp_reset(adi_apollo_device_t *device, bool do_oneshot_sync);
static int32_t set_cnco(const fullchip_pfilt_exec_t *obj, double tone_ratio);
static int32_t sweep(const fullchip_pfilt_exec_t *obj, char *cap_fname_base);
static int32_t fullchip_pfilt_init(fullchip_pfilt_exec_t *obj);
static int32_t pfilt_real_ntap(const fullchip_pfilt_exec_t *obj,
                               uint8_t pfilts, uint16_t streams,
                               adi_apollo_pfilt_mode_e mode,
                               int16_t coeffs[], uint32_t len);
static int32_t pfilt_half_complex(const fullchip_pfilt_exec_t *obj,
                                 uint8_t pfilts,
                                 adi_apollo_pfilt_stream_sel_e filt_stream, adi_apollo_pfilt_stream_sel_e dly_stream,
                                 uint8_t delay,
                                 int16_t coeffs0[], int16_t coeffs1[], uint32_t len);
static int32_t pfilt_full_matrix(const fullchip_pfilt_exec_t *obj, uint8_t pfilts,
                         int16_t coeffs0[], int16_t coeffs1[], int16_t coeffs2[], int16_t coeffs3[], uint32_t len);


/* Filter coefficients */
__maybe_unused static int16_t n32_neg6db[32] = {32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n16_pos1x[16] = {32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n16_neg1x[16] = {-32767, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n16_neg6db[16] = {32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n16_neg0p5[16] = {-32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n16_pos0p5[16] = {32767 / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n16_neg12db[16] = {32767 / 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n8_neg6db[8] = {32767 / 2, 0, 0, 0, 0, 0, 0, 0};
__maybe_unused static int16_t n8_neg12db[8] = {32767 / 4, 0, 0, 0, 0, 0, 0, 0};

int32_t fullchip_pfilt(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive

    printf("fullchip_pfilt\n");

    /* Create a utility container obj*/
    fullchip_pfilt_exec_t *obj = fullchip_pfilt_exec_obj_create(device, fpga_device, profile, interactive);
    
    /* PFILT initialization. */
    err = fullchip_pfilt_init(obj);
	ADI_CMS_ERROR_RETURN(err);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
	ADI_CMS_ERROR_RETURN(err);

    err = play(obj, obj->baseband_tone_ratio, true);
	ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG | ADI_ADS10_APOLLO_CAL_ADC_BG);
	ADI_CMS_ERROR_RETURN(err);

    /*
     * Start with PFILT bypassed, take initial sweep to get ref levels
    */
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_bypass_bypass");
	ADI_CMS_ERROR_RETURN(err);

    /*
     * REAL N (32-taps)
    */
    err = pfilt_real_ntap(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_0, ADI_APOLLO_PFILT_MODE_N_REAL, n32_neg6db, 32);
	ADI_CMS_ERROR_RETURN(err);
    err = pfilt_real_ntap(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_1, ADI_APOLLO_PFILT_MODE_DISABLED, NULL, 0);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_realn_bypass_neg6db_0db");
	ADI_CMS_ERROR_RETURN(err);

    /*
     * REAL N/2 (16-taps)
    */
    err = pfilt_real_ntap(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_0, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL, n16_neg6db, 16);
	ADI_CMS_ERROR_RETURN(err);
    err = pfilt_real_ntap(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_1, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL, n16_neg12db, 16);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_realn2_realn2_neg6db_neg12db");
	ADI_CMS_ERROR_RETURN(err);
    
    /*
     * REAL N/4 (8-taps)
    */
    err = pfilt_real_ntap(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_0, ADI_APOLLO_PFILT_MODE_N_DIV_BY_4_REAL, n8_neg6db, 8);
	ADI_CMS_ERROR_RETURN(err);
    err = pfilt_real_ntap(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_1, ADI_APOLLO_PFILT_MODE_N_DIV_BY_4_REAL, n8_neg12db, 8);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_realn4_realn4_neg6db_neg12db");
	ADI_CMS_ERROR_RETURN(err);

    /*
     * Half-Complex (16-taps) (4T4R CE board baluns have 180deg chan-to-chan phase diff)
    */
    err = pfilt_half_complex(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_0, ADI_APOLLO_PFILT_STREAM_1, 0, n16_pos0p5, n16_pos0p5, 16);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_realn2_hcplx_pos0p5_pos0p5");
	ADI_CMS_ERROR_RETURN(err);
    
    err = pfilt_half_complex(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_0, ADI_APOLLO_PFILT_STREAM_1, 0, n16_pos0p5, n16_neg0p5, 16);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_realn2_hcplx_pos0p5_neg0p5");
	ADI_CMS_ERROR_RETURN(err);

    err = pfilt_half_complex(obj, obj->pfilt_all, ADI_APOLLO_PFILT_STREAM_0, ADI_APOLLO_PFILT_STREAM_1, 0, n16_neg0p5, n16_pos0p5, 16);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_realn2_hcplx_neg0p5_pos0p5");
	ADI_CMS_ERROR_RETURN(err);
    
    /*
     * Full-Matrix (8-taps)
    */
    err = pfilt_full_matrix(obj, obj->pfilt_all, n8_neg6db, n8_neg6db, n8_neg6db, n8_neg6db, 8);
	ADI_CMS_ERROR_RETURN(err);
    err = sweep(obj, "fullchip_pfilt_fmtx_fmtx_neg6db_neg6db_neg6db_neg6db");
	ADI_CMS_ERROR_RETURN(err);
   
    return err;
}

/*
 * Creates a container object for commonly used members
 */
static fullchip_pfilt_exec_t *fullchip_pfilt_exec_obj_create(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, bool interactive)
{
    fullchip_pfilt_exec_t *obj = (fullchip_pfilt_exec_t *)malloc(sizeof(fullchip_pfilt_exec_t));

    obj->device = device;
    obj->fpga_device = fpga_device;
    obj->profile = profile;

    obj->is_interactive = interactive;

    obj->pfilt_all = device->dev_info.is_8t8r ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    obj->cnco_all = device->dev_info.is_8t8r ? ADI_APOLLO_CNCO_ALL : ADI_APOLLO_CNCO_ALL_4T4R;
    obj->fnco_all = device->dev_info.is_8t8r ? ADI_APOLLO_FNCO_ALL : ADI_APOLLO_FNCO_ALL_4T4R;

    obj->baseband_tone_ratio = 0.16390625;

    return obj;
}

static int32_t fullchip_pfilt_init(fullchip_pfilt_exec_t *obj)
{
    int32_t err = API_CMS_ERROR_OK;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];

    /* Load the filter gain and delay settings into the 4 banks */
    adi_apollo_pfilt_gain_dly_pgm_t filt_gain_dly_config = {
        ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, ADI_APOLLO_PFILT_GAIN_ZERO_DB, // Gain for A0, A2, A1, A3
        63, 63, 63, 63,                                                                                                             // x/64 6-bit gain multiplier (32 = ~6dB)
        0                                                                                                                           // half-complex mode delay
    };
    err = adi_apollo_pfilt_gain_dly_pgm(obj->device, ADI_APOLLO_RX, obj->pfilt_all, ADI_APOLLO_PFILT_BANK_ALL, &filt_gain_dly_config);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_gain_dly_pgm(obj->device, ADI_APOLLO_TX, obj->pfilt_all, ADI_APOLLO_PFILT_BANK_ALL, &filt_gain_dly_config);
	ADI_CMS_ERROR_RETURN(err);

    /* Use direct regmap for profile (i.e. coeff bank) selection */
    err = adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_RX, obj->pfilt_all, ADI_APOLLO_PFILT_CHAN_SEL_DIRECT_REGMAP);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_profile_sel_mode_set(obj->device, ADI_APOLLO_TX, obj->pfilt_all, ADI_APOLLO_PFILT_CHAN_SEL_DIRECT_REGMAP);
	ADI_CMS_ERROR_RETURN(err);

    /* Set PFILT data type to real */
    err = adi_apollo_pfilt_data_type_set(obj->device, ADI_APOLLO_RX, obj->pfilt_all, ADI_APOLLO_PFILT_REAL_DATA);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_data_type_set(obj->device, ADI_APOLLO_TX, obj->pfilt_all, ADI_APOLLO_PFILT_REAL_DATA);
	ADI_CMS_ERROR_RETURN(err);

    /* Get Rx and Tx profile info. Will assume all chans match A0 (side-A, chan 0) */
    err = adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &obj->rx_dp_info);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_ex_dp_info_to_str(&obj->rx_dp_info, "Base Rx Profile", str_buff, str_buff_len);
	ADI_CMS_ERROR_RETURN(err);
    printf("\n%s\n\n", str_buff);

    err = adi_ads10_ex_dp_info_get(obj->device, obj->profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &obj->tx_dp_info);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_ex_dp_info_to_str(&obj->tx_dp_info, "Base Tx Profile", str_buff, str_buff_len);
	ADI_CMS_ERROR_RETURN(err);
    printf("\n%s\n\n", str_buff);

    /* Override Device Profile - Set RX FNCO/CNCO to match TX - Tone will land at baseband freq in FFT when ADC-DAC are looped back. */
    err = adi_ads10_apollo_ex_cnco_set(obj->device, ADI_APOLLO_RX, obj->cnco_all, obj->rx_dp_info.adc_sample_rate, obj->tx_dp_info.cnco_freq);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_RX, obj->fnco_all, obj->rx_dp_info.adc_sample_rate / obj->rx_dp_info.cdrc, obj->tx_dp_info.fnco_freq);
	ADI_CMS_ERROR_RETURN(err);

    /* Enable INVSINC for all DACs */
    err = adi_apollo_invsinc_enable(obj->device, ADI_APOLLO_TX_INVSINC_ALL, 1);
	ADI_CMS_ERROR_RETURN(err);

    /*
     * Enable the FDUC gain block for all FDUCs.
     * Set each gain to 0.5. (note gain value of 1.0 is 0x800, 0.5 is 0x400)
     * Must do DP reset after adi_apollo_fduc_subdp_gain_enable() (not necessary for subsequent calls to adi_apollo_fduc_subdp_gain_set())
     * 0.5 is a -6dB change
     *
     * If this FDUC attenuation is not set, then summing FDUCs together results in clipping/distortion. FFT will look really bad!)
     */
    err = adi_apollo_fduc_subdp_gain_enable(obj->device, ADI_APOLLO_FDUC_ALL, 1);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fduc_subdp_gain_set(obj->device, ADI_APOLLO_FDUC_ALL, 0x800 >> 1);
	ADI_CMS_ERROR_RETURN(err); // 0.5 gain reduction if summing two fducs

    err = tx_dp_reset(obj->device, true);
	ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t pfilt_real_ntap(const fullchip_pfilt_exec_t *obj,
                               uint8_t pfilts, uint16_t streams,
                               adi_apollo_pfilt_mode_e mode,
                               int16_t coeffs[], uint32_t len)
{
    int32_t err = API_CMS_ERROR_OK;

    printf("@pfilt_real_ntap()\n");

    /* Load the coeffs for the specified filter mode */
    if (mode != ADI_APOLLO_PFILT_MODE_DISABLED) {
        err = adi_apollo_pfilt_coeff_ntap_set(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0, streams, mode, coeffs, len);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_pfilt_coeff_ntap_set(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0, streams, mode, coeffs, len);
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Set the PFILT configuration mode */
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, pfilts, streams, mode);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, pfilts, streams, mode);
	ADI_CMS_ERROR_RETURN(err);

    /* Select bank 0 for active coefficients */
    err = adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0);
	ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t pfilt_half_complex(const fullchip_pfilt_exec_t *obj, uint8_t pfilts, 
                                 adi_apollo_pfilt_stream_sel_e filt_stream, adi_apollo_pfilt_stream_sel_e dly_stream,
                                 uint8_t delay,
                                 int16_t coeffs0[], int16_t coeffs1[], uint32_t len)
{
    int32_t err = API_CMS_ERROR_OK;

    printf("@pfilt_half_complex()\n");

    /* Load the coeffs for half-complex mode */
    err = adi_apollo_pfilt_coeff_half_complex_set(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0, coeffs0, coeffs1, len);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_coeff_half_complex_set(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0, coeffs0, coeffs1, len);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the delay (in adc samples) */
    adi_apollo_pfilt_half_complex_delay_set(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0, delay);
    adi_apollo_pfilt_half_complex_delay_set(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0, delay);
    
    /* Set the PFILT configuration mode - one stream is filtered, the other is delayed */
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, pfilts, filt_stream, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, pfilts, dly_stream, ADI_APOLLO_PFILT_MODE_HALF_COMPLEX);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, pfilts, filt_stream, ADI_APOLLO_PFILT_MODE_N_DIV_BY_2_REAL);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, pfilts, dly_stream, ADI_APOLLO_PFILT_MODE_HALF_COMPLEX);
	ADI_CMS_ERROR_RETURN(err);
    
    /* Select bank 0 for active coefficients */
    err = adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0);
	ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t pfilt_full_matrix(const fullchip_pfilt_exec_t *obj, uint8_t pfilts,
                                 int16_t coeffs0[], int16_t coeffs1[], int16_t coeffs2[], int16_t coeffs3[], uint32_t len)
{
    int32_t err = API_CMS_ERROR_OK;

    printf("@pfilt_full_matrix()\n");

    /* Load the coeffs for full-matrix mode */
    err = adi_apollo_pfilt_coeff_full_matrix_set(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0, coeffs0, coeffs1, coeffs2, coeffs3, len);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_coeff_full_matrix_set(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0, coeffs0, coeffs1, coeffs2, coeffs3, len);
    ADI_CMS_ERROR_RETURN(err);

    /* Set the PFILT configuration mode - one stream is filtered, the other is delayed */
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_MATRIX);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_mode_enable_set(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_MATRIX);
	ADI_CMS_ERROR_RETURN(err);

    /* Select bank 0 for active coefficients */
    err = adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_BANK0);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_pfilt_coeff_transfer(obj->device, ADI_APOLLO_TX, pfilts, ADI_APOLLO_PFILT_BANK0);
	ADI_CMS_ERROR_RETURN(err);

    return err;
}

static int32_t sweep(const fullchip_pfilt_exec_t *obj, char *cap_fname_base)
{
    int32_t err = API_CMS_ERROR_OK;
    char fname[512];

    /*
     * Sweep Rx and Tx paths
     * 
     * For example, if Fadc = 8000MHz
     *  0.115 => 920
     *  0.300 => 2400
     *  0.399 => 3199.2
     */
    //double tone_ratios[] = {0.115, 0.3, 0.3999};      /* uncomment for multiple tone sweep */
    double tone_ratios[] = {0.115};
    for (int i = 0; i < sizeof(tone_ratios)/sizeof(tone_ratios[0]); i++) {
        
        /* Use the CNCO to set the RF freq */
        err = set_cnco(obj, tone_ratios[i]);
	    ADI_CMS_ERROR_RETURN(err);
        //err = play(obj, obj->baseband_tone_ratio, false);
	    ADI_CMS_ERROR_RETURN(err); /* uncomment for multiple tone sweep */
        
        adi_apollo_hal_delay_us(obj->device, 1000000 / 2);
        
        sprintf(fname, "%s_f%02d", cap_fname_base, i);
        err = capture(obj, fname, false);
	    ADI_CMS_ERROR_RETURN(err);

        printf("Expected DAC output freq: %fMHz\n", tone_ratios[i] * obj->tx_dp_info.dac_sample_rate);
        printf("Expected ADC baseband freq: %fMHz\n", obj->baseband_tone_ratio * obj->rx_dp_info.fdata);

        /* Manually set the ADC analog input frequency */
        printf("@CMD : SET_ADC_INPUT_FREQ : %fMHz\n", tone_ratios[i] * obj->tx_dp_info.dac_sample_rate);
        EXCTL_PAUSE(obj->is_interactive);
    }

    return err;
}

static int32_t capture(const fullchip_pfilt_exec_t *obj, char *cap_fname_base, bool bdir_start)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interleaved = true;
    uint32_t num_samples = 1024 * 16;

    if (bdir_start)
    {
        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        err = adi_fpga_apollo_core_bidir_init(obj->fpga_device);
	    ADI_CMS_ERROR_RETURN(err);
    }

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(obj->device, obj->profile, obj->fpga_device, num_samples, cap_fname_base, interleaved);
	ADI_CMS_ERROR_RETURN(err);

    printf("Done with capture. File base name: %s\n", cap_fname_base);

    return err;
}

static int32_t play(const fullchip_pfilt_exec_t *obj, double tone_ratio, bool bdir_start)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl

    /*
     * Get the FPGA features supported (such as HW transport layer and FSRC)
     */
    err = adi_fpga_apollo_core_feature_flags_get(obj->fpga_device, &fpga_feature_flags);
	ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(obj->fpga_device, obj->profile, ADI_APOLLO_SIDE_ALL, EX_VEC_DEFAULT_SAMPLES_PER_VC, tone_ratio, -1.0);
    ADI_CMS_ERROR_RETURN(err);
    
    if (bdir_start) {
        /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
        err = adi_fpga_apollo_core_bidir_init(obj->fpga_device);
	    ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_fpga_apollo_core_ptn_play_start(obj->fpga_device);
	    ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

static int32_t set_cnco(const fullchip_pfilt_exec_t *obj, double tone_ratio)
{
    int32_t err = API_CMS_ERROR_OK;

    /*
    * Configure the CNCO/FNCO such that the resulting tone ends up at f_final_mhz.
    */
    double fs_mhz = obj->tx_dp_info.dac_sample_rate;
    double f_tone_mhz = (tone_ratio * fs_mhz);
    double f_final_mhz = obj->baseband_tone_ratio * obj->tx_dp_info.fdata;     // Baseband, location in FFT
    double f_fnco_mhz = obj->tx_dp_info.fnco_freq;
    double f_cnco_mhz = f_tone_mhz - f_fnco_mhz - f_final_mhz;

    printf("ToneRatio=%f, Ftone=%f, CNCO=%f, FNCO=%f, Fbb=%f\n", tone_ratio, f_tone_mhz, f_cnco_mhz, f_fnco_mhz, f_final_mhz);
    err = adi_ads10_apollo_ex_cnco_set(obj->device, ADI_APOLLO_RX, obj->cnco_all, fs_mhz, f_cnco_mhz);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_RX, obj->fnco_all, fs_mhz / obj->rx_dp_info.cdrc, f_fnco_mhz);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_cnco_set(obj->device, ADI_APOLLO_TX, obj->cnco_all, fs_mhz, f_cnco_mhz);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_ads10_apollo_ex_fnco_set(obj->device, ADI_APOLLO_TX, obj->fnco_all, fs_mhz / obj->tx_dp_info.cdrc, f_fnco_mhz);
	ADI_CMS_ERROR_RETURN(err);
    
    return err;
}

static int32_t tx_dp_reset(adi_apollo_device_t *device, bool do_oneshot_sync)
{
    int32_t err = API_CMS_ERROR_OK;

    err = adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 0);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 1);
	ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 0);
	ADI_CMS_ERROR_RETURN(err);
    
    if (do_oneshot_sync) {
        err = adi_apollo_clk_mcs_oneshot_sync(device);
        ADI_CMS_ERROR_RETURN(err);
    }
    
    return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
