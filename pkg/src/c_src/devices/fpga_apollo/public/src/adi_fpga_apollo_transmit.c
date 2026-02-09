/*!
 * \brief     FPGA Apollo transmit functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_APOLLO__
 */

/*============= I N C L U D E S ============*/
#include "adi_fpga_apollo_transmit.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_fsrc.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_utils.h"

/*============= D E F I N E S ==============*/

static int32_t create_transmit_buffer(int16_t input_vecs[], uint64_t input_vec_length, adi_fpga_feature_flag_t flags, adi_fpga_jesd_param_t jtx, adi_fpga_sr_fsrc_param_t fsrc, uint8_t **buf, uint32_t *buf_size);
static int32_t transmit_write_mem(adi_fpga_apollo_device_t* fpga_device, uint32_t side_select, uint8_t vec_buffer[], uint64_t vec_buffer_size);
static int32_t sw_tpl_pack(adi_fpga_jesd_param_t jtx, uint64_t vec_len, int16_t vecs[], uint64_t output_len, uint8_t output[]);
static int32_t hw_tpl_pack(int32_t num_dacs, uint64_t vec_len, int16_t vecs[], uint8_t* buff, uint64_t buff_len);
static int32_t jesd204_vecs_to_lanes(int32_t num_dacs, uint64_t vec_len, uint16_t vecs[], int32_t num_lanes, int32_t octets_per_lane, uint8_t lanes_output[num_lanes][octets_per_lane], adi_fpga_jesd_param_t jtx);
static int32_t jesd204_linear_mapping(int32_t num_dacs, uint64_t vec_len, uint16_t vecs[], uint64_t output_len, uint8_t stepone_output[output_len], adi_fpga_jesd_param_t jtx);
static int32_t jesd204_map_to_frames(uint64_t map_len, uint8_t linearmap[map_len], int32_t num_frames, int32_t octets_per_frame_per_lane, uint8_t steptwo_output[num_frames][octets_per_frame_per_lane], int32_t num_dacs, uint64_t vec_len, adi_fpga_jesd_param_t jtx);
static int32_t jesd204_frames_to_lanes(int32_t num_frames, uint64_t octets_per_frame_per_lane, uint8_t framesmap[num_frames][octets_per_frame_per_lane], int32_t num_lanes, int32_t octets_per_lane, uint8_t stepthree_output[num_lanes][octets_per_lane], int32_t num_dacs, uint64_t vec_len, adi_fpga_jesd_param_t jtx);
static int32_t jesd204_lanes_to_mem(int32_t num_lanes, int32_t octets_per_lane, uint8_t lanes_link0[num_lanes][octets_per_lane], int32_t num_lanes1, int32_t octets_per_lane1, uint8_t lanes_link1[num_lanes1][octets_per_lane1], uint64_t buffer_len, uint8_t buffer[buffer_len], int32_t num_links, adi_fpga_jesd_param_t jtx);
/*============= D A T A ====================*/

/*============= C O D E ====================*/

int32_t adi_fpga_apollo_transmit_write(adi_fpga_apollo_device_t *fpga, uint32_t side_select, int16_t input_vecs[], uint32_t input_vecs_length) {
    int32_t err = API_CMS_ERROR_OK;
    uint8_t *buff = NULL;
    uint32_t buff_size = 0;
    uint64_t total_size = 0;
    uint32_t side;
    adi_fpga_feature_flag_t feature_flags;

    adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);

    ADI_CMS_CHECK(feature_flags.tx_hw_tl && fpga->state_info.jtx[0].jesd_np != 16, API_CMS_ERROR_NOT_SUPPORTED);

    for (int32_t i = 0; i < ADI_APOLLO_NUM_SIDES; i++) {
        side = (side_select & (ADI_APOLLO_SIDE_A << i)); 
        if (side > 0) {
            err = create_transmit_buffer(input_vecs, input_vecs_length, feature_flags, fpga->state_info.jtx[i*2], fpga->state_info.tx[i*2], &buff, &buff_size);
            ADI_CMS_ERROR_GOTO(err, end);

            total_size += buff_size; // Sum of vectors generated for both sides
            err = adi_fpga_apollo_core_max_mem_size_check(fpga, total_size);
            ADI_CMS_ERROR_GOTO(err, end);
            
            err = transmit_write_mem(fpga, side, buff, buff_size);
            ADI_CMS_ERROR_GOTO(err, end);

            ADI_CMS_MEM_ALLOC_FREE(buff);

        }
    }

