/*!
 * \brief     CE board power modules example. Voltage and current measurements.
 *
 * \copyright copyright(c) 2021 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_ltc2977_core.h"
#include "adi_ltc2977_pmbus.h"
#include "adi_ltm4681_core.h"
#include "adi_ltm4681_pmbus.h"
#include "adi_ltc2980_core.h"
#include "adi_ltc2980_pmbus.h"
#include "ads10_hal.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_clk.h"
#include "adi_pmbus_format_conv.h"
#include "adi_ads10_apollo_ex_inspect.h"

#include "id00_uc06.h"          /* 20Gsps, jesd 10.3125Gbps */

static int32_t ltc2977_voltage_telemetry(adi_ltc2977_device_t *ltc2977);
static int32_t ltm4681_power_telemetry(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id);
static int32_t ltc2980_voltage_telemetry(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id);
static int32_t ltm4681_check_faults(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id);
static int32_t ltc2980_check_faults(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id);
static int32_t check_faults(adi_ltm4681_device_t *ltm4681, adi_ltc2980_device_t *ltc2980, bool has_ltc2980);
static __maybe_unused int32_t clear_faults(adi_ltm4681_device_t *ltm4681, adi_ltc2980_device_t *ltc2980, bool has_ltc2980);
static void ltc2980_a_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels);
static void ltc2980_b_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels);
static void ltc2980_c_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t vout_mode, uint8_t *ltc_page, uint16_t *ltc_read_vout, uint32_t ltc_num_page_channels);

