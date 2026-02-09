/*!
 * \brief     FPGA Apollo clk functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_CLK_H__
#define __ADI_FPGA_APOLLO_CLK_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Sets JTX and JRX line rate division values
 *
 * \param[in] fpga              Context variable
 * \param[in] link_sel          FPGA Link to program \ref adi_fpga_apollo_link_sel_e
 * \param[in] terminal          FPGA jesd terminal \ref adi_fpga_apollo_jesd_e
 * \param[in] div               link div setting 
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_clk_line_rate_div_set(adi_fpga_apollo_device_t* fpga, uint16_t link_sel, uint16_t terminal, uint32_t div);

/**
 * \brief Gets JTX and JRX line rate division ratios
 *
 * \param[in] fpga              Context variable
 * \param[in] link_sel          FPGA Link to program \ref adi_fpga_apollo_link_sel_e
 * \param[in] terminal          FPGA jesd terminal \ref adi_fpga_apollo_jesd_e
 * \param[out] div              link div setting 
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_clk_line_rate_div_get(adi_fpga_apollo_device_t* fpga, uint16_t link_sel, uint16_t terminal, uint32_t *div);

/**
 * \brief Calculates JTX and JRX line rate division values and updates/stores 
 *        them within FPGA's clock config struct, \ref adi_fpga_clk_info_t
 *
 * \param[in] fpga              Context variable
 * \param[in] side_idx          Apollo side \ref adi_apollo_sides_e 
 * \param[in] terminal          FPGA jesd terminal \ref adi_fpga_apollo_jesd_e
 * \param[in] bitrate           lane rate
 * \param[in] ver               0: JESD204B 1: JESD204C
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_clk_line_rate_div_calc(adi_fpga_apollo_device_t* fpga, uint16_t side_idx, uint16_t terminal, uint32_t bitrate, uint32_t ver);

/**
 * \brief Program all JTX and JRX line rate div values
 *        based on data stored in fpga->state_info.clk_div
 * 
 * \param[in] fpga              Context variable
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_clk_line_rate_div_pgm(adi_fpga_apollo_device_t *fpga);

#ifdef __cplusplus
}
#endif

#endif // __ADI_FPGA_APOLLO_CLK_H__