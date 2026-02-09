/*!
 * \brief     ADS10 Apollo Rx Mux2 Reconfig
 * 
 * This example demonstrates that the relative phase between ADC channels is maintained
 * when alternating CNCO selections via the RX Mux2 crossbar.
 * 
 * This example supports 4T4R and 8T8R devices
 *      4T4R: id00_uc06     (Fclk: 20000.0, FPGA Ref: 156.25, JESD204C)
 *      8T8R: id98_uc05     (Fclk:  8000.0, FPGA Ref: 250.0,  JESD204B)
 * 
 * Basic program flow:
 *  - Configure the Rx CNCO/FNCO freqs for two mux2 positions
 *  - Configure Rx Mux1 for ADC selection
 *  - Enable ADC averaging (optional)
 *  - Run calibrations
 *  - Repeat for # cap sets
 *      - Set mux2 pos (0 or 1)
 *      - FPGA capture to files
 *      - Execute DFT on all cap files (phase, freq, pwr)
 *      - Calculate phase delta across channels
 *      - Print phase info
 *      - Verify overall phase deltas 
 *      - Print Pass/Fail status
 *  
 *
 * Channel Configuration:
 *  FNCOs     => A0/B0   A4/B4   A1/B1   A5/B5       A2/B2   A6/B6     A3/B3     A7/B7     
 *  Virt Conv => m0/m1   m2/m3   m4/m5   m6/m7       m8/m9   m11/m12   m13/m14   m15/m16
 *  Mux-Pos      -----   -----   -----   -----       -----   -------   --------  -------
 *     0         F0      F0      f0      f0          f0      f0        f0        f0
 *     0         F1      F1      f0      f0          f0      f0        f0        f0
 *
 *     1         F0      F0      f1      f1          f1      f1        f1        f1
 *     1         F1      F1      f1      f1          f1      f1        f1        f1
 *      
 *  F0 is mux pos 0 ref chan (fixed)
 *  F1 is mux pos 1 ref chan (fixed)
 *  
 *  f0 is mux pos 0 chan
 *  f1 is mux pos 1 chan
 * 
 *
 * Capture File naming convention:
 *  rx_adc_mux2_0000_L0_m0_data16.txt
 *              |||| || ||-------------------------- VC in link
 *              |||| ||----------------------------- Link (L0=A0, L2=B0)
 *              ||||-------------------------------- Capture #. Even is mux pos 0, Odd is mux pos 1 (cap set = cap # / 2)
 * 
 *  Examples
 *      Cap #0, cap set 0, mux position 0, link A0, I/Q pair from FNCO-A0
 *          rx_adc_mux2_0000_L0_m0_data16.txt
 *          rx_adc_mux2_0000_L0_m1_data16.txt
 *  
 *      Cap #19, cap set 9, mux position 1, link B0, I/Q pair from FNCO-B7
 *          rx_adc_mux2_0019_L2_m15_data16.txt
 *          rx_adc_mux2_0019_L2_m16_data16.txt
 *  
 *
 * Note:
 * - Baseband freqs for mux position 0 and 1 must be equal for phase calculation
 * - This example currently supports SC0 only. Cross link phase requires SC1.
 * - For non-SC1 profiles, the baseband freq can be set equal to a multiple of SYREF (lmfc/lemc per) to 
 *   mitigate FPGA cross-link alignment.
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include "adi_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_apollo.h"
#include "adi_fpga_apollo_core.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_MUX_POS 2
#define DFT_LEN 512
#define MAX_CAP_SETS 100
#define MAX_VCS 16 * 2
#define MAX_PH_DELTA_LIMIT_PS 50.0
#define CAP_FILENAME_PREFIX "rx_adc_mux2_"

#define PRINT_VC_PHASE_DATA(label, m, cap_set, num_vcs, num_vcs_per_link, field) \
    printf("%4d %24s-%d  ", (cap_set), (label), (m));                            \
    for (int j = 0; j < (num_vcs); j += 2) {                                     \
        if ((j > 0) && (j % (num_vcs_per_link) == 0)) {                          \
            printf("\n%4d %24s-%d  ", (cap_set), (label), (m));                  \
        }                                                                        \
        printf("%10.3f ", ph_delta_mtx[m][(cap_set)][j].field);                  \
    }                                                                            \
    printf("\n");

#define PRINT_VC_DATA(label, m, num_vcs, num_vcs_per_link, field) \
    printf("%24s-%d  ", (label), (m));                            \
    for (int j = 0; j < (num_vcs); j += 2) {                      \
        if ((j > 0) && (j % (num_vcs_per_link) == 0)) {           \
            printf("\n%24s-%d  ", (label), (m));                  \
        }                                                         \
        printf("%10.3f ", net_result_mtx[m][j].field);            \
    }                                                             \
    printf("\n");

typedef struct
{
    double ph_abs_rads;
    double ph_delta_rads;
    double freq_mhz;
    double fund_pwr_dbfs;
} phase_info_struct_t;

typedef struct
{
    double min;
    double max;
    double delta;
} result_struct_t;

typedef struct
{
    result_struct_t ph_delta_rads;
    result_struct_t freq_mhz;
} net_result_struct_t;

static int32_t rx_mux1_config(adi_apollo_device_t *device);
static int32_t rx_mux2_config(adi_apollo_device_t *device, int mux_sel);
static int32_t file_to_int16(char *file_path, int16_t **samples, uint32_t *len);
static int32_t find_phase_cmplx(adi_apollo_device_t *device, char *file_path_i, char *file_path_q, adi_ads10_apollo_extras_dft_result *dft_result, int16_t scale, double fs);
static double phase_delta_calc(double ph0, double ph1);
static double f_bb_calc(double f_data, double f_bb_target, uint32_t fft_size);
static int32_t calc_net_result(phase_info_struct_t ph_delta_mtx[][MAX_CAP_SETS][MAX_VCS],
                               net_result_struct_t net_result_mtx[][MAX_VCS],
                               uint16_t num_mux_pos, uint16_t num_cap_sets, uint16_t num_vcs,
                               uint32_t num_vcs_per_link);
static void print_phase_info(phase_info_struct_t ph_delta_mtx[NUM_MUX_POS][MAX_CAP_SETS][MAX_VCS],
                             uint16_t num_mux_pos, uint16_t num_cap_sets, uint16_t num_vcs,
                             uint32_t num_vcs_per_link);
static void print_net_result_info(net_result_struct_t net_result_mtx[][MAX_VCS],
                                  uint16_t num_mux_pos, uint16_t num_vcs,
                                  uint32_t num_vcs_per_link, double f_bb_mhz);

int32_t rx_adc_mux2(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    char file_name_base[256];
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t rx_dp_info; // Rx datapath key info from profile
    uint32_t bgcal_dly = 1 * 1000000;      // Delay after starting bg cal (us)
    char cap_file[MAX_VCS][256];
    adi_ads10_apollo_extras_dft_result dft_result[MAX_VCS];
    const uint32_t num_mux_pos = NUM_MUX_POS;  // Switch between two mux2 positions (alternating CNCOs)
    const uint32_t num_cap_sets = ADI_UTILS_MIN(MAX_CAP_SETS, 10); // num cap sets per iteration
    phase_info_struct_t ph_delta_mtx[NUM_MUX_POS][MAX_CAP_SETS][MAX_VCS];
    net_result_struct_t net_result_mtx[NUM_MUX_POS][MAX_VCS];
    double overall_ph_delta_ps[MAX_VCS];
    double max_ph_delta_ps[NUM_MUX_POS];
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    adi_ads10_apollo_rx_channel_info_t channel;

    uint32_t num_vcs = (profile->jtx[0].tx_link_cfg[0].m_minus1 + 1) * 2; // assume 2 links
    uint32_t num_vcs_per_link = num_vcs / 2;  // assume two links
    uint8_t pave_en = 1;    // 1 to use ADC averaging.


    /* Device block selects for 4t4r or 8t8r device */
    uint16_t fncos = (device->dev_info.is_8t8r) ? ADI_APOLLO_FNCO_ALL : ADI_APOLLO_FNCO_ALL_4T4R;
    uint16_t pfilts = (device->dev_info.is_8t8r) ? ADI_APOLLO_PFILT_ALL : ADI_APOLLO_PFILT_ALL_4T4R;
    uint16_t cncos_mux_pos0 = ADI_APOLLO_CNCO_A0 | ADI_APOLLO_CNCO_B0 |
                              ((device->dev_info.is_8t8r) ? ADI_APOLLO_CNCO_A2 | ADI_APOLLO_CNCO_B2 : 0);
    uint16_t cncos_mux_pos1 = ADI_APOLLO_CNCO_A1 | ADI_APOLLO_CNCO_B1 |
                              ((device->dev_info.is_8t8r) ? ADI_APOLLO_CNCO_A3 | ADI_APOLLO_CNCO_B3 : 0);

    uint8_t is_sc1 = profile->jtx[0].common_link_cfg.subclass == ADI_APOLLO_SUBCLASS_1;
    
    if (is_sc1) {
        EXCTL_FAIL("SC1 profiles not supported");
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_NOT_SUPPORTED);
    }

    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    err = adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Rx Profile", str_buff, str_buff_len);
    ADI_CMS_ERROR_RETURN(err);
    printf("\n%s\n\n", str_buff);

    /* Inspect and print the Apollo JTx link config */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    double cnco_freq = 2150.0;
    double f_bb_mhz = f_bb_calc(rx_dp_info.fdata, 49.999, DFT_LEN);
    double f_adc_in_mhz = cnco_freq + f_bb_mhz;
    double cnco_freq_0 = cnco_freq;
    double cnco_freq_1 = cnco_freq;
    
    printf("f_bb_mhz= %f\n", f_bb_mhz);

    /* Set the CNCOs and FNCOs */
    err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_RX, cncos_mux_pos0, rx_dp_info.adc_sample_rate, cnco_freq_0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_RX, cncos_mux_pos1, rx_dp_info.adc_sample_rate, cnco_freq_1);
    ADI_CMS_ERROR_RETURN(err);
    
    err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_RX, fncos, rx_dp_info.adc_sample_rate / rx_dp_info.cdrc, 0.0);
    ADI_CMS_ERROR_RETURN(err);

   
    /* Set the ADC analog input frequency */
    EXCTL_SIGGEN_ON(0x33, false);
    EXCTL_SIGGEN_LEVEL(3.0, false);
    EXCTL_SIGGEN_FREQ(f_adc_in_mhz, interactive);

    /* RX crossbar1, ADC to CB outs */
    err = rx_mux1_config(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Optionally enable the PFILT ADC averaging block */
    err = adi_apollo_pfilt_ave_mode_set(device, pfilts, pave_en ? ADI_APOLLO_PFILT_AVE_ENABLE_ADD : ADI_APOLLO_PFILT_AVE_DISABLE);
    ADI_CMS_ERROR_RETURN(err);

    /* Bypass the Rx filter (after ADC ave block) */
    err = adi_apollo_pfilt_mode_enable_set(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_STREAM_ALL, ADI_APOLLO_PFILT_MODE_DISABLED);
    ADI_CMS_ERROR_RETURN(err);

    /* Set PFILT data type to complex - this will send ADC pair averages (real) to CNCO I inputs */
    err = adi_apollo_pfilt_data_type_set(device, ADI_APOLLO_RX, pfilts, ADI_APOLLO_PFILT_COMPLEX_DATA);
    ADI_CMS_ERROR_RETURN(err);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    /* Let ADC background cal run a bit to lower noise floor */
    printf("Let ADC bgcal run for %4.1f secs...\n", (bgcal_dly / 1000000.0));
    err = adi_apollo_hal_delay_us(device, bgcal_dly);
    ADI_CMS_ERROR_RETURN(err);

    /* Init FPGA JRx links */
    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    
    /* Alternate CNCO selection and write capture data to files */
    printf("Capturing samples...\n");
    for (int i = 0; i < num_mux_pos * num_cap_sets; i++) {

        /* Set the MUX2 xbar. This routes the CNCOs to the FNCOs*/
        err = rx_mux2_config(device, i % num_mux_pos);
        ADI_CMS_ERROR_RETURN(err);

        /* Read FPGA capture memory and write out individual virtual converter files (non-interleaved) */
        sprintf(file_name_base, "%s%04d", CAP_FILENAME_PREFIX, i);
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, DFT_LEN, file_name_base, false);

        ADI_CMS_ERROR_RETURN(err);
    }

    adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, &channel);    
    cap_frame_info = &fpga_device->state_info.capture_info.capture_frame;

    /* Determine the absolute phases of all captured channels. */
    printf("Determine absolute phases...\n");
    for (int m = 0; m < num_mux_pos; m++) {
        for (int i = 0; i < num_cap_sets; i++) {
            for (int j = 0; j < num_vcs; j++) {
                snprintf(&cap_file[j][0], MAX_PATH_LEN, "%s/%s%04d_L%d_m%d_NP%d_DR%lld_CF%lld.txt", OUTPUT_DIR, CAP_FILENAME_PREFIX,
                                        (i * num_mux_pos) + m, (j / num_vcs_per_link) * 2, j % num_vcs_per_link, cap_frame_info->link_bits_per_sample[(j / num_vcs_per_link) * 2], channel.data_rate_hz, channel.nco_freq_hz);
            }
            for (int j = 0; j < num_vcs; j += 2) {
                err = find_phase_cmplx(device, &cap_file[j][0], &cap_file[j + 1][0], &dft_result[j], 1, rx_dp_info.fdata);
                ADI_CMS_ERROR_RETURN(err);

                ph_delta_mtx[m][i][j].ph_abs_rads = dft_result[j].phase;
                ph_delta_mtx[m][i][j].freq_mhz = dft_result[j].freq;
                ph_delta_mtx[m][i][j].fund_pwr_dbfs = dft_result[j].fund_pwr_dbfs;
            }
        }
    }

    /* Calculate deltas from reference channel */
    printf("Calculate phase deltas...\n");
    for (int m = 0; m < num_mux_pos; m++) {
        for (int i = 0; i < num_cap_sets; i++) {
            for (int j = 0; j < num_vcs; j += 2) {

                // Delta across a single side
                if (j < num_vcs_per_link) {
                    ph_delta_mtx[m][i][j].ph_delta_rads = phase_delta_calc(ph_delta_mtx[m][i][(0)].ph_abs_rads, ph_delta_mtx[m][i][j].ph_abs_rads);
                } else {
                    ph_delta_mtx[m][i][j].ph_delta_rads = phase_delta_calc(ph_delta_mtx[m][i][(num_vcs_per_link)].ph_abs_rads, ph_delta_mtx[m][i][j].ph_abs_rads);
                }

                //  Delta across both sides (requires SC1)
                //ph_delta_mtx[m][i][j].ph_delta_rads = phase_delta_calc(ph_delta_mtx[m][i][(m * num_vcs_per_link)].ph_abs_rads, ph_delta_mtx[m][i][j].ph_abs_rads);
            }
        }
    }

    /* Print individual cap set data */
    print_phase_info(ph_delta_mtx, num_mux_pos, num_cap_sets, num_vcs, num_vcs_per_link);

    /* Combine all cap set data for each channel */
    calc_net_result(ph_delta_mtx, net_result_mtx, num_mux_pos, num_cap_sets, num_vcs, num_vcs_per_link);

    /* Print net result chan info */
    print_net_result_info(net_result_mtx, num_mux_pos, num_vcs, num_vcs_per_link, f_bb_mhz);

    /* Determine the overall phase delta for all chans. */
    for (int m = 0; m < num_mux_pos; m++) {
        for (int j = 0; j < num_vcs; j += 2) {
            overall_ph_delta_ps[j] = (net_result_mtx[m][j].ph_delta_rads.delta / (2 * M_PI * f_bb_mhz * 1e6)) * 1e12;
        }
        adi_ads10_apollo_extras_arr1d_double_max_abs_get(overall_ph_delta_ps, num_vcs, &max_ph_delta_ps[m]);
        printf("%20s-%d %10.3f ps - %s\n", "MAX DELTA", m, max_ph_delta_ps[m], max_ph_delta_ps[m] < MAX_PH_DELTA_LIMIT_PS ? "PASS" : "FAIL");
    }

    /* Overall test assertion. Pass if the ref to chans phase deltas are withing limits for both mux positions. */
    sprintf(str_buff, "Phase delta limit of %fps exceeded. mux0: %fps, mux1: %fps", MAX_PH_DELTA_LIMIT_PS, max_ph_delta_ps[0], max_ph_delta_ps[1]);
    if (max_ph_delta_ps[0] > MAX_PH_DELTA_LIMIT_PS || max_ph_delta_ps[1] > MAX_PH_DELTA_LIMIT_PS) {
        EXCTL_FAIL(str_buff);
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_TEST_FAILED);
    }


    return err;
}

