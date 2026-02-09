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
#include "adi_apollo.h"
#include "adi_apollo_utils.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_vec.h"

static int32_t configure_lb1_test_mode(adi_apollo_device_t *device, uint16_t cducs);
static int32_t configure_lb2_test_mode(adi_apollo_device_t *device, uint16_t fducs, uint16_t cduc_ratio);
static int32_t tx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile);

int32_t loopback1_2(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool lb1_blend = false;
    bool nco_test_mode = true;
    uint16_t lb1_cducs = ADI_APOLLO_CDUC_A0 | ADI_APOLLO_CDUC_A1 | ADI_APOLLO_CDUC_A2 | ADI_APOLLO_CDUC_A3;
    uint16_t lb2_fducs = ADI_APOLLO_FDUC_B_ALL; // All FDUCs on Side B
    uint32_t run_cals = ADI_ADS10_APOLLO_CAL_CC | (nco_test_mode ? ADI_ADS10_APOLLO_CAL_NONE : ADI_ADS10_APOLLO_CAL_ADC);

    printf("Start Loopback 1 on Side A, Loopback 2 on Side B\n");

    // Setup
    if (nco_test_mode) {
        printf("Using NCO test mode...\n");
        err = configure_lb1_test_mode(device, lb1_cducs);
        ADI_CMS_ERROR_RETURN(err);
        err = configure_lb2_test_mode(device, lb2_fducs, profile->tx_path[0].tx_cduc[0].drc_ratio);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_apollo_loopback_lb1_enable_set(device, ADI_APOLLO_SIDE_A, 1); // Enables loopback1 on Rx (per side)
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb1_cduc_enable_set(device, lb1_cducs, 1); // Enables loopback1 on Tx (per cduc)
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb2_enable_set(device, ADI_APOLLO_SIDE_B, 1); // Enables loopback2 on Rx (per side)
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_loopback_lb2_fduc_enable_set(device, lb2_fducs, 1); // Enables loopback2 on Tx (per fduc)
    ADI_CMS_ERROR_RETURN(err);

    // Run cals
    err = adi_ads10_apollo_ex_cals_run(device, profile, run_cals);
    ADI_CMS_ERROR_RETURN(err);

    if (lb1_blend) {
        printf("Enable blending...\n");

        err = tx_jesd(device, fpga_device, profile);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_apollo_loopback_lb1_blend_set(device, lb1_cducs, ADI_APOLLO_LB1_BLEND_DIV_2);
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

    for (int i = 0; i < ADI_APOLLO_CNCO_NUM; i++) {
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

    for (int i = 0; i < ADI_APOLLO_FNCO_NUM; i++) {
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
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t tx_dp_info;
    double tone_ratio = 0.4;         // Percent of JESD data rate side A (0.4 => 500 @1250Gsps)
    uint32_t vec_len = DEFAULT_NUM_SAMPLES_H;
    double tone_freq;

    /* Get Tx datapath info */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);

    /* Display data path freq info */
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, vec_len, tone_ratio, -1.0);
    ADI_CMS_ERROR_RETURN(err);

    /* Transmit data out from FPGA JTx */
    err = adi_fpga_apollo_core_tx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_jrx_rm_fifo_reset(device, ADI_APOLLO_LINK_ALL);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_tx_freq_get(device, profile, tone_ratio, &tone_freq);
    ADI_CMS_ERROR_RETURN(err);

    printf("Expect %fMHz.\n", tone_freq);

    return API_CMS_ERROR_OK;
}
