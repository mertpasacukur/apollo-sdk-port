/*!
 * \brief     Basic FPGA Apollo HW FSRC types
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_HW_FSRC_TYPES_H__
#define __ADI_FPGA_APOLLO_HW_FSRC_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include "adi_cms_api_common.h"
#include "adi_apollo_common.h"

typedef struct {
    uint16_t gpio_change_cnt;               /*!< GPIO change count      */
    uint16_t first_trig_cnt;                /*!< First trigger count    */
    uint16_t second_trig_cnt;               /*!< Second Trigger count   */
    uint16_t fsrc_accum_reset_cnt;          /*!< FSRC accum reset count */
    uint16_t rx_delay_cnt;                  /*!< Rx capture count */
} adi_fpga_apollo_hw_fsrc_count_t;

#endif // !__ADI_FPGA_APOLLO_HW_FSRC_TYPES_H__
