#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     ADS10 Apollo load vector from file example
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
 #include "adi_utils.h"
 #include "adi_fpga_apollo_core.h"
 #include "adi_ads10_apollo_ex.h"
 #include "adi_ads10_apollo_ex_fpga.h"
 #include "adi_ads10_apollo_ex_to_str.h"
 #include "adi_ads10_apollo_ex_inspect.h"
 #include "adi_ads10_apollo_ex_vec.h"
 #include "adi_ads10_apollo_extras.h"
 #include "adi_ads10_apollo_ex_cal.h"
 #include "adi_ads10_apollo_ex_ctl.h"
 #include "adi_vector.h"
 
 int32_t tx_jesd_file(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
 {
     int32_t err;
     const int str_buff_len = 1024;
     char str_buff[str_buff_len];
     adi_ads10_apollo_dp_info_t tx_dp_info;
     char* i_file_name;
     char* q_file_name;
     double nco_ovr;
     double tone_ratio = 0.4;        // tone to data rate ratio (Nyquist is 0.5)
     double tone_freq;
     int n_loc_args = argc - argc_ofst;
 
     uint8_t is_auto = (n_loc_args > 0) && !strcmp("-a", argv[argc_ofst]);
     uint8_t is_file = (n_loc_args > 2) && !strcmp("-f", argv[argc_ofst]);   // -f option requires 2 files
     uint8_t is_nco_ovr = 0;
     if (is_auto) {
         i_file_name = "i_vec.txt";
         q_file_name = "q_vec.txt";
         printf("Auto generating I/Q files (%s, %s)...\n", i_file_name, q_file_name);
     } else if (is_file) {
         i_file_name = argv[argc_ofst + 1];
         q_file_name = argv[argc_ofst + 2];
         if (n_loc_args > 3) {
             nco_ovr = atof(argv[argc_ofst + 3]);        // optional CNCO override
             is_nco_ovr = 1;
         }
         printf("Reading I/Q files (%s, %s)...\n", i_file_name, q_file_name);
     } else {
         printf("Invalid command line, no -a or -f files specified\n");
         return API_CMS_ERROR_INVALID_PARAM;
     }
 
     /* If no I/Q vector files were specified, create a pair with simple tone freq equal to Fdata * .4 */
     if (is_auto) {
         FILE *i_fp, *q_fp;
         uint32_t n_samples = 1024 * 64;
 
         /* Adjust vec_len to be a multiple of JESD S param */
         n_samples = (n_samples * (profile->jrx[0].rx_link_cfg->s_minus1 + 1)) / adi_api_utils_gcd(n_samples, (profile->jrx[0].rx_link_cfg->s_minus1 + 1));
 
         int16_t* i_vec = (int16_t *)malloc(n_samples * sizeof(int16_t));
         int16_t* q_vec = (int16_t *)malloc(n_samples * sizeof(int16_t));
         adi_vector_sine_config_t sine_config = {
             .backoff = -1,
             .phase = 90,
             .ratio = tone_ratio,
             .resolution = profile->jrx[0].rx_link_cfg[0].np_minus1 + 1
         };
 
         i_fp = fopen(i_file_name, "w+");
         q_fp = fopen(q_file_name, "w+");
 
         adi_vector_generate_sine(i_vec, n_samples, &sine_config);
         sine_config.phase -= 90;
         adi_vector_generate_sine(q_vec, n_samples, &sine_config);
 
         for (int i = 0; i < n_samples; i++) {
             fprintf(i_fp, "%d\n", i_vec[i]);
             fprintf(q_fp, "%d\n", q_vec[i]);
         }
 
         fclose(i_fp);
         fclose(q_fp);
         free(i_vec);
         free(q_vec);
     }
 
    err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_CC);
    ADI_CMS_ERROR_RETURN(err);

     /* Get Tx profile info for a channel*/
     adi_ads10_ex_dp_info_get(device, profile, ADI_APOLLO_TX, ADI_APOLLO_SIDE_IDX_A, 0, &tx_dp_info);
     adi_ads10_ex_dp_info_to_str(&tx_dp_info, "Tx Base Profile", str_buff, str_buff_len);
     printf("\n%s\n\n", str_buff);
 
     /* Inspect the JRx link states */
     err = adi_ads10_apollo_ex_inspect_jrx_link_all(device);
     ADI_CMS_ERROR_RETURN(err);
 
     // If NCO override from profile
     if (is_nco_ovr) {
         printf("Overriding the profile Tx-CNCO to %fMHz...\n", nco_ovr);
         adi_ads10_apollo_ex_cnco_set(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_ALL, tx_dp_info.dac_sample_rate, nco_ovr);
     }
 
     err = adi_ads10_apollo_ex_fpga_fsrc_init(fpga_device, profile, ADI_APOLLO_TX);
     ADI_CMS_ERROR_RETURN(err);
 
     err = adi_ads10_apollo_ex_vec_files_write(fpga_device, profile, ADI_APOLLO_SIDE_ALL, 1024*1024, i_file_name, q_file_name, 1.0);
     ADI_CMS_ERROR_RETURN(err);
 
     /* Transmit data out from FPGA JTx */
     err = adi_fpga_apollo_core_tx_links_init(fpga_device);
     ADI_CMS_ERROR_RETURN(err); 
 
     /*
     * Run the serdes calibration
     *
     * NOTE: The FPGA must be transmitting (any data) for serdes cal to function properly
     */
     err = adi_ads10_apollo_ex_cals_run(device, profile, ADI_ADS10_APOLLO_CAL_SERDES | ADI_ADS10_APOLLO_CAL_SERDES_BG);
     ADI_CMS_ERROR_RETURN(err);
     
     err = adi_ads10_apollo_ex_inspect_tx_freq_get(device, profile, tone_ratio, &tone_freq);
     ADI_CMS_ERROR_RETURN(err)
 
     EXCTL_TX_MEAS_FREQ(0x33, tone_freq, 10, true);
 
     return err;
 }

#endif /* !defined(VERSAL_PLATFORM) */
