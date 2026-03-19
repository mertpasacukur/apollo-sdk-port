/*!
 * \brief     Apollo PRBS example running on ADS10 platform
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "adi_apollo.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_fpga_apollo_core.h"
#include "adi_utils.h"

#define ADI_APOLLO_SERDES_SPO_MAX 32

static int32_t jtx_prbs_test(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t active_lanes);
static int32_t jrx_prbs_test(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t active_lanes);
static int32_t spo_sweep_prbs(adi_apollo_device_t *device, adi_apollo_serdes_ck_spo_strobe_e strobe, adi_apollo_deser_rate_mode_e mode, uint8_t active_lanes);

int32_t prbs(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst) {

    int32_t err;
    uint8_t active_links = adi_api_utils_num_selected(profile->jrx->common_link_cfg.lane_enables) * 2; // Assume all links are the same

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Inspect the Apollo JRx and JTx link config */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    /* JTx PRBS Test (FPGA JRx) */
    err = jtx_prbs_test(device, fpga_device, active_links);
    //ADI_CMS_ERROR_RETURN(err);

    /* JRx PRBS Test (FPGA JTx) */
    err = jrx_prbs_test(device, fpga_device, profile, active_links);
    ADI_CMS_ERROR_RETURN(err);

    adi_apollo_deser_rate_mode_e mode = profile->jrx[0].deserializer_lane[0].rx_des_qhf_rate;
    err = spo_sweep_prbs(device, ADI_APOLLO_SERDES_CK_SPO_ISTROBE, mode, active_links); /* Left eye extents */
    ADI_CMS_ERROR_RETURN(err);

    err = spo_sweep_prbs(device, ADI_APOLLO_SERDES_CK_SPO_QSTROBE, mode, active_links); /* Right eye extents */
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

