/*!
 * \brief     ADS10 Apollo examples common inspect function implementations
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <math.h>
#include "adi_apollo.h"
#include "adi_apollo_adc.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_fpga_apollo_clk.h"
#include "adi_ads10_apollo_ex_ctl.h"

int32_t adi_ads10_apollo_ex_inspect_jrx_link_all(adi_apollo_device_t *device)
{
    int err;
    adi_apollo_jesd_rx_inspect_t jrx_status;
    uint16_t links_to_inspect[] = { ADI_APOLLO_LINK_A0, ADI_APOLLO_LINK_A1, ADI_APOLLO_LINK_B0, ADI_APOLLO_LINK_B1 };
    const char * links_to_inspect_str[] = { "A0", "A1", "B0", "B1" };

    for (int l = 0; l < sizeof(links_to_inspect) / sizeof(links_to_inspect[0]); l++) {
        if (err = adi_apollo_jrx_link_inspect(device, links_to_inspect[l], &jrx_status), err != API_CMS_ERROR_OK) {
            printf("Error from adi_apollo_jrx_link_inspect() %d", err);
            return err;
        }
        printf("JRX ADI_APOLLO_LINK_%s: L=%2d M=%2d F=%2d S=%2d Np=%2d Ns=%2d K=%3d CS=%2d JMode=%2d VER=%2d SC=%2d link_en= %-8s\n",
            links_to_inspect_str[l],
            jrx_status.l_minus1+1,
            jrx_status.m_minus1+1,
            jrx_status.f_minus1+1,
            jrx_status.s_minus1+1,
            jrx_status.np_minus1 + 1,
            jrx_status.ns_minus1 + 1,
            jrx_status.k_minus1+1,
            jrx_status.cs,
            jrx_status.jesd_mode,
            jrx_status.ver,
            jrx_status.subclass,
            jrx_status.link_en ? "Enabled" : "Disabled");
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_inspect_jtx_link_all(adi_apollo_device_t *device)
{
    int err;
    adi_apollo_jesd_tx_inspect_t jtx_status;
    uint16_t links_to_inspect[] = { ADI_APOLLO_LINK_A0, ADI_APOLLO_LINK_A1, ADI_APOLLO_LINK_B0, ADI_APOLLO_LINK_B1 };
    const char * links_to_inspect_str[] = { "A0", "A1", "B0", "B1" };

    for (int l = 0; l < sizeof(links_to_inspect) / sizeof(links_to_inspect[0]); l++) {
        if (err = adi_apollo_jtx_link_inspect(device, links_to_inspect[l], &jtx_status), err != API_CMS_ERROR_OK) {
            printf("Error from adi_apollo_jtx_link_inspect() %d", err);
            return err;
        }
        printf("JTX ADI_APOLLO_LINK_%s: L=%2d M=%2d F=%2d S=%2d Np=%2d Ns=%2d K=%3d CS=%2d JMode=%2d VER=%2d SC=%2d link_en= %-8s\n",
            links_to_inspect_str[l],
            jtx_status.l_minus1+1,
            jtx_status.m_minus1+1,
            jtx_status.f_minus1+1,
            jtx_status.s_minus1+1,
            jtx_status.np_minus1+1,
            jtx_status.ns_minus1 + 1,
            jtx_status.k_minus1 + 1,
            jtx_status.cs,
            jtx_status.jesd_mode,
            jtx_status.ver,
            jtx_status.subclass,
            jtx_status.link_en ? "Enabled" : "Disabled");
    }

    return API_CMS_ERROR_OK;
}



int32_t adi_ads10_apollo_ex_inspect_invsinc_all(adi_apollo_device_t *device)
{
    int err;
    adi_apollo_invsinc_inspect_t invsinc_status;
    uint16_t chans_to_inspect[] = { ADI_APOLLO_TX_INVSINC_A0, ADI_APOLLO_TX_INVSINC_A1, ADI_APOLLO_TX_INVSINC_B0, ADI_APOLLO_TX_INVSINC_B1 };
    const char * chans_to_inspect_str[] = { "A0", "A1", "B0", "B1" };

    for (int l = 0; l < sizeof(chans_to_inspect) / sizeof(chans_to_inspect[0]); l++) {
        if (err = adi_apollo_invsinc_inspect(device, chans_to_inspect[l], &invsinc_status), err != API_CMS_ERROR_OK) {
            printf("Error from adi_apollo_invsinc_inspect() %d", err);
            return err;
        }
        printf("INVSINC ADI_APOLLO_TX_INVSINC_%s: invsinc_clk= %-8s invsinc= %-8s\n",
            chans_to_inspect_str[l],
            invsinc_status.invsinc_clk_en ? "Enabled" : "Disabled",
            invsinc_status.invsinc_en ? "Enabled" : "Disabled");
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_inspect_adc_all(adi_apollo_device_t *device)
{
    int err;
    adi_apollo_adc_status_t adc_status;

    err = adi_apollo_adc_status_get(device, &adc_status);
    if (err != API_CMS_ERROR_OK) {
        printf("Error getting adc status\n");
        return err;
    }

    for (int i = 0; i < ADI_APOLLO_ADC_NUM; i++) {
        if (adc_status.mode[i] != ADI_APOLLO_ADC_MODE_DISABLED) {
            printf("Status of ADC%c%d\n", i < ADI_APOLLO_ADC_PER_SIDE_NUM ? 'A' : 'B', i % ADI_APOLLO_ADC_PER_SIDE_NUM);
            printf("\tMode: %s\n", adc_status.mode[i] == ADI_APOLLO_ADC_MODE_RANDOM ? "RANDOM" : "SEQUENTIAL");
            printf("\tFG Cal Duration: %dms\n", adc_status.fg_cal.duration_ms);
            printf("\tFG Cal Error: %d\n", adc_status.fg_cal.err_status[i]);
            printf("\tFG Last Run: %d\n", adc_status.fg_cal.last_run[i]);
            printf("\tFG Since Boot: %d\n", adc_status.fg_cal.since_power_up[i]);
            printf("\tBG Cal Error: %d\n", adc_status.bg_cal.error[i]);
            printf("\tBG Cal Enabled %d\n", adc_status.bg_cal.enabled[i]);
            printf("\tBG Cal State: %d\n\n", adc_status.bg_cal.state[i]);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_inspect_lane_rates(adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t *profile, adi_ads10_apollo_clk_mode_e clk_mode)
{
    int32_t i, err;
    uint32_t bitrate, ver;
    uint16_t side_idx, terminal;
    uint8_t ref_clk_div;
    adi_fpga_clk_info_t clk_info;

    printf("Device Clk Source: %s\n", (clk_mode & ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382) ? "ADF4382" : "External");
    printf("FPGA Clk Source: %s\n", (clk_mode & ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_FMC) ? "FMC" : "External");

    for (i = 0; i < ADI_APOLLO_NUM_SIDES; i++) {
        printf("Apollo Side %c JTX Lane Rate: %d KHz\n", (i ? 'B' : 'A'), profile->jtx[0].common_link_cfg.lane_rate_kHz);
        printf("Apollo Side %c JRX Lane Rate: %d KHz\n", (i ? 'B' : 'A'), profile->jrx[0].common_link_cfg.lane_rate_kHz);
    }

    for (terminal = 0; terminal < ADI_FPGA_APOLLO_JESD_NUM; terminal++) {
        for (side_idx = 0; side_idx < ADI_APOLLO_NUM_SIDES; side_idx++) {
            if (terminal == ADI_FPGA_APOLLO_JRX) {
                ver = profile->jrx[side_idx].common_link_cfg.ver;
                bitrate = profile->jrx[side_idx].common_link_cfg.lane_rate_kHz >> profile->jrx[side_idx].deserializer_lane[0].lanerate_adapt;
            } else {
                ver = profile->jtx[side_idx].common_link_cfg.ver;
                bitrate = profile->jtx[side_idx].common_link_cfg.lane_rate_kHz >> profile->jtx[side_idx].serializer_lane[0].lanerate_adapt;
            }

            err = adi_fpga_apollo_clk_line_rate_div_calc(fpga_device, side_idx, terminal, bitrate, ver);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    clk_info = fpga_device->state_info.clk_info;

    ref_clk_div = (clk_info.clk_div[0].ver == 1 ? (clk_info.is_prbs ? 64 : 66) : 40) >> clk_info.min_link_div;
    fpga_device->state_info.clk_info.ref_clk_div = ref_clk_div;

    EXCTL_DEV_CLK(profile->clk_cfg.dev_clk_freq_kHz / 1e3);
    EXCTL_FPGA_CLK((fpga_device->state_info.clk_info.max_link_rate_khz / 1e3) / fpga_device->state_info.clk_info.ref_clk_div);

    return API_CMS_ERROR_OK;
}
int32_t adi_ads10_apollo_ex_inspect_rx_channel_get(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_ads10_apollo_rx_channel_info_t *channel_info)
{
    int32_t err = API_CMS_ERROR_OK;
    double cnco_freq_hz, fnco_freq_hz;
    double sample_rate_hz = profile->adc_config[0].adc_sampling_rate_Hz;
    adi_apollo_fddc_inspect_t finspect;
    uint32_t fdrc_dcm;
    uint32_t cdrc_dcm;
    adi_apollo_cddc_inspect_t cinspect;
    adi_apollo_fnco_inspect_t fnco_inspect;
    adi_apollo_cnco_inspect_t cnco_inspect;
    adi_apollo_fsrc_inspect_t fsrc_inspect;
    double fsrc_ratio;

    // Get CDDC decimation
    adi_apollo_cddc_inspect(device, ADI_APOLLO_CDDC_A0, &cinspect);
    adi_apollo_cddc_dcm_bf_to_val(device, cinspect.dp_cfg.drc_ratio, &cdrc_dcm);

    // Get CNCO frequency
    err = adi_apollo_cnco_inspect(device, ADI_APOLLO_RX, ADI_APOLLO_CNCO_A0, &cnco_inspect);
    if (cnco_inspect.dp_cfg.nco_if_mode == ADI_APOLLO_MXR_ZERO_IF_MODE) {
        cnco_freq_hz = 0;
    } else {
        cnco_inspect.dp_cfg.nco_phase_inc_frac_b = cnco_inspect.dp_cfg.nco_phase_inc_frac_a ? cnco_inspect.dp_cfg.nco_phase_inc_frac_b : 1;
        double phase_ratio = (double)cnco_inspect.active_phase_inc +
                      (double)cnco_inspect.dp_cfg.nco_phase_inc_frac_a / (double)cnco_inspect.dp_cfg.nco_phase_inc_frac_b;
        cnco_freq_hz = (sample_rate_hz * phase_ratio) / (1ull << 32);
    }

    // Get FNCO frequency
    err = adi_apollo_fnco_inspect(device, ADI_APOLLO_RX, ADI_APOLLO_FNCO_A0, &fnco_inspect);
    if (fnco_inspect.dp_cfg.nco_if_mode == ADI_APOLLO_MXR_TEST_MODE) {
        fnco_freq_hz = 0;
    } else if (fnco_inspect.hop_mode_en) {
        fnco_freq_hz = (sample_rate_hz / (cdrc_dcm)) * ((double)fnco_inspect.active_phase_inc) / (1ull << 48);
    } else {
        fnco_freq_hz = (sample_rate_hz / (cdrc_dcm)) * ((double)fnco_inspect.dp_cfg.nco_phase_inc) / (1ull << 48);
    }

    // Get FDDC ratio
    adi_apollo_fddc_inspect(device, ADI_APOLLO_FDDC_A0, &finspect);
    adi_apollo_fddc_dcm_bf_to_val(device, finspect.dp_cfg.drc_ratio, &fdrc_dcm);

    // Get FSRC ratio
    adi_apollo_fsrc_inspect(device, ADI_APOLLO_RX, ADI_APOLLO_FSRC_A0, &fsrc_inspect);
    fsrc_ratio = 1.0;
    if (fsrc_inspect.dp_cfg.enable && !fsrc_inspect.fsrc_bypass && !fsrc_inspect.dp_cfg.mode_1x) {
        fsrc_ratio =
            ((double)fsrc_inspect.dp_cfg.fsrc_rate_int + ((double)fsrc_inspect.dp_cfg.fsrc_rate_frac_a / (double)fsrc_inspect.dp_cfg.fsrc_rate_frac_b)) /
            pow(2.0, 48.0);
    }

    channel_info->cnco_freq_hz = cnco_freq_hz;
    channel_info->fnco_freq_hz = fnco_freq_hz;
    channel_info->nco_freq_hz = fnco_freq_hz + cnco_freq_hz;
    channel_info->data_rate_hz = (sample_rate_hz / (cdrc_dcm * fdrc_dcm)) * fsrc_ratio;

    return err;
}

int32_t adi_ads10_apollo_ex_inspect_tx_freq_get(adi_apollo_device_t *device, adi_apollo_top_t *profile, double ratio, double *tone_freq_mhz)
{
    int32_t err = API_CMS_ERROR_OK;
    double cnco_freq_hz, fnco_freq_hz;
    double sample_rate_hz = profile->dac_config[0].dac_sampling_rate_Hz;
    adi_apollo_fduc_inspect_t finspect;
    uint32_t fdrc_dcm;
    uint32_t cdrc_dcm;
    adi_apollo_cduc_inspect_t cinspect;
    adi_apollo_fnco_inspect_t fnco_inspect;
    adi_apollo_cnco_inspect_t cnco_inspect;
    adi_apollo_fsrc_inspect_t fsrc_inspect;
    double fsrc_ratio = 1;

    // Get CDUC decimation
    adi_apollo_cduc_inspect(device, ADI_APOLLO_CDUC_A0, &cinspect);
    adi_apollo_cduc_interp_bf_to_val(device, cinspect.dp_cfg.drc_ratio, &cdrc_dcm);

    // Get CNCO frequency
    err = adi_apollo_cnco_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_A0, &cnco_inspect);
    if (cnco_inspect.dp_cfg.nco_if_mode == ADI_APOLLO_MXR_ZERO_IF_MODE) {
        cnco_freq_hz = 0;
    } else {
        cnco_inspect.dp_cfg.nco_phase_inc_frac_b = cnco_inspect.dp_cfg.nco_phase_inc_frac_a ? cnco_inspect.dp_cfg.nco_phase_inc_frac_b : 1;
        double phase_ratio = (double)cnco_inspect.active_phase_inc +
                      (double)cnco_inspect.dp_cfg.nco_phase_inc_frac_a / (double)cnco_inspect.dp_cfg.nco_phase_inc_frac_b;
        cnco_freq_hz = (sample_rate_hz * phase_ratio) / (1ull << 32);
    }

    // Get FNCO frequency
    err = adi_apollo_fnco_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_A0, &fnco_inspect);
    if (fnco_inspect.dp_cfg.nco_if_mode == ADI_APOLLO_MXR_TEST_MODE) {
        fnco_freq_hz = 0;
    } else if (fnco_inspect.hop_mode_en) {
        fnco_freq_hz = (sample_rate_hz / (cdrc_dcm)) * ((double)fnco_inspect.dp_cfg.nco_phase_inc) / (1ull << 32);
    } else {
        fnco_freq_hz = (sample_rate_hz / (cdrc_dcm)) * ((double)fnco_inspect.dp_cfg.nco_phase_inc) / (1ull << 48);
    }

    // Get FDUC ratio
    adi_apollo_fduc_inspect(device, ADI_APOLLO_FDUC_A0, &finspect);
    adi_apollo_fduc_interp_bf_to_val(device, finspect.dp_cfg.drc_ratio, &fdrc_dcm);

    // Get FSRC ratio
    adi_apollo_fsrc_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_FSRC_A0, &fsrc_inspect);
    if (fsrc_inspect.dp_cfg.enable) {
        fsrc_ratio =
            ((double)fsrc_inspect.dp_cfg.fsrc_rate_int + ((double)fsrc_inspect.dp_cfg.fsrc_rate_frac_a / (double)fsrc_inspect.dp_cfg.fsrc_rate_frac_b)) /
            pow(2.0, 48.0);
    }

    double nco_freq_hz = fnco_freq_hz + cnco_freq_hz;
    double data_rate_hz = (sample_rate_hz / (cdrc_dcm * fdrc_dcm)) * fsrc_ratio;
    *tone_freq_mhz = (data_rate_hz * ratio + nco_freq_hz) / 1e6;

    printf("NCOs: F=%f, C=%f\n", fnco_freq_hz / 1e6, cnco_freq_hz / 1e6);
    printf("DUCs: F=%d, C=%d\n", fdrc_dcm, cdrc_dcm);
    printf("FSRC ratio: %f\n", 1.0 / fsrc_ratio);
    printf("Data rate: %f\n", data_rate_hz / 1e6);

    return err;
}
