/*!
 * @brief     Versal Apollo example program main.
 *
 *            Port of ads10_apollo_ex_main/main.c for Xilinx Versal Standalone.
 *            No CLI — profile and example are selected at compile time.
 *
 *            Default configuration:
 *              Example:  fullchip
 *              Profile:  id00_uc06
 *
 *            To change, define before including this file:
 *              #define VERSAL_PROFILE  id00_uc08_f
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>

#include "xil_printf.h"
#include "sleep.h"

/* Apollo SDK — platform-independent */
#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"

/* Versal platform HAL */
#include "versal_hal.h"
#include "versal_fpga.h"

/* Example common — HAL wiring */
#include "versal_apollo_ex.h"

/* Clock device APIs */
#include "adi_ads10_apollo_ex_clk.h"
#include "adi_ads10_apollo_ex_hmc7044.h"
#include "adi_ads10_apollo_ex_adf4382.h"
#include "adi_ads10_apollo_ex_adf4030.h"

/* Example functions */
#include "apollo_examples.h"

/* Device profile — compile-time selection */
#include "id00_uc06.h"

/*============= C O M P I L E - T I M E   C O N F I G ======================*/

/*
 * Change these defines to select a different profile or example.
 * E.g.:
 *   #define VERSAL_PROFILE          id00_uc08_f
 *   #define VERSAL_PROFILE_STRUCT   id00_uc08_f_0
 *   #include "id00_uc08_f.h"
 */
#ifndef VERSAL_PROFILE_STRUCT
#define VERSAL_PROFILE_STRUCT   id00_uc06_0
#endif

#ifndef VERSAL_PROFILE_NAME
#define VERSAL_PROFILE_NAME     "id00_uc06"
#endif

#ifndef VERSAL_EXAMPLE_NAME
#define VERSAL_EXAMPLE_NAME     "fullchip"
#endif

/*============= M A I N =====================================================*/