/* Configure and run a JTx PRBS test using the ADS10 FPGA */
static int32_t jtx_prbs_test(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, uint8_t active_lanes)
{
    int32_t err;
    uint32_t i;
	adi_apollo_fpga_prbs_e prbs = PRBS_7;
    uint16_t num_lanes_active = 0, num_lanes_with_errors = 0;

    adi_apollo_serdes_prbs_generator_enable_t prbs_gen = {
        .enable = 1,
        .mode = ADI_APOLLO_SERDES_JTX_PRBS7     /* PRBS-7 test pattern */
    };

    uint8_t lanes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                        12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    uint32_t lanes_len = sizeof(lanes) / sizeof(lanes[0]);

    /* Configure Apollo device for PRBS mode, all lanes */
    err = adi_apollo_serdes_prbs_generator_enable(device, lanes, lanes_len, &prbs_gen);
    ADI_CMS_ERROR_RETURN(err);

    /* Initialize the FPGA for PRBS testing */
    err = adi_fpga_apollo_core_jesd_rx_phy_prbs_init(fpga_device, prbs);
	ADI_CMS_ERROR_RETURN(err);

    /* Check and print PRBS results. Clear errors on first iteration

        Sample Output:

        - Each row represents a Lane.
        - For this example, JESD 'L' is 8 & 8 for 16 total lanes.
        - There is not a 1-to-1 mapping of Apollo to FPGA lanes. Indexes here represent APollo phy lanes

            PRBS LOCK STATUS: 0x00ffffff(fpga)    0x00ffffff(apollo)
            PRBS all lanes locked
            PRBS LOCKED: 0x00ffffff(fpga)    0x00ffffff(apollo)
            PRBS CYCLES[ 0]: 0x0000000019ef1683    ERRORS[ 0]: 0x00000000      LOCKED[ 0]: 1
            PRBS CYCLES[ 1]: 0x0000000019ef1683    ERRORS[ 1]: 0x00000000      LOCKED[ 1]: 1
            PRBS CYCLES[ 2]: 0x0000000019ef1683    ERRORS[ 2]: 0x00000000      LOCKED[ 2]: 1
            PRBS CYCLES[ 3]: 0x0000000019ef1683    ERRORS[ 3]: 0x00000000      LOCKED[ 3]: 1
            PRBS CYCLES[ 4]: 0x0000000019ef1683    ERRORS[ 4]: 0x00000000      LOCKED[ 4]: 1
            PRBS CYCLES[ 5]: 0x0000000019ef1683    ERRORS[ 5]: 0x00000000      LOCKED[ 5]: 1
            PRBS CYCLES[ 6]: 0x0000000019ef1683    ERRORS[ 6]: 0x00000000      LOCKED[ 6]: 1
            PRBS CYCLES[ 7]: 0x0000000019ef1684    ERRORS[ 7]: 0x00000000      LOCKED[ 7]: 1
            PRBS CYCLES[ 8]: 0x0000000019ef1683    ERRORS[ 8]: 0x00000000      LOCKED[ 8]: 1
            PRBS CYCLES[ 9]: 0x0000000019ef1683    ERRORS[ 9]: 0x00000000      LOCKED[ 9]: 1
            PRBS CYCLES[10]: 0x0000000019ef1683    ERRORS[10]: 0x00000000      LOCKED[10]: 1
            PRBS CYCLES[11]: 0x0000000019ef1683    ERRORS[11]: 0x00000000      LOCKED[11]: 1
            PRBS CYCLES[12]: 0x0000000019ef1684    ERRORS[12]: 0x00000000      LOCKED[12]: 1
            PRBS CYCLES[13]: 0x0000000019ef1684    ERRORS[13]: 0x00000000      LOCKED[13]: 1
            PRBS CYCLES[14]: 0x0000000019ef1683    ERRORS[14]: 0x00000000      LOCKED[14]: 1
            PRBS CYCLES[15]: 0x0000000019ef1683    ERRORS[15]: 0x00000000      LOCKED[15]: 1
            PRBS CYCLES[16]: 0x0000000019ef1683    ERRORS[16]: 0x00000000      LOCKED[16]: 1
            PRBS CYCLES[17]: 0x0000000019ef1684    ERRORS[17]: 0x00000000      LOCKED[17]: 1
            PRBS CYCLES[18]: 0x0000000019ef1683    ERRORS[18]: 0x00000000      LOCKED[18]: 1
            PRBS CYCLES[19]: 0x0000000019ef1684    ERRORS[19]: 0x00000000      LOCKED[19]: 1
            PRBS CYCLES[20]: 0x0000000019ef1683    ERRORS[20]: 0x00000000      LOCKED[20]: 1
            PRBS CYCLES[21]: 0x0000000019ef1684    ERRORS[21]: 0x00000000      LOCKED[21]: 1
            PRBS CYCLES[22]: 0x0000000019ef1683    ERRORS[22]: 0x00000000      LOCKED[22]: 1
            PRBS CYCLES[23]: 0x0000000019ef1683    ERRORS[23]: 0x00000000      LOCKED[23]: 1
            PRBS #Lanes w/ Activity: 24    #Lanes w/ Errors: 0
    */

    for (i = 0; i < 3; i++) {
        adi_apollo_hal_delay_us(device, 2000000); /* Delay to accumulate PRBS data */
        err = adi_fpga_apollo_core_jesd_rx_phy_prbs_print_errors(fpga_device, prbs, (i == 0), &num_lanes_active, &num_lanes_with_errors);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

/* Configure and run a JRx PRBS test using the ADS10 FPGA */
static int32_t jrx_prbs_test(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint8_t active_lanes)
{
    int32_t err;
    uint32_t i;
    uint32_t post_start_delay = 100000;
    uint8_t lanes_wo_err = 0;
    uint16_t serdes = ((((profile->jrx[0].rx_link_cfg[0].link_in_use) || (profile->jrx[0].rx_link_cfg[1].link_in_use))
                           ? ADI_APOLLO_TXRX_SERDES_12PACK_A : ADI_APOLLO_TXRX_SERDES_12PACK_NONE)) |
                      ((((profile->jrx[1].rx_link_cfg[0].link_in_use) || (profile->jrx[1].rx_link_cfg[1].link_in_use))
                           ? ADI_APOLLO_TXRX_SERDES_12PACK_B : ADI_APOLLO_TXRX_SERDES_12PACK_NONE));
    adi_apollo_serdes_bgcal_state_t serdes_bgcal_state[2] = {{0}};

    adi_apollo_serdes_prbs_checker_enable_t prbs_checker = {
        .enable = 1,
        .auto_mode = 0,
        .auto_mode_thres = 4,   // 2^n
        .prbs_mode = ADI_APOLLO_SERDES_JRX_PRBS7,
    };

    uint8_t lanes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                       12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    uint32_t lanes_len = sizeof(lanes)/sizeof(lanes[0]);
    adi_apollo_serdes_prbs_checker_status_t prbs_status[24];

    adi_apollo_fpga_prbs_e lane_prbs[] = {
            PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7,
            PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7, PRBS_7 };
    uint32_t lane_prbs_len = sizeof(lane_prbs) / sizeof(lane_prbs[0]);

    /* Initialize the FPGA for PRBS testing */
    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_pattern_set(fpga_device, lane_prbs, lane_prbs_len);
    ADI_CMS_ERROR_RETURN(err);

    /* Start FPGA PRBS testing */
    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_start(fpga_device, false);
    ADI_CMS_ERROR_RETURN(err);
    adi_apollo_hal_delay_us(device, 2000000);

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

    err = adi_apollo_serdes_jrx_prbs_checker_enable(device, lanes, lanes_len, &prbs_checker);
    ADI_CMS_ERROR_RETURN(err);

    adi_apollo_hal_delay_us(device, 2000000);
    err = adi_apollo_serdes_jrx_prbs_checker_status(device, lanes, prbs_status, lanes_len);
    ADI_CMS_ERROR_RETURN(err);

    printf("\nExpect no JRx PRBS errors\n");
    for (i = 0; i < lanes_len; i++) {
        printf("Lane[%2d] PRBS Error Count: 0x%08x    Err Flag: %d\n", lanes[i], prbs_status[i].err_count, prbs_status[i].err_sticky);
        lanes_wo_err += prbs_status[i].err_sticky != 0 ? 0 : 1;
    }


    /* Start FPGA PRBS testing - FORCE FAILURE by inverting lanes data from FPGA */
    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_start(fpga_device, true);
    ADI_CMS_ERROR_RETURN(err);

    /* Small delay after start to flush data */
    adi_apollo_hal_delay_us(device, post_start_delay);

    err = adi_apollo_serdes_jrx_prbs_checker_enable(device, lanes, lanes_len, &prbs_checker);
    ADI_CMS_ERROR_RETURN(err);

    adi_apollo_hal_delay_us(device, 2000000);
    err = adi_apollo_serdes_jrx_prbs_checker_status(device, lanes, prbs_status, lanes_len);
    ADI_CMS_ERROR_RETURN(err);

    printf("\nExpect JRx PRBS errors on all lanes\n");
    lanes_wo_err = 0;
    for (i = 0; i < lanes_len; i++) {
        printf("Lane[%2d] PRBS Error Count: 0x%08x    Err Flag: %d\n", lanes[i], prbs_status[i].err_count, prbs_status[i].err_sticky);
        lanes_wo_err += prbs_status[i].err_sticky != 0 ? 1 : 0;
    }


    /* Start FPGA PRBS testing - CLEAR FAILURE by not inverting lanes data from FPGA */
    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_start(fpga_device, false);
    ADI_CMS_ERROR_RETURN(err);

    /* Small delay after start to flush data */
    adi_apollo_hal_delay_us(device, post_start_delay);

    err = adi_apollo_serdes_jrx_prbs_checker_enable(device, lanes, lanes_len, &prbs_checker);
    ADI_CMS_ERROR_RETURN(err);

    adi_apollo_hal_delay_us(device, 2000000);
    err = adi_apollo_serdes_jrx_prbs_checker_status(device, lanes, prbs_status, lanes_len);
    ADI_CMS_ERROR_RETURN(err);

    printf("\nExpect no JRx PRBS errors\n");
    lanes_wo_err = 0;
    for (i = 0; i < lanes_len; i++) {
        printf("Lane[%2d] PRBS Error Count: 0x%08x    Err Flag: %d\n", lanes[i], prbs_status[i].err_count, prbs_status[i].err_sticky);
        lanes_wo_err += prbs_status[i].err_sticky != 0 ? 0 : 1;
    }

    return API_CMS_ERROR_OK;
}

static int32_t spo_sweep_prbs(adi_apollo_device_t *device, adi_apollo_serdes_ck_spo_strobe_e strobe, adi_apollo_deser_rate_mode_e mode, uint8_t active_lanes)
{
	int32_t err;
	uint32_t i, sweep;

    uint8_t lanes[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
	                   12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
	uint32_t lanes_len = sizeof(lanes) / sizeof(lanes[0]);
	adi_apollo_serdes_prbs_checker_enable_t prbs_checker = {
		.enable = 1,
		.auto_mode = 0,
		.auto_mode_thres = 0, // 2^n
		.prbs_mode = ADI_APOLLO_SERDES_JRX_PRBS7,
	};
	adi_apollo_serdes_prbs_checker_status_t prbs_status[24];
	uint8_t ber[24] = { 0 };

	uint32_t delay_us = 500000;
    uint8_t lanes_wo_err = 0;

	/* SPO cannot be read out in 16nm, pump down more than allowed to zero out I and Q delay lines. */
	/* This is just for sanity, they should already be at 0 */
	err = adi_apollo_serdes_jrx_spo_dir_set(device, lanes, lanes_len, 0);
	ADI_CMS_ERROR_RETURN(err);

	for (sweep = 1; sweep < (ADI_APOLLO_SERDES_SPO_MAX + 2); sweep++) {
		err = adi_apollo_serdes_jrx_clock_strobe(device, lanes, lanes_len, ADI_APOLLO_SERDES_CK_SPO_ISTROBE);
		ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_serdes_jrx_clock_strobe(device, lanes, lanes_len, ADI_APOLLO_SERDES_CK_SPO_QSTROBE);
		ADI_CMS_ERROR_RETURN(err);
	}

	/* Setup and clear error checker */
	err = adi_apollo_serdes_jrx_prbs_checker_enable(device, lanes, lanes_len, &prbs_checker);
	ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_serdes_jrx_prbs_clear_error(device, lanes, lanes_len);
	ADI_CMS_ERROR_RETURN(err);

	/* Set the flash_mask if quarter rate mode, assumes inners are used. Essentially takes slice 1 offline for phase detection */
	if (mode == ADI_APOLLO_DESER_RATE_MODE_QUARTER_RATE) {
		err = adi_apollo_serdes_jrx_set_flash_mask(device, lanes, lanes_len, 0xA);
		ADI_CMS_ERROR_RETURN(err);
	}

	/* Sweep I/Q delay line and check for errors */
	err = adi_apollo_serdes_jrx_spo_dir_set(device, lanes, lanes_len, 1);
	ADI_CMS_ERROR_RETURN(err);

	for (sweep = 1; sweep < ADI_APOLLO_SERDES_SPO_MAX; sweep++) { /* We assume position 0 is error free here... */
		err = adi_apollo_serdes_jrx_clock_strobe(device, lanes, lanes_len, strobe);
		ADI_CMS_ERROR_RETURN(err);
		adi_apollo_hal_delay_us(device, delay_us);

        err = adi_apollo_serdes_jrx_prbs_checker_status(device, lanes, prbs_status, lanes_len);
		ADI_CMS_ERROR_RETURN(err);
		printf("\rSPO sweep #%d", sweep);
		fflush(stdout);

		/* update the ber[] based on error status */
		for (i = 0; i < lanes_len; i++) {
			if (prbs_status[i].err_sticky == 1) {
				//printf("Lane[%2d] PRBS Error Count: 0x%08x    Err Flag: %d\n", lanes[i], prbs_status[i].err_count, prbs_status[i].err_sticky);
			}
			else {
				ber[i] += 1;
			}
		}
	} /* end of sweep loop */
	printf("\n");

	/* Bring the delay line we swept back to 0, overshoot as it's bounds limited in hardware */
	err = adi_apollo_serdes_jrx_spo_dir_set(device, lanes, lanes_len, 0);
	ADI_CMS_ERROR_RETURN(err);

	for (sweep = 1; sweep < (ADI_APOLLO_SERDES_SPO_MAX + 2); sweep++) {
		err = adi_apollo_serdes_jrx_clock_strobe(device, lanes, lanes_len, strobe);
		ADI_CMS_ERROR_RETURN(err);
	}

	/* reset the flash mask, assumes inners are used. */
	if (mode == ADI_APOLLO_DESER_RATE_MODE_QUARTER_RATE) {
		err = adi_apollo_serdes_jrx_set_flash_mask(device, lanes, lanes_len, 0xF);
		ADI_CMS_ERROR_RETURN(err);
	}

	if (strobe == ADI_APOLLO_SERDES_CK_SPO_ISTROBE) {
		printf("SPO I-strobe sweep results:\n");
	}
	else {
		printf("SPO Q-strobe sweep results:\n");
	}

    /* NOTE: bigger the Error-free spo sweep count, means bigger the open eye. Max value per lane can be 32 */
	for (i = 0; i < lanes_len; i++) {
		printf("Lane[%02d] Error-free spo sweep count: %02d\n", i, ber[i]);
        lanes_wo_err += ber[i] != 0 ? 1 : 0;
	}

	return API_CMS_ERROR_OK;
}
