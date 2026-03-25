#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo Loopback1 and 2 example
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 *
 * This loopback example enables loopback 1 on side A and loopback 2 on side B
 * For lb1, blending can be enabled to combine Apollo JRx data with the loopback.
 *
 * Loopback 1 is always from CDDC->CDUC 0->0, 1->1, 2->2, 3->3, 4->4
 * Profiles must have the same drc_en per CDRC on Tx and Rx or data will be lost.
 * Ensure that all CDDCs that receive data correspond to CDUCs that transmit data.
 *
 *  ADC_DATA  --mux-->  CDDC
 *                        |
 *                        V
 *  DAC_DATA  <--mux--  CDUC
 *
 * Loopback 2 is always from FDDC->FDUC 0->0, 1->1, ..., 6->6, 7->7
 * Profiles must have the same drc_en per FDRC on Tx and Rx or data will be lost.
 * Ensure that all FDDCs that receive data correspond to FDUCs that transmit data.
 *
 *  ADC_DATA  --mux-->  CDDC  --mux-->  FDDC
 *                                        |
 *                                        V
 *  DAC_DATA  <--mux--  CDUC  <--mux--  FDUC
 *
 * For LB1, use the CDDC BMEM for sample delay
 * For LB2, use the FDDC BMEM for sample delay
 * See the example rx_bmem_delay for how to configure BMEM sample delay
 * See the example lb0_bmem_hop_delay for how to configure BMEM sample delay with hopping
 *
 * Set the flag lb1_blend to enable blending on LB1, where the Tx JESD and LB1 data are both combined.
 * Use the FDUC gain to scale JESD and avoid an overflow
 *
 * Set the nco_test_mode flag to inject a test tone into loopback rather than ADC data.
 * For LB1, the CDDC is put into test mode and that data is looped back to the CDUC
 * For lb2, the FDDC is put into test mode and that data is looped back to the FDUC
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "adi_apollo.h"
#include "adi_apollo_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_ex_ctl.h"

#define TONE_RATIO               0.4       /* Percent of JESD data rate side A (0.4 => 500 @1250Gsps) */

/**
 * Test Mode DAC measurement frequencies in MHz.
 * Change them to an appropriate value depending on sample rate but for most profiles, it works as is.
 */
#define LB1_TEST_DAC_A0_FREQ     2000.0    /* MHz */
#define LB1_TEST_DAC_A1_FREQ     2250.0    /* MHz */
#define LB2_TEST_DAC_B0_FREQ     1000.0    /* MHz */
#define LB2_TEST_DAC_B1_FREQ     1100.0    /* MHz */

/* Normal mode measurement parameters */
#define SIGGEN_FREQ              2000.0    /* MHz */
#define SIGGEN_LEVEL             6.0       /* dBm */
#define MEAS_FREQ_TOL            0.1       /* +- tolerance in MHz */
#define MEAS_LEV_TOL             1.0       /* +- tolerance in dBm */

/* Expected tone levels for blending modes in dBm */
#define BLEND_LEVEL_DIV_2        -6.0
#define BLEND_LEVEL_DIV_4        -12.0


/**
 * \brief Configure Loopback1 test mode (CDDC test mode to CDUC)
 *
 * \param[in] device Pointer to the Apollo device structure
 * \param[in] cducs  Bitmask of CDUCs to configure for test mode
 *
 * \return Error code indicating success or failure
 */
static int32_t configure_lb1_test_mode(adi_apollo_device_t *device, uint16_t cducs);

/**
 * \brief Configure Loopback2 test mode (FDDC test mode to FDUC)
 *
 * \param[in] device      Pointer to the Apollo device structure
 * \param[in] fducs       Bitmask of FDUCs to configure for test mode
 * \param[in] cduc_ratio  CDUC decimation/interpolation ratio
 *
 * \return Error code indicating success or failure
 */
static int32_t configure_lb2_test_mode(adi_apollo_device_t *device, uint16_t fducs, uint16_t cduc_ratio);

