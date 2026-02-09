/*!
 * \brief     HMC7044 Core Functionality
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_hmc7044_core.h"
#include "hmc7044_hal.h"

int32_t adi_hmc7044_core_version_get(adi_hmc7044_device_t *hmc7044, adi_hmc7044_version_t *apiVersion)
{
    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(apiVersion);

    apiVersion->major = 0;
    apiVersion->minor = 1;
    apiVersion->patch = 0;

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_core_spi_reg_set(adi_hmc7044_device_t *hmc7044, uint16_t address, uint8_t data)
{
    int32_t err;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(hmc7044->hal_info.spi_write);

    err = hmc7044_spi_reg_set(hmc7044, address, data);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_core_spi_reg_get(adi_hmc7044_device_t *hmc7044, uint16_t address, uint8_t *data)
{
    int32_t err;

    ADI_HMC7044_PTR_CHECK(hmc7044);
    ADI_HMC7044_NULL_CHECK(hmc7044->hal_info.spi_read);

    err = hmc7044_spi_reg_get(hmc7044, address, data);
    ADI_HMC7044_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_core_spi_reg_test(adi_hmc7044_device_t *hmc7044)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint8_t i = 0;
    uint32_t scratchpad_reg = 0x08;     // HMC_GLOBAL_SCRATCHPAD_REG
    uint8_t reg_read = 0x00;
    uint8_t reg_write[] = {0x55, 0xAA};

    ADI_CMS_NULL_PTR_CHECK(hmc7044);
    ADI_CMS_NULL_PTR_CHECK(hmc7044->hal_info.spi_write);
    ADI_CMS_NULL_PTR_CHECK(hmc7044->hal_info.spi_read);

    for (i = 0; i < 2; i++) {
        err = hmc7044_spi_reg_set(hmc7044, scratchpad_reg, reg_write[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = hmc7044_spi_reg_get(hmc7044, scratchpad_reg, &reg_read);
        ADI_CMS_ERROR_RETURN(err);

        ADI_CMS_CHECK((reg_read != reg_write[i]), API_CMS_ERROR_SPI_XFER);
    }

    return API_CMS_ERROR_OK;
}
