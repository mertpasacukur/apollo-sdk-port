/*!
 * \brief     ADS10 Apollo FMCB EvalBoard Auxiliary Device config functions implementation
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_adl6331_core.h"
#include "adi_adl6332_core.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_gpio.h"
#include "adi_ads10_apollo_ex_vga.h"
#include "ads10_fpga.h"


/*============= D E F I N E S ==============*/
#define MAX_ADL6331_PER_BUS     8U      // Up to eight ADL6331 devices can be addressed with the same SPI by using a single CSB line
#define ATTSEL0_TX_GPIO_IDX     25U     // FPGA GPIO index mapped to Tx ATTSEL0
#define ATTSEL1_TX_GPIO_IDX     26U     // FPGA GPIO index mapped to Tx ATTSEL1
#define ATTSEL0_RX_GPIO_IDX     27U     // FPGA GPIO index mapped to Rx ATTSEL0
#define ATTSEL1_RX_GPIO_IDX     28U     // FPGA GPIO index mapped to Rx ATTSEL1


/*==================== P U B L I C   A P I   C O D E ====================*/
int32_t adi_ads10_apollo_ex_tx_rx_vga_initialize(adi_fpga_apollo_device_t *fpga_device)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);

    uint8_t i = 0;
    const uint8_t vga_num = 4;                    // Num Tx / Rx VGA on FMCB
    const uint8_t attsel0_tx_gpio_idx = 25;       // FPGA GPIO index mapped to Tx ATTSEL0
    const uint8_t attsel1_tx_gpio_idx = 26;       // FPGA GPIO index mapped to Tx ATTSEL1
    const uint8_t attsel0_rx_gpio_idx = 27;       // FPGA GPIO index mapped to Rx ATTSEL0
    const uint8_t attsel1_rx_gpio_idx = 28;       // FPGA GPIO index mapped to Rx ATTSEL1

    adi_adl6331_device_t adl6331 = {{0}};
    adi_adl6332_device_t adl6332 = {{0}};

    adi_adl6331_chip_id_e tx_vga_ids[4] = { ADI_ADL6331_CHIP_ID_0,
                                            ADI_ADL6331_CHIP_ID_1,
                                            ADI_ADL6331_CHIP_ID_2,
                                            ADI_ADL6331_CHIP_ID_3 };
    adi_adl6332_chip_id_e rx_vga_ids[4] = { ADI_ADL6332_CHIP_ID_4,
                                            ADI_ADL6332_CHIP_ID_5,
                                            ADI_ADL6332_CHIP_ID_6,
                                            ADI_ADL6332_CHIP_ID_7 };

    adi_adl6331_rfstate_select_e tx_vga_rfstate = ADI_ADL6331_RFSTATE_SELECT_A;     // ATTSEL0_TX and ATTSEL1_TX set to 0
    adi_adl6331_rfstate_t tx_vga_rf_gain_config = {
        .amp1_bypass = 0,   //  AMP1 set in fixed gain mode for RF State A
        .amp2_bypass = 0,   //  AMP2 also set in fixed gain mode for RF State A
        .dsa_attn = 12      //  DSA Attenuation set to 12 dB
    };

    adi_adl6332_rfstate_select_e rx_vga_rfstate = ADI_ADL6332_RFSTATE_SELECT_A;     // ATTSEL0_RX and ATTSEL1_RX set to 0
    adi_adl6332_rfstate_t rx_vga_rf_gain_config = {
        .amp1_bypass = 0,   //  AMP1 set in fixed gain mode for RF State A
        .amp2_bypass = 0,   //  AMP2 also set in fixed gain mode for RF State A
        .dsa_attn = 12      //  DSA Attenuation set to 12 dB
    };

    // Set FPGA GPIOs 25 and 26 as Output and maps ATTSEL0 and ATTSEL1 for ADL6331.
    err = adi_fpga_apollo_gpio_output_set(fpga_device, attsel0_tx_gpio_idx, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_gpio_output_set(fpga_device, attsel1_tx_gpio_idx, 0);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIOs 27 and 28 as Output and maps ATTSEL0 and ATTSEL1 for ADL6332.
    err = adi_fpga_apollo_gpio_output_set(fpga_device, attsel0_rx_gpio_idx, 0);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_gpio_output_set(fpga_device, attsel1_rx_gpio_idx, 0);
    ADI_CMS_ERROR_RETURN(err);

    // Initializes HAL and startup ADL6331 abd ADL6332 VGAs.
    err = adi_ads10_apollo_ex_tx_rx_vga_startup(&adl6331, &adl6332, tx_vga_ids, rx_vga_ids, vga_num);
    ADI_CMS_ERROR_RETURN(err);

    // Set RF gain settings for Amp1, Amp2 and DSA Attenuator.
    for (i = 0; i < vga_num; i++) {
        err = adi_adl6331_sigpath_rfstate_set(&adl6331, tx_vga_ids[i], tx_vga_rfstate, &tx_vga_rf_gain_config);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adl6332_sigpath_rfstate_set(&adl6332, rx_vga_ids[i], rx_vga_rfstate, &rx_vga_rf_gain_config);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_adl6331_hal_config(adi_adl6331_device_t *adl6331, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn)
{
    int32_t err;
    int8_t idx;
    uint8_t spi_wire_mode;
    adi_adl6331_hal_t *hal = &adl6331->hal_info;
    hal_spi_config_t *spi_cfg;

    adi_adl6331_chip_id_e chip_ids[4] = { ADI_ADL6331_CHIP_ID_0,
                                          ADI_ADL6331_CHIP_ID_1,
                                          ADI_ADL6331_CHIP_ID_2,
                                          ADI_ADL6331_CHIP_ID_3 };

    ADI_CMS_NULL_PTR_CHECK(adl6331);
    ADI_CMS_NULL_PTR_CHECK(sdo_en_fcn);

    err = ads10_adl6331_hal_config_data(adl6331, &ads10_adl6331_hal_spi_config_init);
    ADI_CMS_ERROR_RETURN(err);

    hal->delay_us = &ads10_wait_us;
    hal->spi_read = &ads10_cms_spi_read;
    hal->spi_write = &ads10_cms_spi_write;
    spi_cfg = (hal_spi_config_t *)hal->user_data;

    // Set ADL6331 SPI Mode
    spi_wire_mode = (spi_cfg->spi_3wire == 0) ? 0x18 : 0x00;
    for (idx = 0; idx < 4; idx++) {
        err = adi_adl6331_core_spi_reg_set(adl6331, chip_ids[idx], 0x00, spi_wire_mode);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Check if SPI read is functional
    err = adi_adl6331_core_spi_reg_test(adl6331, chip_ids[0]);

    // If SPI read fails, configure for FMCB SPI read
    if (err == API_CMS_ERROR_SPI_XFER) {
        printf("\n");
        printf("*** WARNING: ADL6331 SPI Reg Test Failed!! ***\n");
        printf("Configuring level shifters and Retrying...\n");

        // Configure FMCB SPI read
        hal->spi_read = &ads10_cms_fmcb_spi_read;
        hal_spi_fmcb_config_t *fmcb_cfg = (hal_spi_fmcb_config_t *) calloc(1, sizeof(hal_spi_fmcb_config_t));
        ADI_CMS_MEM_ALLOC_CHECK(fmcb_cfg);

        // ADI_ADL6331_CHIP_ID_0
        fmcb_cfg->spi_byte_0[0] = 0x80;
        fmcb_cfg->target[0] = SDO_OEN_1;
        // ADI_ADL6331_CHIP_ID_1
        fmcb_cfg->spi_byte_0[1] = 0x88;
        fmcb_cfg->target[1] = SDO_OEN_1;
        // ADI_ADL6331_CHIP_ID_2
        fmcb_cfg->spi_byte_0[2] = 0x90;
        fmcb_cfg->target[2] = SDO_OEN_2;
        fmcb_cfg->spi_byte_0[3] = 0x98;
        // ADI_ADL6331_CHIP_ID_3
        fmcb_cfg->target[3] = SDO_OEN_2;

        fmcb_cfg->sdo_en_context = sdo_en_context;
        fmcb_cfg->sdo_en_fcn = sdo_en_fcn;

        spi_cfg->user_data = fmcb_cfg;
        spi_cfg->spi_3wire = 0; // Set to 4 wire mode

        for (idx = 0; idx < 4; idx++) {
            err = adi_adl6331_core_spi_reg_set(adl6331, chip_ids[idx], 0x00, 0x18);
            ADI_CMS_ERROR_RETURN(err);
        }

        // Verify read/write
        for (idx = 0; idx < 4; idx++) {
            err = adi_adl6331_core_spi_reg_test(adl6331, chip_ids[idx]);
            printf("ADL6331, Chip Id: 0x%02X, SPI Reg Test: %s.\n", chip_ids[idx], (err == 0) ? "PASSED" : "FAILED");
            ADI_CMS_ERROR_RETURN(err);
        }
        printf("\n");
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_adl6332_hal_config(adi_adl6332_device_t *adl6332, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn)
{
    int32_t err;
    int8_t idx;
    uint8_t spi_wire_mode;
    adi_adl6332_hal_t *hal = &adl6332->hal_info;
    hal_spi_config_t *spi_cfg;

    adi_adl6332_chip_id_e chip_ids[4] = { ADI_ADL6332_CHIP_ID_4,
                                          ADI_ADL6332_CHIP_ID_5,
                                          ADI_ADL6332_CHIP_ID_6,
                                          ADI_ADL6332_CHIP_ID_7 };

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(sdo_en_fcn);

    err = ads10_adl6332_hal_config_data(adl6332, &ads10_adl6332_hal_spi_config_init);
    ADI_CMS_ERROR_RETURN(err);

    hal->delay_us = &ads10_wait_us;
    hal->spi_read = &ads10_cms_spi_read;
    hal->spi_write = &ads10_cms_spi_write;
    spi_cfg = (hal_spi_config_t *)hal->user_data;

    // Set ADL6332 SPI mode
    spi_wire_mode = (spi_cfg->spi_3wire == 0) ? 0x18 : 0x00;
    for (idx = 0; idx < 4; idx++) {
        err = adi_adl6332_core_spi_reg_set(adl6332, chip_ids[idx], 0x00, spi_wire_mode);
        ADI_CMS_ERROR_RETURN(err);
    }

    // Check if SPI read is functional
    err = adi_adl6332_core_spi_reg_test(adl6332, chip_ids[0]);

    // If SPI read fails, configure for FMCB SPI read
    if (err == API_CMS_ERROR_SPI_XFER) {
        printf("\n");
        printf("*** WARNING: ADL6332 SPI Reg Test Failed!! ***\n");
        printf("Configuring level shifters and Retrying...\n");

        // Configure FMCB SPI read
        hal->spi_read = &ads10_cms_fmcb_spi_read;
        hal_spi_fmcb_config_t *fmcb_cfg = (hal_spi_fmcb_config_t *) calloc(1, sizeof(hal_spi_fmcb_config_t));
        ADI_CMS_MEM_ALLOC_CHECK(fmcb_cfg);

        // ADI_ADL6332_CHIP_ID_4
        fmcb_cfg->spi_byte_0[0] = 0xA0;
        fmcb_cfg->target[0] = SDO_OEN_3;
        // ADI_ADL6332_CHIP_ID_5
        fmcb_cfg->spi_byte_0[1] = 0xA8;
        fmcb_cfg->target[1] = SDO_OEN_3;
        // ADI_ADL6332_CHIP_ID_6
        fmcb_cfg->spi_byte_0[2] = 0xB0;
        fmcb_cfg->target[2] = SDO_OEN_4;
        // ADI_ADL6332_CHIP_ID_7
        fmcb_cfg->spi_byte_0[3] = 0xB8;
        fmcb_cfg->target[3] = SDO_OEN_4;

        fmcb_cfg->sdo_en_context = sdo_en_context;
        fmcb_cfg->sdo_en_fcn = sdo_en_fcn;

        spi_cfg->user_data = fmcb_cfg;
        spi_cfg->spi_3wire = 0; // Set to 4 wire mode

        for (idx = 0; idx < 4; idx++) {
            err = adi_adl6332_core_spi_reg_set(adl6332, chip_ids[idx], 0x00, 0x18);
            ADI_CMS_ERROR_RETURN(err);
        }

        // Verify read/write
        for (idx = 0; idx < 4; idx++) {
            err = adi_adl6332_core_spi_reg_test(adl6332, chip_ids[idx]);
            printf("ADL6332, Chip Id: 0x%02X, SPI Reg Test: %s.\n", chip_ids[idx], (err == 0) ? "PASSED" : "FAILED");
            ADI_CMS_ERROR_RETURN(err);
        }
        printf("\n");
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_tx_rx_vga_startup(adi_adl6331_device_t *adl6331,
                                              adi_adl6332_device_t *adl6332,
                                              adi_adl6331_chip_id_e tx_vga_ids[],
                                              adi_adl6332_chip_id_e rx_vga_ids[],
                                              uint8_t num_vgas)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adl6331);
    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(tx_vga_ids);
    ADI_CMS_NULL_PTR_CHECK(rx_vga_ids);

    uint8_t i = 0;
    adi_apollo_ex_adl6331_startup_t tx_vga_startup = {
        .spi_init = {
            .big_endian = 1,
            .sdo_active = 1,
            .lsb_first = 0
        },
        .core_init = {
            .amp1_enable = 1,
            .amp2_enable = 1,
            .amux_enable = 1,
            .dsa_enable = 1,
            .ldo_1p8_enable = 1
        },
        .amp = ADI_ADL6331_AMP_SELECT_ALL,
        .amp_trim_config = {
            .igref = ADI_ADL6331_MAIN_BIAS_TRM_240mA,
            .idref_z = ADI_ADL6331_ZTAT_BIAS_TRM_1p40mA,
            .idref_p = ADI_ADL6331_PTAT_BIAS_TRM_480uA,
            .cross_z = ADI_ADL6331_ZTAT_BIAS_TRM_1p64mA,
            .cross_p = ADI_ADL6331_PTAT_BIAS_TRM_640uA,
            .use_nvm_igref = 0,
            .use_nvm_idref_z = 0,
            .use_nvm_idref_p = 0,
        }
    };

    adi_apollo_ex_adl6332_startup_t rx_vga_startup = {
        .spi_init = {
            .big_endian = 1,
            .sdo_active = 1,
            .lsb_first = 0
        },
        .core_init = {
            .amp1_enable = 1,
            .amp2_enable = 1,
            .amux_enable = 1,
            .dsa_enable = 1,
            .ldo_1p8_enable = 1
        },
        .amp = ADI_ADL6331_AMP_SELECT_ALL,
        .amp_trim_config = {
            .igref = ADI_ADL6332_MAIN_BIAS_TRM_240mA,
            .idref_z = ADI_ADL6332_ZTAT_BIAS_TRM_1p40mA,
            .idref_p = ADI_ADL6332_PTAT_BIAS_TRM_480uA,
            .cross_z = ADI_ADL6332_ZTAT_BIAS_TRM_1p64mA,
            .cross_p = ADI_ADL6332_PTAT_BIAS_TRM_640uA,
            .use_nvm_igref = 0,
            .use_nvm_idref_z = 0,
            .use_nvm_idref_p = 0,
        }
    };

    // Configure HAL settings for ADL6331 and ADL6332.
    err = adi_ads10_apollo_ex_adl6331_hal_config(adl6331, NULL, &ads10_fpga_fmcb_aux_gpio_write);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_ex_adl6332_hal_config(adl6332, NULL, &ads10_fpga_fmcb_aux_gpio_write);
    ADI_CMS_ERROR_RETURN(err);

    // Initialize devices for startup.
    for (i = 0; i < num_vgas; i++) {
        err = adi_ads10_apollo_ex_adl6331_startup(adl6331, tx_vga_ids[i], &tx_vga_startup);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ads10_apollo_ex_adl6332_startup(adl6332, rx_vga_ids[i], &rx_vga_startup);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_adl6331_startup(adi_adl6331_device_t *adl6331,
                                            adi_adl6331_chip_id_e chip_id,
                                            adi_apollo_ex_adl6331_startup_t *dev_startup)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adl6331);
    ADI_CMS_NULL_PTR_CHECK(dev_startup);
    ADI_CMS_NULL_PTR_CHECK(&dev_startup->spi_init);
    ADI_CMS_NULL_PTR_CHECK(&dev_startup->core_init);
    ADI_CMS_NULL_PTR_CHECK(&dev_startup->amp_trim_config);
    ADI_CMS_RANGE_CHECK(chip_id, ADI_ADL6331_CHIP_ID_0, ADI_ADL6331_CHIP_ID_7);

    err = adi_adl6331_core_spi_init(adl6331, chip_id, &dev_startup->spi_init);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adl6331_core_init(adl6331, chip_id, &dev_startup->core_init);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adl6331_sigpath_trim_config_set(adl6331, chip_id, dev_startup->amp, &dev_startup->amp_trim_config);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_adl6332_startup(adi_adl6332_device_t *adl6332,
                                            adi_adl6332_chip_id_e chip_id,
                                            adi_apollo_ex_adl6332_startup_t *dev_startup)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(adl6332);
    ADI_CMS_NULL_PTR_CHECK(dev_startup);
    ADI_CMS_NULL_PTR_CHECK(&dev_startup->spi_init);
    ADI_CMS_NULL_PTR_CHECK(&dev_startup->core_init);
    ADI_CMS_NULL_PTR_CHECK(&dev_startup->amp_trim_config);
    ADI_CMS_RANGE_CHECK(chip_id, ADI_ADL6332_CHIP_ID_0, ADI_ADL6332_CHIP_ID_7);

    err = adi_adl6332_core_spi_init(adl6332, chip_id, &dev_startup->spi_init);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adl6332_core_init(adl6332, chip_id, &dev_startup->core_init);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_adl6332_sigpath_trim_config_set(adl6332, chip_id, dev_startup->amp, &dev_startup->amp_trim_config);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}
