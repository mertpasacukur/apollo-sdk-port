#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo examples Multi-Chip Sync and Calibration functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/

#include "adi_apollo.h"
#include "ads10_hal.h"
#include "adi_ads10_apollo_ex_types.h"
#include "adi_ads10_apollo_ex_adf4382.h"
#include "adi_ads10_apollo_ex_adf4030.h"
#include "adi_fpga_apollo_core.h"

#ifndef __ADI_ADS10_APOLLO_EX_MCS_H__
#define __ADI_ADS10_APOLLO_EX_MCS_H__

/*!
 * \brief MCS Time-of-Flight Config Structure
 */
typedef struct {
    uint32_t bsync_divider;                                 /*!< Apollo's BSYNC Divider value. */
    uint64_t fpga_ref_freq_hz;                              /*!< FPGA Reference Input Clk Freq. */
    uint64_t adf4030_ref_freq_hz;                           /*!< ADF4030 Reference Input Clk Freq. */
    uint64_t vco_out_freq_hz;                               /*!< ADF4030 VCO Output Clk Freq. */
    uint64_t bsync_out_sysref_freq_hz;                      /*!< ADF4030 BSYNC Output channel's SYSREF Freq. Same for Apollo and FPGA SYSREF. */
    adi_adf4030_channel_id_e bsync_in_ref_ch;               /*!< ADF4030 BSYNC Input channel's Reference SYSREF Freq. */
    adi_adf4030_channel_id_e bsync_out_apollo_sysref_ch;    /*!< ADF4030 BSYNC Output channel used for Apollo SYSREF. */
    adi_adf4030_channel_id_e bsync_out_fpga_sysref_ch;      /*!< ADF4030 BSYNC Output channel used for FPGA SYSREF. */
} mcs_tof_config_t;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Load Apollo's MCS Calibration settings onto FW and configure ADF4382 for MCS operation
 *
 * \param[in]   device                  Context variable - Pointer to the APOLLO device data structure
 * \param[in]   adf4382                 Context variable - Pointer to the ADF4382 device structure
 * \param[in]   adf4382_ref_freq_hz     ADF4382 Reference Input Freq, in Hz
 * \param[in]   adf4382_rfout_freq_hz   ADF4382 Output Freq, in Hz
 * \param[in]   ext_sysref_freq_hz      External Sysref Freq provide to Apollo, in Hz
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_init_cal_setup(adi_apollo_device_t *device,
                                               adi_adf4382_device_t *adf4382,
                                               uint64_t adf4382_ref_freq_hz,
                                               uint64_t adf4382_rfout_freq_hz,
                                               uint64_t ext_sysref_freq_hz);


