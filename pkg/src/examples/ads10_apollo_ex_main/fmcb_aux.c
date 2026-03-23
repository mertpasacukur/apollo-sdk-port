/*!
 * \brief     ADS10 Apollo FMCB Auxiliary Device SPI read back check
 *
 *            FMCB boards require level shifters to be enabled for SPI reads on SPI2 (Artemis and Yellowstone/Sequoia)
 *            Level Shifters are enabled by SDO_OEN_1 - SDO_OEN_5 (note that enables are inverted), connected to FPGA GPIOs
 *            The hal_spi_fmcb_config_t is stored as part of the spi struct, containing the relevant info to toggle the
 *            appropriate GPIOs before and after a spi read. SPI must be set for 4-wire mode on the ADS10 and the device.
 *
 *            The VGAs are accessed using the same SPI hal, so the appropriate level shifter must be determined from the
 *            first byte of the transaction - the VGA address. The VGA addresses and corresponding GPIOs are stored in
 *            the spi_byte_0 and target struct members. For Artemis, only one level shifter is needed, so set
 *            spi_byte_0[0] to 255 and use just target[0].
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "adi_apollo.h"
#include "adi_adl6331_core.h"
#include "adi_adl6332_core.h"
#include "adi_adf4382_core.h"
#include "adi_hmc7044_core.h"
#include "adi_hmc7044_device.h"
#include "adi_ads10_apollo_ex_adf4382.h"
#include "adi_ads10_apollo_ex_hmc7044.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "adi_ads10_apollo_ex_vga.h"
#include "ads10_fpga.h"

#define VGA_NUM 4

int32_t fmcb_aux(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[])
{
    int32_t err = API_CMS_ERROR_OK;
    int32_t i;
    adi_adl6331_device_t adl6331 = {{0}};
    adi_adl6332_device_t adl6332 = {{0}};
    adi_adf4382_device_t adf4382 = {{0}};
    adi_hmc7044_device_t hmc7044 = {{0}};
    uint8_t is_ext_clk = 0;

    adi_adl6331_chip_id_e ys_ids[VGA_NUM] = {ADI_ADL6331_CHIP_ID_1, ADI_ADL6331_CHIP_ID_3, ADI_ADL6331_CHIP_ID_0, ADI_ADL6331_CHIP_ID_2};
    adi_adl6332_chip_id_e sq_ids[VGA_NUM] = {ADI_ADL6332_CHIP_ID_4, ADI_ADL6332_CHIP_ID_6, ADI_ADL6332_CHIP_ID_7, ADI_ADL6332_CHIP_ID_5};

    if (is_ext_clk == 1) {
        err = adi_ads10_apollo_ex_adf4382_hal_config(&adf4382, NULL, &ads10_fpga_fmcb_aux_gpio_write);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_adf4382_core_init(&adf4382);
        ADI_CMS_ERROR_RETURN(err);

        printf("Verify Artemis SPI read/write...\n");
        err = adi_adf4382_core_spi_reg_test(&adf4382);
        ADI_CMS_ERROR_RETURN(err);
        printf("Passed.\n");


        err = adi_ads10_apollo_ex_hmc7044_hal_config(&hmc7044, NULL, &ads10_fpga_fmcb_aux_spi2_sdo_alt_enable_set);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_hmc7044_device_init(&hmc7044);
        ADI_CMS_ERROR_RETURN(err);

        printf("Verify HMC7044 SPI read/write...\n");
        err = adi_hmc7044_core_spi_reg_test(&hmc7044);
        ADI_CMS_ERROR_RETURN(err);

        printf("Passed.\n");
    }

    err = adi_ads10_apollo_ex_adl6331_hal_config(&adl6331, NULL, &ads10_fpga_fmcb_aux_gpio_write);
    ADI_CMS_ERROR_RETURN(err);

    if (adl6331.rev == 1) {
        printf("SPI readback not supported on FMCB Rev C.\n");
    } else {
        printf("Verify ADL6331 SPI read/write...\n");
        for (i = 0; i < VGA_NUM; i++) {
            err = adi_adl6331_core_spi_reg_test(&adl6331, ys_ids[i]);
            ADI_CMS_ERROR_RETURN(err);
        printf("Passed.\n");
        }
    }

    err = adi_ads10_apollo_ex_adl6332_hal_config(&adl6332, NULL, &ads10_fpga_fmcb_aux_gpio_write);
    ADI_CMS_ERROR_RETURN(err);

    if (adl6332.rev == 1) {
        printf("SPI readback not supported on FMCB Rev C.\n");
    } else {
        printf("Verify ADL6332 SPI read/write...\n");
        for (i = 0; i < VGA_NUM; i++) {
            err = adi_adl6332_core_spi_reg_test(&adl6332, sq_ids[i]);
            ADI_CMS_ERROR_RETURN(err);
        }
        printf("Passed.\n");
    }

    return err;

}
