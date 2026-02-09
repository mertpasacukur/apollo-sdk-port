/*!
 * \brief     ADS10 Apollo examples common clocking functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_fpga_apollo_types.h"
#include "apollo_cpu_device_profile_types.h"
#include "adi_apollo.h"

#ifndef __ADI_ADS10_APOLLO_EX_CLK_H__
#define __ADI_ADS10_APOLLO_EX_CLK_H__

typedef enum {
    ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER = 0x01,   /*!< Set Device Clk Mode to External Center (CLK_C) */
    ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_DUAL   = 0x02,   /*!< Set Device Clk Mode to External Dual (CLK_A + CLK_B) */
    ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382         = 0x04,   /*!< Set Device Clk Mode to ADF4382 */
    ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL       = 0x08,   /*!< Set FPGA Clk Mode to External (ADS10 EXT_CLK) */
    ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_FMC            = 0x10    /*!< Set FPGA Clk Mode to FMC (HMC7044) */
} adi_ads10_apollo_clk_mode_e;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Configures the ADS10/EVB clocks.
 *
 * \param[in] fpga_device      Pointer to the FPGA device data structure
 * \param[in] ltc6955_clk_khz  Input frequency to the LTC6955 to drive the HMC7044 and ADF4382
 * \param[in] dev_clk_khz      Device clk for Apollo
 * \param[in] digclk_cycles    Digital clk cycles.  Used for calculating SYSREF
 * \param[in] divg_mode        The ratio of digital clock to device clock.  Used for calculating SYSREF
 * \param[in] lane_rate_khz    JESD lane rate
 * \param[in] fpga_clk_div     FPGA clk divider; 204C = 66; 204B = 40
 * \param[in] mode             ADS10/EVB clocking mode \ref adi_ads10_apollo_clk_mode_e
 *
 * \return API_CMS_ERROR_OK    API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_apollo_ex_configure_clks(adi_fpga_apollo_device_t *fpga_device,
                                           uint32_t ltc6955_clk_khz,
                                           uint32_t dev_clk_khz,
                                           uint16_t digclk_cycles,
                                           adi_apollo_divg_mode_e divg_mode,
                                           uint32_t lane_rate_khz,
                                           uint8_t fpga_clk_div,
                                           adi_ads10_apollo_clk_mode_e mode);

/**
 * \brief Configures the ADS10/EVB clocks based on device profile.
 *
 * \param[in] fpga_device      Pointer to the FPGA device data structure
 * \param[in] ltc6955_clk_khz  Input frequency to the LTC6955 to drive the HMC7044 and ADF4382
 * \param[in] profile          Apollo device profile \ref adi_apollo_top_t
 * \param[in] mode             ADS10/EVB clocking mode \ref adi_ads10_apollo_clk_mode_e
 *
 * \return API_CMS_ERROR_OK    API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_apollo_ex_configure_profile_clks(adi_fpga_apollo_device_t *fpga_device,
                                                   uint32_t ltc6955_clk_khz,
                                                   adi_apollo_top_t *profile,
                                                   adi_ads10_apollo_clk_mode_e mode);

/**
 * \brief Displays clk power level, if ADF4382 calibrate until power level is good.
 *
 * \param[in]   device                          Pointer to the Apollo device structure
 * \param[in]   clk_mode                        ADS10/EVB clocking mode \ref adi_ads10_apollo_clk_mode_e
 * \param[in]   rfout_freq_hz                   Expected Output Freq (in Hertz) from ADF4382.
 * \param[in]   ref_freq_hz                     Reference Freq (in Hertz) provided to ADF4382.
 *
 * \return  API_CMS_ERROR_OK    API Completed Successfully
 * \return  <0                  Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_clk_power_cal(adi_apollo_device_t *device,
                                          adi_ads10_apollo_clk_mode_e clk_mode,
                                          uint64_t rfout_freq_hz,
                                          uint64_t ref_freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_CLK_H__ */
