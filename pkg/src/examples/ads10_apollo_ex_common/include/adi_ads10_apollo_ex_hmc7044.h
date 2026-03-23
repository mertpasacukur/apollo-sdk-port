/*!
 * \brief     ADS10 Apollo examples common HMC7044 functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_hmc7044_device_types.h"
#include "ads10_hal.h"

#include "ads10_hal.h"

#ifndef __ADI_ADS10_APOLLO_EX_COMMON_HMC7044_H__
#define __ADI_ADS10_APOLLO_EX_COMMON_HMC7044_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Initializes HMC7044 device HAL and for FMCB sets up means for external GPIO control.
 *
 * \param[in]   hmc7044                 Context variable - Pointer to the HMC7044 device data structure
 * \param[in]   sdo_en_context          User device context needed for external GPIO control by sdo_en_fcn function
 * \param[in]   sdo_en_fcn              Ptr to Function that can configure alternate GPIO as SPI SDO
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_hmc7044_hal_config(adi_hmc7044_device_t *hmc7044, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

/**
 * \brief   Initializes and configures HMC7044.
 *
 * \param[in]   hmc7044                 Context variable - Pointer to the HMC7044 device data structure
 * \param[in]   ref_freq_hz             Reference Freq (in Hertz) provided to HMC7044.
 * \param[in]   sysref_hz               Sysref Freq (in Hertz) to output by HMC7044. \ref adi_hmc7044_device_rational_freq_t
 * \param[in]   fpga_ref_hz             FPGA Ref Freq (in Hertz) to output by HMC7044. \ref adi_hmc7044_device_rational_freq_t
 * \param[in]   dev_ref_clk_hz          Apollo dev clk PLL ref (in Hertz). 0 if unused. \ref adi_hmc7044_device_rational_freq_t
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_hmc7044_startup(adi_hmc7044_device_t *hmc7044,
                                            uint64_t ref_freq_hz,
                                            adi_hmc7044_device_rational_freq_t *sysref_hz,
                                            adi_hmc7044_device_rational_freq_t *fpga_ref_hz,
                                            adi_hmc7044_device_rational_freq_t *dev_ref_clk_hz);

/**
 * \brief   Perform reset to all HMC7044 dividers and FMC and sync by running reseed.
 *
 * \param[in]   hmc7044                 Context variable - Pointer to the HMC7044 device data structure
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_hmc7044_reset_fsm_reseed(adi_hmc7044_device_t *hmc7044);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_COMMON_HMC7044_H__ */
