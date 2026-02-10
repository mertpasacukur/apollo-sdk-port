#if defined(__linux__)

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
#include "adi_fpga_apollo_transmit.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_vector.h"

#define FPGA_MIN_VC_SAMPLES 8192

static int32_t file_line_count(char *file, uint32_t max_samples, uint32_t *num_samples);

int32_t adi_ads10_apollo_ex_vec_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, adi_vector_generator vec_gen, void *config)
{
    void *configs[1] = {config};
    adi_vector_generator vec_gens[1] = {vec_gen};

    return adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, sides, vec_len, vec_gens, configs, 1);
}

int32_t adi_ads10_apollo_ex_vec_multiconfig_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count)
{
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);

    int16_t* tx_vec_buffer;
    uint32_t tx_vec_buffer_size;
    int32_t err;
    uint8_t side;
    uint8_t num_vc;
    
    if (vec_len % 128 != 0) {
        printf("FPGA requires vectors to be integer multiples of 128\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    for (int s = 0; s < ADI_APOLLO_NUM_SIDES; s++) {
        side = sides & (1 << s);
        if (side) {
            
            /* Adjust vec_len to be a multiple of JESD S param */
            vec_len = (vec_len * (profile->jrx[s].rx_link_cfg->s_minus1 + 1)) / adi_api_utils_gcd(vec_len, (profile->jrx[s].rx_link_cfg->s_minus1 + 1));

            num_vc = (profile->jrx[s].rx_link_cfg[0].m_minus1 + 1);
            tx_vec_buffer_size = num_vc * vec_len;
            tx_vec_buffer = malloc(sizeof(int16_t) * tx_vec_buffer_size);

            if (err = adi_vector_buffer_multiconfig_generate(tx_vec_buffer, tx_vec_buffer_size, vec_len, vec_gen, config, vec_count), err != API_CMS_ERROR_OK) {
                printf("Error in vec_gen %d\n", err);
                free(tx_vec_buffer);
                return err;
            }
            if (err = adi_fpga_apollo_transmit_write(fpga_device, side, tx_vec_buffer, tx_vec_buffer_size), err != API_CMS_ERROR_OK) {
                printf("Error in adi_fpga_apollo_transmit_write %d\n", err);
                free(tx_vec_buffer);
                return err;
            }

            free(tx_vec_buffer);
        }
    }

    return err;
}

int32_t adi_ads10_apollo_ex_vec_cmplx_tone_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, double tone_ratio, double backoff_db)
{
    return adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(fpga_device, profile, sides, vec_len, tone_ratio, 0.0, backoff_db);
}

int32_t adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, double tone_ratio, double tone_ratio_incr, double backoff_db)
{
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    int32_t err;
    uint32_t num_vcs;
    adi_vector_generator *vecgens;
    adi_vector_sine_config_t *iq_configs;
    void **configs;

    for (int s = 0; s < ADI_APOLLO_NUM_SIDES; s++) {

        num_vcs = (profile->jrx[s].rx_link_cfg[0].m_minus1 + 1);

        if (sides & (1 << s)) {

            vecgens = (adi_vector_generator *) malloc(num_vcs * sizeof(adi_vector_generator));
            iq_configs = (adi_vector_sine_config_t *)malloc((num_vcs) * sizeof(adi_vector_sine_config_t));
            configs = malloc(num_vcs * sizeof(void *));

            for (int i = 0; i < num_vcs; i++) {
                    iq_configs[i].ratio = tone_ratio + (tone_ratio_incr * (i/2));
                    iq_configs[i].backoff = backoff_db;
                    iq_configs[i].phase = (i % 2) == 0 ? 90.0 : 0.0;
                    iq_configs[i].resolution = profile->jrx[s].rx_link_cfg[0].np_minus1 + 1;
                    vecgens[i] = &adi_vector_generate_sine;
                    configs[i] = (void *)&iq_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, 1 << s, vec_len, vecgens, configs, num_vcs);

            free(vecgens);
            free(iq_configs);
            free(configs);

            if (err) {
                break;
            }
        }
    }

    return err;
}

