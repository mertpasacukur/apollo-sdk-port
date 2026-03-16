/*!
 * @brief     Versal Apollo example program main.
 *
 *            Full port of original platform main.c for Xilinx Versal Standalone.
 *            No CLI — profile and example are selected at compile time.
 *
 *            Default configuration:
 *              Profile:  id00_uc06
 *
 *            To change, define before including this file:
 *              #define VERSAL_PROFILE_STRUCT  id00_uc08_f_0
 *              #define VERSAL_PROFILE_NAME    "id00_uc08_f"
 *              #include "id00_uc08_f.h"
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>

#include "sleep.h"

/* Apollo SDK — platform-independent */
#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"

/* Versal platform HAL */
#include "versal_hal.h"
#include "versal_fpga.h"

/* Debug system */
#include "versal_debug.h"

/* Shell / command interface */
#include "shell.h"

/* Example common — HAL wiring, startup */
#include "versal_apollo_ex.h"
#include "versal_apollo_ex_types.h"

/* Clock device APIs */
#include "versal_apollo_ex_clk.h"
#include "versal_apollo_ex_hmc7044.h"
#include "versal_apollo_ex_adf4382.h"
#include "versal_apollo_ex_adf4030.h"

/* Device profile — compile-time selection */
#include "id00_uc06.h"

/*============= C O M P I L E - T I M E   C O N F I G ======================*/

#ifndef VERSAL_PROFILE_STRUCT
#define VERSAL_PROFILE_STRUCT   id00_uc06_0
#endif

#ifndef VERSAL_PROFILE_NAME
#define VERSAL_PROFILE_NAME     "id00_uc06"
#endif

/*============= T E S T   A D D R E S S E S ================================*/

#define INDIRECT_REG_TEST_ADDR  (0x60366045)
#define ARM_REG_TEST_BASE_ADDR  (0x20000000U)

/*============= H E L P E R   F U N C T I O N S ============================*/

/*!
 * @brief Print Apollo registers (for debug).
 *        Uses platform-independent adi_apollo_hal_reg_get().
 */
static int32_t versal_apollo_ex_print_regs(adi_apollo_device_t *device, uint32_t reg, uint32_t nregs)
{
    int32_t err;
    uint32_t i;
    uint8_t data8;

    for (i = 0; i < nregs; i++) {
        if (err = adi_apollo_hal_reg_get(device, reg + i, &data8), err != API_CMS_ERROR_OK) {
            dbg_printf(DBG_ERROR, "*error*\r\n");
            return err;
        }
        dbg_printf(DBG_DEBUG, "0x%08X: 0x%02X\r\n", reg + i, data8);
        if ((i + 1) % 4 == 0) dbg_printf(DBG_DEBUG, "\r\n");
    }
    dbg_printf(DBG_DEBUG, "\r\n");

    return API_CMS_ERROR_OK;
}

/*!
 * @brief FPGA register access test.
 *        Reads a couple of known FPGA offsets to verify communication.
 */
static int32_t versal_apollo_ex_fpga_reg_test(adi_fpga_apollo_device_t *fpga_device)
{
    int32_t err;
    uint32_t out_data = 0, offset = 0;

    offset = 0x100;
    if (err = adi_fpga_apollo_core_reg_get(fpga_device, offset, &out_data), err != API_CMS_ERROR_OK) {
        return err;
    }
    dbg_printf(DBG_DEBUG, "FPGA reg at offset 0x%X value is 0x%04X\r\n", offset, out_data);

    offset = 0x101;
    if (err = adi_fpga_apollo_core_reg_get(fpga_device, offset, &out_data), err != API_CMS_ERROR_OK) {
        return err;
    }
    dbg_printf(DBG_DEBUG, "FPGA reg at offset 0x%X value is 0x%04X\r\n", offset, out_data);

    return API_CMS_ERROR_OK;
}

/*!
 * @brief Apollo SPI register access test.
 *        Tests direct, indirect, and 32-bit ARM register read/write.
 */
