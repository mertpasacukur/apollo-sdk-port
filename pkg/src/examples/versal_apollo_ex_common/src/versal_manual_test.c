/*!
 * @brief     Versal Manual Register Test Implementation
 *
 *            Register read/write for APOLLO, HMC7044, ADF4382, ADF4030.
 *            Call versal_manual_test_init() once after devices are initialized,
 *            then use read/write/dump from your serial shell.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#include <string.h>

#include "versal_manual_test.h"
#include "versal_debug.h"
#include "adi_apollo_hal.h"
#include "adi_hmc7044_core.h"
#include "adi_adf4382_core.h"
#include "adi_adf4030_core.h"
#include "adi_cms_api_common.h"

/*============= S T A T I C   D A T A ======================================*/

static adi_apollo_device_t   *g_apollo  = NULL;
static adi_hmc7044_device_t  *g_hmc7044 = NULL;
static adi_adf4382_device_t  *g_adf4382 = NULL;
static adi_adf4030_device_t  *g_adf4030 = NULL;

/*============= D E V I C E   I D   E N U M ================================*/

typedef enum {
    DEV_APOLLO = 0,
    DEV_HMC7044,
    DEV_ADF4382,
    DEV_ADF4030,
    DEV_UNKNOWN
} manual_test_device_e;

/*!
 * @brief Resolve device name string to enum.
 */
static manual_test_device_e resolve_device(const char *device_name)
{
    if (device_name == NULL) {
        return DEV_UNKNOWN;
    }

    if (strcmp(device_name, "APOLLO") == 0) {
        return DEV_APOLLO;
    } else if (strcmp(device_name, "HMC7044") == 0) {
        return DEV_HMC7044;
    } else if (strcmp(device_name, "ADF4382") == 0) {
        return DEV_ADF4382;
    } else if (strcmp(device_name, "ADF4030") == 0) {
        return DEV_ADF4030;
    }

    return DEV_UNKNOWN;
}

/*============= P U B L I C   A P I ========================================*/

void versal_manual_test_init(adi_apollo_device_t   *apollo,
                             adi_hmc7044_device_t  *hmc7044,
                             adi_adf4382_device_t  *adf4382,
                             adi_adf4030_device_t  *adf4030)
{
    g_apollo  = apollo;
    g_hmc7044 = hmc7044;
    g_adf4382 = adf4382;
    g_adf4030 = adf4030;

    dbg_printf(DBG_INFO, "[MANUAL_TEST] Initialized. Devices:\r\n");
    dbg_printf(DBG_INFO, "  APOLLO:  %s\r\n", g_apollo  ? "OK" : "NULL");
    dbg_printf(DBG_INFO, "  HMC7044: %s\r\n", g_hmc7044 ? "OK" : "NULL");
    dbg_printf(DBG_INFO, "  ADF4382: %s\r\n", g_adf4382 ? "OK" : "NULL");
    dbg_printf(DBG_INFO, "  ADF4030: %s\r\n", g_adf4030 ? "OK" : "NULL");
}

