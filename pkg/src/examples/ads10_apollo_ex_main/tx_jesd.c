/*!
 * \brief     ADS10 Apollo TX JESD FSRC data path test
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
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t tx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile,
                     int argc, char *argv[], int argc_ofst) {

    int32_t err;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    adi_ads10_apollo_dp_info_t tx_dp_info;
    double tone_ratio = 0.4;         // Percent of JESD data rate side A (0.4 => 500 @1250Gsps)
    uint32_t vec_len = DEFAULT_NUM_SAMPLES_H;
    double tone_freq;

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

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

    err = adi_ads10_apollo_ex_inspect_tx_freq_get(device, profile, tone_ratio, &tone_freq);
    ADI_CMS_ERROR_RETURN(err)

    EXCTL_TX_MEAS_FREQ(0x33, tone_freq, 10, interactive);

    return err;
}
