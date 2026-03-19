/*
 * \brief     FPGA Apollo transmit functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_TRANSMIT_H__
#define __ADI_FPGA_APOLLO_TRANSMIT_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#endif

#include "adi_fpga_apollo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief       Write vector to FPGA memory for transmit
 *
 * \param[in]   fpga                 Context variable
 * \param[in]   side_select          side A or B              
 * \param[in]   input_vecs           1D vector to transmit
 * \param[in]   input_vecs_length    length of vector
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_transmit_write(adi_fpga_apollo_device_t *fpga, uint32_t side_select, int16_t input_vecs[], uint32_t input_vecs_length);

#ifndef CLIENT_IGNORE

/**
 * \brief       Write vector to FPGA memory for selected links in a vector group 
 *
 * \param[in]   fpga                Context variable
 * \param[in]   vec_grp             Vector group containing per link vector playback info. \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   links               Link select mask. Only selected links are modified. \ref adi_fpga_apollo_link_sel_e 
 * \param[in]   input_vecs          1D vector to transmit (samples)
 * \param[in]   input_vecs_length   length of vector (num samples)
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_transmit_write2(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, int16_t input_vecs[], uint32_t input_vecs_length);

#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif

#endif // __ADI_FPGA_APOLLO_TRANSMIT_H__