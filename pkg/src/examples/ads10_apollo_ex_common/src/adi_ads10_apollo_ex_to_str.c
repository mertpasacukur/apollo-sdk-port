/*!
 * \brief     ADS10 Apollo common examples to string converters
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_ads10_apollo_ex_to_str.h"

int32_t adi_ads10_ex_tmu_data_to_str(const adi_apollo_device_tmu_data_t* tmu_data, char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;

    if (tmu_data == NULL || str_buff == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }
    snprintf(str_buff, max_str_len, "TMU (deg C): serdes_pll=%d mpu_a=%d mpu_b=%d adc_a=%d clk_a=%d adc_b=%d clk_b=%d clk_c=%d (avg: %d, avg mask: 0x%04x)",
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_SERDES_PLL],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_MPU_A],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_MPU_B],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_ADC_A],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_A],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_ADC_B],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_B],
        tmu_data->temp_degrees_celsius[ADI_APOLLO_DEVICE_TMU_CLK_C],
        tmu_data->temp_degrees_celsius_avg,
        tmu_data->avg_mask        
        );
end:
    return err;
}

int32_t adi_ads10_ex_uuid_to_str(uint8_t uuid[], uint32_t uuid_len, char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;
    int j=0;

    if (uuid == NULL || str_buff == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    if (max_str_len < (uuid_len*2 + 1)) {
        err = API_CMS_ERROR_INVALID_PARAM;
        goto end;
    }

    str_buff[uuid_len*2] = '\0';
    for (int i = uuid_len-1; i >= 0; i--, j+=2) {
        sprintf((str_buff+j), "%02X", uuid[i]);
    }

end:
    return err;
}

int32_t adi_ads10_ex_adc_bgcal_to_str(uint16_t adcs, const adi_apollo_adc_bgcal_state_t bgcal_state[], char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t i, adc;
    char buff[256];
    uint32_t char_cnt = 0;

    if (bgcal_state == NULL || str_buff == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }
    
    strcpy(str_buff, "");

    for (i = 0; i < ADI_APOLLO_ADC_NUM; i++) {
        adc = adcs & (ADI_APOLLO_ADC_A0 << i);
        if (adc > 0) {
            snprintf(buff, 256, "BGCAL STATE ADC[%d]: 0x%02X  0x%02X\n", i, bgcal_state[i].bgcal_error, bgcal_state[i].bgcal_state);
            char_cnt += strlen(buff);
            if (char_cnt < max_str_len) {
                strcat(str_buff, buff);
            }
            else {
                err = API_CMS_ERROR_ERROR;
                goto end;
            }
        }
    }

end:
    return err;
}

int32_t adi_ads10_ex_dp_info_to_str(adi_ads10_apollo_dp_info_t *dp_info, char *profile_name, char str_buff[], uint32_t max_str_len) {
    int32_t err = API_CMS_ERROR_OK;
    int idx = 0;

    if (dp_info == NULL || str_buff == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    /* Display data path freq info */
    idx += sprintf(&str_buff[idx], "%-16s %-7s %-5s %-11s %-11s %-4s %-21s %-8s %-8s\n", "Profile         ", "Fclk   ", "PFILT", "CDDC/CDUC  ", "FDDC/FDUC  ", "CFIR", "FSRC     ", "Fdata   ", "JESD DR ");
    idx += sprintf(&str_buff[idx], "%-16s %-7s %-5s %-11s %-11s %-4s %-21s %-8s %-8s\n", "================", "=======", "=====", "===========", "===========", "====", "=====================", "========", "========");
    idx += sprintf(&str_buff[idx], "%-16s %7.1f %-5s %2d:%-8.3f %2d:%-8.3f %-4s %5d/%-5d = %-6.5f %-8.3f %-8.3f\n", 
        profile_name, (dp_info->terminal == ADI_APOLLO_RX ? dp_info->adc_sample_rate : dp_info->dac_sample_rate), 
        dp_info->pfilt_en ? "Y" : "N", dp_info->cdrc, dp_info->cnco_freq, dp_info->fdrc, dp_info->fnco_freq,
        dp_info->cfir_en ? "Y" : "N", dp_info->fsrc_n, dp_info->fsrc_m, dp_info->fsrc_ratio,
        dp_info->fsrc_data_rate, dp_info->fdata);

end:
    return err;
}