end:
    ADI_CMS_MEM_ALLOC_FREE(buff);

    return err;
}

/*============= S T A T I C S ==============*/

static int32_t transmit_write_mem(adi_fpga_apollo_device_t* fpga_device, uint32_t side_select, uint8_t vec_buffer[], uint64_t vec_buffer_size)
{
    int32_t err;
    uint32_t link_count;
    uint8_t section_start;
    uint8_t section_end;
    uint32_t address;
    adi_fpga_design_identifier_e design_id = fpga_device->state_info.design_id;
    
    if (design_id == ADI_FPGA_APOLLO_DESIGN_VCU128 || design_id == ADI_FPGA_APOLLO_DESIGN_MM_REF) {
        address = BLOCK_RAM_TX_VEC_ADDR;
    } else {
        address = HBM_MEM_TX_VEC_ADDR;
    }

    err = adi_fpga_apollo_core_jtx_link_cnt_get(fpga_device, &link_count);
    ADI_CMS_ERROR_RETURN(err);

    if (side_select == 1) {
        /* Select FPGA load section A-SIDE */
        printf("Load mem A-side\n");
        section_start = 0;
        section_end = (link_count > 2 && !fpga_device->state_info.jtx[0].jesd_dual_link) ? 1 : 0;
    } else {
        /* Select FPGA load section B-SIDE */
        printf("Load mem B-side\n");
        section_start = (link_count > 2 && !fpga_device->state_info.jtx[2].jesd_dual_link) ? 2 : 1;
        section_end = (link_count > 2 && !fpga_device->state_info.jtx[2].jesd_dual_link) ? 3 : 1;
    }

    /* Select link sections to be written */
    err = adi_fpga_apollo_core_memory_section_select(fpga_device, section_start, section_end);
    ADI_CMS_ERROR_RETURN(err);
    /* Set JTX Link config params */
    err = adi_fpga_apollo_core_transmit_link_config(fpga_device, address, vec_buffer_size, fpga_device->state_info.jrx[0].jesd_dual_link);
    ADI_CMS_ERROR_RETURN(err);

    /* Write transmit vector (in bytes) to FPGA memory. */
    err = adi_fpga_apollo_core_write_memory(fpga_device, address, vec_buffer, vec_buffer_size);
    ADI_CMS_ERROR_RETURN(err);
    
    return err;
}

