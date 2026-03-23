#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo examples common FPGA functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include "adi_apollo.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex.h"

int32_t adi_ads10_apollo_ex_fpga_profile_init(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);

    /* Configure the ADS10 FPGA JESD Links */
    err = adi_ads10_apollo_ex_fpga_jesd_configure(fpga_device, profile->jtx, profile->jrx);
    ADI_CMS_ERROR_RETURN(err);

    /* Init ADS10 FPGA FSRC */
    err = adi_ads10_apollo_ex_fpga_fsrc_init(fpga_device, profile, ADI_APOLLO_TX);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_fpga_fsrc_init(fpga_device, profile, ADI_APOLLO_RX);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_fpga_fsrc_init(adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, uint32_t terminal)
{
    int32_t err;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl
    adi_apollo_fsrc_cfg_t fsrc_cfg;
    int fsrc_n, fsrc_m;
    uint32_t link_total_ratio, link_dp_ratio;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(profile);
    ADI_CMS_RANGE_CHECK(terminal, ADI_APOLLO_RX, ADI_APOLLO_TX);

    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    if (terminal == ADI_APOLLO_TX) {

        printf("FPGA TX FSRC:   %s\n", fpga_feature_flags.tx_hw_fsrc ? "HW" : "SW");
        printf("FPGA DAC TL:    %s\n", fpga_feature_flags.tx_hw_tl ? "HW" : "SW");

        /* Enable FPGA HW FSRC if supported by the FPGA image */
        err = adi_fpga_apollo_hw_fsrc_tx_enable_set(fpga_device, fpga_feature_flags.tx_hw_fsrc != 0);
        ADI_CMS_ERROR_RETURN(err);

        /* Set FSRC N / M for vector processing */
        fsrc_cfg = profile->tx_path[0].tx_fsrc[0];
        link_total_ratio = profile->jtx[0].tx_link_cfg[0].link_total_ratio;
        link_dp_ratio = profile->jtx[0].tx_link_cfg[0].link_dp_ratio;
        err = adi_fpga_apollo_fsrc_invalid_ratio_set(fpga_device, ADI_FPGA_APOLLO_JTX, ADI_FPGA_APOLLO_LINK_ALL,
                                                     fpga_feature_flags.tx_hw_fsrc ? false : fsrc_cfg.enable,
                                                     fsrc_cfg.fsrc_rate_int, fsrc_cfg.fsrc_rate_frac_a, fsrc_cfg.fsrc_rate_frac_b,
                                                     link_dp_ratio, link_total_ratio);
        ADI_CMS_ERROR_RETURN(err);

        if (fpga_feature_flags.tx_hw_fsrc) {

            /* Determine the n/m values from the profile */
            adi_ads10_apollo_ex_fsrc_ratio_get(&fsrc_cfg, &fsrc_n, &fsrc_m);
            err = adi_fpga_apollo_hw_fsrc_ratio_set(fpga_device, ADI_APOLLO_LINK_ALL, fsrc_n, fsrc_m);
            ADI_CMS_ERROR_RETURN(err);

        }
    } else {
        printf("FPGA RX FSRC:   %s\n", fpga_feature_flags.rx_hw_fsrc ? "HW" : "SW");
        printf("FPGA ADC TL:    %s\n", fpga_feature_flags.rx_hw_tl ? "HW" : "SW");

        /* Enable FPGA HW FSRC if supported by the FPGA image */
        err = adi_fpga_apollo_hw_fsrc_rx_enable_set(fpga_device, fpga_feature_flags.rx_hw_fsrc != 0);
        ADI_CMS_ERROR_RETURN(err);

        /* Set FSRC N / M for capture processing */
        fsrc_cfg = profile->rx_path[0].rx_fsrc[0];
        link_total_ratio = profile->jrx[0].rx_link_cfg[0].link_total_ratio;
        link_dp_ratio = profile->jrx[0].rx_link_cfg[0].link_dp_ratio;
        err = adi_fpga_apollo_fsrc_invalid_ratio_set(fpga_device, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_LINK_ALL,
                                                     fpga_feature_flags.rx_hw_fsrc ? false : fsrc_cfg.enable,
                                                     fsrc_cfg.fsrc_rate_int, fsrc_cfg.fsrc_rate_frac_a, fsrc_cfg.fsrc_rate_frac_b,
                                                     link_dp_ratio, link_total_ratio);
        ADI_CMS_ERROR_RETURN(err);
    }
    
    return err;
}


