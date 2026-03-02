/*!
 * @brief     Versal Raw Register I/O — API-Free Direct SPI Access
 *
 *            Bypasses the Apollo/HMC/ADF API device structs entirely.
 *            Talks directly to XSpi instances initialized by versal_hw_open().
 *
 *            SPI Protocol:
 *              Apollo (AD9084):  3-byte addr (paged SPI), 1-byte data
 *                Write: [addr_hi, addr_lo, data]
 *                Read:  [addr_hi | 0x80, addr_lo, 0x00] → rx[2] = data
 *
 *              HMC7044:  2-byte addr, 1-byte data (3-wire SPI, R/W in addr[15])
 *                Write: [0x00 | (addr>>8 & 0x1F), addr & 0xFF, data]
 *                Read:  [0x80 | (addr>>8 & 0x1F), addr & 0xFF, 0x00]
 *
 *              ADF4382:  2-byte addr, 1-byte data
 *                Write: [addr_hi & 0x7F, addr_lo, data]
 *                Read:  [addr_hi | 0x80, addr_lo, 0x00]
 *
 *              ADF4030:  2-byte addr, 1-byte data
 *                Write: [addr_hi & 0x7F, addr_lo, data]
 *                Read:  [addr_hi | 0x80, addr_lo, 0x00]
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#include <string.h>
#include <stdlib.h>

#include "xspi.h"
#include "xil_printf.h"

#include "versal_raw_regio.h"
#include "versal_config.h"

/*============= E X T E R N   S P I   I N S T A N C E S ====================*/
/*
 * These are defined in versal_hal.c. We reference them directly to avoid
 * going through the API device struct layer.
 */
extern XSpi g_spi0;   /* SPI0: Apollo */
extern XSpi g_spi1;   /* SPI1: HMC7044 (CS0), ADF4030 (CS1), ADF4382 (CS2) */

/*============= D E V I C E   T A B L E ====================================*/

static const raw_device_info_t g_dev_table[RAW_DEV_COUNT] = {
    /* APOLLO:  SPI0, CS0, 2-byte addr (hi|lo), 4-wire, read_bit=0x80 on addr_hi */
    { "APOLLO",  0, 0, 2, 0, 0x80 },
    /* HMC7044: SPI1, CS0, 2-byte addr, 3-wire, read_bit=0x80 on first byte */
    { "HMC7044", 1, 0, 2, 1, 0x80 },
    /* ADF4382: SPI1, CS2, 2-byte addr, 4-wire, read_bit=0x80 */
    { "ADF4382", 1, 2, 2, 0, 0x80 },
    /* ADF4030: SPI1, CS1, 2-byte addr, 4-wire, read_bit=0x80 */
    { "ADF4030", 1, 1, 2, 0, 0x80 },
};

/*============= S T A T I C   B U F F E R S ================================*/

static uint8_t g_tx[8];
static uint8_t g_rx[8];

/*============= H E L P E R S ==============================================*/

static XSpi *get_spi_instance(uint8_t spi_idx)
{
    return (spi_idx == 0U) ? &g_spi0 : &g_spi1;
}

static int32_t spi_select_cs(XSpi *inst, uint8_t cs)
{
    /* XSpi slave select is a bitmask: bit N = CS N active low → set all high except target */
    uint32_t ss_mask = ~(1U << cs) & 0xFFU;
    int status = XSpi_SetSlaveSelect(inst, ss_mask);
    if (status != XST_SUCCESS) {
        xil_printf("[RAW] ERROR: CS select failed (cs=%u, status=%d)\r\n", cs, status);
        return -1;
    }
    return 0;
}

static int32_t spi_xfer(XSpi *inst, uint8_t *tx, uint8_t *rx, uint32_t len)
{
    int status = XSpi_Transfer(inst, tx, rx, len);
    if (status != XST_SUCCESS) {
        xil_printf("[RAW] ERROR: SPI transfer failed (status=%d)\r\n", status);
        return -1;
    }
    return 0;
}

/*============= P U B L I C   A P I ========================================*/

