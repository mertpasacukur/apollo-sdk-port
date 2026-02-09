/*!
 * \brief     FPGA Apollo capture functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_RX_HW_TPL_TO_SAMP_H__
#define __ADI_FPGA_APOLLO_RX_HW_TPL_TO_SAMP_H__

#include "adi_fpga_apollo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Convert capture data from fpga to samples/sort by virtual converter
 *
 * \param[in] fpga_state          Context variable - Pointer to the FPGA state data structure
 * \param[in] raw_cap_buf         Pointer to the capture data buffer
 * \param[in] num_cap_bytes       Number of bytes in the capture buffer
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_raw_cap_to_samples(adi_fpga_apollo_state_t *fpga_state, uint8_t raw_cap_buf[], uint32_t num_cap_bytes);

/**
 * \brief Set all sample data to zero
 * \note  This is used for unsupported capture modes
 *
 * \param[in] fpga_state          Context variable - Pointer to the FPGA state data structure
 * \param[in] raw_cap_buf         Pointer to the capture data buffer
 * \param[in] num_cap_bytes       Number of bytes in the capture buffer
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_raw_cap_to_samples_zeros(adi_fpga_apollo_state_t *fpga_state, uint8_t raw_cap_buf[], uint32_t num_cap_bytes);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_FPGA_APOLLO_RX_HW_TPL_TO_SAMP_H__