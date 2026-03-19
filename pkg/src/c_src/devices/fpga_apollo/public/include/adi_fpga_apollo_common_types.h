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

#define MAX_JESD_LINKS                   4
#define MAX_JESD_LANES_PER_LINK          12
/**
 * HBM Memory Map
 * Tx Addr: 0x0 ensures Tx Vector always occupies the first half of each HBM section
 * Rx Addr: Two configurations are supported depending on the memory allocation per link
 * case 1: Single Memory per link (2G per link Rx/Tx)
 * case 2: Full Memory per link (4G per link Rx/Tx)
 */
#define HBM_MEM_TX_VEC_ADDR              0x0
#define HBM_MEM_RX_CAP_ADDR_SINGLE_MEM   0x40000000
#define HBM_MEM_RX_CAP_ADDR_FULL_MEM     0x80000000
/**
 * Block RAM Map
 * Tx Addr: 0x0 ensures Tx Vector always occupies the first half of each Block RAM section
 * Rx Addr: Two configurations are supported depending on the memory allocation per link
 * case 1: Single Memory per link (2M per link Rx/Tx)
 * case 2: Full Memory per link (4M per link Rx/Tx)
 */
#define BLOCK_RAM_TX_VEC_ADDR            0x0
#define BLOCK_RAM_RX_CAP_ADDR_SINGLE_MEM 0x00040000
#define BLOCK_RAM_RX_CAP_ADDR_FULL_MEM   0x00080000

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