static int32_t find_phase_cmplx(adi_apollo_device_t *device, char *file_path_i, char *file_path_q, adi_ads10_apollo_extras_dft_result *dft_result, int16_t scale, double fs)
{
    int32_t err = API_CMS_ERROR_OK;
    int i = 0;
    uint32_t len;
    int16_t *samples_i = NULL;
    int16_t *samples_q = NULL;
    adi_ads10_apollo_extras_fcomplex *data_in = NULL;
    adi_ads10_apollo_extras_fcomplex *data_out = NULL;
    double *mag_sqr = NULL;
    double *phase = NULL;
    double freq;

    err = file_to_int16(file_path_i, &samples_i, &len);
    ADI_CMS_ERROR_GOTO(err, end);

    err = file_to_int16(file_path_q, &samples_q, &len);
    ADI_CMS_ERROR_GOTO(err, end);

    for (i = 0; i < len; i++) {
        samples_i[i] *= scale;
        samples_q[i] *= scale;
    }

    len = ADI_UTILS_MIN(DFT_LEN, len);

    data_in = (adi_ads10_apollo_extras_fcomplex *)calloc(len, sizeof(adi_ads10_apollo_extras_fcomplex));
    data_out = (adi_ads10_apollo_extras_fcomplex *)calloc(len, sizeof(adi_ads10_apollo_extras_fcomplex));

    for (i = 0; i < len; i++) {
        data_in[i].real = samples_i[i];
        data_in[i].imag = samples_q[i];
    }

    err = adi_ads10_apollo_extras_dft_calc(data_in, data_out, len);

    mag_sqr = (double *)calloc(len, sizeof(double));
    phase = (double *)calloc(len, sizeof(double));

    err = adi_ads10_apollo_extras_dft_results_get(data_out, mag_sqr, phase, len);

    double min, max, delta;
    err = adi_ads10_apollo_extras_arr1d_double_delta_get(mag_sqr, len, &min, &max, &delta);

    uint32_t max_bin;
    adi_ads10_apollo_extras_arr1d_double_max_get(mag_sqr, len, &max, &max_bin);

    freq = (max_bin < (len / 2)) ? (fs / len) * max_bin : -(fs / len) * (len - max_bin);
    
    dft_result->fund = max_bin;
    dft_result->mag = mag_sqr[max_bin];
    dft_result->fund_pwr_dbfs = 20 * log10(ADI_UTILS_MAX(-140.0, sqrt(dft_result->mag) / pow(2, 15)));  // ref to 16-bits
    dft_result->phase = phase[max_bin];
    dft_result->freq = freq;

end:
    if (samples_i != NULL) {
        free(samples_i);
    }
    if (samples_q != NULL) {
        free(samples_q);
    }
    if (data_in != NULL) {
        free(data_in);
    }
    if (data_out != NULL) {
        free(data_out);
    }
    if (mag_sqr != NULL) {
        free(mag_sqr);
    }
    if (phase != NULL) {
        free(phase);
    }
    
    return err;
}


