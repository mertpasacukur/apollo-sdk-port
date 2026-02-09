/*!
 * \brief     ADF4382 Core Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4382_core.h"
#include "adi_adf4382_hal.h"
#include "adi_adf4382_regmap_inline_bfs.h"

#define SPI_IN_OUT_BUFF_SZ 0x3



int32_t adi_adf4382_core_init(adi_adf4382_device_t *adf4382)
{
    int32_t err = API_CMS_ERROR_ERROR;
    adi_adf4382_chip_ver_sel_e chip_ver_sel = ADI_ADF4382_CHIP_VER_AUTO;
    uint8_t chip_ver = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);

    switch (chip_ver_sel) {

        case ADI_ADF4382_CHIP_VER_AUTO:
            err = adi_adf4382_core_chip_ver_get(adf4382, &chip_ver);
            ADI_CMS_ERROR_RETURN(err);

            err = adi_adf4382_hal_reg_default_set(adf4382, chip_ver);
            ADI_CMS_ERROR_RETURN(err);
            break;

        case ADI_ADF4382_CHIP_VER_U2:
            err = adi_adf4382_hal_reg_default_set(adf4382, ADI_ADF4382_CHIP_VER_U2);
            ADI_CMS_ERROR_RETURN(err);
            break;

        case ADI_ADF4382_CHIP_VER_U4:
            err = adi_adf4382_hal_reg_default_set(adf4382, ADI_ADF4382_CHIP_VER_U4);
            ADI_CMS_ERROR_RETURN(err);
            break;

        case ADI_ADF4382_CHIP_VER_U5_A:
        case ADI_ADF4382_CHIP_VER_U5_B:
        case ADI_ADF4382_CHIP_VER_U5_C:
            err = adi_adf4382_hal_reg_default_set(adf4382, chip_ver_sel);
            ADI_CMS_ERROR_RETURN(err);
            break;

        default:
            printf("Invalid ADF4382 Chip Version Selected: %d.\n", chip_ver_sel);
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
            break;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_core_version_get(adi_adf4382_device_t *adf4382, adi_adf4382_version_t *apiVersion)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(apiVersion);

    apiVersion->major = 0;
    apiVersion->minor = 1;
    apiVersion->patch = 0;

    return err;
}

int32_t adi_adf4382_core_spi_reg_get(adi_adf4382_device_t *adf4382 , uint16_t address, uint8_t *data)
{
    return adi_adf4382_hal_reg_get(adf4382, address, data);
}

int32_t adi_adf4382_core_spi_reg_set(adi_adf4382_device_t *adf4382, uint16_t address, uint8_t data)
{
    return adi_adf4382_hal_reg_set(adf4382, address, data);
}

int32_t adi_adf4382_core_spi_reg_test(adi_adf4382_device_t *adf4382)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint8_t i = 0;
    uint32_t scratchpad_reg = 0x0A;
    uint8_t reg_read = 0x00;
    uint8_t reg_write[] = {0x55, 0xAA};

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_write);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_read);

    for (i = 0; i < 2; i++) {
        err = adi_adf4382_hal_reg_set(adf4382, scratchpad_reg, reg_write[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4382_hal_reg_get(adf4382, scratchpad_reg, &reg_read);
        ADI_CMS_ERROR_RETURN(err);

        ADI_CMS_CHECK((reg_read != reg_write[i]), API_CMS_ERROR_SPI_XFER);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_core_chip_ver_get(adi_adf4382_device_t *adf4382, uint8_t *chip_version)
{
    int32_t err = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(chip_version);

    err = adi_adf4382_bf___REG0067___VERSION_get(adf4382, chip_version);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}