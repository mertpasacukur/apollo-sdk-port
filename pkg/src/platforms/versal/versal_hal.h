/*!
 * @brief     Versal Standalone platform configuration header file.
 *            Ported from ADS10 platform (ads10_hal.h) for Xilinx Versal + Standalone (baremetal).
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 *
 * @note      Porting by Alfred (AI Assistant) for Paşa's Versal platform.
 *            Platform-independent code preserved exactly from original SDK.
 *            Only platform-dependent HAL layer replaced.
 */

#ifndef __VERSAL_HAL__
#define __VERSAL_HAL__

/*============= I N C L U D E S ============*/
#include "adi_apollo_types.h"
#include "adi_adf4030_types.h"
#include "adi_adf4382_types.h"
#include "adi_hmc7044_types.h"

/*============= D E F I N E S ==============*/

/*
 * SPI Topology on Versal:
 *   SPI Instance 0: Apollo (AD9084) — dedicated
 *   SPI Instance 1: HMC7044 (CS0), ADF4030 (CS1), ADF4382 (CS2) — shared bus
 *
 * Both SPI instances are AXI Quad SPI PL IP, driven by Vitis 2023.2 XSpi BSP driver.
 */

/* Vitis xparameters.h will define these — placeholder device IDs */
#ifndef XPAR_SPI_0_DEVICE_ID
#define XPAR_SPI_0_DEVICE_ID    0   /* Apollo SPI */
#endif
#ifndef XPAR_SPI_1_DEVICE_ID
#define XPAR_SPI_1_DEVICE_ID    1   /* HMC7044 / ADF4030 / ADF4382 SPI */
#endif

/* Slave select masks for SPI Instance 1 */
#define VERSAL_SPI1_SS_HMC7044  0x01    /* CS0 */
#define VERSAL_SPI1_SS_ADF4030  0x02    /* CS1 */
#define VERSAL_SPI1_SS_ADF4382  0x04    /* CS2 */

/* FPGA register base address (AXI-accessible from PS/PL) */
#ifndef VERSAL_FPGA_REG_BASEADDR
#define VERSAL_FPGA_REG_BASEADDR    0x80000000  /* TODO: Set from actual Vivado address map */
#endif

/* Apollo reset GPIO — directly controlled via AXI GPIO or EMIO */
#ifndef VERSAL_APOLLO_RESETB_GPIO
#define VERSAL_APOLLO_RESETB_GPIO   0   /* TODO: Set from actual design */
#endif

/* QSPI Flash for FW images */
#define VERSAL_QSPI_DEVICE_ID       0   /* PS QSPI, MT25QU02G */

/*============= E N U M S ==============*/

/*============= S T R U C T S ==============*/

/**
 * @brief   SPI configuration for Versal AXI Quad SPI.
 *          Maps to the same concept as ads10's hal_spi_config_t.
 */
typedef struct {
    void      *user_data;       /*!< XSpi instance pointer */
    char      *name;            /*!< Descriptive name */
    uint8_t    spi_cs;          /*!< Chip select number (slave select) */
    uint8_t    spi_3wire;       /*!< 0 = 4-wire, 1 = 3-wire */
    uint8_t    spi_lsb_first;   /*!< 0 = MSB first, 1 = LSB first */
} versal_spi_config_t;

/**
 * @brief   Platform HAL config for Versal — equivalent to adi_fpga_apollo_hal_config_t on ADS10.
 */
typedef struct {
    void                  *user_data;
    versal_spi_config_t   *spi0;    /*!< Apollo SPI (Instance 0) */
    versal_spi_config_t   *spi1;    /*!< HMC7044/ADF4030/ADF4382 SPI (Instance 1) */
} versal_hal_config_t;


/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Allocates and returns the Versal platform HAL config.
 *          Equivalent to ads10_apollo_hal_instance().
 */
versal_hal_config_t *versal_apollo_hal_instance(void);

/**
 * @brief   Configures SPI settings for Apollo device.
 *          Equivalent to ads10_apollo_hal_config_data().
 */
uint32_t versal_apollo_hal_config_data(adi_apollo_device_t *device, uint8_t spi0_cs, uint8_t spi1_cs);

/**
 * @brief   Initialize HMC7044 SPI config.
 */
int32_t versal_hmc7044_hal_spi_config_init(versal_spi_config_t *config);

/**
 * @brief   Configure HMC7044 HAL.
 */
int32_t versal_hmc7044_hal_config_data(adi_hmc7044_device_t *device, int32_t(*spi_config_init)(versal_spi_config_t*));

/**
 * @brief   Initialize ADF4382 SPI config.
 */
int32_t versal_adf4382_hal_spi_config_init(versal_spi_config_t *config);

/**
 * @brief   Configure ADF4382 HAL.
 */
int32_t versal_adf4382_hal_config_data(adi_adf4382_device_t *device, int32_t(*spi_config_init)(versal_spi_config_t*));

/**
 * @brief   Initialize ADF4030 SPI config.
 */
int32_t versal_adf4030_hal_spi_config_init(versal_spi_config_t *config);

/**
 * @brief   Configure ADF4030 HAL.
 */
int32_t versal_adf4030_hal_config_data(adi_adf4030_device_t *device, int32_t(*spi_config_init)(versal_spi_config_t*));

/**
 * @brief   Initialize Versal platform.
 *          Sets up AXI Quad SPI instances, GPIO for reset, QSPI for flash.
 *          Equivalent to ads10_hw_open().
 */
int32_t versal_hw_open(const char *log_file);

/**
 * @brief   De-initialize Versal platform.
 *          Equivalent to ads10_hw_close().
 */
int32_t versal_hw_close(void);

/**
 * @brief   Platform logging function.
 *          Uses xil_printf on baremetal.
 */
int32_t versal_log_write(void *user_data, int32_t log_type, const char *message, va_list argp);

/**
 * @brief   Platform delay function (microseconds).
 *          Uses usleep() from Vitis BSP (sleep.h).
 */
int32_t versal_wait_us(void *user_data, uint32_t time_us);

/**
 * @brief   Apollo RESETB pin control via GPIO.
 */
int32_t versal_hw_rst_pin_ctrl_apollo(void *user_data, uint8_t enable);

/**
 * @brief   Free user_data memory.
 */
int32_t versal_user_data_free(void **user_data);

/**
 * @brief   Read 32-bit FPGA register via AXI memory-mapped access.
 *          Uses Xil_In32().
 */
int32_t versal_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data);

/**
 * @brief   Write 32-bit FPGA register via AXI memory-mapped access.
 *          Uses Xil_Out32().
 */
int32_t versal_axi_reg_write32(uint32_t reg_offset, uint32_t data);

/**
 * @brief   SPI read function for Apollo HAL.
 *          Uses XSpi_Transfer().
 */
int32_t versal_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config);

/**
 * @brief   SPI write function for Apollo HAL.
 *          Uses XSpi_Transfer().
 */
int32_t versal_spi_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes, adi_apollo_hal_txn_config_t *txn_config);

/**
 * @brief   FPGA memory write (for capture/transmit DMA buffers).
 *          TODO: Implement when DMA is needed.
 */
int32_t versal_fpga_mem_write(uint32_t addr, uint32_t *data, uint32_t len);

/**
 * @brief   FPGA memory read (for capture/transmit DMA buffers).
 *          TODO: Implement when DMA is needed.
 */
int32_t versal_fpga_mem_read(uint32_t addr, uint32_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* __VERSAL_HAL__ */