static int32_t file_to_int16(char *file_path, int16_t **samples, uint32_t *len)
{
    int32_t err = API_CMS_ERROR_OK;
    int i = 0;
    const int max_num_str_len = 16;
    char num_str[max_num_str_len];
    FILE *fp;

    ADI_CMS_NULL_PTR_CHECK(file_path);

    if (fp = fopen(file_path, "r"), fp == NULL) {
        printf("Can't open file %s\n", file_path);
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_FILE_OPEN);
    }

    *samples = (int16_t *)malloc(1024 * 64 * sizeof(int16_t)); // max 64k samples

    while ((fgets(num_str, max_num_str_len, fp) != NULL)) {
        (*samples)[i] = (int16_t)atoi(num_str);
        i++;
    }

    *len = i;

    fclose(fp);

    return err;
}

/*
 * Configures Rx MUX1 for averaging. Overrides device profile settings.
 */
static int32_t rx_mux1_config(adi_apollo_device_t *device)
{
    int32_t err = API_CMS_ERROR_OK;

    adi_apollo_rx_mux0_sel_e cbout_to_adc_4t4r_a[] = {ADI_APOLLO_4T4R_CB_OUT_0_FROM_ADC0, ADI_APOLLO_4T4R_CB_OUT_1_FROM_ADC1}; // ADCA0->CB0 + ADCA1->CB1 (Side A)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_4t4r_b[] = {ADI_APOLLO_4T4R_CB_OUT_0_FROM_ADC0, ADI_APOLLO_4T4R_CB_OUT_1_FROM_ADC1}; // ADCB0->CB0 + ADCB1->CB1 (Side B)

    adi_apollo_rx_mux0_sel_e cbout_to_adc_8t8r_a[] = {ADI_APOLLO_8T8R_CB_OUT_FROM_ADC0, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC2,  // ADCA0->CB0 + ADCA2->CB1
                                                      ADI_APOLLO_8T8R_CB_OUT_FROM_ADC1, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC3}; // ADCA1->CB2 + ADCA3->CB3 (Side A)
    adi_apollo_rx_mux0_sel_e cbout_to_adc_8t8r_b[] = {ADI_APOLLO_8T8R_CB_OUT_FROM_ADC0, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC2,  // ADCB0->CB0 + ADCB2->CB1
                                                      ADI_APOLLO_8T8R_CB_OUT_FROM_ADC1, ADI_APOLLO_8T8R_CB_OUT_FROM_ADC3}; // ADCB1->CB2 + ADCB3->CB3 (Side B)

    if (device->dev_info.is_8t8r) {
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_A, cbout_to_adc_8t8r_a, ADI_APOLLO_RX_MUX0_NUM_8T8R);
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_B, cbout_to_adc_8t8r_b, ADI_APOLLO_RX_MUX0_NUM_8T8R);
    } else {
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_A, cbout_to_adc_4t4r_a, ADI_APOLLO_RX_MUX0_NUM_4T4R);
        err |= adi_apollo_rxmux_xbar1_set(device, ADI_APOLLO_SIDE_B, cbout_to_adc_4t4r_b, ADI_APOLLO_RX_MUX0_NUM_4T4R);
    }  
    
    return err;
}

