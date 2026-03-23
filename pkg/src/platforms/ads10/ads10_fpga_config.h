#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     Basic FPGA config functions
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADS10_FPGA_CONFIG_H__
#define __ADS10_FPGA_CONFIG_H__


#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Loads the image file into the FPGA
 *
 * \param[in] file[]  FPGA image file as byte stream
 * \param[in] length  Size of the FPGA image file in bytes
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t ads10_fpga_config_load_image(uint8_t file[], uint32_t length);
    
#ifdef __cplusplus
}
#endif

#endif /* __ADS10_FPGA_CONFIG_H__ */
#endif /* !defined(VERSAL_PLATFORM) */
