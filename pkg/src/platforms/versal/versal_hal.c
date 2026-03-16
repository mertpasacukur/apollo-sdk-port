/*!
 * @brief     Versal platform HAL implementation for Apollo SDK port.
 *
 *            Replaces all original Linux platform-dependent code with Xilinx
 *            Versal Standalone BSP. Uses AXI Quad SPI (XSpi) for device
 *            communication.
 *
 *            SPI Topology:
 *              SPI0 (Instance 0): AD9084 only
 *              SPI1 (Instance 1): HMC7044 (CS0), ADF4030 (CS1), ADF4382 (CS2)
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "xspi.h"
/* #include "xil_io.h" */  /* TODO: Uncomment when FPGA register access is implemented */
#include "xil_printf.h"
#include "sleep.h"

#include "versal_hal.h"
#include "versal_config.h"
#include "versal_debug.h"

/*============= D E F I N E S ==============*/
#define SPI_BUF_SIZE    256

/*============= S T A T I C   D A T A ======================================*/

/* XSpi instances */
static XSpi     g_spi0;        /* SPI0: AD9084 */
static XSpi     g_spi1;        /* SPI1: HMC7044, ADF4030, ADF4382 */

/* SPI tx/rx buffers */
static uint8_t  g_spi_tx_buf[SPI_BUF_SIZE];
static uint8_t  g_spi_rx_buf[SPI_BUF_SIZE];

/* SPI counters */
static uint32_t g_spi_rd_cnt = 0;
static uint32_t g_spi_wr_cnt = 0;

/*============= S T A T I C   H E L P E R S ================================*/

/**
 * @brief   Initialize a single AXI Quad SPI instance in master/polled mode.
 */
static int32_t versal_spi_instance_init(XSpi *inst, uint16_t device_id, const char *label)
{
    int status;

    status = XSpi_Initialize(inst, device_id);
    if (status != XST_SUCCESS) {
        dbg_printf(DBG_ERROR, "ERROR: %s XSpi_Initialize failed (%d)\r\n", label, status);
        return API_CMS_ERROR_HW_OPEN;
    }

    /* Master mode, manual slave select, SPI Mode 3 (CPOL=1, CPHA=1) for AD9084 */
    status = XSpi_SetOptions(inst,
                              XSP_MASTER_OPTION |
                              XSP_MANUAL_SSELECT_OPTION |
                              XSP_CLK_ACTIVE_LOW_OPTION |
                              XSP_CLK_PHASE_1_OPTION);
    if (status != XST_SUCCESS) {
        dbg_printf(DBG_ERROR, "ERROR: %s XSpi_SetOptions failed (%d)\r\n", label, status);
        return API_CMS_ERROR_HW_OPEN;
    }

    status = XSpi_Start(inst);
    if (status != XST_SUCCESS) {
        dbg_printf(DBG_ERROR, "ERROR: %s XSpi_Start failed (%d)\r\n", label, status);
        return API_CMS_ERROR_HW_OPEN;
    }

    /* Polled mode — disable interrupts */
    XSpi_IntrGlobalDisable(inst);

    dbg_printf(DBG_INFO, "INFO: %s initialized\r\n", label);
    return API_CMS_ERROR_OK;
}

/**
 * @brief   Get XSpi instance for a device based on its name.
 *
 *          APOLLO_SPI0 → g_spi0
 *          Everything else (HMC7044, ADF4382, ADF4030) → g_spi1
 */
static XSpi *versal_get_spi_instance(hal_spi_config_t *spi_desc)
{
    if (strcmp(spi_desc->name, APOLLO_SPI0_NAME) == 0 ||
        strcmp(spi_desc->name, APOLLO_SPI1_NAME) == 0) {
        return &g_spi0;
    }
    /* HMC7044, ADF4382, ADF4030 all go through SPI1 */
    return &g_spi1;
}

/**
 * @brief   Map from hal_spi_config_t to CMS txn_config.
 */
static adi_apollo_hal_txn_config_t cms_hal_to_apollo_hal_map(adi_cms_hal_txn_config_t *txn_config)
{
    adi_apollo_hal_txn_config_t mapped = {
        .addr_len = txn_config->addr_len,
        .data_len = txn_config->data_len,
        .stream_len = txn_config->stream_len,
        .mask = txn_config->mask
    };
    return mapped;
}

/*============= H A R D W A R E   O P E N / C L O S E ======================*/

