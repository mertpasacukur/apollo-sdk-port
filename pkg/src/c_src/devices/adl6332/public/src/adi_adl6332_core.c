/*!
 * \brief     ADL6332 Core Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_adl6332_core.h"
#include "adi_adl6332_hal.h"

#define __ADL6332_AMUX1_OPTIONS(muxout) ( \
       muxout == ADI_ADL6332_MUXOUT_AMP1_CELL_PTAT_BIAS_MONITOR \
    || muxout == ADI_ADL6332_MUXOUT_AMP2_CELL_PTAT_BIAS_MONITOR \
    || muxout == ADI_ADL6332_MUXOUT_AMP1_CELL_ZTAT_BIAS_MONITOR \
    || muxout == ADI_ADL6332_MUXOUT_AMP2_CELL_ZTAT_BIAS_MONITOR \
    || muxout == ADI_ADL6332_MUXOUT_AMP1_CELL_GAIN_EF_BIAS_MONITOR \
    || muxout == ADI_ADL6332_MUXOUT_AMP2_CELL_GAIN_EF_BIAS_MONITOR \
)

#define __ADL6332_AMUX2_OPTIONS(muxout) (muxout == ADI_ADL6332_MUXOUT_PTAT)

#define __ADL6332_AMUX3_OPTIONS(muxout) ( \
       muxout == ADI_ADL6332_MUXOUT_ADC_INPUT \
    || muxout == ADI_ADL6332_MUXOUT_ADC_CLK \
    || muxout == ADI_ADL6332_MUXOUT_V33_FUSE_SUPPLY_VOLTAGE_LEVEL \
    || muxout == ADI_ADL6332_MUXOUT_V33_AMP1_SUPPLY_VOLTAGE_LEVEL \
    || muxout == ADI_ADL6332_MUXOUT_V33_AMP2_SUPPLY_VOLTAGE_LEVEL \
    || muxout == ADI_ADL6332_MUXOUT_AMP1_INPUT_CM_VOLTAGE_LEVEL \
    || muxout == ADI_ADL6332_MUXOUT_AMP2_INPUT_CM_VOLTAGE_LEVEL \
)

#define SIG_PATH0_0_ADDR  0x100
#define LDO18_EN_OFFSET   0
#define AMUX_BG_EN_OFFSET 4

#define SIG_PATH1_0_ADDR  0x101
#define AMP1_EN_OFFSET    0
#define DSA_EN_OFFSET     1
#define AMP2_EN_OFFSET    2

#define AMUX_SEL_ADDR     0x120
#define AMUX_1_OFFSET     0
#define AMUX_2_OFFSET     3
#define AMUX_3_OFFSET     4

int32_t adi_adl6332_core_spi_init(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_spi_init_t *config)
{
    int32_t err;
    uint8_t val = 0;

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(config);

    err = adi_adl6332_hal_reg_set(adl6332, chip_id, 0x00, 0x81);
    ADI_CMS_ERROR_RETURN(err);

    if (config->sdo_active) {
        val |= 0x18;
    }

    if (config->big_endian) {
        val |= 0x24;
    }

    if (config->lsb_first) {
        val |= 0x42;
    }

    // We need 4 total SPI cycles after reset
    err = adi_adl6332_hal_reg_set(adl6332, chip_id, 0x00, val);
    ADI_CMS_ERROR_RETURN(err);

    // Remaining cycles will write to scratch.
    for (val = 0; val < 3; val++) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, 0x0A, val);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adl6332_core_init(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_init_t *config)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(config);

    err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH0_0_ADDR,
        (  (!!config->amux_enable)    << AMUX_BG_EN_OFFSET)
        |  (1 << 3) // ADC_EN
        | ((!!config->ldo_1p8_enable) << LDO18_EN_OFFSET));
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adl6332_hal_reg_set(adl6332, chip_id, SIG_PATH1_0_ADDR,
        (  (!!config->amp1_enable) << AMP1_EN_OFFSET)
        | ((!!config->amp2_enable) << AMP2_EN_OFFSET)
        | ((!!config->dsa_enable)  << DSA_EN_OFFSET));
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adl6332_core_muxout_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, adi_adl6332_muxout_e muxout)
{
    int32_t err;
    uint8_t amux_1 = 0;
    uint8_t amux_2 = 1;
    uint8_t amux_3 = 0;

    ADI_CMS_NULL_PTR_CHECK(adl6332);

    if (muxout != ADI_ADL6332_MUXOUT_NONE) {
        if (__ADL6332_AMUX2_OPTIONS(muxout)) {
            amux_2 = 0;
        } else if (__ADL6332_AMUX1_OPTIONS(muxout)) {
            amux_3 = 0;
            switch (muxout) {
                case ADI_ADL6332_MUXOUT_AMP1_CELL_PTAT_BIAS_MONITOR:
                    amux_1 = 2;
                    break;

                case ADI_ADL6332_MUXOUT_AMP2_CELL_PTAT_BIAS_MONITOR:
                    amux_1 = 3;
                    break;

                case ADI_ADL6332_MUXOUT_AMP1_CELL_ZTAT_BIAS_MONITOR:
                    amux_1 = 4;
                    break;

                case ADI_ADL6332_MUXOUT_AMP2_CELL_ZTAT_BIAS_MONITOR:
                    amux_1 = 5;
                    break;

                case ADI_ADL6332_MUXOUT_AMP1_CELL_GAIN_EF_BIAS_MONITOR:
                    amux_1 = 6;
                    break;

                case ADI_ADL6332_MUXOUT_AMP2_CELL_GAIN_EF_BIAS_MONITOR:
                    amux_1 = 7;
                    break;
                default:
                    return API_CMS_ERROR_ERROR;
            }
        } else if (__ADL6332_AMUX3_OPTIONS(muxout)) {
            amux_1 = 1;
            switch (muxout) {
                case ADI_ADL6332_MUXOUT_ADC_INPUT:
                    amux_3 = 1;
                    break;

                case ADI_ADL6332_MUXOUT_ADC_CLK:
                    amux_3 = 2;
                    break;

                case ADI_ADL6332_MUXOUT_V33_FUSE_SUPPLY_VOLTAGE_LEVEL:
                    amux_3 = 3;
                    break;

                case ADI_ADL6332_MUXOUT_V33_AMP1_SUPPLY_VOLTAGE_LEVEL:
                    amux_3 = 4;
                    break;

                case ADI_ADL6332_MUXOUT_V33_AMP2_SUPPLY_VOLTAGE_LEVEL:
                    amux_3 = 5;
                    break;

                case ADI_ADL6332_MUXOUT_AMP1_INPUT_CM_VOLTAGE_LEVEL:
                    amux_3 = 6;
                    break;

                case ADI_ADL6332_MUXOUT_AMP2_INPUT_CM_VOLTAGE_LEVEL:
                    amux_3 = 7;
                    break;
                default:
                    return API_CMS_ERROR_ERROR;
            }
        } else {
            return API_CMS_ERROR_ERROR;
        }
    }

    err = adi_adl6332_hal_reg_set(adl6332, chip_id, AMUX_SEL_ADDR,
        (  amux_1 << AMUX_1_OFFSET)
        | (amux_2 << AMUX_2_OFFSET)
        | (amux_3 << AMUX_3_OFFSET));
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adl6332_core_version_get(adi_adl6332_device_t *adl6332, adi_adl6332_version_t *apiVersion)
{
    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(apiVersion);

    apiVersion->major = 0;
    apiVersion->minor = 1;
    apiVersion->patch = 0;

    return API_CMS_ERROR_OK;
}

int32_t adi_adl6332_core_spi_reg_set(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, uint16_t address, uint8_t data)
{
    return adi_adl6332_hal_reg_set(adl6332, chip_id, address, data);
}

int32_t adi_adl6332_core_spi_reg_get(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id, uint16_t address, uint8_t *data)
{
    return adi_adl6332_hal_reg_get(adl6332, chip_id, address, data);
}

int32_t adi_adl6332_core_spi_reg_test(adi_adl6332_device_t *adl6332, adi_adl6332_chip_id_e chip_id)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint8_t i = 0;
    uint32_t scratchpad_reg = 0x0A;
    uint8_t reg_read = 0x00;
    uint8_t reg_write[] = {0x55, 0xAA};

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(adl6332->hal_info.spi_write);
    ADI_CMS_NULL_PTR_CHECK(adl6332->hal_info.spi_read);
    ADI_CMS_RANGE_CHECK(chip_id, ADI_ADL6332_CHIP_ID_0, ADI_ADL6332_CHIP_ID_7);

    for (i = 0; i < 2; i++) {
        err = adi_adl6332_hal_reg_set(adl6332, chip_id, scratchpad_reg, reg_write[i]);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adl6332_hal_reg_get(adl6332, chip_id, scratchpad_reg, &reg_read);
        ADI_CMS_ERROR_RETURN(err);

        ADI_CMS_CHECK((reg_read != reg_write[i]), API_CMS_ERROR_SPI_XFER);
    }

    return API_CMS_ERROR_OK;
}