/*
 * Sets the ADC MUX2 selection
 */
static int32_t rx_mux2_config(adi_apollo_device_t *device, int mux_sel)
{
    int32_t err = API_CMS_ERROR_OK;

    /* 
     * 4T4R
     *  CNCO-A0 reference mux pos 0
     *  CNCO-B1 reference mux pos 1
     */
    /* mux pos 0 */
    static adi_apollo_rx_mux2_sel_e coarse_to_fine_xbar_4t4r_sel_0[2][4] = {
        {ADI_APOLLO_CB_4T4R_C0_TO_F0, ADI_APOLLO_CB_4T4R_C0_TO_F1, ADI_APOLLO_CB_4T4R_C0_TO_F2, ADI_APOLLO_CB_4T4R_C0_TO_F3}, // side A
        {ADI_APOLLO_CB_4T4R_C1_TO_F0, ADI_APOLLO_CB_4T4R_C0_TO_F1, ADI_APOLLO_CB_4T4R_C0_TO_F2, ADI_APOLLO_CB_4T4R_C0_TO_F3}  // side B
    };

    /* mux pos 1 */
    static adi_apollo_rx_mux2_sel_e coarse_to_fine_xbar_4t4r_sel_1[2][4] = {
        {ADI_APOLLO_CB_4T4R_C0_TO_F0, ADI_APOLLO_CB_4T4R_C1_TO_F1, ADI_APOLLO_CB_4T4R_C1_TO_F2, ADI_APOLLO_CB_4T4R_C1_TO_F3}, // side A
        {ADI_APOLLO_CB_4T4R_C1_TO_F0, ADI_APOLLO_CB_4T4R_C1_TO_F1, ADI_APOLLO_CB_4T4R_C1_TO_F2, ADI_APOLLO_CB_4T4R_C1_TO_F3}, // side B
    };
    

    /* 
     * 8T8R
     *  CNCO-A0 reference mux pos 0
     *  CNCO-B3 reference mux pos 1
     */
    /* mux pos 0 */
    static adi_apollo_rx_mux2_sel_e coarse_to_fine_xbar_8t8r_sel_0[2][4] = {
        {ADI_APOLLO_CB_8T8R_C0_TO_F0_F4, ADI_APOLLO_CB_8T8R_C0_TO_F1_F5, ADI_APOLLO_CB_8T8R_C0_TO_F2_F6, ADI_APOLLO_CB_8T8R_C0_TO_F3_F7},   // side A
        {ADI_APOLLO_CB_8T8R_C3_TO_F0_F4, ADI_APOLLO_CB_8T8R_C0_TO_F1_F5, ADI_APOLLO_CB_8T8R_C0_TO_F2_F6, ADI_APOLLO_CB_8T8R_C0_TO_F3_F7},   // side B
    };
    
    /* mux pos 1 */
    static adi_apollo_rx_mux2_sel_e coarse_to_fine_xbar_8t8r_sel_1[2][4] = {
        {ADI_APOLLO_CB_8T8R_C0_TO_F0_F4, ADI_APOLLO_CB_8T8R_C3_TO_F1_F5, ADI_APOLLO_CB_8T8R_C3_TO_F2_F6, ADI_APOLLO_CB_8T8R_C3_TO_F3_F7},   // side A
        {ADI_APOLLO_CB_8T8R_C3_TO_F0_F4, ADI_APOLLO_CB_8T8R_C3_TO_F1_F5, ADI_APOLLO_CB_8T8R_C3_TO_F2_F6, ADI_APOLLO_CB_8T8R_C3_TO_F3_F7},   // side B
    };
    
    if (device->dev_info.is_8t8r) {
        err = adi_apollo_rxmux_xbar2_set(device, ADI_APOLLO_SIDE_A, mux_sel == 0 ? coarse_to_fine_xbar_8t8r_sel_0[0] : coarse_to_fine_xbar_8t8r_sel_1[0], 4);
        err = adi_apollo_rxmux_xbar2_set(device, ADI_APOLLO_SIDE_B, mux_sel == 0 ? coarse_to_fine_xbar_8t8r_sel_0[1] : coarse_to_fine_xbar_8t8r_sel_1[1], 4);
    } else {
        err = adi_apollo_rxmux_xbar2_set(device, ADI_APOLLO_SIDE_A, mux_sel == 0 ? coarse_to_fine_xbar_4t4r_sel_0[0] : coarse_to_fine_xbar_4t4r_sel_1[0], 4);
        err = adi_apollo_rxmux_xbar2_set(device, ADI_APOLLO_SIDE_B, mux_sel == 0 ? coarse_to_fine_xbar_4t4r_sel_0[1] : coarse_to_fine_xbar_4t4r_sel_1[1], 4);
    }

    return err;
}