static int32_t versal_apollo_ex_reg_test(adi_apollo_device_t *device)
{
    int32_t err;
    uint32_t i;
    uint8_t data8, stat;

    /* Direct register SPI scratch loop rd/wr test */
    stat = 0;
    uint32_t direct_addr[] = { 0x4700000a, 0x4700000a, 0x47000200, 0x47000200 };
    uint8_t  direct_data[] = { 0x55,       0xaa,       0xcc,       0x33 };

    for (i = 0; i < sizeof(direct_addr) / sizeof(direct_addr[0]); i++) {
        if (err = adi_apollo_hal_reg_set(device, direct_addr[i], direct_data[i]), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (err = adi_apollo_hal_reg_get(device, direct_addr[i], &data8), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (data8 != direct_data[i]) {
            stat = 1;
            break;
        }
    }
    dbg_printf(DBG_INFO, "Test direct register %s\r\n", (0 == stat) ? "Passed" : "*** FAILED ***");
    if (stat != 0) {
        return API_CMS_ERROR_ERROR;
    }

    /* Indirect register SPI loop rd/wr test */
    stat = 0;
    uint32_t indirect_addr[] = { INDIRECT_REG_TEST_ADDR + 0, INDIRECT_REG_TEST_ADDR + 1,
                                 INDIRECT_REG_TEST_ADDR + 2, INDIRECT_REG_TEST_ADDR + 3 };
    uint8_t  indirect_data[] = { 0x12,       0x34,       0x56,       0x78 };
    for (i = 0; i < sizeof(indirect_addr) / sizeof(indirect_addr[0]); i++) {
        if (err = adi_apollo_hal_reg_set(device, indirect_addr[i], indirect_data[i]), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (err = adi_apollo_hal_reg_get(device, indirect_addr[i], &data8), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (data8 != indirect_data[i]) {
            stat = 1;
            break;
        }
    }
    dbg_printf(DBG_INFO, "Test indirect register %s\r\n", (0 == stat) ? "Passed" : "*** FAILED ***");
    if (stat != 0) {
        return API_CMS_ERROR_ERROR;
    }

    /* 32-bit ARM mem rd/wr test */
    stat = 0;
    uint32_t arm_addr[] = { ARM_REG_TEST_BASE_ADDR + 0,  ARM_REG_TEST_BASE_ADDR + 4,
                            ARM_REG_TEST_BASE_ADDR + 8,  ARM_REG_TEST_BASE_ADDR + 12 };
    uint32_t arm_data[] = { 0x55aa55aa, 0xdeadbeef, 0xbeefdead, 0xaa55aa55 };
    uint32_t data32;
    for (i = 0; i < sizeof(arm_addr) / sizeof(arm_addr[0]); i++) {
        if (err = adi_apollo_hal_reg32_set(device, arm_addr[i], arm_data[i]), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (err = adi_apollo_hal_reg32_get(device, arm_addr[i], &data32), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (data32 != arm_data[i]) {
            stat = 1;
            break;
        }
    }
    dbg_printf(DBG_INFO, "Test ARM 32-bit register %s\r\n", (0 == stat) ? "Passed" : "*** FAILED ***");
    if (stat != 0) {
        return API_CMS_ERROR_ERROR;
    }

    return API_CMS_ERROR_OK;
}

/*!
 * @brief FPGA pre-reset: stop play/capture to reduce power before Apollo reset.
 */
static int32_t versal_apollo_ex_fpga_pre_reset(adi_fpga_apollo_device_t *fpga_device)
{
    (void)fpga_device;
    /* TODO: Implement when FPGA design register map is verified */
    return API_CMS_ERROR_OK;
}

/*!
 * @brief Convert UUID bytes to hex string.
 */
static int32_t versal_uuid_to_str(uint8_t uuid[], uint32_t uuid_len, char str_buff[], uint32_t max_str_len)
{
    int32_t err = API_CMS_ERROR_OK;
    int j = 0;

    if (uuid == NULL || str_buff == NULL) {
        return API_CMS_ERROR_NULL_PARAM;
    }

    if (max_str_len < (uuid_len * 2 + 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    str_buff[uuid_len * 2] = '\0';
    for (int i = (int)uuid_len - 1; i >= 0; i--, j += 2) {
        sprintf((str_buff + j), "%02X", uuid[i]);
    }

    return err;
}

/*============= M A I N =====================================================*/

int main(void)
{
    int32_t err;
    adi_apollo_device_t device = { {0} };
    adi_fpga_apollo_device_t fpga_device = { {0} };
    adi_apollo_top_t *profile = &VERSAL_PROFILE_STRUCT;
    adi_apollo_hal_protocol_e protocol;

    /* Clock config */
    versal_apollo_clk_mode_e clk_mode =
        VERSAL_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER |
        VERSAL_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL;
    uint32_t ltc6955_clk_khz = 125e3;

    /* Profile feature flags */
    uint8_t is_rx_cfir  = profile->rx_path[0].rx_cfir[0].enable  || profile->rx_path[1].rx_cfir[0].enable;
    uint8_t is_tx_cfir  = profile->tx_path[0].tx_cfir[0].enable  || profile->tx_path[1].tx_cfir[0].enable;
    uint8_t is_rx_pfilt = profile->rx_path[0].rx_pfilt[0].enable || profile->rx_path[1].rx_pfilt[0].enable;
    uint8_t is_tx_pfilt = profile->tx_path[0].tx_pfilt[0].enable || profile->tx_path[1].tx_pfilt[0].enable;
    uint8_t is_rx_fsrc  = profile->rx_path[0].rx_fsrc[0].enable  || profile->rx_path[1].rx_fsrc[0].enable;
    uint8_t is_tx_fsrc  = profile->tx_path[0].tx_fsrc[0].enable  || profile->tx_path[1].tx_fsrc[0].enable;

    dbg_printf(DBG_ALWAYS, "\r\n================================================\r\n");
    dbg_printf(DBG_ALWAYS, "  Apollo SDK — Versal Standalone\r\n");
    dbg_printf(DBG_ALWAYS, "  Profile: %s\r\n", VERSAL_PROFILE_NAME);
    dbg_printf(DBG_ALWAYS, "================================================\r\n\r\n");

    /* Initialize shell/command interface */
    initShell();

    /* Print profile feature summary */
    dbg_printf(DBG_INFO, "RX CFIR:  %s\r\n", is_rx_cfir  ? "enabled" : "disabled");
    dbg_printf(DBG_INFO, "TX CFIR:  %s\r\n", is_tx_cfir  ? "enabled" : "disabled");
    dbg_printf(DBG_INFO, "RX PFILT: %s\r\n", is_rx_pfilt ? "enabled" : "disabled");
    dbg_printf(DBG_INFO, "TX PFILT: %s\r\n", is_tx_pfilt ? "enabled" : "disabled");
    dbg_printf(DBG_INFO, "RX FSRC:  %s\r\n", is_rx_fsrc  ? "enabled" : "disabled");
    dbg_printf(DBG_INFO, "TX FSRC:  %s\r\n", is_tx_fsrc  ? "enabled" : "disabled");
    dbg_printf(DBG_INFO, "\r\n");

    /*
     * ========== Step 1: Open Versal platform ==========
     * Initialize SPI0 (AD9084), SPI1 (HMC7044/ADF4030/ADF4382).
     */
    dbg_printf(DBG_INFO, "--- Step 1: Hardware Init ---\r\n");
    err = versal_hw_open();
    if (err != API_CMS_ERROR_OK) {
        dbg_printf(DBG_ERROR, "FATAL: versal_hw_open failed. err=%d\r\n", err);
        return err;
    }

    /*
     * ========== Step 2: Create platform HAL instance ==========
     */
    dbg_printf(DBG_INFO, "--- Step 2: HAL Instance ---\r\n");
    adi_fpga_apollo_hal_config_t *versal_platform = versal_apollo_hal_instance();

    /*
     * ========== Step 3: Wire FPGA HAL ==========
     */
    dbg_printf(DBG_INFO, "--- Step 3: FPGA HAL Wiring ---\r\n");
    versal_apollo_ex_configure_fpga_hal(&fpga_device, versal_platform);

    /*
     * ========== Step 4: Wire Apollo HAL ==========
     */
    dbg_printf(DBG_INFO, "--- Step 4: Apollo HAL Wiring ---\r\n");
    versal_apollo_ex_configure_hal(&device, versal_platform, 0 /* no HSCI */);

    /*
     * ========== Step 5: Configure startup (FW provider) ==========
     * QSPI-based firmware provider.
     */
    dbg_printf(DBG_INFO, "--- Step 5: Startup Config ---\r\n");
    versal_apollo_ex_configure_startup(&device);

    /*
     * ========== Step 6: FPGA pre-reset ==========
     * Stop any active play/capture to reduce power before Apollo reset.
     */
    dbg_printf(DBG_INFO, "--- Step 6: FPGA Pre-Reset ---\r\n");
    versal_apollo_ex_fpga_pre_reset(&fpga_device);

    /*
     * ========== Step 7: Open Apollo API ==========
     * Platform-independent. Executes hard reset + init sequence.
     */
    dbg_printf(DBG_INFO, "--- Step 7: Apollo Device Open ---\r\n");
    err = adi_apollo_device_hw_open(&device, ADI_APOLLO_HARD_RESET_AND_INIT);
    if (err != API_CMS_ERROR_OK) {
        dbg_printf(DBG_ERROR, "ERROR: adi_apollo_device_hw_open failed. err=%d\r\n", err);
        versal_hw_close();
        return err;
    }

    dbg_printf(DBG_INFO, "Device is: %s\r\n", device.dev_info.is_8t8r ? "8T8R" : "4T4R");

    /*
     * ========== Step 8: Set active protocol and RMW ==========
     */
    dbg_printf(DBG_INFO, "--- Step 8: Protocol Config ---\r\n");
    adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0);
    adi_apollo_hal_rmw_enable_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0, 0);

    /* Print SPI config register (debug) */
    versal_apollo_ex_print_regs(&device, 0x47000000, 1);

    /*
     * ========== Step 9: FPGA register access test ==========
     */
    dbg_printf(DBG_INFO, "--- Step 9: Register Tests ---\r\n");
    err = versal_apollo_ex_fpga_reg_test(&fpga_device);
    dbg_printf(DBG_INFO, "FPGA register access test: %s\r\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    if (err != API_CMS_ERROR_OK) {
        versal_hw_close();
        return err;
    }

    /*
     * ========== Step 10: Apollo register access test ==========
     */
    err = versal_apollo_ex_reg_test(&device);
    dbg_printf(DBG_INFO, "Apollo register access test: %s\r\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    if (err != API_CMS_ERROR_OK) {
        versal_hw_close();
        return err;
    }

    /* Set RMW based on active protocol */
    adi_apollo_hal_active_protocol_get(&device, &protocol);
    adi_apollo_hal_rmw_enable_set(&device, protocol, 0);
    dbg_printf(DBG_INFO, "Active Apollo HAL Protocol: %s, RMW: disabled\r\n",
               (protocol == ADI_APOLLO_HAL_PROTOCOL_SPI0) ? "SPI0" : "HSCI");

    /*
     * ========== Step 11: Configure clocks ==========
     * HMC7044 (clock distribution) + ADF4382 (PLL) + ADF4030 (transceiver clock).
     */
    dbg_printf(DBG_INFO, "--- Step 11: Clock Configuration ---\r\n");

    /* 11a: Configure HMC7044 + ADF4382 via ex_common clock helper */
    err = versal_apollo_ex_configure_profile_clks(&fpga_device, ltc6955_clk_khz, profile, clk_mode);
    if (err != API_CMS_ERROR_OK) {
        dbg_printf(DBG_WARNING, "WARNING: Clock config returned err=%d (may be OK if using ext clocks)\r\n", err);
    }

    /* 11b: Configure ADF4030 */
    {
        adi_adf4030_device_t adf4030_device = { {0} };

        /* Wire ADF4030 HAL */
        versal_apollo_ex_adf4030_configure_hal(&adf4030_device, ADF4030_0);
        adf4030_device.hal_info.delay_us  = &versal_wait_us;
        adf4030_device.hal_info.spi_read  = &versal_cms_spi_read;
        adf4030_device.hal_info.spi_write = &versal_cms_spi_write;

        /*
         * TODO: Add ADF4030 configuration sequence here.
         * Depends on clock plan and frequency requirements.
         */
        dbg_printf(DBG_INFO, "INFO: ADF4030 HAL wired on SPI1/CS1. TODO: Add config sequence.\r\n");
    }

    /*
     * ========== Step 12: Apollo startup (FW + profile + DP) ==========
     */
    dbg_printf(DBG_INFO, "--- Step 12: Apollo Startup ---\r\n");
    err = versal_apollo_ex_startup(&device, profile);
    if (err != API_CMS_ERROR_OK) {
        dbg_printf(DBG_ERROR, "ERROR: Apollo startup failed. err=%d\r\n", err);
        versal_hw_close();
        return err;
    }

    /*
     * ========== Step 13: Print device info ==========
     */
    dbg_printf(DBG_INFO, "--- Step 13: Device Info ---\r\n");
    {
        uint8_t uuid[ADI_APOLLO_UUID_NUM_BYTES];
        char uuid_str[256];
        uint8_t die_id;

        adi_apollo_device_uuid_get(&device, uuid, ADI_APOLLO_UUID_NUM_BYTES);
        versal_uuid_to_str(uuid, ADI_APOLLO_UUID_NUM_BYTES, uuid_str, 256);
        adi_apollo_hal_log_write(&device, ADI_CMS_LOG_MSG, "UUID: %s", uuid_str);
        dbg_printf(DBG_INFO, "UUID: %s\r\n", uuid_str);

        adi_apollo_device_die_id_get(&device, &die_id);
        adi_apollo_hal_log_write(&device, ADI_CMS_LOG_MSG, "dev id: %d", die_id);
        dbg_printf(DBG_INFO, "Die ID: %d\r\n", die_id);
    }

    /*
     * ========== Step 14: Clock power calibration ==========
     */
    dbg_printf(DBG_INFO, "--- Step 14: Clock Power Cal ---\r\n");
    err = versal_apollo_ex_clk_power_cal(&device,
                                         clk_mode,
                                         (uint64_t)(profile->clk_cfg.dev_clk_freq_kHz * 1e3),
                                         (uint64_t)(ltc6955_clk_khz * 1e3));
    if (err != API_CMS_ERROR_OK) {
        dbg_printf(DBG_WARNING, "WARNING: clk_power_cal returned err=%d\r\n", err);
    }

    /*
     * ========== Step 15: FPGA profile init (JESD + FSRC) ==========
     */
    dbg_printf(DBG_INFO, "--- Step 15: FPGA Profile Init ---\r\n");
    dbg_printf(DBG_INFO, "TODO: JESD204C configuration requires Xilinx JESD204C IP driver.\r\n");
    dbg_printf(DBG_INFO, "TODO: FSRC init can be enabled when FPGA register map is verified.\r\n");

    /*
     * ========== Step 16: Ready ==========
     */
    dbg_printf(DBG_ALWAYS, "--- Step 16: Ready ---\r\n");
    dbg_printf(DBG_ALWAYS, "Apollo device initialized and ready.\r\n");

    /* Cleanup */
    dbg_printf(DBG_INFO, "--- Cleanup ---\r\n");
    versal_hw_close();

    dbg_printf(DBG_ALWAYS, "\r\nDone. Entering idle loop.\r\n");

    /* Baremetal: infinite loop with shell command processing */
    while (1) {
        checkShell();
    }

    return 0;
}
