/*!
 * @brief     FW image provider for Versal — reads from MT25QU02G QSPI flash.
 *            Ported from ads10_fw_provider.c.
 *
 * @note      Uses Xilinx PS QSPI driver (XQspiPsu) from Vitis 2023.2 BSP.
 *            FW images stored at predefined flash offsets.
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xil_printf.h"
#include "xqspipsu.h"

#include "adi_cms_api_common.h"
#include "adi_apollo_hal.h"
#include "versal_fw_provider.h"

/*============= D E F I N E S ==============*/
#define DESC_STR    "versal-qspi"

/* QSPI commands for MT25QU02G */
#define QSPI_READ_CMD           0x03    /* Normal Read */
#define QSPI_FAST_READ_CMD      0x0B    /* Fast Read */
#define QSPI_READ_STATUS_CMD    0x05    /* Read Status Register */

/*============= T Y P E S ==============*/
typedef struct {
    adi_apollo_device_t *device;
    uint8_t *buffer;                    /* Allocated buffer for FW image */
    uint32_t buffer_size;
} versal_fw_provider_obj_t;

/*============= S T A T I C  D A T A ==============*/
static XQspiPsu g_qspi_instance;
static int g_qspi_initialized = 0;

/*
 * Flash offset lookup table — maps FW ID to flash address.
 * Order must match adi_apollo_startup_fw_id_e enum.
 */
static const uint32_t fw_flash_offsets[ADI_APOLLO_FW_ID_MAX] = {
    VERSAL_FW_FLASH_OFFSET_CPU0_B,          /* ADI_APOLLO_FW_ID_CPU0_B */
    VERSAL_FW_FLASH_OFFSET_CPU1_B,          /* ADI_APOLLO_FW_ID_CPU1_B */
    VERSAL_FW_FLASH_OFFSET_SE_01030000,     /* ADI_APOLLO_FW_ID_SE_01030000 */
    VERSAL_FW_FLASH_OFFSET_SE_20000000,     /* ADI_APOLLO_FW_ID_SE_20000000 */
    VERSAL_FW_FLASH_OFFSET_SE_02000000,     /* ADI_APOLLO_FW_ID_SE_02000000 */
    VERSAL_FW_FLASH_OFFSET_SE_21000000,     /* ADI_APOLLO_FW_ID_SE_21000000 */
    VERSAL_FW_FLASH_OFFSET_SEP_01030000,    /* ADI_APOLLO_FW_ID_SEP_01030000 */
    VERSAL_FW_FLASH_OFFSET_SEP_20000000,    /* ADI_APOLLO_FW_ID_SEP_20000000 */
    VERSAL_FW_FLASH_OFFSET_SEP_02000000,    /* ADI_APOLLO_FW_ID_SEP_02000000 */
    VERSAL_FW_FLASH_OFFSET_SEP_21000000,    /* ADI_APOLLO_FW_ID_SEP_21000000 */
};

/*
 * FW image sizes — must be set based on actual image sizes.
 * TODO: Read actual sizes from flash header or use known fixed sizes.
 * For now, use a placeholder approach: read up to max and detect end.
 */

/*============= S T A T I C  F U N C T I O N S ==============*/

static int32_t versal_qspi_init(void)
{
    XQspiPsu_Config *config;
    int status;

    if (g_qspi_initialized) return API_CMS_ERROR_OK;

    config = XQspiPsu_LookupConfig(VERSAL_QSPI_DEVICE_ID);
    if (config == NULL) {
        xil_printf("ERROR: QSPI LookupConfig failed\r\n");
        return API_CMS_ERROR_ERROR;
    }

    status = XQspiPsu_CfgInitialize(&g_qspi_instance, config, config->BaseAddress);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: QSPI CfgInitialize failed: %d\r\n", status);
        return API_CMS_ERROR_ERROR;
    }

    /* Set QSPI clock prescaler */
    XQspiPsu_SetClkPrescaler(&g_qspi_instance, XQSPIPSU_CLK_PRESCALE_8);

    /* Select flash (CS0) */
    XQspiPsu_SelectFlash(&g_qspi_instance, XQSPIPSU_SELECT_FLASH_CS_LOWER,
                          XQSPIPSU_SELECT_FLASH_BUS_LOWER);

    g_qspi_initialized = 1;
    xil_printf("QSPI initialized (MT25QU02G)\r\n");

    return API_CMS_ERROR_OK;
}

/**
 * @brief   Read data from QSPI flash at given offset.
 *
 * @param[in]  flash_addr   Flash byte address
 * @param[out] buffer       Buffer to store read data
 * @param[in]  byte_count   Number of bytes to read
 *
 * @return  API_CMS_ERROR_OK on success
 */
