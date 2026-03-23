#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo examples common BMEM related functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_apollo.h"
#include "adi_ads10_apollo_ex_bmem.h"
#include "adi_vector.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_apollo_linux_utilities.h"

int32_t adi_ads10_apollo_ex_bmem_awg_config(adi_apollo_device_t* device, uint16_t bmem_sel)
{
    int32_t err;
    adi_apollo_bmem_awg_t config;

    /* Configure HSDIN  BMEM for AWG mode */
    config.parity_check_en = 1;
    config.ramclk_ph_dis = 0;
    config.sample_size = 0;

    err = adi_apollo_bmem_hsdin_awg_config(device, bmem_sel, &config);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}


int32_t adi_ads10_apollo_ex_bmem_awg_tone_write(adi_apollo_device_t* device, uint16_t bmem_sel, double tone_ratio, double backoff, bool debug)
{
    double tone_ratio_out = 0;

    return adi_ads10_apollo_ex_bmem_awg_tone_write_v2(device, bmem_sel, tone_ratio, backoff, debug, &tone_ratio_out);
}

int32_t adi_ads10_apollo_ex_bmem_awg_tone_write_v2(adi_apollo_device_t* device, uint16_t bmem_sel, double tone_ratio, double backoff, bool debug, double *tone_ratio_out)
{
    int32_t err = API_CMS_ERROR_OK;
    int32_t vec16_len = (device->dev_info.is_8t8r) ? 32 * 1024 : 64 * 1024;
    uint32_t scratch32_len = (device->dev_info.is_8t8r) ? vec16_len : vec16_len / 2;
    int16_t *vec16 = NULL;
    uint32_t *scratch32 = NULL;
    adi_apollo_hal_protocol_e ap;
    adi_vector_sine_config_t config = {
       .backoff = backoff,
       .resolution = 12,
       .ratio = tone_ratio,
       .phase = 0,
       .sample_total = vec16_len,
       .sample_idx = 0
   };

    /* Set the active protocol to SPI for writing BMEM AWG (will restore ap) */
    err = adi_apollo_hal_active_protocol_get(device, &ap);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_hal_active_protocol_set(device, ADI_APOLLO_HAL_PROTOCOL_SPI0);
    ADI_CMS_ERROR_RETURN(err);

    vec16 = (int16_t *)malloc(vec16_len * sizeof(int16_t));
    ADI_CMS_MEM_ALLOC_CHECK(vec16);

    printf("Create/load %f tone into BMEM...\n", tone_ratio);
    adi_vector_generate_sine_v2(vec16, vec16_len, &config, tone_ratio_out);

    if (debug) {
        adi_apollo_utilities_mkdir(device, true, BMEM_VEC_FILE_PATH);
        FILE * fp = fopen(BMEM_VEC_FILE, "a");
        if (fp == NULL) {
            err = API_CMS_ERROR_FILE_OPEN;
            ADI_CMS_ERROR_GOTO(err, end);
        }
        fprintf(fp, "Vector Ratio %f\n", tone_ratio);
        for (int i = 0; i < vec16_len; i++) {
            fprintf(fp, "%d\n", vec16[i]);
        }
        ADI_CMS_FILE_CLOSE(fp);
    }

    err = adi_apollo_bmem_hsdin_awg_stop(device, bmem_sel);
    ADI_CMS_ERROR_GOTO(err, end);

    // Configure BMEM for AWG mode and load samples.
    if (device->dev_info.is_8t8r) {
        scratch32 = (uint32_t *)malloc(scratch32_len * sizeof(uint32_t));
        if (scratch32 == NULL) {
            err = API_CMS_ERROR_MEM_ALLOC;
            ADI_CMS_ERROR_GOTO(err, end);
        }
        err = adi_apollo_bmem_hsdin_awg_sample_shared_write(device, bmem_sel, vec16, vec16, vec16_len, scratch32, scratch32_len);
        ADI_CMS_ERROR_GOTO(err, end);
    } else {
        err = adi_apollo_bmem_hsdin_awg_sample_write(device, bmem_sel, vec16, vec16_len);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    err = adi_apollo_bmem_hsdin_awg_start(device, bmem_sel);
    ADI_CMS_ERROR_GOTO(err, end);

end:    
    ADI_CMS_MEM_ALLOC_FREE(vec16);
    ADI_CMS_MEM_ALLOC_FREE(scratch32);

    // Return any err from called func
    ADI_CMS_ERROR_RETURN(err);

    // Restore active protocol
    err = adi_apollo_hal_active_protocol_set(device, ap);
    ADI_CMS_ERROR_RETURN(err)

    return err;
}

int32_t adi_ads10_apollo_ex_bmem_capture(adi_apollo_device_t* device, uint8_t bmem_sel, char *file_name)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t *bmem_cap32 = NULL;
    char full_filename[MAX_PATH_LEN];
    char full_filename2[MAX_PATH_LEN];
    int32_t side, sram;
    uint8_t bmem;
    uint32_t num_samples;           // Num samples to capture
    adi_apollo_hal_protocol_e ap;

    /* Get the active protocol */
    err = adi_apollo_hal_active_protocol_get(device, &ap);
    ADI_CMS_ERROR_RETURN(err);

    /* Currently, when using HSCI, num_samples is limited by rd transaction block size. SPI is limited by max BMEM size */
    num_samples = (ap == ADI_APOLLO_HAL_PROTOCOL_SPI0) ? (ADI_APOLLO_BMEM_HSDIN_MEM_SIZE_BYTES / 2) : (ADI_APOLLO_HAL_REGIO_HSCI_STREAM_DEFAULT_PAYLOAD_SIZE / 2);
    
    /* Allocate mem for HSDIN (ADC) capture samples */
    bmem_cap32 = (uint32_t *)malloc(num_samples * sizeof(int16_t));
    ADI_CMS_MEM_ALLOC_CHECK(bmem_cap32);

    err = adi_apollo_bmem_hsdin_capture_run(device, bmem_sel);
    ADI_CMS_ERROR_RETURN(err);

    for (side = 0; side < ADI_APOLLO_NUM_SIDES; side++) {
        for (sram = 0; sram < 2; sram++) {
            bmem =  (1 << sram) << (side*ADI_APOLLO_BMEM_PER_SIDE);
            if ((bmem & bmem_sel) != 0) {
                snprintf(full_filename, MAX_PATH_LEN, "%s_%c%d_I_DR%lld_CF0.txt", file_name, side == 0 ? 'A' : 'B', sram, device->dev_info.dev_freq_hz);
                snprintf(full_filename2, MAX_PATH_LEN, "%s_%c%d_I_DR%lld_CF0.txt", file_name, side == 0 ? 'A' : 'B', sram+2, device->dev_info.dev_freq_hz);
                err = adi_apollo_bmem_hsdin_capture_get(device, bmem, bmem_cap32, num_samples / 2);
                ADI_CMS_ERROR_GOTO(err, end);

                /* Extract the 16-bit samples from each BMEM 32-bit word */
                if (device->dev_info.is_8t8r) {
                    err = adi_ads10_apollo_ex_samples_16x2_stride_to_file(full_filename, 0, 2, bmem_cap32, num_samples / 2);
                    ADI_CMS_ERROR_GOTO(err, end);
                    err = adi_ads10_apollo_ex_samples_16x2_stride_to_file(full_filename2, 1, 2, bmem_cap32, num_samples / 2);
                    ADI_CMS_ERROR_GOTO(err, end);
                } else {
                    err = adi_ads10_apollo_ex_samples_16x2_stride_to_file(full_filename, 0, 1, bmem_cap32, num_samples / 2);
                    ADI_CMS_ERROR_GOTO(err, end);
                }
            }
        }
    }

end:
    ADI_CMS_MEM_ALLOC_FREE(bmem_cap32);
    return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
