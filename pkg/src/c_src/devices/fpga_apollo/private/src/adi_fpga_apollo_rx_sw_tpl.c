/*!
 * @brief     FPGA JRX SW Transport Layer
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */


/*============= I N C L U D E S ============*/
#include "adi_fpga_apollo_rx_sw_tpl.h"
#include "adi_fpga_apollo_capture.h"

/*============= C O D E ====================*/

static int32_t rx_sw_tpl_parse_raw_link(adi_fpga_apollo_capture_frame_t *raw_frame_info, uint8_t raw_data[], uint8_t link_data[],
                                                uint32_t data_length);
static int32_t rx_sw_tpl_lane_to_samps(adi_fpga_jesd_param_t jesd_param, uint8_t raw_data[], uint32_t data_length);
static int32_t rx_sw_tpl_pack_link_data(adi_fpga_apollo_capture_frame_t *raw_cap_frame, uint8_t raw_data[], uint8_t link_data[],  
                                                uint32_t data_length);

int32_t rx_jesd204_sw_tpl(adi_fpga_apollo_state_t *state_info, uint8_t raw_data[], uint32_t data_length)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_fpga_apollo_capture_frame_t *capture_frame = &state_info->capture_info.capture_frame;
    uint32_t data_per_link;
    uint8_t *link_data = NULL;
    uint32_t offset = 0;
    uint32_t links;

    link_data = malloc(sizeof(uint8_t) * data_length);
    ADI_CMS_MEM_ALLOC_CHECK(link_data);

    // Parse the raw data into individual links
    err = rx_sw_tpl_parse_raw_link(capture_frame, raw_data, link_data, data_length);
    ADI_CMS_ERROR_GOTO(err, end);

    // For each link convert from lane data to samples
    for (links = 0; links < MAX_JESD_LINKS; links++) {
        if (capture_frame->link_enabled[links]) {
            offset = (data_length / 4) * links;
            data_per_link = (capture_frame->fpga_link_bits[links] == 4096 ? data_length / 2 : data_length / 4);
            err = rx_sw_tpl_lane_to_samps(state_info->jrx[links], &link_data[offset], data_per_link);
            ADI_CMS_ERROR_GOTO(err, end);
        }
    }

    // Interleave sample data by link 
    err = rx_sw_tpl_pack_link_data(capture_frame, raw_data, link_data, data_length);
    ADI_CMS_ERROR_GOTO(err, end);

end:
    ADI_CMS_MEM_ALLOC_FREE(link_data);
    return err;
}


static int32_t rx_sw_tpl_parse_raw_link(adi_fpga_apollo_capture_frame_t *raw_frame_info, uint8_t raw_data[], uint8_t link_data[],
                                                uint32_t data_length)
{
    uint8_t link_index = 0;
    uint32_t link_buff_index = 0;
    uint32_t chunk_count = 0;
    uint32_t offset;

    ADI_CMS_NULL_PTR_CHECK(raw_data);
    ADI_CMS_NULL_PTR_CHECK(link_data);

    while (link_buff_index != data_length) {
        for (link_index = 0; link_index < MAX_JESD_LINKS; ++link_index) {
            if (raw_frame_info->fpga_link_bits[link_index] == 0) {
                continue;
            }
            offset = (data_length / 4) * link_index + (chunk_count * raw_frame_info->fpga_link_bits[link_index] / 8); // get offset into link data arr
            memcpy(&link_data[offset], &raw_data[link_buff_index], raw_frame_info->fpga_link_bits[link_index] / 8); // move chunks of bits (2048 if single link or 4096 for dual link) into arr
            link_buff_index += (raw_frame_info->fpga_link_bits[link_index] / 8); // increment by num of bits transfered
        }
        chunk_count++;
    }

    return API_CMS_ERROR_OK;
}

