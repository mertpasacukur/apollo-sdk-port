#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     CE board power modules example. Voltage and current measurements.
 *
 * \copyright copyright(c) 2025 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_ads10_apollo_ex_power_modules.h"


static int32_t pwr_modules_telemetry(ce_brd_pwr_modules_t *pwr_mods, uint8_t has_ltc2980);
static int32_t pwr_modules_status_faults_check(ce_brd_pwr_modules_t *pwr_mods, uint8_t has_ltc2980);
static __maybe_unused int32_t pwr_modules_status_faults_clear(ce_brd_pwr_modules_t *pwr_mods, uint8_t has_ltc2980);

int32_t power_readback(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst)
{
    int32_t err;
    ce_brd_pwr_modules_t pwr_mods = {{{0}}};
    uint8_t has_ltc2980;
    uint16_t ltc_id;

    err = adi_ex_pwr_modules_hal_config(&pwr_mods);
    ADI_CMS_ERROR_RETURN(err);

    // Check if CE board has LTC2980 or LTC2977
    err = adi_ltc2980_core_device_id_get(&pwr_mods.ltc2980, ADI_LTC2980_SUB_DEVICE_A, &ltc_id, &has_ltc2980);
    ADI_CMS_ERROR_RETURN(err);

    printf("Power Module Address:\n");
    if (has_ltc2980) {
        printf("ltc2980_A Addr: 0x%X.\n", pwr_mods.ltc2980.device_a_i2c_addr);
        printf("ltc2980_B Addr: 0x%X.\n", pwr_mods.ltc2980.device_b_i2c_addr);
        printf("ltc2980_C Addr: 0x%X.\n", pwr_mods.ltc2980.device_c_i2c_addr);
    } else {
        printf("ltc2977 Addr: 0x%X.\n", pwr_mods.ltc2977.device_i2c_addr);
    }
    printf("ltm4681_01 Addr: 0x%X.\n", pwr_mods.ltm4681.chan_01_i2c_addr);
    printf("ltm4681_23 Addr: 0x%X.\n", pwr_mods.ltm4681.chan_23_i2c_addr);

    /********************** Read Voltage and Current Telemetry. **********************/
    err = pwr_modules_telemetry(&pwr_mods, has_ltc2980);
    ADI_CMS_ERROR_RETURN(err);

    /********************** Check Status Faults. **********************/
    err = pwr_modules_status_faults_check(&pwr_mods, has_ltc2980);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

static int32_t pwr_modules_telemetry(ce_brd_pwr_modules_t *pwr_mods, uint8_t has_ltc2980)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(pwr_mods);
    ADI_CMS_INVALID_PARAM_CHECK(has_ltc2980 > 1);

    if (has_ltc2980) {
        err = adi_ex_pwr_modules_ltc2980_voltage_telemetry_print(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_A);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ex_pwr_modules_ltc2980_voltage_telemetry_print(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_B);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ex_pwr_modules_ltc2980_voltage_telemetry_print(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_C);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_ex_pwr_modules_ltc2977_voltage_telemetry_print(&pwr_mods->ltc2977);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_ex_pwr_modules_ltm4681_power_telemetry_print(&pwr_mods->ltm4681, ADI_LTM4681_CHAN_01);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ex_pwr_modules_ltm4681_power_telemetry_print(&pwr_mods->ltm4681, ADI_LTM4681_CHAN_23);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

static int32_t pwr_modules_status_faults_check(ce_brd_pwr_modules_t *pwr_mods, uint8_t has_ltc2980)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(pwr_mods);
    ADI_CMS_INVALID_PARAM_CHECK(has_ltc2980 > 1);

    if (has_ltc2980) {
        err = adi_ex_pwr_modules_ltc2980_status_faults_check_print(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_A);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ex_pwr_modules_ltc2980_status_faults_check_print(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_B);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ex_pwr_modules_ltc2980_status_faults_check_print(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_B);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_ex_pwr_modules_ltc2977_status_faults_check_print(&pwr_mods->ltc2977);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_ex_pwr_modules_ltm4681_status_faults_check_print(&pwr_mods->ltm4681, ADI_LTM4681_CHAN_01);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ex_pwr_modules_ltm4681_status_faults_check_print(&pwr_mods->ltm4681, ADI_LTM4681_CHAN_23);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

static int32_t pwr_modules_status_faults_clear(ce_brd_pwr_modules_t *pwr_mods, uint8_t has_ltc2980)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(pwr_mods);
    ADI_CMS_INVALID_PARAM_CHECK(has_ltc2980 > 1);

    if (has_ltc2980) {
        err = adi_ltc2980_core_fault_status_clear(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_A);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ltc2980_core_fault_status_clear(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_B);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_ltc2980_core_fault_status_clear(&pwr_mods->ltc2980, ADI_LTC2980_SUB_DEVICE_B);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_ltc2977_core_fault_status_clear(&pwr_mods->ltc2977);
        ADI_CMS_ERROR_RETURN(err);
    }

    err = adi_ltm4681_core_fault_status_clear(&pwr_mods->ltm4681, ADI_LTM4681_CHAN_01);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ltm4681_core_fault_status_clear(&pwr_mods->ltm4681, ADI_LTM4681_CHAN_23);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

#endif /* !defined(VERSAL_PLATFORM) */
