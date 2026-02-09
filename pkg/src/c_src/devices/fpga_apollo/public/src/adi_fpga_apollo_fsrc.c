/*!
 * \brief     Basic FPGA Apollo functions for handling FSRC invalid samples.
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_APOLLO__
 */

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"
#include "adi_fpga_apollo_fsrc.h"
#include "adi_utils.h"

int32_t adi_fpga_apollo_fsrc_n_m_ratio_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint16_t link, uint8_t enable, uint32_t fsrc_n, uint32_t fsrc_m, uint32_t link_drc, uint32_t total_drc)
{
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(enable > 1);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);
    uint64_t rate_int = 0, rate_frac_a = 0, rate_frac_b = 0;

    /* recalcualte fsrc n and m values */
    adi_api_utils_ratio_decomposition(fsrc_m * total_drc, fsrc_n * link_drc, 48, &rate_int, &rate_frac_a, &rate_frac_b);
    adi_api_utils_ratio_composition(rate_int, rate_frac_a, rate_frac_b, &fsrc_m, &fsrc_n, 48);

    for (int i = 0; i < MAX_JESD_LINKS; i++) {
        if ((1 << i) & link) {
            if (terminal == ADI_FPGA_APOLLO_JRX) {
                    fpga->state_info.rx[i].n = fsrc_n;
                    fpga->state_info.rx[i].m = fsrc_m;
                    fpga->state_info.rx[i].fsrc = enable;
                    fpga->state_info.rx[i].link_xdrc = link_drc;
                    fpga->state_info.rx[i].tot_xdrc = total_drc;
            } else {
                    fpga->state_info.tx[i].n = fsrc_n;
                    fpga->state_info.tx[i].m = fsrc_m;
                    fpga->state_info.tx[i].fsrc = enable;
                    fpga->state_info.tx[i].link_xdrc = link_drc;
                    fpga->state_info.tx[i].tot_xdrc = total_drc;
            }
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_fsrc_invalid_ratio_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint16_t link, uint8_t enable, uint64_t rate_int, uint64_t frac_a, uint64_t frac_b, uint32_t link_drc, uint32_t total_drc)
{
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(enable > 1);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);

    uint32_t fsrc_n = 1;
    uint32_t fsrc_m = 1;

    /* If frac_a == frac_b assume FSRC is in 1x mode */
    if (frac_a != frac_b) {
        adi_api_utils_ratio_composition(rate_int, frac_a, frac_b, &fsrc_m, &fsrc_n, 48);
    }

    return adi_fpga_apollo_fsrc_n_m_ratio_set(fpga, terminal, link, enable, fsrc_n, fsrc_m, link_drc, total_drc);
}

