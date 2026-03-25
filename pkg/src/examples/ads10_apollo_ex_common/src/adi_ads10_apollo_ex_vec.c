#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples common vector related functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_apollo.h"
#include "adi_utils.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_transmit.h"
#include "adi_fpga_apollo_vec_grp.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_vector.h"

#define FPGA_MIN_VC_SAMPLES 8192
#define NUM_SAMPLES_PER_VC_CHUNK_SIZE 64 * 1024

#define LINK_SEL_MASK(profile, links) ((links) & (((profile)->jrx[0].common_link_cfg.dual_link ? 0x03 : 0x01) | ((profile)->jrx[1].common_link_cfg.dual_link ? 0x0c : 0x04)))

static int32_t file_sample_count_get(char *file_name, uint32_t max_samples, uint32_t *num_samples);
static uint32_t s_fpga_calc(adi_apollo_top_t *profile, int side, int link_side_idx);
static uint32_t vec_len_align(uint32_t vec_len, uint32_t s_fpga);

int32_t adi_ads10_apollo_ex_vec_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                      adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                      uint32_t vec_len, adi_vector_loader_t vec_loader, void *config)
{
    adi_vector_loader_t vec_loaders[1] = {vec_loader};
    void *configs[1] = {config};

    return adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, links, vec_len, vec_loaders, configs, 1);
}

int32_t adi_ads10_apollo_ex_vec_multiconfig_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, 
                                                       adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, 
                                                       uint32_t vec_len, adi_vector_loader_t vec_loader[], void *config[], uint8_t vec_count)
{
    int16_t *tx_vec_buffer;
    int32_t err;
    uint8_t side;
    uint8_t link;
    uint8_t link_side_idx;
    uint8_t num_vc;
    uint32_t vc_sample_blk_sz;

    uint32_t vc_samples_remaining;
    uint32_t vc_samples_chunk;
    uint32_t s_fpga;
    adi_vector_generator vec_gens[16]; // max vcs per side

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);

    links = LINK_SEL_MASK(profile, links);

    if (vec_len % 128 != 0) {
        printf("FPGA requires vectors to be integer multiples of 128\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            printf("Processing %d samples for link select 0x%02x\n", vec_len, (1 << link));

            for (int vl = 0; vl < vec_count; vl++) {
                (vec_loader + vl)->init(config[vl]);
                vec_gens[vl] = (vec_loader + vl)->generate;
            }

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            err = adi_fpga_apollo_vec_grp_len_set(fpga_device, vec_grp, (1 << link), 0);
            ADI_CMS_ERROR_RETURN(err);

            num_vc = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);      // number of VCs per link

            // Make vc_sample_blk_sz a multiple of FPGA S param. Ensures aligned writes.
            s_fpga = s_fpga_calc(profile, side, link_side_idx);
            vc_sample_blk_sz = ADI_UTILS_MAX((NUM_SAMPLES_PER_VC_CHUNK_SIZE / s_fpga) * s_fpga, s_fpga);

            tx_vec_buffer = malloc(sizeof(int16_t) * num_vc * vc_sample_blk_sz);        // Samples chunk buffer
            ADI_CMS_MEM_ALLOC_CHECK(tx_vec_buffer);

            vc_samples_remaining = vec_len;

            while (vc_samples_remaining > 0) {
                vc_samples_chunk = (vc_samples_remaining > vc_sample_blk_sz) ? vc_sample_blk_sz : vc_samples_remaining;

                if (err = adi_vector_buffer_multiconfig_generate(tx_vec_buffer, num_vc * vc_samples_chunk, vc_samples_chunk, vec_gens, config, vec_count), err != API_CMS_ERROR_OK) {
                    printf("Error in vec_gen %d\n", err);
                    free(tx_vec_buffer);
                    return err;
                }

                if (err = adi_fpga_apollo_transmit_write2(fpga_device, vec_grp, (1 << link), tx_vec_buffer, num_vc * vc_samples_chunk), err != API_CMS_ERROR_OK) {
                    printf("Error in adi_fpga_apollo_transmit_write2 %d\n", err);
                    free(tx_vec_buffer);
                    return err;
                }

                vc_samples_remaining -= vc_samples_chunk;
            }

            ADI_CMS_MEM_ALLOC_FREE(tx_vec_buffer);

            for (int vl = 0; vl < vec_count; vl++) {
                (vec_loader + vl)->deinit(config[vl]);
            }
        }
    }
    
    return err;
}


