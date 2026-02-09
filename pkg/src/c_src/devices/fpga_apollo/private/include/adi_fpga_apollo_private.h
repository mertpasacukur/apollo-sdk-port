/*!
 * \brief Private FPGA Apollo functions
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_APOLLO_FPGA_PRIVATE_H__
#define __ADI_APOLLO_FPGA_PRIVATE_H__

#include <stdint.h>
#include <stdbool.h>
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_jesd_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief       Write a bitfield for a 32-bit register
 *
 * \param[in]    fpga    Context variable
 * \param[in]    reg     Offset of the register where the bitfiled is
 * \param[in]    mask    The bitfield mask
 * \param[in]    val     The value of the bitfield
 *
 * \return       API_CMS_ERROR_OK    if API completed successfully
 */
int32_t adi_fpga_apollo_private_write32_bitfield(adi_fpga_apollo_device_t *fpga,
				uint32_t reg, uint32_t mask, uint32_t val);

/**
 * \brief       Read a bitfield for a 32-bit register
 *
 * \param[in]    fpga    Context variable
 * \param[in]    reg     Offset of the register where the bitfiled is
 * \param[in]    mask    The bitfield mask
 * \param[out]   val     The value of the bitfield
 *
 * \return       API_CMS_ERROR_OK    if API completed successfully
 */
int32_t adi_fpga_apollo_private_read32_bitfield(adi_fpga_apollo_device_t *fpga,
                                                uint32_t reg, uint32_t mask, uint32_t *val);


/**
 * \brief   Start FPGA PRBS testing
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_private_jesd_rx_phy_prbs_start(adi_fpga_apollo_device_t *fpga);


/**
 * \brief   Start FPGA capture
 *
 * \param[in]   fpga            Context variable
 * \param[in]   skip_link_init  Option to skip link init
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_private_capture_start(adi_fpga_apollo_device_t *fpga,
                                           bool skip_link_init);


/**
 * \brief   Set the FPGA PRBS pattern type
 *
 * \param[in]   fpga            Context variable
 * \param[in]   prbs            Transceiver RX PRBS pattern check select
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_private_jesd_rx_phy_prbs_pat_set(adi_fpga_apollo_device_t *fpga,
                                                      adi_apollo_fpga_prbs_e prbs);


/**
 * \brief   Sets all FPGA JESD lanes for PRBS testing
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_private_jesd_rx_phy_prbs_lanes_set(adi_fpga_apollo_device_t *fpga);


#ifdef __cplusplus
}
#endif

#endif // !__ADI_APOLLO_FPGA_PRIVATE_H__