int32_t versal_manual_test_read(const char *device_name, uint32_t address, uint8_t *data)
{
    int32_t err;
    manual_test_device_e dev;

    if (data == NULL) {
        dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: data pointer is NULL\r\n");
        return API_CMS_ERROR_NULL_PARAM;
    }

    dev = resolve_device(device_name);

    switch (dev) {
    case DEV_APOLLO:
        if (g_apollo == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: APOLLO not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_apollo_hal_reg_get(g_apollo, address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[APOLLO] READ  0x%08X = 0x%02X\r\n", address, *data);
        } else {
            dbg_printf(DBG_DEBUG, "[APOLLO] READ  0x%08X FAILED (err=%d)\r\n", address, err);
        }
        break;

    case DEV_HMC7044:
        if (g_hmc7044 == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: HMC7044 not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_hmc7044_core_spi_reg_get(g_hmc7044, (uint16_t)address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[HMC7044] READ  0x%04X = 0x%02X\r\n", (uint16_t)address, *data);
        } else {
            dbg_printf(DBG_DEBUG, "[HMC7044] READ  0x%04X FAILED (err=%d)\r\n", (uint16_t)address, err);
        }
        break;

    case DEV_ADF4382:
        if (g_adf4382 == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: ADF4382 not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_adf4382_core_spi_reg_get(g_adf4382, (uint16_t)address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[ADF4382] READ  0x%04X = 0x%02X\r\n", (uint16_t)address, *data);
        } else {
            dbg_printf(DBG_DEBUG, "[ADF4382] READ  0x%04X FAILED (err=%d)\r\n", (uint16_t)address, err);
        }
        break;

    case DEV_ADF4030:
        if (g_adf4030 == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: ADF4030 not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_adf4030_core_spi_reg_get(g_adf4030, (uint16_t)address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[ADF4030] READ  0x%04X = 0x%02X\r\n", (uint16_t)address, *data);
        } else {
            dbg_printf(DBG_DEBUG, "[ADF4030] READ  0x%04X FAILED (err=%d)\r\n", (uint16_t)address, err);
        }
        break;

    default:
        dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: Unknown device \"%s\"\r\n", device_name);
        dbg_printf(DBG_ERROR, "  Valid: APOLLO, HMC7044, ADF4382, ADF4030\r\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    return err;
}

int32_t versal_manual_test_write(const char *device_name, uint32_t address, uint8_t data)
{
    int32_t err;
    manual_test_device_e dev;

    dev = resolve_device(device_name);

    switch (dev) {
    case DEV_APOLLO:
        if (g_apollo == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: APOLLO not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_apollo_hal_reg_set(g_apollo, address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[APOLLO] WRITE 0x%08X = 0x%02X OK\r\n", address, data);
        } else {
            dbg_printf(DBG_DEBUG, "[APOLLO] WRITE 0x%08X = 0x%02X FAILED (err=%d)\r\n", address, data, err);
        }
        break;

    case DEV_HMC7044:
        if (g_hmc7044 == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: HMC7044 not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_hmc7044_core_spi_reg_set(g_hmc7044, (uint16_t)address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[HMC7044] WRITE 0x%04X = 0x%02X OK\r\n", (uint16_t)address, data);
        } else {
            dbg_printf(DBG_DEBUG, "[HMC7044] WRITE 0x%04X = 0x%02X FAILED (err=%d)\r\n", (uint16_t)address, data, err);
        }
        break;

    case DEV_ADF4382:
        if (g_adf4382 == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: ADF4382 not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_adf4382_core_spi_reg_set(g_adf4382, (uint16_t)address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[ADF4382] WRITE 0x%04X = 0x%02X OK\r\n", (uint16_t)address, data);
        } else {
            dbg_printf(DBG_DEBUG, "[ADF4382] WRITE 0x%04X = 0x%02X FAILED (err=%d)\r\n", (uint16_t)address, data, err);
        }
        break;

    case DEV_ADF4030:
        if (g_adf4030 == NULL) {
            dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: ADF4030 not initialized\r\n");
            return API_CMS_ERROR_NULL_PARAM;
        }
        err = adi_adf4030_core_spi_reg_set(g_adf4030, (uint16_t)address, data);
        if (err == API_CMS_ERROR_OK) {
            dbg_printf(DBG_DEBUG, "[ADF4030] WRITE 0x%04X = 0x%02X OK\r\n", (uint16_t)address, data);
        } else {
            dbg_printf(DBG_DEBUG, "[ADF4030] WRITE 0x%04X = 0x%02X FAILED (err=%d)\r\n", (uint16_t)address, data, err);
        }
        break;

    default:
        dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: Unknown device \"%s\"\r\n", device_name);
        dbg_printf(DBG_ERROR, "  Valid: APOLLO, HMC7044, ADF4382, ADF4030\r\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    return err;
}

int32_t versal_manual_test_dump(const char *device_name, uint32_t start_addr, uint32_t count)
{
    int32_t err;
    uint8_t data;
    uint32_t i;
    manual_test_device_e dev;

    dev = resolve_device(device_name);
    if (dev == DEV_UNKNOWN) {
        dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: Unknown device \"%s\"\r\n", device_name);
        return API_CMS_ERROR_INVALID_PARAM;
    }

    if (count == 0U || count > 256U) {
        dbg_printf(DBG_ERROR, "[MANUAL_TEST] ERROR: count must be 1-256\r\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    dbg_printf(DBG_DEBUG, "[%s] DUMP 0x%04X .. 0x%04X (%u regs)\r\n",
               device_name, start_addr, start_addr + count - 1U, count);

    for (i = 0; i < count; i++) {
        err = versal_manual_test_read(device_name, start_addr + i, &data);
        if (err != API_CMS_ERROR_OK) {
            dbg_printf(DBG_ERROR, "[%s] DUMP aborted at offset %u (err=%d)\r\n", device_name, i, err);
            return err;
        }
    }

    return API_CMS_ERROR_OK;
}
