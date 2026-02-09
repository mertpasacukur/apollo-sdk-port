/*!
 * \brief     ADS10 Apollo fullchip Rx/Tx FSRC data path test using ADC as input source
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
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_fpga_apollo_fsrc.h"
#include "adi_fpga_apollo_core.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t fullchip(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    double tone_ratio = 0.40;         // Percent of JESD data rate side A (0.4 => 500 @1250Gsps)
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;   /* min num samples per virt conv */
    char* cap_fname_base = "fullchip";
    uint32_t vec_len = DEFAULT_NUM_SAMPLES_H;
    double tone_freq, adc_input_freq = 2300;
    adi_ads10_apollo_dp_info_t tx_dp_info;
    adi_ads10_apollo_dp_info_t rx_dp_info;
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];

    err = adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Profile Tx", str_buff, str_buff_len);
    ADI_CMS_ERROR_RETURN(err);
    printf("\n%s\n\n", str_buff);

    err = adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Profile Rx", str_buff, str_buff_len);
    ADI_CMS_ERROR_RETURN(err);
    printf("\n%s\n\n", str_buff);

    /* Inspect the Apollo JRx and JTx link config */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, vec_len, tone_ratio, -1.0);
    ADI_CMS_ERROR_RETURN(err);

    /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG | ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_apollo_jrx_rm_fifo_reset(device, ADI_APOLLO_LINK_ALL);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_tx_freq_get(device, profile, tone_ratio, &tone_freq);
    ADI_CMS_ERROR_RETURN(err)

    EXCTL_SIGGEN_ON(0x33, false);
    EXCTL_SIGGEN_LEVEL(3.0, false);
    EXCTL_SIGGEN_FREQ(adc_input_freq, interactive);

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, true);
    ADI_CMS_ERROR_RETURN(err);

#ifdef EXCTL_AUTOMATION
    EXCTL_RX_MEAS_FREQ(0x33, cap_fname_base, adc_input_freq, 10);
#else
    adi_ads10_apollo_rx_channel_info_t channel;
    adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, &channel);
    EXCTL_RX_MEAS_FREQ(0x33, cap_fname_base, (adc_input_freq - channel.nco_freq_hz/1e6), 10);
#endif

    EXCTL_TX_MEAS_FREQ(0x33, tone_freq, 10, interactive);

    return err;
}
