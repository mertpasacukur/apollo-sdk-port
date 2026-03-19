/*!
 * \brief     ADS10 Apollo examples Multi-Chip Sync and Calibration function implementations
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_ads10_apollo_ex_mcs.h"
#include "adi_apollo_bf_mcs_tdc.h"
#include "adi_apollo_bf_mcs_sync.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_jesd_registers.h"
#include "ads10_fpga.h"

// #define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define MY_PRINTF(...) printf(__VA_ARGS__)
#else
#define MY_PRINTF(...)
#endif

#define CENTER_SYSREF_ARR_IDX       0
#define A_SIDE_SYSREF_ARR_IDX       1
#define B_SIDE_SYSREF_ARR_IDX       2
#define NUM_APOLLO_SYSREF_CLK       3

static __maybe_unused int32_t calculate_phase_value_fs(adi_adf4382_device_t *adf4382,
                                                       uint64_t adf4382_ref_freq_hz,
                                                       uint8_t phase_adjustment);

static __maybe_unused int32_t _apollo_delta_t0_measurement(adi_apollo_device_t *device,
                                                           adi_adf4030_device_t *adf4030,
                                                           mcs_tof_config_t *mcs_tof,
                                                           int64_t apollo_delta_t0[],
                                                           uint32_t arr_len);

static __maybe_unused int32_t _apollo_delta_t1_measurement(adi_apollo_device_t *device,
                                                           adi_adf4030_device_t *adf4030,
                                                           mcs_tof_config_t *mcs_tof,
                                                           int64_t apollo_delta_t1[],
                                                           uint32_t arr_len);

static __maybe_unused int32_t _apollo_adf4030_tdc_measurement(adi_adf4030_device_t *adf4030,
                                                              mcs_tof_config_t *mcs_tof,
                                                              int64_t adf4030_delta[],
                                                              uint32_t arr_len);

static __maybe_unused int32_t _restore_pre_apollo_path_delay_measurement_setup(adi_apollo_device_t *device,
                                                                               adi_adf4030_device_t *adf4030,
                                                                               mcs_tof_config_t *mcs_tof);

static __maybe_unused int32_t _fpga_clock_0_measurement(adi_fpga_apollo_device_t *fpga_device,
                                                        adi_adf4030_device_t *adf4030,
                                                        mcs_tof_config_t *mcs_tof,
                                                        int32_t *clock_0);

static __maybe_unused int32_t _fpga_clock_1_measurement(adi_fpga_apollo_device_t *fpga_device,
                                                        adi_adf4030_device_t *adf4030,
                                                        mcs_tof_config_t *mcs_tof,
                                                        int32_t *clock_1);

static __maybe_unused int32_t _fpga_clock_out_tap_delay_measurement(adi_fpga_apollo_device_t *fpga_device,
                                                                    adi_adf4030_device_t *adf4030,
                                                                    mcs_tof_config_t *mcs_tof,
                                                                    int64_t *total_tap_delay);

static __maybe_unused int32_t _restore_pre_fpga_path_delay_measurement_setup(adi_fpga_apollo_device_t *fpga_device,
                                                                             adi_adf4030_device_t *adf4030,
                                                                             mcs_tof_config_t *mcs_tof);

static __maybe_unused int32_t _fpga_sysref_setup(adi_fpga_apollo_device_t *fpga_device, uint32_t sysref_glblclk_ratio);

static __maybe_unused int32_t _fpga_sysref_phase_get(adi_fpga_apollo_device_t *fpga_device, int32_t *clock_phase);

static __maybe_unused int32_t _adf4030_fpga_delay_offset(adi_adf4030_device_t *adf4030,
                                                         mcs_tof_config_t *mcs_tof,
                                                         uint8_t align_cycles_iters,
                                                         int64_t delay_fs);


/*==================== P U B L I C   A P I   C O D E ====================*/

int32_t adi_ads10_apollo_ex_mcs_init_cal_setup(adi_apollo_device_t *device,
                                               adi_adf4382_device_t *adf4382,
                                               uint64_t adf4382_ref_freq_hz,
                                               uint64_t adf4382_rfout_freq_hz,
                                               uint64_t ext_sysref_freq_hz)
{
    int32_t err = 0;

    adi_apollo_ex_adf4382_mcs_init_config_t adf4382_mcs_init_config = {
        .bleed_word_config = {
            .bleed_current_coarse = 0,
            .bleed_current_fine   = 0
        },
        .bleed_pol = ADI_ADF4382_CPCTRL_BLEED_POLARITY_SINK,
        .bleed_adj_cal = 0,
        .phase_adj_config = {
            .ref_freq_hz    = adf4382_ref_freq_hz,    // ltc6955_clk_hz
            .rfout_freq_hz  = adf4382_rfout_freq_hz,
            .phase_mode     = ADI_ADF4382_PHASE_ADJ_BLEED_CURRENT,
            .trigger_mode   = ADI_ADF4382_PHASE_ADJ_TRIGGER_2WIRE
        },
        .en_bleed = 1,
        .en_phase_resync = 1,
        .en_auto_align = 0,
    };

    uint8_t phase_adj_rb = 0;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(adf4382_ref_freq_hz, ADI_ADF4382_REF_MIN, ADI_ADF4382_REF_MAX);
    ADI_CMS_RANGE_CHECK(adf4382_rfout_freq_hz, ADI_ADF4382_RFOUT_MIN, ADI_ADF4382_RFOUT_MAX);

    // Optional: Calibration values for offset between internal and external SYSREF may be customized to align with hardware setup
    if (!device->dev_info.is_dual_clk) {
        err = adi_apollo_mcs_cal_parameter_set(device, MCS_OFFSET_C_FEMTOSECONDS_INT64, 0);
        ADI_APOLLO_ERROR_RETURN(err);
    } else {
        err = adi_apollo_mcs_cal_parameter_set(device, MCS_OFFSET_A_FEMTOSECONDS_INT64, 0);
        ADI_APOLLO_ERROR_RETURN(err);
        err = adi_apollo_mcs_cal_parameter_set(device, MCS_OFFSET_B_FEMTOSECONDS_INT64, 0);
        ADI_APOLLO_ERROR_RETURN(err);
    }

    adf4382_mcs_init_config.phase_adj_config.phase_value_fs = calculate_phase_value_fs(adf4382,
                                                                                       adf4382_mcs_init_config.phase_adj_config.ref_freq_hz, 1);

    err = adi_ads10_apollo_ex_adf4382_mcs_init_config(adf4382, &adf4382_mcs_init_config, &phase_adj_rb);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_tracking_cal_setup(adi_apollo_device_t *device, uint16_t mcs_track_decimation, uint8_t initialize_track_cal)
{
    int32_t err = 0;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_RANGE_CHECK(initialize_track_cal, 0, 1);

    // Set MCS tracking cal decimation for more precise TDC measurements.
    err = adi_apollo_mcs_cal_tracking_decimation_set(device, mcs_track_decimation);
    ADI_CMS_ERROR_RETURN(err);

    // Optional: Calibration values for setting tracking offset between internal and external SYSREF may be customized to align with hardware setup
    err = adi_apollo_mcs_cal_parameter_set(device, MCS_ADF4382_TRACK_TARGET_0_INT32, 0);
    ADI_APOLLO_ERROR_RETURN(err);

    if (device->dev_info.is_dual_clk) {
        err = adi_apollo_mcs_cal_parameter_set(device, MCS_ADF4382_TRACK_TARGET_1_INT32, 0);
        ADI_APOLLO_ERROR_RETURN(err);
    }

    // Enable MCS Tracking Cal. Tracking decimation needs to be updated before this API call.
    err = adi_apollo_mcs_cal_tracking_enable(device, 1);
    ADI_CMS_ERROR_RETURN(err);

    // Initialize MCS Tracking Cal if not done by device profile or mcs_cal_config struct (i.e. cal_config.track_initialize = 1).
    if (initialize_track_cal) {
        err = adi_apollo_mcs_cal_tracking_initialize_set(device);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_mcs_init_cal_validate(adi_apollo_device_t *device,
                                                  adi_apollo_top_t *profile,
                                                  adi_apollo_mcs_cal_init_status_t *cal_status)
{
    int32_t err = 0;
    uint8_t is_locked = 0;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    ADI_CMS_NULL_PTR_CHECK(cal_status);

    if (profile->clk_cfg.single_dual_clk_sel == true) {
        if ((profile->mcs_cfg.aside_sysref.sysref_present == false) && (profile->mcs_cfg.bside_sysref.sysref_present == false)) {
            printf("is_A_Locked: %d.\n", cal_status->data.is_A_Locked);
            is_locked = cal_status->data.is_A_Locked;
        } else {
            printf("is_A_Locked: %d, is_B_Locked: %d.\n", cal_status->data.is_A_Locked, cal_status->data.is_B_Locked);
            // To-Do: Validate this on single side SYSREF only profiles.
            is_locked = cal_status->data.is_A_Locked | cal_status->data.is_B_Locked;
        }
    } else {
        if (profile->mcs_cfg.center_sysref.sysref_present == true) {
            printf("is_C_Locked: %d.\n", cal_status->data.is_C_Locked);
            is_locked = cal_status->data.is_C_Locked;
        } else {
            is_locked = 0;
            printf("Invalid Profile selected. No SYSREF needed.\n");
            return API_CMS_ERROR_INVALID_PROFILE_SELECT;
        }
    }

    // Check if sysref is locked
    if (is_locked == 0) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "MCS Init Cal did not lock SysRefs.\n");
        err = API_CMS_ERROR_MCS_INIT_CAL_ERROR;
        goto end;
    }

    // Check for Cal errors
    if (cal_status->hdr.errorCode != 0) {
        printf("errorCode: %d.\n", cal_status->hdr.errorCode);
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "MCS Init Cal Apollo CPU errorCode: 0x%X.\n", cal_status->hdr.errorCode);
        err = API_CMS_ERROR_MCS_INIT_CAL_ERROR;
        goto end;
    }

    if (cal_status->mcsErr != 0) {
        printf("mcsErr: %d.\n", cal_status->mcsErr);
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "MCS Init Cal MCS errorCode: 0x%X.\n", cal_status->mcsErr);
        err = API_CMS_ERROR_MCS_INIT_CAL_ERROR;
        goto end;
    }

