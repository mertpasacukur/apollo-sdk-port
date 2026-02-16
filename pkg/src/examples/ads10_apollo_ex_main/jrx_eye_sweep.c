#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     Apollo Serdes JRx Horizontal and Vertical Eye Sweep Example
 *
 * Horizontal JRx Eye Sweeps returns Left and Right SPO index values for corresponding lanes and prints them.
 *
 * Vertical JRx Eye Sweeps return an array of positive and negative SPO values for each SPO step.
 * User can perform multiple back-to-back sweeps and the number of sweeps can be set using variable `total_sweeps`. Default is 32.
 * At the end of each vertical sweep, for a given lane, all SPO values are saved to a file with the name and location of each file printed.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__linux__)
#include <unistd.h>
#endif
#include <math.h>
#include "adi_apollo.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_types.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_inspect.h"


int32_t jrx_eye_sweep(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err;
    uint16_t i = 0;
    uint16_t total_sweeps = 32;
    uint16_t serdes = ((((profile->jrx[0].rx_link_cfg[0].link_in_use) || (profile->jrx[0].rx_link_cfg[1].link_in_use))
                           ? ADI_APOLLO_TXRX_SERDES_12PACK_A : ADI_APOLLO_TXRX_SERDES_12PACK_NONE)) |
                      ((((profile->jrx[1].rx_link_cfg[0].link_in_use) || (profile->jrx[1].rx_link_cfg[1].link_in_use))
                           ? ADI_APOLLO_TXRX_SERDES_12PACK_B : ADI_APOLLO_TXRX_SERDES_12PACK_NONE));
    adi_apollo_serdes_bgcal_state_t serdes_bgcal_state[2] = {{0}};
    uint8_t sweep_data_per_run = ADI_APOLLO_SERDES_JRX_VERT_EYE_TEST_RESP_BUF_SIZE;
    uint16_t sweep_cnt = 0;
    char dir[MAX_PATH_LEN];     // Directory in which sweep data will be saved.
    char fname[MAX_PATH_LEN];   // File name for sweep data.
    FILE *fp = NULL;

    adi_apollo_fpga_prbs_e fpga_prbs_pat = PRBS_31;
    adi_apollo_fpga_prbs_e lane_prbs[] = { fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat,
                                           fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat,
                                           fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat,
                                           fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat,
                                           fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat,
                                           fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat, fpga_prbs_pat };
    uint32_t lane_prbs_len = sizeof(lane_prbs) / sizeof(lane_prbs[0]);

    adi_apollo_serdes_jrx_horiz_eye_resp_t horz_resp[ADI_APOLLO_SERDES_LANE_LEN];
    adi_apollo_serdes_jrx_vert_eye_resp_t vert_resp;
    uint8_t serdes_prbs_pat = ADI_APOLLO_SERDES_JRX_PRBS31;
    uint8_t lane_num = 0;

    /* Make sure JRX_EYE_SWEEP_FILE_PATH/Profile exists */
    snprintf(dir, MAX_PATH_LEN, "%s/%s", JRX_EYE_SWEEP_FILE_PATH, argv[2]);
    err = adi_apollo_utilities_mkdir(device, true, dir);
    ADI_CMS_ERROR_RETURN(err);

    /* Clock conditioning calibration */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Inspect the Apollo JRx and JTx link config */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* Initialize the FPGA for PRBS testing */
    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_pattern_set(fpga_device, lane_prbs, lane_prbs_len);
    ADI_CMS_ERROR_RETURN(err);

    /* Start FPGA PRBS testing */
    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_start(fpga_device, false);
    ADI_CMS_ERROR_RETURN(err);

    adi_apollo_hal_delay_us(device, 100 * 1000);
    ADI_CMS_ERROR_RETURN(err);

    /* Run SerDes Init(FG) Cal */
    if (adi_ads10_apollo_ex_run_serdes_init_cal_get(profile)) {
        printf("Run SERDES JRx FG cal...\n");
        err = adi_apollo_serdes_jrx_init_cal(device, serdes, ADI_APOLLO_INIT_CAL_ENABLED);
        // ADI_CMS_ERROR_RETURN(err);  // Ignoring SERDES cal failure for setups and/or device profiles that don't support all lanes
        printf("SERDES JRx FG cal %s\n", (err == 0) ? "done" : "FAILED!!");
    }

    /* Start SerDes Track(BG) Cal */
    if (adi_ads10_apollo_ex_run_serdes_track_cal_get(profile)) {
        printf("Run SERDES JRx BG cal...\n");
        err = adi_apollo_serdes_jrx_bgcal_unfreeze(device, serdes);
        ADI_CMS_ERROR_RETURN(err);
        printf("SERDES JRx BG cal done\n");

        err = adi_apollo_serdes_jrx_bgcal_state_get(device, serdes, serdes_bgcal_state, 2);
        ADI_CMS_ERROR_RETURN(err);

        printf("Serdes BGCal State after cal run.\n");
        for(i = 0; i < 2; i++) {
            printf("Serdes Pack: %02d \t", i);
            printf("Serdes state_valid: %02d \t", serdes_bgcal_state[i].state_valid);
            printf("Serdes bgcal_error: 0x%X \t", serdes_bgcal_state[i].bgcal_error);
            printf("Serdes bgcal_state: 0x%X.\n", serdes_bgcal_state[i].bgcal_state);
        }
    }

    /* Let SerDes BG cal run for a bit */
    adi_apollo_hal_delay_us(device, 10 * 1000000);

    /* Halt SerDes Track(BG) Cal for sweeps */
    if (adi_ads10_apollo_ex_run_serdes_track_cal_get(profile)) {
        printf("Freeze SERDES JRx BG cal...\n");
        err = adi_apollo_serdes_jrx_bgcal_freeze(device, serdes);
        ADI_CMS_ERROR_RETURN(err);
    }

    printf("\nRunning JRx Horizontal EYE Sweep...\n");
    for (lane_num = 0; lane_num < ADI_APOLLO_SERDES_LANE_LEN; lane_num++) {

        err = adi_apollo_serdes_jrx_horiz_eye_sweep(device, lane_num, serdes_prbs_pat);
        ADI_CMS_ERROR_RETURN(err);

        adi_apollo_hal_delay_us(device, 1000);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_serdes_jrx_horiz_eye_sweep_resp_get(device, lane_num, &horz_resp[lane_num]);
        ADI_CMS_ERROR_RETURN(err);

        printf("lane_num: %02d\t version: %02d\t spo_left: %02d\t spo_right: %02d.\n", lane_num, horz_resp[lane_num].ver, horz_resp[lane_num].spo_left, horz_resp[lane_num].spo_right);
    }

    printf("\nRunning JRx Vertical EYE Sweeps...\n");
    printf("Total Sweeps per lane: %02d.\n", total_sweeps);
    for (lane_num = 0; lane_num < ADI_APOLLO_SERDES_LANE_LEN; lane_num++) {

        snprintf(fname, MAX_PATH_LEN, "%s/%s_lane_%02d.txt", dir, "apollo_jrx_eye_data", lane_num);
        printf("lane_num: %02d \t Saving File: %s\n", lane_num, fname);

        for (sweep_cnt = 0; sweep_cnt < total_sweeps; sweep_cnt++) {
            err = adi_apollo_serdes_jrx_vert_eye_sweep(device, lane_num);
            ADI_CMS_ERROR_RETURN(err);

            err = adi_apollo_hal_delay_us(device, 1000);
            ADI_CMS_ERROR_RETURN(err);

            err = adi_apollo_serdes_jrx_vert_eye_sweep_resp_get(device, lane_num, &vert_resp);
            ADI_CMS_ERROR_RETURN(err);

            fp = fopen(fname, "a");
            ADI_CMS_FILE_OPEN_CHECK(fp);

            for (i = 0; i < sweep_data_per_run; i++) {
                fprintf(fp, "%02d\n", vert_resp.eye_heights_at_spo[i]);
            }

            ADI_CMS_FILE_CLOSE(fp);
        }
    }

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
