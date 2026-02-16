#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples common vector functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__linux__)
#include <unistd.h>
#endif

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_apollo_arm_types.h"
#include "adi_fpga_apollo_types.h"
#include "adi_vector.h"


#ifndef __ADI_ADS10_APOLLO_COMMON_EX_VEC_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_VEC_H__

#define EX_VEC_DEFAULT_SAMPLES_PER_VC	8192

#ifdef __cplusplus
extern "C" {
#endif

/**
* \brief  Generate and load a generic signal to FPGA memory
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile
* \param[in] sides   		Side select
* \param[in] vec_len        vector size per virtual converter
* \param[in] vec_gen        Vector generator function \ref adi_vector_generator
* \param[in] config         Generic config for generator
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, adi_vector_generator vec_gen, void *config);

/**
* \brief  Generate and load generic signals to FPGA memory
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile
* \param[in] sides   		Side select
* \param[in] vec_len        vector size per virtual converter
* \param[in] vec_gen        Vector generator function array \ref adi_vector_generator
* \param[in] config         Generic config for generators array
* \param[in] vec_count      Number of vectors. Virtual converter m_i is populated with vec[i%vec_count]
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_multiconfig_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count);

/**
* \brief  Generate and load a complex tone to FPGA memory
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile
* \param[in] sides   		Side select
* \param[in] vec_len        vector size per virtual converter
* \param[in] tone_ratio		Percentage of 'f_data_rate' of tone
* \param[in] backoff_db     Backoff from full scale in dB
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_cmplx_tone_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, double tone_ratio, double backoff_db);

/**
* \brief  Generate and load complex tone(s) to FPGA memory. Optionally increases/decreases freq value per I/Q pair. 
*
* \param[in] fpga_device        Context variable - Pointer to the FPGA device data structure
* \param[in] profile		    Device profile
* \param[in] sides   		    Side select
* \param[in] vec_len            Vector size per virtual converter
* \param[in] tone_ratio		    Percentage of 'f_data_rate' of tone
* \param[in] tone_ratio_incr    Tone ratio freq increase (or decrease) per I/Q pair
* \param[in] backoff_db         Backoff from full scale in dB
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, double tone_ratio, double tone_ratio_incr, double backoff_db);

/**
* \brief  Generate square wave in FPGA memory
*
* Note: Side A and B divide values are evaluated together so vectors with different
*       freqs can be properly sized for coherency (i.e. vecs must be same length)
* 
* \param[in] fpga_device        Context variable - Pointer to the FPGA device data structure
* \param[in] profile		    Device profile
* \param[in] div_a              Sample clock (e.g. Fdata) divider for side A links. If 0, then no vec not loaded.
* \param[in] div_b              Sample clock (e.g. Fdata) divider for side B links. If 0, then no vec not loaded.
* \param[in] is_leading		    1 = start with leading edge, 0 = trailing
* \param[in] duty_cycle         Duty cyle
* \param[in] backoff_db         Backoff from full scale in dB
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_square_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                             uint32_t div_a, uint32_t div_b, uint8_t is_leading, double duty_cycle, double backoff_db);

/**
* \brief  Load vector from files
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile	   
* \param[in] sides			Side select
* \param[in] max_samples    Maximum number of samples to load from file
* \param[in] i_file      	Path to I data file
* \param[in] q_file      	Path to Q data file (can be NULL)
* \param[in] scalar         Scalar to apply to all samples
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_files_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t max_samples, char *i_file, char *q_file, double scalar);

/**
* \brief  Generate and load a ramp to FPGA memory
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile
* \param[in] sides			Side select
* \param[in] vec_len        vector size per virtual converter
* \param[in] step		    Step for consecutive samples
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_ramp_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, int8_t step);

/**
* \brief  Generate and load constant values incrementing with each virtual converter
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile
* \param[in] sides			Side select
* \param[in] vec_len        vector size per virtual converter
* \param[in] offset		    value at which to start for m0
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_constants_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, int16_t offset);


/**
* \brief  Generate and load a pulse to FPGA memory
*
* \param[in] fpga_device    Context variable - Pointer to the FPGA device data structure
* \param[in] profile		Device profile
* \param[in] sides			Side select
* \param[in] vec_len        vector size per virtual converter
* \param[in] duty_cycle     Pulse duty cycle from 0 to 1
* \param[in] amplitude      Amplitude ratio from 0 to 1
* \param[in] offset         DC offset for pulse. amplitude + offset must be less than or equal to 1
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_vec_pulse_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t sides, uint32_t vec_len, double duty_cycle, double amplitude, double offset);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_VEC_H__ */

#endif /* !defined(VERSAL_PLATFORM) */
