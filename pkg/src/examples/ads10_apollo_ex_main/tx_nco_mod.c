#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo Tx data path NCO test
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
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
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_utils.h"

int32_t tx_nco_mod(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* CNCO only */
    adi_apollo_cnco_mode_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, ADI_APOLLO_MXR_TEST_MODE);        /* CNCO to test mode, blocks FNCO data */

    adi_ads10_apollo_ex_cnco_freq_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, false, 10, 1);
    printf("Measure freq - modulus disabled\n"); 
    if (interactive) {
        getchar();
    } 

    adi_ads10_apollo_ex_cnco_freq_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, true, 10, 1);
    printf("Measure freq - modulus enabled\n"); 
    if (interactive) {
        getchar();
    }
    
    return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
