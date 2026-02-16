#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples common FPGA functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__linux__)
#include <unistd.h>
#endif

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_apollo_arm_types.h"
#include "adi_fpga_apollo_types.h"
#include "adi_ads10_apollo_ex_types.h"


#ifndef __ADI_ADS10_APOLLO_COMMON_EX_FPGA_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_FPGA_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
* \brief  Configure FPGA from Apollo device profile
*
* \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
* \param[in] profile           Pointer to Apollo profile struct
*
* \return API_CMS_ERROR_OK                     API Completed Successfully.
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_fpga_profile_init(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile);

/**
* \brief  Configure FPGA FSRC values
*
* \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
* \param[in] profile           Pointer to profile struct
* \param[in] terminal          Target terminal \ref adi_apollo_terminal_e
*
* \return API_CMS_ERROR_OK                     API Completed Successfully.
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_fpga_fsrc_init(adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t *profile, uint32_t terminal);

/**
* \brief  Configure FPGA FSRC values
*
* \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
* \param[in] fsrc_config       FSRC dynamic reconfig settings
* \param[in] terminal          Target terminal \ref adi_apollo_terminal_e
*
* \return API_CMS_ERROR_OK                     API Completed Successfully.
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_fpga_fsrc_pgm(adi_fpga_apollo_device_t *fpga_device, adi_ads10_apollo_fsrc_dr_cfg_t *fsrc_config, uint32_t terminal);

/**
* \brief  Program FPGA sample repeat
*
*         Set sample repeat parameters in FPGA state info and enable HW sample repeat
*         if supported.
*
* \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
* \param[in] sr_config         SR dynamic reconfig settings
* \param[in] terminal          Target terminal \ref adi_apollo_terminal_e
*
* \return API_CMS_ERROR_OK                     API Completed Successfully.
* \return <0                                   Failed. \ref adi_cms_error_e for details.
*/
int32_t adi_ads10_apollo_ex_fpga_sr_pgm(adi_fpga_apollo_device_t *fpga_device, adi_ads10_apollo_sr_dr_cfg_t *sr_config, uint32_t terminal);

/**
 * \brief       Program All FPGA Tx lane parameters
 *
 * \param[in]   fpga            Context variable
 * \param[in]   tx_diff_ctrl    gt_tx_diff_ctrl val
 * \param[in]   tx_pre_cur      gt_tx_pre_cursor val
 * \param[in]   tx_post_cur     gt_tx_post_cursor val
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_ads10_apollo_ex_fpga_tx_phy_drive_config_all(adi_fpga_apollo_device_t *fpga, uint8_t tx_diff_ctrl, uint8_t tx_pre_cur, uint8_t tx_post_cur);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_FPGA_H__ */

#endif /* !defined(VERSAL_PLATFORM) */
