/*!
 * @brief     ADI Vector Generators
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_VECTOR_GENERATION_H__
#define __ADI_VECTOR_GENERATION_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"

/* Defines a function pointer that generates a vector (e.g. sine) */
typedef int32_t(*adi_vector_generator)(int16_t samples[], uint32_t sample_count, void *config);

/*!
* \brief Tone vector parameters
*/
typedef struct {
    uint8_t resolution; /*< number of bits in a sample */
    double ratio;       /*< ratio of tone to data rate*/
    double phase;       /*< phase in degrees */
    double backoff;     /*< amplitude (in db) 0db is full scale */
} adi_vector_sine_config_t;

/*!
* \brief Square wave vector parameters
*/
typedef struct
{
    uint8_t resolution; /*< number of bits in a sample */
    uint32_t div;       /*< sample clock divider. Freq equals Fclk / div */
    uint8_t is_leading; /*< 1 indicates first samples high, else low */
    double duty_cycle;  /*< Duty cycle */
    double backoff;     /*< amplitude (in db) 0db is full scale */
} adi_vector_square_config_t;

/*!
* \brief Ramp vector parameters
*/
typedef struct {
    uint32_t step;      /*< Step between samples */
    int16_t  start;     /*< Start value */
    int16_t  stop;      /*< Stop value */
} adi_vector_ramp_config_t;

/*!
* \brief File load vector parameters
*/
typedef struct {
    char file[256]; /*< Path to file */
    double scalar;  /*< Scalar applied to signal */
} adi_vector_file_config_t;

/*!
* \brief Pulse vector parameters
*/
typedef struct {
    uint8_t resolution;
    double pulse_start;   /*< Pulse location from 0 to 1. pulse_start + duty_cycle must be less than or equal to 1 */
    double duty_cycle;    /*< Pulse duty cycle from 0 to 1 */
    double amplitude;     /*< Amplitude ratio from 0 to 1 */
    double offset;        /*< DC offset for pulse. amplitude + offset must be less than or equal to 1 */
} adi_vector_pulse_config_t;

/**
 * @brief  Populate buffer with constant value
 *
 * @param samples array containing generated samples
 * @param sample_count max number of samples to load
 * @param config constant value pointer to int16_t
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_vector_generate_constant(int16_t samples[], uint32_t sample_count, void *config);

/**
 * @brief Generate sine wave
 *
 * @param samples array containing generated samples
 * @param sample_count max number of samples to load
 * @param config tone config \ref adi_ads10_apollo_tone_config_t
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_vector_generate_sine(int16_t samples[], uint32_t sample_count, void *config);

/**
 * @brief  Generate square wave
 *
 * @param samples array containing generated samples
 * @param sample_count max number of samples to load
 * @param config square wave config \ref adi_vector_square_config_t
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_vector_generate_square(int16_t samples[], uint32_t sample_count, void *config);

/**
 * @brief  Generate ramp
 *
 * @param samples array containing generated samples
 * @param sample_count max number of samples to load
 * @param config tone config \ref adi_vector_file_config_t
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_vector_generate_ramp(int16_t samples[], uint32_t sample_count, void *config);

/**
 * @brief  Generate pulse train
 *
 * @param samples array containing generated samples
 * @param sample_count max number of samples to load
 * @param config tone config \ref adi_vector_file_config_t
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_vector_generate_pulse(int16_t samples[], uint32_t sample_count, void *config);

/**
 * @brief  Load vector from file
 *
 * @param samples array containing generated samples
 * @param sample_count max number of samples to load
 * @param config tone config \ref adi_vector_file_config_t
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_vector_generate_from_file(int16_t samples[], uint32_t sample_count, void *config);

#endif