static int32_t versal_qspi_read(uint32_t flash_addr, uint8_t *buffer, uint32_t byte_count)
{
    XQspiPsu_Msg msg[2];
    int status;
    uint8_t cmd_buf[5];

    /* Build read command: 0x03 + 3-byte or 4-byte address */
    /* MT25QU02G supports 4-byte addressing for >16MB */
    cmd_buf[0] = QSPI_READ_CMD;
    cmd_buf[1] = (flash_addr >> 16) & 0xFF;
    cmd_buf[2] = (flash_addr >> 8) & 0xFF;
    cmd_buf[3] = flash_addr & 0xFF;

    /* Message 0: Command + Address (TX only) */
    memset(msg, 0, sizeof(msg));
    msg[0].TxBfrPtr = cmd_buf;
    msg[0].RxBfrPtr = NULL;
    msg[0].ByteCount = 4;  /* cmd + 3 addr bytes */
    msg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
    msg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

    /* Message 1: Data (RX only) */
    msg[1].TxBfrPtr = NULL;
    msg[1].RxBfrPtr = buffer;
    msg[1].ByteCount = byte_count;
    msg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
    msg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

    status = XQspiPsu_PolledTransfer(&g_qspi_instance, msg, 2);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: QSPI read failed at 0x%08X: %d\r\n", flash_addr, status);
        return API_CMS_ERROR_ERROR;
    }

    return API_CMS_ERROR_OK;
}


/*============= P U B L I C  F U N C T I O N S ==============*/

adi_apollo_fw_provider_t *versal_fw_provider_create(adi_apollo_device_t *device)
{
    if (device == NULL) return NULL;

    adi_apollo_fw_provider_t *obj = (adi_apollo_fw_provider_t *)malloc(sizeof(adi_apollo_fw_provider_t));
    if (obj == NULL) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "FW provider malloc failed");
        return NULL;
    }

    versal_fw_provider_obj_t *ctx = (versal_fw_provider_obj_t *)malloc(sizeof(versal_fw_provider_obj_t));
    if (ctx == NULL) {
        free(obj);
        return NULL;
    }

    obj->desc = DESC_STR;
    obj->tag = ctx;
    ctx->device = device;
    ctx->buffer = NULL;
    ctx->buffer_size = 0;

    return obj;
}

/*============================================================================*/

int32_t versal_fw_provider_open(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id)
{
    int32_t err;
    versal_fw_provider_obj_t *ctx;

    if (fw_provider == NULL || fw_provider->tag == NULL)
        return API_CMS_ERROR_INVALID_HANDLE_PTR;

    ctx = (versal_fw_provider_obj_t *)fw_provider->tag;

    /* Initialize QSPI if not already done */
    err = versal_qspi_init();
    if (err != API_CMS_ERROR_OK) return err;

    /* Allocate buffer for FW image */
    if (ctx->buffer == NULL) {
        ctx->buffer = (uint8_t *)malloc(VERSAL_FW_MAX_IMAGE_SIZE);
        if (ctx->buffer == NULL) {
            xil_printf("ERROR: Failed to allocate FW buffer (%u bytes)\r\n", VERSAL_FW_MAX_IMAGE_SIZE);
            return API_CMS_ERROR_ERROR;
        }
        ctx->buffer_size = VERSAL_FW_MAX_IMAGE_SIZE;
    }

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_fw_provider_close(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id)
{
    versal_fw_provider_obj_t *ctx;

    if (fw_provider == NULL || fw_provider->tag == NULL)
        return API_CMS_ERROR_INVALID_HANDLE_PTR;

    ctx = (versal_fw_provider_obj_t *)fw_provider->tag;

    /* Free buffer after use to save memory */
    if (ctx->buffer != NULL) {
        free(ctx->buffer);
        ctx->buffer = NULL;
        ctx->buffer_size = 0;
    }

    return API_CMS_ERROR_OK;
}

/*============================================================================*/

int32_t versal_fw_provider_get(adi_apollo_fw_provider_t *fw_provider, adi_apollo_startup_fw_id_e fw_id,
                                uint8_t **byte_arr, uint32_t *bytes_read)
{
    int32_t err;
    versal_fw_provider_obj_t *ctx;
    uint32_t flash_offset;
    uint32_t image_size;

    if (fw_provider == NULL || fw_provider->tag == NULL)
        return API_CMS_ERROR_INVALID_HANDLE_PTR;

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) {
        xil_printf("ERROR: Invalid FW ID: %d\r\n", fw_id);
        return API_CMS_ERROR_INVALID_PARAM;
    }

    ctx = (versal_fw_provider_obj_t *)fw_provider->tag;

    if (ctx->buffer == NULL) {
        xil_printf("ERROR: FW buffer not allocated (call open first)\r\n");
        return API_CMS_ERROR_ERROR;
    }

    flash_offset = fw_flash_offsets[fw_id];

    /*
     * TODO: Determine actual image size. Options:
     *   1. Store a size header at each flash offset
     *   2. Use known fixed sizes per FW ID
     *   3. Read max and let the API handle truncation
     *
     * For now, we read a fixed size. The Apollo startup API
     * knows the expected sizes internally and will only use
     * what it needs.
     */
    image_size = VERSAL_FW_MAX_IMAGE_SIZE;

    xil_printf("FW provider: Reading FW ID %d from flash offset 0x%08X (%u bytes)\r\n",
               fw_id, flash_offset, image_size);

    err = versal_qspi_read(flash_offset, ctx->buffer, image_size);
    if (err != API_CMS_ERROR_OK) return err;

    *byte_arr = ctx->buffer;
    *bytes_read = image_size;

    return API_CMS_ERROR_OK;
}