/**
 * \brief Set up JESD transmit path and generate a test tone from the FPGA
 *
 * \param[in] device      Pointer to the Apollo device structure
 * \param[in] fpga_device Pointer to the FPGA device structure
 * \param[in] profile     Pointer to the Apollo profile structure
 *
 * \return Error code indicating success or failure
 */
static int32_t tx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile);

/**
 * \brief Perform frequency measurements in NCO test mode for all DACs
 *
 * This function prints the expected frequencies and calls the measurement macros for each DAC.
 *
 * \param[in] interactive Interactive mode flag for measurements
 */
static void perform_nco_test_mode_measurements(bool interactive);

/**
 * \brief Perform measurements for blending mode on Side A (Configured with Loopback1)
 *
 * This function performs measurements when blending is enabled on Loopback 1 (Side A only).
 * It uses the provided selector array to iterate over channels and measure frequency and level
 * for each DAC channel.
 *
 * \param[in] device                Pointer to the Apollo device structure
 * \param[in] profile               Pointer to the Apollo profile structure
 * \param[in] selectors_A           2D array of RX/TX selectors for Side A (selectors_A[channel][0]=RX, [1]=TX)
 * \param[in] expected_tone_level   Expected tone level in dBm
 * \param[in] interactive           Interactive mode flag
 * \param[in] rx_channel_info       Pointer to structure to receive last Rx channel info
 * \param[in] tx_channel_info       Pointer to structure to receive last Tx channel info
 *
 * \retval API_CMS_ERROR_OK    Success
 * \retval <0                  Error code. Check \ref adi_cms_error_e enum for details.
 *
 * \note This function only operates on Side A channels since blending is LB1-specific.
 */
static int32_t perform_blend_measurements(adi_apollo_device_t *device,
                                          adi_apollo_top_t *profile,
                                          adi_ads10_apollo_channel_selectors_t selectors_A[4][2],
                                          double expected_tone_level,
                                          bool interactive,
                                          adi_ads10_apollo_channel_info_t *rx_channel_info,
                                          adi_ads10_apollo_channel_info_t *tx_channel_info);

/**
 * \brief Perform frequency measurements for loopback configuration
 *
 * This unified function performs frequency measurements for both 4T4R and 8T8R device configurations.
 * It automatically detects the device type and uses the appropriate number of channels and DAC mappings.
 * The function iterates over RX/TX channel pairs for both Side A (LB1) and Side B (LB2) using the
 * provided selector arrays.
 *
 * \param[in] device              Pointer to the Apollo device structure
 * \param[in] profile             Pointer to the Apollo profile structure
 * \param[in] selectors_A         2D array of RX/TX selectors for Side A (selectors_A[channel][0]=RX, [1]=TX)
 * \param[in] selectors_B         2D array of RX/TX selectors for Side B (selectors_B[channel][0]=RX, [1]=TX)
 * \param[in] interactive         Interactive mode flag
 * \param[in] rx_channel_info     Pointer to structure to receive last Rx channel info
 * \param[in] tx_channel_info     Pointer to structure to receive last Tx channel info
 *
 * \retval API_CMS_ERROR_OK    Success
 * \retval <0                  Error code. Check \ref adi_cms_error_e enum for details.
 *
 * \note For 4T4R devices: Uses channels 0-1, Side B includes FNCO frequency compensation
 * \note For 8T8R devices: Uses channels 0-3, Side B includes FNCO frequency compensation
 * \note Side A (LB1): CNCO compensation only (Loopback1 path)
 * \note Side B (LB2): CNCO + FNCO compensation (Loopback2 path)
 */
static int32_t perform_measurements(adi_apollo_device_t *device,
                                    adi_apollo_top_t *profile,
                                    adi_ads10_apollo_channel_selectors_t selectors_A[4][2],
                                    adi_ads10_apollo_channel_selectors_t selectors_B[4][2],
                                    bool interactive,
                                    adi_ads10_apollo_channel_info_t *rx_channel_info,
                                    adi_ads10_apollo_channel_info_t *tx_channel_info);