int32_t main(int argc, char *argv[])
{
    int32_t err = API_CMS_ERROR_ERROR;
    adi_apollo_top_t *profile = &id00_uc06_0;     /* 20Gsps, jesd 10.3125Gbps */
    adi_apollo_device_t device = { 0 };             /* Apollo device object */
    adi_fpga_apollo_device_t fpga_device = { 0 };   /* FPGA device object */

    // Clocks to configure
    adi_ads10_apollo_clk_mode_e clk_mode = ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER
                                         | ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL;
    // Frequency going to the LTC6955 (J17 Frequency or 125MHz for onboard crystal).
    uint32_t ltc6955_clk_khz = 125e3;
    adi_apollo_hal_protocol_e protocol;
    bool enable_hsci = false;
    bool fault_checking = false;

    uint8_t has_ltc2980;
    uint16_t ltc_id;

    printf("Power Modules Example: Voltage and Current Measurement.\n");
    printf("*** Assume ADS10 FPGA image has been configured, power supplies up and ext clock on ***\n");

    adi_ads10_apollo_ex_inspect_lane_rates(&fpga_device, profile, clk_mode);

    /* Open ADS10 platform - This will init the FPGA SPI Master */
    if (err = ads10_hw_open("apollo_app.log"), err != API_CMS_ERROR_OK) {
        printf("ADS10 failed to open. Exiting...\n");
        return -1;
    }

    adi_ltc2977_device_t ltc2977 = {
        .hal_info = {
            .delay_us = &ads10_wait_us,
        },
        .smbus = {
            .read  = &ads10_i2c_read,
            .write = &ads10_i2c_write,
            .xfer  = NULL,
        },
    };

    adi_ltm4681_device_t ltm4681 = {
        .hal_info = {
            .delay_us = &ads10_wait_us,
        },
        .smbus = {
            .read  = &ads10_i2c_read,
            .write = &ads10_i2c_write,
            .xfer  = NULL,
        }
    };

    adi_ltc2980_device_t ltc2980 = {
        .hal_info = {
            .delay_us = &ads10_wait_us,
        },
        .smbus = {
            .read  = &ads10_i2c_read,
            .write = &ads10_i2c_write,
            .xfer  = NULL,
        },
    };

    /* Create an ADS10 platform instance. This provides FPGA register access. */
    adi_fpga_apollo_hal_config_t *ads10_platform = ads10_apollo_hal_instance();     // Allocates memory for the platform HAL data structure.

    ads10_ltc2977_hal_config_data(&ltc2977);
    ads10_ltc2980_hal_config_data(&ltc2980);
    ads10_ltm4681_hal_config_data(&ltm4681);

    /* Wire up the platform specific FPGA HAL for the FPGA API */
    adi_ads10_apollo_ex_configure_fpga_hal(&fpga_device, ads10_platform);

    /* Wire up the platform specific APOLLO HAL for the Apollo API */
    adi_ads10_apollo_ex_configure_hal(&device, ads10_platform, (uint8_t)enable_hsci);

    /* Setup the startup sequence info used by adi_apollo_startup_execute()  */
    adi_ads10_apollo_ex_configure_startup(&device);

    /* Perform any FPGA init before powering up Apollo (e.g. stop play/capture to reduce pwr) */
    adi_ads10_apollo_ex_fpga_pre_reset(&fpga_device);

    /* Open the Apollo API. This will execute required API initialization. */
    if (err = adi_apollo_device_hw_open(&device, ADI_APOLLO_HARD_RESET_AND_INIT), err != API_CMS_ERROR_OK) {
        printf("Error ocurred opening device. %d\n", err);
        return err;
    }

    /* Set the active driver to SPI0 */
    adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0);

    /* Disable the read-modify-write bitfield setting */
    adi_apollo_hal_rmw_enable_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0, 0);

    /* Print spi config reg (e.g. msb first, sdo, etc.) */
    adi_ads10_apollo_ex_print_regs(&device, 0x47000000, 1);

    /* Execute an optional ADS10 FPGA register access test */
    err = adi_ads10_apollo_ex_fpga_reg_test(&fpga_device);
    printf("FPGA register access test: %s\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    /*  Execute an optional Apollo register access test */
    err = adi_ads10_apollo_ex_reg_test(&device);
    printf("SPI test: %s\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    if (enable_hsci) {
        /* Set the active protocol to HSCI for remainder of execution */
        adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_HSCI);

        /*  Execute an optional Apollo register access test using HSCI */
        err = adi_ads10_apollo_ex_reg_test(&device);
        printf("HSCI reg test: %s\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    adi_apollo_hal_active_protocol_get(&device, &protocol);
    printf("Active Apollo HAL Protocol: %s\n", (protocol == 0) ? "SPI0" : "HSCI");

    /* Configure clks */
    if (err = adi_ads10_apollo_ex_configure_profile_clks(&fpga_device, ltc6955_clk_khz, profile, clk_mode),
        err != API_CMS_ERROR_OK) {
        printf("Error in adi_ads10_apollo_ex_configure_profile_clks(). err=%d\n", err);
        return err;
    }

    printf("Running Apollo startup sequence...\n");
    /* Load firmware image, device profile and configure digital data path */
    if (err = adi_ads10_apollo_ex_startup(&device, profile), err != API_CMS_ERROR_OK) {
        printf("Error in adi_ads10_apollo_ex_startup\n");
        return err;
    }

    /* Cal clk power level */
    if (err = adi_ads10_apollo_ex_clk_power_cal(&device, clk_mode, (uint64_t)(profile->clk_cfg.dev_clk_freq_kHz * 1e3), (uint64_t)(ltc6955_clk_khz * 1e3)), err != API_CMS_ERROR_OK) {
        printf("Error in clk power calibration. err = %d\n", err);
        return err;
    }

    /********************** EEPROM Board Name Read. **********************/

    printf("\n\n");
    printf("ltc2977 Addr: 0x%X.\n",    ltc2977.device_i2c_addr);

    printf("ltc2980_A Addr: 0x%X.\n", ltc2980.device_a_i2c_addr);
    printf("ltc2980_B Addr: 0x%X.\n", ltc2980.device_b_i2c_addr);
    printf("ltc2980_C Addr: 0x%X.\n", ltc2980.device_c_i2c_addr);

    printf("ltm4681_01 Addr: 0x%X.\n", ltm4681.chan_01_i2c_addr);
    printf("ltm4681_23 Addr: 0x%X.\n", ltm4681.chan_23_i2c_addr);

    char vendor[16], board[64], board_rev[16];
    if (err = ads10_i2c_board_name_get(0, vendor, board, board_rev), err == API_CMS_ERROR_OK) {

        if (strcmp(vendor, "Analog Devices") == 0) {
            printf("\nVendor: %s.\n", vendor);
            printf("Found connected board: %s Rev:%s.\n\n", board, board_rev);
        }
        else
            printf("\nWrong EEPROM Setup !!\n");
    }
    else
        printf("\nEEPROM Read Failed !!\n");

    if (err = adi_ltc2980_core_device_id_get(&ltc2980, ADI_LTC2980_SUB_DEVICE_A, &ltc_id, &has_ltc2980), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while reading manufacturer device id for LTC2980_A. %d\n", err);
        return err;
    }

    /********************** Voltage and Current Read. **********************/

    if (has_ltc2980) {
        if (err = ltc2980_voltage_telemetry(&ltc2980, ADI_LTC2980_SUB_DEVICE_A), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading output voltage values for LTC2980_A. %d\n", err);
            return err;
        }

        if (err = ltc2980_voltage_telemetry(&ltc2980, ADI_LTC2980_SUB_DEVICE_B), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading output voltage values for LTC2980_B. %d\n", err);
            return err;
        }

        if (err = ltc2980_voltage_telemetry(&ltc2980, ADI_LTC2980_SUB_DEVICE_C), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading output voltage values for LTC2980_C. %d\n", err);
            return err;
        }
    } else {
        if (err = ltc2977_voltage_telemetry(&ltc2977), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading output voltage values for LTC2977. %d\n", err);
            return err;
        }
    }

    if (err = ltm4681_power_telemetry(&ltm4681, ADI_LTM4681_CHAN_01), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while reading output power telemetry parameters for LTM4681_01. %d\n", err);
        return err;
    }

    if (err = ltm4681_power_telemetry(&ltm4681, ADI_LTM4681_CHAN_23), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while reading output power telemetry parameters for LTM4681_23. %d\n", err);
        return err;
    }

    /********************** Fault Check. **********************/

    if (fault_checking) {
        printf("Fault checking...\n");

        if (err = check_faults(&ltm4681, &ltc2980, has_ltc2980), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while checking faults. %d\n", err);
            return err;
        }
    }


    /********************** Reset. **********************/

    /********************** End. **********************/

    free(ads10_platform);
    ads10_platform = NULL;

    ads10_hw_close();
    printf("Goodbye\n");

    return API_CMS_ERROR_OK;
}

static int32_t check_faults(adi_ltm4681_device_t *ltm4681, adi_ltc2980_device_t *ltc2980, bool has_ltc2980) {
    int32_t err;

    if (err =  ltm4681_check_faults(ltm4681, ADI_LTM4681_CHAN_01), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while reading fault status for LTM4681_01. %d\n", err);
        return err;
    }

    if (err =  ltm4681_check_faults(ltm4681, ADI_LTM4681_CHAN_23), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while reading fault status for LTM4681_23. %d\n", err);
        return err;
    }

    if (has_ltc2980) {

        if (err =  ltc2980_check_faults(ltc2980, ADI_LTC2980_SUB_DEVICE_A), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading fault status for LTC2980A. %d\n", err);
            return err;
        }

        if (err =  ltc2980_check_faults(ltc2980, ADI_LTC2980_SUB_DEVICE_B), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading fault status for LTC2980B. %d\n", err);
            return err;
        }

        if (err =  ltc2980_check_faults(ltc2980, ADI_LTC2980_SUB_DEVICE_C), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while reading fault status for LTC2980C. %d\n", err);
            return err;
        }

    }

    return err;
}

static int32_t clear_faults(adi_ltm4681_device_t *ltm4681, adi_ltc2980_device_t *ltc2980, bool has_ltc2980) {
    int32_t err;

    if (err = adi_ltm4681_core_fault_status_clear(ltm4681, ADI_LTM4681_CHAN_01), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while clearing fault status for LTM4681_01. %d\n", err);
        return err;
    }

    if (err = adi_ltm4681_core_fault_status_clear(ltm4681, ADI_LTM4681_CHAN_23), err != API_CMS_ERROR_OK) {
        printf("Error ocurred while clearing fault status for LTM4681_23. %d\n", err);
        return err;
    }

    if (has_ltc2980) {

        if (err = adi_ltc2980_core_fault_status_clear(ltc2980, ADI_LTC2980_SUB_DEVICE_A), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while clearing fault status for LTC2980A. %d\n", err);
            return err;
        }

        if (err = adi_ltc2980_core_fault_status_clear(ltc2980, ADI_LTC2980_SUB_DEVICE_B), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while clearing fault status for LTC2980B. %d\n", err);
            return err;
        }

        if (err = adi_ltc2980_core_fault_status_clear(ltc2980, ADI_LTC2980_SUB_DEVICE_C), err != API_CMS_ERROR_OK) {
            printf("Error ocurred while clearing fault status for LTC2980C. %d\n", err);
            return err;
        }
    }

    return err;

}

static int32_t ltc2977_voltage_telemetry(adi_ltc2977_device_t *ltc2977)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t ltc_num_page_channels = 8;
    uint8_t ltc_page[8] = {0,1,2,3,4,5,6,7};
    uint8_t ltc_vout_mode = 0;
    uint16_t ltc_vout_l16[8] = {0};
    float ltc_read_vout[8] = {0.0};
    uint16_t vin_l11 = 0;
    float vin = 0;

    printf("\nLTC2977 Voltage Monitoring.\n");

    err = adi_ltc2977_pmbus_l11_read(ltc2977, 0x88, &vin_l11);    // 0x88 = PMBUS_CMD_READ_VIN
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(&vin_l11, &vin, 1);

    printf("\nLTC2977_READ_VIN: %.4f V.\n", vin);

    err = adi_ltc2977_pmbus_vout_mode_get(ltc2977, &ltc_vout_mode);
    ADI_CMS_ERROR_RETURN(err);
    ltc_vout_mode &= 0x1F;

    err = adi_ltc2977_core_vout_measure(ltc2977, ltc_page, ltc_vout_l16, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l16_to_float(ltc_vout_mode, ltc_vout_l16, ltc_read_vout, ltc_num_page_channels);

    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_VOUT: %.4f V.\n", ltc_read_vout[i]);
    }

    return API_CMS_ERROR_OK;
}

