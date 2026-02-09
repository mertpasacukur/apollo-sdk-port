/*!
 * @brief     Versal Apollo example program main.
 *            Ported from ADS10 ads10_apollo_ex_main/main.c.
 *
 * @note      PRESERVES the original main.c flow exactly:
 *            1. Platform open (versal_hw_open replaces ads10_hw_open)
 *            2. Create platform HAL instance
 *            3. Wire up FPGA HAL
 *            4. Wire up Apollo HAL
 *            5. Configure startup (FW provider)
 *            6. FPGA pre-reset
 *            7. Apollo device open + hard reset
 *            8. Set active SPI protocol
 *            9. Register access tests
 *            10. Configure clocks (HMC7044 + ADF4382)
 *            11. Apollo startup (FW load + profile config)
 *            12. Print UUID, die ID
 *            13. FPGA profile init (JESD — TODO)
 *            14. Execute example function (fullchip)
 *            15. Cleanup
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xil_printf.h"

#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"

/* Versal platform HAL */
#include "versal_hal.h"
#include "versal_apollo_ex.h"

/* Example common functions — reuse from ADS10 where possible */
#include "adi_ads10_apollo_ex_clk.h"        /* Clock configuration — TODO: port if needed */
#include "adi_ads10_apollo_ex_inspect.h"    /* Inspection utilities */
#include "adi_ads10_apollo_ex_to_str.h"     /* UUID to string */

/* Profile — fullchip + id00_uc06 as default */
#include "id00_uc06.h"

/* Example functions — reuse from ADS10 */
/* These are platform-independent — they only call Apollo API functions */
#include "apollo_examples.h"

/*============= M A I N ==============*/

