/*!
 * \brief     Basic FPGA Apollo FSRC types
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_FSRC_TYPES_H__
#define __ADI_FPGA_APOLLO_FSRC_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include "adi_cms_api_common.h"
#include "adi_apollo_common.h"

/*!
 * \brief sample information for tx fsrc
 */
typedef struct {
    uint32_t num_total_samples;        /* num of total samples after adding invalid samples */
    uint32_t num_valid_samples;        /* num of valid samples in vector after adding invalid samples */
    uint32_t num_invalid_samples;      /* num of invalid samples */
        
} adi_apollo_fsrc_tx_vector_length_t;


#endif // !__ADI_FPGA_APOLLO_FSRC_TYPES_H__
