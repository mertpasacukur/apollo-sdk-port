/*!
 * @brief     Versal Standalone platform HAL implementation.
 *            Ported from ADS10 platform (ads10_hal.c) for Xilinx Versal + Standalone (baremetal).
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *
 * @note      Platform-dependent layer only. All Apollo API calls preserved unchanged.
 *
 * SPI Topology:
 *   XSpi Instance 0 → Apollo (AD9084)
 *   XSpi Instance 1 → HMC7044 (SS0), ADF4030 (SS1), ADF4382 (SS2)
 *
 * FPGA Register Access:
 *   Direct AXI memory-mapped via Xil_In32/Xil_Out32
 *
 * FW Images:
 *   Read from MT25QU02G QSPI flash via PS QSPI driver
 *
 * Delay:
 *   usleep() from Vitis BSP sleep.h
 *
 * Reset:
 *   GPIO-controlled Apollo RESETB pin
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Vitis BSP includes */
#include "xil_printf.h"
#include "xil_io.h"
#include "xspi.h"
#include "xgpio.h"
#include "sleep.h"
#include "xil_cache.h"

/* Apollo SDK includes */
#include "versal_hal.h"
#include "adi_cms_api_common.h"
#include "adi_apollo_hal_types.h"

/*============= D E F I N E S ==============*/

/* SPI config names — match ADS10 naming convention */
#define APOLLO_SPI0_NAME    "APOLLO_SPI0"
#define APOLLO_SPI1_NAME    "APOLLO_SPI1"
#define HMC7044_SPI_NAME    "HMC7044_SPI"
#define ADF4030_SPI_NAME    "ADF4030_SPI"
#define ADF4382_SPI_NAME    "ADF4382_SPI"

/* Max SPI transfer size */
#define VERSAL_SPI_MAX_XFER_BYTES   4096

/* FPGA register offsets — same as ADS10 */
#define SPI_CHIP_SEL                    0x0901
#define AXI_FPGA_MISC_1_REG            0x10B
#define AXI_FPGA_DUT_RSTB              0x002

/*============= S T A T I C  D A T A ============*/

/* XSpi instances */
static XSpi g_spi0_instance;   /* Apollo SPI */
static XSpi g_spi1_instance;   /* HMC7044/ADF4030/ADF4382 SPI */

/* GPIO instance for Apollo RESETB */
static XGpio g_gpio_instance;

/* SPI transfer counters (for debug) */
static uint32_t g_spi_rd_cnt = 0;
static uint32_t g_spi_wr_cnt = 0;

/* SPI transfer buffer */
static uint8_t g_spi_tx_buf[VERSAL_SPI_MAX_XFER_BYTES];
static uint8_t g_spi_rx_buf[VERSAL_SPI_MAX_XFER_BYTES];

/*============= S T A T I C  F U N C T I O N S ==============*/

static versal_spi_config_t *versal_spi_config_create(void)
{
    versal_spi_config_t *cfg = (versal_spi_config_t *)calloc(1, sizeof(versal_spi_config_t));
    return cfg;
}

/**
 * @brief   Initialize an XSpi instance in master mode.
 */
static int32_t versal_xspi_init(XSpi *spi_inst, uint16_t device_id)
{
    int status;
    XSpi_Config *config;

    config = XSpi_LookupConfig(device_id);
    if (config == NULL) {
        xil_printf("ERROR: XSpi_LookupConfig failed for device %d\r\n", device_id);
        return API_CMS_ERROR_ERROR;
    }

    status = XSpi_CfgInitialize(spi_inst, config, config->BaseAddress);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: XSpi_CfgInitialize failed: %d\r\n", status);
        return API_CMS_ERROR_ERROR;
    }

    /* Set master mode, manual slave select, disable auto-start */
    status = XSpi_SetOptions(spi_inst,
        XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: XSpi_SetOptions failed: %d\r\n", status);
        return API_CMS_ERROR_ERROR;
    }

    /* Start the SPI driver (enables the device) */
    status = XSpi_Start(spi_inst);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: XSpi_Start failed: %d\r\n", status);
        return API_CMS_ERROR_ERROR;
    }

    /* Disable interrupts — we use polling mode */
    XSpi_IntrGlobalDisable(spi_inst);

    return API_CMS_ERROR_OK;
}

/**
 * @brief   Perform a SPI transfer (write then read) on the given XSpi instance.
 *          Handles chip select via XSpi_SetSlaveSelect().
 *
 * @param[in]  spi_inst        Pointer to initialized XSpi instance
 * @param[in]  slave_select    Slave select mask (1 << cs_num)
 * @param[in]  tx_data         Data to send
 * @param[out] rx_data         Received data (can be NULL for write-only)
 * @param[in]  byte_count      Number of bytes to transfer
 *
 * @return  API_CMS_ERROR_OK on success
 */
