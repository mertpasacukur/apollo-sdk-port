/*!
 * \brief     FPGA Apollo capture functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_CAPTURE_H__
#define __ADI_FPGA_APOLLO_CAPTURE_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#endif

#include "adi_fpga_apollo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FPGA_CAPTURE_SIZE_BLOCK_64KB 65536

/**
 * \brief        Calculate and populate FPGA capture frame information
 *
 * \param[in]   fpga               Context variable
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_capture_frame_populate(adi_fpga_apollo_device_t *fpga);

/**
 * \brief       Malloc memory for capture transfer buffer.
 *
 * \param[in]   fpga              Context variable
 * \param[in, out]   num_samples  Ptr to the minimum number of samples per converter to capture
 * \param[out]  max_transf        Maximum number of samples that can be transferred in a single call to adi_fpga_apollo_capture_data_get
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_capture_transfer_setup(adi_fpga_apollo_device_t *fpga, uint32_t *num_samples, uint32_t *max_transf);

/**
 * \brief       Setup and execute FPGA capture. Request by minimum number of samples per converter.
 *
 * \param[in]   fpga                  Context variable
 * \param[in]   num_samples           Number of samples to capture (min across all links)
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_capture_execute(adi_fpga_apollo_device_t *fpga, uint32_t num_samples);

/**
 * \brief       Get capture frame information
 * 
 * \param[in]   fpga            Context variable
 * \param[out]  cap_frame_info  Pointer to struct that describes how a capture frame is organized
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_capture_frame_info_get(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_capture_frame_t *cap_frame_info);

/**
 * \brief       Get number of bytes in capture FIFO
 * 
 * \param[in]   fpga            Context variable
 * \param[out]  fifo_bytes      Number of bytes in capture FIFO
 */
int32_t adi_fpga_apollo_capture_fifo_bytes_get(adi_fpga_apollo_device_t *fpga, uint64_t *fifo_bytes);

/**
 * \brief       Get capture memory from FPGA, store in transfer buffer
 *
 * \param[in]   fpga            Context variable
 * \param[in]   num_cap_bytes   Number of capture bytes to read
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_capture_data_get(adi_fpga_apollo_device_t *fpga, uint32_t num_cap_bytes);

/**
 * \brief       Get number of samples captured for specified converter
 * 
 * \param[in]   fpga            Context variable
 * \param[in]   apollo_jtx_link Target link \ref adi_apollo_jesd_link_select_e
 * \param[in]   vc_idx          Virtual converter index
 * \param[out]  num_samples     Number of samples captured
 */
int32_t adi_fpga_apollo_capture_transf_buff_num_samples_get(adi_fpga_apollo_device_t *fpga, uint16_t apollo_jtx_link, uint16_t vc_idx, uint32_t *num_samples);

/**
 * \brief       Get number of bytes captured for specified converter
 * 
 * \param[in]   fpga            Context variable
 * \param[in]   apollo_jtx_link Target link \ref adi_apollo_jesd_link_select_e
 * \param[in]   vc_idx          Virtual converter index
 * \param[out]  num_bytes       Number of bytes captured
 */
int32_t adi_fpga_apollo_capture_transf_buff_num_bytes_get(adi_fpga_apollo_device_t *fpga, uint16_t apollo_jtx_link, uint16_t vc_idx, uint32_t *num_bytes);

/**
 * \brief       Get capture transfer buffer for specified converter
 *
 * \param[in]   fpga            Context variable
 * \param[in]   apollo_jtx_link Target link \ref adi_apollo_jesd_link_select_e
 * \param[in]   vc_idx          Virtual converter index
 * \param[out]  cap_buf         Array to store capture data
 * \param[in]   cap_num_bytes   Number of bytes to get 
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_capture_transf_buff_get(adi_fpga_apollo_device_t *fpga, uint16_t apollo_jtx_link, uint16_t vc_idx, int16_t cap_buf[], uint32_t cap_num_bytes);

/**
 * \brief       Free memory allocated for capture transfer
 *
 * \param[in]   fpga                 Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_capture_transfer_cleanup(adi_fpga_apollo_device_t *fpga);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_FPGA_APOLLO_CAPTURE_H__