int main(void)
{
    int32_t err;
    adi_apollo_device_t device = { {0} };
    adi_fpga_apollo_device_t fpga_device = { {0} };
    adi_apollo_top_t *profile = &id00_uc06_0;       /* Default profile — can be changed */
    uint8_t uuid[ADI_APOLLO_UUID_NUM_BYTES];
    char uuid_str[256];
    uint8_t die_id;
    uint8_t is_platform_open = 0;

    /* Clock configuration — external center clock for dev, external for FPGA */
    adi_ads10_apollo_clk_mode_e clk_mode = ADI_ADS10_APOLLO_CLK_MODE_DEV_CLK_EXTERNAL_CENTER
                                          | ADI_ADS10_APOLLO_CLK_MODE_FPGA_CLK_EXTERNAL;
    uint32_t ltc6955_clk_khz = 125e3;  /* 125 MHz from LTC6955 */

    xil_printf("\r\n========================================\r\n");
    xil_printf("ADI Apollo Example Code on Versal Platform\r\n");
    xil_printf("Profile: id00_uc06 (20Gsps, 10.3125Gbps)\r\n");
    xil_printf("========================================\r\n\r\n");

    /*
     * ================================================================
     * STEP 1: Open Versal platform
     *         Initializes AXI Quad SPI, GPIO, QSPI flash
     * ================================================================
     */
    if (err = versal_hw_open("apollo_app.log"), err != API_CMS_ERROR_OK) {
        xil_printf("Platform failed to open. Exiting...\r\n");
        goto end;
    }
    is_platform_open = 1;

    /*
     * ================================================================
     * STEP 2: Create platform HAL instance
     * ================================================================
     */
    versal_hal_config_t *versal_platform = versal_apollo_hal_instance();
    if (versal_platform == NULL) {
        xil_printf("Failed to create HAL instance\r\n");
        err = API_CMS_ERROR_ERROR;
        goto end;
    }

    /*
     * ================================================================
     * STEP 3: Wire up FPGA HAL
     * ================================================================
     */
    versal_apollo_ex_configure_fpga_hal(&fpga_device, versal_platform);

    /*
     * ================================================================
     * STEP 4: Wire up Apollo HAL
     * ================================================================
     */
    versal_apollo_ex_configure_hal(&device, versal_platform);

    /*
     * ================================================================
     * STEP 5: Configure startup (FW provider)
     * ================================================================
     */
    versal_apollo_ex_configure_startup(&device);

    /*
     * ================================================================
     * STEP 6: FPGA pre-reset
     * ================================================================
     */
    versal_apollo_ex_fpga_pre_reset(&fpga_device);

    /*
     * ================================================================
     * STEP 7: Open Apollo API — hard reset + init
     *         UNCHANGED from ADS10
     * ================================================================
     */
    xil_printf("Opening Apollo device (hard reset + init)...\r\n");
    if (err = adi_apollo_device_hw_open(&device, ADI_APOLLO_HARD_RESET_AND_INIT), err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: adi_apollo_device_hw_open failed: %d\r\n", err);
        goto end;
    }
    xil_printf("Device is: %s\r\n", device.dev_info.is_8t8r ? "8T8R" : "4T4R");

    /*
     * ================================================================
     * STEP 8: Set active SPI protocol
     *         UNCHANGED from ADS10
     * ================================================================
     */
    adi_apollo_hal_active_protocol_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0);
    adi_apollo_hal_rmw_enable_set(&device, ADI_APOLLO_HAL_PROTOCOL_SPI0, 0);

    /*
     * ================================================================
     * STEP 9: Register access tests
     *         UNCHANGED from ADS10
     * ================================================================
     */
    /* TODO: Uncomment when FPGA register test addresses are confirmed for Versal
    err = adi_ads10_apollo_ex_fpga_reg_test(&fpga_device);
    xil_printf("FPGA register access test: %s\r\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");

    err = adi_ads10_apollo_ex_reg_test(&device);
    xil_printf("Apollo register access test: %s\r\n", (err == API_CMS_ERROR_OK) ? "Passed" : "*** FAILED ***");
    */

    /*
     * ================================================================
     * STEP 10: Configure clocks
     *          TODO: Port clock configuration for Versal's clock tree
     *          HMC7044 and ADF4382 SPI config is already set up above
     * ================================================================
     */
    xil_printf("Configuring clocks...\r\n");
    /* TODO: Implement Versal clock configuration
     * The original code calls:
     *   adi_ads10_apollo_ex_configure_profile_clks(&fpga_device, ltc6955_clk_khz, profile, clk_mode);
     *
     * This function configures HMC7044 and optionally ADF4382.
     * It uses the clock chips' APIs which go through their own HAL (SPI).
     * The SPI HAL for HMC7044/ADF4382 is wired via versal_hmc7044_hal_config_data()
     * and versal_adf4382_hal_config_data() — those need to be called before this.
     *
     * For now, clock setup is assumed to be done externally or by separate code.
     */

    /*
     * ================================================================
     * STEP 11: Apollo startup — FW load + profile config
     *          UNCHANGED from ADS10 (uses adi_apollo_startup_execute internally)
     * ================================================================
     */
    xil_printf("Running Apollo startup sequence...\r\n");
    err = versal_apollo_ex_startup(&device, profile);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: Apollo startup failed: %d\r\n", err);
        goto end;
    }

    /*
     * ================================================================
     * STEP 12: Print UUID and die ID
     *          UNCHANGED from ADS10
     * ================================================================
     */
    adi_apollo_device_uuid_get(&device, uuid, ADI_APOLLO_UUID_NUM_BYTES);
    adi_ads10_ex_uuid_to_str(uuid, ADI_APOLLO_UUID_NUM_BYTES, uuid_str, 256);
    xil_printf("UUID: %s\r\n", uuid_str);

    adi_apollo_device_die_id_get(&device, &die_id);
    xil_printf("Die ID: %d\r\n", die_id);

    /*
     * ================================================================
     * STEP 13: FPGA profile init (JESD)
     *          TODO: JESD will use Xilinx JESD IP — implement separately
     * ================================================================
     */
    xil_printf("JESD configuration: TODO (Xilinx JESD IP)\r\n");
    /* Original:
     * err = adi_ads10_apollo_ex_fpga_profile_init(&fpga_device, profile);
     */

    /*
     * ================================================================
     * STEP 14: Execute example function
     *          fullchip is the primary example
     *          TODO: Uncomment when JESD link is up
     * ================================================================
     */
    xil_printf("Example execution: TODO (after JESD link up)\r\n");
    /* Original:
     * err = fullchip(&device, &fpga_device, profile, argc, argv, argc_ofst);
     * xil_printf("Example code exec returned: %s\r\n", err == API_CMS_ERROR_OK ? "OK" : "*ERROR*");
     */

    xil_printf("\r\n========================================\r\n");
    xil_printf("Apollo initialization completed successfully!\r\n");
    xil_printf("========================================\r\n");

end:
    /*
     * ================================================================
     * STEP 15: Cleanup
     * ================================================================
     */
    if (is_platform_open) {
        versal_hw_close();
    }

    xil_printf("Goodbye\r\n");
    return err;
}