int32_t versal_hw_open(void)
{
    int32_t err;

    g_spi_rd_cnt = 0;
    g_spi_wr_cnt = 0;

    dbg_printf(DBG_ALWAYS, "\r\n========================================\r\n");
    dbg_printf(DBG_ALWAYS, " Apollo SDK — Versal Standalone HAL\r\n");
    dbg_printf(DBG_ALWAYS, "========================================\r\n");

    /* Initialize SPI0 (AD9084) */
    err = versal_spi_instance_init(&g_spi0, VERSAL_SPI0_DEVICE_ID, "SPI0 (AD9084)");
    if (err != API_CMS_ERROR_OK) return err;

    /* Initialize SPI1 (HMC7044 / ADF4030 / ADF4382) */
    err = versal_spi_instance_init(&g_spi1, VERSAL_SPI1_DEVICE_ID, "SPI1 (Clocks)");
    if (err != API_CMS_ERROR_OK) return err;

    dbg_printf(DBG_INFO, "INFO: versal_hw_open() complete\r\n");
    return API_CMS_ERROR_OK;
}

int32_t versal_hw_close(void)
{
    dbg_printf(DBG_INFO, "INFO: versal_hw_close() — SPI rd=%lu, wr=%lu, total=%lu\r\n",
               (unsigned long)g_spi_rd_cnt, (unsigned long)g_spi_wr_cnt,
               (unsigned long)(g_spi_rd_cnt + g_spi_wr_cnt));

    XSpi_Stop(&g_spi0);
    XSpi_Stop(&g_spi1);

    return API_CMS_ERROR_OK;
}

/*============= S P I   R E A D =============================================*/

