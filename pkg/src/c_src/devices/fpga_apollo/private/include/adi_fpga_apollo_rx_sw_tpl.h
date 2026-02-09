/*!
 * @brief     FPGA JRX SW Transport Layer Header File.
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_RX_SW_TPL_H__
#define __ADI_FPGA_APOLLO_RX_SW_TPL_H__

/*============= I N C L U D E S ============*/
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_fsrc_types.h"
#include "adi_fpga_apollo_common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Perform JESD204 Transport Layer processing on the captured ADC data.
 *
 * \param[in]   state_info     JESD Parameters for all the links. \ref adi_fpga_apollo_state_t
 * \param[in]   raw_frame      Byte array containing raw captured data.
 * \param[in]   frame_length   Length of the captured data buffer.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t rx_jesd204_sw_tpl(adi_fpga_apollo_state_t *state_info, uint8_t raw_frame[], uint32_t frame_length);


#ifdef __cplusplus
}
#endif

#endif // __ADI_FPGA_APOLLO_RX_SW_TPL_H__