int32_t create_transmit_buffer(int16_t input_vecs[], uint64_t input_vec_length, adi_fpga_feature_flag_t flags, adi_fpga_jesd_param_t jtx, adi_fpga_sr_fsrc_param_t fsrc, uint8_t **buf, uint32_t *buf_size)
{
    int32_t err;
    uint8_t* vec_buffer = NULL;
    int16_t* fsrc_vecs = NULL;
    int16_t* sr_vecs = NULL;
    uint32_t vec_buffer_size;
    adi_apollo_fsrc_tx_vector_length_t fsrc_vec_len;
    uint8_t num_dacs = jtx.jesd_m;
    int16_t* pre_tpl_buffer = input_vecs;
    uint32_t pre_tpl_buffer_size = input_vec_length;

    uint32_t vec_blks;
    uint32_t vec_blk_residual;

    /* Software FSRC */
    if (flags.tx_hw_fsrc == 0 && fsrc.fsrc) {
        /* Determine the number of valid and invalid samples required for the FSRC ratio */
        printf("ADS10 SW FSRC\n");
        adi_fpga_apollo_fsrc_tx_vector_length_get(pre_tpl_buffer_size/num_dacs, fsrc.n, fsrc.m, &fsrc_vec_len);

        /* Allocate memory for FSRC translated vector */
        fsrc_vecs = malloc(sizeof(int16_t) * fsrc_vec_len.num_total_samples * num_dacs);
        ADI_CMS_MEM_ALLOC_CHECK(fsrc_vecs);

        /* Do the invalid sample insertion. Process in I/Q pairs */
        for (int32_t i = 0; i < num_dacs; i = i + 2) {
            adi_fpga_apollo_fsrc_sample_insert((uint16_t*)&pre_tpl_buffer[i * pre_tpl_buffer_size/num_dacs], (uint16_t*)&pre_tpl_buffer[(i + 1) * pre_tpl_buffer_size/num_dacs], pre_tpl_buffer_size/num_dacs, 8, (uint16_t*)&fsrc_vecs[i * fsrc_vec_len.num_total_samples ], (uint16_t*)&fsrc_vecs[(i + 1) * fsrc_vec_len.num_total_samples], &fsrc_vec_len);        
        }
        pre_tpl_buffer = fsrc_vecs;
        pre_tpl_buffer_size = fsrc_vec_len.num_total_samples * num_dacs;
    } 

    if (fsrc.sr) {
        /* Determine the number of valid and invalid samples required for the FSRC ratio */
        printf("ADS10 SW SR\n");

        /* Allocate memory for FSRC translated vector */
        sr_vecs = malloc(sizeof(int16_t) * pre_tpl_buffer_size * fsrc.link_xdrc / fsrc.tot_xdrc);
        ADI_CMS_MEM_ALLOC_CHECK(sr_vecs);

        /* Do the invalid sample insertion. Process in I/Q pairs */
        uint32_t samp_idx = 0;
        for (int i = 0; i < pre_tpl_buffer_size; i++) {
            for (int j = 0; j < (fsrc.link_xdrc / fsrc.tot_xdrc); j++) { 
                sr_vecs[samp_idx] = input_vecs[i];
                samp_idx++;
            }
        }
        printf("sample_idx: %d\n", samp_idx);
        pre_tpl_buffer = sr_vecs;
        pre_tpl_buffer_size = pre_tpl_buffer_size * fsrc.link_xdrc / fsrc.tot_xdrc;
    }

    /* HW transport: determine memory size requirement (bytes) */
    vec_buffer_size = (jtx.jesd_np / 8.0) * pre_tpl_buffer_size;

    if (!flags.tx_hw_tl) {
        /* SW transport: determine memory size, ADS10 memory layout requires (128*L byte) boundaries */
        vec_blks = vec_buffer_size / (128 * jtx.jesd_l);
        vec_blk_residual = vec_buffer_size % (128 * jtx.jesd_l);
        vec_blks += (vec_blk_residual == 0) ? 0 : 1;
        vec_buffer_size = vec_blks * (128 * jtx.jesd_l);
    }


    vec_buffer = (uint8_t*)malloc(vec_buffer_size * sizeof(uint8_t));
    if (vec_buffer == NULL) {
        err = API_CMS_ERROR_MEM_ALLOC;
        ADI_CMS_ERROR_GOTO(err, end);
    }

    if (!flags.tx_hw_tl) {

        /* Do JESD204 transport layer processing on sample data */
        printf("ADS10 SW Transport\n");
        err = sw_tpl_pack(jtx, pre_tpl_buffer_size, pre_tpl_buffer, vec_buffer_size, vec_buffer);
        ADI_CMS_ERROR_RETURN(err);

    }
    else {

        /* Fill buffer with samples organized for ADS10 FPGA HW Transport format */
        printf("ADS10 HW Transport\n");
        err = hw_tpl_pack(num_dacs, pre_tpl_buffer_size, pre_tpl_buffer, vec_buffer, vec_buffer_size);
        ADI_CMS_ERROR_RETURN(err);
    }

end:

    ADI_CMS_MEM_ALLOC_FREE(fsrc_vecs);
    ADI_CMS_MEM_ALLOC_FREE(sr_vecs);
    *buf_size = vec_buffer_size;
    *buf = vec_buffer;
    return err;
}