static double phase_delta_calc(double ph0, double ph1)
{
    double ph_delta_rads = ph1 - ph0;

    /* restrict the phase difference in the range[-pi, pi] */

    if (ph_delta_rads > M_PI) {
        return (ph_delta_rads - (2 * M_PI));
    } else if (ph_delta_rads <= -M_PI) {
        return (ph_delta_rads + (2 * M_PI));
    }

    return (ph_delta_rads);
}

static double f_bb_calc(double f_data, double f_bb_target, uint32_t fft_size)
{
    int m = ((fft_size * f_bb_target) / f_data);
    return (f_data / fft_size) * m;
}

void print_phase_info(phase_info_struct_t ph_delta_mtx[][MAX_CAP_SETS][MAX_VCS],
                                     uint16_t num_mux_pos, uint16_t num_cap_sets, uint16_t num_vcs,
                                     uint32_t num_vcs_per_link)
{
    for (int m = 0; m < num_mux_pos; m++) {
        printf("\n\n%10s-%d\n", "MUXPOS", m);

        for (int i = 0; i < num_cap_sets; i++) {

            PRINT_VC_PHASE_DATA("freq_mhz", m, i, num_vcs, num_vcs_per_link, freq_mhz);
            PRINT_VC_PHASE_DATA("fund_pwr_dbfs", m, i, num_vcs, num_vcs_per_link, fund_pwr_dbfs);
            PRINT_VC_PHASE_DATA("ph_abs_rad", m, i, num_vcs, num_vcs_per_link, ph_abs_rads);
            PRINT_VC_PHASE_DATA("ph_delta_rad", m, i, num_vcs, num_vcs_per_link, ph_delta_rads);
            printf("\n");
        }
    }
}