int32_t adi_ads10_apollo_ex_fpga_fsrc_pgm(adi_fpga_apollo_device_t *fpga_device, adi_ads10_apollo_fsrc_dr_cfg_t *fsrc_config, uint32_t terminal)
{
    int32_t err;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(fsrc_config);
    ADI_CMS_RANGE_CHECK(terminal, ADI_APOLLO_RX, ADI_APOLLO_TX);

    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    if (terminal == ADI_APOLLO_TX) {
        err = adi_fpga_apollo_fsrc_n_m_ratio_set(fpga_device, ADI_FPGA_APOLLO_JTX, ADI_FPGA_APOLLO_LINK_ALL,
                        !(fpga_feature_flags.tx_hw_fsrc || (fsrc_config->link_xdrc == fsrc_config->base_xdrc && fsrc_config->mode_1x)),
                        fsrc_config->fsrc_n, fsrc_config->fsrc_m, fsrc_config->link_xdrc, fsrc_config->base_xdrc);
        ADI_CMS_ERROR_RETURN(err);
        if (fpga_feature_flags.tx_hw_fsrc) {
            err = adi_fpga_apollo_hw_fsrc_ratio_set(fpga_device, ADI_APOLLO_LINK_ALL,
                                      fsrc_config->fsrc_n * fsrc_config->link_xdrc, fsrc_config->fsrc_m * fsrc_config->base_xdrc);
            ADI_CMS_ERROR_RETURN(err);
        }
    } else {
        err = adi_fpga_apollo_fsrc_n_m_ratio_set(fpga_device, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_LINK_ALL,
                        !(fpga_feature_flags.rx_hw_fsrc || (fsrc_config->link_xdrc == fsrc_config->base_xdrc && fsrc_config->mode_1x)),
                        fsrc_config->fsrc_n, fsrc_config->fsrc_m, fsrc_config->link_xdrc, fsrc_config->base_xdrc);
        ADI_CMS_ERROR_RETURN(err);
    }
    
    return err;
}

int32_t adi_ads10_apollo_ex_fpga_sr_pgm(adi_fpga_apollo_device_t *fpga_device, adi_ads10_apollo_sr_dr_cfg_t *sr_config, uint32_t terminal)
{
    int32_t err;
    adi_fpga_feature_flag_t fpga_feature_flags; // indicates fpga support for hw/sw fsrc and tpl
    uint32_t xdrc_ratio = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);
    ADI_CMS_NULL_PTR_CHECK(sr_config);
    ADI_CMS_RANGE_CHECK(terminal, ADI_APOLLO_RX, ADI_APOLLO_TX);

    xdrc_ratio = (sr_config->link_xdrc / sr_config->base_xdrc);

    err = adi_fpga_apollo_core_feature_flags_get(fpga_device, &fpga_feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    if (terminal == ADI_APOLLO_TX) {
        if (!fpga_feature_flags.tx_hw_tl) {
            err = adi_fpga_apollo_fsrc_sr_set(fpga_device, ADI_FPGA_APOLLO_JTX, ADI_FPGA_APOLLO_LINK_ALL, xdrc_ratio != 1, sr_config->link_xdrc, sr_config->base_xdrc);
            ADI_CMS_ERROR_RETURN(err);
        } else {
            err = adi_fpga_apollo_hw_fsrc_sr_enable_set(fpga_device, ADI_FPGA_APOLLO_JTX, xdrc_ratio >> 1);
            ADI_CMS_ERROR_RETURN(err);
        }
    } else {
        if (!fpga_feature_flags.rx_hw_tl) {
            err = adi_fpga_apollo_fsrc_sr_set(fpga_device, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_LINK_ALL, xdrc_ratio != 1, sr_config->link_xdrc, sr_config->base_xdrc);
            ADI_CMS_ERROR_RETURN(err);
        } else {
            err = adi_fpga_apollo_hw_fsrc_sr_enable_set(fpga_device, ADI_FPGA_APOLLO_JRX, xdrc_ratio >> 1);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return err;
}

int32_t adi_ads10_apollo_ex_fpga_tx_phy_drive_config_all(adi_fpga_apollo_device_t *fpga, uint8_t tx_diff_ctrl, uint8_t tx_pre_cur, uint8_t tx_post_cur) 
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t diff_amp[NUM_JTX_LANES];
    uint8_t pre_cur[NUM_JTX_LANES];
	uint8_t post_cur[NUM_JTX_LANES];

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Set FPGA JTx pre/post cursor, diff_amp */
	for (int i = 0; i < NUM_JTX_LANES; i++)
    {
	    diff_amp[i] = tx_diff_ctrl;
        pre_cur[i] = tx_pre_cur;
        post_cur[i] = tx_post_cur;
    }

    err = adi_fpga_apollo_core_jesd_tx_phy_prbs_drive_config(fpga, diff_amp, pre_cur, post_cur, NUM_JTX_LANES);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

#endif /* !defined(VERSAL_PLATFORM) */