static int32_t hw_tpl_pack(int32_t num_dacs, uint64_t input_vec_len, int16_t input_vec[], uint8_t* output_buff, uint64_t output_buff_len)
{
    uint32_t chunk_grp_size = 64 * num_dacs;
    uint32_t num_chunk_grps = input_vec_len / chunk_grp_size;
    uint32_t chunk_grp = 0;
    uint32_t vc = 0;
    uint16_t* chunk_grp_ptr = (uint16_t *)output_buff;
    uint16_t* vc_chunk_ptr;
    uint16_t sample;

    for (chunk_grp = 0; chunk_grp < num_chunk_grps; chunk_grp++) {

        for (vc = 0; vc < num_dacs; vc++) {
            vc_chunk_ptr = chunk_grp_ptr + (vc * 64);

            for (int32_t i = 0; i < 64; i++) {
                sample = (uint16_t)input_vec[(vc * input_vec_len / num_dacs) + i + chunk_grp * 64];
                *(vc_chunk_ptr + i) = sample;
            }

        }

        chunk_grp_ptr += chunk_grp_size;
    }

    return 0;
}

static int32_t sw_tpl_pack(adi_fpga_jesd_param_t jtx, uint64_t vec_len, int16_t vecs[], uint64_t output_len, uint8_t output[])
{
    uint16_t num_links = jtx.jesd_dual_link ? 2 : 1;
    uint16_t num_dacs = jtx.jesd_m;
    uint16_t num_lanes = jtx.jesd_l;
    uint32_t num_frames = (vec_len * (jtx.jesd_np / 8.0)) / jtx.jesd_f;
    uint32_t octets_per_lane = (num_frames / num_lanes) * jtx.jesd_f;
    uint8_t (*lanes_output)[octets_per_lane] = malloc(num_lanes * octets_per_lane * sizeof(uint8_t));

    jesd204_vecs_to_lanes(num_dacs, vec_len, (uint16_t*)vecs, num_lanes, octets_per_lane, lanes_output, jtx);

    jesd204_lanes_to_mem(num_lanes, octets_per_lane, lanes_output, num_lanes, octets_per_lane, NULL, output_len, output, num_links, jtx);

    free(lanes_output);

    return API_CMS_ERROR_OK;
}

static int32_t jesd204_vecs_to_lanes(int32_t num_dacs, uint64_t vec_len,  uint16_t vecs[], int32_t num_lanes, int32_t octets_per_lane, uint8_t lanes_output[num_lanes][octets_per_lane], adi_fpga_jesd_param_t jtx)
{
    if (jtx.jesd_l != 0) {
        if (jtx.jesd_f != (jtx.jesd_m * jtx.jesd_s * jtx.jesd_np) / (8*jtx.jesd_l)) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
        }
    }

    vec_len = vec_len / num_dacs;

    double bytes_per_sample = jtx.jesd_np / 8.0;
    int32_t map_len = jtx.jesd_m * vec_len * bytes_per_sample;
    uint8_t (*linearoutput) = calloc(map_len, sizeof(uint8_t));

    jesd204_linear_mapping(num_dacs, vec_len, vecs, map_len, linearoutput, jtx);

    int32_t num_frames = (jtx.jesd_m * vec_len * (jtx.jesd_np / 8.0)) / jtx.jesd_f;
    int32_t octets_per_frame_per_lane = jtx.jesd_f;
    uint8_t (*frameoutput)[octets_per_frame_per_lane] = malloc(num_frames * octets_per_frame_per_lane * sizeof(uint8_t));
    
    jesd204_map_to_frames(map_len, linearoutput, num_frames, octets_per_frame_per_lane, frameoutput, jtx.jesd_m, vec_len, jtx);

    jesd204_frames_to_lanes(num_frames, octets_per_frame_per_lane, frameoutput, num_lanes, octets_per_lane, lanes_output, jtx.jesd_m, vec_len, jtx);

    free(linearoutput);
    free(frameoutput);

    return 0;
}