static int32_t ltm4681_power_telemetry(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id)
{
    int32_t err = API_CMS_ERROR_ERROR;
    char *ltm_chan_id = (chan_id == ADI_LTM4681_CHAN_01) ? "01" : "23";

    uint32_t ltm_num_page_channels = 2;
    uint8_t ltm_page[2] = {0,1};
    uint8_t ltm_vout_mode = 0;
    uint16_t ltm_vout_l16[2] = {0.0};
    float ltm_read_vout[2] = {0.0};
    uint16_t ltm_iout_l11[2] = {0.0};
    float ltm_read_iout[2] = {0.0};
    uint16_t ltm_pout_l11[2] = {0.0};
    float ltm_read_pout[2] = {0.0};
    uint16_t vin_l11 = 0;
    float vin = 0;

    printf("\nLTM4681_%s Power Monitoring.\n", ltm_chan_id);

    err = adi_ltm4681_pmbus_l11_read(ltm4681, chan_id, 0x88, &vin_l11);    // 0x88 = PMBUS_CMD_READ_VIN
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(&vin_l11, &vin, 1);
    printf("\nLTM4681_%s_READ_VIN: %.4f V.\n", ltm_chan_id, vin);

    err = adi_ltm4681_pmbus_vout_mode_get(ltm4681, chan_id, &ltm_vout_mode);
    ADI_CMS_ERROR_RETURN(err);
    ltm_vout_mode &= 0x1F;

    err = adi_ltm4681_core_vout_measure(ltm4681, chan_id, ltm_page, ltm_vout_l16, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l16_to_float(ltm_vout_mode, ltm_vout_l16, ltm_read_vout, ltm_num_page_channels);

    err = adi_ltm4681_core_iout_measure(ltm4681, chan_id, ltm_page, ltm_iout_l11, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(ltm_iout_l11, ltm_read_iout, ltm_num_page_channels);

    err = adi_ltm4681_core_pout_measure(ltm4681, chan_id, ltm_page, ltm_pout_l11, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(ltm_pout_l11, ltm_read_pout, ltm_num_page_channels);

    for (uint8_t i = 0; i < ltm_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltm_page[i]);
        printf("READ_VOUT: %.4f V.\t", ltm_read_vout[i]);
        printf("READ_IOUT: %.4f A.\t", ltm_read_iout[i]);
        printf("READ_POUT: %.4f W.\n", ltm_read_pout[i]);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}

static int32_t ltc2980_voltage_telemetry(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t ltc_num_page_channels = 8;
    uint8_t ltc_page[8] = {0,1,2,3,4,5,6,7};
    uint8_t ltc_vout_mode = 0;
    uint16_t ltc_vout_l16[8] = {0};
    float ltc_read_vout[8] = {0.0};
    uint16_t vin_l11 = 0;
    float vin = 0;
    char *ltc_sub_dev_id = (sub_dev_id == ADI_LTC2980_SUB_DEVICE_A) ? "A" :
                           (sub_dev_id == ADI_LTC2980_SUB_DEVICE_B) ? "B" : "C";

    printf("\nLTC2980_%s Voltage Monitoring.\n", ltc_sub_dev_id);
    err = adi_ltc2980_pmbus_l11_read(ltc2980, sub_dev_id, 0x88, &vin_l11);    // 0x88 = PMBUS_CMD_READ_VIN
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l11_to_float(&vin_l11, &vin, 1);

    printf("\nLTC2980_%s_READ_VIN: %.4f V.\n", ltc_sub_dev_id, vin);

    err = adi_ltc2980_pmbus_vout_mode_get(ltc2980, sub_dev_id, &ltc_vout_mode);
    ADI_CMS_ERROR_RETURN(err);
    ltc_vout_mode &= 0x1F;

    err = adi_ltc2980_core_vout_measure(ltc2980, sub_dev_id, ltc_page, ltc_vout_l16, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);
    adi_pmbus_format_conv_l16_to_float(ltc_vout_mode, ltc_vout_l16, ltc_read_vout, ltc_num_page_channels);

    switch (sub_dev_id) {
        case ADI_LTC2980_SUB_DEVICE_A:
           ltc2980_a_telemetry(ltc2980, &ltc_page[0], &ltc_read_vout[0], ltc_num_page_channels);
           break;
        case ADI_LTC2980_SUB_DEVICE_B:
           ltc2980_b_telemetry(ltc2980, &ltc_page[0], &ltc_read_vout[0], ltc_num_page_channels);
           break;
        case ADI_LTC2980_SUB_DEVICE_C:
           ltc2980_c_telemetry(ltc2980, ltc_vout_mode, &ltc_page[0], &ltc_vout_l16[0], ltc_num_page_channels);
           break;
        default:
            printf("Invalid sub device %d\n", sub_dev_id);
            return API_CMS_ERROR_INVALID_PARAM;
    }

    return API_CMS_ERROR_OK;
}

static void ltc2980_a_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels) {
    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_VOUT: %.4f V.\n", ltc_read_vout[i]);
    }
}

static void ltc2980_b_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t *ltc_page, float *ltc_read_vout, uint32_t ltc_num_page_channels) {
    const float voltage_current_scalars[8] = {3.3003299, 3.3003299, 2.5, 2.5, 15.6232166, 0.0, 0.0, 500.0};
    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_IOUT: %.9f A.\n", ltc_read_vout[i] * voltage_current_scalars[i]);
    }
}

