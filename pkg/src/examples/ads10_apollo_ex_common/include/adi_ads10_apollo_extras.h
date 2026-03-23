/*!
 * @brief     ADS10 Apollo Standalone App Extras Header File
 *            This file contains all the publicly exposed methods and data
 *            structures to interface with Standalone Application.
 *
 * @copyright copyright(c) 2020 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup adi_ads10_apollo_extras
 * @{
 */

#ifndef __ADI_ADS10_APOLLO_EXTRAS_H__
#define __ADI_ADS10_APOLLO_EXTRAS_H__

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

typedef struct {
    float real;
    float imag;
} adi_ads10_apollo_extras_fcomplex;

typedef struct {
    double mag;
    double fund_pwr_dbfs;
    double phase;
    double freq;
    uint32_t fund;
} adi_ads10_apollo_extras_dft_result;

/*!
*@brief Structure containing calculations (e.g. RMS) from an I/Q data array
*/
typedef struct {
    double      rms_i;
    double      rms_q;
    int16_t     min_sample_i;
    int16_t     max_sample_i;
    int16_t     min_sample_q;
    int16_t     max_sample_q;
} adi_ads10_apollo_extras_cap_anal_t;

/**
 * @brief  Generate single tone sine waves
 *
 * @param f_data_rate the rate samples are output from the FPGA
 * @param f_tone fundamental tone frequency requested
 * @param num_bits number of bits in amplitude
 * @param phase phase in degrees
 * @param backoff amplitude (in db) 0db is full scale
 * @param n_samples number of samples to generate
 * @param vec array data structure to hold generated sine wave
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed.
 */
int32_t adi_ads10_apollo_extras_create_single_tone(double f_data_rate, double f_tone, uint32_t num_bits, double phase, double backoff, uint32_t n_samples, int16_t vec[n_samples]);

/**
 * @brief  Calculates RMS value of interleaved I/Q sample array
 *
 * @param iq_data	    Array of I/Q interleaved samples
 * @param iq_data_len   Length of interleaved iq array
 * @param cap_anal      Pointer to result structure \ref adi_ads10_apollo_extras_cap_anal_t
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_calc_rms(int16_t *iq_data, int iq_data_len, adi_ads10_apollo_extras_cap_anal_t *cap_anal);


/**
 * @brief  Finds a peak value (pos or neg) from a slice of array data.
 *
 * @param iq_data	    Array of I/Q interleaved samples
 * @param iq_data_len   Length of interleaved iq array
 * @param find_pos_pk   If 1, find positive peak, else negative peak
 * @param first         First sample to start search
 * @param window_len    # of samples to look for peak, starting at 'first'
 * @param pk_sample_num Sample # in iq_data (between first + window_len) that contains peak value
 * @param pk_sample_val Peak sample value
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_find_peak(int16_t *iq_data, int iq_data_len, bool find_pos_pk, uint32_t first, uint32_t window_len, uint32_t *pk_sample_num,  int16_t *pk_sample_val);

/**
 * @brief  Perform complex DFT and return raw complex result array
 *
 * @param datain        Complex array of input samples
 * @param dataout       Complex array of DFT results
 * @param n             Size of input array
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_dft_calc(adi_ads10_apollo_extras_fcomplex datain[], adi_ads10_apollo_extras_fcomplex dataout[], uint32_t n);

/**
 * @brief  Return the magnitude and phase info from complex DFT results
 *
 * @param dftin             Complex array of DFT results
 * @param mag_sqr           Array of mag squared DFT bin results
 * @param phase_rads        Array of phase radians DFT bin results (-pi to +pi) 
 * @param n                 Size of dftin array
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_dft_results_get(adi_ads10_apollo_extras_fcomplex dftin[], double *mag_sqr, double *phase_rads, uint32_t n);

/**
 * @brief  Return the minimum value in a 1D double array 
 *
 * @param data          Input array of values
 * @param len           Length of input array
 * @param min_val       Pointer to min value result
 * @param min_loc       Pointer to min value location in array
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_arr1d_double_min_get(double data[], uint32_t len, double *min_val, uint32_t *min_loc);

/**
 * @brief  Return the maximum value in a 1D double array 
 *
 * @param data          Input array of values
 * @param len           Length of input array
 * @param max_val       Pointer to max value result
 * @param min_loc       Pointer to max value location in array
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_arr1d_double_max_get(double data[], uint32_t len, double *max_val, uint32_t *max_loc);

/**
 * @brief  Return the min, max and delta values in a 1D double array 
 *
 * @param data          Input array of values
 * @param len           Length of input array
 * @param min_val       Pointer to min value result
 * @param max_val       Pointer to max value result
 * @param delta         Pointer to max absolute delta value result
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_arr1d_double_delta_get(double data[], uint32_t len, double *min_val, double *max_val, double *delta);

/**
 * @brief  Return the maximum absolute value in a 1D double array 
 *
 * @param data          Input array of values
 * @param len           Length of input array
 * @param max_abs_val   Pointer to max absolute value result
 *
 * @return 0                                    Success
 * @return <0                                   Failed
 */
int32_t adi_ads10_apollo_extras_arr1d_double_max_abs_get(double data[], uint32_t len, double *max_abs_val);

#endif