int32_t adi_ads10_apollo_ex_vec_cmplx_tone_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                                      adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                                      uint32_t vec_len, double tone_ratio, double backoff_db)
{
    return adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(fpga_device, profile, vec_grp, links, vec_len, tone_ratio, 0.0, backoff_db);
}

int32_t adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                                           adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                                           uint32_t vec_len, double tone_ratio, double tone_ratio_incr, double backoff_db)
{
    int32_t err;
    int link;
    int link_side_idx;
    int side;
    uint32_t num_vcs;
    uint32_t vec_len_aligned;
    adi_vector_sine_config_t *vc_configs;
    void **configs;
    adi_vector_loader_t *vec_loaders;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    links = LINK_SEL_MASK(profile, links);

    if (vec_grp == NULL) {
        adi_fpga_apollo_vec_grp_t *vec_grp = adi_fpga_apollo_vec_grp_create("cmplx tone");
        ADI_CMS_MEM_ALLOC_CHECK(vec_grp);
        err = adi_fpga_apollo_vec_grp_init(fpga_device, vec_grp);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(fpga_device, profile, vec_grp, links, vec_len, tone_ratio, tone_ratio_incr, backoff_db);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_ptn_vec_group_load(fpga_device, vec_grp, links);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_vec_grp_destroy(vec_grp);
        ADI_CMS_ERROR_RETURN(err);

        return API_CMS_ERROR_OK;
    }

    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            num_vcs = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);
            
            /* Adjust vec_len to be a multiple of FPGA S param. Ensures aligned writes. */
            vec_len_aligned = vec_len_align(vec_len, s_fpga_calc(profile, side, link_side_idx));

            vec_loaders = (adi_vector_loader_t *)malloc(num_vcs * sizeof(adi_vector_loader_t));
            ADI_CMS_MEM_ALLOC_CHECK(vec_loaders);
            vc_configs = (adi_vector_sine_config_t *)malloc(num_vcs * sizeof(adi_vector_sine_config_t));
            ADI_CMS_MEM_ALLOC_CHECK(vc_configs);
            configs = (void **)malloc(num_vcs * sizeof(void *));
            ADI_CMS_MEM_ALLOC_CHECK(configs);

            for (int i = 0; i < num_vcs; i++) {
                vec_loaders[i].generate = &adi_vector_generate_sine;
                vec_loaders[i].init = &adi_vector_nop;
                vec_loaders[i].deinit = &adi_vector_nop;

                vc_configs[i].ratio = tone_ratio + (tone_ratio_incr * (i / 2));
                vc_configs[i].backoff = backoff_db;
                vc_configs[i].phase = (i % 2) == 0 ? 90.0 : 0.0;
                vc_configs[i].resolution = profile->jrx[side].rx_link_cfg[link_side_idx].np_minus1 + 1;
                vc_configs[i].sample_total = vec_len_aligned;
                vc_configs[i].sample_idx = 0;

                configs[i] = (void *)&vc_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, (1 << link), vec_len_aligned, vec_loaders, configs, num_vcs);

            ADI_CMS_MEM_ALLOC_FREE(vec_loaders);
            ADI_CMS_MEM_ALLOC_FREE(vc_configs);
            ADI_CMS_MEM_ALLOC_FREE(configs);

            if (err) {
                printf("Error processing link 0x%02x. Error = %d\n", (1<<link), err);
                break;
            }
        }
    }

    return err;
}

