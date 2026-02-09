/*!
 * \brief    Private FPGA Apollo functions implementation
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_fpga_apollo_private.h"
#include "adi_fpga_apollo_rx_hw_tpl_to_samp.h"
#include "adi_fpga_apollo_capture.h"

static int32_t raw_cap_to_samples_12b(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link);
static int32_t raw_cap_to_samples_16b(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link);
static int32_t raw_cap_to_samples_8b(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link);
static int32_t raw_cap_to_samples_null(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link);

typedef int32_t (*raw_cap_to_samples_t)(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link);


/* Sets all sample data to zero */
int32_t adi_fpga_apollo_raw_cap_to_samples_zeros(adi_fpga_apollo_state_t *fpga_state, uint8_t raw_cap_buf[], uint32_t num_cap_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t frame, link, vc, idx;
    uint32_t frame_count;

    ADI_CMS_INVALID_PARAM_CHECK(num_cap_bytes == 0);
    ADI_CMS_NULL_PTR_CHECK(raw_cap_buf);
    ADI_CMS_NULL_PTR_CHECK(fpga_state);

    adi_fpga_apollo_capture_frame_t *cap_frame_info = &fpga_state->capture_info.capture_frame;
    frame_count = num_cap_bytes / cap_frame_info->frame_size_bytes;

    /* set cap buffer counts to 0 */
    for (link = 0; link < MAX_JESD_LINKS; link++) {
        for (vc = 0; vc < cap_frame_info->link_converter_count[link]; vc++) {
            fpga_state->capture_info.cap_transf_cnt[link][vc] = 0;
        }
    }

    for (frame = 0; frame < frame_count; frame++) {
        for (link = 0; link < MAX_JESD_LINKS; link++) {
            if (cap_frame_info->link_enabled[link]) {
                for (vc = 0; vc < cap_frame_info->link_converter_count[link]; vc++) {
                    for (idx = 0; idx < cap_frame_info->frame_samples_per_conv[link]; idx++) {
                        if (fpga_state->capture_info.cap_transf_cnt[link][vc] > cap_frame_info->transf_samps_per_conv[link]) {
                            return API_CMS_ERROR_OVERFLOW;
                        }
                        fpga_state->capture_info.cap_transf_buff[link][vc][idx*2] = 0;
                        fpga_state->capture_info.cap_transf_buff[link][vc][idx*2 + 1] = 0;
                        fpga_state->capture_info.cap_transf_cnt[link][vc]++;
                    }
                }
            }
        }
    }

    return err;
}

int32_t adi_fpga_apollo_raw_cap_to_samples(adi_fpga_apollo_state_t *fpga_state, uint8_t raw_cap_buf[], uint32_t num_cap_bytes)
{
    uint8_t *raw_cap_buf_ptr, *raw_cap_buf_ptr_end;
    uint32_t raw_cap_buf_link_ofst;
    raw_cap_to_samples_t raw_cap_to_samples[MAX_JESD_LINKS];
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    adi_fpga_jesd_param_t *jrx;
    uint32_t frame_count;
    uint32_t link, vc, frame;

    ADI_CMS_NULL_PTR_CHECK(fpga_state);
    ADI_CMS_NULL_PTR_CHECK(raw_cap_buf);
    ADI_CMS_INVALID_PARAM_CHECK(num_cap_bytes == 0);

    raw_cap_buf_ptr = raw_cap_buf;
    raw_cap_buf_ptr_end = raw_cap_buf + num_cap_bytes;
    cap_frame_info = &fpga_state->capture_info.capture_frame;
    jrx = fpga_state->jrx;
    frame_count = num_cap_bytes / cap_frame_info->frame_size_bytes;

    /* Set the function for unpacking raw samples based on Np */
    for (link = 0; link < MAX_JESD_LINKS; link++) {
        if (jrx[link].jesd_np == 16) {
            raw_cap_to_samples[link] = raw_cap_to_samples_16b;
        } else if (jrx[link].jesd_np == 12) {
            raw_cap_to_samples[link] = raw_cap_to_samples_12b;
        } else if (jrx[link].jesd_np == 8) {
            raw_cap_to_samples[link] = raw_cap_to_samples_8b;
        } else {
            raw_cap_to_samples[link] = raw_cap_to_samples_null;
        }
    }

    /* set cap buffer counts to 0 */
    for (link = 0; link < MAX_JESD_LINKS; link++) {
        for (vc = 0; vc < cap_frame_info->link_converter_count[link]; vc++) {
            fpga_state->capture_info.cap_transf_cnt[link][vc] = 0;
        }
    }

    for (frame = 0; frame < frame_count; frame++) {

        raw_cap_buf_link_ofst = 0;

        for (link = 0; link < MAX_JESD_LINKS; link++) {
            /* Don't process links with no data */
            if (cap_frame_info->fpga_link_bits[link] == 0) {
                continue;
            }
            /* Call the proper function to unpack raw samples */
            raw_cap_to_samples[link](fpga_state, raw_cap_buf_ptr + raw_cap_buf_link_ofst, link); 
            /* Advance raw data ptr to next block of cap frame raw samples */
            raw_cap_buf_link_ofst += cap_frame_info->fpga_link_bits[link] / 8;
        }
        /* Advance raw data ptr to next block of cap frame raw samples */
        raw_cap_buf_ptr += cap_frame_info->frame_size_bytes;
        /* Check for invalid memory access */
        if (raw_cap_buf_ptr > raw_cap_buf_ptr_end) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OVERFLOW);
        }
    }

    return API_CMS_ERROR_OK;
}