/**
 * \brief   Configure Apollo's MCS Calibration FW for tracking calibration
 *
 * \param[in]   device                  Context variable - Pointer to the APOLLO device data structure
 * \param[in]   mcs_track_decimation    TDC Decimation rate
 * \param[in]   initialize_track_cal    Set 0: If tracking cal init already done by device profile or mcs_cal_config struct
 *                                      Set 1: To initialize tracking cal settings and Apollo's TDC, if not done earlier
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_tracking_cal_setup(adi_apollo_device_t *device, uint16_t mcs_track_decimation, uint8_t initialize_track_cal);


/**
 * \brief   Validate successful completion of MCS Init Calibration
 *
 * \param[in]   device                  Context variable - Pointer to the APOLLO device data structure
 * \param[in]   cal_status              Pointer to struct that holds MCS Init cal status results. \ref adi_apollo_mcs_cal_init_status_t
 * \param[in]   dev_clk_hz              Apollo's device clock freq, in Hz
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_init_cal_validate(adi_apollo_device_t *device,
                                                  adi_apollo_mcs_cal_init_status_t *cal_status,
                                                  uint64_t dev_clk_hz);


/**
 * \brief   Validate successful completion of MCS Init Calibration
 *
 * \param[in]   device                  Context variable - Pointer to the APOLLO device data structure
 * \param[in]   cal_status              Pointer to struct that holds MCS Tracking cal status results. \ref adi_apollo_mcs_cal_status_t
 * \param[in]   hw_bleed_pol            ADF4382 Bleed Current Polarity value.
 * \param[in]   hw_current_coarse       ADF4382 Bleed Current Coarse value.
 * \param[in]   hw_current_fine         ADF4382 Bleed Current Fine value.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_tracking_cal_validate(adi_apollo_device_t *device,
                                                      adi_apollo_mcs_cal_status_t *cal_status,
                                                      uint8_t hw_bleed_pol,
                                                      int8_t hw_current_coarse,
                                                      int16_t hw_current_fine);


/**
 * \brief   Validate successful completion of MCS Init Calibration
 *
 * \param[in]   cal_status              Pointer to struct that holds MCS Init cal status results. \ref adi_apollo_mcs_cal_init_status_t
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_init_cal_status_print(adi_apollo_mcs_cal_init_status_t *cal_status);


/**
 * \brief   Validate successful completion of MCS Init Calibration
 *
 * \param[in]   cal_status              Pointer to struct that holds MCS Tracking cal status results. \ref adi_apollo_mcs_cal_status_t
 * \param[in]   print_full_state        Flag used to print full cal_status if set to 1 or just validation status when set to 0.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_track_cal_status_print(adi_apollo_mcs_cal_status_t *cal_status, uint8_t print_full_state);


/**
 * \brief   Measure Path delay between ADF4030 and Apollo in FemtoSeconds
 *
 * \param[in]   device                  Context variable - Pointer to the APOLLO device data structure
 * \param[in]   adf4030                 Context variable - Pointer to the ADF4030 device structure
 * \param[in]   mcs_tof                 Pointer to MCS Time-of-Flight Config Structure
 * \param[out]  path_delay              Measured Path delay between ADF4030 and Apollo in FemtoSeconds
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_measurement(adi_apollo_device_t *device,
                                                                      adi_adf4030_device_t *adf4030,
                                                                      mcs_tof_config_t *mcs_tof,
                                                                      int64_t *path_delay);

/**
 * \brief   Apply delay offset between ADF4030 and Apollo to account for path delay
 *
 * \param[in]   adf4030                 Context variable - Pointer to the ADF4030 device structure
 * \param[in]   mcs_tof                 Pointer to MCS Time-of-Flight Config Structure
 * \param[in]   align_cycles_iters      The desired number of alignment cycles to be executed minus 1
 * \param[in]   path_delay              The delay offset which will be added to Target Channel in FemtoSeconds
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_offset(adi_adf4030_device_t *adf4030,
                                                                 mcs_tof_config_t *mcs_tof,
                                                                 uint8_t align_cycles_iters,
                                                                 int64_t path_delay);

/**
 * \brief   Measure Path delay between ADF4030 and FPGA in FemtoSeconds
 *
 * \param[in]   adf4030                 Context variable - Pointer to the ADF4030 device structure
 * \param[in]   fpga_device             Context variable - Pointer to the FPGA device structure
 * \param[in]   mcs_tof                 Pointer to MCS Time-of-Flight Config Structure
 * \param[out]  path_delay              Measured Path delay between ADF4030 and FPGA in FemtoSeconds
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_measurement(adi_adf4030_device_t *adf4030,
                                                                    adi_fpga_apollo_device_t* fpga_device,
                                                                    mcs_tof_config_t *mcs_tof,
                                                                    int64_t *path_delay);
/**
 * \brief   Load Apollo's MCS Calibration settings onto FW and configure ADF4382 for MCS operation
 *
 * \param[in]   adf4030                 Context variable - Pointer to the ADF4030 device structure
 * \param[in]   mcs_tof                 Pointer to MCS Time-of-Flight Config Structure
 * \param[in]   align_cycles_iters      The desired number of alignment cycles to be executed minus 1
 * \param[in]   path_delay              The delay offset which will be added to Target Channel in FemtoSeconds
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully.
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_offset(adi_adf4030_device_t *adf4030,
                                                               mcs_tof_config_t *mcs_tof,
                                                               uint8_t align_cycles_iters,
                                                               int64_t path_delay);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_MCS_H__ */

#endif /* !defined(VERSAL_PLATFORM) */