end:
    adi_apollo_hal_log_write(device, (err == API_CMS_ERROR_OK) ? ADI_CMS_LOG_API : ADI_CMS_LOG_ERR,
                             "MCS Init Cal Validation: %s.\n", (err == API_CMS_ERROR_OK) ? "Passed" : "Failed!");
    printf("MCS Init Cal Validation: %s.\n", (err == API_CMS_ERROR_OK) ? "Passed" : "Failed!");

    return err;
}


int32_t adi_ads10_apollo_ex_mcs_tracking_cal_validate(adi_apollo_device_t *device,
                                                      adi_apollo_mcs_cal_status_t *cal_status,
                                                      uint8_t hw_bleed_pol,
                                                      int8_t hw_current_coarse,
                                                      int16_t hw_current_fine)
{
    int32_t err = 0;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(cal_status);

    // Tracking Cal Bleed Current readback
    int16_t fw_current_fine = cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].current_fine_value;
    int8_t fw_current_coarse = cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].current_coarse_value;
    uint8_t fw_bleed_pol = cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].bleed_pol;

    if (hw_bleed_pol != fw_bleed_pol) {
        printf("ADF4382 Bleed Pol: %d and Cal Status value: %d do not match!\n", hw_bleed_pol, fw_bleed_pol);
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "ADF4382 Bleed Pol: %d and Cal Status value: %d do not match!\n", hw_bleed_pol, fw_bleed_pol);
        err = API_CMS_ERROR_MCS_TRACK_CAL_ERROR;
        goto end;
    }

    if (hw_current_coarse != fw_current_coarse) {
        printf("ADF4382 Coarse Current: %d and Cal Status value: %d do not match!\n", hw_current_coarse, fw_current_coarse);
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "ADF4382 Coarse Current: %d and Cal Status value: %d do not match!\n", hw_current_coarse, fw_current_coarse);
        err = API_CMS_ERROR_MCS_TRACK_CAL_ERROR;
        goto end;
    }

    if (hw_current_fine != fw_current_fine) {
        printf("ADF4382 Fine Current: %d and Cal Status value: %d do not match!\n", hw_current_fine, fw_current_fine);
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "ADF4382 Fine Current: %d and Cal Status value: %d do not match!\n", hw_current_fine, fw_current_fine);
        err = API_CMS_ERROR_MCS_TRACK_CAL_ERROR;
        goto end;
    }

end:
    adi_apollo_hal_log_write(device, (err == API_CMS_ERROR_OK) ? ADI_CMS_LOG_API : ADI_CMS_LOG_ERR,
                            "MCS Tracking Cal Validation: %s.\n", (err == API_CMS_ERROR_OK) ? "Passed" : "Failed!");
    printf("MCS Tracking Cal Validation: %s.\n", (err == API_CMS_ERROR_OK) ? "Passed" : "Failed!");

    return err;
}