int32_t versal_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[],
                        uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    hal_spi_config_t *spi_desc = (hal_spi_config_t *)user_data;
    XSpi *spi_inst;
    int status;

    g_spi_rd_cnt++;

    spi_inst = versal_get_spi_instance(spi_desc);

    /*
     * Total transfer size = addr_len + data_len * stream_count
     * The tx_data from the SDK already has addr bytes packed.
     * For read, we send addr and then dummy bytes; MISO returns data.
     */
    uint32_t data_len = txn_config->data_len;
    if (txn_config->is_bf_txn) {
        data_len = txn_config->data_len * txn_config->stream_len;
    }
    uint32_t total = txn_config->addr_len + data_len;

    if (total > SPI_BUF_SIZE) {
        dbg_printf(DBG_ERROR, "ERROR: SPI read size %lu exceeds buffer\r\n", (unsigned long)total);
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Set chip select: XSpi uses bitmask where bit N = slave N */
    status = XSpi_SetSlaveSelect(spi_inst, (1u << spi_desc->spi_cs));
    if (status != XST_SUCCESS) {
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Build TX: copy addr bytes from tx_data, pad rest with 0 for read */
    memset(g_spi_tx_buf, 0x00, total);
    memcpy(g_spi_tx_buf, tx_data, txn_config->addr_len);
    memset(g_spi_rx_buf, 0x00, total);

    /* Full-duplex SPI transfer */
    status = XSpi_Transfer(spi_inst, g_spi_tx_buf, g_spi_rx_buf, total);
    if (status != XST_SUCCESS) {
        dbg_printf(DBG_ERROR, "ERROR: SPI read transfer failed (%d) dev=%s\r\n", status, spi_desc->name);
        return API_CMS_ERROR_SPI_XFER;
    }

    /*
     * Copy received data back.
     * The SDK layer expects rx_data to mirror the full transfer:
     *   rx_data[0..addr_len-1] = addr echo (don't care)
     *   rx_data[addr_len..] = actual data
     */
    memcpy(rx_data, g_spi_rx_buf, total);

    /* Debug: print TX/RX bytes */
    dbg_printf(DBG_DEBUG, "[SPI-RD] dev=%s cs=%u len=%lu TX:", spi_desc->name, spi_desc->spi_cs, (unsigned long)total);
    for (uint32_t i = 0; i < total && i < 8U; i++) {
        dbg_printf(DBG_DEBUG, " %02X", g_spi_tx_buf[i]);
    }
    dbg_printf(DBG_DEBUG, " RX:");
    for (uint32_t i = 0; i < total && i < 8U; i++) {
        dbg_printf(DBG_DEBUG, " %02X", g_spi_rx_buf[i]);
    }
    dbg_printf(DBG_DEBUG, "\r\n");

    return API_CMS_ERROR_OK;
}

/*============= S P I   W R I T E ===========================================*/

int32_t versal_spi_write(void *user_data, const uint8_t tx_data[],
                         uint32_t num_tx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    hal_spi_config_t *spi_desc = (hal_spi_config_t *)user_data;
    XSpi *spi_inst;
    int status;

    g_spi_wr_cnt++;

    spi_inst = versal_get_spi_instance(spi_desc);

    if (num_tx_bytes > SPI_BUF_SIZE) {
        dbg_printf(DBG_ERROR, "ERROR: SPI write size %lu exceeds buffer\r\n", (unsigned long)num_tx_bytes);
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Set chip select */
    status = XSpi_SetSlaveSelect(spi_inst, (1u << spi_desc->spi_cs));
    if (status != XST_SUCCESS) {
        return API_CMS_ERROR_SPI_XFER;
    }

    /* TX buffer already contains [addr | data] from SDK */
    memcpy(g_spi_tx_buf, tx_data, num_tx_bytes);
    memset(g_spi_rx_buf, 0x00, num_tx_bytes);

    /* Full-duplex transfer (rx discarded for writes) */
    status = XSpi_Transfer(spi_inst, g_spi_tx_buf, g_spi_rx_buf, num_tx_bytes);
    if (status != XST_SUCCESS) {
        dbg_printf(DBG_ERROR, "ERROR: SPI write transfer failed (%d) dev=%s\r\n", status, spi_desc->name);
        return API_CMS_ERROR_SPI_XFER;
    }

    /* Debug: print TX bytes */
    dbg_printf(DBG_DEBUG, "[SPI-WR] dev=%s cs=%u len=%lu TX:", spi_desc->name, spi_desc->spi_cs, (unsigned long)num_tx_bytes);
    for (uint32_t i = 0; i < num_tx_bytes && i < 8U; i++) {
        dbg_printf(DBG_DEBUG, " %02X", g_spi_tx_buf[i]);
    }
    dbg_printf(DBG_DEBUG, "\r\n");

    return API_CMS_ERROR_OK;
}

/*============= C M S   S P I   W R A P P E R S ============================*/
/*
 * HMC7044, ADL6331/6332, and other CMS devices use adi_cms_hal_txn_config_t.
 * These wrappers convert to adi_apollo_hal_txn_config_t and call the core SPI.
 */

int32_t versal_cms_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[],
                            uint32_t num_tx_rx_bytes, adi_cms_hal_txn_config_t *txn_config)
{
    adi_apollo_hal_txn_config_t mapped = cms_hal_to_apollo_hal_map(txn_config);
    return versal_spi_read(user_data, tx_data, rx_data, num_tx_rx_bytes, &mapped);
}

int32_t versal_cms_spi_write(void *user_data, const uint8_t tx_data[],
                             uint32_t num_tx_bytes, adi_cms_hal_txn_config_t *txn_config)
{
    adi_apollo_hal_txn_config_t mapped = cms_hal_to_apollo_hal_map(txn_config);
    return versal_spi_write(user_data, tx_data, num_tx_bytes, &mapped);
}

/*============= D E L A Y ===================================================*/

int32_t versal_wait_us(void *user_data, uint32_t time_us)
{
    (void)user_data;
    usleep(time_us);
    return API_CMS_ERROR_OK;
}

/*============= R E S E T   P I N ===========================================*/

int32_t versal_hw_rst_pin_ctrl_apollo(void *user_data, uint8_t enable)
{
    (void)user_data;

    /*
     * Apollo reset control via FPGA register.
     * Register map (from Apollo_3U1502_regMap):
     *   Offset 0: version_read_ctrl
     *   Offset 1: reset_reg — bit[0] = Apollo_resetb (write 1 to release reset)
     *   Offset 2: JESD_reset_reg
     *
     * Base address comes from xparameters.h:
     *   TODO PASA: Set VERSAL_FPGA_REG_BASE_ADDR in versal_config.h
     *              from xparameters.h (XPAR_AXI_..._BASEADDR)
     */
    #ifndef VERSAL_FPGA_REG_BASE_ADDR
    #error "VERSAL_FPGA_REG_BASE_ADDR not defined — set it in versal_config.h from xparameters.h"
    #endif

    #define APOLLO_RESET_REG_OFFSET  1   /* Register 1 = reset_reg */
    #define APOLLO_RESETB_BIT        0   /* Bit 0 = Apollo_resetb */

    uint32_t addr = VERSAL_FPGA_REG_BASE_ADDR + (APOLLO_RESET_REG_OFFSET * 4);

    if (enable) {
        /* Release reset: write 1 to Apollo_resetb bit */
        Xil_Out32(addr, Xil_In32(addr) | (1U << APOLLO_RESETB_BIT));
    } else {
        /* Assert reset: write 0 to Apollo_resetb bit */
        Xil_Out32(addr, Xil_In32(addr) & ~(1U << APOLLO_RESETB_BIT));
    }

    dbg_printf(DBG_DEBUG, "rst_pin_ctrl_apollo: enable=%u, addr=0x%08lX, val=0x%08lX\r\n",
               enable, (unsigned long)addr, (unsigned long)Xil_In32(addr));

    return API_CMS_ERROR_OK;
}

/*============= L O G G I N G ===============================================*/

int32_t versal_log_write(void *user_data, int32_t log_type, const char *message, va_list argp)
{
    char log_msg[512];
    const char *type_str;

    (void)user_data;

    type_str = "MSG";
    if ((log_type & ADI_CMS_LOG_WARN) > 0)
        type_str = "WRN";
    if ((log_type & ADI_CMS_LOG_ERR) > 0)
        type_str = "ERR";

    vsnprintf(log_msg, sizeof(log_msg), message, argp);
    xil_printf("[%s] %s\r\n", type_str, log_msg);

    return API_CMS_ERROR_OK;
}

/*============= U S E R   D A T A   F R E E ================================*/

int32_t versal_user_data_free(void **user_data)
{
    if (*user_data != NULL) {
        free(*user_data);
        *user_data = NULL;
    }
    return API_CMS_ERROR_OK;
}

/*============= F P G A   R E G I S T E R   A C C E S S ====================*/

int32_t versal_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    /*
     * TODO: Implement FPGA register read.
     *
     * Read a 32-bit value from the Apollo FPGA register block.
     * Address = VERSAL_FPGA_REG_BASE_ADDR + (4 * reg_offset)
     *
     * Implementation (once FPGA base address is known):
     *   *out_data = Xil_In32(VERSAL_FPGA_REG_BASE_ADDR + 4 * reg_offset);
     *
     * Requires: VERSAL_FPGA_REG_BASE_ADDR defined in versal_config.h.
     */
    (void)reg_offset;
    *out_data = 0;
    dbg_printf(DBG_WARNING, "TODO: versal_axi_reg_read32(offset=0x%03lX) — "
               "FPGA base address not yet defined\r\n", (unsigned long)reg_offset);
    return API_CMS_ERROR_OK;
}

