/*!
 * \brief     ADS10 Apollo common examples to string converters
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_apollo.h"
#include "adi_ads10_apollo_ex_types.h"

#ifndef __ADI_ADS10_APOLLO_COMMON_EX_TO_STR_H__
#define __ADI_ADS10_APOLLO_COMMON_EX_TO_STR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create string representation of TMU data
 * 
 * The tmu_data is obtained from calling API \ref adi_apollo_device_tmu_get
 *
 * \param[in] tmu_data          TMU data \ref adi_apollo_device_tmu_data_t
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_tmu_data_to_str(const adi_apollo_device_tmu_data_t* tmu_data, char str_buff[], uint32_t max_str_len);

/**
 * \brief Create string representation of ADC bgcal data
 * 
 * The bgcal_state is obtained from calling API \ref adi_apollo_adc_bgcal_state_get
 *
 * \param[in] adcs              ADC selection mask \ref adi_apollo_adc_select_e
 * \param[in] bgcal_state       Array of \ref adi_apollo_adc_bgcal_state_t structs
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_adc_bgcal_to_str(uint16_t adcs, const adi_apollo_adc_bgcal_state_t bgcal_state[], char str_buff[], uint32_t max_str_len);


/**
 * \brief Create string representation of UUID
 * 
 * The uuid is obtained from calling API \ref adi_apollo_device_uuid_get
 *
 * \param[in] uuid              Byte array containing uuid
 * \param[in] uuid_len          Length of uuid array
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_uuid_to_str(uint8_t uuid[], uint32_t uuid_len, char str_buff[], uint32_t max_str_len);

/**
 * \brief Create string representation of data path info
 * 
 * The dp_info param is obtained from calling API \ref adi_ads10_ex_dp_info_get
 *
 * \param[in] dp_info           Pointer to a adi_ads10_apollo_dp_info_t struct
 * \param[in] profile_name      Profile name or other description
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_dp_info_to_str(adi_ads10_apollo_dp_info_t *dp_info,  char *profile_name, char str_buff[], uint32_t max_str_len);

/**
 * \brief Create string representation of Apollo JRx IRQ state 
 * 
 * \param[in] device            Context variable - Pointer to the APOLLO device data structure 
 * \param[in] irq_mask          IRQ mask
 * \param[in] clear             1: Clear IRQ state 0: Do not clear
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_jrx_irq_to_str(adi_apollo_device_t *device, uint32_t irq_mask, uint8_t clear, char str_buff[], uint32_t max_str_len);

/**
 * \brief Create string representation of FSRC dynamic reconfiguration config
 * 
 * \param[in] fsrc_ratio_cfg    Pointer to FSRC dynamic reconfiguration config
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_fsrc_dr_cfg_to_str(adi_ads10_apollo_fsrc_dr_cfg_t *fsrc_ratio_cfg, char str_buff[], uint32_t max_str_len);

/**
 * \brief Create string representation of Sample Repeat dynamic reconfiguration config
 * 
 * \param[in] sr_ratio_cfg      Pointer to Sample Repeat dynamic reconfiguration config
 * \param[in] str_buff          Result string buffer
 * \param[in] max_str_len       Max length of str_buff
 *
 * \return API_CMS_ERROR_OK                 API Completed Successfully
 * \return <0
 */
int32_t adi_ads10_ex_sr_dr_cfg_to_str(adi_ads10_apollo_sr_dr_cfg_t *sr_ratio_cfg, char str_buff[], uint32_t max_str_len);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_COMMON_EX_TO_STR_H__ */
