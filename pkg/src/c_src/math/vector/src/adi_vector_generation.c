/*!
 * @brief     ADI vector generators
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include <math.h>
#include "adi_vector_generation.h"

/*============= C O D E ====================*/

static int32_t file_16b_read(FILE *fp, int16_t *sample_arr, uint32_t max_samples, uint32_t *num_samples, double scalar);

int32_t adi_vector_generate_constant(int16_t samples[], uint32_t sample_count, void *config)
{
    uint32_t i;
    adi_vector_constant_config_t *cfg;

    ADI_CMS_NULL_PTR_CHECK(samples);
    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_constant_config_t *)config;

    for (i = 0; i < sample_count; i++) {
        samples[i] = cfg->value;
    }

    cfg->sample_idx += sample_count;

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_generate_sine(int16_t samples[], uint32_t sample_count, void *config)
{
    double tone_ratio_out = 0;

    return adi_vector_generate_sine_v2(samples, sample_count, config, &tone_ratio_out);
}

int32_t adi_vector_generate_sine_v2(int16_t samples[], uint32_t sample_count, void *config, double *tone_ratio_out)
{
    adi_vector_sine_config_t *cfg;
    double phase_rads;
    double f_tone;
    double f_data_rate;
    uint8_t resolution;
    uint32_t m_cycles;
    double ratio;
    double amplitude;
    double backoff_gain;

    ADI_CMS_NULL_PTR_CHECK(samples);
    ADI_CMS_NULL_PTR_CHECK(config);
    ADI_CMS_NULL_PTR_CHECK(tone_ratio_out);

    cfg = (adi_vector_sine_config_t *)config;
    phase_rads = cfg->phase * M_PI / 180.0;
    f_tone = cfg->ratio;
    resolution = cfg->resolution;
    f_data_rate = 1.0;
    m_cycles = fabs(f_tone * cfg->sample_total) / f_data_rate;

    if (m_cycles % 2 == 0) {
        m_cycles++;
    }
    f_tone = (m_cycles * f_data_rate) / cfg->sample_total;

    ratio = ((2 * M_PI * f_tone) / f_data_rate);
    amplitude = (1 << (resolution - 1)) - 1;
    backoff_gain = pow(10, (cfg->backoff / 20.0));

    for (int i = 0; i < sample_count; i++) {
        samples[i] = (int)round(amplitude * backoff_gain * sin((ratio * (i + cfg->sample_idx)) + phase_rads));
    }

    cfg->sample_idx += sample_count;

    *tone_ratio_out = f_tone;

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_generate_square(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_square_config_t *cfg;
    uint32_t i;
    uint32_t samples_per_cycle;
    uint32_t mid_sample;
    uint8_t resolution;
    double amplitude;
    double backoff_gain;
    int16_t first_half_val;
    int16_t second_half_val;

    ADI_CMS_NULL_PTR_CHECK(samples);
    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_square_config_t *)config;

    samples_per_cycle = cfg->div;
    mid_sample = (samples_per_cycle * cfg->duty_cycle) + 0.5;
    resolution = cfg->resolution;
    amplitude = (1 << (resolution - 1)) - 1;
    backoff_gain = pow(10, (cfg->backoff / 20.0));
    first_half_val = cfg->is_leading ? ((int)amplitude * backoff_gain) : 0;
    second_half_val = cfg->is_leading ? 0 : ((int)amplitude * backoff_gain);

    for (i = 0; i < sample_count; i++) {
        samples[i] = (((i + cfg->sample_idx) % samples_per_cycle) < mid_sample) ? first_half_val : second_half_val;
    }

    cfg->sample_idx += sample_count;

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_generate_ramp(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_ramp_config_t *cfg;
    uint32_t i;
    int32_t value;

    ADI_CMS_NULL_PTR_CHECK(samples);
    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_ramp_config_t *)config;

    value = cfg->start;

    for (i = 0; i < sample_count; i++) {
        samples[i] = (int16_t)value;
        value += cfg->step;

        if (value > cfg->stop && cfg->step > 0) {
            value = cfg->start + (value - cfg->stop - 1);
        }
        if (value < cfg->stop && cfg->step < 0) {
            value = cfg->start - (cfg->stop - value - 1);
        }
    }

    cfg->start = value;
    cfg->sample_idx += sample_count;

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_generate_pulse(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_pulse_config_t *cfg;
    int pulse_size;
    int pulse_sample;
    int16_t amp_samp;
    int16_t offset_samp;
    int16_t hi_val;
    int16_t low_val;

    ADI_CMS_NULL_PTR_CHECK(samples);
    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_pulse_config_t*) config;
    
    pulse_size = cfg->duty_cycle * cfg->sample_total;
    pulse_sample = cfg->sample_total * cfg->pulse_start;
    amp_samp = cfg->amplitude * (cfg->resolution == 12 ? 0x7FF : 0x7FFF);
    offset_samp = cfg->offset * (cfg->resolution == 12 ? 0x7FF : 0x7FFF);
    hi_val = offset_samp + amp_samp;
    low_val = offset_samp - amp_samp;

    for (int s = 0; s < sample_count; s++) {
        if ((s + cfg->sample_idx) < pulse_sample) {
            samples[s] = low_val;
        } else if ((s + cfg->sample_idx) < (pulse_sample + pulse_size)) {
            samples[s] = hi_val;
        } else {
            samples[s] = low_val;
        }
    }

    cfg->sample_idx += sample_count;

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_file_init(void *config) 
{
    adi_vector_file_config_t *cfg;
    FILE *fp;

    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_file_config_t *) config;

    fp = fopen(cfg->file, "r");
    if (fp == NULL) {
        printf("Error opening file: %s\n", cfg->file);
        return API_CMS_ERROR_FILE_OPEN;
    }

    cfg->fp = fp;

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_file_deinit(void *config)
{
    adi_vector_file_config_t *cfg;

    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_file_config_t *)config;

    if (cfg->fp != NULL) {
        fclose(cfg->fp);
        cfg->fp = NULL;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_vector_file_generate(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_file_config_t *cfg;
    int32_t err;
    uint32_t num_read;

    ADI_CMS_NULL_PTR_CHECK(config);

    cfg = (adi_vector_file_config_t *)config;

    err = file_16b_read(cfg->fp, samples, sample_count, &num_read, cfg->scalar);
    if (err != API_CMS_ERROR_OK) {
        printf("Error reading file: %s\n", cfg->file);
        return API_CMS_ERROR_FILE_READ;
    }
    
    if (num_read != sample_count) {
        printf("Warning: not enough samples in file starting from index %u. Vector is appended with %d 0s\n", cfg->sample_idx, sample_count - num_read);
    }

    cfg->sample_idx += sample_count;

    return err;
}

int32_t adi_vector_nop(void *config)
{
    return 0;
}

static int32_t file_16b_read(FILE *fp, int16_t *sample_arr, uint32_t max_samples, uint32_t *num_samples, double scalar)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i;
    uint32_t num_samples_cnt = 0;
    const int max_num_str_len = 16;
    char num_str[max_num_str_len];
    int16_t *p16;

    p16 = sample_arr;
    for (i = 0; i < max_samples; i++) {
        if (fgets(num_str, max_num_str_len, fp) != NULL) {
            *p16++ = scalar * atoi(num_str);
            num_samples_cnt++;
        } else {
            err = API_CMS_ERROR_FILE_READ;
            break;
        }
    }

    *num_samples = num_samples_cnt;

    return err;
}

/*! @} */