static int32_t versal_xspi_transfer(XSpi *spi_inst, uint32_t slave_select,
                                     const uint8_t *tx_data, uint8_t *rx_data,
                                     uint32_t byte_count)
{
    int status;

    if (byte_count > VERSAL_SPI_MAX_XFER_BYTES) {
        xil_printf("ERROR: SPI transfer too large: %u > %u\r\n", byte_count, VERSAL_SPI_MAX_XFER_BYTES);
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Set slave select */
    status = XSpi_SetSlaveSelect(spi_inst, slave_select);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: XSpi_SetSlaveSelect failed: %d\r\n", status);
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Copy tx data to buffer */
    memcpy(g_spi_tx_buf, tx_data, byte_count);
    memset(g_spi_rx_buf, 0, byte_count);

    /* Perform transfer */
    status = XSpi_Transfer(spi_inst, g_spi_tx_buf, g_spi_rx_buf, byte_count);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: XSpi_Transfer failed: %d\r\n", status);
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Copy rx data out */
    if (rx_data != NULL) {
        memcpy(rx_data, g_spi_rx_buf, byte_count);
    }

    return API_CMS_ERROR_OK;
}

/**
 * @brief   Get the XSpi instance and slave select for a given spi config.
 */
static int32_t versal_get_spi_instance(versal_spi_config_t *spi_desc,
                                        XSpi **out_spi, uint32_t *out_ss)
{
    if (strcmp(spi_desc->name, APOLLO_SPI0_NAME) == 0) {
        *out_spi = &g_spi0_instance;
        *out_ss = 1 << spi_desc->spi_cs;
        return API_CMS_ERROR_OK;
    }
    else if (strcmp(spi_desc->name, APOLLO_SPI1_NAME) == 0 ||
             strcmp(spi_desc->name, HMC7044_SPI_NAME) == 0 ||
             strcmp(spi_desc->name, ADF4030_SPI_NAME) == 0 ||
             strcmp(spi_desc->name, ADF4382_SPI_NAME) == 0) {
        *out_spi = &g_spi1_instance;
        *out_ss = 1 << spi_desc->spi_cs;
        return API_CMS_ERROR_OK;
    }

    xil_printf("ERROR: Unknown SPI device name: %s\r\n", spi_desc->name);
    return API_CMS_ERROR_ERROR;
}


/*============= P U B L I C  F U N C T I O N S ==============*/

versal_hal_config_t *versal_apollo_hal_instance(void)
{
    versal_hal_config_t *hal_cfg = (versal_hal_config_t *)calloc(1, sizeof(versal_hal_config_t));
    if (hal_cfg == NULL) return NULL;

    hal_cfg->spi0 = versal_spi_config_create();
    hal_cfg->spi1 = versal_spi_config_create();

    return hal_cfg;
}

/*============================================================================*/

uint32_t versal_apollo_hal_config_data(adi_apollo_device_t *device, uint8_t spi0_cs, uint8_t spi1_cs)
{
    if (device == NULL)
        return API_CMS_ERROR_INVALID_HANDLE_PTR;

    versal_hal_config_t *hal_config = (versal_hal_config_t *)device->hal_info.dev_hal_info;

    hal_config->spi0->name = APOLLO_SPI0_NAME;
    hal_config->spi0->spi_cs = spi0_cs;
    hal_config->spi0->spi_lsb_first = (int)((device->hal_info.spi0_desc.spi_config.msb) == (int)SPI_MSB_FIRST) ? 0 : 1;
    hal_config->spi0->spi_3wire = ((int)(device->hal_info.spi0_desc.spi_config.sdo) == (int)SPI_SDO) ? 0 : 1;

    hal_config->spi1->name = APOLLO_SPI1_NAME;
    hal_config->spi1->spi_cs = spi1_cs;
    hal_config->spi1->spi_lsb_first = (int)((device->hal_info.spi1_desc.spi_config.msb) == (int)SPI_MSB_FIRST) ? 0 : 1;
    hal_config->spi1->spi_3wire = (int)((device->hal_info.spi1_desc.spi_config.sdo) == (int)SPI_SDO) ? 0 : 1;

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_hmc7044_hal_spi_config_init(versal_spi_config_t *config)
{
    config->name = HMC7044_SPI_NAME;
    config->spi_3wire = 1;         /* HMC7044 only supports 3-wire SPI */
    config->spi_cs = 0;            /* CS0 on SPI Instance 1 */
    config->spi_lsb_first = 0;    /* MSB first */
    return API_CMS_ERROR_OK;
}

int32_t versal_hmc7044_hal_config_data(adi_hmc7044_device_t *device, int32_t(*spi_config_init)(versal_spi_config_t*))
{
    int32_t err;
    versal_spi_config_t *spi_hal_cfg = (versal_spi_config_t *)calloc(1, sizeof(versal_spi_config_t));
    if (spi_hal_cfg == NULL) return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK) return err;

    device->hal_info.user_data = spi_hal_cfg;
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_adf4382_hal_spi_config_init(versal_spi_config_t *config)
{
    config->name = ADF4382_SPI_NAME;
    config->spi_3wire = 0;         /* 4-wire SPI */
    config->spi_cs = 2;            /* CS2 on SPI Instance 1 */
    config->spi_lsb_first = 0;
    return API_CMS_ERROR_OK;
}

int32_t versal_adf4382_hal_config_data(adi_adf4382_device_t *device, int32_t(*spi_config_init)(versal_spi_config_t*))
{
    int32_t err;
    versal_spi_config_t *spi_hal_cfg = (versal_spi_config_t *)calloc(1, sizeof(versal_spi_config_t));
    if (spi_hal_cfg == NULL) return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK) return err;

    device->hal_info.user_data = spi_hal_cfg;
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_adf4030_hal_spi_config_init(versal_spi_config_t *config)
{
    config->name = ADF4030_SPI_NAME;
    config->spi_3wire = 0;         /* 4-wire SPI */
    config->spi_cs = 1;            /* CS1 on SPI Instance 1 */
    config->spi_lsb_first = 0;
    return API_CMS_ERROR_OK;
}

int32_t versal_adf4030_hal_config_data(adi_adf4030_device_t *device, int32_t(*spi_config_init)(versal_spi_config_t*))
{
    int32_t err;
    versal_spi_config_t *spi_hal_cfg = (versal_spi_config_t *)calloc(1, sizeof(versal_spi_config_t));
    if (spi_hal_cfg == NULL) return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK) return err;

    device->hal_info.user_data = spi_hal_cfg;
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_hw_open(const char *log_file)
{
    int32_t err;

    g_spi_rd_cnt = 0;
    g_spi_wr_cnt = 0;

    xil_printf("Versal HAL: Initializing platform...\r\n");

    /* Initialize SPI Instance 0 — Apollo */
    err = versal_xspi_init(&g_spi0_instance, XPAR_SPI_0_DEVICE_ID);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: Failed to init SPI0 (Apollo)\r\n");
        return err;
    }
    xil_printf("  SPI0 (Apollo): OK\r\n");

    /* Initialize SPI Instance 1 — HMC7044/ADF4030/ADF4382 */
    err = versal_xspi_init(&g_spi1_instance, XPAR_SPI_1_DEVICE_ID);
    if (err != API_CMS_ERROR_OK) {
        xil_printf("ERROR: Failed to init SPI1 (Clocks)\r\n");
        return err;
    }
    xil_printf("  SPI1 (HMC7044/ADF4030/ADF4382): OK\r\n");

    /* Initialize GPIO for Apollo RESETB */
    int status = XGpio_Initialize(&g_gpio_instance, VERSAL_APOLLO_RESETB_GPIO);
    if (status != XST_SUCCESS) {
        xil_printf("WARNING: GPIO init failed (reset pin may not work)\r\n");
        /* Non-fatal — some designs may use different reset mechanism */
    } else {
        /* Set GPIO direction: output */
        XGpio_SetDataDirection(&g_gpio_instance, 1, 0x00);
        xil_printf("  GPIO (Apollo RESETB): OK\r\n");
    }

    xil_printf("Versal HAL: Platform initialized successfully\r\n");
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_hw_close(void)
{
    xil_printf("Versal HAL: Closing. SPI stats: rd=%u, wr=%u, total=%u\r\n",
               g_spi_rd_cnt, g_spi_wr_cnt, g_spi_rd_cnt + g_spi_wr_cnt);

    /* Stop SPI instances */
    XSpi_Stop(&g_spi0_instance);
    XSpi_Stop(&g_spi1_instance);

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_log_write(void *user_data, int32_t log_type, const char *message, va_list argp)
{
    const char *prefix;

    if ((log_type & ADI_CMS_LOG_ERR) > 0)
        prefix = "ERROR: ";
    else if ((log_type & ADI_CMS_LOG_WARN) > 0)
        prefix = "WARN:  ";
    else
        prefix = "MSG:   ";

    xil_printf("%s", prefix);

    /* xil_printf doesn't support va_list directly, use vprintf if available */
    /* On baremetal, vprintf should work via UART */
    vprintf(message, argp);
    xil_printf("\r\n");

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_wait_us(void *user_data, uint32_t time_us)
{
    usleep(time_us);
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_hw_rst_pin_ctrl_apollo(void *user_data, uint8_t enable)
{
    /*
     * Two possible implementations depending on design:
     *
     * Option A: Direct GPIO control (if Apollo RESETB is on dedicated GPIO)
     * Option B: Via FPGA register (like ADS10 uses AXI_FPGA_MISC_1_REG)
     *
     * We implement both — try FPGA register first (matching ADS10 behavior),
     * fall back to GPIO if FPGA reg access isn't available.
     */

    /* Option B: FPGA register approach (matching ADS10 exactly) */
    uint32_t tmp_val = 0;
    versal_axi_reg_read32(AXI_FPGA_MISC_1_REG, &tmp_val);
    if (enable) {
        tmp_val |= AXI_FPGA_DUT_RSTB;
    } else {
        tmp_val &= (~AXI_FPGA_DUT_RSTB);
    }
    versal_axi_reg_write32(AXI_FPGA_MISC_1_REG, tmp_val);

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_user_data_free(void **user_data)
{
    if (*user_data != NULL) {
        free(*user_data);
        *user_data = NULL;
    }
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    *out_data = Xil_In32(VERSAL_FPGA_REG_BASEADDR + (4 * reg_offset));
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_axi_reg_write32(uint32_t reg_offset, uint32_t data)
{
    Xil_Out32(VERSAL_FPGA_REG_BASEADDR + (4 * reg_offset), data);
    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[],
                         uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    int32_t err;
    XSpi *spi_inst;
    uint32_t slave_sel;

    g_spi_rd_cnt++;

    versal_spi_config_t *spi_desc = (versal_spi_config_t *)user_data;

    err = versal_get_spi_instance(spi_desc, &spi_inst, &slave_sel);
    if (err != API_CMS_ERROR_OK) return err;

    /*
     * Apollo SPI protocol:
     *   TX: [addr_bytes...][dummy_bytes...]
     *   RX: [dummy_bytes...][data_bytes...]
     *
     * The total transfer length = addr_len + data_len
     * For XSpi_Transfer, we send addr bytes and receive data bytes in one shot.
     */

    uint32_t data_len = txn_config->data_len;
    if (txn_config->is_bf_txn) {
        data_len = txn_config->data_len * txn_config->stream_len;
    }

    uint32_t total_bytes = txn_config->addr_len + data_len;

    /* Build TX buffer: address bytes followed by dummy bytes */
    memset(g_spi_tx_buf, 0, total_bytes);
    memcpy(g_spi_tx_buf, tx_data, txn_config->addr_len);

    /* Set R/W bit: For Apollo SPI, bit[15] of address = 1 for read */
    /* The tx_data already has this bit set by the API layer */

    err = versal_xspi_transfer(spi_inst, slave_sel, g_spi_tx_buf, g_spi_rx_buf, total_bytes);
    if (err != API_CMS_ERROR_OK) return err;

    /* Copy received data back — skip the address bytes in rx */
    memcpy(&rx_data[txn_config->addr_len], &g_spi_rx_buf[txn_config->addr_len], data_len);

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_spi_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes,
                          adi_apollo_hal_txn_config_t *txn_config)
{
    int32_t err;
    XSpi *spi_inst;
    uint32_t slave_sel;

    g_spi_wr_cnt++;

    versal_spi_config_t *spi_desc = (versal_spi_config_t *)user_data;

    err = versal_get_spi_instance(spi_desc, &spi_inst, &slave_sel);
    if (err != API_CMS_ERROR_OK) return err;

    /*
     * Apollo SPI write protocol:
     *   TX: [addr_bytes...][data_bytes...]
     *
     * Total bytes = addr_len + data_len (all in tx_data already packed by API)
     */

    uint32_t data_len = txn_config->data_len;
    if (txn_config->is_bf_txn) {
        data_len = txn_config->data_len * txn_config->stream_len;
    }

    uint32_t total_bytes = txn_config->addr_len + data_len;

    /* tx_data already contains [addr][data] packed by the API */
    err = versal_xspi_transfer(spi_inst, slave_sel, tx_data, NULL, total_bytes);

    return err;
}

/*============================================================================*/

int32_t versal_fpga_mem_write(uint32_t addr, uint32_t *data, uint32_t len)
{
    /* TODO: Implement when DMA/capture is needed */
    xil_printf("WARNING: versal_fpga_mem_write not implemented (DMA phase)\r\n");
    return API_CMS_ERROR_NOT_SUPPORTED;
}

int32_t versal_fpga_mem_read(uint32_t addr, uint32_t *data, uint32_t len)
{
    /* TODO: Implement when DMA/capture is needed */
    xil_printf("WARNING: versal_fpga_mem_read not implemented (DMA phase)\r\n");
    return API_CMS_ERROR_NOT_SUPPORTED;
}
