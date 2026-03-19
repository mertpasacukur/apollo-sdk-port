/*!
 * \brief     ADS10 Apollo Jesd Loopback
 *
 * Expect the capture files to contain interleaved constants
 *
 * Sample:
 *     A0/B0    A1/B1
 *       1        3
 *       2        4
 *       1        3
 *       2        4
 *       1        3
 *       2        4
 *
 * The profile must be configured so that the same FDUCs/FDDCs are enabled and the JTx/JRx sample xbars match
 * (sample xbars can be changed at runtime)
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
#include "adi_apollo.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_capture.h"
#include "adi_fpga_apollo_transmit.h"
#include "adi_ads10_apollo_ex_cal.h"

int32_t jesd_loopback(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interleaved = 1;               /* 1 = interleave data, 0 = separate I/Q files */
    uint32_t num_samples = 64 * 1024;       /* min num samples per virt conv */
    char* cap_fname_base = "jesd_loopback";

    // JTx and JRx sample xbars must match
    // adi_apollo_jesd_dfrm_sample_xbar_select_e sample_xbar[] = {2,3};
    // err = adi_apollo_rxmux_sample_xbar_set(device, ADI_APOLLO_LINK_A0 | ADI_APOLLO_LINK_B0, 2, sample_xbar, 2);
    // ADI_CMS_ERROR_RETURN(err);

    // Enable loopback3
    err = adi_apollo_loopback_jesd_enable_set(device, ADI_APOLLO_SIDE_ALL, 1);
    ADI_CMS_ERROR_RETURN(err);

    /* Clock conditioning calibration */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_vec_constants_incr_write(fpga_device, profile, NULL, ADI_APOLLO_LINK_ALL, 128, 1, 1);
    ADI_CMS_ERROR_RETURN(err);

    // Dynamic Sync Serdes Links gradually in a sequence
    printf("Run Rx-TX SerDes Links Dyn Sync...\n");
    err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(device);
    ADI_CMS_ERROR_RETURN(err);

    // Allow clks and supplies to settle after sync
    err = adi_apollo_hal_delay_us(device, 1000);
    ADI_CMS_ERROR_RETURN(err);

    /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /*
    * Run the SERDES calibration
    *
    * NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly
    */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, true, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    printf("Done with capture\n");

    return err;
}