// For device C, some channels are set to high resolution (see mfr_config_adc_hires bit in mfr_config), so they have L11 format with mA as unit
static void ltc2980_c_telemetry(adi_ltc2980_device_t *ltc2980, uint8_t vout_mode, uint8_t *ltc_page, uint16_t *ltc_read_vout, uint32_t ltc_num_page_channels) {
    const float voltage_current_scalars[8] = {0.0, 500.0, 3.3003299, 200.0, 0.0, 20.0, 0.0, 5.0};
    float value;
    char *unit;
    uint16_t mfr_cfg[8];
    adi_ltc2980_core_mfr_config_get(ltc2980, ADI_LTC2980_SUB_DEVICE_C, ltc_page, mfr_cfg, ltc_num_page_channels);

    for (uint8_t i=0; i < ltc_num_page_channels; ++i) {
        if (mfr_cfg[i] & 0x200) {
            adi_pmbus_format_conv_l11_to_float(&ltc_read_vout[i], &value, 1);
            unit = "mA";
        } else {
            adi_pmbus_format_conv_l16_to_float(vout_mode, &ltc_read_vout[i], &value, 1);
            unit = "A";
        }
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_IOUT: %.4f %s.\n", value * voltage_current_scalars[i], unit);
    }
}

static int32_t  ltm4681_check_faults(adi_ltm4681_device_t *ltm4681, adi_ltm4681_chan_pair_id_e chan_id)
{
    int32_t err = API_CMS_ERROR_ERROR;
    char *ltm_chan_id = (chan_id == ADI_LTM4681_CHAN_01) ? "01" : "23";

    uint32_t ltm_num_page_channels = 2;
    uint8_t ltm_page[2] = {0,1};
    adi_ltm4681_fault_status_t ltm_read_status[2] = {0};

    printf("\nLTM4681_%s Fault Monitoring.\n", ltm_chan_id);

    err = adi_ltm4681_core_fault_status_get(ltm4681, chan_id, ltm_page, ltm_read_status, ltm_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t i = 0; i < ltm_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltm_page[i]);
        printf("READ_STATUS: 0x%X VOUT: 0x%X IOUT: 0x%X INPUT: 0x%X TEMP: 0x%X CML: 0x%X MFR: 0x%X.\n", ltm_read_status[i].status_word, ltm_read_status[i].status_vout, ltm_read_status[i].status_iout, ltm_read_status[i].status_input, ltm_read_status[i].status_temp, ltm_read_status[i].status_cml, ltm_read_status[i].status_mfr);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}

