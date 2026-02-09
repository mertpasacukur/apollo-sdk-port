/*!
 * \brief     Apollo vector generation
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_vector.h"

#define DEBUG_VECTOR_FILE "debug_vector.txt"

static int32_t vector_buffer_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count, uint8_t debug);

int32_t adi_vector_buffer_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen, void *config)
{
    void *configs[1] = {config};
    adi_vector_generator vec_gens[1] = {vec_gen};
    return adi_vector_buffer_multiconfig_generate(samples, sample_count, vec_len, vec_gens, configs, 1);
}

int32_t adi_vector_buffer_multiconfig_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count)
{
    return vector_buffer_generate(samples, sample_count, vec_len, vec_gen, config, vec_count, 0);
}

int32_t adi_vector_buffer_multiconfig_debug_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count)
{
    return vector_buffer_generate(samples, sample_count, vec_len, vec_gen, config, vec_count, 1);
}

static int32_t vector_buffer_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count, uint8_t debug)
{
    int32_t err;
    uint8_t num_vc;

    if (sample_count % vec_len != 0) {
        return -1;
    }

    num_vc = sample_count / vec_len;

    for (int i = 0; i < num_vc; i++) {
        if (err = vec_gen[i%vec_count](&samples[i*vec_len], vec_len, config[i%vec_count]), err != 0) {
            return err;
        }
    }

    if (debug) {
        FILE * fp = fopen("debug_vector.txt", "w+");
        for (int i = 0; i < num_vc; i++) {
            fprintf(fp, "m%d\n", i);
            for (int j = 0; j < vec_len; j++) {
                    fprintf(fp, "%d\n",samples[i*vec_len + j]);
            }
        }
        fclose(fp);
        }

    return err;
}
