/*!
 * \brief     Basic FPGA Apollo functions for handling FSRC invalid samples.
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_FSRC_H__
#define __ADI_FPGA_APOLLO_FSRC_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_fsrc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief      Set FSRC values given n/m parameters (n >= m)
 *
 * \param[in]   fpga                 Context variable
 * \param[in]   terminal             Target terminal \ref adi_fpga_apollo_jesd_e
 * \param[in]   link                 Target link \ref adi_fpga_apollo_link_sel_e
 * \param[in]   enable               FSRC enable
 * \param[in]   fsrc_n               FSRC integer component
 * \param[in]   fsrc_m               FSRC fractional numerator
 * \param[in]   link_drc             Decimation for an individual link
 * \param[in]   total_drc            Total decimation for all links
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_fsrc_n_m_ratio_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint16_t link, uint8_t enable, uint32_t fsrc_n, uint32_t fsrc_m, uint32_t link_drc, uint32_t total_drc);

/**
 * \brief      Set FSRC values
 *
 * \param[in]   fpga                 Context variable
 * \param[in]   terminal             Target terminal \ref adi_fpga_apollo_jesd_e
 * \param[in]   link                 Target link \ref adi_fpga_apollo_link_sel_e
 * \param[in]   enable               FSRC enable
 * \param[in]   rate_int             FSRC integer component
 * \param[in]   frac_a               FSRC fractional numerator
 * \param[in]   frac_b               FSRC fractional denominator
 * \param[in]   link_drc             Decimation for an individual link
 * \param[in]   total_drc            Total decimation for all links
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_fsrc_invalid_ratio_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint16_t link, uint8_t enable, uint64_t rate_int, uint64_t frac_a, uint64_t frac_b, uint32_t link_drc, uint32_t total_drc);

/**
 * \brief      Set sample repeat values
 *
 * \param[in]   fpga                 Context variable
 * \param[in]   terminal             Target terminal \ref adi_fpga_apollo_jesd_e
 * \param[in]   link                 Target link \ref adi_fpga_apollo_link_sel_e
 * \param[in]   enable               FSRC enable
 * \param[in]   link_drc             Decimation for an individual link
 * \param[in]   total_drc            Total decimation for all links
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_fsrc_sr_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint16_t link, uint8_t enable, uint32_t link_drc, uint32_t total_drc);


int32_t adi_fpga_apollo_fsrc_tx_vector_length_get(uint32_t length, uint32_t fsrc_n, uint32_t fsrc_m,
                                                  adi_apollo_fsrc_tx_vector_length_t *vec_length);

int32_t  adi_fpga_apollo_fsrc_sample_insert(uint16_t i_vec[], uint16_t q_vec[], uint32_t length, uint8_t jrx_ns,
                                       uint16_t fsrc_i_vec[], uint16_t fsrc_q_vec[],
                                       adi_apollo_fsrc_tx_vector_length_t *vec_length);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_FPGA_APOLLO_FSRC_H__