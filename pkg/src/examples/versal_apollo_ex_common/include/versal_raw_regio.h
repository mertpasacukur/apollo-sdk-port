/*!
 * @brief     Versal Raw Register I/O — API-Free Direct SPI Access
 *
 *            Provides direct SPI read/write for Apollo (AD9084), HMC7044,
 *            ADF4382, and ADF4030 WITHOUT requiring the full API device
 *            structs to be initialized. Only needs versal_hw_open() to
 *            init the SPI instances.
 *
 *            Includes an interactive UART shell for register debugging:
 *              > r APOLLO 0x47000200
 *              > w APOLLO 0x47000200 0xAA
 *              > d HMC7044 0x0000 16
 *              > rmw ADF4382 0x0010 0x0F 0x05
 *
 *            SPI Topology (from versal_config.h):
 *              SPI0: AD9084 (Apollo) — CS0
 *              SPI1: HMC7044 (CS0), ADF4030 (CS1), ADF4382 (CS2)
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef VERSAL_RAW_REGIO_H
#define VERSAL_RAW_REGIO_H

#include <stdint.h>

/*============= D E V I C E   E N U M ======================================*/

typedef enum {
    RAW_DEV_APOLLO  = 0,
    RAW_DEV_HMC7044 = 1,
    RAW_DEV_ADF4382 = 2,
    RAW_DEV_ADF4030 = 3,
    RAW_DEV_COUNT   = 4,
    RAW_DEV_UNKNOWN = -1
} raw_device_e;

/*============= S P I   P R O T O C O L   I N F O =========================*/

typedef struct {
    const char *name;           /*!< Human-readable name */
    uint8_t     spi_instance;   /*!< 0 = SPI0, 1 = SPI1 */
    uint8_t     cs_index;       /*!< Chip-select index */
    uint8_t     addr_bytes;     /*!< Address width: 2 = 16-bit, 3 = 24-bit (Apollo paged) */
    uint8_t     spi_3wire;      /*!< 0 = 4-wire, 1 = 3-wire */
    uint8_t     read_bit;       /*!< Bit to OR into addr byte for read (0x80 typical) */
} raw_device_info_t;

/*============= P U B L I C   A P I ========================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief  Initialize raw regio module. Call AFTER versal_hw_open().
 *         This does NOT need any API device structs.
 */
void raw_regio_init(void);

/*!
 * @brief  Raw SPI register read.
 *
 * @param[in]  dev      Device enum
 * @param[in]  address  Register address (16-bit for HMC/ADF, up to 32-bit for Apollo)
 * @param[out] data     Read value (8-bit)
 * @return 0 on success, negative on error
 */
int32_t raw_regio_read(raw_device_e dev, uint32_t address, uint8_t *data);

/*!
 * @brief  Raw SPI register write.
 *
 * @param[in]  dev      Device enum
 * @param[in]  address  Register address
 * @param[in]  data     Value to write (8-bit)
 * @return 0 on success, negative on error
 */
int32_t raw_regio_write(raw_device_e dev, uint32_t address, uint8_t data);

/*!
 * @brief  Read-Modify-Write: read reg, apply (val & ~mask) | (data & mask), write back.
 *
 * @param[in] dev      Device enum
 * @param[in] address  Register address
 * @param[in] mask     Bits to modify
 * @param[in] data     New value for masked bits
 * @return 0 on success, negative on error
 */
int32_t raw_regio_rmw(raw_device_e dev, uint32_t address, uint8_t mask, uint8_t data);

/*!
 * @brief  Dump consecutive registers.
 *
 * @param[in] dev        Device enum
 * @param[in] start_addr Start address
 * @param[in] count      Number of registers (max 256)
 * @return 0 on success, negative on error
 */
int32_t raw_regio_dump(raw_device_e dev, uint32_t start_addr, uint32_t count);

/*!
 * @brief  Resolve device name string to enum.
 *         Accepts: "APOLLO", "HMC7044", "HMC", "ADF4382", "ADF4030"
 */
raw_device_e raw_regio_resolve_name(const char *name);

/*!
 * @brief  Interactive UART shell. Blocks forever, processes commands.
 *
 *         Commands:
 *           r <DEV> <ADDR>              — Read register
 *           w <DEV> <ADDR> <DATA>       — Write register
 *           d <DEV> <ADDR> <COUNT>      — Dump registers
 *           rmw <DEV> <ADDR> <MASK> <DATA> — Read-modify-write
 *           scan <DEV>                  — Read chip/product ID registers
 *           help                        — Show commands
 *           q                           — Quit shell (return to caller)
 *
 *         All numeric values accept 0x hex or decimal.
 */
void raw_regio_shell(void);

#ifdef __cplusplus
}
#endif

#endif /* VERSAL_RAW_REGIO_H */
