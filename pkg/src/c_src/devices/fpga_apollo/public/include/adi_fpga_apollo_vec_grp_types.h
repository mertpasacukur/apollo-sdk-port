/*!
 * \brief     FPGA vector group types
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_VEC_GRP_TYPES_H__
#define __ADI_FPGA_APOLLO_VEC_GRP_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif
#include "adi_cms_api_common.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core_types.h"

#define MAX_VEC_GROUP_LABEL_LEN 80

#ifndef CLIENT_IGNORE

typedef struct {
    uint64_t base_addr;
    uint64_t len;
} adi_fpga_apollo_vec_t;

typedef struct {
    char label[MAX_VEC_GROUP_LABEL_LEN];
    adi_fpga_apollo_vec_t vec[MAX_JESD_LINKS];
} adi_fpga_apollo_vec_grp_t;

#endif // !CLIENT_IGNORE

#endif // !__ADI_FPGA_APOLLO_VEC_GRP_TYPES_H__