void raw_regio_init(void)
{
    xil_printf("\r\n");
    xil_printf("============================================\r\n");
    xil_printf("  RAW REGISTER I/O — Direct SPI Access\r\n");
    xil_printf("============================================\r\n");
    xil_printf("  APOLLO  : SPI0, CS0 (AD9084)\r\n");
    xil_printf("  HMC7044 : SPI1, CS0\r\n");
    xil_printf("  ADF4030 : SPI1, CS1\r\n");
    xil_printf("  ADF4382 : SPI1, CS2\r\n");
    xil_printf("============================================\r\n");
    xil_printf("  Type 'help' for commands.\r\n\r\n");
}

raw_device_e raw_regio_resolve_name(const char *name)
{
    if (name == NULL) return RAW_DEV_UNKNOWN;

    if (strcmp(name, "APOLLO") == 0 || strcmp(name, "apollo") == 0 ||
        strcmp(name, "AD9084") == 0 || strcmp(name, "ad9084") == 0) {
        return RAW_DEV_APOLLO;
    }
    if (strcmp(name, "HMC7044") == 0 || strcmp(name, "hmc7044") == 0 ||
        strcmp(name, "HMC") == 0     || strcmp(name, "hmc") == 0) {
        return RAW_DEV_HMC7044;
    }
    if (strcmp(name, "ADF4382") == 0 || strcmp(name, "adf4382") == 0) {
        return RAW_DEV_ADF4382;
    }
    if (strcmp(name, "ADF4030") == 0 || strcmp(name, "adf4030") == 0) {
        return RAW_DEV_ADF4030;
    }

    return RAW_DEV_UNKNOWN;
}

int32_t raw_regio_read(raw_device_e dev, uint32_t address, uint8_t *data)
{
    const raw_device_info_t *info;
    XSpi *inst;

    if (dev < 0 || dev >= RAW_DEV_COUNT || data == NULL) {
        return -1;
    }

    info = &g_dev_table[dev];
    inst = get_spi_instance(info->spi_instance);

    if (spi_select_cs(inst, info->cs_index) != 0) {
        return -1;
    }

    memset(g_tx, 0, sizeof(g_tx));
    memset(g_rx, 0, sizeof(g_rx));

    /* Build SPI frame: [addr_hi | read_bit, addr_lo, 0x00] */
    g_tx[0] = ((address >> 8) & 0xFFU) | info->read_bit;
    g_tx[1] = (address & 0xFFU);
    g_tx[2] = 0x00U;  /* dummy byte for read */

    if (spi_xfer(inst, g_tx, g_rx, 3) != 0) {
        return -1;
    }

    *data = g_rx[2];
    return 0;
}

int32_t raw_regio_write(raw_device_e dev, uint32_t address, uint8_t data)
{
    const raw_device_info_t *info;
    XSpi *inst;

    if (dev < 0 || dev >= RAW_DEV_COUNT) {
        return -1;
    }

    info = &g_dev_table[dev];
    inst = get_spi_instance(info->spi_instance);

    if (spi_select_cs(inst, info->cs_index) != 0) {
        return -1;
    }

    memset(g_tx, 0, sizeof(g_tx));

    /* Build SPI frame: [addr_hi & ~read_bit, addr_lo, data] */
    g_tx[0] = ((address >> 8) & 0xFFU) & (uint8_t)(~info->read_bit);
    g_tx[1] = (address & 0xFFU);
    g_tx[2] = data;

    if (spi_xfer(inst, g_tx, g_rx, 3) != 0) {
        return -1;
    }

    return 0;
}

int32_t raw_regio_rmw(raw_device_e dev, uint32_t address, uint8_t mask, uint8_t data)
{
    uint8_t val;
    int32_t err;

    err = raw_regio_read(dev, address, &val);
    if (err != 0) return err;

    val = (val & (uint8_t)(~mask)) | (data & mask);

    return raw_regio_write(dev, address, val);
}

