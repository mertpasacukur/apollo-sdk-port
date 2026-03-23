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
#include <unistd.h>

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_apollo_arm_types.h"
#include "adi_fpga_apollo_types.h"
#include "adi_vector.h"


#ifndef __ADI_ADS10_APOLLO_COMMON_EX_VEC_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_VEC_H__

#define EX_VEC_DEFAULT_SAMPLES_PER_VC   8192

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Generate and write a single user defined vector to FPGA memory
 *
 * \param[in] fpga_device   Context variable - Pointer to the FPGA device data structure
 * \param[in] profile       Device profile
 * \param[in] vec_grp       Vector group. If NULL, then the vector is processed and made ready to run.
 * \param[in] links         Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] vec_len       Number of samples in vector per virtual converter
 * \param[in] vec_loader    Vector loader functions for creating vectors \ref adi_vector_loader_t
 * \param[in] config        Generic config parameters for generator
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_vec_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                      adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                      uint32_t vec_len, adi_vector_loader_t vec_loader, void *config);

/**
 * \brief  Generate and write multiple user defined vectors to FPGA memory
 *
 * \param[in] fpga_device   Context variable - Pointer to the FPGA device data structure
 * \param[in] profile       Device profile
 * \param[in] vec_grp       Vector group. If NULL, then the vector is processed and made ready to run
 * \param[in] links         Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] vec_len       Number of samples in vector per virtual converter
 * \param[in] vec_loaders   Vector loader functions for creating vectors. \ref adi_vector_loader_t
 * \param[in] configs       Generic config parameters for generators
 * \param[in] vec_count     Number of vec_loaders and configs. Vectors are assigned to VCs as: [vc % vec_count]
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_vec_multiconfig_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                                  adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                                  uint32_t vec_len, adi_vector_loader_t vec_loaders[], void *configs[], uint8_t vec_count);

/**
 * \brief  Generate and load a complex tone to FPGA memory
 *
 * \param[in] fpga_device   Context variable - Pointer to the FPGA device data structure
 * \param[in] profile       Device profile
 * \param[in] vec_grp       Vector group. If NULL, then the vector is processed and made ready to run
 * \param[in] links         Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] vec_len       Number of samples in vector per virtual converter
 * \param[in] tone_ratio    Percentage of data rate (e.g. Ftone/Fdata)
 * \param[in] backoff_db    Backoff from full scale in dB
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_vec_cmplx_tone_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, 
                                                 adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, 
                                                 uint32_t vec_len, double tone_ratio, double backoff_db);

/**
 * \brief  Generate and load complex tone(s) to FPGA memory. Optionally increases/decreases freq value per I/Q pair.
 *
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] profile           Device profile
 * \param[in] vec_grp           Vector group. If NULL, then the vector is processed and made ready to run
 * \param[in] links             Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] vec_len           Number of samples in vector per virtual converter
 * \param[in] tone_ratio        Percentage of data rate (e.g. Ftone/Fdata)
 * \param[in] tone_ratio_incr   Tone ratio freq increase (or decrease) per I/Q pair
 * \param[in] backoff_db        Backoff from full scale in dB
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                                      adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, 
                                                      uint32_t vec_len, double tone_ratio, double tone_ratio_incr, double backoff_db);
/**
 * \brief  Generate and load square wave in FPGA memory
 *
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] profile           Device profile
 * \param[in] vec_grp           Vector group. If NULL, then the vector is processed and made ready to run
 * \param[in] links             Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] vec_len           Number of samples in vector per virtual converter
 * \param[in] div               Fdata divider (e.g. id00_uc06 sysref: 1250 / 256 = 4.8828125)
 * \param[in] is_leading        1 = start with leading edge, 0 = trailing
 * \param[in] duty_cycle        Duty cycle
 * \param[in] backoff_db        Backoff from full scale in dB
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_vec_square_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                             adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                             uint32_t vec_len, uint32_t div, uint8_t is_leading, double duty_cycle, double backoff_db);

/**
 * \brief  Load vectors from files to FPGA memory
 *
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] profile           Device profile
 * \param[in] vec_grp           Vector group. If NULL, then the vector is processed and made ready to run
 * \param[in] links             Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] max_samples       Maximum number of samples to load from file
 * \param[in] i_file            Path to I data file
 * \param[in] q_file            Path to Q data file (can be NULL)
 * \param[in] scalar            Scalar to apply to all samples
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_vec_files_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                            adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                            uint32_t max_samples, char *i_file, char *q_file, double scalar);

/**
 * \brief  Generate and load a ramp to FPGA memory
 *
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] profile           Device profile
 * \param[in] vec_grp           Vector group. If NULL, then the vector is processed and made ready to run
 * \param[in] links             Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
 * \param[in] vec_len           Number of samples in vector per virtual converter
 * \param[in] start             Start sample value
 * \param[in] stop              Stop sample value
 * \param[in] step              Step size for consecutive samples
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
 int32_t adi_ads10_apollo_ex_vec_ramp_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, 
                                           adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                           uint32_t vec_len, int16_t start, int16_t stop, int16_t step);

 /**
  * \brief  Generate and load constant values incrementing with each virtual converter
  *
  * \param[in] fpga_device      Context variable - Pointer to the FPGA device data structure
  * \param[in] profile          Device profile
  * \param[in] vec_grp          Vector group. If NULL, then the vector is processed and made ready to run
  * \param[in] links            Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
  * \param[in] vec_len          Number of samples in vector per virtual converter
  * \param[in] offset           Start value
  * \param[in] incr             Increment offset added to each VC in link (val = offset + (VC# * incr))
  *
  * \return API_CMS_ERROR_OK                     API Completed Successfully
  * \return <0                                   Failed. \ref adi_cms_error_e for details.
  */
 int32_t adi_ads10_apollo_ex_vec_constants_incr_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                                      adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                                      uint32_t vec_len, int16_t offset, int16_t incr);

 /**
  * \brief  Generate and load a pulse to FPGA memory
  *
  * \param[in] fpga_device      Context variable - Pointer to the FPGA device data structure
  * \param[in] profile          Device profile
  * \param[in] vec_grp          Vector group. If NULL, then the vector is processed and made ready to run
  * \param[in] links            Links within vec_grp to program. \ref adi_apollo_jesd_link_select_e
  * \param[in] vec_len          Number of samples in vector per virtual converter
  * \param[in] duty_cycle       Pulse duty cycle from 0 to 1
  * \param[in] amplitude        Amplitude ratio from 0 to 1
  * \param[in] offset           DC offset for pulse. amplitude + offset must be less than or equal to 1
  *
  * \return API_CMS_ERROR_OK                     API Completed Successfully
  * \return <0                                   Failed. \ref adi_cms_error_e for details.
  */
 int32_t adi_ads10_apollo_ex_vec_pulse_write(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                                             adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links,
                                             uint32_t vec_len, double duty_cycle, double amplitude, double offset);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_VEC_H__ */
