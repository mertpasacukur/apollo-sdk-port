/*!
 * \brief     ADI vector formatting and generation
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_vector_generation.h"


#ifndef __ADI_VECTOR_H__
#define __ADI_VECTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
* \brief  Generate a generic signal in memory buffer
*
* \param[in] samples        Buffer to hold samples
* \param[in] sample_count   Number of samples in buffer
* \param[in] vec_len        vector size per virtual converter
* \param[in] vec_gen        Vector generator function \ref adi_vector_generator
* \param[in] config         Generic config for generator
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_vector_buffer_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen, void *config);

/**
* \brief  Generate generic vectors to memory buffer
*
* \param[in] samples        Buffer to hold samples
* \param[in] sample_count   Number of samples in buffer
* \param[in] vec_len        vector size per virtual converter
* \param[in] vec_gen        Vector generator function array \ref adi_vector_generator
* \param[in] config         Generic config for generators array
* \param[in] vec_count      Number of vectors. Virtual converter m_i is populated with vec[i%vec_count]
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_vector_buffer_multiconfig_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count);

/**
* \brief  Generate generic vectors to memory buffer
*
* \param[in] samples        Buffer to hold samples
* \param[in] sample_count   Number of samples in buffer
* \param[in] vec_len        vector size per virtual converter
* \param[in] vec_gen        Vector generator function array \ref adi_vector_generator
* \param[in] config         Generic config for generators array
* \param[in] vec_count      Number of vectors. Virtual converter m_i is populated with vec[i%vec_count]
*
* \return API_CMS_ERROR_OK                     API Completed Successfully
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_vector_buffer_multiconfig_debug_generate(int16_t samples[], uint32_t sample_count, uint32_t vec_len, adi_vector_generator vec_gen[], void *config[], uint8_t vec_count);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_VECTOR_H__ */