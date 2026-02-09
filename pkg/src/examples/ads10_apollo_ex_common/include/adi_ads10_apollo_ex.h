/*!
 * \brief     ADS10 Apollo examples common functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_types.h"
#include "adi_apollo_arm_types.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_fsrc.h"

#ifndef __ADI_ADS10_APOLLO_COMMON_EX_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Ads10 Apollo example code generic startup.
 *
 * \param[in] device                Context variable - Pointer to the APOLLO device data structure.
 * \param[in] dev_profile           Device profile structure pointer.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_startup(adi_apollo_device_t *device, adi_apollo_top_t *dev_profile);

/**
 * \brief  Configure Apollo HAL for the ADS10 platform
 *
 * \param[in] apollo_device         Context variable - Pointer to the APOLLO device data structure.
 * \param[in] ads10_platform        Structure containing SPI and HSCI config info.
 * \param[in] enable_hsci           1 to enable HSCI, 0 to disable
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_configure_hal(adi_apollo_device_t *apollo_device, adi_fpga_apollo_hal_config_t *ads10_platform, uint8_t enable_hsci);

/**
 * \brief  Configure startup sequence info for the ADS10 platform. Assign the FW provider.
 *
 * \param[in] apollo_device         Context variable - Pointer to the APOLLO device data structure.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_configure_startup(adi_apollo_device_t *apollo_device);


/**
 * \brief  Configure FPGA HAL for the ADS10 platform
 *
 * \param[in] fpga_device         Context variable - Pointer to the FPGA device data structure.
 * \param[in] ads10_platform      Structure containing SPI and HSCI config info.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_configure_fpga_hal(adi_fpga_apollo_device_t *fpga_device, adi_fpga_apollo_hal_config_t *ads10_platform);

/**
 * \brief  Print Apollo registers
 *
 * \param[in] device    Context variable - Pointer to the APOLLO device data structure
 * \param[in] reg       Address of first reg to print
 * \param[in] nregs     Number of regs to print starting with reg
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_print_regs(adi_apollo_device_t *device, uint32_t reg, uint32_t nregs);

/**
 * \brief  Print FPGA registers
 *
 * \param[in] fpga_device   Context variable - Pointer to the FPGA device data structure
 * \param[in] reg           Address of first reg to print
 * \param[in] nregs         Number of regs to print starting with reg
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
uint32_t adi_ads10_apollo_ex_fpga_print_regs(adi_fpga_apollo_device_t* fpga_device, uint32_t reg, uint32_t nregs);


int32_t adi_ads10_apollo_ex_reg_test(adi_apollo_device_t *device);

int32_t adi_ads10_apollo_ex_fpga_reg_test(adi_fpga_apollo_device_t *fpga_device);

int32_t adi_ads10_apollo_ex_fpga_jesd_configure(adi_fpga_apollo_device_t *fpga_device, adi_apollo_jesd_tx_cfg_t jtx[], adi_apollo_jesd_rx_cfg_t jrx[]);

/**
 * \brief  Reads FPGA capture data (16-bits), allocates and returns (4) I/Q channel data arrays (interleaved)
 *
 * \remarks This functions assumes m=4 for two sides.
 *
 * \param[in] device            Context variable - Pointer to the Apollo device data structure
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] samples_per_conv  Ptr to return val containing the number of samples per virtual converter
 * \param[in] adc_iq_data       Array of I/Q interleaved channel data. Routine allocates mem. Caller must free.
 * \param[in] num_adcs          The number of channels (4)
 *
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_fpga_capture_to_array(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint32_t* samples_per_conv, uint16_t* adc_iq_data[], uint32_t num_adcs);

/**
 * \brief  Reads FPGA capture data (12-bits), allocates and returns (4) I/Q channel data arrays (interleaved)
 *
 * \remarks This functions assumes m=4 for two sides.
 *
 * \param[in] device            Context variable - Pointer to the Apollo device data structure
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] samples_per_conv  Ptr to return val containing the number of samples per virtual converter
 * \param[in] adc_iq_data       Array of I/Q interleaved channel data. Routine allocates mem. Caller must free.
 * \param[in] num_adcs          The number of channels (4)
 *
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_fpga_capture_to_array_12b(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint32_t* samples_per_conv, uint16_t* adc_iq_data[], uint32_t num_adcs);

/**
 * \brief  FPGA operations prior to resetting the Apollo device.
 *
 * \remarks
 * This function can be called before resetting the Apollo device to terminate
 * any running transmit or capture. This will reduce power supply current and lesson the
 * transient on the power modules. Will possibly help avoid PS fault conditions.
 *
 * \param[in] device    Context variable - Pointer to the FPGA device data structure
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_fpga_pre_reset(adi_fpga_apollo_device_t *fpga_device);

/**
 * \brief  Determine ratio and FSRC N and M values from device settings
 *
 * \param[in]   fsrc_cfg    Ptr to an FSRC profile struct
 * \param[out]  fsrc_n      FSRC N result pointer
 * \param[out]  fsrc_m      FSRC M result pointer
 *
 * \return FSRC ratio N/M
 */