static int32_t ltc2980_check_faults(adi_ltc2980_device_t *ltc2980, adi_ltc2980_sub_device_id_e sub_dev_id)
{
    int32_t err = API_CMS_ERROR_ERROR;
    char *ltc_sub_dev_id = (sub_dev_id == ADI_LTC2980_SUB_DEVICE_A) ? "A" :
                           (sub_dev_id == ADI_LTC2980_SUB_DEVICE_B) ? "B" : "C";

    uint32_t ltc_num_page_channels = 8;
    uint8_t ltc_page[8] = {0,1,2,3,4,5,6,7};
    adi_ltc2980_fault_status_t ltc_read_status[8] = {0};

    printf("\nLTC2980_%s Fault Monitoring.\n", ltc_sub_dev_id);

    err = adi_ltc2980_core_fault_status_get(ltc2980, sub_dev_id, ltc_page, ltc_read_status, ltc_num_page_channels);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t i = 0; i < ltc_num_page_channels; ++i) {
        printf("Page[%d]: %d.\t", i, ltc_page[i]);
        printf("READ_STATUS: 0x%X VOUT: 0x%X INPUT: 0x%X TEMP: 0x%X CML: 0x%X MFR: 0x%X.\n", ltc_read_status[i].status_word, ltc_read_status[i].status_vout, ltc_read_status[i].status_input, ltc_read_status[i].status_temp, ltc_read_status[i].status_cml, ltc_read_status[i].status_mfr);
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}
