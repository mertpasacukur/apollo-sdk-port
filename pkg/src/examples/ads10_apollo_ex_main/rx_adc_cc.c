#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo Rx ADC Clock Conditioning test
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 *
 * \remarks
 * This example demonstrates on-demand clock conditioning. A clock conditioning calibration
 * may be required after the device temperature has shifted by a certain amount (e.g. 10C)
 *
 * The routine, rx_adc_cc, will run an initial CC cal and then acquire ADC samples and measure the
 * RMS noise of a disconnected input. If the CC cal is functioning properly, the RMS noise power will
 * be below a max passing level. After N captures/calculations a new CC will be executed simulating a
 * temperature change.
 *
 * This routine will return an error of the RMS level of any channel exceeds the upper limit.
 *
 * Note:
 *  - An RMS value of ~30 is about -145dBFs dBFS/Hz
 *  - Assumes all ADC inputs are not driven
 *  - Background cals must 'freeze' before running a CC cal. 'unfreeze' after CC cal finishes. See
 *    API calls adi_apollo_adc_bgcal_freeze() and adi_apollo_adc_bgcal_unfreeze()
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "adi_apollo.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_apollo_linux_utilities.h"

static int32_t cc_run(adi_apollo_device_t* device, uint16_t adc_cal_chans, bool run_fg_cal);

#define MAX_RMS_ERROR_THRESHOLD 100     /* ADC rms error threshold, 100rms ~ 135dB/Hz  */

int32_t rx_adc_cc(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    uint16_t adc_cal_chans = device->dev_info.is_8t8r ? ADI_APOLLO_ADC_ALL : ADI_APOLLO_ADC_ALL_4T4R;
    char tmu_str[256];
    char* cap_fname_base = "rx_adc_cc";
    char fname[MAX_PATH_LEN];
    adi_ads10_apollo_extras_cap_anal_t cap_anal;
    adi_apollo_device_tmu_data_t tmu_data;
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_L;
    uint8_t num_vc;
    adi_ads10_apollo_channel_info_t channel;
    adi_ads10_apollo_channel_selectors_t channel_selectors = { ADI_APOLLO_CDDC_A0, ADI_APOLLO_CNCO_A0, ADI_APOLLO_FNCO_A0, ADI_APOLLO_FDDC_A0, ADI_APOLLO_FSRC_A0};
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    int16_t *capture_buf;
    uint32_t num_samples_file = 0;
    uint8_t i, j = 0;

    double rms_min_i[4] = { 1000000.0, 1000000.0, 1000000.0, 1000000.0 };
    double rms_max_i[4] = { -1000000.0,-1000000.0,-1000000.0,-1000000.0 };
    double rms_min_q[4] = { 1000000.0, 1000000.0, 1000000.0, 1000000.0 };
    double rms_max_q[4] = { -1000000.0,-1000000.0,-1000000.0,-1000000.0 };

    /* This loop demonstrates on-demand clock conditioning. */
    for (i = 0; i < 100; i++) {

        /* Read device temperature */
        printf("\nCounter: %d\n", i);
        err = adi_apollo_device_tmu_get(device, &tmu_data);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_ex_tmu_data_to_str(&tmu_data, tmu_str, sizeof(tmu_str));
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_hal_log_write(device, ADI_CMS_LOG_MSG, "%s", tmu_str);
        ADI_CMS_ERROR_RETURN(err);
        printf("%s\n", tmu_str);

        /* Simulate a temperature event causing a clock conditioning cal */
        if (i % 10 == 0) {
            err = cc_run(device, adc_cal_chans, (i==0));
            ADI_CMS_ERROR_RETURN(err);

            /* Dynamic Sync Serdes Links gradually in a sequence */
            printf("Run Rx-TX SerDes Links Dyn Sync...\n");
            err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(device);
            ADI_CMS_ERROR_RETURN(err);
        }

        /* Capture data into separate I/Q arrays */
        err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, true, true);
        ADI_CMS_ERROR_RETURN(err);

        cap_frame_info = &fpga_device->state_info.capture_info.capture_frame;
        err = adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, channel_selectors, &channel);
        ADI_CMS_ERROR_RETURN(err);

        /* Process the capture data - calc rms level per channel. With no input can get rough noise measurement */
        for (j = 0; j < MAX_JESD_LINKS; j++) {
            num_vc = cap_frame_info->link_converter_count[j] / 2;
            for (int vc = 0; vc < num_vc; vc++) {
                /* get correct filename */
                snprintf(fname, MAX_PATH_LEN, "%s/%s_L%d_IQ%d_NP%d_DR%lld_CF%lld.txt", OUTPUT_DIR, cap_fname_base, j, vc, cap_frame_info->link_bits_per_sample[j], channel.data_rate_hz, channel.nco_freq_hz);

                err = adi_apollo_utilities_file_to_16b_samples_arr(device, fname, &capture_buf, &num_samples_file, 0);
                ADI_CMS_ERROR_GOTO(err, end);

                adi_ads10_apollo_extras_calc_rms(capture_buf, num_samples_file, &cap_anal);
                ADI_CMS_MEM_ALLOC_FREE(capture_buf);

                if (cap_anal.rms_i > rms_max_i[j]) { rms_max_i[j] = cap_anal.rms_i; }
                if (cap_anal.rms_i < rms_min_i[j]) { rms_min_i[j] = cap_anal.rms_i; }
                if (cap_anal.rms_q > rms_max_q[j]) { rms_max_q[j] = cap_anal.rms_q; }
                if (cap_anal.rms_q < rms_min_q[j]) { rms_min_q[j] = cap_anal.rms_q; }

                printf("ADC %1d I: rms = %7.1f%s    samp_min = %6d    samp_max = %6d    rms_min = %7.1f    rms_max = %7.1f\n",
                    j, cap_anal.rms_i, cap_anal.rms_i > MAX_RMS_ERROR_THRESHOLD ? "*" : " ", cap_anal.min_sample_i, cap_anal.max_sample_i, rms_min_i[j], rms_max_i[j]);
                printf("ADC %1d Q: rms = %7.1f%s    samp_min = %6d    samp_max = %6d    rms_min = %7.1f    rms_max = %7.1f\n",
                    j, cap_anal.rms_q, cap_anal.rms_q > MAX_RMS_ERROR_THRESHOLD ? "*" : " ", cap_anal.min_sample_q, cap_anal.max_sample_q, rms_min_q[j], rms_max_q[j]);

                /* The simple RMS value should not exceed MAX_RMS_ERROR_THRESHOLD if CC is working properly. */
                if ( (cap_anal.rms_i > MAX_RMS_ERROR_THRESHOLD) || (cap_anal.rms_q > MAX_RMS_ERROR_THRESHOLD) ){
                    ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
                }
            }
        }
        adi_apollo_hal_delay_us(device, 250 * 1000);
    }

