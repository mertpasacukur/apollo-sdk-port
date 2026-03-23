#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo Rx Nyquist zone test using ADC inputs and BMEM capture
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
#include "adi_ads10_apollo_extras.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_types.h"
#include "ads10_fpga.h"
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_ads10_apollo_ex_bmem.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t rx_adc_nz(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{

	int32_t err = API_CMS_ERROR_OK;
    bool interactive = (argc > argc_ofst) && !strncmp("-i", argv[argc_ofst], 2); // default not interactive
    uint32_t bgcal_dly = 10 * 1000000;
    adi_apollo_device_tmu_data_t tmu_data;
    uint16_t adc_cal_chans = ADI_APOLLO_ADC_A0;

    uint32_t nz;
    uint32_t nz_get;
    double fclk_div_2;
    double fin;
    double fspur;
    int n_loops;
    adi_ads10_apollo_dp_info_t rx_dp_info;
    adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_RX, 0, 0, &rx_dp_info);
    char fname[MAX_PATH_LEN];

    /*
     * Configure BMEM capture
    */
    adi_apollo_bmem_capture_t bmem_config = {
        .sample_size = 0,
        .ramclk_ph_dis = 0,
        .st_addr_cpt = 0,
        .end_addr_cpt = ADI_APOLLO_BMEM_HSDIN_MEM_SIZE_WORDS - 1,
        .parity_check_en = 1
    };
    
    err = adi_apollo_bmem_hsdin_capture_config(device, ADI_APOLLO_BMEM_A0, &bmem_config);
    ADI_CMS_ERROR_RETURN(err);

    adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC | ADI_ADS10_APOLLO_CAL_ADC);
    ADI_CMS_ERROR_RETURN(err);    

    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_ADC_BG);
    ADI_CMS_ERROR_RETURN(err);

    /*
     * The DDR spur is located at (Fs/2 - Fin)
     *
     * For id00_uc06_T:
     *  Fclk = 20000
     *
     *  DDR spur location: 10000 - 5400.1 = 4599.9
     *
    */

    /* Setup to put spur in first Nyquist */
    fclk_div_2 = rx_dp_info.fclk / 2;
    fin = 5400.1;
    fspur = fclk_div_2 - fin;
    n_loops = 2;
    printf("DDR spur (fs/2 - fin) = (%f - %f) = %f\n", fclk_div_2, fin, fspur);

    printf("@CMD:SOURCE:FREQ %f\n", fin);
    EXCTL_SIGGEN_FREQ(fin, interactive);

    for (int i = 0; i < n_loops; i++) {
        nz = (i % 2) + 1;

        // 1 = first nyquist, 2 = second nyquist
        printf("NZ = %d\n", nz);
        err = adi_apollo_adc_nyquist_zone_set(device, adc_cal_chans, nz);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_apollo_adc_nyquist_zone_get(device, ADI_APOLLO_ADC_A0, &nz_get);
        ADI_CMS_ERROR_RETURN(err);
        printf("nz_get = %d\n", nz_get);

        /* Check for expected nz val from FW */
        if (nz != nz_get) {
            printf("Error: expected nz = %d but read %d\n", nz, nz_get);
            return API_CMS_ERROR_ERROR;
        }

        /* Let cal run a bit to lower noise floor */
        printf("Let bgcal run for %4.1f secs...\n", (bgcal_dly / 1000000.0)*3);
        adi_apollo_hal_delay_us(device, bgcal_dly*3);

        snprintf(fname, MAX_PATH_LEN, "rx_adc_nz_%d", i);

        err = adi_ads10_apollo_ex_bmem_capture(device, ADI_APOLLO_BMEM_A0, fname);
        ADI_CMS_ERROR_RETURN(err);
    }

    /*
     * Read device temperature sensors.
    */
    adi_apollo_device_tmu_get(device, &tmu_data);
    printf("TMU (deg C): %d %d %d %d %d %d %d %d \n",
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_SERDES_PLL],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_MPU_A],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_MPU_B],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_ADC_A],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_A],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_ADC_B],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_B],
        tmu_data.temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_C]
    );

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
