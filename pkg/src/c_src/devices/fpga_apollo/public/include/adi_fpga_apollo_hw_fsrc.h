/*!
 * \brief     Basic FPGA Apollo functions for handling H/W FSRC.
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_HW_FSRC_H__
#define __ADI_FPGA_APOLLO_HW_FSRC_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_hw_fsrc_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************** FPGA FSRC API ********************/ 

/**
 * \brief configures FSRC TX Sequencer
 *
 * \param[in] fpga                  Context variable - Pointer to the FPGA device data structure 
 * \param[in] count                 counter values \ref adi_fpga_apollo_hw_fsrc_count_t
 * 
 * \return API_CMS_ERROR_OK         API Completed Successfully
 * \return <0                       Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_hw_fsrc_tx_sequencer_config(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_hw_fsrc_count_t *count);

/**
 * \brief    Rx HW FSRC enable get
 *
 * \param[in]   fpga                Context variable
 * \param[out]  enable              Ptr to result flag. 0 indicates HW FSRC disabled, 1 indicates enabled
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_rx_enable_get(adi_fpga_apollo_device_t *fpga, bool *enable);

/**
 * \brief    Rx HW FSRC enable set
 *
 * \param[in]   fpga                Context variable
 * \param[in]   enable              0 to disable HW FSRC, 1 to enable
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_rx_enable_set(adi_fpga_apollo_device_t *fpga, bool enable);

/**
 * \brief    Tx HW FSRC enable get
 *
 * \param[in]   fpga                Context variable
 * \param[out]  enable              Ptr to result flag. 0 indicates HW FSRC disabled, 1 indicates enabled
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_tx_enable_get(adi_fpga_apollo_device_t *fpga, bool *enable);

/**
 * \brief    Tx HW FSRC enable set
 *
 * \param[in]   fpga                Context variable
 * \param[in]   enable              0 to disable HW FSRC, 1 to enable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_tx_enable_set(adi_fpga_apollo_device_t *fpga, bool enable);

/**
 * \brief    Set the HW FSRC ratio given the n/m ratio
 *
 * FSRC ratio is n/m. The ratio must between 1 and 2, inclusive.
 * 
 * \param[in]   fpga                Context variable
 * \param[in]   links               0 to disable HW FSRC, 1 to enable
 * \param[in]   n                   Numerator (n > m)
 * \param[in]   m                   Denominator (m < n)
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_ratio_set(adi_fpga_apollo_device_t *fpga, adi_apollo_jesd_link_select_e links, uint32_t n, uint32_t m);

/**
 * \brief    HW Sample repeat enable set
 *
 * \param[in]   fpga                Context variable
 * \param[in]   terminal            FPGA jesd terminal \ref adi_fpga_apollo_jesd_e
 * \param[in]   enable              Enable Sample repeat on all links powered up (only decimation ratio = 2 supported)
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_sr_enable_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint8_t enable);

/**
 * \brief    HW Sample repeat enable get
 *
 * \param[in]   fpga                Context variable
 * \param[in]   terminal            FPGA jesd terminal \ref adi_fpga_apollo_jesd_e 
 * \param[out]  links_enable        Sample repeat enable status (1 bit per link) 
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_hw_fsrc_sr_enable_get(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint32_t *links_enable);

#ifdef __cplusplus
}
#endif
    
#endif /* __ADI_FPGA_APOLLO_HW_FSRC_H__ */