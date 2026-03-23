/*!
 * \brief     ADS10 Apollo Rx JESD capture example. CLI flag -b to inject a bmem tone
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
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_fpga_apollo_capture.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_ads10_apollo_ex_bmem.h"
#include "adi_ads10_apollo_ex_fpga.h"

int32_t rx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
	int32_t err = API_CMS_ERROR_OK;
    char msg[256];
    double f_adc_mhz;                   // ADC sampling rate
    double f_adc_in_tone;               // ADC input tone freq (real)
    double f_adc_tone_ratio_out;        // The coherent, actual tone ratio used
    double f_out_mhz;                   // Baseband output tone, after Rx datapath nco shift 
    double f_fnco_mhz;                  // Fine nco freq
    double f_cnco_mhz;                  // Coarse nco freq
    uint32_t fddc_dcm, cddc_dcm;        // Coarse and fine decimation
    double fsrc_ratio;                  // FSRC ratio
    double f_data_rate;                 // Effective FSRC data rate (cddc_dcm*fddc_dcm*fsrc_ratio)
    uint16_t total_decimation;          // Link path decimation (doesn't include fsrc ratio)
    int fsrc_n;                         // FSRC N   (fsrc ratio = N/M)
    int fsrc_m;                         // FSRC M   (fsrc ratio = N/M)
    uint32_t num_samples = DEFAULT_NUM_SAMPLES_H;       /* min num samples per virt conv */
    char * cap_fname_base = "rx_jesd";
    bool bmem_awg = (argc > argc_ofst) && !strncmp("-b", argv[argc_ofst], 2); // default not bmem
    uint16_t bmem_sel = ADI_APOLLO_BMEM_A0 | ADI_APOLLO_BMEM_A1 | ADI_APOLLO_BMEM_B0 | ADI_APOLLO_BMEM_B1;

    /* Get ADC sampling freq from profile */
    f_adc_mhz = profile->adc_config->adc_sampling_rate_Hz / 1e6;

    /*
     * Determine the coarse and fine freq shifts from profile. Assume chans are the same.
    */
    f_out_mhz = -204.0;             // Expect location on input tone after Rx datapath nco shift
    adi_apollo_cddc_dcm_bf_to_val(device, profile->rx_path[0].rx_cddc[0].drc_ratio, &cddc_dcm);
    adi_apollo_fddc_dcm_bf_to_val(device, profile->rx_path[0].rx_fddc[0].drc_ratio, &fddc_dcm);
    f_cnco_mhz = (profile->rx_path[0].rx_cddc[0].nco[0].nco_phase_inc / (double)(1ull << 32)) * f_adc_mhz;
    f_fnco_mhz = (profile->rx_path[0].rx_fddc[0].nco[0].nco_phase_inc / (double)(1ull << 48)) * (f_adc_mhz / cddc_dcm);
    total_decimation = profile->jtx[0].tx_link_cfg[0].link_dp_ratio;
    fsrc_ratio = adi_ads10_apollo_ex_fsrc_ratio_get(&profile->rx_path[0].rx_fsrc[0], &fsrc_n, &fsrc_m);
    f_data_rate = f_adc_mhz / (total_decimation * fsrc_ratio);
    f_adc_in_tone = f_out_mhz + f_cnco_mhz + f_fnco_mhz;

    printf("%-12s %-7s %-5s %-11s %-11s %-4s %-21s %-8s %-8s\n", "Profile     ", "Fdac   ", "PFILT", "CDUC       ", "FDUC       ", "CFIR", "FSRC     ", "Fdata   ", "JESD DR ");
    printf("%-12s %-7s %-5s %-11s %-11s %-4s %-21s %-8s %-8s\n", "============", "=======", "=====", "===========", "===========", "====", "=====================", "========", "========");
    printf("%-12s %7.1f %-5s %2d:%-8.3f %2d:%-8.3f %-4s %-5d/%-5d = %-6.5f %-8.3f %-8.3f\n",
        argv[2], f_adc_mhz, profile->rx_path[0].rx_pfilt[0].enable ? "Y" : "N", cddc_dcm, f_cnco_mhz, fddc_dcm, f_fnco_mhz,
        profile->rx_path[0].rx_cfir[0].enable ? "Y" : "N", fsrc_n, fsrc_m, fsrc_ratio,
        f_data_rate, f_adc_mhz/(cddc_dcm*fddc_dcm));

    printf("\n");

    if (bmem_awg) {
        err = adi_ads10_apollo_ex_bmem_awg_config(device, bmem_sel);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_ads10_apollo_ex_bmem_awg_tone_write_v2(device, bmem_sel, f_adc_in_tone/f_adc_mhz, 0, false, &f_adc_tone_ratio_out);
        ADI_CMS_ERROR_RETURN(err);

        if (fabs(f_adc_in_tone - (f_adc_tone_ratio_out * f_adc_mhz)) > 0.5) {
            sprintf(msg, "Requested BMEM tone %14.3f but got %14.3f difference > 0.5MHz\n", f_adc_in_tone, (f_adc_tone_ratio_out * f_adc_mhz));
            EXCTL_FAIL(msg);
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
        }

        f_adc_in_tone = f_adc_tone_ratio_out * f_adc_mhz;
        f_out_mhz = f_adc_in_tone - f_cnco_mhz - f_fnco_mhz;
        printf("ADC input tone    JTx out tone  \n");
        printf("==============    ==============\n");
        printf("%14.3f    %14.3f\n", f_adc_in_tone, f_out_mhz);
    }
    else {
        printf("ADC input tone    JTx out tone  \n");
        printf("==============    ==============\n");
        printf("%14.3f    %14.3f\n", f_adc_in_tone, f_out_mhz);
        err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC | ADI_ADS10_APOLLO_CAL_ADC_BG);
        ADI_CMS_ERROR_RETURN(err);
        
        EXCTL_SIGGEN_FREQ(f_adc_in_tone, false);
        EXCTL_SIGGEN_LEVEL(6, false);
        EXCTL_SIGGEN_ON(0x33, true);
    }

    // Bring up the links
    err = adi_fpga_apollo_core_rx_links_init(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    /* Read FPGA capture memory and write out i/q files */
    err = adi_ads10_apollo_ex_fpga_capture(device, profile, fpga_device, num_samples, cap_fname_base, true, true);
    ADI_CMS_ERROR_RETURN(err);

	EXCTL_RX_MEAS_FREQ(0x33, cap_fname_base, f_adc_in_tone, 10);
    EXCTL_RX_MEAS_NSD(0x33, cap_fname_base, bmem_awg ? -160 : -145, bmem_awg ? 7.5 : 3); // C# analysis library has issues with uc08a/uc08a1 bmem NSD

	return err;
}