int32_t versal_axi_reg_write32(uint32_t reg_offset, uint32_t data)
{
    /*
     * TODO: Implement FPGA register write.
     *
     * Write a 32-bit value to the Apollo FPGA register block.
     * Address = VERSAL_FPGA_REG_BASE_ADDR + (4 * reg_offset)
     *
     * Implementation (once FPGA base address is known):
     *   Xil_Out32(VERSAL_FPGA_REG_BASE_ADDR + 4 * reg_offset, data);
     *
     * Requires: VERSAL_FPGA_REG_BASE_ADDR defined in versal_config.h.
     */
    (void)reg_offset;
    (void)data;
    dbg_printf(DBG_WARNING, "TODO: versal_axi_reg_write32(offset=0x%03lX, data=0x%08lX) — "
               "FPGA base address not yet defined\r\n",
               (unsigned long)reg_offset, (unsigned long)data);
    return API_CMS_ERROR_OK;
}

/*============= F P G A   M E M O R Y   A C C E S S   ( S T U B S ) ========*/

int32_t versal_fpga_mem_read(uint32_t mem_addr, uint32_t num_bytes, uint8_t *buffer)
{
    (void)mem_addr;
    (void)num_bytes;
    (void)buffer;
    dbg_printf(DBG_WARNING, "TODO: versal_fpga_mem_read — DMA not implemented\r\n");
    return API_CMS_ERROR_ERROR;
}

int32_t versal_fpga_mem_write(uint32_t mem_addr, uint32_t num_bytes, uint8_t *buffer)
{
    (void)mem_addr;
    (void)num_bytes;
    (void)buffer;
    dbg_printf(DBG_WARNING, "TODO: versal_fpga_mem_write — DMA not implemented\r\n");
    return API_CMS_ERROR_ERROR;
}

/*============= H A L   I N S T A N C E =====================================*/

static hal_spi_config_t *hal_spi_config_create(void)
{
    hal_spi_config_t *cfg = (hal_spi_config_t *)calloc(1, sizeof(hal_spi_config_t));
    return cfg;
}

static hal_hsci_config_t *hal_hsci_config_create(void)
{
    hal_hsci_config_t *cfg = (hal_hsci_config_t *)calloc(1, sizeof(hal_hsci_config_t));
    return cfg;
}