double adi_ads10_apollo_ex_fsrc_ratio_get(const adi_apollo_fsrc_cfg_t *fsrc_cfg, int *fsrc_n, int *fsrc_m);

/**
 * \brief Set the cnco frequency
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] terminal      Target terminal \ref adi_apollo_terminal_e
 * \param[in] cncos         Target CNCOs \ref adi_apollo_coarse_nco_select_e
 * \param[in] rate          Data rate at nco input
 * \param[in] cnco_freq     Requested Nco frequency
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_cnco_set(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, const uint16_t cncos, double rate, double cnco_freq);

/**
 * \brief Set the cnco frequency. fs and fo can be any units as long as they match (ratio is what matters)
 * 
 * \note  Does not handle negative frequency shifts TODO: Add support for negative ratio
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] terminal      Target terminal \ref adi_apollo_terminal_e
 * \param[in] cncos         Target CNCOs \ref adi_apollo_coarse_nco_select_e
 * \param[in] mod_en        Use modulus for higher res
 * \param[in] fs            Data rate at nco input
 * \param[in] fo            Requested Nco frequency
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_cnco_freq_set(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, const uint16_t cncos, bool mod_en, uint64_t fs, uint64_t fo);

/**
 * \brief Set the main fnco frequency
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] terminal      Target terminal \ref adi_apollo_terminal_e
 * \param[in] fncos         Target FNCOs \ref adi_apollo_fine_nco_select_e
 * \param[in] rate          Data rate at nco input
 * \param[in] fnco_freq     Requested NCO frequency
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_fnco_set(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, const uint16_t fncos, double rate, double fnco_freq);

/**
 * \brief Reads the range of Apollo registers. Optionally write out to file
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] file_name     File name to dump reg data. If NULL, no file is created/written.
 * \param[in] first         First APOLLO reg to read
 * \param[out] regs         Array containing register values (8-bit) from first to (first+len)
 * \param[in] len           Len of regs array
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_apollo_ex_reg_dump(adi_apollo_device_t* device, char* file_name, uint32_t first, uint8_t regs[], uint32_t len);

/**
 * \brief Reads the range of FPGA registers. Optionally write out to file
 *
 * \param[in] fpga_device   Context variable - Pointer to the FPGA device data structure
 * \param[in] file_name     File name to dump reg data. If NULL, no file is created/written.
 * \param[in] first         First FPGA reg to read
 * \param[out] regs         Array containing register values from first to (first+len)
 * \param[in] len           Len of regs array
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_apollo_ex_fpga_reg_dump(adi_fpga_apollo_device_t* fpga_device, char* file_name, uint32_t first, uint32_t regs[], uint32_t len);

/**
 * \brief Reads PFILT coeffs from a file and loads into selected PFILT bank(s)
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] terminal      Target terminal \ref adi_apollo_terminal_e
 * \param[in] pfilts        PFILT block select \ref adi_apollo_pfilt_sel_e
 * \param[in] pfilt_banks   PFILT bank select \ref adi_apollo_pfilt_bank_sel_e
 * \param[in] file_path     Path to file containing coefficients
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_pfilt_coeff_file_load(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, uint8_t pfilts, uint8_t pfilt_banks, char* file_path);


/**
 * \brief Extract useful Rx/Tx data path parameter values from a profile
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] profile       Device profile
 * \param[in] terminal      Rx or Tx path \ref adi_apollo_terminal_e
 * \param[in] side          Device side, A or B. \ref adi_apollo_sides_e
 * \param[in] idx           Channel index. \ref adi_apollo_rx_channel_idx_e \ref adi_apollo_tx_channel_idx_e
 * \param[out] dp_info      Pointer to return structure containing data path info. \ref adi_ads10_apollo_dp_info_t
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_dp_info_get(adi_apollo_device_t* device, adi_apollo_top_t* profile, adi_apollo_terminal_e terminal, uint32_t side, uint32_t idx, adi_ads10_apollo_dp_info_t* dp_info);

/**
 * \brief  Write array of 32-bit values to file as two 16-bit samples
 *
 * \param[in] file_name         Name of output file
 * \param[in] data              Array of 32-bit values, each containing two 16-bit values
 * \param[in] len               Length of data array
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_samples_16x2_to_file(char* file_name, uint32_t data[], uint32_t len);

/**
 * \brief  Write array of 32-bit values to file as two 16-bit samples, options for start offset and stride
 *
 * \param[in] file_name         Name of output file
 * \param[in] offset            Offset into data array of first sample
 * \param[in] stride            Next index increment when iterating through the data array
 * \param[in] data              Array of 32-bit words, each containing two 16-bit values
 * \param[in] len               Length of data array
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_samples_16x2_stride_to_file(char* file_name, uint32_t offset, uint32_t stride, uint32_t data[], uint32_t len);

/**
 * \brief  Write array of 16-bit samples to a file. Optional de-interleave of IQ data
 *
 * \param[in] file_name         Name of output file
 * \param[in] data              Array of 16-bit values. data may be IQ interleaved
 * \param[in] len               length of data array
 * \param[in] de_interleave     if 1, data is IQ interleaved. Separate I/Q files are written
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_samples_to_file(char* file_name, int16_t data[], uint32_t len, bool de_interleave);


/**
 * \brief  print Apollo JESD JRx link status
 *
 * \param[in] device        Context variable - Pointer to the APOLLO device data structure
 * \param[in] link          Link num
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_jesd_rx_status(adi_apollo_device_t* device, uint16_t link);

/**
 * \brief  Reads FPGA capture data and writes to file based on profile
 * \note   This function will produce a number of capture files based on the number of converters(M). 
 *         If interleaved=true, M/2 files will be generated and contain data from two converters 
 *         interleaved (I/Q) in the same file; moreover, the files will have the naming format: 
 *         `ex_func_L<link_idx>_IQ<vc_idx>_NP<bits>_DR<freq>_CR<freq>`. If interleaved=false, M files 
 *         will be generated and contain data from a single converter with the naming format: 
 *         `ex_func_L<link_idx>_m<vc_idx>_NP<bits>_DR<freq>_CR<freq>`. 
 *         Also note num_samples is the minimum number of samples per converter; the actual number 
 *         may be greater depending on jesd params and the jesd mode of other links. 
 *
 * \param[in] device            Context variable - Pointer to the Apollo device data structure
 * \param[in] profile           Pointer to profile
 * \param[in] fpga_device       Context variable - Pointer to the FPGA device data structure
 * \param[in] num_samples       Num samples to capture (may result in more than requested)
 * \param[in] file_name_base    Specifier for file names to write data
 * \param[in] interleaved       Decides if files are interleaved or seperated
 *
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */

int32_t adi_ads10_apollo_ex_fpga_capture(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_fpga_apollo_device_t *fpga_device, uint32_t num_samples, char *file_name_base, bool interleaved);


#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_H__ */