int32_t adi_ads10_apollo_ex_vec_square_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, 
                                             uint32_t div_a, uint32_t div_b, uint8_t is_leading, double duty_cycle, double backoff_db)
{
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    ADI_CMS_INVALID_PARAM_CHECK(div_a < 2);
    ADI_CMS_INVALID_PARAM_CHECK(div_b < 2);
    ADI_CMS_RANGE_CHECK(duty_cycle, 0.0, 1.0);

    int32_t err;
    uint32_t num_vcs;
    adi_vector_generator *vecgens;
    adi_vector_square_config_t *vec_configs;
    uint32_t vec_len;
    void **configs;
    uint32_t sides;
    
    sides = ((div_a != 0) ? ADI_APOLLO_SIDE_A : 0) | ((div_b != 0) ? ADI_APOLLO_SIDE_B : 0);
    
    if (sides == 0) {
        printf("No side divide value provided\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    if (sides == ADI_APOLLO_SIDE_ALL) {
        vec_len = adi_api_utils_lcm(div_b, adi_api_utils_lcm(div_a, FPGA_MIN_VC_SAMPLES));
    } else if (sides == ADI_APOLLO_SIDE_A) {
        vec_len = adi_api_utils_lcm(div_a, FPGA_MIN_VC_SAMPLES);
    } else {
        vec_len = adi_api_utils_lcm(div_b, FPGA_MIN_VC_SAMPLES);
    }

    for (int s = 0; s < ADI_APOLLO_NUM_SIDES; s++) {

        num_vcs = (profile->jrx[s].rx_link_cfg[0].m_minus1 + 1);

        if (sides & (1 << s)) {

            vecgens = (adi_vector_generator *)malloc(num_vcs * sizeof(adi_vector_generator));
            vec_configs = (adi_vector_square_config_t *)malloc((num_vcs) * sizeof(adi_vector_square_config_t));
            configs = malloc(num_vcs * sizeof(void *));

            for (int i = 0; i < num_vcs; i++) {
                vec_configs[i].div = (s == 0) ? div_a : div_b;
                vec_configs[i].is_leading = is_leading;
                vec_configs[i].duty_cycle = duty_cycle;
                vec_configs[i].resolution = profile->jrx[s].rx_link_cfg[0].np_minus1 + 1;
                vec_configs[i].backoff = backoff_db;
                vecgens[i] = &adi_vector_generate_square;
                configs[i] = (void *)&vec_configs[i];
            }

            err = adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, 1 << s, vec_len, vecgens, configs, num_vcs);

            free(vecgens);
            free(vec_configs);
            free(configs);

            if (err) {
                break;
            }
        }
    }

    return err;
}

int32_t adi_ads10_apollo_ex_vec_ramp_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, int8_t step)
{
    adi_vector_ramp_config_t config = {
        .step = step,
        .start = -vec_len / 2,
        .stop = vec_len / 2 - 1
    };
    return adi_ads10_apollo_ex_vec_write(fpga_device, profile, sides, vec_len, adi_vector_generate_ramp, &config);
}

int32_t adi_ads10_apollo_ex_vec_pulse_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, double duty_cycle, double amplitude, double offset)
{
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile); 
    
    adi_vector_pulse_config_t config = {
        .amplitude = amplitude,
        .duty_cycle = duty_cycle,
        .offset = offset,
        .resolution = profile->jrx[0].rx_link_cfg[0].np_minus1 + 1,
        .pulse_start = 0.0
    };
    return adi_ads10_apollo_ex_vec_write(fpga_device, profile, sides, vec_len, adi_vector_generate_pulse, &config);
}

int32_t adi_ads10_apollo_ex_vec_files_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t max_samples, char *i_file, char *q_file, double scalar) 
{
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    ADI_CMS_NULL_PTR_CHECK(i_file);
    ADI_CMS_NULL_PTR_CHECK(q_file);
    
    adi_vector_generator vecgens[2] = {&adi_vector_generate_from_file, &adi_vector_generate_from_file};
    adi_vector_file_config_t i_config = {
        .scalar = scalar
    };
    adi_vector_file_config_t q_config = {
        .scalar = scalar
    };
    uint32_t vec_len;
    strcpy(i_config.file, i_file);
    strcpy(q_config.file, q_file);
    void * config[2] = {&i_config, &q_config};
    int16_t zero = 0;
    if (q_file == NULL) {
        config[1] = &zero;
    }
    file_line_count(i_file, max_samples, &vec_len);
    return adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, sides, vec_len, vecgens, config, 2);
}

int32_t adi_ads10_apollo_ex_vec_constants_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, int16_t offset) 
{
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    
    adi_vector_generator vecgens[4] = {adi_vector_generate_constant, adi_vector_generate_constant, adi_vector_generate_constant, adi_vector_generate_constant};
    int16_t zero = 0 + offset, one = 1 + offset, two = 2 + offset, three = 3 + offset;
    void * config[4] = {&zero, &one, &two, &three};
    return adi_ads10_apollo_ex_vec_multiconfig_write(fpga_device, profile, sides, vec_len, vecgens, config, 4);
}

static int32_t file_line_count(char *file, uint32_t max_samples, uint32_t *num_samples)
{
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Couldn't load file\n");
        return API_CMS_ERROR_FILE_OPEN;
    }

    *num_samples = 0;
    const int max_num_str_len = 16;
    char num_str[max_num_str_len];

    for (uint32_t i = 0; i < max_samples; i++){
        if (fgets(num_str, max_num_str_len, fp) != NULL) {
            *num_samples = *num_samples + 1;
        }
        else {
            break;
        }
    }
    fclose(fp);
    return API_CMS_ERROR_OK;
}
#endif /* defined(__linux__) */
