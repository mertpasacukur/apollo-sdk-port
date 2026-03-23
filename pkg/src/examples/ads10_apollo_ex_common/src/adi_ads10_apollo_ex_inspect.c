#if !defined(VERSAL_PLATFORM)

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
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_fpga_apollo_clk.h"
#include "adi_ads10_apollo_ex_ctl.h"
#include "adi_utils.h"

static uint64_t calc_cnco_freq_hz(adi_apollo_cnco_inspect_t *cnco_inspect, uint64_t sample_rate_hz);

static uint64_t calc_fnco_freq_hz(adi_apollo_fnco_inspect_t *fnco_inspect, uint64_t sample_rate_hz, uint32_t cdrc_dcm);

static const char * clk_mode_to_str(adi_ads10_apollo_clk_mode_e clk_mode);

int32_t adi_ads10_apollo_ex_inspect_jrx_link_all(adi_apollo_device_t *device)
{
    int32_t err;
    adi_apollo_jesd_rx_inspect_t jrx_status;
    uint16_t links_to_inspect[] = { ADI_APOLLO_LINK_A0, ADI_APOLLO_LINK_A1, ADI_APOLLO_LINK_B0, ADI_APOLLO_LINK_B1 };
    const char * links_to_inspect_str[] = { "A0", "A1", "B0", "B1" };

    for (int8_t l = 0; l < sizeof(links_to_inspect) / sizeof(links_to_inspect[0]); l++) {
        err = adi_apollo_jrx_link_inspect(device, links_to_inspect[l], &jrx_status);
        ADI_CMS_ERROR_RETURN(err);

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
    int32_t err;
    adi_apollo_jesd_tx_inspect_t jtx_status;
    uint16_t links_to_inspect[] = { ADI_APOLLO_LINK_A0, ADI_APOLLO_LINK_A1, ADI_APOLLO_LINK_B0, ADI_APOLLO_LINK_B1 };
    const char * links_to_inspect_str[] = { "A0", "A1", "B0", "B1" };

    for (int8_t l = 0; l < sizeof(links_to_inspect) / sizeof(links_to_inspect[0]); l++) {
        err = adi_apollo_jtx_link_inspect(device, links_to_inspect[l], &jtx_status);
        ADI_CMS_ERROR_RETURN(err);

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
    int32_t err;
    adi_apollo_invsinc_inspect_t invsinc_status;
    uint16_t chans_to_inspect[] = { ADI_APOLLO_TX_INVSINC_A0, ADI_APOLLO_TX_INVSINC_A1, ADI_APOLLO_TX_INVSINC_B0, ADI_APOLLO_TX_INVSINC_B1 };
    const char * chans_to_inspect_str[] = { "A0", "A1", "B0", "B1" };
    for (int8_t l = 0; l < sizeof(chans_to_inspect) / sizeof(chans_to_inspect[0]); l++) {
        err = adi_apollo_invsinc_inspect(device, chans_to_inspect[l], &invsinc_status);
        ADI_CMS_ERROR_RETURN(err);

        printf("INVSINC ADI_APOLLO_TX_INVSINC_%s: invsinc_clk= %-8s invsinc= %-8s\n",
            chans_to_inspect_str[l],
            invsinc_status.invsinc_clk_en ? "Enabled" : "Disabled",
            invsinc_status.invsinc_en ? "Enabled" : "Disabled");
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_inspect_adc_all(adi_apollo_device_t *device)
{
    int32_t err;
    adi_apollo_adc_status_t adc_status;

    err = adi_apollo_adc_status_get(device, &adc_status);
    ADI_CMS_ERROR_RETURN(err);

    for (int8_t i = 0; i < ADI_APOLLO_ADC_NUM; i++) {
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

int32_t adi_ads10_apollo_ex_inspect_lane_rates(adi_fpga_apollo_device_t* fpga_device,
                                               adi_apollo_top_t *profile,
                                               adi_ads10_apollo_clk_mode_e clk_mode)
{
    int32_t i, err;
    uint32_t bitrate, ver;
    uint16_t side_idx, terminal;
    uint8_t ref_clk_div;
    adi_fpga_clk_info_t clk_info;

    printf("Device Clk Source: %s\n", clk_mode_to_str(clk_mode & DEV_CLK_MODE_MASK));
    printf("FPGA Clk Source: %s\n", clk_mode_to_str(clk_mode & FPGA_CLK_MODE_MASK));

    for (i = 0; i < ADI_APOLLO_NUM_SIDES; i++) {
        printf("Apollo Side %c JTX Lane Rate: %d KHz\n", (i ? 'B' : 'A'), profile->jtx[0].common_link_cfg.lane_rate_kHz);
        printf("Apollo Side %c JRX Lane Rate: %d KHz\n", (i ? 'B' : 'A'), profile->jrx[0].common_link_cfg.lane_rate_kHz);
    }

    for (terminal = 0; terminal < ADI_FPGA_APOLLO_JESD_NUM; terminal++) {
        for (side_idx = 0; side_idx < ADI_APOLLO_NUM_SIDES; side_idx++) {
            if (terminal == ADI_FPGA_APOLLO_JRX) {
                ver = profile->jtx[side_idx].common_link_cfg.ver;
                bitrate = profile->jtx[side_idx].common_link_cfg.lane_rate_kHz >> profile->jtx[side_idx].serializer_lane[0].lanerate_adapt;
            } else {
                ver = profile->jrx[side_idx].common_link_cfg.ver;
                bitrate = profile->jrx[side_idx].common_link_cfg.lane_rate_kHz >> profile->jrx[side_idx].deserializer_lane[0].lanerate_adapt;
            }

            err = adi_fpga_apollo_clk_line_rate_div_calc(fpga_device, side_idx, terminal, bitrate, ver);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    clk_info = fpga_device->state_info.clk_info;

    ref_clk_div = (clk_info.clk_div[0].ver == 1 ? (clk_info.is_prbs ? 64 : 66) : 40) >> clk_info.min_link_div;
    fpga_device->state_info.clk_info.ref_clk_div = ref_clk_div;

    if ((clk_mode & ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_PLL) || (clk_mode & ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_HMC7044_PLL)) {
        EXCTL_DEV_PLL_CLK(profile->clk_cfg.ref_clk_freq_kHz / 1e3);
    }

    EXCTL_DEV_CLK(profile->clk_cfg.dev_clk_freq_kHz / 1e3);
    EXCTL_FPGA_CLK((fpga_device->state_info.clk_info.max_link_rate_khz / 1e3) / fpga_device->state_info.clk_info.ref_clk_div);

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_inspect_rx_channel_get(adi_apollo_device_t *device,
                                                   adi_apollo_top_t *profile,
                                                   adi_ads10_apollo_channel_selectors_t channel_selectors,
                                                   adi_ads10_apollo_channel_info_t *channel_info)
{
    int32_t err = API_CMS_ERROR_OK;
    uint64_t cnco_freq_hz = 0;
    uint64_t fnco_freq_hz = 0;
    double fsrc_ratio = 1.0;
    uint32_t fdrc_dcm = 1;
    uint32_t cdrc_dcm = 1;
    uint8_t side_idx;
    uint64_t sample_rate_hz;
    adi_apollo_fddc_inspect_t finspect;
    adi_apollo_cddc_inspect_t cddc_inspect;
    adi_apollo_fnco_inspect_t fnco_inspect;
    adi_apollo_cnco_inspect_t cnco_inspect;
    adi_apollo_fsrc_inspect_t fsrc_inspect;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_NULL_POINTER_RETURN(profile);
    ADI_APOLLO_NULL_POINTER_RETURN(channel_info);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.cdxc_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.cnco_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.fnco_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.fdxc_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.fsrc_sel);

    side_idx = (channel_selectors.cdxc_sel == ADI_APOLLO_CDDC_A0 || channel_selectors.cdxc_sel == ADI_APOLLO_CDDC_A1 ||
                channel_selectors.cdxc_sel == ADI_APOLLO_CDDC_A2 || channel_selectors.cdxc_sel == ADI_APOLLO_CDDC_A3)
                ? 0 : 1;
    sample_rate_hz = profile->adc_config[side_idx].adc_sampling_rate_Hz;

    // Get CDDC decimation
    err = adi_apollo_cddc_inspect(device, channel_selectors.cdxc_sel, &cddc_inspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cddc_dcm_bf_to_val(device, cddc_inspect.dp_cfg.drc_ratio, &cdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    // Get CNCO frequency
    err = adi_apollo_cnco_inspect(device, ADI_APOLLO_RX, channel_selectors.cnco_sel, &cnco_inspect);
    ADI_CMS_ERROR_RETURN(err);
    cnco_freq_hz = calc_cnco_freq_hz(&cnco_inspect, sample_rate_hz);

    // Get FNCO frequency
    err = adi_apollo_fnco_inspect(device, ADI_APOLLO_RX, channel_selectors.fnco_sel, &fnco_inspect);
    ADI_CMS_ERROR_RETURN(err);
    fnco_freq_hz = calc_fnco_freq_hz(&fnco_inspect, sample_rate_hz, cdrc_dcm);

    // Get FDDC ratio
    err = adi_apollo_fddc_inspect(device, channel_selectors.fdxc_sel, &finspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fddc_dcm_bf_to_val(device, finspect.dp_cfg.drc_ratio, &fdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    // Get FSRC ratio
    err = adi_apollo_fsrc_inspect(device, ADI_APOLLO_RX, channel_selectors.fsrc_sel, &fsrc_inspect);
    ADI_CMS_ERROR_RETURN(err);
    if (fsrc_inspect.dp_cfg.enable && !fsrc_inspect.fsrc_bypass && !fsrc_inspect.dp_cfg.mode_1x) {
        fsrc_ratio =
        ((double)fsrc_inspect.dp_cfg.fsrc_rate_int + ((double)fsrc_inspect.dp_cfg.fsrc_rate_frac_a / (double)fsrc_inspect.dp_cfg.fsrc_rate_frac_b)) /
        pow(2.0, 48.0);
    }

    channel_info->cnco_freq_hz = cnco_freq_hz;
    channel_info->fnco_freq_hz = fnco_freq_hz;
    channel_info->nco_freq_hz = fnco_freq_hz + cnco_freq_hz;

    /* Prevent division by zero in denominator */
    uint32_t denominator = cdrc_dcm * fdrc_dcm;
    if (denominator == 0) {
        denominator = 1;
    }
    channel_info->data_rate_hz = (uint64_t)(((double)sample_rate_hz / (double)denominator) * fsrc_ratio);

    printf("\nRX Channel Information:\n");
    printf("----------------------\n");
    printf("CNCO Frequency: %.2f MHz\n", channel_info->cnco_freq_hz / 1e6);
    printf("FNCO Frequency: %.2f MHz\n", channel_info->fnco_freq_hz / 1e6);
    printf("Total NCO Frequency: %.2f MHz\n", channel_info->nco_freq_hz / 1e6);
    printf("Data Rate: %.2f MHz\n", channel_info->data_rate_hz / 1e6);
    printf("----------------------\n");

    return err;
}

int32_t adi_ads10_apollo_ex_inspect_tx_channel_get(adi_apollo_device_t *device,
                                                   adi_apollo_top_t *profile,
                                                   adi_ads10_apollo_channel_selectors_t channel_selectors,
                                                   adi_ads10_apollo_channel_info_t *channel_info)
{
    int32_t err = API_CMS_ERROR_OK;
    uint64_t cnco_freq_hz = 0;
    uint64_t fnco_freq_hz = 0;
    double fsrc_ratio = 1.0;
    uint32_t fdrc_dcm = 1;
    uint32_t cdrc_dcm = 1;
    uint8_t side_idx;
    uint64_t sample_rate_hz;
    adi_apollo_fduc_inspect_t finspect;
    adi_apollo_cduc_inspect_t cduc_inspect;
    adi_apollo_fnco_inspect_t fnco_inspect;
    adi_apollo_cnco_inspect_t cnco_inspect;
    adi_apollo_fsrc_inspect_t fsrc_inspect;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_NULL_POINTER_RETURN(profile);
    ADI_APOLLO_NULL_POINTER_RETURN(channel_info);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.cdxc_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.cnco_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.fnco_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.fdxc_sel);
    ADI_CMS_SINGLE_SELECT_CHECK(channel_selectors.fsrc_sel);

    side_idx = (channel_selectors.cdxc_sel == ADI_APOLLO_CDUC_A0 || channel_selectors.cdxc_sel == ADI_APOLLO_CDUC_A1
                || channel_selectors.cdxc_sel == ADI_APOLLO_CDUC_A2 || channel_selectors.cdxc_sel == ADI_APOLLO_CDUC_A3)
                ? 0 : 1;
    sample_rate_hz = profile->dac_config[side_idx].dac_sampling_rate_Hz;

    // Get CDUC decimation
    err = adi_apollo_cduc_inspect(device, channel_selectors.cdxc_sel, &cduc_inspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cduc_interp_bf_to_val(device, cduc_inspect.dp_cfg.drc_ratio, &cdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    // Get CNCO frequency
    err = adi_apollo_cnco_inspect(device, ADI_APOLLO_TX, channel_selectors.cnco_sel, &cnco_inspect);
    ADI_CMS_ERROR_RETURN(err);
    cnco_freq_hz = calc_cnco_freq_hz(&cnco_inspect, sample_rate_hz);

    // Get FNCO frequency
    err = adi_apollo_fnco_inspect(device, ADI_APOLLO_TX, channel_selectors.fnco_sel, &fnco_inspect);
    ADI_CMS_ERROR_RETURN(err);
    fnco_freq_hz = calc_fnco_freq_hz(&fnco_inspect, sample_rate_hz, cdrc_dcm);

    // Get FDUC ratio
    err = adi_apollo_fduc_inspect(device, channel_selectors.fdxc_sel, &finspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fduc_interp_bf_to_val(device, finspect.dp_cfg.drc_ratio, &fdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    // Get FSRC ratio
    if(channel_selectors.fsrc_sel != ADI_APOLLO_FSRC_NONE) {
        err = adi_apollo_fsrc_inspect(device, ADI_APOLLO_TX, channel_selectors.fsrc_sel, &fsrc_inspect);
        ADI_CMS_ERROR_RETURN(err);
        if (fsrc_inspect.dp_cfg.enable) {
            fsrc_ratio =
                ((double)fsrc_inspect.dp_cfg.fsrc_rate_int + ((double)fsrc_inspect.dp_cfg.fsrc_rate_frac_a / (double)fsrc_inspect.dp_cfg.fsrc_rate_frac_b)) /
                pow(2.0, 48.0);
        }
    }

    channel_info->cnco_freq_hz = cnco_freq_hz;
    channel_info->fnco_freq_hz = fnco_freq_hz;
    channel_info->nco_freq_hz = fnco_freq_hz + cnco_freq_hz;

    /* Prevent division by zero in denominator */
    uint32_t denominator = cdrc_dcm * fdrc_dcm;
    if (denominator == 0) {
        denominator = 1;
    }
    channel_info->data_rate_hz = (uint64_t)(((double)sample_rate_hz / (double)denominator) * fsrc_ratio);

    printf("\nTX Channel Information:\n");
    printf("----------------------\n");
    printf("CNCO Frequency: %.2f MHz\n", channel_info->cnco_freq_hz / 1e6);
    printf("FNCO Frequency: %.2f MHz\n", channel_info->fnco_freq_hz / 1e6);
    printf("Total NCO Frequency: %.2f MHz\n", channel_info->nco_freq_hz / 1e6);
    printf("Data Rate: %.2f MHz\n", channel_info->data_rate_hz / 1e6);
    printf("----------------------\n");

    return err;
}

int32_t adi_ads10_apollo_ex_inspect_tx_freq_get(adi_apollo_device_t *device,
                                                adi_apollo_top_t *profile,
                                                double ratio,
                                                double *tone_freq_mhz)
{
    int32_t err = API_CMS_ERROR_OK;
    uint64_t cnco_freq_hz;
    uint64_t fnco_freq_hz;
    uint64_t sample_rate_hz = profile->dac_config[0].dac_sampling_rate_Hz;
    adi_apollo_fduc_inspect_t finspect;
    uint32_t fdrc_dcm;
    uint32_t cdrc_dcm;
    adi_apollo_cduc_inspect_t cduc_inspect;
    adi_apollo_fnco_inspect_t fnco_inspect;
    adi_apollo_cnco_inspect_t cnco_inspect;
    adi_apollo_fsrc_inspect_t fsrc_inspect;
    double fsrc_ratio = 1;

    ADI_APOLLO_NULL_POINTER_RETURN(device);
    ADI_APOLLO_NULL_POINTER_RETURN(profile);
    ADI_APOLLO_NULL_POINTER_RETURN(tone_freq_mhz);

    // Get CDUC decimation
    err = adi_apollo_cduc_inspect(device, ADI_APOLLO_CDUC_A0, &cduc_inspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_cduc_interp_bf_to_val(device, cduc_inspect.dp_cfg.drc_ratio, &cdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    // Get CNCO frequency
    err = adi_apollo_cnco_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_CNCO_A0, &cnco_inspect);
    ADI_CMS_ERROR_RETURN(err);
    cnco_freq_hz = calc_cnco_freq_hz(&cnco_inspect, sample_rate_hz);

    // Get FNCO frequency
    err = adi_apollo_fnco_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_FNCO_A0, &fnco_inspect);
    ADI_CMS_ERROR_RETURN(err);
    fnco_freq_hz = calc_fnco_freq_hz(&fnco_inspect, sample_rate_hz, cdrc_dcm);

    // Get FDUC ratio
    err = adi_apollo_fduc_inspect(device, ADI_APOLLO_FDUC_A0, &finspect);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_fduc_interp_bf_to_val(device, finspect.dp_cfg.drc_ratio, &fdrc_dcm);
    ADI_CMS_ERROR_RETURN(err);

    // Get FSRC ratio
    err = adi_apollo_fsrc_inspect(device, ADI_APOLLO_TX, ADI_APOLLO_FSRC_A0, &fsrc_inspect);
    ADI_CMS_ERROR_RETURN(err);
    if (fsrc_inspect.dp_cfg.enable) {
        fsrc_ratio =
            ((double)fsrc_inspect.dp_cfg.fsrc_rate_int + ((double)fsrc_inspect.dp_cfg.fsrc_rate_frac_a
            / (double)fsrc_inspect.dp_cfg.fsrc_rate_frac_b)) / pow(2.0, 48.0);
    }

    uint64_t nco_freq_hz = fnco_freq_hz + cnco_freq_hz;
    /* Prevent division by zero in denominator */
    uint32_t denominator = cdrc_dcm * fdrc_dcm;
    if (denominator == 0) {
        denominator = 1;
    }
    double data_rate_hz = ((double)sample_rate_hz / (double)denominator) * fsrc_ratio;
    *tone_freq_mhz = (data_rate_hz * ratio + (double)nco_freq_hz) / 1e6;

    printf("NCOs: F=%f, C=%f\n", (double)fnco_freq_hz / 1e6, (double)cnco_freq_hz / 1e6);
    printf("DUCs: F=%d, C=%d\n", fdrc_dcm, cdrc_dcm);
    printf("FSRC ratio: %f\n", 1.0 / fsrc_ratio);
    printf("Data rate: %f\n", data_rate_hz / 1e6);

    return err;
}

/**
 * \brief Private function to calculate CNCO frequency.
 *
 * \param[in] cnco_inspect   CNCO inspect structure containing NCO parameters
 * \param[in] sample_rate_hz Sample rate in Hz
 *
 * \return CNCO frequency in Hz, or 0 for bypass modes (ZERO_IF_MODE)
 *
 * \note Uses 32-bit phase accumulator resolution for CNCO calculations
 * \note Incorporates fractional phase increment correction (modulus mode)
 */
static uint64_t calc_cnco_freq_hz(adi_apollo_cnco_inspect_t *cnco_inspect, uint64_t sample_rate_hz)
{
    double phase_ratio = 0.0;

    ADI_CMS_NULL_PTR_CHECK(cnco_inspect);

    if (cnco_inspect->dp_cfg.nco_if_mode == ADI_APOLLO_MXR_ZERO_IF_MODE) {
        return 0;
    }

    /* Ensure denominator is never zero to prevent division by zero */
    if (cnco_inspect->dp_cfg.nco_phase_inc_frac_b == 0) {
        cnco_inspect->dp_cfg.nco_phase_inc_frac_b = 1;
    }

    phase_ratio = (double)cnco_inspect->active_phase_inc +
        (double)cnco_inspect->dp_cfg.nco_phase_inc_frac_a / (double)cnco_inspect->dp_cfg.nco_phase_inc_frac_b;

    return (uint64_t)(((double)sample_rate_hz * phase_ratio) / (1ull << 32));
}

/**
 * \brief Private function to calculate FNCO frequency.
 *
 * \param[in] fnco_inspect   FNCO inspect structure containing NCO parameters
 * \param[in] sample_rate_hz Sample rate in Hz
 * \param[in] cdrc_dcm       Coarse decimation factor, must be non-zero
 *
 * \return FNCO frequency in Hz, or 0 for bypass modes (TEST_MODE, ZERO_IF_MODE)
 *
 * \note Uses 48-bit phase accumulator resolution for FNCO calculations
 * \note In hop mode, uses active_phase_inc; otherwise uses profile nco_phase_inc
 */
static uint64_t calc_fnco_freq_hz(adi_apollo_fnco_inspect_t *fnco_inspect, uint64_t sample_rate_hz, uint32_t cdrc_dcm)
{
    ADI_CMS_NULL_PTR_CHECK(fnco_inspect);
    if (cdrc_dcm == 0) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    /* Handle special NCO modes that bypass frequency calculation */
    if (fnco_inspect->dp_cfg.nco_if_mode == ADI_APOLLO_MXR_ZERO_IF_MODE) {
        return 0;
    }
    
    /* Calculate FNCO frequency depending on hop mode */
    if (fnco_inspect->hop_mode_en) {
        return (uint64_t)(((double)sample_rate_hz / (double)cdrc_dcm) * ((double)fnco_inspect->active_phase_inc) / (1ull << 48));
    } else {
        return (uint64_t)(((double)sample_rate_hz / (double)cdrc_dcm) * ((double)fnco_inspect->dp_cfg.nco_phase_inc) / (1ull << 48));
    }
}

/**
 * \brief Private function to convert clock mode enum to string.
 *
 * \param[in] clk_mode Clock mode enum value to convert
 *
 * \return String representation of the clock mode, or "?" for unknown modes
 *
 * \note Supports both device and FPGA clock source modes
 * \note Used for human-readable output in lane rate inspection functions
 */
static const char * clk_mode_to_str(adi_ads10_apollo_clk_mode_e clk_mode)
{

    switch (clk_mode) {
    case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER:
        return "External";
    case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_DUAL:
        return "External-DUAL";
    case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_ADF4382:
        return "ADF4382";
    case ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL:
        return "External";
    case ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_FMC:
        return "FMC";
    case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_PLL:
        return "External-PLL";
    case ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_HMC7044_PLL:
        return "HMC7044-PLL";

    default:
        return "?";
    };
}

#endif /* !defined(VERSAL_PLATFORM) */