static int32_t jesd204_linear_mapping(int32_t num_dacs, uint64_t vec_len,  uint16_t vecs[], uint64_t output_len, uint8_t stepone_output[output_len], adi_fpga_jesd_param_t jtx)
{
    double bytes_per_sample = jtx.jesd_np / 8.0;
    int32_t nibbles_per_sample = (int32_t) 2*bytes_per_sample;
    int32_t num_samples_per_frame = jtx.jesd_s;

    ADI_CMS_INVALID_PARAM_CHECK(num_dacs != jtx.jesd_m);

    int32_t samples_per_dac = vec_len;

    int32_t byte_counter = 0;
    int32_t nibble_counter = 0;

    uint16_t sample = 0;

    for (uint32_t i = 0; i < samples_per_dac; i += num_samples_per_frame) {
        for (uint32_t m = 0; m < num_dacs; m++) {
            for (uint32_t s = 0; s < num_samples_per_frame; s++) {
                if ((i + s) > vec_len) {
                    break;
                }
                sample = (uint16_t)vecs[m * vec_len + i + s];
                for (int32_t n = 0; n < nibbles_per_sample; n++) {
                    int32_t shift = 4 * (nibbles_per_sample - n - 1);
                    stepone_output[byte_counter] |= (uint8_t)((sample >> shift) & 0x000f) << (4 - ((nibble_counter % 2) * 4));
                    nibble_counter++;
                    if ((nibble_counter % 2) == 0) {
                        byte_counter++;
                    }
                }
            }
        }
    }

    return 0;
}

static int32_t jesd204_map_to_frames(uint64_t map_len, uint8_t linearmap[map_len], int32_t num_frames, int32_t octets_per_frame_per_lane, uint8_t steptwo_output[num_frames][octets_per_frame_per_lane], int32_t num_dacs, uint64_t vec_len, adi_fpga_jesd_param_t jtx)
{
    ADI_CMS_INVALID_PARAM_CHECK(((int32_t)(num_dacs * vec_len * (jtx.jesd_np / 8.0)) % octets_per_frame_per_lane) != 0);

    int32_t byte_counter = 0;
    for (int32_t i = 0; i < num_frames; i++) {
        for (int32_t j = 0; j < octets_per_frame_per_lane; j++) {
            steptwo_output[i][j] = linearmap[byte_counter];
            byte_counter++;
        }
    }
    return 0;
}

static int32_t jesd204_frames_to_lanes(int32_t num_frames, uint64_t octets_per_frame_per_lane, uint8_t framesmap[num_frames][octets_per_frame_per_lane], int32_t num_lanes, int32_t octets_per_lane, uint8_t stepthree_output[num_lanes][octets_per_lane], int32_t num_dacs, uint64_t vec_len, adi_fpga_jesd_param_t jtx)
{
    ADI_CMS_INVALID_PARAM_CHECK(num_frames % num_lanes != 0);

    for (int32_t l = 0; l < num_lanes; l++) {
        int32_t octet = 0;
        for (int32_t f = l; f < num_frames; f += num_lanes) {
            for (int32_t byteofframe = 0; byteofframe < octets_per_frame_per_lane; byteofframe++) {
                if (octet > (octets_per_lane - 1)) {
                    break;
                }
                stepthree_output[l][octet] = framesmap[f][byteofframe];
                octet++;
            }
        }
    }
    return 0;
}

static int32_t jesd204_lanes_to_mem(int32_t num_lanes, int32_t octets_per_lane, uint8_t lanes_link0[num_lanes][octets_per_lane], int32_t num_lanes1, int32_t octets_per_lane1, uint8_t lanes_link1[num_lanes1][octets_per_lane1], uint64_t buffer_len, uint8_t buffer[buffer_len], int32_t num_links, adi_fpga_jesd_param_t jtx)
{
    int32_t ads10 = 128;

    int32_t offset = 0;
    int32_t count = 0;
    int32_t total = octets_per_lane / ads10;
    if (octets_per_lane % ads10 != 0) {
        total += 1;
    }

    for(int32_t set = 0 ; set < total ; set++) {
        offset = set*ads10;
        for (int32_t link = 0; link < num_links; link++) {
            for (int32_t l = 0; l < jtx.jesd_l; l++) {
                for (int32_t i = 0; i < ads10; i++) {
                    if (i + offset < octets_per_lane) {
                        if (link == 0) {
                            buffer[count] = lanes_link0[l][i + offset];
                        }
                        else {
                            buffer[count] = lanes_link1[l][i + offset];
                        }
                    }
                    count++;
                }
            }
        }
    }

    return 0;
}