int32_t raw_regio_dump(raw_device_e dev, uint32_t start_addr, uint32_t count)
{
    uint32_t i;
    uint8_t val;
    int32_t err;
    const raw_device_info_t *info;

    if (dev < 0 || dev >= RAW_DEV_COUNT) return -1;
    if (count == 0U || count > 256U) {
        xil_printf("[RAW] ERROR: count must be 1-256\r\n");
        return -1;
    }

    info = &g_dev_table[dev];
    xil_printf("[%s] DUMP 0x%04X..0x%04X (%u regs)\r\n",
               info->name, start_addr, start_addr + count - 1U, count);

    /* Header */
    xil_printf("ADDR      ");
    for (i = 0; i < 16U && i < count; i++) {
        xil_printf("+%X ", (unsigned)(i & 0xFU));
    }
    xil_printf("\r\n");

    for (i = 0; i < count; i++) {
        if ((i % 16U) == 0U) {
            xil_printf("0x%04X:   ", start_addr + i);
        }

        err = raw_regio_read(dev, start_addr + i, &val);
        if (err != 0) {
            xil_printf("XX ");
        } else {
            xil_printf("%02X ", val);
        }

        if ((i % 16U) == 15U || i == count - 1U) {
            xil_printf("\r\n");
        }
    }

    return 0;
}

/*============= I N T E R A C T I V E   S H E L L ==========================*/

/* Known chip ID registers for 'scan' command */
typedef struct {
    raw_device_e dev;
    const char  *label;
    uint32_t     addr;
} scan_entry_t;

static const scan_entry_t g_scan_table[] = {
    { RAW_DEV_APOLLO,  "APOLLO  CHIP_TYPE       ", 0x0003 },
    { RAW_DEV_APOLLO,  "APOLLO  PRODUCT_ID_L    ", 0x0004 },
    { RAW_DEV_APOLLO,  "APOLLO  PRODUCT_ID_H    ", 0x0005 },
    { RAW_DEV_APOLLO,  "APOLLO  CHIP_GRADE      ", 0x0006 },
    { RAW_DEV_APOLLO,  "APOLLO  SPI_REVISION    ", 0x000B },
    { RAW_DEV_HMC7044, "HMC7044 PRODUCT_ID_L    ", 0x0078 },
    { RAW_DEV_HMC7044, "HMC7044 PRODUCT_ID_H    ", 0x0079 },
    { RAW_DEV_ADF4382, "ADF4382 CHIP_TYPE       ", 0x0003 },
    { RAW_DEV_ADF4382, "ADF4382 PRODUCT_ID_L    ", 0x0004 },
    { RAW_DEV_ADF4382, "ADF4382 PRODUCT_ID_H    ", 0x0005 },
    { RAW_DEV_ADF4030, "ADF4030 CHIP_TYPE       ", 0x0003 },
    { RAW_DEV_ADF4030, "ADF4030 PRODUCT_ID_L    ", 0x0004 },
    { RAW_DEV_ADF4030, "ADF4030 PRODUCT_ID_H    ", 0x0005 },
};

#define SCAN_TABLE_SIZE  (sizeof(g_scan_table) / sizeof(g_scan_table[0]))

static void shell_print_help(void)
{
    xil_printf("\r\n=== RAW REGISTER I/O SHELL ===\r\n");
    xil_printf("Devices: APOLLO (AD9084), HMC7044 (HMC), ADF4382, ADF4030\r\n\r\n");
    xil_printf("Commands:\r\n");
    xil_printf("  r <DEV> <ADDR>                  Read register\r\n");
    xil_printf("  w <DEV> <ADDR> <DATA>           Write register\r\n");
    xil_printf("  d <DEV> <ADDR> <COUNT>          Dump consecutive regs (hex table)\r\n");
    xil_printf("  rmw <DEV> <ADDR> <MASK> <DATA>  Read-modify-write\r\n");
    xil_printf("  scan                            Read all chip ID registers\r\n");
    xil_printf("  scan <DEV>                      Read chip ID for one device\r\n");
    xil_printf("  help                            Show this help\r\n");
    xil_printf("  q                               Quit shell\r\n\r\n");
    xil_printf("Numbers: 0x1234 (hex) or 4660 (decimal)\r\n\r\n");
    xil_printf("Examples:\r\n");
    xil_printf("  r APOLLO 0x0003\r\n");
    xil_printf("  w HMC7044 0x0001 0x55\r\n");
    xil_printf("  d ADF4382 0x0000 32\r\n");
    xil_printf("  rmw ADF4030 0x0010 0x0F 0x05\r\n\r\n");
}

