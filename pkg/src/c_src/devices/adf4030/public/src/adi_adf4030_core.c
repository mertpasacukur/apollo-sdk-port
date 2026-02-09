/*!
 * \brief     Source file implementing APIs for ADF4030's basic core functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_CORE
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_core_types.h"
#include "adi_adf4030_hal.h"
#include "adi_adf4030_bf.h"

/*============= D A T A ====================*/
static uint16_t adf4030_api_version[3] = { 0, 1, 0 };       // {major, minor, patch}

/*============= C O D E ====================*/


int32_t adi_adf4030_core_api_ver_get(adi_adf4030_device_t *adf4030, adi_adf4030_core_version_t *apiVersion)
{
    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_NULL_CHECK(apiVersion);

    apiVersion->major = adf4030_api_version[0];
    apiVersion->minor = adf4030_api_version[1];
    apiVersion->patch = adf4030_api_version[2];

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_core_soft_reset(adi_adf4030_device_t *adf4030)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);

    // note: This bit clears automatically.
    err = adi_adf4030_bf___REG00FF___SOFTRESET_CHIP_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_core_init(adi_adf4030_device_t *adf4030)
{
    return adi_adf4030_hal_reg_default_set(adf4030);
}


int32_t adi_adf4030_core_power_up(adi_adf4030_device_t *adf4030)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);

    err = adi_adf4030_bf___REG003c___PD_ALL_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG003c___PD_PLL_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG003c___PD_TDC_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_core_wait_us(adi_adf4030_device_t *adf4030, uint32_t delay_us)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(adf4030->hal_info.delay_us);

    err = adf4030->hal_info.delay_us(adf4030->hal_info.user_data, delay_us);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_core_spi_reg_set(adi_adf4030_device_t *adf4030, uint16_t address, uint8_t data)
{
    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_NULL_CHECK(adf4030->hal_info.spi_write);

    return adi_adf4030_hal_reg_set(adf4030, address, data);
}


int32_t adi_adf4030_core_spi_reg_get(adi_adf4030_device_t *adf4030, uint16_t address, uint8_t *data)
{
    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_NULL_CHECK(adf4030->hal_info.spi_read);

    return adi_adf4030_hal_reg_get(adf4030, address, data);
}


int32_t adi_adf4030_core_spi_reg_test(adi_adf4030_device_t *adf4030)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint8_t i = 0;
    uint32_t scratchpad_reg = 0x0A;
    uint8_t reg_read = 0x00;
    uint8_t reg_write[] = {0x55, 0xAA};

    ADI_CMS_NULL_PTR_CHECK(adf4030);
    ADI_CMS_NULL_PTR_CHECK(adf4030->hal_info.spi_write);
    ADI_CMS_NULL_PTR_CHECK(adf4030->hal_info.spi_read);

    for (i = 0; i < 2; i++) {
        err = adi_adf4030_hal_reg_set(adf4030, scratchpad_reg, reg_write[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4030_hal_reg_get(adf4030, scratchpad_reg, &reg_read);
        ADI_CMS_ERROR_RETURN(err);

        ADI_CMS_CHECK((reg_read != reg_write[i]), API_CMS_ERROR_SPI_XFER);
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_core_die_temp_get(adi_adf4030_device_t *adf4030, uint16_t *die_temp)
{
    int32_t err;
    uint8_t reg_read = 0;
    uint8_t reg_write = 0;
    uint8_t adc_busy = 0;
    uint8_t timeout = 0xFF;
    err = adi_adf4030_core_spi_reg_get(adf4030, 0x61, &reg_read);
    ADI_ADF4030_CHECK_ERR_OK(err);

    // REG0061___EN_ADC | REG0061___EN_ADC_CLK | REG0061___EN_ADC_CNV | REG0061___ADC_CLK_TEST_SEL
    reg_write = reg_read | ((1 << 0) | (1 << 1) | (1 << 2) | (0 << 6));

    err = adi_adf4030_core_spi_reg_set(adf4030, 0x61, reg_write);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG003c___PD_ADC_set(adf4030, 0);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG0072___ADC_ST_CNV_set(adf4030, 1);
    ADI_ADF4030_CHECK_ERR_OK(err);

    err = adi_adf4030_bf___REG008f___ADC_BUSY_get(adf4030, &adc_busy);
    ADI_ADF4030_CHECK_ERR_OK(err);

    while (adc_busy) {
        err = adi_adf4030_core_wait_us(adf4030, 100);
        ADI_ADF4030_CHECK_ERR_OK(err);

        err = adi_adf4030_bf___REG008f___ADC_BUSY_get(adf4030, &adc_busy);
        ADI_ADF4030_CHECK_ERR_OK(err);

        if (!(timeout--)) {
            break;
        }
    }

    err = adi_adf4030_bf___REG0092___TEMP_MEAS_get(adf4030, die_temp);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


/*! @} */
