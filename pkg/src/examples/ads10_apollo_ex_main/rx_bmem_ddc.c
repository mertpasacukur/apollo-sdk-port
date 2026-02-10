/*!
 * \brief     ADS10 Apollo Rx DDC data path test using BMEM-AWG as input source
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
#include "adi_ads10_apollo_extras.h"
#include "adi_ads10_apollo_ex_to_str.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_ads10_apollo_ex_bmem.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_fpga_apollo_core.h"

int32_t rx_bmem_ddc(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst)
{
    int32_t err = API_CMS_ERROR_OK;
    char file_name_base[256];
    const int str_buff_len = 1024;
    char str_buff[str_buff_len];
    double tone_ratio = 0.115;
    adi_ads10_apollo_dp_info_t rx_dp_info;
    uint16_t bmem_sel = ADI_APOLLO_BMEM_A0 | ADI_APOLLO_BMEM_A1 | ADI_APOLLO_BMEM_B0 | ADI_APOLLO_BMEM_B1; // BMEM-AWG selection, each instance represents an ADC
    bool interleaved = 1;
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;

    err = adi_ads10_apollo_ex_bmem_awg_config(device, bmem_sel);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

    /* Get Rx profile info. Will assume all chans match A0 (side-A, chan 0) */
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, ADI_APOLLO_SIDE_IDX_A, 0, &rx_dp_info);
    adi_ads10_ex_dp_info_to_str(&rx_dp_info, "Base Profile", str_buff, str_buff_len);
    printf("\n%s\n\n", str_buff);

    // Bring up the links
    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /* Use BMEM to inject a real tone into the Rx data path. Capture data with FPGA */
    printf("Create/load %fMHz tone into BMEM...\n", tone_ratio * rx_dp_info.fclk);
    err = adi_ads10_apollo_ex_bmem_awg_tone_write(device, bmem_sel, tone_ratio, -6.0, false);
    ADI_CMS_ERROR_RETURN(err);

    // Measure the fundamental power for the following conditions.
    //
    //        Fixed given uc06              Gain enables              Fund
    //        --------------------------    ---------------------    -----
    //         Input    CDDC    FDDC        CDDC HB1     FDCC HB1    Total
    //         =====    ====    ====        ========     ========    =====
    //          -6       -6       0             0           0         -12
    //          -6       -6       0             0          +6          -6
    //          -6       -6       0            +6           0          -6
    //          -6       -6       0            +6          +6           0
    //
    //

    // CDDC gain = 0, FDDC gain = 0
    adi_apollo_cddc_gain_enable_set(device, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_CDDC_GAIN_TB1, 0);
    adi_apollo_cddc_gain_enable_set(device, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_CDDC_GAIN_HB1, 0);
    adi_apollo_fddc_gain_enable_set(device, ADI_APOLLO_FDDC_ALL, 0);
    /* Read FPGA capture memory and write out i/q files */
    sprintf(file_name_base, "%s", "apollo_bmem_ddc_gain_c0_f0");
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    EXCTL_RX_REF(bmem_sel, file_name_base);

    // CDDC gain = 0, FDDC gain = +6
    adi_apollo_cddc_gain_enable_set(device, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_CDDC_GAIN_HB1, 0);
    adi_apollo_fddc_gain_enable_set(device, ADI_APOLLO_FDDC_ALL, 1);
    sprintf(file_name_base, "%s", "apollo_bmem_ddc_gain_c0_f6");
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    EXCTL_RX_MEAS_LEVEL(bmem_sel, file_name_base, 6, 1);

    // CDDC gain = +6, FDDC gain = 0
    adi_apollo_cddc_gain_enable_set(device, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_CDDC_GAIN_HB1, 1);
    adi_apollo_fddc_gain_enable_set(device, ADI_APOLLO_FDDC_ALL, 0);
    sprintf(file_name_base, "%s", "apollo_bmem_ddc_gain_c6_f0");
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    EXCTL_RX_MEAS_LEVEL(bmem_sel, file_name_base, 6, 1);

    // CDDC gain = +6, FDDC gain = +6
    // In practice, you typically wouldn't enable both cddc and fddc gains.
    adi_apollo_cddc_gain_enable_set(device, ADI_APOLLO_CDDC_ALL, ADI_APOLLO_CDDC_GAIN_HB1, 1);
    adi_apollo_fddc_gain_enable_set(device, ADI_APOLLO_FDDC_ALL, 1);
    sprintf(file_name_base, "%s", "apollo_bmem_ddc_gain_c6_f6");
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, file_name_base, interleaved);
    ADI_CMS_ERROR_RETURN(err);

    EXCTL_RX_MEAS_LEVEL(bmem_sel, file_name_base, 12, 1);

    return err;
}
