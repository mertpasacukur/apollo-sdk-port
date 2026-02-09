/*!
 * \brief     FPGA Apollo CAPTURE types.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

 /*!
  * \addtogroup  ADI_FPGA_APOLLO_CAPTURE
  * \ingroup     ADI_FPGA_APOLLO
  * @{
  */
#ifndef __ADI_FPGA_APOLLO_CAPTURE_TYPES_H__
#define __ADI_FPGA_APOLLO_CAPTURE_TYPES_H__

#include "adi_cms_api_common.h"
#include "adi_fpga_apollo_common_types.h"
#include "adi_fpga_apollo_core_types.h"

/*!
* \brief FPGA capture frame parameters
*/
typedef struct {
    uint8_t is_valid;                                       /*!< 1 if struct has been populated */
    uint8_t link_enabled[MAX_JESD_LINKS];                   /*!< 1 if enabled */
    uint32_t frame_samples_per_conv[MAX_JESD_LINKS];        /*!< number of samples per vc in a raw frame */
    uint32_t fpga_link_bits[MAX_JESD_LINKS];                /*!< 2048 dual link, 4096 if single link, 0 if link not used */
    uint8_t link_converter_count[MAX_JESD_LINKS];           /*!< number of converters (m) per link */
    uint8_t link_bits_per_sample[MAX_JESD_LINKS];           /*!< number of bits per sample per link (np) */
    uint32_t link_min_samples_per_converter;                /*!< minimum number of samples per conv in a frame */
    uint32_t frame_mult;                                    /*!< number of frames needed for integer samples/conv */
    uint32_t frame_size_bytes;                              /*!< number of bytes in capture in a frame (incorporates frame_mult) */
    uint32_t sw_tpl_mult;                                   /*!< number 64K blocks needed for sw_tpl */
    uint32_t tot_samps_per_conv[MAX_JESD_LINKS];            /*!< Total number of expected samples per converter for entire capture */
    uint32_t transf_samps_per_conv[MAX_JESD_LINKS];         /*!< Total number of samples per converter for a single transfer */
} adi_fpga_apollo_capture_frame_t;

/*!
* \brief FPGA capture information
*/
typedef struct {
    adi_fpga_apollo_capture_frame_t capture_frame;	/*!< Capture raw framing info for reading back capture memory */
    uint64_t fifo_queue_bytes;                      /*!< Number of bytes in the FIFO queue */	
#ifndef CLIENT_IGNORE
    uint8_t **cap_transf_buff[MAX_JESD_LINKS];      /*!< Capture transfer buffer */
    uint32_t *cap_transf_cnt[MAX_JESD_LINKS];       /*!< Capture transfer count per link */
#endif /* CLIENT_IGNORE */
} adi_fpga_apollo_capture_t;            
#endif // !__ADI_FPGA_APOLLO_CAPTURE_TYPES_H__
/*! @} */