int main(void)
{
    int32_t err;
    adi_apollo_device_t device = { {0} };
    adi_fpga_apollo_device_t fpga_device = { {0} };
    adi_apollo_top_t *profile = &VERSAL_PROFILE_STRUCT;
    uint8_t is_hw_open = 0;

    /* Clock config */
    adi_ads10_apollo_clk_mode_e clk_mode =
        ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER |
        ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL;
    uint32_t ltc6955_clk_khz = 125e3;

    xil_printf("\r\n================================================\r\n");
    xil_printf("  Apollo SDK — Versal Standalone\r\n");
    xil_printf("  Example: %s\r\n", VERSAL_EXAMPLE_NAME);
    xil_printf("  Profile: %s\r\n", VERSAL_PROFILE_NAME);
    xil_printf("================================================\r\n\r\n");

    /*
     * ========== Step 1: Open Versal platform ==========
     * Initialize SPI0 (AD9084), SPI1 (HMC7044/ADF4030/ADF4382).
     */
    xil_printf("--- Step 1: Hardware Init ---\r\n");
    if (err = versal_hw_open(), err != API_CMS_ERROR_OK) {
        xil_printf("FATAL: versal_hw_open failed. err=%d\r\n", err);
        goto end;
    }
    is_hw_open = 1;

    /*
     * ========== Step 2: Create platform HAL instance ==========
     * Allocates spi0, spi1, hsci config structs.
     */
    xil_printf("--- Step 2: HAL Instance ---\r\n");
    adi_fpga_apollo_hal_config_t *versal_platform = versal_apollo_hal_instance();

    /*
     * ========== Step 3: Wire FPGA HAL ==========
     * Connects reg_read/write, delay, mem_read/write to Versal implementations.
     */
    xil_printf("--- Step 3: FPGA HAL Wiring ---\r\n");
    versal_apollo_ex_configure_fpga_hal(&fpga_device, versal_platform);

    /*
     * ========== Step 4: Wire Apollo HAL ==========
     * Connects SPI read/write, delay, reset, log to Versal implementations.
     */
    xil_printf("--- Step 4: Apollo HAL Wiring ---\r\n");
    versal_apollo_ex_configure_hal(&device, versal_platform, 0 /* no HSCI */);

    /*
     * ========== Step 5: Configure startup (FW provider) ==========
     * QSPI-based firmware provider replaces ADS10 filesystem provider.
     */
    xil_printf("--- Step 5: Startup Config ---\r\n");
    versal_apollo_ex_configure_startup(&device);

    /*
     * ========== Step 6: FPGA pre-reset ==========
     * Stop any active play/capture to reduce power before Apollo reset.
     *
     * TODO: This uses FPGA-register-relative offsets that may differ
     * from ADS10. Verify FPGA register map matches your Vivado design.
     */
    xil_printf("--- Step 6: FPGA Pre-Reset ---\r\n");
    /* adi_ads10_apollo_ex_fpga_pre_reset(&fpga_device); */
    /* TODO: Implement versal-specific pre-reset if FPGA design differs */

    /*
     * ========== Step 7: Open Apollo API ==========
     * Platform-independent. Executes hard reset + init sequence.
     * This calls our wired HAL functions (SPI, delay, reset).
     */
    xil_printf("--- Step 7: Apollo Device Open ---\r\n");
    if (err = adi_apollo_device_hw_open(&device, ADI_APOLLO_HARD_RESET_AND_INIT), err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: adi_apollo_device_hw_open failed. err=%d\r\n", err);
        goto end;
    }

    xil_printf("Device is: %s\r\n", device.dev_info.is_8t8r ? "8T8R" : "4T4R");

    /* Set active protocol to SPI0 */
    adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0);
    adi_apollo_hal_rmw_enable_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0, 0);

    /*
     * ========== Step 8: Configure clocks ==========
     * HMC7044 (clock distribution) + ADF4382 (PLL) + ADF4030 (transceiver clock).
     *
     * The clock config functions from ex_common use HAL function pointers
     * (SPI read/write, delay) that we've already wired to versal_ functions.
     * So they work as-is — the platform abstraction is working!
     */
    xil_printf("--- Step 8: Clock Configuration ---\r\n");

    /* 8a: Configure HMC7044 + ADF4382 via ex_common clock helper */
    err = adi_ads10_apollo_ex_configure_profile_clks(&fpga_device, ltc6955_clk_khz, profile, clk_mode);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("WARNING: Clock config returned err=%d (may be OK if using ext clocks)\r\n", err);
        /* Don't bail — external clock mode may not need HMC7044/ADF4382 */
    }

    /* 8b: Configure ADF4030 (not in original ADS10 main.c, but needed for our chain) */
    {
        adi_adf4030_device_t adf4030_device = { {0} };

        /* Wire ADF4030 HAL */
        versal_adf4030_hal_config_data(&adf4030_device, &versal_adf4030_hal_spi_config_init);
        adf4030_device.hal_info.delay_us  = &versal_wait_us;
        adf4030_device.hal_info.spi_read  = &versal_cms_spi_read;
        adf4030_device.hal_info.spi_write = &versal_cms_spi_write;

        /*
         * TODO: Add ADF4030 configuration sequence here.
         * This depends on the clock plan and frequency requirements.
         * Example:
         *   adi_ads10_apollo_ex_adf4030_configure_hal(&adf4030_device, ADF4030_0);
         *   adi_adf4030_device_init(&adf4030_device);
         *   adi_adf4030_vco_config(&adf4030_device, ...);
         */
        xil_printf("INFO: ADF4030 HAL wired on SPI1/CS1. TODO: Add config sequence.\r\n");
    }

    /*
     * ========== Step 9: Apollo startup (FW + profile + DP) ==========
     * Loads firmware from QSPI flash, programs device profile,
     * configures digital datapath, enables JESD links.
     */
    xil_printf("--- Step 9: Apollo Startup ---\r\n");
    err = versal_apollo_ex_startup(&device, profile);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: Apollo startup failed. err=%d\r\n", err);
        goto end;
    }

    /*
     * ========== Step 10: FPGA profile init (JESD) ==========
     * On ADS10, this configures FPGA JESD Rx/Tx links from the profile.
     * On Versal, the Xilinx JESD204 IP handles this differently.
     *
     * TODO: Integrate with Xilinx JESD204C IP driver.
     */
    xil_printf("--- Step 10: FPGA JESD Init ---\r\n");
    xil_printf("TODO: JESD204 configuration — uses Xilinx JESD204 IP.\r\n");
    xil_printf("      ADS10's adi_ads10_apollo_ex_fpga_jesd_configure() is not portable.\r\n");
    xil_printf("      Versal equivalent requires Xilinx JESD204C IP driver integration.\r\n");
    /* err = adi_ads10_apollo_ex_fpga_jesd_configure(&fpga_device, ...); */

    /*
     * ========== Step 11: Run example ==========
     * On ADS10, this calls ex_func (fullchip, tx_nco, etc.) parsed from CLI.
     * On Versal, we call the compile-time selected example directly.
     */
    xil_printf("--- Step 11: Run Example (%s) ---\r\n", VERSAL_EXAMPLE_NAME);

    /* Default: fullchip example */
    err = fullchip(&device, &fpga_device, profile, 0, NULL, 0);
    xil_printf("Example returned: %s (err=%d)\r\n",
               err == API_CMS_ERROR_OK ? "OK" : "*ERROR*", err);

end:
    /*
     * ========== Step 12: Cleanup ==========
     */
    xil_printf("--- Step 12: Cleanup ---\r\n");
    if (is_hw_open) {
        versal_hw_close();
    }

    xil_printf("\r\nDone. Entering idle loop.\r\n");

    /* Baremetal: infinite loop (no OS to return to) */
    while (1) {
        sleep(1);
    }

    return err;
}
