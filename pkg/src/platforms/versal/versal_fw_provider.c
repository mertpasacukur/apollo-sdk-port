/*!
 * @brief     FW image provider for Versal — reads from QSPI flash (MT25QU02G).
 *
 *            On the original Linux platform, FW images are .bin files on a filesystem.
 *            On Versal Standalone, they are stored in QSPI flash at fixed
 *            offsets with a simple [4-byte LE size][data] format.
 *
 *            Flash layout (configurable in versal_config.h):
 *              Partition 0: VERSAL_FW_FLASH_BASE_OFFSET + 0*2MB  (Core0 Std FW)
 *              Partition 1: VERSAL_FW_FLASH_BASE_OFFSET + 1*2MB  (Core1 Std FW)
 *              Partition 2: VERSAL_FW_FLASH_BASE_OFFSET + 2*2MB  (Secure Boot Hdr)
 *              ...
 *              Partition 9: VERSAL_FW_FLASH_BASE_OFFSET + 9*2MB  (Prod TYE Oper)
 *
 *            Each partition has a 256-byte header:
 *              [0x00] uint32_t fw_id      — Firmware ID (not validated yet)
 *              [0x04] uint32_t fw_size    — Data size in bytes (excl. header)
 *              [0x08] uint32_t checksum   — CRC32 placeholder (not validated yet)
 *              [0x0C] 244 bytes reserved  — Must be 0xFF
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

/*============= I N C L U D E S ============*/
#include <stdlib.h>
#include <string.h>
#include "xil_printf.h"
#include "xqspipsu.h"

#include "adi_cms_api_common.h"
#include "adi_apollo_hal.h"
#include "versal_fw_provider.h"
#include "versal_config.h"

/*============= D E F I N E S ==============*/
#define DESC_STR            "versal-qspi"
#define QSPI_READ_CMD       0x13    /* 4-byte address Read Data command */
#define QSPI_CHUNK_SIZE     (64 * 1024)  /* Read 64KB at a time */

/*============= S T A T I C   D A T A ======================================*/

typedef struct {
    adi_apollo_device_t *device;
    uint8_t *buffer;
    XQspiPsu qspi_inst;
    int qspi_initialized;
} versal_fw_provider_obj_t;

/*
 * Flash offset + size for each FW ID.
 * Matches original Linux SDK file mapping:
 *   fw_id 0 → flash_image_0x01030000.bin
 *   fw_id 1 → flash_image_0x20000000.bin
 *   fw_id 2 → flash_image_0x02000000.bin
 *   fw_id 3 → flash_image_0x21000000.bin
 *
 * Files are written RAW to these offsets (no header).
 * Size is read from first 4 bytes of each image (LE uint32).
 */
static const struct {
    uint32_t flash_addr;
    uint32_t max_size;
} fw_flash_map[] = {
    { 0x01030000UL,   4096UL },     /* fw_id 0: flash_image_0x01030000.bin (4,096 B) */
    { 0x20000000UL, 196604UL },    /* fw_id 1: flash_image_0x20000000.bin (196,604 B) */
    { 0x02000000UL, 360444UL },    /* fw_id 2: flash_image_0x02000000.bin (360,444 B) */
    { 0x21000000UL,  23424UL },    /* fw_id 3: flash_image_0x21000000.bin (23,424 B) */
};
#define FW_FLASH_MAP_COUNT (sizeof(fw_flash_map) / sizeof(fw_flash_map[0]))

static uint32_t fw_flash_offset(adi_apollo_startup_fw_id_e fw_id)
{
    if ((uint32_t)fw_id < FW_FLASH_MAP_COUNT) {
        return fw_flash_map[(uint32_t)fw_id].flash_addr;
    }
    return 0xFFFFFFFF; /* Invalid */
}

/*============= Q S P I   H E L P E R S ====================================*/

/**
 * @brief   Initialize PS QSPI in single mode.
 */
static int32_t versal_qspi_init(XQspiPsu *qspi_inst)
{
    XQspiPsu_Config *cfg;
    int status;

    cfg = XQspiPsu_LookupConfig(VERSAL_QSPI_DEVICE_ID);
    if (cfg == NULL) {
        xil_printf("ERROR: QSPI LookupConfig failed\r\n");
        return API_CMS_ERROR_ERROR;
    }

    status = XQspiPsu_CfgInitialize(qspi_inst, cfg, cfg->BaseAddress);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: QSPI CfgInitialize failed (%d)\r\n", status);
        return API_CMS_ERROR_ERROR;
    }

    /* Set QSPI clock prescaler (div by 8 = conservative speed) */
    XQspiPsu_SetClkPrescaler(qspi_inst, XQSPIPSU_CLK_PRESCALE_8);

    xil_printf("INFO: PS QSPI initialized\r\n");
    return API_CMS_ERROR_OK;
}