int32_t adi_ads10_apollo_ex_mcs_init_cal_status_print(adi_apollo_mcs_cal_init_status_t *cal_status)
{
    ADI_CMS_NULL_PTR_CHECK(cal_status);

    printf("\n\n");
    printf("errorCode: %d.\n"        , cal_status->hdr.errorCode);
    printf("percentComplete: %d.\n"  , cal_status->hdr.percentComplete);
    printf("performanceMetric: %d.\n", cal_status->hdr.performanceMetric);
    printf("iterCount: %d.\n"        , cal_status->hdr.iterCount);
    printf("updateCount: %d.\n\n"    , cal_status->hdr.updateCount);

    printf("mcsErr: %d.\n\n", cal_status->mcsErr);

    printf("is_C_Locked: %d.\n"  , cal_status->data.is_C_Locked);
    printf("is_A_Locked: %d.\n"  , cal_status->data.is_A_Locked);
    printf("is_B_Locked: %d.\n\n", cal_status->data.is_B_Locked);

    printf("diff_C_Before_femtoseconds: %lld.\n"       , cal_status->data.diff_C_Before_femtoseconds);
    printf("diff_A_Before_femtoseconds: %lld.\n"       , cal_status->data.diff_A_Before_femtoseconds);
    printf("diff_B_Before_femtoseconds: %lld.\n"       , cal_status->data.diff_B_Before_femtoseconds);
    printf("internal_period_C_femtoseconds: %lld.\n"   , cal_status->data.internal_period_C_femtoseconds);
    printf("internal_period_A_femtoseconds: %lld.\n"   , cal_status->data.internal_period_A_femtoseconds);
    printf("internal_period_B_femtoseconds: %lld.\n"   , cal_status->data.internal_period_B_femtoseconds);
    printf("diff_C_After_femtoseconds: %lld.\n"        , cal_status->data.diff_C_After_femtoseconds);
    printf("diff_A_After_femtoseconds: %lld.\n"        , cal_status->data.diff_A_After_femtoseconds);
    printf("diff_B_After_femtoseconds: %lld.\n"        , cal_status->data.diff_B_After_femtoseconds);
    printf("recommended_offset_C_femtoseconds: %lld.\n", cal_status->data.recommended_offset_C_femtoseconds);
    printf("recommended_offset_A_femtoseconds: %lld.\n", cal_status->data.recommended_offset_A_femtoseconds);
    printf("recommended_offset_B_femtoseconds: %lld.\n", cal_status->data.recommended_offset_B_femtoseconds);

    printf("\n\n");

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_track_cal_status_print(adi_apollo_mcs_cal_status_t *cal_status, uint8_t print_full_state)
{
    ADI_CMS_NULL_PTR_CHECK(cal_status);
    ADI_CMS_RANGE_CHECK(print_full_state, 0, 1);

    if (print_full_state == 1) {
        printf("\n\n");
        printf("errorCode: %d.\n"        , cal_status->hdr.errorCode);
        printf("percentComplete: %d.\n"  , cal_status->hdr.percentComplete);
        printf("performanceMetric: %d.\n", cal_status->hdr.performanceMetric);
        printf("iterCount: %d.\n"        , cal_status->hdr.iterCount);
        printf("updateCount: %d.\n\n"    , cal_status->hdr.updateCount);

        printf("foreground_done: %d.\n"         , cal_status->mcs_tracking_cal_status.foreground_done);
        printf("track_state[0]: %d.\n"          , cal_status->mcs_tracking_cal_status.track_state[0]);
        printf("track_state[1]: %d.\n"          , cal_status->mcs_tracking_cal_status.track_state[1]);
        printf("track_lock[0]: %d.\n"           , cal_status->mcs_tracking_cal_status.track_lock[0]);
        printf("track_lock[1]: %d.\n"           , cal_status->mcs_tracking_cal_status.track_lock[1]);
        printf("halt_active: %d.\n\n"           , cal_status->mcs_tracking_cal_status.halt_active);
        printf("force_background_done[0]: %d.\n", cal_status->mcs_tracking_cal_status.force_background_done[0]);
        printf("force_background_done[1]: %d.\n", cal_status->mcs_tracking_cal_status.force_background_done[1]);
        printf("abort_done: %d.\n\n"            , cal_status->mcs_tracking_cal_status.abort_done);

        printf("adf4382_specific_status[0].bleed_pol: %d.\n"           , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].bleed_pol);
        printf("adf4382_specific_status[0].current_coarse_value: %d.\n", cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].current_coarse_value);
        printf("adf4382_specific_status[0].current_fine_value: %d.\n"  , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].current_fine_value);
        printf("adf4382_specific_status[0].EOR_POS: %d.\n"             , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].EOR_POS);
        printf("adf4382_specific_status[0].EOR_NEG: %d.\n"             , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].EOR_NEG);
        printf("adf4382_specific_status[0].EOR_Coarse: %d.\n"          , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].EOR_Coarse);

        printf("adf4382_specific_status[1].bleed_pol: %d.\n"           , cal_status->mcs_tracking_cal_status.adf4382_specific_status[1].bleed_pol);
        printf("adf4382_specific_status[1].current_coarse_value: %d.\n", cal_status->mcs_tracking_cal_status.adf4382_specific_status[1].current_coarse_value);
        printf("adf4382_specific_status[1].current_fine_value: %d.\n"  , cal_status->mcs_tracking_cal_status.adf4382_specific_status[1].current_fine_value);
        printf("adf4382_specific_status[1].EOR_POS: %d.\n"             , cal_status->mcs_tracking_cal_status.adf4382_specific_status[1].EOR_POS);
        printf("adf4382_specific_status[1].EOR_NEG: %d.\n"             , cal_status->mcs_tracking_cal_status.adf4382_specific_status[1].EOR_NEG);
        printf("adf4382_specific_status[1].EOR_Coarse: %d.\n"          , cal_status->mcs_tracking_cal_status.adf4382_specific_status[1].EOR_Coarse);

        printf("current_measure[0]: %lld.\n", cal_status->mcs_tracking_cal_status.current_measure[0]);
        printf("current_measure[1]: %lld.\n", cal_status->mcs_tracking_cal_status.current_measure[1]);

        printf("\n\n");
    } else {
        printf("Tracking Cal[0]: --> \t");
        printf("bleed_pol: %d. \t"           , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].bleed_pol);
        printf("current_coarse: %d. \t"      , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].current_coarse_value);
        printf("current_fine: %d. \t"        , cal_status->mcs_tracking_cal_status.adf4382_specific_status[0].current_fine_value);
        printf("current_measure: %lld. \n"   , cal_status->mcs_tracking_cal_status.current_measure[0]);
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_measurement(adi_apollo_device_t *device,
                                                                      adi_adf4030_device_t *adf4030,
                                                                      mcs_tof_config_t *mcs_tof,
                                                                      int64_t path_delay[],
                                                                      uint32_t arr_len)
{
    int32_t err;
    // Arr[CENTER_SYSREF_ARR_IDX, A_SIDE_SYSREF_ARR_IDX, B_SIDE_SYSREF_ARR_IDX]
    int64_t apollo_delta_t0[NUM_APOLLO_SYSREF_CLK] = {0};
    int64_t apollo_delta_t1[NUM_APOLLO_SYSREF_CLK] = {0};
    int64_t adf4030_delta_t0[NUM_APOLLO_SYSREF_CLK] = {0};
    int64_t adf4030_delta_t1[NUM_APOLLO_SYSREF_CLK] = {0};
    int64_t calc_delay = 0;
    int64_t bsync_out_period_fs = 0;
    int64_t round_trip_delay = 0;
    int64_t result = 0;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(path_delay);

    /*> Step: 01) Configure Apollo FW for delta_t0 measurement. */
    printf("Running ADF4030-Apollo Round-Trip delay measurements...\n\n");
    err = _apollo_delta_t0_measurement(device, adf4030, mcs_tof, apollo_delta_t0, NUM_APOLLO_SYSREF_CLK);
    ADI_CMS_ERROR_RETURN(err);

    /*> Step: 02) Configure ADF4030 for adf4030_delta_t0 measurement. */
    err = _apollo_adf4030_tdc_measurement(adf4030,
                                          mcs_tof,
                                          adf4030_delta_t0,
                                          NUM_APOLLO_SYSREF_CLK);
    ADI_CMS_ERROR_RETURN(err);

    /*> Step: 03) Configure Apollo FW for delta_t1 measurement. */
    err = _apollo_delta_t1_measurement(device, adf4030, mcs_tof, apollo_delta_t1, NUM_APOLLO_SYSREF_CLK);
    ADI_CMS_ERROR_RETURN(err);

    /*> Step: 04) Configure ADF4030 for adf4030_delta_t1 measurement. */
    err = _apollo_adf4030_tdc_measurement(adf4030,
                                          mcs_tof,
                                          adf4030_delta_t1,
                                          NUM_APOLLO_SYSREF_CLK);
    ADI_CMS_ERROR_RETURN(err);

    /*> Step: 05) Calculate round trip and then path delay. */
    printf("Calculate Path delay:\n");
    if (mcs_tof->center.sysref_present) {
        bsync_out_period_fs = (int64_t) (1e15 / mcs_tof->center.bsync_out_sysref_freq_hz);
        calc_delay = (int64_t) ((adf4030_delta_t0[CENTER_SYSREF_ARR_IDX] - adf4030_delta_t1[CENTER_SYSREF_ARR_IDX])
                                 - (apollo_delta_t1[CENTER_SYSREF_ARR_IDX] - apollo_delta_t0[CENTER_SYSREF_ARR_IDX]));
        round_trip_delay = (int64_t) ((calc_delay + bsync_out_period_fs) % bsync_out_period_fs);

        path_delay[CENTER_SYSREF_ARR_IDX] = (int64_t) (round_trip_delay / 2);
        path_delay[A_SIDE_SYSREF_ARR_IDX] = 0;
        path_delay[B_SIDE_SYSREF_ARR_IDX] = 0;

        result = path_delay[CENTER_SYSREF_ARR_IDX];
        printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "Center SYSREF Path delay", result, (result / 1e3), (result / 1e6));
    } else {
        if (mcs_tof->aside.sysref_present) {
            bsync_out_period_fs = (int64_t) (1e15 / mcs_tof->aside.bsync_out_sysref_freq_hz);
            calc_delay = (int64_t) ((adf4030_delta_t0[A_SIDE_SYSREF_ARR_IDX] - adf4030_delta_t1[A_SIDE_SYSREF_ARR_IDX])
                                    - (apollo_delta_t1[A_SIDE_SYSREF_ARR_IDX] - apollo_delta_t0[A_SIDE_SYSREF_ARR_IDX]));
            round_trip_delay = (int64_t) ((calc_delay + bsync_out_period_fs) % bsync_out_period_fs);

            path_delay[A_SIDE_SYSREF_ARR_IDX] = (int64_t) (round_trip_delay / 2);

            result = path_delay[A_SIDE_SYSREF_ARR_IDX];
            printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "A-side SYSREF Path delay", result, (result / 1e3), (result / 1e6));
        }
        if (mcs_tof->bside.sysref_present) {
            bsync_out_period_fs = (int64_t) (1e15 / mcs_tof->bside.bsync_out_sysref_freq_hz);
            calc_delay = (int64_t) ((adf4030_delta_t0[B_SIDE_SYSREF_ARR_IDX] - adf4030_delta_t1[B_SIDE_SYSREF_ARR_IDX])
                                    - (apollo_delta_t1[B_SIDE_SYSREF_ARR_IDX] - apollo_delta_t0[B_SIDE_SYSREF_ARR_IDX]));
            round_trip_delay = (int64_t) ((calc_delay + bsync_out_period_fs) % bsync_out_period_fs);

            path_delay[B_SIDE_SYSREF_ARR_IDX] = (int64_t) (round_trip_delay / 2);

            result = path_delay[B_SIDE_SYSREF_ARR_IDX];
            printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "B-side SYSREF Path delay", result, (result / 1e3), (result / 1e6));
        }
        path_delay[CENTER_SYSREF_ARR_IDX] = 0;
    }

    /*> Step: 06)  Restore BSYNC 5 Channel Setup post round trip measurement. */
    err = _restore_pre_apollo_path_delay_measurement_setup(device, adf4030, mcs_tof);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_offset(adi_adf4030_device_t *adf4030,
                                                                 mcs_tof_config_t *mcs_tof,
                                                                 uint8_t align_cycles_iters,
                                                                 int64_t path_delay[],
                                                                 uint32_t arr_len)
{
    int32_t err = 0;
    int64_t adf4030_delta[NUM_APOLLO_SYSREF_CLK] = {0};
    int64_t tdc_result = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(path_delay);

    err = adi_adf4030_auto_align_iteration_set(adf4030, align_cycles_iters);
    ADI_CMS_ERROR_RETURN(err);

    if (mcs_tof->center.sysref_present) {
        err = adi_adf4030_auto_align_delay_set(adf4030,
                                               mcs_tof->center.bsync_out_sysref_ch,
                                               mcs_tof->adf4030_ref_freq_hz,
                                               path_delay[CENTER_SYSREF_ARR_IDX] * -1);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_adf4030_auto_align_single_channel_set(adf4030,
                                                        mcs_tof->bsync_in_ref_ch,
                                                        mcs_tof->center.bsync_out_sysref_ch);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        if (mcs_tof->aside.sysref_present) {
            err = adi_adf4030_auto_align_delay_set(adf4030,
                                                   mcs_tof->aside.bsync_out_sysref_ch,
                                                   mcs_tof->adf4030_ref_freq_hz,
                                                   path_delay[A_SIDE_SYSREF_ARR_IDX] * -1);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_adf4030_auto_align_single_channel_set(adf4030,
                                                            mcs_tof->bsync_in_ref_ch,
                                                            mcs_tof->aside.bsync_out_sysref_ch);
            ADI_CMS_ERROR_RETURN(err);
        }
        if (mcs_tof->bside.sysref_present) {
            err = adi_adf4030_auto_align_delay_set(adf4030,
                                                   mcs_tof->bside.bsync_out_sysref_ch,
                                                   mcs_tof->adf4030_ref_freq_hz,
                                                   path_delay[B_SIDE_SYSREF_ARR_IDX] * -1);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_adf4030_auto_align_single_channel_set(adf4030,
                                                            mcs_tof->bsync_in_ref_ch,
                                                            mcs_tof->bside.bsync_out_sysref_ch);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    printf("\nAfter adding -ve offset delay!\n");
    err = _apollo_adf4030_tdc_measurement(adf4030,
                                          mcs_tof,
                                          adf4030_delta,
                                          NUM_APOLLO_SYSREF_CLK);

    if (mcs_tof->center.sysref_present) {
        tdc_result = adf4030_delta[CENTER_SYSREF_ARR_IDX];
        printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "ADF4030 - Center SYSREF Offset", tdc_result, (tdc_result / 1e3), (tdc_result / 1e6));
    } else {
        if (mcs_tof->aside.sysref_present) {
            tdc_result = adf4030_delta[A_SIDE_SYSREF_ARR_IDX];
            printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "ADF4030 - A_Side SYSREF Offset", tdc_result, (tdc_result / 1e3), (tdc_result / 1e6));
        }
        if (mcs_tof->bside.sysref_present) {
            tdc_result = adf4030_delta[B_SIDE_SYSREF_ARR_IDX];
            printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "ADF4030 - B_Side SYSREF Offset", tdc_result, (tdc_result / 1e3), (tdc_result / 1e6));
        }
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_measurement(adi_adf4030_device_t *adf4030,
                                                                    adi_fpga_apollo_device_t *fpga_device,
                                                                    mcs_tof_config_t *mcs_tof,
                                                                    int64_t *path_delay)
{
    int32_t err = 0;
    int64_t bsync_out_period_fs = (int64_t) (1e15 / mcs_tof->fpga.bsync_out_sysref_freq_hz);
    uint64_t fpga_refclk_fs = (uint64_t) (1e15 / mcs_tof->fpga_ref_freq_hz);
    int32_t clock_0 = 0;
    int32_t clock_1 = 0;
    int64_t adf4030_delta_t0 = 0;
    int64_t adf4030_delta_t1 = 0;
    int64_t internal_fpga_delay = 0;
    int64_t calc_delay = 0;
    int64_t round_trip_delay = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(path_delay);


    /*> Step: 01) Configure FPGA for inwards SYSREF internal delay measurement. */
    printf("Running ADF4030-FPGA Round-Trip delay measurements...\n\n");
    err = _fpga_clock_0_measurement(fpga_device, adf4030, mcs_tof, &clock_0);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("FINAL clock_0: %d.\n", clock_0);

    /*> Step: 02) Configure ADF4030 for adf4030_delta_t0 measurement. */
    err = adi_adf4030_tdc_measure(adf4030,
                                  mcs_tof->bsync_in_ref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_freq_hz,
                                  &adf4030_delta_t0);
    ADI_CMS_ERROR_RETURN(err);

    /*> Step: 03) Configure FPGA for outwards SYSREF internal delay measurement. */
    err = _fpga_clock_1_measurement(fpga_device, adf4030, mcs_tof, &clock_1);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("FINAL clock_1: %d.\n", clock_1);

    /*> Step: 04) Configure ADF4030 for adf4030_delta_t1 measurement. */
    err = adi_adf4030_tdc_measure(adf4030,
                                  mcs_tof->bsync_in_ref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_freq_hz,
                                  &adf4030_delta_t1);
    ADI_CMS_ERROR_RETURN(err);

    /*> Step: 05) Calculate Round trip and path delay. */
    internal_fpga_delay = (int64_t) ((clock_0 - clock_1) * fpga_refclk_fs);
    calc_delay = (int64_t) ((adf4030_delta_t0 - adf4030_delta_t1) - internal_fpga_delay);
    round_trip_delay = (int64_t) ((calc_delay + bsync_out_period_fs) % bsync_out_period_fs);
    *path_delay = (int64_t) (round_trip_delay / 2);

/*> Results: */
    printf("Results:\n");
    MY_PRINTF("%-16s: %d .\n", "clock_0", clock_0);
    MY_PRINTF("%-16s: %d .\n", "clock_1", clock_1);
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "adf4030_delta_t0", adf4030_delta_t0, (adf4030_delta_t0 / 1e3), (adf4030_delta_t0 / 1e6));
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "adf4030_delta_t1", adf4030_delta_t1, (adf4030_delta_t1 / 1e3), (adf4030_delta_t1 / 1e6));
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "internal_fpga_delay", internal_fpga_delay, (internal_fpga_delay / 1e3), (internal_fpga_delay / 1e6));
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "calc_delay", calc_delay, (calc_delay / 1e3), (calc_delay / 1e6));
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "round_trip_delay", round_trip_delay, (round_trip_delay / 1e3), (round_trip_delay / 1e6));
    printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "FPGA path_delay", *path_delay, (*path_delay / 1e3), (*path_delay / 1e6));
    MY_PRINTF("\n\n");

    /*> Step: 06)  Restore BSYNC 8 Channel Setup post round trip measurement. */
    err = _restore_pre_fpga_path_delay_measurement_setup(fpga_device, adf4030, mcs_tof);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_offset(adi_adf4030_device_t *adf4030,
                                                               mcs_tof_config_t *mcs_tof,
                                                               uint8_t align_cycles_iters,
                                                               int64_t path_delay)
{
    int32_t err = 0;
    int64_t adf4030_delta_t = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);

    err = _adf4030_fpga_delay_offset(adf4030, mcs_tof, align_cycles_iters, path_delay * -1);
    ADI_CMS_ERROR_RETURN(err);

    printf("\nAfter adding -ve offset delay!\n");
    err = adi_adf4030_tdc_measure(adf4030,
                                  mcs_tof->bsync_in_ref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_freq_hz,
                                  &adf4030_delta_t);
    ADI_CMS_ERROR_RETURN(err);
    printf("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "adf4030_fpga_path_delay_offset", adf4030_delta_t, (adf4030_delta_t / 1e3), (adf4030_delta_t / 1e6));

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_sysref_gen_align_cal_setup(adi_apollo_device_t *device,
                                                           adi_fpga_apollo_device_t *fpga_device,
                                                           adi_apollo_top_t *profile)
{
    int32_t err;

    // MCS Calibration Data Variables
    uint64_t dev_clk_hz = (uint64_t) profile->clk_cfg.dev_clk_freq_kHz * 1e3;
    uint64_t adf4382_ref_freq_hz = 125e6;
    uint64_t adf4382_rfout_freq_hz = dev_clk_hz;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);

    /* NOTE: Assuming SYSREF settings for Center, A-side and B-side paths are the same, will use center for freq calculation */
    uint8_t divg_modulus = (profile->clk_cfg.clocking_mode == 0) ? 8 : 4;   // divg is global digital divider
    uint32_t sysref_prd_digclk_cycles = profile->mcs_cfg.internal_sysref_prd_digclk_cycles_center;
    double int_sysref_freq_hz = (double) (dev_clk_hz) / (divg_modulus * sysref_prd_digclk_cycles);
    uint64_t ext_sysref_freq_hz = (uint64_t) (int_sysref_freq_hz);
    printf("Sysref Freq (%f Hz) Supported: %s.\n", int_sysref_freq_hz, ((int_sysref_freq_hz == ext_sysref_freq_hz) ? "True" : "False"));

    // Currently supporting SYSREF Freq with minimum resolution of Hz.
    ADI_CMS_CHECK((int_sysref_freq_hz != ext_sysref_freq_hz), API_CMS_ERROR_NOT_SUPPORTED);

    uint8_t fpga_ref_div = (profile->jtx[0].common_link_cfg.ver == ADI_APOLLO_JESD_204C) ? 66 : 40;
    uint64_t fpga_ref_freq_hz = profile->jtx[0].common_link_cfg.lane_rate_kHz / fpga_ref_div * 1e3;

    // ADF4030 Device Struct and Config
    adi_adf4030_device_t adf4030 = {{0}};
    uint8_t adf4030_dev_id = ADF4030_0;
    uint16_t apollo_bsync_out_ch_sel = 0;
    uint16_t bsync_out_ch_sel = 0;
    uint16_t bsync_in_ch_sel = (1 << ADI_ADF4030_CHANNEL_ID_0);
    uint8_t align_cycles_iters = 5;
    const uint8_t num_apollo_sysref_path = 3;
    // Arr[Central, A-side, B-side]
    int64_t adf4030_apollo_path_delay[3] = {0};
    int64_t adf4030_fpga_path_delay = 0;
    uint16_t die_temp = 0;

    // MCS Config Struct
    mcs_tof_config_t mcs_tof = {
        .bsync_divider = divg_modulus * sysref_prd_digclk_cycles,
        .fpga_ref_freq_hz = fpga_ref_freq_hz,
        .adf4030_ref_freq_hz = (uint64_t) (fpga_ref_freq_hz < ADI_ADF4030_REF_FREQ_MAX) ? fpga_ref_freq_hz : (fpga_ref_freq_hz / 2),    // Generated by HMC7044. To-Do: HMC7044 to generate fixed ref clk.
        .vco_out_freq_hz = 0,
        .bsync_in_ref_ch = ADI_ADF4030_CHANNEL_ID_0,
        .center = {
            .sysref_present = profile->mcs_cfg.center_sysref.sysref_present,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_5,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        },
        .aside = {
            .sysref_present = profile->mcs_cfg.aside_sysref.sysref_present,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_6,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        },
        .bside = {
            .sysref_present = profile->mcs_cfg.bside_sysref.sysref_present,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_7,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        },
        .fpga = {
            .sysref_present = true,
            .bsync_out_sysref_ch = ADI_ADF4030_CHANNEL_ID_8,
            .bsync_out_sysref_freq_hz = ext_sysref_freq_hz
        }
    };

    /* Determine Apollo SYSREF BSYNC Source based on profile clk config */
    err = adi_ads10_apollo_ex_adf4030_apollo_bsync_out_ch_get(profile, &apollo_bsync_out_ch_sel);
    ADI_CMS_ERROR_RETURN(err);

    // CH_1: Observe SYSREF, CH_8: FPGA SYSREF, Profile based Apollo SYSREF Channel
    bsync_out_ch_sel = ((1 << ADI_ADF4030_CHANNEL_ID_1) | (1 << ADI_ADF4030_CHANNEL_ID_8) | apollo_bsync_out_ch_sel);

    err = adi_ads10_apollo_ex_adf4030_vco_freq_calc(mcs_tof.adf4030_ref_freq_hz, mcs_tof.center.bsync_out_sysref_freq_hz, &mcs_tof.vco_out_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // MCS Init Cal
    adi_apollo_mcs_cal_init_status_t init_cal_status = {{0}};

    // MCS Tracking cal
    uint16_t mcs_tracking_decimation = 1023;

    // ADF4382 Device Struct
    adi_adf4382_device_t adf4382 = {{0}};

    printf("\n");
    printf("dev_clk_hz: %lld.\t", dev_clk_hz);
    printf("divg_modulus: %d.\t sysref_prd_digclk_cycles: %d.\n", divg_modulus, sysref_prd_digclk_cycles);
    printf("int_sysref_freq_hz: %f.\n", int_sysref_freq_hz);
    printf("ext_sysref_freq_hz: %lld.\n", ext_sysref_freq_hz);
    printf("bsync_divider: %d.\n", mcs_tof.bsync_divider);
    printf("bsync_out_sysref_freq_hz: %lld.\n", mcs_tof.center.bsync_out_sysref_freq_hz);
    printf("adf4030_ref_freq_hz: %lld.\n", mcs_tof.adf4030_ref_freq_hz);
    printf("vco_out_freq_hz: %lld.\n\n", mcs_tof.vco_out_freq_hz);

/*  Step: 01. Configure device struct and HAL settings for ADF4382 and ADF4030.
              Perform ADF4030 startup and configure BSYNC input/output settings. */

    // Configure ADF4382 HAL
    err = adi_ads10_apollo_ex_adf4382_hal_config(&adf4382,
                                                 NULL,
                                                 &ads10_fpga_fmcb_aux_gpio_write);
    ADI_CMS_ERROR_RETURN(err);

    // Configure ADF4030 HAL.
    err = adi_ads10_apollo_ex_adf4030_configure_hal(&adf4030, adf4030_dev_id);
    ADI_CMS_ERROR_RETURN(err);

    // Perform ADF4030 power up and initialization.
    err = adi_ads10_apollo_ex_adf4030_startup(&adf4030, mcs_tof.adf4030_ref_freq_hz, mcs_tof.vco_out_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // BSYNC setup for sysref generation.
    err = adi_ads10_apollo_ex_adf4030_bsync_input_set(&adf4030, bsync_in_ch_sel);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_adf4030_bsync_output_set(&adf4030,
                                                       bsync_out_ch_sel,
                                                       mcs_tof.vco_out_freq_hz,
                                                       mcs_tof.center.bsync_out_sysref_freq_hz, 1);
    ADI_CMS_ERROR_RETURN(err);

    // SYNC all BSYNC_OUT channels with BSYNC_IN Ref channel.
    printf("Align all BSYNC_OUT Channels with ref BSYNC_IN Channel...\n");
    err = adi_ads10_apollo_ex_adf4030_align_bsync_out(&adf4030,
                                                      bsync_in_ch_sel,
                                                      bsync_out_ch_sel,
                                                      mcs_tof.center.bsync_out_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

    // Measure ADF4030 DIE Temperature.
    err = adi_adf4030_core_die_temp_get(&adf4030, &die_temp);
    ADI_CMS_ERROR_RETURN(err);

    printf("ADF4030 Die Temp: %d *C.\n", die_temp);


/*  Step: 02. Program MCS Init Cal, including most of settings for ADF4382 Tracking.
              And configure ADF4382 for MCS init cal. */

    err = adi_ads10_apollo_ex_mcs_init_cal_setup(device,
                                                 &adf4382,
                                                 adf4382_ref_freq_hz,
                                                 adf4382_rfout_freq_hz,
                                                 ext_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 03. ADF4030 - Apollo Time-of-Flight Measurement and offset. */

    // Measure sysref path delay.
    err = adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_measurement(device,
                                                                        &adf4030,
                                                                        &mcs_tof,
                                                                        adf4030_apollo_path_delay,
                                                                        num_apollo_sysref_path);
    ADI_CMS_ERROR_RETURN(err);

    // Offset path delay from BSYNC output.
    err = adi_ads10_apollo_ex_mcs_adf4030_apollo_path_delay_offset(&adf4030,
                                                                   &mcs_tof,
                                                                   align_cycles_iters,
                                                                   adf4030_apollo_path_delay,
                                                                   num_apollo_sysref_path);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 04. ADF4030 - FPGA Time-of-Flight Measurement and offset. */

    err = adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_measurement(&adf4030,
                                                                      fpga_device,
                                                                      &mcs_tof,
                                                                      &adf4030_fpga_path_delay);
    ADI_CMS_ERROR_RETURN(err);

    // Offset path delay from BSYNC output.
    err = adi_ads10_apollo_ex_mcs_adf4030_fpga_path_delay_offset(&adf4030,
                                                                 &mcs_tof,
                                                                 align_cycles_iters,
                                                                 adf4030_fpga_path_delay);
    ADI_CMS_ERROR_RETURN(err);


/*  Step: 05. SYNC all BSYNC_OUT channels with BSYNC_IN Ref channel. */

    printf("Align all BSYNC_OUT Channels with ref BSYNC_IN Channel...\n");
    err = adi_ads10_apollo_ex_adf4030_align_bsync_out(&adf4030,
                                                      bsync_in_ch_sel,
                                                      bsync_out_ch_sel,
                                                      mcs_tof.center.bsync_out_sysref_freq_hz);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 06. Perform MCS Init Cal for Sysref Alignment.
              Read back MCS init cal status data and validate init cal success. */

    printf("Running MCS Init Calibration...\n");
    err = adi_apollo_mcs_cal_init_run(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_mcs_cal_init_status_get(device, &init_cal_status);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_mcs_init_cal_validate(device, profile, &init_cal_status);
    ADI_CMS_ERROR_RETURN(err);

/*  Step: 07. Optional. Perform MCS Tracking Cal for maintaining alignment.
              MCS Tracking Calibration restricted to Single Clock scheme due to HW limitation. */
    if (profile->clk_cfg.single_dual_clk_sel == false) {
        err = adi_ads10_apollo_ex_mcs_tracking_cal_setup(device, mcs_tracking_decimation, 1);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4382_phase_adjust_auto_align_enable(&adf4382, 1);
        ADI_CMS_ERROR_RETURN(err);

        /* Execute MCS Foreground Tracking cal for faster SysRef Alignment.
           Performs TDC measurements and multi phase correction strobes for clock calibration. */
        printf("Running MCS FG Tracking Calibration...\n");
        err = adi_apollo_mcs_cal_fg_tracking_run(device);
        ADI_CMS_ERROR_RETURN(err);

        /* Trigger MCS Background Tracking cal for maintaining alignment.
           Executes in background periodically, taking TDC measurements and phase correction if required. */
        printf("Running MCS BG Tracking Calibration...\n");
        err = adi_apollo_mcs_cal_bg_tracking_run(device);
        ADI_CMS_ERROR_RETURN(err);
    }


    // Measure ADF4030 DIE Temperature.
    err = adi_adf4030_core_die_temp_get(&adf4030, &die_temp);
    ADI_CMS_ERROR_RETURN(err);

    printf("ADF4030 Die Temp: %d *C.\n", die_temp);

    // err = adi_ads10_apollo_ex_mcs_sysref_info_print(device);
    // ADI_CMS_ERROR_RETURN(err);
    // err = adi_ads10_apollo_ex_mcs_sysref_phase_print(device);
    // ADI_CMS_ERROR_RETURN(err);
    // err = adi_ads10_apollo_ex_mcs_link_phase_print(device);
    // ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_sysref_info_print(adi_apollo_device_t *device)
{
    int32_t err = 0;
    uint8_t num = 0;
    uint8_t clk_scheme = 0;
    uint8_t sysref_en = 0;
    uint8_t sysref_lvl = 0;
    uint8_t int_sysref_period[2];
    // Arr_inst[Central, A-side, B-side]
    uint32_t mcs_tdc_inst[3] = {MCS_TDC_MCSTOP0, MCS_TDC_MCSTOP1, MCS_TDC_MCSTOP2};
    uint32_t mcs_sync_inst[3] = {MCS_SYNC_MCSTOP0, MCS_SYNC_MCSTOP1, MCS_SYNC_MCSTOP2};

    ADI_CMS_NULL_PTR_CHECK(device);

    for (clk_scheme = 0; clk_scheme < 3; clk_scheme++) {
        printf("%s Clock.\n",
               (clk_scheme == 0) ? "Center" :
               ((clk_scheme == 1) ? "A-side" : "B-side"));

        printf("BF_SYSREF_LEVEL_INFO(MCS_TDC_MCSTOP%d):", clk_scheme);
        for (num = 0; num < 32; num++) {
            err = adi_apollo_hal_bf_get(device, BF_SYSREF_LEVEL_INFO(mcs_tdc_inst[clk_scheme]), &sysref_lvl, 1);
            ADI_CMS_ERROR_RETURN(err);

            if ((num % 16) == 0) {
                printf("\n");
            }
            printf("%d  ", sysref_lvl);
            sysref_lvl = 0;
        }
        printf("\n");
        err = adi_apollo_hal_bf_get(device, BF_SYSREF_EN_INFO(mcs_tdc_inst[clk_scheme]), &sysref_en, 1);
        ADI_CMS_ERROR_RETURN(err);
        printf("BF_SYSREF_EN_INFO(MCS_TDC_MCSTOP%d): %d.\n", clk_scheme, sysref_en);

        err = adi_apollo_hal_bf_get(device, BF_INT_SYSREF_PERIOD_INFO(mcs_sync_inst[clk_scheme]), int_sysref_period, 2);
        ADI_CMS_ERROR_RETURN(err);
        printf("BF_INT_SYSREF_PERIOD_INFO(MCS_SYNC_MCSTOP%d): %d.\n", clk_scheme, (int_sysref_period[1] << 8) | int_sysref_period[0]);
        printf("\n");
    }
    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_sysref_phase_print(adi_apollo_device_t *device)
{
    int32_t err = 0;
    uint32_t sysref_phase = 0;
    ADI_CMS_NULL_PTR_CHECK(device);

    err = adi_apollo_clk_mcs_sysref_phase_get(device, &sysref_phase);
    ADI_CMS_ERROR_RETURN(err);
    printf("Apollo sysref_phase = 0x%x  %d\n", sysref_phase, sysref_phase);

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_mcs_link_phase_print(adi_apollo_device_t *device)
{
    int32_t err = 0;
    uint16_t jrx_phase_diff0, jrx_phase_diff1 = 0;
    uint16_t jrx_phase_adjust0, jrx_phase_adjust1 = 0;
    ADI_CMS_NULL_PTR_CHECK(device);

    err = adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_diff0);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_jrx_phase_diff_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_diff1);
    ADI_CMS_ERROR_RETURN(err);
    printf("jrx_phase_diff0 / jrx_phase_diff1 = %d %d\n", jrx_phase_diff0, jrx_phase_diff1);

    err = adi_apollo_jrx_phase_adjust_get(device, ADI_APOLLO_LINK_A0, &jrx_phase_adjust0);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_jrx_phase_adjust_get(device, ADI_APOLLO_LINK_B0, &jrx_phase_adjust1);
    ADI_CMS_ERROR_RETURN(err);
    printf("jrx_phase_adjust0 / jrx_phase_adjust1 = %d %d\n", jrx_phase_adjust0, jrx_phase_adjust1);

    return API_CMS_ERROR_OK;
}




static int32_t calculate_phase_value_fs(adi_adf4382_device_t *adf4382, uint64_t adf4382_ref_freq_hz, uint8_t phase_adjustment)
{
    int32_t err = API_CMS_ERROR_OK;
    uint64_t pfd_freq = 0;
    int32_t phase_value_fs; // femto seconds
    uint8_t r_div = 0;
    uint8_t rdblr = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(adf4382_ref_freq_hz, ADI_ADF4382_REF_MIN, ADI_ADF4382_REF_MAX);

    err = adi_adf4382_bf___REG0020___R_DIV_get(adf4382, &r_div);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4382_bf___REG0020___EN_RDBLR_get(adf4382, &rdblr);
    ADI_CMS_ERROR_RETURN(err);

    rdblr = rdblr > 0 ? 2 : 1;
    pfd_freq = (adf4382_ref_freq_hz / r_div) * rdblr;

    phase_value_fs = (((phase_adjustment * 250) / (10.08 * pfd_freq * 511 * 1e3))) / 1e-15;

    return phase_value_fs;
}


static int32_t _apollo_delta_t0_measurement(adi_apollo_device_t *device,
                                            adi_adf4030_device_t *adf4030,
                                            mcs_tof_config_t *mcs_tof,
                                            int64_t apollo_delta_t0[],
                                            uint32_t arr_len)
{
    int32_t err;
    adi_apollo_mailbox_cmd_mcs_bsync_set_config_t bsync_set_config_cmd = {0};
    adi_apollo_mailbox_resp_mcs_bsync_set_config_t bsync_set_config_resp = {0};
    adi_apollo_mailbox_resp_mcs_bsync_go_t bsync_go_resp = {0};
    adi_apollo_mailbox_resp_mcs_bsync_get_config_t bsync_get_config_resp = {0};

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(apollo_delta_t0);

    bsync_set_config_cmd.func_mode = APOLLO_MCS_BSYNC_ALIGN;
    bsync_set_config_cmd.bsync_div = mcs_tof->bsync_divider;

    err = adi_apollo_mailbox_mcs_bsync_set_config(device, &bsync_set_config_cmd, &bsync_set_config_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_set_config_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);
    MY_PRINTF("bsync_set_config_resp.status: %d.\n", bsync_set_config_resp.status);

    err = adi_apollo_mailbox_mcs_bsync_go(device, &bsync_go_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_go_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);
    MY_PRINTF("bsync_go_resp.status: %d.\n", bsync_go_resp.status);

    err = adi_apollo_mailbox_mcs_bsync_get_config(device, &bsync_get_config_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_get_config_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);

    apollo_delta_t0[CENTER_SYSREF_ARR_IDX] = bsync_get_config_resp.delta_t0;
    apollo_delta_t0[A_SIDE_SYSREF_ARR_IDX] = bsync_get_config_resp.delta_t0_a;
    apollo_delta_t0[B_SIDE_SYSREF_ARR_IDX] = bsync_get_config_resp.delta_t0_b;

    MY_PRINTF("%s(): bsync_get_config_resp:\n", __func__);
    MY_PRINTF("\t status: %d.\n", bsync_get_config_resp.status);
    MY_PRINTF("\t func_mode: %d.\n", bsync_get_config_resp.func_mode);
    MY_PRINTF("\t bsync_div: %d.\n", bsync_get_config_resp.bsync_div);
    MY_PRINTF("\t done_flag: %d.\n", bsync_get_config_resp.done_flag);
    MY_PRINTF("\t delta_t0: %lld.\n", bsync_get_config_resp.delta_t0);
    MY_PRINTF("\t delta_t1: %lld.\n", bsync_get_config_resp.delta_t1);
    MY_PRINTF("\t delta_t0_a: %lld.\n", bsync_get_config_resp.delta_t0_a);
    MY_PRINTF("\t delta_t1_a: %lld.\n", bsync_get_config_resp.delta_t1_a);
    MY_PRINTF("\t delta_t0_b: %lld.\n", bsync_get_config_resp.delta_t0_b);
    MY_PRINTF("\t delta_t1_b: %lld.\n\n", bsync_get_config_resp.delta_t1_b);

    return API_CMS_ERROR_OK;
}


static int32_t _apollo_delta_t1_measurement(adi_apollo_device_t *device,
                                            adi_adf4030_device_t *adf4030,
                                            mcs_tof_config_t *mcs_tof,
                                            int64_t apollo_delta_t1[],
                                            uint32_t arr_len)
{
    int32_t err;
    adi_apollo_mailbox_cmd_mcs_bsync_set_config_t bsync_set_config_cmd = {0};
    adi_apollo_mailbox_resp_mcs_bsync_set_config_t bsync_set_config_resp = {0};
    adi_apollo_mailbox_resp_mcs_bsync_go_t bsync_go_resp = {0};
    adi_apollo_mailbox_resp_mcs_bsync_get_config_t bsync_get_config_resp = {0};

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(apollo_delta_t1);

    // Set BSYNC_CH as Input
    if (mcs_tof->center.sysref_present) {
        MY_PRINTF("Setting BSYNC_CH: 0x%02X as Input!\n", mcs_tof->center.bsync_out_sysref_ch);
        err = adi_ads10_apollo_ex_adf4030_bsync_input_set(adf4030, (1 << mcs_tof->center.bsync_out_sysref_ch));
        ADI_CMS_ERROR_RETURN(err);
    } else {
        if (mcs_tof->aside.sysref_present) {
            MY_PRINTF("Setting BSYNC_CH: 0x%02X as Input!\n", mcs_tof->aside.bsync_out_sysref_ch);
            err = adi_ads10_apollo_ex_adf4030_bsync_input_set(adf4030, (1 << mcs_tof->aside.bsync_out_sysref_ch));
            ADI_CMS_ERROR_RETURN(err);
        }
        if (mcs_tof->bside.sysref_present) {
            MY_PRINTF("Setting BSYNC_CH: 0x%02X as Input!\n", mcs_tof->bside.bsync_out_sysref_ch);
            err = adi_ads10_apollo_ex_adf4030_bsync_input_set(adf4030, (1 << mcs_tof->bside.bsync_out_sysref_ch));
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    bsync_set_config_cmd.func_mode = APOLLO_MCS_BSYNC_OUTPUT_EN;
    bsync_set_config_cmd.bsync_div = mcs_tof->bsync_divider;

    err = adi_apollo_mailbox_mcs_bsync_set_config(device, &bsync_set_config_cmd, &bsync_set_config_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_set_config_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);
    MY_PRINTF("bsync_set_config_resp.status: %d.\n", bsync_set_config_resp.status);

    err = adi_apollo_mailbox_mcs_bsync_go(device, &bsync_go_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_go_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);
    MY_PRINTF("bsync_go_resp.status: %d.\n", bsync_go_resp.status);

    err = adi_apollo_mailbox_mcs_bsync_get_config(device, &bsync_get_config_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_get_config_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);

    apollo_delta_t1[CENTER_SYSREF_ARR_IDX] = bsync_get_config_resp.delta_t1;
    apollo_delta_t1[A_SIDE_SYSREF_ARR_IDX] = bsync_get_config_resp.delta_t1_a;
    apollo_delta_t1[B_SIDE_SYSREF_ARR_IDX] = bsync_get_config_resp.delta_t1_b;

    MY_PRINTF("%s(): bsync_get_config_resp:\n", __func__);
    MY_PRINTF("\t status: %d.\n", bsync_get_config_resp.status);
    MY_PRINTF("\t func_mode: %d.\n", bsync_get_config_resp.func_mode);
    MY_PRINTF("\t bsync_div: %d.\n", bsync_get_config_resp.bsync_div);
    MY_PRINTF("\t done_flag: %d.\n", bsync_get_config_resp.done_flag);
    MY_PRINTF("\t delta_t0: %lld.\n", bsync_get_config_resp.delta_t0);
    MY_PRINTF("\t delta_t1: %lld.\n", bsync_get_config_resp.delta_t1);
    MY_PRINTF("\t delta_t0_a: %lld.\n", bsync_get_config_resp.delta_t0_a);
    MY_PRINTF("\t delta_t1_a: %lld.\n", bsync_get_config_resp.delta_t1_a);
    MY_PRINTF("\t delta_t0_b: %lld.\n", bsync_get_config_resp.delta_t0_b);
    MY_PRINTF("\t delta_t1_b: %lld.\n\n", bsync_get_config_resp.delta_t1_b);

    return API_CMS_ERROR_OK;
}

static int32_t _apollo_adf4030_tdc_measurement(adi_adf4030_device_t *adf4030,
                                               mcs_tof_config_t *mcs_tof,
                                               int64_t adf4030_delta[],
                                               uint32_t arr_len)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(adf4030_delta);

    if (mcs_tof->center.sysref_present) {
        err = adi_adf4030_tdc_measure(adf4030,
                                      mcs_tof->bsync_in_ref_ch,
                                      mcs_tof->center.bsync_out_sysref_ch,
                                      mcs_tof->center.bsync_out_sysref_freq_hz,
                                      &adf4030_delta[CENTER_SYSREF_ARR_IDX]);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        if (mcs_tof->aside.sysref_present) {
            err = adi_adf4030_tdc_measure(adf4030,
                                          mcs_tof->bsync_in_ref_ch,
                                          mcs_tof->aside.bsync_out_sysref_ch,
                                          mcs_tof->aside.bsync_out_sysref_freq_hz,
                                          &adf4030_delta[A_SIDE_SYSREF_ARR_IDX]);
            ADI_CMS_ERROR_RETURN(err);
        }
        if (mcs_tof->bside.sysref_present) {
            err = adi_adf4030_tdc_measure(adf4030,
                                          mcs_tof->bsync_in_ref_ch,
                                          mcs_tof->bside.bsync_out_sysref_ch,
                                          mcs_tof->bside.bsync_out_sysref_freq_hz,
                                          &adf4030_delta[B_SIDE_SYSREF_ARR_IDX]);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return API_CMS_ERROR_OK;
}


static int32_t _restore_pre_apollo_path_delay_measurement_setup(adi_apollo_device_t *device,
                                                                adi_adf4030_device_t *adf4030,
                                                                mcs_tof_config_t *mcs_tof)
{
    int32_t err;
    adi_apollo_mailbox_cmd_mcs_bsync_set_config_t bsync_set_config_cmd = {0};
    adi_apollo_mailbox_resp_mcs_bsync_set_config_t bsync_set_config_resp = {0};
    adi_apollo_mailbox_resp_mcs_bsync_go_t bsync_go_resp = {0};

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);

    bsync_set_config_cmd.func_mode = APOLLO_MCS_BSYNC_OUTPUT_DIS;
    bsync_set_config_cmd.bsync_div = mcs_tof->bsync_divider;

    err = adi_apollo_mailbox_mcs_bsync_set_config(device, &bsync_set_config_cmd, &bsync_set_config_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_set_config_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);
    MY_PRINTF("bsync_set_config_resp.status: %d.\n", bsync_set_config_resp.status);

    err = adi_apollo_mailbox_mcs_bsync_go(device, &bsync_go_resp);
    ADI_CMS_ERROR_RETURN(err);
    ADI_CMS_CHECK((bsync_go_resp.status != API_CMS_ERROR_OK), API_CMS_ERROR_MAILBOX_RESP_STATUS);
    MY_PRINTF("bsync_go_resp.status: %d.\n", bsync_go_resp.status);

    // Set BSYNC_CH as Output
    if (mcs_tof->center.sysref_present) {
        MY_PRINTF("Setting BSYNC_CH: 0x%02X as Output!\n", mcs_tof->center.bsync_out_sysref_ch);
        err = adi_ads10_apollo_ex_adf4030_bsync_output_set(adf4030,
                                                           (1 << mcs_tof->center.bsync_out_sysref_ch),
                                                           mcs_tof->vco_out_freq_hz,
                                                           mcs_tof->center.bsync_out_sysref_freq_hz,
                                                           1);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        if (mcs_tof->aside.sysref_present) {
            MY_PRINTF("Setting BSYNC_CH: 0x%02X as Output!\n", mcs_tof->aside.bsync_out_sysref_ch);
            err = adi_ads10_apollo_ex_adf4030_bsync_output_set(adf4030,
                                                               (1 << mcs_tof->aside.bsync_out_sysref_ch),
                                                               mcs_tof->vco_out_freq_hz,
                                                               mcs_tof->aside.bsync_out_sysref_freq_hz,
                                                               1);
            ADI_CMS_ERROR_RETURN(err);
        }
        if (mcs_tof->bside.sysref_present) {
            MY_PRINTF("Setting BSYNC_CH: 0x%02X as Output!\n", mcs_tof->bside.bsync_out_sysref_ch);
            err = adi_ads10_apollo_ex_adf4030_bsync_output_set(adf4030,
                                                               (1 << mcs_tof->bside.bsync_out_sysref_ch),
                                                               mcs_tof->vco_out_freq_hz,
                                                               mcs_tof->bside.bsync_out_sysref_freq_hz,
                                                               1);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return API_CMS_ERROR_OK;
}


static int32_t _fpga_clock_0_measurement(adi_fpga_apollo_device_t *fpga_device,
                                         adi_adf4030_device_t *adf4030,
                                         mcs_tof_config_t *mcs_tof,
                                         int32_t *clock_0)
{
    int32_t err = 0;
    uint8_t iter = 0;
    uint32_t sysref_glblclk_ratio = (uint32_t) (mcs_tof->fpga_ref_freq_hz / mcs_tof->fpga.bsync_out_sysref_freq_hz);
    uint64_t fpga_refclk_fs = (uint64_t) (1e15 / mcs_tof->fpga_ref_freq_hz);
    uint64_t bsync8_delay_fs = 0;
    uint64_t sar_delay_fs = fpga_refclk_fs;
    int32_t clock_temp_0 = 0;
    int32_t clock_sar = 0;
    uint8_t align_cycles_iters = 5;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(clock_0);

    MY_PRINTF("sysref_glblclk_ratio: %d.\n", sysref_glblclk_ratio);
    MY_PRINTF("fpga_refclk_fs: %lld.\n", fpga_refclk_fs);
    MY_PRINTF("bsync8_delay_fs: %lld.\n\n", bsync8_delay_fs);

/*> Step: 01) Add a FPGA clk cycle delay to BSYNC8 (i.e FPGA's external SYSREF). */
    err = _adf4030_fpga_delay_offset(adf4030, mcs_tof, align_cycles_iters, bsync8_delay_fs);
    ADI_CMS_ERROR_RETURN(err);

/*> Step: 02) Configure FPGA SYSREF Setup and start Internal SYSREF Alignment with External.  */
    err = _fpga_sysref_setup(fpga_device, sysref_glblclk_ratio);
    ADI_CMS_ERROR_RETURN(err);

/*> Step: 03) Get initial clock alignment for SAR routine. */
    err = _fpga_sysref_phase_get(fpga_device, &clock_temp_0);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("clock_temp_0: %d.\n\n", clock_temp_0);

/*> Step: 04) Using SAR routine align SYSREF with next FPGA clk edge. */
    bsync8_delay_fs += sar_delay_fs;
    for (iter = 0; iter < 12; ++iter) {
        MY_PRINTF("iter: %d.\t", iter);
        MY_PRINTF("sar_delay_fs: %lld.\t", sar_delay_fs);
        MY_PRINTF("bsync8_delay_fs: %lld.\n", bsync8_delay_fs);
        err = _adf4030_fpga_delay_offset(adf4030, mcs_tof, 1, bsync8_delay_fs);
        ADI_CMS_ERROR_RETURN(err);

        err = _fpga_sysref_phase_get(fpga_device, &clock_sar);
        ADI_CMS_ERROR_RETURN(err);
        MY_PRINTF("clock_sar: %d.\t", clock_sar);

        if ((clock_temp_0 < 0) && (clock_sar > 0)) {
            clock_sar -= 32;
        }
        MY_PRINTF("clock_sar: %d.\n", clock_sar);

        if (clock_sar < clock_temp_0) {
            bsync8_delay_fs -= (sar_delay_fs / 2);
        } else {
            bsync8_delay_fs += (sar_delay_fs / 2);
        }
        sar_delay_fs /= 2;
        MY_PRINTF("\n");
    }

//  ****************  ADD INFO HERE  *****************
    bsync8_delay_fs += (fpga_refclk_fs / 2);
    MY_PRINTF("bsync8_delay_fs: %lld.\n", bsync8_delay_fs);
    err = _adf4030_fpga_delay_offset(adf4030, mcs_tof, align_cycles_iters, bsync8_delay_fs);
    ADI_CMS_ERROR_RETURN(err);

//  ****************  ADD INFO HERE  *****************
    err = _fpga_sysref_phase_get(fpga_device, &clock_temp_0);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("new clock_temp_0: %d.\n\n", clock_temp_0);

    *clock_0 = clock_temp_0;

    return API_CMS_ERROR_OK;
}


static int32_t _fpga_clock_1_measurement(adi_fpga_apollo_device_t *fpga_device,
                                         adi_adf4030_device_t *adf4030,
                                         mcs_tof_config_t *mcs_tof,
                                         int32_t *clock_1)
{
    int32_t err = 0;
    uint8_t iter = 0;
    uint64_t fpga_refclk_fs = (uint64_t) (1e15 / mcs_tof->fpga_ref_freq_hz);
    uint8_t delay_line_width = 6;
    uint8_t fpga_sar_delay = 1 << delay_line_width;
    uint8_t del_val = fpga_sar_delay - 1;
    int32_t clock_temp_1 = 0;
    int32_t clock_fpga_sar = 0;

    int64_t total_tap_delay_fs = 0;
    uint64_t delay_per_tap_fs = 0;
    uint8_t n_taps_for_half_refclk_delay = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(clock_1);

/*> Step: 06) Configure FPGA to transmit SYSREF and ADF4030 to receive. */
    // Set BSYNC_8 as Input
    err = adi_ads10_apollo_ex_adf4030_bsync_input_set(adf4030, (1 << mcs_tof->fpga.bsync_out_sysref_ch));
    ADI_CMS_ERROR_RETURN(err);

    // sysref_io_buffer_direction: SYSREF I/O Buffer set as Output.
    err = adi_fpga_apollo_core_bf_set(fpga_device, CLK_SRC_SEL, SYSREF_IO_BUFFER_DIRECTION_MASK, 0);
    ADI_CMS_ERROR_RETURN(err);

    //delay_fb_enable: enable delay feedback path
    err = adi_fpga_apollo_core_bf_set(fpga_device, OUTPUT_DELAY, DELAY_FB_ENABLE_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);

/*> Step: 07) Get clock alignment after setting sysref i/o buffer as output. */
    err = _fpga_sysref_phase_get(fpga_device, &clock_temp_1);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("clock_temp_1: %d.\n\n", clock_temp_1);

    err = _fpga_clock_out_tap_delay_measurement(fpga_device, adf4030, mcs_tof, &total_tap_delay_fs);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n\n", "total_tap_delay_fs", total_tap_delay_fs, (total_tap_delay_fs / 1e3), (total_tap_delay_fs / 1e6));

    delay_per_tap_fs = (uint64_t) (total_tap_delay_fs / del_val);
    n_taps_for_half_refclk_delay = (uint8_t) ((0.5 * fpga_refclk_fs) / delay_per_tap_fs);
    MY_PRINTF("\n");
    MY_PRINTF("%-16s: %12lld fs  %12.3f ps  %12.6f ns.\n", "delay_per_tap_fs", delay_per_tap_fs, (delay_per_tap_fs / 1e3), (delay_per_tap_fs / 1e6));
    MY_PRINTF("n_taps_for_half_refclk_delay: %d.\n", n_taps_for_half_refclk_delay);


/*> Step: 08) Align FPGA driven SYSREF to next FPGA ref clk. */
    for (iter = 0; iter < (delay_line_width + 1); ++iter) {
        MY_PRINTF("iter: %d.\t", iter);
        MY_PRINTF("del_val: %d.\t", del_val);
        MY_PRINTF("fpga_sar_delay: %d.\n", fpga_sar_delay);

        // delay_sel:
        err = adi_fpga_apollo_core_bf_set(fpga_device, OUTPUT_DELAY, DELAY_SEL_MASK, del_val);
        ADI_CMS_ERROR_RETURN(err);

        err = _fpga_sysref_phase_get(fpga_device, &clock_fpga_sar);
        ADI_CMS_ERROR_RETURN(err);
        MY_PRINTF("clock_fpga_sar: %d.\t", clock_fpga_sar);

        if ((clock_temp_1 < 0) && (clock_fpga_sar > 0)) {
            clock_fpga_sar -= 32;
        }
        MY_PRINTF("clock_fpga_sar: %d.\n", clock_fpga_sar);

        if (iter < delay_line_width) {
            if (clock_fpga_sar < clock_temp_1) {
                del_val -= (fpga_sar_delay / 2);
            } else {
                del_val += (fpga_sar_delay / 2);
            }
            fpga_sar_delay /= 2;
        }
        MY_PRINTF("\n");
    }

    //  ****************  ADD INFO HERE  *****************
// DELAY TIMING STUFF
    del_val += n_taps_for_half_refclk_delay;
    MY_PRINTF("del_val: %d.\n\n", del_val);

    // delay_sel:
    err = adi_fpga_apollo_core_bf_set(fpga_device, OUTPUT_DELAY, DELAY_SEL_MASK, del_val);
    ADI_CMS_ERROR_RETURN(err);

    err = _fpga_sysref_phase_get(fpga_device, &clock_temp_1);
    ADI_CMS_ERROR_RETURN(err);
    MY_PRINTF("new clock_temp_1: %d.\n\n", clock_temp_1);

    *clock_1 = clock_temp_1;

    return API_CMS_ERROR_OK;
}


static int32_t _fpga_clock_out_tap_delay_measurement(adi_fpga_apollo_device_t *fpga_device,
                                                     adi_adf4030_device_t *adf4030,
                                                     mcs_tof_config_t *mcs_tof,
                                                     int64_t *total_tap_delay)
{
    int32_t err = 0;
    uint8_t delay_line_width = 6;
    uint8_t fpga_sar_delay = 1 << delay_line_width;
    uint8_t del_val = fpga_sar_delay - 1;
    int64_t tap_delay_min = 0;
    int64_t tap_delay_max = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);
    ADI_CMS_NULL_PTR_CHECK(total_tap_delay);

     // delay_sel: MIN Value
    err = adi_fpga_apollo_core_bf_set(fpga_device, OUTPUT_DELAY, DELAY_SEL_MASK, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_tdc_measure(adf4030,
                                  mcs_tof->bsync_in_ref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_freq_hz,
                                  &tap_delay_min);
    ADI_CMS_ERROR_RETURN(err);

    // delay_sel: MAX Value
    err = adi_fpga_apollo_core_bf_set(fpga_device, OUTPUT_DELAY, DELAY_SEL_MASK, del_val);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_tdc_measure(adf4030,
                                  mcs_tof->bsync_in_ref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_ch,
                                  mcs_tof->fpga.bsync_out_sysref_freq_hz,
                                  &tap_delay_max);
    ADI_CMS_ERROR_RETURN(err);

    *total_tap_delay = (int64_t) abs(tap_delay_min - tap_delay_max);

    return API_CMS_ERROR_OK;
}


static int32_t _restore_pre_fpga_path_delay_measurement_setup(adi_fpga_apollo_device_t *fpga_device,
                                                              adi_adf4030_device_t *adf4030,
                                                              mcs_tof_config_t *mcs_tof)
{
    int32_t err = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);

    // Set BSYNC_CH as Output
    err = adi_ads10_apollo_ex_adf4030_bsync_output_set(adf4030,
                                                       (1 << mcs_tof->fpga.bsync_out_sysref_ch),
                                                       mcs_tof->vco_out_freq_hz,
                                                       mcs_tof->fpga.bsync_out_sysref_freq_hz,
                                                       1);
    ADI_CMS_ERROR_RETURN(err);

    // sysref_io_buffer_direction: SYSREF I/O Buffer set as Input.
    err = adi_fpga_apollo_core_bf_set(fpga_device, CLK_SRC_SEL, SYSREF_IO_BUFFER_DIRECTION_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);

    // delay_sel and delay_fb_en: Min output delay and delay feedback disabled.
    err = adi_fpga_apollo_core_reg_set(fpga_device, OUTPUT_DELAY, 0);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


static int32_t _fpga_sysref_setup(adi_fpga_apollo_device_t *fpga_device, uint32_t sysref_glblclk_ratio)
{
    int32_t err = 0;
    uint32_t mgt_refclk_sel = (1 << 0);                 // mgt_refclk_sel: USE FMC as FPGA REFCLK.
    uint32_t sysref_sel = (1 << 16);                    // sysref_sel: USE SYSREF from FMC.
    uint32_t sysref_io_buffer_direction = (1 << 18);    // sysref_io_buffer_direction: SYSREF I/O Buffer set as Input.
    // WHAT'S THE FREQ HERE ????
    uint8_t ad9528_vcxo_select = 1;                     // ad9528_vcxo_select: USE FMC CLK1 for AD9528 VCXO Input.

    ADI_CMS_NULL_PTR_CHECK(fpga_device);

    /* Setup External SYSREF source settings. */
    err = adi_fpga_apollo_core_reg_set(fpga_device, CLK_SRC_SEL, (sysref_io_buffer_direction | sysref_sel | mgt_refclk_sel));
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_bf_set(fpga_device, MISC_0, AD9528_VCXO_SELECT_MASK, ad9528_vcxo_select);
    ADI_CMS_ERROR_RETURN(err);

    /* Setup Internal SYSREF generation. */
    err = adi_fpga_apollo_core_bf_set(fpga_device, CTRL_TRANSMIT, TRANSMIT_START_MASK, 1);                  // transmit_start
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_reg_set(fpga_device, SYSREF_GLBLCLK_RATIO, sysref_glblclk_ratio);    // sysref_glblclk_ratio: Divider value for internal SYSREF generation.
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_bf_set(fpga_device, SYSREF_COUNT, SYSREF_GLBLCLK_RATIO_SET_MASK, 1);                  // sysref_glblclk_ratio_set: Load divider value to generate internal SYSREF.
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_wait_us(fpga_device, 100);
    ADI_CMS_ERROR_RETURN(err);

    /* Start Internal SYSREF Alignment with External. */
    // err = adi_fpga_apollo_core_bf_set(fpga_device, SYSREF_COUNT, SYSREF_COUNT_START_MASK, 1);            // sysref_count_start: SYSREF counter starts.
    // ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_reg_set(fpga_device, OUTPUT_DELAY, 0);                 // delay_sel and delay_fb_en: Min output delay and delay feedback disabled.
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


static int32_t _fpga_sysref_phase_get(adi_fpga_apollo_device_t *fpga_device, int32_t *clock_phase)
{
    int32_t err = 0;
    uint32_t sysref_capt_phase_signed = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(clock_phase);

    // sysref_capt_phase_signed: Phase of the captured sysref with respect to the internal sysref
    err = adi_fpga_apollo_core_bf_get(fpga_device, SYSREF_COUNT, SYSREF_CAPT_PHASE_SIGNED_MASK, &sysref_capt_phase_signed);
    ADI_CMS_ERROR_RETURN(err);

    if (sysref_capt_phase_signed > 0x7FFF) {
        *clock_phase = sysref_capt_phase_signed - 0x10000;
    } else {
        *clock_phase = sysref_capt_phase_signed;
    }

    return API_CMS_ERROR_OK;
}


static int32_t _adf4030_fpga_delay_offset(adi_adf4030_device_t *adf4030,
                                          mcs_tof_config_t *mcs_tof,
                                          uint8_t align_cycles_iters,
                                          int64_t delay_fs)
{
    int32_t err = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(mcs_tof);

    err = adi_adf4030_auto_align_delay_set(adf4030,
                                           mcs_tof->fpga.bsync_out_sysref_ch,
                                           mcs_tof->adf4030_ref_freq_hz,
                                           delay_fs);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_auto_align_iteration_set(adf4030, align_cycles_iters);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adf4030_auto_align_single_channel_set(adf4030,
                                                    mcs_tof->bsync_in_ref_ch,
                                                    mcs_tof->fpga.bsync_out_sysref_ch);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}
