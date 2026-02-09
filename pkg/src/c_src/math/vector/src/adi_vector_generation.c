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
    for (i = 0; i < sample_count; i++) {
        samples[i] = *(int16_t*)config;
    }
    return 0;
}

int32_t adi_vector_generate_sine(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_sine_config_t *cfg = (adi_vector_sine_config_t*) config;
    double phase_rads = cfg->phase * M_PI / 180.0;
    double f_tone = cfg->ratio;
    double f_data_rate = 1.0;
    uint8_t resolution = cfg->resolution;
    uint32_t m_cycles = fabs(f_tone * sample_count) / f_data_rate;
    if (m_cycles % 2 == 0) {
        m_cycles++;
    }
    f_tone = (m_cycles * f_data_rate) / sample_count;


    double ratio = ((2 * M_PI * f_tone) / f_data_rate);
    double amplitude = (1 << (resolution - 1)) - 1;
    double backoff_gain = pow(10, (cfg->backoff / 20.0));
    for (int i = 0; i < sample_count; i++) {
        samples[i] = (int) round(amplitude * backoff_gain * sin((ratio * i) + phase_rads));
    }
    return 0;
}

int32_t adi_vector_generate_square(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_square_config_t *cfg = (adi_vector_square_config_t *)config;

    uint32_t i;
    uint32_t samples_per_cycle = cfg->div;
    uint32_t mid_sample = (samples_per_cycle * cfg->duty_cycle) + 0.5;
    uint8_t resolution = cfg->resolution;
    double amplitude = (1 << (resolution - 1)) - 1;
    double backoff_gain = pow(10, (cfg->backoff / 20.0));
    int16_t first_half_val = cfg->is_leading ? ((int)amplitude * backoff_gain) : 0;
    int16_t second_half_val = cfg->is_leading ? 0 : ((int)amplitude * backoff_gain);

    for (i = 0; i < sample_count; i++) {
        samples[i] = ((i % samples_per_cycle) < mid_sample) ? first_half_val : second_half_val;
    }
   
    return 0;
}



int32_t adi_vector_generate_ramp(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_ramp_config_t *cfg = (adi_vector_ramp_config_t*) config;
    uint16_t value = cfg->start;
    uint32_t i;
    for (i = 0; i < sample_count; i++) {
        samples[i] = (int16_t) value;
        value += cfg->step;
        if (value > cfg->stop && cfg->step > 0) {
            value = cfg->start + (value - cfg->stop - 1);
        }
        if (value < cfg->stop && cfg->step < 0) {
            value = cfg->start - (cfg->stop - value - 1);
        }
    }

    return 0;
}

int32_t adi_vector_generate_pulse(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_pulse_config_t *cfg = (adi_vector_pulse_config_t*) config;
    int pulse_size = cfg->duty_cycle * sample_count;
    int pulse_sample = sample_count * cfg->pulse_start;
    int16_t amp_samp = cfg->amplitude * (cfg->resolution == 12 ? 0x7FF : 0x7FFF);
    int16_t offset_samp = cfg->offset * (cfg->resolution == 12 ? 0x7FF : 0x7FFF);
    int16_t hi_val = amp_samp + offset_samp;
    int16_t low_val = offset_samp - amp_samp;


    for (int i = 0; i < pulse_sample; i++) {
        samples[i] = low_val;
    }
    for (int i = pulse_sample; i < pulse_sample + pulse_size; i++) {
        samples[i] = hi_val;
    }
    for (int i = pulse_sample + pulse_size; i < sample_count; i++) {
        samples[i] = low_val;
    }

    return 0;
}

int32_t adi_vector_generate_from_file(int16_t samples[], uint32_t sample_count, void *config)
{
    adi_vector_file_config_t *cfg = (adi_vector_file_config_t*) config;
    int32_t err;
    uint32_t num_read;
    
    FILE * fp = 0;

    for (int i = 0; i < sample_count; i++) {
        samples[i] = 0;
    }

    fp = fopen(cfg->file, "r");
    err = file_16b_read(fp, samples, sample_count, &num_read, cfg->scalar);
    if (err != 0) {
        printf("Error reading I file: %s\n", cfg->file);
    }
    if (num_read != sample_count) {
        printf("Warning: not enough samples in files. Vector is appended with %d 0s\n", sample_count-num_read);
    }
    
    if (fp != NULL) {
        fclose(fp);
    }

    return err;
}

static int32_t file_16b_read(FILE *fp, int16_t *sample_arr, uint32_t max_samples, uint32_t *num_samples, double scalar)
{
    int32_t err = 0;
    uint32_t num_samples_cnt = 0;
    const int max_num_str_len = 16;
    char num_str[max_num_str_len];
    int16_t *p16;

    p16 = sample_arr;
    for (uint32_t i =0; i < max_samples; i++){
        if (fgets(num_str, max_num_str_len, fp) != NULL) {
            *p16++ = scalar * atoi(num_str);
            num_samples_cnt++;
        }
        else {
            break;
        }
    }

    *num_samples = num_samples_cnt;
    return err;
}

/*! @} */