static int32_t rx_sw_tpl_lane_to_samps(adi_fpga_jesd_param_t jesd_param, uint8_t raw_data[], uint32_t data_length)
{
    uint8_t *raw_frame_ptr = raw_data;
    uint8_t jesd_lanes = jesd_param.jesd_l;
    uint8_t jesd_frames = jesd_param.jesd_f;
    uint8_t jesd_samps = jesd_param.jesd_s;
    uint8_t samp_size = jesd_param.jesd_np;
    uint8_t num_conv = jesd_param.jesd_m;
    uint32_t num_lane_bytes_chunk = data_length / (8 * jesd_lanes);
    uint8_t *ordered_data = NULL;
    uint32_t octet_idx, chunk, octets_per_frame, lane_idx, nibbles_chunk, samp_idx, conv_idx, nibble, frame_idx, octets_per_samp;
    uint32_t raw_idx = 0, nibble_idx = 0, sample_idx = 0, nibble_data = 0;
    uint32_t ordered_idx = 0;
    uint32_t inter_idx = 0;
    uint8_t bytes_per_samp = samp_size / 8;

    ADI_CMS_NULL_PTR_CHECK(raw_data);

    ordered_data = (uint8_t *)malloc(sizeof(uint8_t) * data_length);
    ADI_CMS_MEM_ALLOC_CHECK(ordered_data);

    // Convert from lane data bytes to ordered octets
    for (chunk = 0; chunk < num_lane_bytes_chunk; chunk++) {  // iterate through chunks of 8 (lane data byte) x num lanes
        for (octets_per_frame = 0; octets_per_frame < (8/jesd_frames); octets_per_frame++) { // iterate through groups of octest per frame 8/F 
            for (lane_idx = 0; lane_idx < jesd_lanes; lane_idx++) { // iterate through lanes
                for (octet_idx = 0; octet_idx < jesd_frames; octet_idx++) { // iterate through octets per frame
                    raw_idx = (chunk * 8 * jesd_lanes) + (8 * lane_idx) + (octets_per_frame*jesd_frames + octet_idx);  // offset to lane byte chunk + offset to lane byte + offset to octect
                    ordered_data[ordered_idx] = raw_frame_ptr[raw_idx];
                    ordered_idx++;
                }
            }
        }
    }
    memset(raw_data, 0, data_length);
    // convert ordered octets to interleaved samples (NP=12)
    if (samp_size == 12) {
        for (nibbles_chunk = 0; nibbles_chunk < (data_length * 2 / (jesd_samps * num_conv * 3)); nibbles_chunk++) { // iterate through each jesd frame (in terms of nibbles)
            for (samp_idx = 0; samp_idx < jesd_samps; samp_idx++) { // iterate though num of samples per jesd frame
                for (conv_idx = 0; conv_idx < num_conv; conv_idx++) { // iterate through each conv
                    for (nibble = 0; nibble < 3; nibble++) {  // iterate through the three nibbles that comprise each sample
                        nibble_idx = (nibbles_chunk * jesd_samps * num_conv * 3) + ((conv_idx * jesd_samps + samp_idx) * 3) + nibble; // offset to nibble chunck + offset to sample + offset to nibble
                        nibble_data =  (nibble_idx%2 == 0 ? (ordered_data[nibble_idx/2] & 0xF0) >> 4 : ordered_data[nibble_idx/2] & 0x0F); // octet = [nibble_idx even | nibble_idx odd]
                        sample_idx = (inter_idx % 3 == 0 ? inter_idx + 2 : (inter_idx % 3 == 2 ? inter_idx - 2 : inter_idx)); // get index to correct sample
                        raw_data[sample_idx/2] |= (sample_idx%2 == 0 ? nibble_data : (nibble_data << 4));
                        inter_idx++; 
                    }
                }
            }
        }
    // convert ordered octets to interleaved samples
    } else {
        for (frame_idx = 0; frame_idx < (data_length / (jesd_samps * num_conv *  bytes_per_samp)); frame_idx++) { // iterate through each jesd frame
            for (samp_idx = 0; samp_idx < jesd_samps; samp_idx++) { // iterate through num of samples per jesd frame
                for (conv_idx = 0; conv_idx < num_conv; conv_idx++) { // iterate through each conv
                    for (octets_per_samp = 0; octets_per_samp < bytes_per_samp; octets_per_samp++) { // iterate through number of octets per sample
                        octet_idx =((frame_idx * jesd_samps * num_conv) +  (conv_idx * jesd_samps + samp_idx)) * bytes_per_samp  + octets_per_samp;  // offset to frame + offset to conv + offset to sample + offset to octet
                        raw_data[(inter_idx % 2 == 0 ? inter_idx + 1 : inter_idx - 1)] = ordered_data[octet_idx]; // insert and flip to match HW format
                        inter_idx++; 
                    }
                }
            }
        }
    }

    ADI_CMS_MEM_ALLOC_FREE(ordered_data);
    return API_CMS_ERROR_OK;
}

static int32_t rx_sw_tpl_pack_link_data(adi_fpga_apollo_capture_frame_t *raw_cap_frame, uint8_t raw_data[], uint8_t link_data[], uint32_t data_length)
{
    uint32_t bytes_transfered = 0;
    uint32_t frame_offset;
    uint32_t frame_count = 0;
    uint32_t link_index;

    ADI_CMS_NULL_PTR_CHECK(raw_data);
    ADI_CMS_NULL_PTR_CHECK(link_data);

    // Interleave link data by 4096 bits (single link) or 2048 bits (dual link)
    while(bytes_transfered != data_length) {
        for (link_index = 0; link_index < MAX_JESD_LINKS; ++link_index) {
            if (raw_cap_frame->link_enabled[link_index]) {
                frame_offset = (data_length / 4) * link_index + (frame_count * raw_cap_frame->fpga_link_bits[link_index] / 8);
                memcpy(&raw_data[bytes_transfered], &link_data[frame_offset], raw_cap_frame->fpga_link_bits[link_index] / 8);
                bytes_transfered += (raw_cap_frame->fpga_link_bits[link_index] / 8);
            }
        }
        frame_count++;
    }

    return API_CMS_ERROR_OK;
}