static uint32_t parse_uint32(const char *s)
{
    if (s == NULL) return 0;
    return (uint32_t)strtoul(s, NULL, 0);  /* auto-detect hex with 0x prefix */
}

/* Simple line reader using xil_printf/inbyte */
#define CMD_BUF_SIZE  128
#define MAX_TOKENS    6

static int32_t shell_readline(char *buf, uint32_t size)
{
    uint32_t idx = 0;
    char c;

    xil_printf("raw> ");

    while (idx < size - 1U) {
        c = (char)inbyte();

        if (c == '\r' || c == '\n') {
            buf[idx] = '\0';
            xil_printf("\r\n");
            return (int32_t)idx;
        }
        else if (c == '\b' || c == 0x7F) {
            if (idx > 0U) {
                idx--;
                xil_printf("\b \b");
            }
        }
        else if (c >= 0x20) {
            buf[idx++] = c;
            outbyte(c);
        }
    }

    buf[idx] = '\0';
    xil_printf("\r\n");
    return (int32_t)idx;
}

static int32_t tokenize(char *buf, char *tokens[], int32_t max_tokens)
{
    int32_t count = 0;
    char *p = buf;

    while (*p != '\0' && count < max_tokens) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        tokens[count++] = p;

        /* Advance to next whitespace */
        while (*p != '\0' && *p != ' ' && *p != '\t') p++;
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    return count;
}

static void cmd_scan(raw_device_e filter)
{
    uint32_t i;
    uint8_t val;
    int32_t err;

    xil_printf("\r\n--- CHIP ID SCAN ---\r\n");

    for (i = 0; i < SCAN_TABLE_SIZE; i++) {
        if (filter != RAW_DEV_UNKNOWN && g_scan_table[i].dev != filter) {
            continue;
        }

        err = raw_regio_read(g_scan_table[i].dev, g_scan_table[i].addr, &val);
        if (err == 0) {
            xil_printf("  %s [0x%04X] = 0x%02X\r\n",
                       g_scan_table[i].label, g_scan_table[i].addr, val);
        } else {
            xil_printf("  %s [0x%04X] = ERROR\r\n",
                       g_scan_table[i].label, g_scan_table[i].addr);
        }
    }

    xil_printf("-------------------\r\n\r\n");
}

