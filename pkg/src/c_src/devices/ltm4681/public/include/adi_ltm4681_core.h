/*!
 * \brief     LTM4681 Core Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_LTM4681_CORE
 * \ingroup     LTM4681
 * @{
 */

#ifndef __ADI_LTM4681_CORE_H__
#define __ADI_LTM4681_CORE_H__

/*============= I N C L U D E S ============*/
#include "adi_ltm4681_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get version of the API
 *
 * \param[in]   ltm4681     Context variable - Pointer to the LTM4681 device data structure.
 * \param[out]  apiVersion  The API version. \ref adi_ltm4681_version_t.
 *
 * \returns 0 for success
 */
int32_t adi_ltm4681_core_version_get(adi_ltm4681_device_t *ltm4681, adi_ltm4681_version_t *apiVersion);


/**
 * \brief       Sets output voltage for selected pages/channels.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[in]   vout                Array to store the channel's output voltage value in PMBus's LINEAR16 Numeric Format.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_vout_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t vout[], uint32_t num_page_channels);


/**
 * \brief       Reads ADC measured output voltage for selected pages/channels.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[out]  read_vout           Array to store the most recent ADC measured value of the channel’s output voltage in PMBus's LINEAR16 Numeric Format.
 * \param[in]   num_page_channels   Number of pages/channels from which output voltage needs to be measured.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_vout_measure(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t read_vout[], uint32_t num_page_channels);


/**
 * \brief       Reads measured output current for selected pages/channels.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[out]  read_iout           Array to store the most recent ADC measured value of the channel’s output current in PMBus's LINEAR11 Numeric Format.
 * \param[in]   num_page_channels   Number of pages/channels from which output current needs to be measured.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_iout_measure(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t read_iout[], uint32_t num_page_channels);


/**
 * \brief       Reads measured output power for selected pages/channels.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[out]  read_pout           Array to store the most recent ADC measured value of the channel’s output power in PMBus's LINEAR11 Numeric Format.
 * \param[in]   num_page_channels   Number of pages/channels from which output power needs to be measured.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_pout_measure(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t read_pout[], uint32_t num_page_channels);


/**
 * \brief       Sets the amount of time, in milliseconds, that a selected page/channel waits following the start of an ON command \n
 *              before its VOUT_EN pin enables a DC/DC converter. This delay is counted using SHARE_CLK only. \n
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[in]   ton_delay           Array to store the delay time in milliseconds, in LINEAR11 numeric format.
 * \param[in]   num_page_channels   Number of pages/channels whose TON_DELAY needs to be set.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_ton_delay_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t ton_delay[], uint32_t num_page_channels);


/**
 * \brief       Sets the amount of time, in milliseconds, that elapses after the power supply has been enabled until the LTM4681’s DAC \n
 *              soft-connects and servos the output voltage to the desired level. \n
 *              This delay is counted using SHARE_CLK if available, otherwise the internal oscillator is used. \n
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[in]   ton_rise            Array to store the rise time in milliseconds, in LINEAR11 numeric format.
 * \param[in]   num_page_channels   Number of pages/channels whose TON_RISE needs to be set.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_ton_rise_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t ton_rise[], uint32_t num_page_channels);


/**
 * \brief       Sets the amount of time, in milliseconds, that a selected page/channel waits following the OFF command \n
 *              (or CONTROLn pin is deserted) before its VOUT_EN pin disables a DC/DC converter. \n
 *              This delay is counted using SHARE_CLK only. \n
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[in]   toff_delay          Array to store the delay time in milliseconds, in LINEAR11 numeric format.
 * \param[in]   num_page_channels   Number of pages/channels whose TOFF_DELAY needs to be set.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_toff_delay_set(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t toff_delay[], uint32_t num_page_channels);


/**
 * \brief       System level delay / sleep in milliseconds.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   delay_ms            Wait / sleep time in milliseconds.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_wait_ms(adi_ltm4681_device_t *ltm4681, uint32_t delay_ms);


/**
 * \brief       Configures OPERATIONS command for selected pages/channels to turn ON or OFF.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page value corresponding to a channel. If use_global_page is true, set to 0x00.
 * \param[in]   num_page_channels   Number of pages/channels who needs to be turned ON. If use_global_page is true, set to 0x00.
 * \param[in]   ch_enable           Turn ON (True) or Turn OFF (False) selected pages/channels.
 * \param[in]   use_sequence_off    If ch_enable is false, Turn OFF in sequence (True) or immediately Turn OFF (False) selected pages/channels. If ch_enable is true, don't care.
 * \param[in]   use_global_page     If true, OPERATION command register responds to the global page command (PAGE = 0xFF) else only to selected pages.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_enable_channels(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint32_t num_page_channels, bool ch_enable, bool use_sequence_off, bool use_global_page);


/**
 * \brief       Turns ON (Enables) the channels after setting ton rise time and delay.
 *              Possible combinations:
 *              -> USE TON_DELAY COMMAND and INDIVIDUALLY PAGED CHANNELS.
 *              -> USE TON_DELAY COMMAND and GLOBALLY PAGED.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[in]   ton_delay           Array to store the delay time in milliseconds, in LINEAR11 numeric format.
 * \param[in]   ton_rise            Array to store the rise time in milliseconds, in LINEAR11 numeric format.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 * \param[in]   use_global_on       If true, using global page command (PAGE=0xFF) all channels will be turned on, else only selected pages.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_power_up(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t ton_delay[], uint16_t ton_rise[], uint32_t num_page_channels, bool use_global_on);


/**
 * \brief       Turns OFF (Disables) the channels in a sequence after setting toff delay or immediately.
 *              Possible combinations:
 *              -> USE TOFF_DELAY COMMAND and INDIVIDUALLY PAGED CHANNELS.
 *              -> USE TOFF_DELAY COMMAND and GLOBALLY PAGED.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Array to store page or channel value corresponding to DC/DC converter channels that can be managed.
 * \param[in]   toff_delay          Array to store the delay time in milliseconds, in LINEAR11 numeric format.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 * \param[in]   use_sequence_off    Turn OFF in sequence (True) or immediately Turn OFF (False) selected pages/channels.
 * \param[in]   use_global_off      If true, using global page command (PAGE=0xFF) all channels will be turned off, else only selected pages.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_power_down(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], uint16_t toff_delay[], uint32_t num_page_channels, bool use_sequence_off, bool use_global_off);

/**
 * \brief       Forces device to turn off both channels, load the operating memory from internal EEPROM, clear faults and then perform a soft-start of PWM channels, if enabled.
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_reset(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id);

/**
 * \brief       Read status commands that summarizes most critical faults or warnings
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 * \param[in]   page                Pages to get status word
 * \param[out]  status              Array of fault status structs \ref adi_ltm4681_fault_status_t
 * \param[in]   num_page_channels   Number of pages
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_fault_status_get(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id, uint8_t page[], adi_ltm4681_fault_status_t status[], uint32_t num_page_channels);

/**
 * \brief       Clear fault status registers
 *
 * \param[in]   ltm4681             Context variable - Pointer to the LTM4681 device data structure.
 * \param[in]   chan_id             Channel pair selection ID. Select either channels 0 and 1 or channels 2 and 3 for configuration. \ref adi_ltm4681_chan_pair_id_e.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ltm4681_core_fault_status_clear(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_LTM4681_CORE_H__

/*! @} */