int32_t loopback1_2(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool lb1_blend = true;
    bool nco_test_mode = false;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); /* default not interactive */
    adi_ads10_apollo_channel_info_t rx_channel_info;
    adi_ads10_apollo_channel_info_t tx_channel_info;
    uint16_t lb1_cducs = ADI_APOLLO_CDUC_A0 | ADI_APOLLO_CDUC_A1 | ADI_APOLLO_CDUC_A2 | ADI_APOLLO_CDUC_A3;   /* Select CDUCs for Side A */
    uint16_t lb2_fducs = ADI_APOLLO_FDUC_B_ALL; /* Select All FDUCs on Side B */
    uint32_t run_cals = ADI_ADS10_APOLLO_CAL_CC | (nco_test_mode ? ADI_ADS10_APOLLO_CAL_NONE : ADI_ADS10_APOLLO_CAL_ADC | ADI_ADS10_APOLLO_CAL_ADC_BG);
    /* Channel selectors for Side A (Loopback1). Paired RX and TX */
    adi_ads10_apollo_channel_selectors_t selectors_A[4][2] = {
        { {ADI_APOLLO_CDDC_A0, ADI_APOLLO_CNCO_A0, ADI_APOLLO_FNCO_A0, ADI_APOLLO_FDDC_A0, ADI_APOLLO_FSRC_A0},
            {ADI_APOLLO_CDUC_A0, ADI_APOLLO_CNCO_A0, ADI_APOLLO_FNCO_A0, ADI_APOLLO_FDUC_A0, ADI_APOLLO_FSRC_A0} },
        { {ADI_APOLLO_CDDC_A1, ADI_APOLLO_CNCO_A1, ADI_APOLLO_FNCO_A1, ADI_APOLLO_FDDC_A1, ADI_APOLLO_FSRC_A0},
            {ADI_APOLLO_CDUC_A1, ADI_APOLLO_CNCO_A1, ADI_APOLLO_FNCO_A1, ADI_APOLLO_FDUC_A1, ADI_APOLLO_FSRC_A0} },
        { {ADI_APOLLO_CDDC_A2, ADI_APOLLO_CNCO_A2, ADI_APOLLO_FNCO_A2, ADI_APOLLO_FDDC_A2, ADI_APOLLO_FSRC_A1},
            {ADI_APOLLO_CDUC_A2, ADI_APOLLO_CNCO_A2, ADI_APOLLO_FNCO_A2, ADI_APOLLO_FDUC_A2, ADI_APOLLO_FSRC_A1} },
        { {ADI_APOLLO_CDDC_A3, ADI_APOLLO_CNCO_A3, ADI_APOLLO_FNCO_A3, ADI_APOLLO_FDDC_A3, ADI_APOLLO_FSRC_A1},
            {ADI_APOLLO_CDUC_A3, ADI_APOLLO_CNCO_A3, ADI_APOLLO_FNCO_A3, ADI_APOLLO_FDUC_A3, ADI_APOLLO_FSRC_A1} }
    };
    /* Channel selectors for Side B (Loopback2). Paired RX and TX */
    adi_ads10_apollo_channel_selectors_t selectors_B[4][2] = {
        { {ADI_APOLLO_CDDC_B0, ADI_APOLLO_CNCO_B0, ADI_APOLLO_FNCO_B0, ADI_APOLLO_FDDC_B0, ADI_APOLLO_FSRC_B0},
            {ADI_APOLLO_CDUC_B0, ADI_APOLLO_CNCO_B0, ADI_APOLLO_FNCO_B0, ADI_APOLLO_FDUC_B0, ADI_APOLLO_FSRC_B0} },
        { {ADI_APOLLO_CDDC_B1, ADI_APOLLO_CNCO_B1, ADI_APOLLO_FNCO_B1, ADI_APOLLO_FDDC_B1, ADI_APOLLO_FSRC_B0},
            {ADI_APOLLO_CDUC_B1, ADI_APOLLO_CNCO_B1, ADI_APOLLO_FNCO_B1, ADI_APOLLO_FDUC_B1, ADI_APOLLO_FSRC_B0} },
        { {ADI_APOLLO_CDDC_B2, ADI_APOLLO_CNCO_B2, ADI_APOLLO_FNCO_B2, ADI_APOLLO_FDDC_B2, ADI_APOLLO_FSRC_B1},
            {ADI_APOLLO_CDUC_B2, ADI_APOLLO_CNCO_B2, ADI_APOLLO_FNCO_B2, ADI_APOLLO_FDUC_B2, ADI_APOLLO_FSRC_B1} },
        { {ADI_APOLLO_CDDC_B3, ADI_APOLLO_CNCO_B3, ADI_APOLLO_FNCO_B3, ADI_APOLLO_FDDC_B3, ADI_APOLLO_FSRC_B1},
            {ADI_APOLLO_CDUC_B3, ADI_APOLLO_CNCO_B3, ADI_APOLLO_FNCO_B3, ADI_APOLLO_FDUC_B3, ADI_APOLLO_FSRC_B1} }
    };

    printf("Start Loopback 1 on Side A, Loopback 2 on Side B\n");

    // Setup
    if (nco_test_mode) {
        printf("Using NCO test mode...\n");
        err = configure_lb1_test_mode(device, lb1_cducs);
        ADI_CMS_ERROR_RETURN(err);
        err = configure_lb2_test_mode(device, lb2_fducs, profile->tx_path[0].tx_cduc[0].drc_ratio);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_apollo_loopback_lb1_enable_set(device, ADI_APOLLO_SIDE_A, 1); /* Enables loopback1 on Rx (per side) */
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb1_cduc_enable_set(device, lb1_cducs, 1); /* Enables loopback1 on Tx (per cduc) */
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb2_enable_set(device, ADI_APOLLO_SIDE_B, 1); /* Enables loopback2 on Rx (per side) */
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb2_fduc_enable_set(device, lb2_fducs, 1); /* Enables loopback2 on Tx (per fduc) */
    ADI_CMS_ERROR_RETURN(err);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, run_cals);
    ADI_CMS_ERROR_RETURN(err);

    if (nco_test_mode) {
        perform_nco_test_mode_measurements(interactive);
    } else {
        printf("Setting signal generator and performing DACs measurements.\n");
        EXCTL_SIGGEN_ON(0xFF, 1);
        EXCTL_SIGGEN_FREQ(SIGGEN_FREQ, 0);
        EXCTL_SIGGEN_LEVEL(SIGGEN_LEVEL, interactive);

        /* Use unified measurement function - automatically detects device type */
        err = perform_measurements(device, profile, selectors_A, selectors_B, interactive, &rx_channel_info, &tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (lb1_blend) {
        printf("Enable blending...\n");
        err = tx_jesd(device, fpga_device, profile);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_loopback_lb1_blend_set(device, lb1_cducs, ADI_APOLLO_LB1_BLEND_DISABLE);
        ADI_CMS_ERROR_RETURN(err);

        /* Get channel info using unified selector approach for channel A0 */
        err = adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, selectors_A[0][0], &rx_channel_info);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_inspect_tx_channel_get(device, profile, selectors_A[0][1], &tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        /* Use device-appropriate DAC selector and direct calculation in macro */
        if (device->dev_info.is_8t8r) {
            EXCTL_TX_REF(ADI_APOLLO_DAC_A_ALL,
                        (SIGGEN_FREQ + (((double)tx_channel_info.cnco_freq_hz - (double)rx_channel_info.cnco_freq_hz) / 1e6)),
                        interactive);
        } else {
            EXCTL_TX_REF(ADI_APOLLO_DAC_A_ALL_4T4R,
                        (SIGGEN_FREQ + (((double)tx_channel_info.cnco_freq_hz - (double)rx_channel_info.cnco_freq_hz) / 1e6)),
                        interactive);
        }

        err = adi_apollo_loopback_lb1_blend_set(device, lb1_cducs, ADI_APOLLO_LB1_BLEND_DIV_2);
        ADI_CMS_ERROR_RETURN(err);
        err = perform_blend_measurements(device, profile, selectors_A, BLEND_LEVEL_DIV_2, interactive, &rx_channel_info, &tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_loopback_lb1_blend_set(device, lb1_cducs, ADI_APOLLO_LB1_BLEND_DIV_4);
        ADI_CMS_ERROR_RETURN(err);
        err = perform_blend_measurements(device, profile, selectors_A, BLEND_LEVEL_DIV_4, interactive, &rx_channel_info, &tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_loopback_lb1_blend_set(device, lb1_cducs, ADI_APOLLO_LB1_BLEND_DISABLE);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

static int32_t configure_lb1_test_mode(adi_apollo_device_t *device, uint16_t cducs)
{
    int32_t err = API_CMS_ERROR_OK;
    uint16_t cnco;
    double nco_freq_offset = .05;
    double nco_freq_step = .0125;

    // Loop over all CNCOs (constant defined in API)
    for (uint32_t i = 0; i < ADI_APOLLO_CNCO_NUM; i++) {
        cnco = (1 << i);
        if ((cducs & cnco) > 0) {
            // Set Rx CNCO to negative and Tx to positive, base tone with no step will be at 2 * nco_freq_offset
            err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_RX, cnco, 1, -nco_freq_offset - nco_freq_step * (i%ADI_APOLLO_CNCO_PER_SIDE_NUM));
            ADI_CMS_ERROR_RETURN(err);
            err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_TX, cnco, 1, nco_freq_offset);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_apollo_cnco_test_mode_val_set(device, ADI_APOLLO_RX, cnco, ADI_APOLLO_CNCO_MXR_TEST_RX_FS_BY2);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_apollo_cnco_mode_set(device, ADI_APOLLO_RX, cnco, ADI_APOLLO_MXR_TEST_MODE);
            ADI_CMS_ERROR_RETURN(err);
        }
    }
    return API_CMS_ERROR_OK;
}

static int32_t configure_lb2_test_mode(adi_apollo_device_t *device, uint16_t fducs, uint16_t cduc_ratio)
{
    int32_t err = API_CMS_ERROR_OK;
    uint16_t fnco;
    uint16_t cnco;
    double nco_freq_step = .5;

    // Loop over all FNCOs (constant defined in API)
    for (uint32_t i = 0; i < ADI_APOLLO_FNCO_NUM; i++) {
        fnco = (1 << i);
        if ((fducs & fnco) > 0) {
            // Set Rx FNCO to negative and Tx to 0, so tone will be at Tx CNCO + | Rx FNCO freq |
            err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_RX, fnco, 100. / cduc_ratio, -nco_freq_step * (i%ADI_APOLLO_FNCO_PER_SIDE_NUM));
            ADI_CMS_ERROR_RETURN(err);
            err = adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_TX, fnco, 100. / cduc_ratio, 0);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_apollo_fnco_test_mode_val_set(device, ADI_APOLLO_RX, fnco, ADI_APOLLO_FNCO_MXR_TEST_RX_FS_BY2);
            ADI_CMS_ERROR_RETURN(err);
            err = adi_apollo_fnco_mode_set(device, ADI_APOLLO_RX, fnco, ADI_APOLLO_MXR_TEST_MODE);
            ADI_CMS_ERROR_RETURN(err);
            cnco = 1 << (i%ADI_APOLLO_CNCO_PER_SIDE_NUM) << (i >= ADI_APOLLO_FNCO_PER_SIDE_NUM ? 4 : 0);
            err = adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_TX, cnco, 20, 1);
            ADI_CMS_ERROR_RETURN(err);
        }
    }
    return API_CMS_ERROR_OK;
}