static int32_t raw_cap_to_samples_16b(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link)
{
    uint16_t sample_val;
    uint32_t frame, byte_idx;
    uint32_t vc = 0;
    uint32_t idx = 0;
    adi_fpga_apollo_capture_t *capture_info = &fpga_state->capture_info;
    adi_fpga_apollo_capture_frame_t *cap_frame_info = &capture_info->capture_frame;
    uint8_t *raw_cap_combined_buf;
    uint32_t jesd_m = cap_frame_info->link_converter_count[link];
    uint32_t raw_link_frame_bytes = cap_frame_info->fpga_link_bits[link] / 8;
    uint32_t raw_frame_size_bytes = cap_frame_info->frame_size_bytes / cap_frame_info->frame_mult;

    /* Concatenate raw link frames if mutliple frames is required */
    raw_cap_combined_buf = (uint8_t *) malloc(raw_link_frame_bytes * cap_frame_info->frame_mult);
    ADI_CMS_MEM_ALLOC_CHECK(raw_cap_combined_buf);
    for (frame = 0; frame < cap_frame_info->frame_mult; frame++) {
        memcpy(raw_cap_combined_buf + frame * raw_link_frame_bytes, raw_cap_ptr + frame * raw_frame_size_bytes, raw_link_frame_bytes);
    }

    /* Unpack raw link data into the capture frame */
    for (byte_idx = 0; byte_idx < raw_link_frame_bytes * cap_frame_info->frame_mult; byte_idx+=2) {
        /* Remove invalids if FSRC enabled */
        if (fpga_state->rx[link].fsrc) {
            sample_val = (raw_cap_combined_buf[byte_idx + 1] << 8) | raw_cap_combined_buf[byte_idx];
            if (sample_val == 0x8000)
                continue;
        }
        /* Remove repeated samples if Sample Repeat enabeld */
        if (fpga_state->rx[link].sr) {
            if (((byte_idx/2) / jesd_m) % (fpga_state->rx[link].link_xdrc / fpga_state->rx[link].tot_xdrc) != 0) {
                continue;
            }
        }
        /* Store sample in correct transfer buffer, increment count */
        idx = capture_info->cap_transf_cnt[link][vc];
        capture_info->cap_transf_buff[link][vc][idx*2] = raw_cap_combined_buf[byte_idx];
        capture_info->cap_transf_buff[link][vc][idx*2 + 1] = raw_cap_combined_buf[byte_idx + 1];
        capture_info->cap_transf_cnt[link][vc]++;
        /* Check for memory overlfow */
        if (capture_info->cap_transf_cnt[link][vc] > cap_frame_info->transf_samps_per_conv[link]) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OVERFLOW);
        }
        vc = (vc + 1) % jesd_m;
    }
    ADI_CMS_MEM_ALLOC_FREE(raw_cap_combined_buf);

    return API_CMS_ERROR_OK;
}