void raw_regio_shell(void)
{
    char buf[CMD_BUF_SIZE];
    char *tokens[MAX_TOKENS];
    int32_t ntok;
    raw_device_e dev;
    uint32_t addr;
    uint8_t data, mask;
    uint32_t count;
    int32_t err;

    shell_print_help();

    while (1) {
        shell_readline(buf, CMD_BUF_SIZE);
        ntok = tokenize(buf, tokens, MAX_TOKENS);

        if (ntok == 0) continue;

        /* ---- QUIT ---- */
        if (strcmp(tokens[0], "q") == 0 || strcmp(tokens[0], "quit") == 0) {
            xil_printf("[RAW] Shell exiting.\r\n");
            return;
        }

        /* ---- HELP ---- */
        if (strcmp(tokens[0], "help") == 0 || strcmp(tokens[0], "?") == 0) {
            shell_print_help();
            continue;
        }

        /* ---- SCAN ---- */
        if (strcmp(tokens[0], "scan") == 0) {
            if (ntok >= 2) {
                dev = raw_regio_resolve_name(tokens[1]);
                cmd_scan(dev);
            } else {
                cmd_scan(RAW_DEV_UNKNOWN);  /* scan all */
            }
            continue;
        }

        /* ---- READ ---- */
        if (strcmp(tokens[0], "r") == 0 || strcmp(tokens[0], "read") == 0) {
            if (ntok < 3) {
                xil_printf("Usage: r <DEV> <ADDR>\r\n");
                continue;
            }
            dev = raw_regio_resolve_name(tokens[1]);
            if (dev == RAW_DEV_UNKNOWN) {
                xil_printf("Unknown device: %s\r\n", tokens[1]);
                continue;
            }
            addr = parse_uint32(tokens[2]);
            err = raw_regio_read(dev, addr, &data);
            if (err == 0) {
                xil_printf("[%s] 0x%04X = 0x%02X (%u)\r\n",
                           g_dev_table[dev].name, addr, data, data);
            } else {
                xil_printf("[%s] 0x%04X READ FAILED\r\n",
                           g_dev_table[dev].name, addr);
            }
            continue;
        }

        /* ---- WRITE ---- */
        if (strcmp(tokens[0], "w") == 0 || strcmp(tokens[0], "write") == 0) {
            if (ntok < 4) {
                xil_printf("Usage: w <DEV> <ADDR> <DATA>\r\n");
                continue;
            }
            dev = raw_regio_resolve_name(tokens[1]);
            if (dev == RAW_DEV_UNKNOWN) {
                xil_printf("Unknown device: %s\r\n", tokens[1]);
                continue;
            }
            addr = parse_uint32(tokens[2]);
            data = (uint8_t)parse_uint32(tokens[3]);

            err = raw_regio_write(dev, addr, data);
            if (err == 0) {
                xil_printf("[%s] 0x%04X <= 0x%02X OK\r\n",
                           g_dev_table[dev].name, addr, data);
            } else {
                xil_printf("[%s] 0x%04X <= 0x%02X FAILED\r\n",
                           g_dev_table[dev].name, addr, data);
            }
            continue;
        }

        /* ---- DUMP ---- */
        if (strcmp(tokens[0], "d") == 0 || strcmp(tokens[0], "dump") == 0) {
            if (ntok < 4) {
                xil_printf("Usage: d <DEV> <ADDR> <COUNT>\r\n");
                continue;
            }
            dev = raw_regio_resolve_name(tokens[1]);
            if (dev == RAW_DEV_UNKNOWN) {
                xil_printf("Unknown device: %s\r\n", tokens[1]);
                continue;
            }
            addr  = parse_uint32(tokens[2]);
            count = parse_uint32(tokens[3]);
            raw_regio_dump(dev, addr, count);
            continue;
        }

        /* ---- READ-MODIFY-WRITE ---- */
        if (strcmp(tokens[0], "rmw") == 0) {
            if (ntok < 5) {
                xil_printf("Usage: rmw <DEV> <ADDR> <MASK> <DATA>\r\n");
                continue;
            }
            dev = raw_regio_resolve_name(tokens[1]);
            if (dev == RAW_DEV_UNKNOWN) {
                xil_printf("Unknown device: %s\r\n", tokens[1]);
                continue;
            }
            addr = parse_uint32(tokens[2]);
            mask = (uint8_t)parse_uint32(tokens[3]);
            data = (uint8_t)parse_uint32(tokens[4]);

            /* Show before */
            uint8_t before;
            raw_regio_read(dev, addr, &before);
            xil_printf("[%s] RMW 0x%04X: before=0x%02X, mask=0x%02X, data=0x%02X\r\n",
                       g_dev_table[dev].name, addr, before, mask, data);

            err = raw_regio_rmw(dev, addr, mask, data);
            if (err == 0) {
                uint8_t after;
                raw_regio_read(dev, addr, &after);
                xil_printf("[%s] RMW 0x%04X: after=0x%02X OK\r\n",
                           g_dev_table[dev].name, addr, after);
            } else {
                xil_printf("[%s] RMW FAILED\r\n", g_dev_table[dev].name);
            }
            continue;
        }

        /* ---- UNKNOWN ---- */
        xil_printf("Unknown command: '%s'. Type 'help'.\r\n", tokens[0]);
    }
}
