/*!
 * \brief     FPGA Apollo COMMON types.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

 /*!
  * \addtogroup  ADI_FPGA_APOLLO_COMMON
  * \ingroup     ADI_FPGA_APOLLO
  * @{
  */
#ifndef __ADI_FPGA_APOLLO_COMMON_TYPES_H__
#define __ADI_FPGA_APOLLO_COMMON_TYPES_H__

#include "adi_cms_api_common.h"

#define MAX_JESD_LINKS 4
#define MAX_JESD_VC 4
#define HBM_MEM_TX_VEC_ADDR 0x80000000
#define BLOCK_RAM_TX_VEC_ADDR 0x00080000

typedef enum {
    ADI_FPGA_APOLLO_LINK_NONE   = 0x0,                        /*!< No Links */
    ADI_FPGA_APOLLO_LINK_0      = 0x1,                        /*!< Link 0 */
    ADI_FPGA_APOLLO_LINK_1      = 0x2,                        /*!< Link 1 */
    ADI_FPGA_APOLLO_LINK_2      = 0x4,                        /*!< Link 2 */
    ADI_FPGA_APOLLO_LINK_3      = 0x8,                        /*!< Link 3 */
    ADI_FPGA_APOLLO_LINK_ALL    = 0xF,                        /*!< All Links */
} adi_fpga_apollo_link_sel_e;

typedef enum {
    ADI_FPGA_APOLLO_JRX          = 0x0,                 /*!< RX TERMINAL SIGNAL PATH*/
    ADI_FPGA_APOLLO_JTX          = 0x1,                 /*!< TX TERMINAL SIGNAL PATH */
    ADI_FPGA_APOLLO_JESD_NUM     = 0x2,                 /*!< NUM JESD Paths */
} adi_fpga_apollo_jesd_e;

#endif // !__ADI_FPGA_APOLLO_COMMON_TYPES_H__
/*! @} */
