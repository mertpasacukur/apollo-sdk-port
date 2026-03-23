#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     ADS10 Apollo examples calibration function implementations
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include <stdlib.h>
#include <unistd.h>
#include "adi_ads10_apollo_ex_cal.h"
#include "adi_apollo_linux_utilities.h"
#include "adi_ads10_apollo_ex.h"

/*==================== P U B L I C   A P I   C O D E ====================*/
static uint32_t Crc32(const uint8_t buf[], uint32_t bufLen);


bool adi_ads10_apollo_ex_run_serdes_init_cal_get(adi_apollo_top_t *profile) {
    /* SerDes Init Cal required if lane rate > 8.0Gbps */
    return profile->jrx[0].common_link_cfg.lane_rate_kHz > 8000000;
}

bool adi_ads10_apollo_ex_run_serdes_track_cal_get(adi_apollo_top_t *profile) {
    /* SerDes Track Cal required if lane rate > 16.0Gbps */
    return profile->jrx[0].common_link_cfg.lane_rate_kHz > 16000000;
}

int32_t adi_ads10_apollo_ex_cals_run(adi_apollo_device_t* device, adi_apollo_top_t *profile, uint16_t cals) {
    int32_t err;
    uint16_t adc_channels = device->dev_info.is_8t8r ? ADI_APOLLO_ADC_ALL : ADI_APOLLO_ADC_ALL_4T4R;
    uint16_t serdes = ((((profile->jrx[0].rx_link_cfg[0].link_in_use) || (profile->jrx[0].rx_link_cfg[1].link_in_use))
                           ? ADI_APOLLO_TXRX_SERDES_12PACK_A : ADI_APOLLO_TXRX_SERDES_12PACK_NONE)) |
                      ((((profile->jrx[1].rx_link_cfg[0].link_in_use) || (profile->jrx[1].rx_link_cfg[1].link_in_use))
                           ? ADI_APOLLO_TXRX_SERDES_12PACK_B : ADI_APOLLO_TXRX_SERDES_12PACK_NONE));

    if (cals & ADI_ADS10_APOLLO_CAL_CC) {
        printf("Run clock conditioning cal (can take up to %d secs)...\n", ADI_APOLLO_SYSCLK_COND_CENTER_MAX_TO);
        err = adi_apollo_sysclk_cond_cal(device);
        ADI_CMS_ERROR_RETURN(err);
        printf("clock conditioning cal done\n");
    }

    if (cals & ADI_ADS10_APOLLO_CAL_ADC) {
        printf("Run ADC FG cal (can take up to %d secs)...\n", (device->dev_info.is_8t8r ? ADI_APOLLO_8T8R_ADC_TIMEOUT : ADI_APOLLO_4T4R_ADC_TIMEOUT));
        err = adi_apollo_adc_cal(device, adc_channels);
        ADI_CMS_ERROR_RETURN(err);
        printf("ADC FG cal done\n");
    }

    if ((cals & ADI_ADS10_APOLLO_CAL_ADC_BG) || (cals & ADI_ADS10_APOLLO_CAL_ADC_BG_10S)) {
        /* Start the bgcals */
        printf("Start ADC BGcal...\n");
        err = adi_apollo_adc_bgcal_unfreeze(device, adc_channels);
        ADI_CMS_ERROR_RETURN(err);

        if (cals & ADI_ADS10_APOLLO_CAL_ADC_BG_10S) {
            /* Let cal run a bit to lower noise floor */
            printf("Let bgcal run for 10 secs...\n");
            adi_apollo_hal_delay_us(device, 10 * 1000000);
        }
    }

    if (cals & ADI_ADS10_APOLLO_CAL_RXTX_SYNC) {
        /* Dynamic Sync Serdes Links gradually in a sequence */
        printf("Run Rx-TX SerDes Links Dyn Sync...\n");
        err = adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run(device);
        ADI_CMS_ERROR_RETURN(err);
    }

    if ((cals & ADI_ADS10_APOLLO_CAL_SERDES) && serdes) {
        /* NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly */
        if (adi_ads10_apollo_ex_run_serdes_init_cal_get(profile)) {
            printf("Run SERDES JRx FG cal...\n");
            err = adi_apollo_serdes_jrx_init_cal(device, serdes, ADI_APOLLO_INIT_CAL_ENABLED);
            ADI_CMS_ERROR_RETURN(err);
            printf("SERDES JRx FG cal done\n");

            adi_apollo_hal_delay_us(device, 2 * 1000000);   /* Need to wait before JRX link gets up */

            /* Read Apollo JRx status A-SIDE */
            if (serdes & ADI_APOLLO_TXRX_SERDES_12PACK_A) {
                err = adi_ads10_apollo_ex_jesd_rx_status(device, ADI_APOLLO_LINK_A0);
                ADI_CMS_ERROR_RETURN(err);
            }

            /* Read Apollo JRx status B-SIDE */
            if (serdes & ADI_APOLLO_TXRX_SERDES_12PACK_B) {
                err = adi_ads10_apollo_ex_jesd_rx_status(device, ADI_APOLLO_LINK_B0);
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }

    if (cals & ADI_ADS10_APOLLO_CAL_SERDES_BG) {
        /* NOTE: The FPGA must be transmitting (any data) for SERDES cal to function properly */
        if (adi_ads10_apollo_ex_run_serdes_track_cal_get(profile)) {
            printf("Run SERDES JRx BG cal...\n");
            err = adi_apollo_serdes_jrx_bgcal_unfreeze(device, serdes);
            ADI_CMS_ERROR_RETURN(err);
            printf("SERDES JRx BG cal done\n");
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_adc_cal_data_dump_to_file(adi_apollo_device_t *device, char *file_name, uint16_t adc_cal_chans, uint8_t mode)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t len = 0;
    uint8_t adc_num = 0;
    uint32_t checksum_readback = 0;
    uint32_t checksum_calc = 0;
    uint8_t i = 0;
    adi_apollo_mailbox_resp_update_cal_data_crc_t resp;
    uint8_t *cal_data;

    if (device == NULL) {
        err = API_CMS_ERROR_INVALID_HANDLE_PTR;
        goto end;
    }

    if (mode > 1) {
        err = API_CMS_ERROR_INVALID_PARAM;
        goto end;
    }


    /* Freeze ADC background cal */
    printf("FREEZE bgcal\n");
    if (err = adi_apollo_adc_bgcal_freeze(device, adc_cal_chans), err != API_CMS_ERROR_OK) {
        printf("Error from adi_apollo_adc_bgcal_freeze() %d\n", err);
        goto end;
    }

    adi_apollo_hal_delay_us(device, 5000);


    // calData update is handled by ADC FW while CRC on the data is handled by Apollo FW.
    // So if crc isn't updated before the dump, there might be an older crc value.
    if (err = adi_apollo_mailbox_update_cal_data_crc(device, &resp), err != API_CMS_ERROR_OK) {
        printf("Error in adi_apollo_mailbox_update_cal_data_crc() %d\t", err);
        goto end;
    }

    /* Get size (in bytes) of ADC calibration data. Includes ending 32-bit checksum. Same length for all ADC instances */
    if (err = adi_apollo_cfg_adc_cal_data_len_get(device, mode, &len), err != API_CMS_ERROR_OK) {
        printf("Error in adi_apollo_cfg_adc_cal_data_len_get() %d\t", err);
        goto end;
    }

    cal_data = (uint8_t *)malloc(len);

    if (cal_data == NULL) {
        printf("Memory allocation error\n");
        err = API_CMS_ERROR_MEM_ALLOC;
        goto end;
    }

    printf("Reading calData from the device and saving to a file for %s mode.\n", (mode == 0) ? "Random" : "Sequential");
    for (adc_num = 0; adc_num < ADI_APOLLO_ADC_NUM; adc_num++) {
        if (adc_cal_chans & (ADI_APOLLO_ADC_A0 << adc_num)) {

            /* Readback ADC calData based on selected ADC and cal mode. Only one ADC converter to be selected at a time */
            if (err = adi_apollo_cfg_adc_cal_data_get(device, (ADI_APOLLO_ADC_A0 << adc_num),
                                                      mode, cal_data, len), err != API_CMS_ERROR_OK) {
                printf("Error in adi_apollo_cfg_adc_cal_data_get() %d\n", err);
                goto free_cal_data;
            }

            /* Save populated cal_data array to a file */
            if (err = adi_apollo_utilities_byte_arr_to_file_append(device, file_name, cal_data, len), err != API_CMS_ERROR_OK) {
                printf("Error in adi_apollo_utilities_byte_arr_to_file_append() %d\n", err);
                goto free_cal_data;
            }

            checksum_readback = 0x00;
            for (i = 0; i < 4; i++) {
                checksum_readback |= (cal_data[(len - 4) + i] << (i * 8));
            }

            checksum_calc = Crc32(cal_data, (len - 4));

            if (checksum_calc != checksum_readback) {
                printf("Calculated CRC Checksum (0x%08X) does not match with readback value  (0x%08X).\n", checksum_calc, checksum_readback);
            }

        }
    }

free_cal_data:
    free(cal_data);
end:
    return err;
}

int32_t adi_ads10_apollo_ex_adc_cal_data_reload_from_file(adi_apollo_device_t *device, char *file_name, uint16_t adc_cal_chans, uint8_t mode)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t len = 0;
    uint8_t adc_num = 0;
    uint8_t *cal_data;
    uint32_t cal_data_len = 0;
    uint8_t offset_index = 0;

    if (device == NULL) {
        err = API_CMS_ERROR_INVALID_HANDLE_PTR;
        goto end;
    }

    if (mode > 1) {
        err = API_CMS_ERROR_INVALID_PARAM;
        goto end;
    }


    /* Freeze ADC background cal */
    printf("FREEZE bgcal\n");
    if (err = adi_apollo_adc_bgcal_freeze(device, adc_cal_chans), err != API_CMS_ERROR_OK) {
        printf("Error from adi_apollo_adc_bgcal_freeze() %d\n", err);
        goto end;
    }

    adi_apollo_hal_delay_us(device, 5000);


    /* Get size (in bytes) of ADC calibration data. Includes ending 32-bit checksum. Same length for all ADC instances */
    if (err = adi_apollo_cfg_adc_cal_data_len_get(device, mode, &len), err != API_CMS_ERROR_OK) {
        printf("Error from adi_apollo_cfg_adc_cal_data_len_get() %d\n", err);
        goto end;
    }

    printf("Reading calData from a file and loading to the device for %s mode.\n", (mode == 0) ? "Random" : "Sequential");
    for (adc_num = 0; adc_num < ADI_APOLLO_ADC_NUM; adc_num++) {
        if (adc_cal_chans & (ADI_APOLLO_ADC_A0 << adc_num)) {

            if (err = adi_apollo_utilities_file_to_byte_arr_by_offset(device, file_name,
                                                                      (offset_index*len), len,
                                                                      &cal_data, &cal_data_len), err != API_CMS_ERROR_OK) {
                printf("Error from adi_apollo_utilities_file_to_byte_arr_by_offset() %d\n", err);
                goto end;
            }

            if (err = adi_apollo_cfg_adc_cal_data_set(device, (ADI_APOLLO_ADC_A0 << adc_num), mode, cal_data, len), err != API_CMS_ERROR_OK)
            {
                printf("Error from adi_apollo_cfg_adc_cal_data_set() %d\n", err);
                goto end;
            }
            offset_index++;

            free(cal_data);
        }
    }

    /* Disable InitCal, use calData from user */
    printf("ADC Cal: ADI_APOLLO_INIT_CAL_DISABLED_WARMBOOT_FROM_USER\n");
    if (err = adi_apollo_adc_init_cal(device, adc_cal_chans, ADI_APOLLO_INIT_CAL_DISABLED_WARMBOOT_FROM_USER), err != API_CMS_ERROR_OK) {
        printf("Error from adi_apollo_adc_init_cal() %d\n", err);
        goto end;
    }

end:
    return err;
}

static const uint32_t LUT_CRC32[16] = {
  0x00000000u, 0x04c11db7u, 0x09823b6eu, 0x0d4326d9u,
  0x130476dcu, 0x17c56b6bu, 0x1a864db2u, 0x1e475005u,
  0x2608edb8u, 0x22c9f00fu, 0x2f8ad6d6u, 0x2b4bcb61u,
  0x350c9b64u, 0x31cd86d3u, 0x3c8ea00au, 0x384fbdbdu
};

static uint32_t Crc32(const uint8_t buf[], uint32_t bufLen)
{
  uint32_t i;
  uint32_t a, b, c, d;

  a = 0x00000000u;

  for(i = 0u; i < bufLen; i++) {
    b = (a >> 28u) & 0xfu;
    c = LUT_CRC32[b];
    d = buf[i];
    a = (a << 4u) | (d >> 4u);
    a = a ^ c;
    b = (a >> 28u) & 0xfu;
    c = LUT_CRC32[b];
    a = (a << 4u) | (d & 0xfu);
    a = a ^ c;
  }

  for(i = 0u; i < 4u; i++){
    b = (a >> 28u) & 0xfu;
    c = LUT_CRC32[b];
    a = (a << 4u);
    a = a ^ c;
    b = (a >> 28u) & 0xfu;
    c = LUT_CRC32[b];
    a = (a << 4u);
    a = a ^ c;
  }

  return a;
}
#endif /* !defined(VERSAL_PLATFORM) */