int32_t adi_ads10_apollo_ex_vec_square_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                                  adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                                  uint32_t vec_len, uint32_t div, uint8_t is_leading, double duty_cycle, double backoff_db)
{
    int32_t err;
    int link;
    int link_side_idx;
    int side;
    uint32_t num_vcs;
    uint32_t vec_len_aligned;
    adi_vector_square_config_t *vc_configs;
    void **configs;
    adi_vector_loader_t *vec_loaders;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    links = LINK_SEL_MASK(profile, links);

    if (vec_grp == NULL) {
        adi_fpga_apollo_vec_grp_t *vec_grp = adi_fpga_apollo_vec_grp_create("square");
        ADI_CMS_MEM_ALLOC_CHECK(vec_grp);
        err = adi_fpga_apollo_vec_grp_init(fpga_device, vec_grp);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_vec_square_write(fpga_device, profile, vec_grp, links, vec_len, div, is_leading, duty_cycle, backoff_db);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_ptn_vec_group_load(fpga_device, vec_grp, links);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_vec_grp_destroy(vec_grp);
        ADI_CMS_ERROR_RETURN(err);

        return API_CMS_ERROR_OK;
    }

    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            num_vcs = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);

            /* Adjust vec_len to be a multiple of FPGA S param. Ensures aligned writes. */
            vec_len_aligned = vec_len_align(vec_len, s_fpga_calc(profile, side, link_side_idx));

            vec_loaders = (adi_vector_loader_t *)malloc(num_vcs * sizeof(adi_vector_loader_t));
            ADI_CMS_MEM_ALLOC_CHECK(vec_loaders);
            vc_configs = (adi_vector_square_config_t *)malloc(num_vcs * sizeof(adi_vector_square_config_t));
            ADI_CMS_MEM_ALLOC_CHECK(vc_configs);
            configs = (void **)malloc(num_vcs * sizeof(void *));
            ADI_CMS_MEM_ALLOC_CHECK(configs);

            for (int i = 0; i < num_vcs; i++) {
                vec_loaders[i].generate = &adi_vector_generate_square;
                vec_loaders[i].init = &adi_vector_nop;
                vec_loaders[i].deinit = &adi_vector_nop;

                vc_configs[i].div = div;
                vc_configs[i].is_leading = is_leading;
                vc_configs[i].duty_cycle = duty_cycle;
                vc_configs[i].resolution = profile->jrx[side].rx_link_cfg[link_side_idx].np_minus1 + 1;
                vc_configs[i].backoff = backoff_db;
                vc_configs[i].sample_total = vec_len_aligned;
                vc_configs[i].sample_idx = 0;

                configs[i] = (void *)&vc_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, (1 << link), vec_len_aligned, vec_loaders, configs, num_vcs);

            ADI_CMS_MEM_ALLOC_FREE(vec_loaders);
            ADI_CMS_MEM_ALLOC_FREE(vc_configs);
            ADI_CMS_MEM_ALLOC_FREE(configs);

            if (err) {
                printf("Error processing link 0x%02x. Error = %d\n", (1<<link), err);
                break;
            }
        }
    }

    return err;
}

int32_t adi_ads10_apollo_ex_vec_ramp_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                           adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint32_t vec_len,
                                           int16_t start, int16_t stop, int16_t step)
{
    int32_t err;
    int link;
    int link_side_idx;
    int side;
    uint32_t num_vcs;
    uint32_t vec_len_aligned;
    adi_vector_ramp_config_t *vc_configs;
    void **configs;
    adi_vector_loader_t *vec_loaders;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    links = LINK_SEL_MASK(profile, links);

    if (vec_grp == NULL) {
        adi_fpga_apollo_vec_grp_t *vec_grp = adi_fpga_apollo_vec_grp_create("ramp");
        ADI_CMS_MEM_ALLOC_CHECK(vec_grp);
        err = adi_fpga_apollo_vec_grp_init(fpga_device, vec_grp);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_vec_ramp_write(fpga_device, profile, vec_grp, links, vec_len, start, stop, step);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_ptn_vec_group_load(fpga_device, vec_grp, links);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_vec_grp_destroy(vec_grp);
        ADI_CMS_ERROR_RETURN(err);

        return API_CMS_ERROR_OK;
    }

    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            num_vcs = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);

            /* Adjust vec_len to be a multiple of FPGA S param. Ensures aligned writes. */
            vec_len_aligned = vec_len_align(vec_len, s_fpga_calc(profile, side, link_side_idx));

            vec_loaders = (adi_vector_loader_t *)malloc(num_vcs * sizeof(adi_vector_loader_t));
            ADI_CMS_MEM_ALLOC_CHECK(vec_loaders);
            vc_configs = (adi_vector_ramp_config_t *)malloc(num_vcs * sizeof(adi_vector_ramp_config_t));
            ADI_CMS_MEM_ALLOC_CHECK(vc_configs);
            configs = (void **)malloc(num_vcs * sizeof(void *));
            ADI_CMS_MEM_ALLOC_CHECK(configs);

            for (int i = 0; i < num_vcs; i++) {
                vec_loaders[i].generate = &adi_vector_generate_ramp;
                vec_loaders[i].init = &adi_vector_nop;
                vec_loaders[i].deinit = &adi_vector_nop;

                vc_configs[i].start = start;
                vc_configs[i].stop = stop;
                vc_configs[i].step = step;
                vc_configs[i].sample_total = vec_len_aligned;
                vc_configs[i].sample_idx = 0;

                configs[i] = (void *)&vc_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, (1 << link), vec_len_aligned, vec_loaders, configs, num_vcs);

            ADI_CMS_MEM_ALLOC_FREE(vec_loaders);
            ADI_CMS_MEM_ALLOC_FREE(vc_configs);
            ADI_CMS_MEM_ALLOC_FREE(configs);

            if (err) {
                printf("Error processing link 0x%02x. Error = %d\n", (1<<link), err);
                break;
            }
        }
    }
    return err;
}