/**
 * @brief   Read bytes from QSPI flash at a given 4-byte address.
 */
static int32_t versal_qspi_read(XQspiPsu *qspi_inst, uint32_t flash_addr,
                                uint8_t *buffer, uint32_t num_bytes)
{
    XQspiPsu_Msg msg[2];
    uint8_t cmd_buf[5];
    int status;
    uint32_t remaining = num_bytes;
    uint32_t offset = 0;

    while (remaining > 0) {
        uint32_t chunk = (remaining > QSPI_CHUNK_SIZE) ? QSPI_CHUNK_SIZE : remaining;
        uint32_t addr = flash_addr + offset;

        /* Command: Read 4-byte address */
        cmd_buf[0] = QSPI_READ_CMD;
        cmd_buf[1] = (addr >> 24) & 0xFF;
        cmd_buf[2] = (addr >> 16) & 0xFF;
        cmd_buf[3] = (addr >> 8) & 0xFF;
        cmd_buf[4] = addr & 0xFF;

        memset(msg, 0, sizeof(msg));

        /* Msg 0: Send command + address */
        msg[0].TxBfrPtr = cmd_buf;
        msg[0].RxBfrPtr = NULL;
        msg[0].ByteCount = 5;
        msg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
        msg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

        /* Msg 1: Receive data */
        msg[1].TxBfrPtr = NULL;
        msg[1].RxBfrPtr = buffer + offset;
        msg[1].ByteCount = chunk;
        msg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
        msg[1].Flags = XQSPIPSU_MSG_FLAG_RX;
        if (remaining - chunk == 0) {
            msg[1].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
        }

        status = XQspiPsu_PolledTransfer(qspi_inst, msg, 2);
        if (status != XST_SUCCESS) {
            xil_printf("ERROR: QSPI read failed at 0x%08lX (%d)\r\n",
                       (unsigned long)addr, status);
            return API_CMS_ERROR_ERROR;
        }

        offset += chunk;
        remaining -= chunk;
    }

    return API_CMS_ERROR_OK;
}

/*============= P U B L I C   A P I =========================================*/

adi_apollo_fw_provider_t *versal_fw_provider_create(adi_apollo_device_t *device, char *fw_image_dir)
{
    versal_fw_provider_obj_t *obj_data;
    adi_apollo_fw_provider_t *obj;
    int32_t err;

    (void)fw_image_dir; /* Not used on Versal — FW comes from QSPI */

    if (device == NULL) return NULL;

    obj = (adi_apollo_fw_provider_t *)malloc(sizeof(adi_apollo_fw_provider_t));
    if (obj == NULL) {
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "FW provider: malloc failed");
        return NULL;
    }

    obj->tag = malloc(sizeof(versal_fw_provider_obj_t));
    if (obj->tag == NULL) {
        free(obj);
        adi_apollo_hal_log_write(device, ADI_CMS_LOG_ERR, "FW provider: malloc tag failed");
        return NULL;
    }

    obj_data = (versal_fw_provider_obj_t *)obj->tag;
    obj->desc = DESC_STR;
    obj_data->device = device;
    obj_data->buffer = NULL;
    obj_data->qspi_initialized = 0;

    /* Initialize QSPI */
    err = versal_qspi_init(&obj_data->qspi_inst);
    if (err == API_CMS_ERROR_OK) {
        obj_data->qspi_initialized = 1;
    } else {
        xil_printf("WARNING: QSPI init failed — FW loading will not work\r\n");
    }

    return obj;
}

int32_t versal_fw_provider_open(adi_apollo_fw_provider_t *obj, adi_apollo_startup_fw_id_e fw_id)
{
    versal_fw_provider_obj_t *data;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(obj->tag);

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) return API_CMS_ERROR_INVALID_PARAM;

    data = (versal_fw_provider_obj_t *)obj->tag;
    adi_apollo_hal_log_write(data->device, ADI_CMS_LOG_MSG,
                             "FW provider open: id=%d, flash_offset=0x%08X",
                             (int)fw_id, fw_flash_offset(fw_id));

    return API_CMS_ERROR_OK;
}