int32_t tx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile)
{
    int32_t err;
    const uint32_t str_buff_len = 1024U;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t tx_dp_info;
    double tone_ratio = TONE_RATIO;
    uint32_t vec_len = DEFAULT_NUM_SAMPLES_H;

    /* Get Tx datapath info */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);

    /* Display data path freq info */
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, NULL, ADI_APOLLO_LINK_ALL, vec_len, tone_ratio, -1.0);
    ADI_CMS_ERROR_RETURN(err);

    /* Transmit data out from FPGA JTx */
    err = adi_fpga_apollo_core_tx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_jrx_rm_fifo_reset(device, ADI_APOLLO_LINK_ALL);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

void perform_nco_test_mode_measurements(bool interactive)
{
    printf("NCO Test Mode Enabled, no ADC calibration performed. Checking DACs measurements.\n");
    /* TAKE INTO ACCOUNT PROFILE DATA RATES AND FREQ MAY BE CHANGED WITH SOME PROFILES, SUPPORTED ONES ARE COVERED WITH
    THESE VALUES */
    EXCTL_TX_MEAS_FREQ(ADI_APOLLO_DAC_A0, LB1_TEST_DAC_A0_FREQ, MEAS_FREQ_TOL, interactive);
    EXCTL_TX_MEAS_FREQ(ADI_APOLLO_DAC_A1, LB1_TEST_DAC_A1_FREQ, MEAS_FREQ_TOL, interactive);
    EXCTL_TX_MEAS_FREQ(ADI_APOLLO_DAC_B0, LB2_TEST_DAC_B0_FREQ, MEAS_FREQ_TOL, interactive);
    EXCTL_TX_MEAS_FREQ(ADI_APOLLO_DAC_B1, LB2_TEST_DAC_B1_FREQ, MEAS_FREQ_TOL, interactive);
}