int32_t adi_ads10_apollo_ex_vec_pulse_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint32_t vec_len, double duty_cycle, double amplitude, double offset)
{
    int32_t err;
    int link;
    int link_side_idx;
    int side;
    uint32_t num_vcs;
    uint32_t vec_len_aligned;
    adi_vector_pulse_config_t *vc_configs;
    void **configs;
    adi_vector_loader_t *vec_loaders;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    links = LINK_SEL_MASK(profile, links);
    
    if (vec_grp == NULL) {
        adi_fpga_apollo_vec_grp_t *vec_grp = adi_fpga_apollo_vec_grp_create("pulse");
        ADI_CMS_MEM_ALLOC_CHECK(vec_grp);
        err = adi_fpga_apollo_vec_grp_init(fpga_device, vec_grp);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_vec_pulse_write(fpga_device, profile, vec_grp, links, vec_len, duty_cycle, amplitude, offset);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_ptn_vec_group_load(fpga_device, vec_grp, links);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_vec_grp_destroy(vec_grp);
        ADI_CMS_ERROR_RETURN(err);

        return API_CMS_ERROR_OK;
    }

    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            num_vcs = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);

            /* Adjust vec_len to be a multiple of FPGA S param. Ensures aligned writes. */
            vec_len_aligned = vec_len_align(vec_len, s_fpga_calc(profile, side, link_side_idx));

            vec_loaders = (adi_vector_loader_t *)malloc(num_vcs * sizeof(adi_vector_loader_t));
            ADI_CMS_MEM_ALLOC_CHECK(vec_loaders);
            vc_configs = (adi_vector_pulse_config_t *)malloc(num_vcs * sizeof(adi_vector_pulse_config_t));
            ADI_CMS_MEM_ALLOC_CHECK(vc_configs);
            configs = (void **)malloc(num_vcs * sizeof(void *));
            ADI_CMS_MEM_ALLOC_CHECK(configs);

            for (int i = 0; i < num_vcs; i++) {
                vec_loaders[i].generate = &adi_vector_generate_pulse;
                vec_loaders[i].init = &adi_vector_nop;
                vec_loaders[i].deinit = &adi_vector_nop;

                vc_configs[i].amplitude = amplitude;
                vc_configs[i].duty_cycle = duty_cycle;
                vc_configs[i].offset = offset;
                vc_configs[i].resolution = profile->jrx[link_side_idx].rx_link_cfg[link].np_minus1 + 1;
                vc_configs[i].pulse_start = 0.0;
                vc_configs[i].sample_total = vec_len_aligned;
                vc_configs[i].sample_idx = 0;

                configs[i] = (void *)&vc_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, (1 << link), vec_len_aligned, vec_loaders, configs, num_vcs);

            ADI_CMS_MEM_ALLOC_FREE(vec_loaders);
            ADI_CMS_MEM_ALLOC_FREE(vc_configs);
            ADI_CMS_MEM_ALLOC_FREE(configs);

            if (err) {
                printf("Error processing link 0x%02x. Error = %d\n", (1<<link), err);
                break;
            }
        }
    }
    return err;  
}