int32_t versal_fw_provider_close(adi_apollo_fw_provider_t *obj, adi_apollo_startup_fw_id_e fw_id)
{
    versal_fw_provider_obj_t *data;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(obj->tag);

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) return API_CMS_ERROR_INVALID_PARAM;

    data = (versal_fw_provider_obj_t *)obj->tag;

    if (data->buffer != NULL) {
        free(data->buffer);
        data->buffer = NULL;
    }

    adi_apollo_hal_log_write(data->device, ADI_CMS_LOG_MSG,
                             "FW provider close: id=%d", (int)fw_id);

    return API_CMS_ERROR_OK;
}

int32_t versal_fw_provider_get(adi_apollo_fw_provider_t *obj,
                               adi_apollo_startup_fw_id_e fw_id,
                               uint8_t **byte_arr, uint32_t *bytes_read)
{
    int32_t err;
    uint32_t flash_addr;
    uint32_t fw_size;
    versal_fw_provider_obj_t *data;

    ADI_CMS_NULL_PTR_CHECK(obj);
    ADI_CMS_NULL_PTR_CHECK(byte_arr);
    ADI_CMS_NULL_PTR_CHECK(bytes_read);
    ADI_CMS_NULL_PTR_CHECK(obj->tag);

    if (fw_id >= ADI_APOLLO_FW_ID_MAX) return API_CMS_ERROR_INVALID_PARAM;

    data = (versal_fw_provider_obj_t *)obj->tag;

    if (!data->qspi_initialized) {
        xil_printf("ERROR: QSPI not initialized — cannot load FW\r\n");
        return API_CMS_ERROR_ERROR;
    }

    if (data->buffer != NULL) {
        xil_printf("ERROR: FW buffer not freed from previous transaction\r\n");
        return API_CMS_ERROR_ERROR;
    }

    flash_addr = fw_flash_offset(fw_id);
    if (flash_addr == 0xFFFFFFFF) {
        xil_printf("ERROR: Invalid FW ID %d\r\n", (int)fw_id);
        return API_CMS_ERROR_INVALID_PARAM;
    }

    /*
     * RAW flash read — no header.
     * Files are written directly to flash at their mapped offsets.
     * We read the actual file size from the .bin content.
     *
     * Strategy: read first 4 bytes to get the embedded size,
     * or use the known max_size from the map and read entire partition.
     *
     * For Apollo FW images, the binary is self-contained.
     * We read the full file (known size from flash map).
     */
    uint32_t max_fw_size = (uint32_t)fw_id < FW_FLASH_MAP_COUNT ?
                           fw_flash_map[(uint32_t)fw_id].max_size : 0;

    if (max_fw_size == 0) {
        xil_printf("ERROR: FW ID %d has no size mapping\r\n", (int)fw_id);
        return API_CMS_ERROR_ERROR;
    }

    /* Read first 4 bytes to check if flash is populated (not 0xFFFFFFFF) */
    uint8_t probe[4];
    err = versal_qspi_read(&data->qspi_inst, flash_addr, probe, 4);
    if (err != API_CMS_ERROR_OK) return err;

    uint32_t first_word = (uint32_t)probe[0] |
                          ((uint32_t)probe[1] << 8) |
                          ((uint32_t)probe[2] << 16) |
                          ((uint32_t)probe[3] << 24);

    if (first_word == 0xFFFFFFFF) {
        xil_printf("ERROR: Flash empty at 0x%08lX (fw_id=%d) — FW not programmed\r\n",
                   (unsigned long)flash_addr, (int)fw_id);
        return API_CMS_ERROR_ERROR;
    }

    /* Allocate buffer and read full FW image */
    fw_size = max_fw_size;
    data->buffer = (uint8_t *)malloc(fw_size);
    if (data->buffer == NULL) {
        xil_printf("ERROR: FW buffer malloc failed (size=%lu)\r\n", (unsigned long)fw_size);
        return API_CMS_ERROR_MEM_ALLOC;
    }

    err = versal_qspi_read(&data->qspi_inst, flash_addr, data->buffer, fw_size);
    if (err != API_CMS_ERROR_OK) {
        free(data->buffer);
        data->buffer = NULL;
        return err;
    }

    *byte_arr = data->buffer;
    *bytes_read = fw_size;

    adi_apollo_hal_log_write(data->device, ADI_CMS_LOG_MSG,
                             "FW provider get: id=%d, size=%lu bytes",
                             (int)fw_id, (unsigned long)fw_size);

    return API_CMS_ERROR_OK;
}
