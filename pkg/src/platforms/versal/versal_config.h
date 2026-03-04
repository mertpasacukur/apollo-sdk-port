/*!
 * @brief     Versal platform hardware configuration for Apollo SDK port.
 *
 *            All configurable hardware addresses, device IDs, and platform
 *            constants are defined here. Update these values to match your
 *            Vivado block design and xparameters.h.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone (baremetal).
 */

#ifndef __VERSAL_CONFIG_H__
#define __VERSAL_CONFIG_H__

/*============= A X I   Q U A D   S P I   D E V I C E   I D s ==============*/
/*
 * SPI0: AD9084 (Apollo) — dedicated bus
 * SPI1: HMC7044 (CS0), ADF4030 (CS1), ADF4382 (CS2) — shared bus
 *
 * TODO: Replace with XPAR_SPI_x_DEVICE_ID from your xparameters.h
 */
#define VERSAL_SPI0_DEVICE_ID       0
#define VERSAL_SPI1_DEVICE_ID       1

/*============= F P G A   R E G I S T E R   B A S E   A D D R E S S ========*/
/*
 * AXI base address of the Apollo FPGA register block as seen from Versal PS.
 *
 * TODO: Fill in from Vivado Address Editor once FPGA design is finalized.
 *       This is the base address assigned to the custom Apollo register block
 *       in the Versal block design. Typically found in xparameters.h as
 *       XPAR_<IP_NAME>_BASEADDR after bitstream generation.
 */
/* #define VERSAL_FPGA_REG_BASE_ADDR   0x????????UL */

/*============= G P I O   F O R   R E S E T   P I N =======================*/
/*
 * AD9084 hardware reset via AXI GPIO.
 *
 * TODO: Fill in from Vivado Address Editor once GPIO IP is placed.
 *       - VERSAL_GPIO_BASE_ADDR: Base address of the AXI GPIO instance
 *         controlling AD9084 RESETB pin.
 *       - VERSAL_RESET_PIN_BIT: Bit position within GPIO channel that
 *         drives the AD9084 RESETB line.
 *       Values come from xparameters.h after bitstream generation.
 */
/* #define VERSAL_GPIO_BASE_ADDR       0x????????UL */
/* #define VERSAL_GPIO_DATA_OFFSET     0x00            */
/* #define VERSAL_GPIO_TRI_OFFSET      0x04            */
/* #define VERSAL_RESET_PIN_BIT        ?               */

/*============= A X I   J E S D   R E G I S T E R S =======================*/
/*
 * FPGA-register-relative offsets for Apollo FPGA misc/control registers.
 *
 * TODO: Fill in once the FPGA register map is defined.
 *       These offsets are relative to VERSAL_FPGA_REG_BASE_ADDR.
 *       On the original ADS10 platform the values were:
 *         AXI_FPGA_MISC_1_REG = 0x10B
 *         AXI_FPGA_DUT_RSTB   = 0x002
 *         AXI_FPGA_DUT_TX_EN  = 0x00C
 *         AXI_FPGA_DUT_RX_EN  = 0x030
 *       Verify against your Versal FPGA design's register map.
 */
/* #define AXI_FPGA_MISC_1_REG        0x??? */
/* #define AXI_FPGA_DUT_RSTB          0x??? */
/* #define AXI_FPGA_DUT_TX_EN         0x??? */
/* #define AXI_FPGA_DUT_RX_EN         0x??? */

/*============= P S   Q S P I   C O N F I G ================================*/
/*
 * MT25QU02G flash for firmware storage.
 * TODO: Replace with XPAR_XQSPIPSU_0_DEVICE_ID from xparameters.h
 */
#define VERSAL_QSPI_DEVICE_ID      0

/*============= F I R M W A R E   F L A S H   L A Y O U T ==================*/
/*
 * 2MB per firmware partition, configurable.
 *
 * Each partition starts with a 256-byte header:
 *   Offset  Size   Field
 *   ------  ----   -----
 *   0x00    4      fw_id       (uint32_t LE) — Firmware ID (adi_apollo_startup_fw_id_e)
 *   0x04    4      fw_size     (uint32_t LE) — Firmware data size in bytes (excludes header)
 *   0x08    4      checksum    (uint32_t LE) — Reserved for CRC32 over fw data (not validated yet)
 *   0x0C    244    reserved    — Padding, must be 0xFF. Reserved for future use.
 *
 * Firmware data immediately follows the 256-byte header.
 * Partition format: [256-byte header][raw firmware data]
 *
 * TODO: Implement fw_id validation against requested fw_id.
 * TODO: Implement CRC32 checksum validation over firmware data.
 * TODO: Adjust base offset and partition size for your flash layout.
 */
#define VERSAL_FW_HEADER_SIZE       256U
#define VERSAL_FW_FLASH_BASE_OFFSET 0x01000000UL  /* 16MB into flash */
#define VERSAL_FW_PARTITION_SIZE    0x00200000UL  /* 2MB per partition */

/*============= O U T P U T   D I R ========================================*/
/*
 * On the original platform, OUTPUT_DIR is a filesystem path. On Versal baremetal, we don't
 * have a filesystem, so utilities that write files are stubbed out.
 */
#define OUTPUT_DIR                  "/tmp"

/*============= F W   I M A G E S   D I R ==================================*/
/*
 * On the original platform, FW_IMAGES_DIR is a filesystem path containing .bin files.
 * On Versal, FW comes from QSPI flash. This define is kept for compatibility.
 */
#define FW_IMAGES_DIR               "qspi_flash"

#endif /* __VERSAL_CONFIG_H__ */
