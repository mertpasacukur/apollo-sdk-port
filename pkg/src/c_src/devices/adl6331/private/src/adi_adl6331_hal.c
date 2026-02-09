/*!
 * \brief     ADL6331 HAL Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adl6331_hal.h"

static int32_t __hal_reg_get(adi_adl6331_device_t *adl6331, uint8_t chip_id, uint32_t reg, uint8_t *data);
static int32_t __hal_reg_set(adi_adl6331_device_t *adl6331, uint8_t chip_id, uint32_t reg, uint8_t data);
static void __txn_config_init(adi_cms_hal_txn_config_t* config);
static int32_t __get_chip_id(adi_adl6331_chip_id_e chip_id, uint8_t *value);

typedef int32_t(*__byte_index_calc)(uint8_t index, uint8_t size);

int32_t adi_adl6331_hal_reg_get(adi_adl6331_device_t *adl6331, adi_adl6331_chip_id_e chip_id, uint32_t reg, uint8_t *data)
{
    int32_t err;
    uint8_t code;

    ADI_CMS_NULL_PTR_CHECK(adl6331);
    ADI_CMS_NULL_PTR_CHECK(adl6331->hal_info.spi_read);
    ADI_CMS_NULL_PTR_CHECK(data);
    ADI_CMS_SINGLE_SELECT_CHECK(chip_id);

    err = __get_chip_id(chip_id, &code);
    ADI_CMS_ERROR_RETURN(err);

    return __hal_reg_get(adl6331, code, reg, data);
}

int32_t adi_adl6331_hal_reg_set(adi_adl6331_device_t *adl6331, adi_adl6331_chip_id_e chip_id, uint32_t reg, uint8_t data)
{
    int32_t err;
    adi_adl6331_chip_id_e id;
    uint8_t i;
    uint8_t code;

    ADI_CMS_NULL_PTR_CHECK(adl6331);
    ADI_CMS_NULL_PTR_CHECK(adl6331->hal_info.spi_write);

    for (i = 0; i < __ADL6331_CHIP_ID_COUNT; i++) {
        id = (adi_adl6331_chip_id_e)(1 << i);
        if ((id & chip_id) > 0) {
            err = __get_chip_id(id, &code);
            ADI_CMS_ERROR_RETURN(err);
            err = __hal_reg_set(adl6331, code, reg, data);
            ADI_CMS_ERROR_RETURN(err);
        }
    }

    return API_CMS_ERROR_OK;
}

static int32_t __hal_reg_get(adi_adl6331_device_t *adl6331, uint8_t chip_id, uint32_t reg, uint8_t *data)
{
    int32_t err;
    uint8_t in_data[] = {
        __ADL6331_GET_ADDRESS_MSB(reg, chip_id) | 0x80,
        __ADL6331_GET_ADDRESS_LSB(reg),
        0
    };
    uint8_t out_data[__ADL6331_SPI_MSG_SIZE] = { 0 };
    adi_cms_hal_txn_config_t txn_config;

    __txn_config_init(&txn_config);

    err = adl6331->hal_info.spi_read(
        adl6331->hal_info.user_data,
        in_data,
        out_data,
        __ADL6331_SPI_MSG_SIZE,
        &txn_config);

    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }

    *data = out_data[2];

    return API_CMS_ERROR_OK;
}

static int32_t __hal_reg_set(adi_adl6331_device_t *adl6331, uint8_t chip_id, uint32_t reg, uint8_t data)
{
    int32_t err;
    uint8_t in_data[] = {
        __ADL6331_GET_ADDRESS_MSB(reg, chip_id),
        __ADL6331_GET_ADDRESS_LSB(reg),
        data
    };

    adi_cms_hal_txn_config_t txn_config;

    __txn_config_init(&txn_config);

    err = adl6331->hal_info.spi_write(
        adl6331->hal_info.user_data,
        in_data,
        __ADL6331_SPI_MSG_SIZE,
        &txn_config);

    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }

    return API_CMS_ERROR_OK;
}

static int32_t __get_chip_id(adi_adl6331_chip_id_e chip_id, uint8_t *value)
{
    switch (chip_id) {
        case ADI_ADL6331_CHIP_ID_0:
            *value = 0;
            break;

        case ADI_ADL6331_CHIP_ID_1:
            *value = 1;
            break;

        case ADI_ADL6331_CHIP_ID_2:
            *value = 2;
            break;

        case ADI_ADL6331_CHIP_ID_3:
            *value = 3;
            break;

        case ADI_ADL6331_CHIP_ID_4:
            *value = 4;
            break;

        case ADI_ADL6331_CHIP_ID_5:
            *value = 5;
            break;

        case ADI_ADL6331_CHIP_ID_6:
            *value = 6;
            break;

        case ADI_ADL6331_CHIP_ID_7:
            *value = 7;
            break;

        default:
            return API_CMS_ERROR_ERROR;
    }

    return API_CMS_ERROR_OK;
}

static void __txn_config_init(adi_cms_hal_txn_config_t* config)
{
    config->addr_len = 2;
    config->data_len = 1;
    config->stream_len = 0;
    config->mask = 0x00FFFFFF; // 24bit
}