int32_t adi_ads10_ex_jrx_irq_to_str(adi_apollo_device_t *device, uint32_t irq_mask, uint8_t clear, char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;
    uint16_t rmfifo_status[2];
    uint32_t irq_status[2];
    int ret = 0, idx = 0;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(str_buff);

    /* Get JRx and RMFIFO IRQs */
    err = adi_apollo_jrx_j204c_irq_get(device, ADI_APOLLO_LINK_A0, irq_mask, clear, &irq_status[0]);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_jrx_j204c_irq_get(device, ADI_APOLLO_LINK_B0, irq_mask, clear, &irq_status[1]);
    ADI_CMS_ERROR_RETURN(err);

    /* Get RMFIFO overflow/underflow status */
    err = adi_apollo_jrx_rm_fifo_status(device, ADI_APOLLO_LINK_A0, &rmfifo_status[0]);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_apollo_jrx_rm_fifo_status(device, ADI_APOLLO_LINK_B0, &rmfifo_status[1]);
    ADI_CMS_ERROR_RETURN(err);

    ret = snprintf(&str_buff[idx], max_str_len - idx, "%8s    DRLOST    EMB    MB    SH   CRC    RMFULL    RMEMPTY        RMFULL    RMEMPTY\n", "");
    if (ret < 0) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
    }
    idx += ret;
    ret = snprintf(&str_buff[idx], max_str_len - idx, "%8s    ======    ===    ==    ==   ===    ======    =======        ======    =======\n", "");
    if (ret < 0) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
    }
    idx += ret;

    for (int i = 0; i < 2; i++) {
        ret = snprintf(&str_buff[idx], max_str_len - idx, "%-8s      %s        %s      %s     %s    %s       %s          %s             %s          %s\n",
               (i == 0) ? "LINK A0" : "LINK B0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_DATA_RDY_LOST) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_DATA_RDY_LOST) ? "1" : "0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_EMB) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_EMB) ? "1" : "0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_MB) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_MB) ? "1" : "0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_SH) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_SH) ? "1" : "0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_CRC) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_CRC) ? "1" : "0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_RM_FIFO_FULL) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_RM_FIFO_FULL) ? "1" : "0",
               !(irq_mask & ADI_APOLLO_JRX_J204C_IRQ_RM_FIFO_EMPTY) ? "-" : (irq_status[i] & ADI_APOLLO_JRX_J204C_IRQ_RM_FIFO_EMPTY) ? "1" : "0",
               rmfifo_status[i] & 0x02 ? "1" : "0",
               rmfifo_status[i] & 0x01 ? "1" : "0");
        if (ret < 0) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
        }
        idx += ret;
    }


    return err;
}

int32_t adi_ads10_ex_fsrc_dr_cfg_to_str(adi_ads10_apollo_fsrc_dr_cfg_t *fsrc_ratio_cfg, char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t ret = 0;

    ADI_CMS_NULL_PTR_CHECK(fsrc_ratio_cfg);
    ADI_CMS_NULL_PTR_CHECK(str_buff);

    ret = snprintf(str_buff, max_str_len, "%-25s    %14.3f    %6d    %6d    %3d   %3d    %10.3f    %10.3f ",
            fsrc_ratio_cfg->name, fsrc_ratio_cfg->fdata,
            fsrc_ratio_cfg->fsrc_m, fsrc_ratio_cfg->fsrc_n,
            fsrc_ratio_cfg->cdrc_ratio_val, fsrc_ratio_cfg->fdrc_ratio_val,
            fsrc_ratio_cfg->cnco_freq, fsrc_ratio_cfg->fnco_freq);

    if (ret < 0) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
    }

    return err;
}

int32_t adi_ads10_ex_sr_dr_cfg_to_str(adi_ads10_apollo_sr_dr_cfg_t *sr_ratio_cfg, char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t ret = 0;

    ADI_CMS_NULL_PTR_CHECK(sr_ratio_cfg);
    ADI_CMS_NULL_PTR_CHECK(str_buff);

    ret = snprintf(str_buff, max_str_len, "%-25s    %14.3f    %3d    %3d    %10.3f    %10.3f ",
            sr_ratio_cfg->name, sr_ratio_cfg->fdata,
            sr_ratio_cfg->cdrc_ratio_val, sr_ratio_cfg->fdrc_ratio_val,
            sr_ratio_cfg->cnco_freq, sr_ratio_cfg->fnco_freq);

    if (ret < 0) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
    }

    return err;
}