adi_fpga_apollo_hal_config_t *versal_apollo_hal_instance(void)
{
    adi_fpga_apollo_hal_config_t *hal_cfg =
        (adi_fpga_apollo_hal_config_t *)calloc(1, sizeof(adi_fpga_apollo_hal_config_t));

    hal_cfg->spi0 = hal_spi_config_create();
    hal_cfg->spi1 = hal_spi_config_create();
    hal_cfg->hsci = hal_hsci_config_create();

    return hal_cfg;
}

/*============= A P O L L O   H A L   C O N F I G   D A T A ================*/

uint32_t versal_apollo_hal_config_data(adi_apollo_device_t *device, uint8_t spi0_cs, uint8_t spi1_cs)
{
    if (device == NULL)
        return API_CMS_ERROR_INVALID_HANDLE_PTR;

    adi_fpga_apollo_hal_config_t *hal_config =
        (adi_fpga_apollo_hal_config_t *)device->hal_info.dev_hal_info;

    hal_config->spi0->name = APOLLO_SPI0_NAME;
    hal_config->spi0->spi_cs = spi0_cs;
    hal_config->spi0->spi_lsb_first =
        (int)((device->hal_info.spi0_desc.spi_config.msb) == (int)SPI_MSB_FIRST) ? 0 : 1;
    hal_config->spi0->spi_3wire =
        ((int)(device->hal_info.spi0_desc.spi_config.sdo) == (int)SPI_SDO) ? 0 : 1;

    hal_config->spi1->name = APOLLO_SPI1_NAME;
    hal_config->spi1->spi_cs = spi1_cs;
    hal_config->spi1->spi_lsb_first =
        (int)((device->hal_info.spi1_desc.spi_config.msb) == (int)SPI_MSB_FIRST) ? 0 : 1;
    hal_config->spi1->spi_3wire =
        (int)((device->hal_info.spi1_desc.spi_config.sdo) == (int)SPI_SDO) ? 0 : 1;

    return API_CMS_ERROR_OK;
}

/*============= D E V I C E   H A L   C O N F I G S ========================*/

/* ---- HMC7044 ---- */
int32_t versal_hmc7044_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = HMC7044_SPI_NAME;
    config->spi_3wire = 1;         /* HMC7044 = 3-wire SPI only */
    config->spi_cs = 0;            /* SPI1/CS0 */
    config->spi_lsb_first = 0;    /* MSB first */
    return API_CMS_ERROR_OK;
}

int32_t versal_hmc7044_hal_config_data(adi_hmc7044_device_t *device,
                                       int32_t(*spi_config_init)(hal_spi_config_t*))
{
    int32_t err;
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *)calloc(1, sizeof(hal_spi_config_t));
    if (spi_hal_cfg == NULL) return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK)
        return err;

    device->hal_info.user_data = spi_hal_cfg;
    return API_CMS_ERROR_OK;
}

/* ---- ADF4382 ---- */
int32_t versal_adf4382_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = ADF4382_SPI_NAME;
    config->spi_3wire = 0;         /* 4-wire SPI */
    config->spi_cs = 2;            /* SPI1/CS2 */
    config->spi_lsb_first = 0;    /* MSB first */
    return API_CMS_ERROR_OK;
}

int32_t versal_adf4382_hal_config_data(adi_adf4382_device_t *device,
                                       int32_t(*spi_config_init)(hal_spi_config_t*))
{
    int32_t err;
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *)calloc(1, sizeof(hal_spi_config_t));
    if (spi_hal_cfg == NULL) return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK)
        return err;

    device->hal_info.user_data = spi_hal_cfg;
    return API_CMS_ERROR_OK;
}

/* ---- ADF4030 ---- */
int32_t versal_adf4030_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = ADF4030_SPI_NAME;
    config->spi_3wire = 0;         /* 4-wire SPI */
    config->spi_cs = 1;            /* SPI1/CS1 */
    config->spi_lsb_first = 0;    /* MSB first */
    return API_CMS_ERROR_OK;
}

int32_t versal_adf4030_hal_config_data(adi_adf4030_device_t *device,
                                       int32_t(*spi_config_init)(hal_spi_config_t*))
{
    int32_t err;
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *)calloc(1, sizeof(hal_spi_config_t));
    if (spi_hal_cfg == NULL) return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK)
        return err;

    device->hal_info.user_data = spi_hal_cfg;
    return API_CMS_ERROR_OK;
}