void print_net_result_info(net_result_struct_t net_result_mtx[][MAX_VCS],
                                          uint16_t num_mux_pos, uint16_t num_vcs,
                                          uint32_t num_vcs_per_link, double f_bb_mhz)
{
    for (int m = 0; m < num_mux_pos; m++) {
        printf("\n");
        printf("%s-%d\n", "MUXPOS", m);

        PRINT_VC_DATA("freq_mhz_min", m, num_vcs, num_vcs_per_link, freq_mhz.min);
        PRINT_VC_DATA("freq_mhz_min", m, num_vcs, num_vcs_per_link, freq_mhz.max);
        PRINT_VC_DATA("freq_mhz_delta", m, num_vcs, num_vcs_per_link, freq_mhz.delta);
        printf("\n");
        
        PRINT_VC_DATA("net_ph_delta_min", m, num_vcs, num_vcs_per_link, ph_delta_rads.min);
        PRINT_VC_DATA("net_ph_delta_max", m, num_vcs, num_vcs_per_link, ph_delta_rads.max);
        PRINT_VC_DATA("net_ph_delta", m, num_vcs, num_vcs_per_link, ph_delta_rads.delta);
        printf("\n");
    }    
}

/*
 * Determine the min, max and delta values from all capture sets for each chan
 */