int32_t adi_ads10_apollo_ex_vec_files_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint32_t max_samples, char *i_file, char *q_file, double scalar)
{
    int32_t err;
    int link;
    int link_side_idx;
    int side;
    uint32_t num_vcs;
    uint32_t i_vec_len;
    uint32_t q_vec_len;
    uint32_t vec_len;
    adi_vector_loader_t *vec_loaders;
    adi_vector_file_config_t *vc_configs;
    void **configs;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    ADI_CMS_NULL_PTR_CHECK(i_file);
    ADI_CMS_NULL_PTR_CHECK(q_file);

    links = LINK_SEL_MASK(profile, links);

    if (vec_grp == NULL) {
        adi_fpga_apollo_vec_grp_t *vec_grp = adi_fpga_apollo_vec_grp_create("files");
        ADI_CMS_MEM_ALLOC_CHECK(vec_grp);
        err = adi_fpga_apollo_vec_grp_init(fpga_device, vec_grp);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_vec_files_write(fpga_device, profile, vec_grp, links, max_samples, i_file, q_file, scalar);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_ptn_vec_group_load(fpga_device, vec_grp, links);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_vec_grp_destroy(vec_grp);
        ADI_CMS_ERROR_RETURN(err);

        return API_CMS_ERROR_OK;
    }

    vec_len = max_samples;
    if (max_samples == 0) {
        err = file_sample_count_get(i_file, 0, &i_vec_len);
        ADI_CMS_ERROR_RETURN(err);
        err = file_sample_count_get(q_file, 0, &q_vec_len);
        ADI_CMS_ERROR_RETURN(err);

        if (i_vec_len != q_vec_len) {
            return API_CMS_ERROR_VEC_IQ_LEN_MISMATCH;
        }

        vec_len = i_vec_len;
    }
    
    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            num_vcs = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);

            vec_loaders = (adi_vector_loader_t *)malloc(num_vcs * sizeof(adi_vector_loader_t));
            ADI_CMS_MEM_ALLOC_CHECK(vec_loaders);
            vc_configs = (adi_vector_file_config_t *)malloc(num_vcs * sizeof(adi_vector_file_config_t));
            ADI_CMS_MEM_ALLOC_CHECK(vc_configs);
            configs = (void **)malloc(num_vcs * sizeof(void *));
            ADI_CMS_MEM_ALLOC_CHECK(configs);

            for (int i = 0; i < num_vcs; i++) {
                vec_loaders[i].generate = &adi_vector_file_generate;
                vec_loaders[i].init = &adi_vector_file_init;
                vec_loaders[i].deinit = adi_vector_file_deinit;

                vc_configs[i].scalar = scalar;
                strcpy(vc_configs[i].file, (i % 2) ? q_file : i_file);
                vc_configs[i].sample_total = vec_len;
                vc_configs[i].sample_idx = 0;

                configs[i] = (void *)&vc_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, (1 << link), vec_len, vec_loaders, configs, num_vcs);

            ADI_CMS_MEM_ALLOC_FREE(vec_loaders);
            ADI_CMS_MEM_ALLOC_FREE(vc_configs);
            ADI_CMS_MEM_ALLOC_FREE(configs);

            if (err) {
                printf("Error processing link 0x%02x. Error = %d\n", (1<<link), err);
                break;
            }
            
        }
    }

    return err; 
}

