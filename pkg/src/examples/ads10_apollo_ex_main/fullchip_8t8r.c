#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo fullchip 8T8R loopback
 *
 * Profile id98_uc05
 *      8G/8G, 8T/8R dual band
 *
 *      All DACs looped back to ADCs
 *
 *      DAC             Output(MHz) (CNCO=960, FNCO=160)    BaseBand
 *      -------------   --------------------------------    --------
 *      DAC-A0/DAC-B0   1170 & 1165                         50 & 45
 *      DAC-A1/DAC-B1   1160 & 1155                         40 & 35
 *      DAC-A2/DAC-B2   1150 & 1145                         30 & 25
 *      DAC-A3/DAC-B3   1140 & 1135                         20 & 15
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
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_fpga_apollo_transmit.h"
#include "adi_ads10_apollo_ex_vec.h"
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_fpga_apollo_capture.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_inspect.h"

static int32_t tx_dp_reset(adi_apollo_device_t* device);

int32_t fullchip_8t8r(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    double tone_ratio_base = 0.4;         // Percent of JESD data rate side A (0.04 => 50 @1250Gsps)
    char tmu_str[256];
    bool interleaved = 1;               /* 1 = interleave data, 0 = separate I/Q files */
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;       /* min num samples per virt conv */
    char* cap_fname_base = "fullchip_8t8r";
    adi_apollo_device_tmu_data_t tmu_data;
    adi_ads10_apollo_dp_info_t rx_dp_info, tx_dp_info;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];

    /*
     * Get the FPGA features supported (such as HW transport layer and FSRC)
     */
    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);


    /* Datapath (M)   DAC   i or q   Tone (MHz)    FDUC
     * ------------  -----  ------  ------------  ------
     *      0        A0/B0    i          50        A0/B0
     *      1        A0/B0    q          50        A0/B0
     *      2        A0/B0    i          45        A4/B4
     *      3        A0/B0    q          45        A4/B4
     *      4        A1/B1    i          40        A1/B1
     *      5        A1/B1    q          40        A1/B1
     *      6        A1/B1    i          35        A5/B5
     *      7        A1/B1    q          35        A5/B5
     *      8        A2/B2    i          30        A2/B2
     *      9        A2/B2    q          30        A2/B2
     *      10       A2/B2    i          25        A6/B6
     *      11       A2/B2    q          25        A6/B6
     *      12       A3/B3    i          20        A3/B3
     *      13       A3/B3    q          20        A3/B3
     *      14       A3/B3    i          15        A7/B7
     *      15       A3/B3    q          15        A7/B7
     */

    /*
     * Enable the fduc gain block for all FDUCs.
     * Set each gain to 0.5. (note gain value of 1.0 is 0x800, 0.5 is 0x400)
     * Must do DP reset after adi_apollo_fduc_subdp_gain_enable() (not necessary for subsequent calls to adi_apollo_fduc_subdp_gain_set())
     * 0.5 is a -6dB change
     *
     * If this FDUC attenuation is not set, then summing FDUCs together results in clipping/distortion. FFT will look really bad!)
    */
    adi_apollo_fduc_subdp_gain_enable(device, ADI_APOLLO_FDUC_ALL, 1);
    adi_apollo_fduc_subdp_gain_set(device, ADI_APOLLO_FDUC_ALL, 0x800>>1); // 0.5 gain reduction if summing two fducs
    tx_dp_reset(device);

    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Rx Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    /* Get Tx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
    adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Tx Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);


    /* Inspect the Apollo JRx and JTx link config */
    err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_inspect_jtx_link_all(device);
    ADI_CMS_ERROR_RETURN(err);

    tone_ratio_base = tone_ratio_base * (rx_dp_info.fdata / tx_dp_info.fdata);  // If rx baseband rate is less than tx, adjust to avoid alias

    /* Override Profile - Set TX FNCO/CNCO to match RX - Tone will land at baseband freq in FFT when adc-dac are loop backed */
    adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, tx_dp_info.dac_sample_rate, rx_dp_info.cnco_freq);
    adi_ads10_apollo_ex_fnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_ALL, tx_dp_info.dac_sample_rate/tx_dp_info.cdrc, rx_dp_info.fnco_freq);

    /* Run cals */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_vec_cmplx_tone_incr_write(fpga_device, profile, NULL, ADI_APOLLO_LINK_ALL, num_samples, tone_ratio_base, -tone_ratio_base/10.0, -1.0);
    ADI_CMS_ERROR_RETURN(err);

    /*** ADS10 FPGA simultaneous Rx/Tx link startup ***/
    err = adi_fpga_apollo_core_bidir_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /*
    * Run the SERDES calibration
    * Unfreeze ADC background cal (Should do this after ADC and SERDES init cals)
    *
    * NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly
    */
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG| ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
    ADI_CMS_ERROR_RETURN(err);

    /* Read device temperature */
    adi_apollo_device_tmu_get(device, &tmu_data);
    adi_ads10_ex_tmu_data_to_str(&tmu_data, tmu_str, sizeof(tmu_str));
    printf("%s\n", tmu_str);

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, true, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    printf("Done with capture\n");

    return err;
}


static int32_t tx_dp_reset(adi_apollo_device_t* device)
{
    int32_t err = API_CMS_ERROR_OK;

    err |= adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 0);
    err |= adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 1);
    err |= adi_apollo_txmisc_dp_reset(device, ADI_APOLLO_SIDE_ALL, 0);
    return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
