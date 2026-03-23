#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo null operation, used to test dp config after profile load
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
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

int32_t dp_load(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[])
{
    int32_t err = API_CMS_ERROR_OK;
    adi_apollo_cduc_inspect_t cduc_inspect[ADI_APOLLO_CDUC_NUM];
    adi_apollo_cnco_inspect_t tx_cnco_inspect[ADI_APOLLO_CNCO_NUM];
    adi_apollo_fduc_inspect_t fduc_inspect[ADI_APOLLO_FDUC_NUM];
    adi_apollo_fnco_inspect_t tx_fnco_inspect[ADI_APOLLO_FNCO_NUM];
    adi_apollo_txmisc_inspect_t txmisc_inspect[ADI_APOLLO_NUM_SIDES];

    adi_apollo_cddc_inspect_t cddc_inspect[ADI_APOLLO_CDDC_NUM];
    adi_apollo_cnco_inspect_t rx_cnco_inspect[ADI_APOLLO_CNCO_NUM];
    adi_apollo_fddc_inspect_t fddc_inspect[ADI_APOLLO_FDDC_NUM];
    adi_apollo_fnco_inspect_t rx_fnco_inspect[ADI_APOLLO_FDDC_NUM];

    /* Inspect TX Path */
    uint16_t cducs_to_inspect[] = { ADI_APOLLO_CDUC_A0, ADI_APOLLO_CDUC_A1,
                                     ADI_APOLLO_CDUC_B0, ADI_APOLLO_CDUC_B1 };
    uint16_t cncos_to_inspect[] = { ADI_APOLLO_CNCO_A0, ADI_APOLLO_CNCO_A1,
                                     ADI_APOLLO_CNCO_B0, ADI_APOLLO_CNCO_B1 };
    uint16_t fducs_to_inspect[] = { ADI_APOLLO_FDUC_A0, ADI_APOLLO_FDUC_A1, ADI_APOLLO_FDUC_A2, ADI_APOLLO_FDUC_A3,
                                     ADI_APOLLO_FDUC_B0, ADI_APOLLO_FDUC_B1, ADI_APOLLO_FDUC_B2, ADI_APOLLO_FDUC_B3 };
    uint16_t fncos_to_inspect[] = { ADI_APOLLO_FNCO_A0, ADI_APOLLO_FNCO_A1, ADI_APOLLO_FNCO_A2, ADI_APOLLO_FNCO_A3,
                                     ADI_APOLLO_FNCO_B0, ADI_APOLLO_FNCO_B1, ADI_APOLLO_FNCO_B2, ADI_APOLLO_FNCO_B3 };
    uint16_t txmisc_to_inspect[] = { ADI_APOLLO_SIDE_A, ADI_APOLLO_SIDE_B };


    uint16_t cddcs_to_inspect[] = { ADI_APOLLO_CDDC_A0, ADI_APOLLO_CDDC_A1,
                                      ADI_APOLLO_CDDC_B0, ADI_APOLLO_CDDC_B1 };
    uint16_t fddcs_to_inspect[] = { ADI_APOLLO_FDDC_A0, ADI_APOLLO_FDDC_A1, ADI_APOLLO_FDDC_A2, ADI_APOLLO_FDDC_A3,
                                    ADI_APOLLO_FDDC_B0, ADI_APOLLO_FDDC_B1, ADI_APOLLO_FDDC_B2, ADI_APOLLO_FDDC_B3 };


    /*
     * Oneshot sync - Synchronize all clocks
     *
    */
    if (err = adi_apollo_clk_mcs_oneshot_sync(device), err != API_CMS_ERROR_OK) {
        printf("Oneshot sync failure.\n");
        goto end;
    }

    /* Link inspections */
    printf("\nApollo JRx link\n");
    adi_ads10_apollo_ex_inspect_jrx_link_all(device);

    printf("\nApollo JTx link\n");
    adi_ads10_apollo_ex_inspect_jtx_link_all(device);

    /* Tx inspections */
    for (int i = 0; i < 4; i++) {
        adi_apollo_cduc_inspect(device, cducs_to_inspect[i], &cduc_inspect[i]);
        adi_apollo_cnco_inspect(device, ADI_APOLLO_TX, cncos_to_inspect[i], &tx_cnco_inspect[i]);
    }

    for (int i = 0; i < 8; i++) {
        adi_apollo_fduc_inspect(device, fducs_to_inspect[i], &fduc_inspect[i]);
        adi_apollo_fnco_inspect(device, ADI_APOLLO_TX, fncos_to_inspect[i], &tx_fnco_inspect[i]);
    }

    for (int i = 0; i < 2; i++) {
        adi_apollo_txmisc_inspect(device, txmisc_to_inspect[i], &txmisc_inspect[i]);
    }

    /* Rx inspections */
    for (int i = 0; i < 4; i++) {
        adi_apollo_cddc_inspect(device, cddcs_to_inspect[i], &cddc_inspect[i]);
        adi_apollo_cnco_inspect(device, ADI_APOLLO_RX, cncos_to_inspect[i], &rx_cnco_inspect[i]);
    }

    for (int i = 0; i < 8; i++) {
        adi_apollo_fddc_inspect(device, fddcs_to_inspect[i], &fddc_inspect[i]);
        adi_apollo_fnco_inspect(device, ADI_APOLLO_RX, fncos_to_inspect[i], &rx_fnco_inspect[i]);
    }

end:
    return err;
}
#endif /* !defined(VERSAL_PLATFORM) */