static int32_t raw_cap_to_samples_12b(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link)
{
    uint16_t sample_val;
    uint32_t tot_sample_cnt = 0;
    uint32_t frame, byte_idx;
    uint32_t vc = 0;
    uint32_t idx = 0;
    adi_fpga_apollo_capture_t *capture_info = &fpga_state->capture_info;
    adi_fpga_apollo_capture_frame_t *cap_frame_info = &capture_info->capture_frame;
    uint8_t *raw_cap_combined_buf;
    uint32_t jesd_m = cap_frame_info->link_converter_count[link];
    uint32_t raw_link_frame_bytes = cap_frame_info->fpga_link_bits[link] / 8;
    uint32_t raw_frame_size_bytes = cap_frame_info->frame_size_bytes / cap_frame_info->frame_mult;

    /* Concatenate raw link frames if mutliple frames is required */
    raw_cap_combined_buf = (uint8_t *) malloc(raw_link_frame_bytes * cap_frame_info->frame_mult);
    ADI_CMS_MEM_ALLOC_CHECK(raw_cap_combined_buf);
    for (frame = 0; frame < cap_frame_info->frame_mult; frame++) {
        memcpy(raw_cap_combined_buf + frame * raw_link_frame_bytes, raw_cap_ptr + frame * raw_frame_size_bytes, raw_link_frame_bytes);
    }

    /* Unpack raw link data into the capture frame */
    for (byte_idx = 0; byte_idx < raw_link_frame_bytes * cap_frame_info->frame_mult;) {
        if (tot_sample_cnt % 2 == 0) {
            sample_val = ((raw_cap_combined_buf[byte_idx + 1] & 0x0f) << 8) | raw_cap_combined_buf[byte_idx];
        } else {
            sample_val = (raw_cap_combined_buf[byte_idx + 2] << 4) | (raw_cap_combined_buf[byte_idx + 1] >> 4);
            byte_idx += 3;
        }
        /* Remove invalids if FSRC enabled */
        if (fpga_state->rx[link].fsrc) {
            if (sample_val == 0x800)
                continue;
        }
        /* Sign extend to 16 bit */
        sample_val = (sample_val & 0x800) ? (sample_val | 0xF000) : sample_val;
        /* Store sample in correct transfer buffer, increment count */
        idx = capture_info->cap_transf_cnt[link][vc];
        capture_info->cap_transf_buff[link][vc][idx*2] = sample_val & 0xFF;
        capture_info->cap_transf_buff[link][vc][idx*2 + 1] = sample_val >> 8 & 0xFF;
        capture_info->cap_transf_cnt[link][vc]++;
        /* Check for memory overlfow */
        if (capture_info->cap_transf_cnt[link][vc] > cap_frame_info->transf_samps_per_conv[link]) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OVERFLOW);
        }
        vc = (vc + 1) % jesd_m;
        tot_sample_cnt++;
    }
    ADI_CMS_MEM_ALLOC_FREE(raw_cap_combined_buf);

    return API_CMS_ERROR_OK;
}

static int32_t raw_cap_to_samples_8b(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link)
{
    uint32_t frame, byte_idx;
    uint32_t vc = 0;
    uint32_t idx = 0;
    adi_fpga_apollo_capture_t *capture_info = &fpga_state->capture_info;
    adi_fpga_apollo_capture_frame_t *cap_frame_info = &capture_info->capture_frame;
    uint8_t *raw_cap_combined_buf;
    uint32_t jesd_m = cap_frame_info->link_converter_count[link];
    uint32_t raw_link_frame_bytes = cap_frame_info->fpga_link_bits[link] / 8;
    uint32_t raw_frame_size_bytes = cap_frame_info->frame_size_bytes / cap_frame_info->frame_mult;

    /* Concatenate raw link frames if mutliple frames is required */
    raw_cap_combined_buf = (uint8_t *) malloc(raw_link_frame_bytes * cap_frame_info->frame_mult);
    ADI_CMS_MEM_ALLOC_CHECK(raw_cap_combined_buf);
    for (frame = 0; frame < cap_frame_info->frame_mult; frame++) {
        memcpy(raw_cap_combined_buf + frame * raw_link_frame_bytes, raw_cap_ptr + frame * raw_frame_size_bytes, raw_link_frame_bytes);
    }

    /* Store sample in correct transfer buffer, increment count */
    for (byte_idx = 0; byte_idx < raw_link_frame_bytes * cap_frame_info->frame_mult; byte_idx++) {
        idx = capture_info->cap_transf_cnt[link][vc];
        capture_info->cap_transf_buff[link][vc][idx] = raw_cap_combined_buf[byte_idx];
        capture_info->cap_transf_cnt[link][vc]++;
        /* Check for memory overlfow */
        if (capture_info->cap_transf_cnt[link][vc] > cap_frame_info->transf_samps_per_conv[link]) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OVERFLOW);
        }
        vc = (vc + 1) % jesd_m;
    }
    ADI_CMS_MEM_ALLOC_FREE(raw_cap_combined_buf);

    return API_CMS_ERROR_OK;
}

static int32_t raw_cap_to_samples_null(adi_fpga_apollo_state_t *fpga_state, uint8_t *raw_cap_ptr, uint32_t link)
{
    return API_CMS_ERROR_OK;
}