int32_t perform_blend_measurements(adi_apollo_device_t *device,
                                   adi_apollo_top_t *profile,
                                   adi_ads10_apollo_channel_selectors_t selectors_A[4][2],
                                   double expected_tone_level,
                                   bool interactive,
                                   adi_ads10_apollo_channel_info_t *rx_channel_info,
                                   adi_ads10_apollo_channel_info_t *tx_channel_info)
{
    int32_t err = API_CMS_ERROR_OK;
    double tone_freq_mhz;
    uint8_t ch;
    const uint8_t num_channels = (device->dev_info.is_8t8r) ? 4U : 2U;
    const uint16_t dac_all = (device->dev_info.is_8t8r) ? ADI_APOLLO_DAC_A_ALL : ADI_APOLLO_DAC_A_ALL_4T4R;
    static const uint16_t dac_channel[4] = {ADI_APOLLO_DAC_A0, ADI_APOLLO_DAC_A1, ADI_APOLLO_DAC_A2, ADI_APOLLO_DAC_A3};

    ADI_CMS_NULL_PTR_CHECK(selectors_A);
    ADI_CMS_NULL_PTR_CHECK(rx_channel_info);
    ADI_CMS_NULL_PTR_CHECK(tx_channel_info);

    printf("LB1 blending enabled, checking blended tone frequency...\n");

    err = adi_ads10_apollo_ex_inspect_tx_freq_get(device, profile, TONE_RATIO, &tone_freq_mhz);
    ADI_CMS_ERROR_RETURN(err);

    printf("Expect %fMHz.\n", tone_freq_mhz);
    printf("%s device, checking %u CDUCs...\n",
        (device->dev_info.is_8t8r) ? "8T8R" : "4T4R", num_channels);

    /* Measure blended frequency on all active DACs from jesd tx_injected tone */
    EXCTL_TX_MEAS_FREQ(dac_all, tone_freq_mhz, MEAS_FREQ_TOL, interactive);

    /* Loop through channels based on device type */
    for (ch = 0; ch < num_channels; ch++) {
        /* Get RX channel info using selectors */
        err = adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, selectors_A[ch][0], rx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        /* Get TX channel info using selectors */
        err = adi_ads10_apollo_ex_inspect_tx_channel_get(device, profile, selectors_A[ch][1], tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        /* Measure frequency and level on each DAC channel from each ADC channel*/
        EXCTL_TX_MEAS_FREQ(dac_channel[ch],
                          (SIGGEN_FREQ + (((double)tx_channel_info->cnco_freq_hz - (double)rx_channel_info->cnco_freq_hz) / 1e6)),
                           MEAS_FREQ_TOL,
                           interactive);
        EXCTL_TX_MEAS_LEVEL(dac_channel[ch],
                           (SIGGEN_FREQ + (((double)tx_channel_info->cnco_freq_hz - (double)rx_channel_info->cnco_freq_hz) / 1e6)),
                            expected_tone_level,
                            MEAS_LEV_TOL,
                            interactive);
    }

    return API_CMS_ERROR_OK;
}

int32_t perform_measurements(adi_apollo_device_t *device,
                             adi_apollo_top_t *profile,
                             adi_ads10_apollo_channel_selectors_t selectors_A[4][2],
                             adi_ads10_apollo_channel_selectors_t selectors_B[4][2],
                             bool interactive,
                             adi_ads10_apollo_channel_info_t *rx_channel_info,
                             adi_ads10_apollo_channel_info_t *tx_channel_info)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t ch;
    const uint8_t num_channels = (device->dev_info.is_8t8r) ? 4U : 2U;
    /* Static DAC mappings for both device types */
    static const uint16_t dac_a[4] = {ADI_APOLLO_DAC_A0, ADI_APOLLO_DAC_A1, ADI_APOLLO_DAC_A2, ADI_APOLLO_DAC_A3};
    static const uint16_t dac_b[4] = {ADI_APOLLO_DAC_B0, ADI_APOLLO_DAC_B1, ADI_APOLLO_DAC_B2, ADI_APOLLO_DAC_B3};

    /* Input parameter validation */
    ADI_CMS_NULL_PTR_CHECK(selectors_A);
    ADI_CMS_NULL_PTR_CHECK(selectors_B);
    ADI_CMS_NULL_PTR_CHECK(rx_channel_info);
    ADI_CMS_NULL_PTR_CHECK(tx_channel_info);

    printf("Performing %s measurements (%u channels per side)...\n",
           (device->dev_info.is_8t8r) ? "8T8R" : "4T4R", num_channels);

    /* Side A measurements (Loopback1 - CNCO compensation only) */
    for (ch = 0; ch < num_channels; ch++) {
        err = adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, selectors_A[ch][0], rx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_inspect_tx_channel_get(device, profile, selectors_A[ch][1], tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        /* Side A frequency calculation: SIGGEN_FREQ  + TX_CNCO - RX_CNCO*/
        EXCTL_TX_MEAS_FREQ(dac_a[ch],
                          (SIGGEN_FREQ + (((double)tx_channel_info->cnco_freq_hz - (double)rx_channel_info->cnco_freq_hz) / 1e6)),
                          MEAS_FREQ_TOL,
                          interactive);
    }

    /* Side B measurements (Loopback2 - CNCO + FNCO compensation) */
    for (ch = 0; ch < num_channels; ch++) {
        err = adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, selectors_B[ch][0], rx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_inspect_tx_channel_get(device, profile, selectors_B[ch][1], tx_channel_info);
        ADI_CMS_ERROR_RETURN(err);

        /* Side B frequency calculation: SIGGEN_FREQ + TX_CNCO + TX_FNCO - RX_CNCO - RX_FNCO */
        EXCTL_TX_MEAS_FREQ(dac_b[ch],
                          (SIGGEN_FREQ + (((double)tx_channel_info->cnco_freq_hz + (double)tx_channel_info->fnco_freq_hz
                                - (double)rx_channel_info->cnco_freq_hz - (double)rx_channel_info->fnco_freq_hz) / 1e6)),
                          MEAS_FREQ_TOL,
                          interactive);
    }

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