int32_t adi_ads10_apollo_ex_vec_constants_incr_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint32_t vec_len, int16_t offset, int16_t incr) 
{
    int32_t err;
    int link;
    int link_side_idx;
    int side;
    uint32_t num_vcs;
    uint32_t vec_len_aligned;
    adi_vector_constant_config_t *vc_configs;
    void **configs;
    adi_vector_loader_t *vec_loaders;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    links = LINK_SEL_MASK(profile, links);

    if (vec_grp == NULL) {
        adi_fpga_apollo_vec_grp_t *vec_grp = adi_fpga_apollo_vec_grp_create("constants");
        ADI_CMS_MEM_ALLOC_CHECK(vec_grp);
        err = adi_fpga_apollo_vec_grp_init(fpga_device, vec_grp);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_vec_constants_incr_write(fpga_device, profile, vec_grp, links, vec_len, offset, incr);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_ptn_vec_group_load(fpga_device, vec_grp, links);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_vec_grp_destroy(vec_grp);
        ADI_CMS_ERROR_RETURN(err);

        return API_CMS_ERROR_OK;
    }

    for (link = 0; link < ADI_APOLLO_NUM_JRX_LINKS; link++) {
        if ((1 << link) & links) {

            side = link / ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;
            link_side_idx = link % ADI_APOLLO_NUM_JRX_LINKS_PER_SIDE;

            num_vcs = (profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1);

            /* Adjust vec_len to be a multiple of FPGA S param. Ensures aligned writes. */
            vec_len_aligned = vec_len_align(vec_len, s_fpga_calc(profile, side, link_side_idx));

            vec_loaders = (adi_vector_loader_t *)malloc(num_vcs * sizeof(adi_vector_loader_t));
            ADI_CMS_MEM_ALLOC_CHECK(vec_loaders);
            vc_configs = (adi_vector_constant_config_t *)malloc(num_vcs * sizeof(adi_vector_constant_config_t));
            ADI_CMS_MEM_ALLOC_CHECK(vc_configs);
            configs = (void **)malloc(num_vcs * sizeof(void *));
            ADI_CMS_MEM_ALLOC_CHECK(configs);

            for (int i = 0; i < num_vcs; i++) {
                vec_loaders[i].generate = &adi_vector_generate_constant;
                vec_loaders[i].init = &adi_vector_nop;
                vec_loaders[i].deinit = &adi_vector_nop;

                vc_configs[i].value = offset + (i * incr);     // increment constant per vc
                vc_configs[i].sample_total = vec_len_aligned;
                vc_configs[i].sample_idx = 0;

                configs[i] = (void *)&vc_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, vec_grp, (1 << link), vec_len_aligned, vec_loaders, configs, num_vcs);

            ADI_CMS_MEM_ALLOC_FREE(vec_loaders);
            ADI_CMS_MEM_ALLOC_FREE(vc_configs);
            ADI_CMS_MEM_ALLOC_FREE(configs);

            if (err) {
                printf("Error processing link 0x%02x. Error = %d\n", (1<<link), err);
                break;
            }
        }
    }
    
    return err;
}

static int32_t file_sample_count_get(char *file_name, uint32_t max_samples, uint32_t *num_samples)
{
    char line[256];

    ADI_CMS_NULL_PTR_CHECK(file_name);
    ADI_CMS_NULL_PTR_CHECK(num_samples);

    /* Open file of samples */
    FILE *fp = fopen(file_name, "r");
    ADI_CMS_FILE_OPEN_CHECK(fp);

    *num_samples = 0;

    /* get number of samples in file */
    while (fgets(line, sizeof(line), fp) != NULL) {
        (*num_samples)++;

        if (max_samples && (*num_samples == max_samples)) {
            break;
        }
    }

    if (0 != ferror(fp)) {
        return API_CMS_ERROR_FILE_OPERATION;
    }

    ADI_CMS_FILE_CLOSE(fp);

    return API_CMS_ERROR_OK;
}

// Return FPGA S parameter based on profile settings. Represents samples/conv/fpga frame
static uint32_t s_fpga_calc(adi_apollo_top_t *profile, int side, int link_side_idx)
{
    uint32_t s_fpga;
    uint32_t l, m, np;

    l = profile->jrx[side].rx_link_cfg[link_side_idx].l_minus1 + 1;
    m = profile->jrx[side].rx_link_cfg[link_side_idx].m_minus1 + 1;
    np = profile->jrx[side].rx_link_cfg[link_side_idx].np_minus1 + 1;

    s_fpga = (128 * l * 8) / (np * m);

    return s_fpga;
}

// Align vec_len to multiple of at least 4k samples (mitigates ads10 tx when jesd s=6)
static uint32_t vec_len_align(uint32_t vec_len, uint32_t s_fpga)
{
    uint32_t vec_len_aligned;
    vec_len_aligned = adi_api_utils_lcm(4 * 1024, s_fpga);
    vec_len_aligned = ((vec_len + vec_len_aligned - 1) / vec_len_aligned) * vec_len_aligned; // Round up requested vec_len to nearest multiple
    return vec_len_aligned;
}

#endif /* !defined(VERSAL_PLATFORM) */