int32_t adi_fpga_apollo_fsrc_sr_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint16_t link, uint8_t enable, uint32_t link_drc, uint32_t total_drc)
{
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);

    for (int i = 0; i < MAX_JESD_LINKS; i++) {
        if ((1 << i) & link) {
            if (terminal == ADI_FPGA_APOLLO_JRX) {
                fpga->state_info.rx[i].sr = enable;
                fpga->state_info.rx[i].link_xdrc = link_drc;
                fpga->state_info.rx[i].tot_xdrc = total_drc;
            } else {
                fpga->state_info.tx[i].sr = enable;
                fpga->state_info.tx[i].link_xdrc = link_drc;
                fpga->state_info.tx[i].tot_xdrc = total_drc;
            }
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_fsrc_tx_vector_length_get(uint32_t length, uint32_t fsrc_n, uint32_t fsrc_m,
                                                  adi_apollo_fsrc_tx_vector_length_t *vec_length)
{

    uint32_t num_total_samples, num_valid_samples, num_invalid_samples; 
    uint32_t scale, multiplier;

    ADI_CMS_NULL_PTR_CHECK(vec_length);

    num_total_samples = fsrc_n;
    num_valid_samples = fsrc_m ;
    num_invalid_samples = num_total_samples - num_valid_samples;

    /* Make it a multiple of 128 for FPGA requirements */
    scale = adi_api_utils_lcm(num_total_samples, 128) / num_total_samples;

    num_total_samples = num_total_samples * scale;
    num_valid_samples = num_valid_samples * scale;
    num_invalid_samples = num_invalid_samples * scale;

    multiplier = 1;
    
    while (true) {

        if (multiplier > 10000) {
            return API_CMS_ERROR_ERROR;
        }

        if (num_valid_samples >= length) {

            /* Multiply num of valid samples such that it becomes a multiple of length */
            if ( (num_valid_samples * multiplier) % length == 0) {
                
                /* repeat is how many times we want to repeat the length / original vec 
                   scale is how many times we wanna scale valid, invalid, total samples */                   
                scale = multiplier;
                num_valid_samples = num_valid_samples * scale;
                num_invalid_samples = num_invalid_samples * scale;
                num_total_samples = num_total_samples * scale;
                break;

            }            
                
        }
        else {

            /* valid samples < length
               Multiply Length so that it becomes a multiple of num_valid_samples */
            if ( (length * multiplier) % num_valid_samples == 0) {

                scale = (length * multiplier) / num_valid_samples;

                num_invalid_samples = num_invalid_samples * scale;
                num_total_samples = num_total_samples * scale;
                num_valid_samples = num_valid_samples * scale;
                break;
            }
        }

        multiplier++;

    }

    vec_length->num_total_samples = num_total_samples;
    vec_length->num_valid_samples = num_valid_samples;
    vec_length->num_invalid_samples = num_invalid_samples;

    return API_CMS_ERROR_OK;
}

int32_t  adi_fpga_apollo_fsrc_sample_insert(uint16_t i_vec[], uint16_t q_vec[], uint32_t length, uint8_t jrx_ns,
                                       uint16_t fsrc_i_vec[], uint16_t fsrc_q_vec[],
                                       adi_apollo_fsrc_tx_vector_length_t *vec_length)
{

    uint32_t num_total_samples, num_valid_samples, num_invalid_samples;
    int i, j;
    double desired_valid_invalid_ratio, valid_invalid_ratio;
    uint32_t valid_invalid_floor, valid_invalid_ceil;
    uint32_t total_chunk_count = 0, valid_chunk_count = 0, invalid_chunk_count = 0, sample_count = 0,
             valid_chunks_insert = 0;

    ADI_CMS_NULL_PTR_CHECK(i_vec);
    ADI_CMS_NULL_PTR_CHECK(q_vec);
    ADI_CMS_NULL_PTR_CHECK(fsrc_i_vec);
    ADI_CMS_NULL_PTR_CHECK(fsrc_q_vec);
    ADI_CMS_NULL_PTR_CHECK(vec_length);
    ADI_CMS_INVALID_PARAM_CHECK(vec_length->num_total_samples % jrx_ns != 0);
    ADI_CMS_INVALID_PARAM_CHECK((vec_length->num_valid_samples % jrx_ns != 0) || (vec_length->num_invalid_samples % jrx_ns != 0));

    num_total_samples = vec_length->num_total_samples;
    num_valid_samples = vec_length->num_valid_samples;
    num_invalid_samples = vec_length->num_invalid_samples;

    /* Check for 1x mode (i.e. no invalids) */
    if (num_invalid_samples == 0) {
        for (j = 0; j < num_total_samples; j++) {
            fsrc_i_vec[j] = i_vec[j];
            fsrc_q_vec[j] = q_vec[j];
        }
        return API_CMS_ERROR_OK;
    }

    desired_valid_invalid_ratio = (double) num_valid_samples / num_invalid_samples;
    valid_invalid_floor = num_valid_samples / num_invalid_samples;
    valid_invalid_ceil = valid_invalid_floor + 1;

    /* Insert valid_invalid_floor chunks of valid data */
    for (i = 0; i < valid_invalid_floor; i++) {
        /* Each chunk will be of size jrx_ns */
        for (j = 0; j < jrx_ns; j++) {
            fsrc_i_vec[(total_chunk_count * jrx_ns + j)] = i_vec[(valid_chunk_count * jrx_ns + j) % length];
            fsrc_q_vec[(total_chunk_count * jrx_ns + j)] = q_vec[(valid_chunk_count * jrx_ns + j) % length];
        }

        sample_count += jrx_ns;
        valid_chunk_count += 1;
        total_chunk_count += 1;

    }

    /* Insert 1 invalid chunk data */
    for (j = 0; j < jrx_ns; j++) {

        fsrc_i_vec[(total_chunk_count * jrx_ns + j)] = (uint16_t) 0x8000;
        fsrc_q_vec[(total_chunk_count * jrx_ns + j)] = (uint16_t) 0x8000;

    }

    sample_count += jrx_ns;
    invalid_chunk_count += 1;
    total_chunk_count += 1;

    /*Algorithm for maintaining desired_valid_invalid_ratio*/
    while (sample_count < num_total_samples) {
 
        valid_invalid_ratio = (double) valid_chunk_count / invalid_chunk_count;
        
        /* Get next num of valid chunks to insert */
        valid_chunks_insert = (valid_invalid_ratio >= desired_valid_invalid_ratio) ? 
                               valid_invalid_floor : valid_invalid_ceil;

        for (i = 0; i < valid_chunks_insert; i++) {

            for (j = 0; j < jrx_ns; j++) {

                fsrc_i_vec[(total_chunk_count * jrx_ns + j)] = i_vec[(valid_chunk_count * jrx_ns + j) % length];
                fsrc_q_vec[(total_chunk_count * jrx_ns + j)] = q_vec[(valid_chunk_count * jrx_ns + j) % length];
            }

            sample_count += jrx_ns;
            valid_chunk_count += 1;
            total_chunk_count += 1;
        }
        /* Insert 1 invalid chunk data */
        for (j = 0; j < jrx_ns; j++) {

            fsrc_i_vec[(total_chunk_count * jrx_ns + j)] = (uint16_t) 0x8000;
            fsrc_q_vec[(total_chunk_count * jrx_ns + j)] = (uint16_t) 0x8000;

        }

        sample_count += jrx_ns;
        invalid_chunk_count +=1;
        total_chunk_count += 1;

    }

    return API_CMS_ERROR_OK;
}