int32_t calc_net_result(phase_info_struct_t ph_delta_mtx[][MAX_CAP_SETS][MAX_VCS],
                                       net_result_struct_t net_result_mtx[][MAX_VCS],
                                       uint16_t num_mux_pos, uint16_t num_cap_sets, uint16_t num_vcs,
                                       uint32_t num_vcs_per_link)
{
    int32_t err = API_CMS_ERROR_OK;
    double col[MAX_CAP_SETS];

    for (int m = 0; m < num_mux_pos; m++) {
        for (int j = 0; j < num_vcs; j += 2) {
            for (int i = 0; i < num_cap_sets; i++) {
                col[i] = ph_delta_mtx[m][i][j].ph_delta_rads;
            }
            
            err = adi_ads10_apollo_extras_arr1d_double_delta_get(col, num_cap_sets,
                                                                 &net_result_mtx[m][j].ph_delta_rads.min,
                                                                 &net_result_mtx[m][j].ph_delta_rads.max,
                                                                 &net_result_mtx[m][j].ph_delta_rads.delta);
            ADI_CMS_ERROR_RETURN(err);
            
            for (int i = 0; i < num_cap_sets; i++) {
                col[i] = ph_delta_mtx[m][i][j].freq_mhz;
            }
            err = adi_ads10_apollo_extras_arr1d_double_delta_get(col, num_cap_sets,
                                                                 &net_result_mtx[m][j].freq_mhz.min,
                                                                 &net_result_mtx[m][j].freq_mhz.max,
                                                                 &net_result_mtx[m][j].freq_mhz.delta);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return err;
}