end:
    ADI_CMS_MEM_ALLOC_FREE(capture_buf);
    return err;
}

static int32_t cc_run(adi_apollo_device_t* device, uint16_t adc_cal_chans, bool run_fg_cal)
{
    int32_t err = API_CMS_ERROR_OK;
    bool bgcal_state_get = false;   // optional
    char bgcal_state_str[1024];
    adi_apollo_adc_bgcal_state_t bgcal_state[ADI_APOLLO_ADC_NUM];

    /* Freeze ADC background cal */
    printf("FREEZE bgcal\n");
    err = adi_apollo_adc_bgcal_freeze(device, adc_cal_chans);
    ADI_CMS_ERROR_RETURN(err);

    if (bgcal_state_get) {
        /* Print bgcal state */
        err = adi_apollo_adc_bgcal_state_get(device, adc_cal_chans, bgcal_state, ADI_APOLLO_ADC_NUM);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_ex_adc_bgcal_to_str(adc_cal_chans, bgcal_state, bgcal_state_str, sizeof(bgcal_state_str));
        ADI_CMS_ERROR_RETURN(err);
        printf("%s\n", bgcal_state_str);
    }

    /* Run Clock Conditioning Cal */
    printf("Run clock conditioning cal (can take up to %d secs)...\n", ADI_APOLLO_SYSCLK_COND_CENTER_MAX_TO);
    err = adi_apollo_sysclk_cond_cal(device);
    ADI_CMS_ERROR_RETURN(err);
    printf("clock conditioning cal done\n");

    /* Run optional Foreground Cal */
    if (run_fg_cal) {
        printf("Run ADC FG cal (can take up to %d secs)...\n", (device->dev_info.is_8t8r ? 120 : 60));
        err = adi_apollo_adc_cal(device, adc_cal_chans);
        ADI_CMS_ERROR_RETURN(err);
        printf("ADC FG cal done\n");
    }

    /* Unfreeze ADC background cal */
    printf("UNFREEZE bgcal\n");
    err = adi_apollo_adc_bgcal_unfreeze(device, adc_cal_chans);
    ADI_CMS_ERROR_RETURN(err);

    if (bgcal_state_get) {
        /* Print bgcal state */
        err = adi_apollo_adc_bgcal_state_get(device, adc_cal_chans, bgcal_state, ADI_APOLLO_ADC_NUM);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_ex_adc_bgcal_to_str(adc_cal_chans, bgcal_state, bgcal_state_str, sizeof(bgcal_state_str));
        ADI_CMS_ERROR_RETURN(err);
        printf("%s\n", bgcal_state_str);
    }

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
