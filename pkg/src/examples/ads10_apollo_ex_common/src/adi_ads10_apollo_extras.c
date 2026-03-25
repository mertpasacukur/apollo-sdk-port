#if !defined(VERSAL_PLATFORM)
/*!
 * @brief     ADS10 Apollo Standalone App Extras Source File
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

/*============= I N C L U D E S ============*/
#include "adi_ads10_apollo_extras.h"
#include "adi_utils.h"

/*============= C O D E ====================*/

int32_t adi_ads10_apollo_extras_create_single_tone(double f_data_rate, double f_tone, uint32_t num_bits, double phase, double backoff, uint32_t n_samples, int16_t vec[n_samples])
{
    double phase_rads = phase * M_PI / 180.0;
    uint32_t m_cycles = fabs(f_tone * n_samples) / f_data_rate;
    if (m_cycles % 2 == 0) {
        m_cycles++;
    }
    f_tone = (m_cycles * f_data_rate) / n_samples;

    double ratio = ((2 * M_PI * f_tone) / f_data_rate);
    double amplitude = (1 << ((uint16_t)num_bits - 1)) - 1;
    double backoff_db = pow(10, (backoff / 20.0));
    for (int i = 0; i < n_samples; i++) {
        vec[i] = (int) amplitude * backoff_db * sin((ratio * i) + phase_rads);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_calc_rms(int16_t *iq_data, int iq_data_len, adi_ads10_apollo_extras_cap_anal_t *cap_anal)
{
    double n = 0;
    double sum_i = 0.0;
    double sum_q = 0.0;
    double sum_squares_i = 0.0;
    double sum_squares_q = 0.0;
    cap_anal->min_sample_i = 32767;
    cap_anal->max_sample_i = -32768;
    cap_anal->min_sample_q = 32767;
    cap_anal->max_sample_q = -32768;
    cap_anal->rms_i = 0.0;
    cap_anal->rms_q = 0.0;

    for (int i = 0; i < iq_data_len; i += 2) {
        n += 1.0;
        sum_i += iq_data[i];
        sum_squares_i += iq_data[i] * iq_data[i];

        sum_q += iq_data[i + 1];
        sum_squares_q += iq_data[i + 1] * iq_data[i + 1];
        cap_anal->rms_i = (n == 1) ? 0.0 : sqrt((sum_squares_i - (sum_i * sum_i / n)) / (n - 1.0));
        cap_anal->rms_q = (n == 1) ? 0.0 : sqrt((sum_squares_q - (sum_q * sum_q / n)) / (n - 1.0));

        cap_anal->min_sample_i = (iq_data[i] < cap_anal->min_sample_i) ? iq_data[i] : cap_anal->min_sample_i;
        cap_anal->max_sample_i = (iq_data[i] > cap_anal->max_sample_i) ? iq_data[i] : cap_anal->max_sample_i;
        cap_anal->min_sample_q = (iq_data[i + 1] < cap_anal->min_sample_q) ? iq_data[i + 1] : cap_anal->min_sample_q;
        cap_anal->max_sample_q = (iq_data[i + 1] > cap_anal->max_sample_q) ? iq_data[i + 1] : cap_anal->max_sample_q;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_find_peak(int16_t* iq_data, int iq_data_len, bool find_pos_pk, uint32_t first, uint32_t window_len, uint32_t* pk_sample_num, int16_t* pk_sample_val)
{
    int i;
    adi_ads10_apollo_extras_cap_anal_t cap_anal;

    *pk_sample_val = 0;
    *pk_sample_num = 0;

    if ((iq_data_len < 10) || (first + window_len) > iq_data_len) {
        return -1;
    }

    adi_ads10_apollo_extras_calc_rms(iq_data + first, window_len, &cap_anal);

    //printf("min_i = %6d    max_i = %6d\n", cap_anal.min_sample_i, cap_anal.max_sample_i);
    //printf("min_q = %6d    max_q = %6d\n", cap_anal.min_sample_q, cap_anal.max_sample_q);

    // Assume interleaved data
    if (find_pos_pk) {
        for (i = first; i < first + window_len; i += 2) {

            // I data only
            if (iq_data[i] == cap_anal.max_sample_i) {
                *pk_sample_num = i;
                *pk_sample_val = iq_data[i];
                return i;
            }
        }
    }
    else {
        for (i = first; i < first + window_len; i += 2) {
            // I data only
            if (iq_data[i] == cap_anal.min_sample_i) {
                *pk_sample_num = i;
                *pk_sample_val = iq_data[i];
                return i;
            }
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_dft_calc(adi_ads10_apollo_extras_fcomplex data_in[], adi_ads10_apollo_extras_fcomplex data_out[], uint32_t n)
{
    uint32_t i, j, k, p;
    double arg;
    adi_ads10_apollo_extras_fcomplex *cfptr, *dinptr;
    static int n_coeffs = 0;
    static adi_ads10_apollo_extras_fcomplex *cf;

    ADI_CMS_NULL_PTR_CHECK(data_in);
    ADI_CMS_NULL_PTR_CHECK(data_out);

    /* Store static trig values for n-point calculation. */
    if (n != n_coeffs) {
        if (n_coeffs != 0) {
            free(cf);
        }

        cf = (adi_ads10_apollo_extras_fcomplex *) calloc(n, sizeof(adi_ads10_apollo_extras_fcomplex));
        if (!cf) {
            printf("Can't alloc DFT coeffs\n");
            return (API_CMS_ERROR_MEM_ALLOC);
        }

        n_coeffs = n;
        
        arg = 2 * M_PI / n;
        for (i = 0; i < n; i++) {
            cf[i].real = (float)cos(arg * i);
            cf[i].imag = -(float)sin(arg * i);
        }
    }

    for (k = 0; k < n; k++) {
        dinptr = data_in;
        data_out->real = dinptr->real;
        data_out->imag = dinptr->imag;
        dinptr++;

        for (j = 1; j < n; j++) {
            p = ((j * k) % n);
            cfptr = cf + p;

            data_out->real += dinptr->real * cfptr->real - dinptr->imag * cfptr->imag;
            data_out->imag += dinptr->real * cfptr->imag + dinptr->imag * cfptr->real;
            dinptr++;
        }

        data_out++;
    }
    
    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_dft_results_get(adi_ads10_apollo_extras_fcomplex data_in[], double mag_sqr[], double phase_rads[], uint32_t n)
{
    adi_ads10_apollo_extras_fcomplex *dptr;
    double *mptr;
    double *pptr;
    double mag;
    double nsq;

    ADI_CMS_NULL_PTR_CHECK(data_in);
    ADI_CMS_NULL_PTR_CHECK(mag_sqr);
    ADI_CMS_NULL_PTR_CHECK(phase_rads);

    dptr = data_in;
    mptr = mag_sqr;
    pptr = phase_rads;
    nsq = n * n;
    for (int i = 0; i < n; i++) {
        *pptr++ = atan2(dptr->imag, dptr->real);

        mag = dptr->real * dptr->real;
        mag += dptr->imag * dptr->imag;
        mag = mag / nsq;
        *mptr++ = mag;
        
        dptr++;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_arr1d_double_max_get(double data[], uint32_t len, double *max_val, uint32_t *max_loc)
{
    int i;

    ADI_CMS_NULL_PTR_CHECK(data);
    ADI_CMS_NULL_PTR_CHECK(max_val);
    ADI_CMS_NULL_PTR_CHECK(max_loc);
    ADI_CMS_INVALID_PARAM_CHECK(len < 1);

    *max_loc = 0;
    *max_val = data[0];
    for (i = 1; i < len; i++) {
        if (data[i] > *max_val) {
            *max_val = data[i];
            *max_loc = i;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_arr1d_double_min_get(double data[], uint32_t len, double *min_val, uint32_t *min_loc)
{
    int i;

    ADI_CMS_NULL_PTR_CHECK(data);
    ADI_CMS_NULL_PTR_CHECK(min_val);
    ADI_CMS_NULL_PTR_CHECK(min_loc);
    ADI_CMS_INVALID_PARAM_CHECK(len < 1);

    *min_loc = 0;
    *min_val = data[0];
    for (i = 1; i < len; i++) {
        if (data[i] < *min_val) {
            *min_val = data[i];
            *min_loc = i;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_arr1d_double_max_abs_get(double data[], uint32_t len, double *max_abs_val)
{
    int i;

    ADI_CMS_NULL_PTR_CHECK(data);
    ADI_CMS_NULL_PTR_CHECK(max_abs_val);
    ADI_CMS_INVALID_PARAM_CHECK(len < 1);
    double abs_val;

    *max_abs_val = fabs(data[0]);
    for (i = 1; i < len; i++) {
        abs_val = fabs(data[i]);
        if (abs_val > *max_abs_val) {
            *max_abs_val = abs_val;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_extras_arr1d_double_delta_get(double data[], uint32_t len, double *min_val, double *max_val, double *delta)
{
    int i;

    ADI_CMS_NULL_PTR_CHECK(data);
    ADI_CMS_NULL_PTR_CHECK(min_val);
    ADI_CMS_NULL_PTR_CHECK(max_val);
    ADI_CMS_NULL_PTR_CHECK(delta);
    ADI_CMS_INVALID_PARAM_CHECK(len < 1);

    *min_val = data[0];
    *max_val = data[0];
    for (i = 1; i < len; i++) {
        if (data[i] < *min_val) {
            *min_val = data[i];
        }
        if (data[i] > *max_val) {
            *max_val = data[i];
        }
    }

    *delta = fabs(*max_val - *min_val);
    
    return API_CMS_ERROR_OK;
}
/*! @} */

#endif /* !defined(VERSAL_PLATFORM) */
