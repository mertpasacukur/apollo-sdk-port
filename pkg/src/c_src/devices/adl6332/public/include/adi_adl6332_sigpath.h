/*!
 * \brief     ADL6332 Signal Path Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_ADL6332_SIGPATH_H__
#define __ADI_ADL6332_SIGPATH_H__

#include "adi_adl6332_sigpath_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Configure a signal path RF State.
 *
 * \param[in] adl6332    [context variable] Reference to the device.
 * \param[in] chip_id                       Target chip \ref adi_adl6332_chip_id_e
 * \param[in] rfstate                       The rfstate to configure \ref adi_adl6332_rfstate_select_e
 * \param[in] config                        Configuration structure \ref adi_adl6332_rfstate_t
 *
 * \returns 0 for success
 */
int32_t adi_adl6332_sigpath_rfstate_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_rfstate_select_e rfstate, adi_adl6332_rfstate_t *config);

/**
 * \brief Set CROSS PTAT Bias Trim
 *
 * \param[in] adl6332    [context variable] Reference to the device.
 * \param[in] chip_id                       Target chip \ref adi_adl6332_chip_id_e
 * \param[in] amp                           Target amp \ref adi_adl6332_amp_select_e
 * \param[in] trim                          PTAT Bias trim \ref adi_adl6332_ptat_bias_trm_e
 *
 * \returns 0 for success
 */
int32_t adi_adl6332_sigpath_crossp_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ptat_bias_trm_e trim);

/**
 * \brief Set CROSS ZTAT Bias Trim
 *
 * \param[in] adl6332    [context variable] Reference to the device.
 * \param[in] chip_id                       Target chip \ref adi_adl6332_chip_id_e
 * \param[in] amp                           Target amp \ref adi_adl6332_amp_select_e
 * \param[in] trim                          ZTAT Bias trim \ref adi_adl6332_ztat_bias_trm_e
 *
 * \returns 0 for success
 */
int32_t adi_adl6332_sigpath_crossz_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_ztat_bias_trm_e trim);

/**
 * \brief Set amp trim config
 *
 * \param[in] adl6332    [context variable] Reference to the device.
 * \param[in] chip_id                       Target chip \ref adi_adl6332_chip_id_e
 * \param[in] amp                           Target amp \ref adi_adl6332_amp_select_e
 * \param[in] config                        Trim config \ref adi_adl6332_trim_config_t
 *
 * \returns 0 for success
 */
int32_t adi_adl6332_sigpath_trim_config_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_amp_select_e amp, adi_adl6332_trim_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADL6332_SIGPATH_H__
