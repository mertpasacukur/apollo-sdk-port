/*!
 * \brief     ADS10 Apollo Rx deep capture example
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
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
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_fpga_apollo_capture.h"

static int32_t verify_ramp(adi_fpga_apollo_device_t* fpga_device, uint32_t link, uint32_t vc, char* cap_fname_base, bool interleaved, adi_ads10_apollo_channel_info_t *channel);

int32_t rx_adc_deep(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    uint8_t tmode_en = 0;       /* 1 = use tmode to generate capture data, use normal datapath otherwise */
    bool interleaved = 1;       /* 1 = interleave data, 0 = separate I/Q files */
    int32_t err;
    adi_apollo_device_tmu_data_t tmu_data;
    uint32_t num_samples = 1024*1024;             /* min num samples per virt conv */
    char* cap_fname_base = "apollo_deep_cap";
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    adi_ads10_apollo_channel_info_t channel;
    adi_ads10_apollo_channel_selectors_t channel_selectors = { ADI_APOLLO_CDDC_A0, ADI_APOLLO_CNCO_A0, ADI_APOLLO_FNCO_A0, ADI_APOLLO_FDDC_A0, ADI_APOLLO_FSRC_A0};

    /* Inspect the JTx link states */
    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    if (tmode_en) {
        /* Select the tmode number of bits (assume all links have same Np val) */
        if (profile->jtx[0].tx_link_cfg->np_minus1 == ADI_APOLLO_BITS_PER_SAMP_16) {
            adi_apollo_tmode_config_set(device, ADI_APOLLO_LINK_ALL, 0xff, ADI_APOLLO_TMODE_TYPE_SEL_RAMP, ADI_APOLLO_RX_TMODE_RES_16B);
        } else if (profile->jtx[0].tx_link_cfg->np_minus1 == ADI_APOLLO_BITS_PER_SAMP_12) {
            adi_apollo_tmode_config_set(device, ADI_APOLLO_LINK_ALL, 0xff, ADI_APOLLO_TMODE_TYPE_SEL_RAMP, ADI_APOLLO_RX_TMODE_RES_12B);
        } else {
            adi_apollo_tmode_config_set(device, ADI_APOLLO_LINK_ALL, 0xff, ADI_APOLLO_TMODE_TYPE_SEL_RAMP, ADI_APOLLO_RX_TMODE_RES_16B);    /* There isn't an 8B tmode */
        }
    }

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | (tmode_en ? ADI_ADS10_APOLLO_CAL_NONE : ADI_ADS10_APOLLO_CAL_ADC));
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_adc_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, tmode_en ? ADI_ADS10_APOLLO_CAL_NONE : ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, true, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, channel_selectors, &channel);
    ADI_CMS_ERROR_RETURN(err);

    /* If running with TMODE generated ramp data, verify samples from files */
    if (tmode_en) {
        cap_frame_info = &fpga_device->state_info.capture_info.capture_frame;

        for (uint32_t link = 0; link < MAX_JESD_LINKS; link++) {
            if (!cap_frame_info->link_enabled[link]) {
                continue;
            }

            for (uint32_t vc = 0; vc < cap_frame_info->link_converter_count[link]; vc++) {
                err = verify_ramp(fpga_device, link, vc, cap_fname_base, interleaved, &channel);
                if (interleaved && (cap_frame_info->link_converter_count[link] % 2 != 1)) {
                    vc++;
                }
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }

    /*
     * Read device temperature sensors.
     */
    adi_apollo_device_tmu_get(device, &tmu_data);
    printf("TMU (deg C): %d %d %d %d %d %d %d %d \n",
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_SERDES_PLL],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_MPU_A],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_MPU_B],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_ADC_A],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_A],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_ADC_B],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_B],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_C]
    );

    return err;
}

static int32_t verify_ramp(adi_fpga_apollo_device_t* fpga_device, uint32_t link, uint32_t vc, char* cap_fname_base, bool interleaved, adi_ads10_apollo_channel_info_t *channel)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    uint16_t val_mask;
    uint8_t first = 1;
    uint16_t count;
    uint16_t sample;
    uint32_t sample_num = 0;
    char fname[MAX_PATH_LEN];
    char line[32];
    FILE* fp;
    uint8_t check = 0;

    cap_frame_info = &fpga_device->state_info.capture_info.capture_frame;
    val_mask = (uint16_t)(1 << cap_frame_info->link_bits_per_sample[0]) - 1;

    /* Determine file name for the link and vc */
    if (interleaved && (cap_frame_info->link_converter_count[link] % 2 != 1))
        snprintf(fname, MAX_PATH_LEN, "%s/%s_L%d_IQ%d_NP%d_DR%lld_CF%lld.txt", OUTPUT_DIR, cap_fname_base, link, vc/2, cap_frame_info->link_bits_per_sample[link], channel->data_rate_hz, channel->nco_freq_hz);
    else
        snprintf(fname, MAX_PATH_LEN, "%s/%s_L%d_m%d_NP%d_DR%lld_CF%lld.txt", OUTPUT_DIR, cap_fname_base, link, vc, cap_frame_info->link_bits_per_sample[link], channel->data_rate_hz, channel->nco_freq_hz);

    printf("Verifying ramp data in %s\n", fname);

    fp = fopen(fname, "r");
    if (fp == NULL) {
        printf("Error: can't open file %s\n", fname);
        err = API_CMS_ERROR_FILE_OPEN;
        goto end;
    }

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%hu", &sample);
        sample = sample & val_mask;
        if (first) {
            first = 0;
            count = sample;
        }

        if (sample != count) {
            printf("Error: data mismatch in capture at line %d in file %s\n", sample_num+1, fname);
            err = API_CMS_ERROR_ERROR;
            break;
        }

        if (interleaved && (cap_frame_info->link_converter_count[link] % 2 != 1)) {
            if (check)
                count = (count + 1) & val_mask;
            check = !check;
        } else {
            count = (count + 1) & val_mask;
        }

        sample_num++;
    }

    fclose(fp);

end:

    if (err == API_CMS_ERROR_OK) {
        printf("OK\n");
    }

    return err;
}
