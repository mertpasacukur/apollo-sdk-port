/*!
 * @brief     Versal platform configuration header file.
 *
 *            Port of ads10_hal.h for Xilinx Versal Standalone (baremetal).
 *            Uses XSpi for SPI communication, Xil_In32/Out32 for FPGA regs.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef __VERSAL_HAL_H__
#define __VERSAL_HAL_H__

/*============= I N C L U D E S ============*/
#include <stdint.h>
#include <stdarg.h>

#include "adi_apollo_types.h"
#include "adi_adf4030_types.h"
#include "adi_adf4382_types.h"
#include "adi_hmc7044_types.h"

/*============= D E F I N E S ==============*/

/* SPI config names — must match what the SDK checks in device HAL code */
#define ADF4030_SPI_NAME    "ADF4030_SPI"
#define ADF4382_SPI_NAME    "ADF4382_SPI"
#define APOLLO_SPI0_NAME    "APOLLO_SPI0"
#define APOLLO_SPI1_NAME    "APOLLO_SPI1"
#define HMC7044_SPI_NAME    "HMC7044_SPI"

/*============= S T R U C T S ==============*/

/*!
 * @brief   SPI HAL configuration.
 *          Matches the original ads10_hal.h hal_spi_config_t layout
 *          so the SDK's device HAL code works without changes.
 */
typedef struct {
    void      *user_data;       /*!< User data (unused on Versal, kept for compatibility) */
    char      *name;            /*!< Device name string (e.g., APOLLO_SPI0_NAME) */
    uint8_t   spi_cs;           /*!< Chip select index for this device */
    uint8_t   spi_3wire;        /*!< 0 = 4-wire SPI, 1 = 3-wire SPI */
    uint8_t   spi_lsb_first;    /*!< 0 = MSB first, 1 = LSB first */
} hal_spi_config_t;

/*!
 * @brief   HSCI HAL configuration (stub — not used on Versal initially).
 */
typedef struct {
    void      *user_data;
} hal_hsci_config_t;

/*!
 * @brief   Apollo HAL configuration combining SPI0, SPI1, and HSCI.
 *          Same layout as ads10_hal.h adi_fpga_apollo_hal_config_t.
 */
typedef struct {
    void                *user_data;
    hal_spi_config_t    *spi0;      /*!< Apollo SPI0 config */
    hal_spi_config_t    *spi1;      /*!< Apollo SPI1 config */
    hal_hsci_config_t   *hsci;      /*!< HSCI config (stub) */
} adi_fpga_apollo_hal_config_t;


/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/* --- Hardware Open / Close --- */
int32_t versal_hw_open(void);
int32_t versal_hw_close(void);

/* --- Apollo HAL instance --- */
adi_fpga_apollo_hal_config_t *versal_apollo_hal_instance(void);

/* --- Apollo HAL config data (sets SPI config from device struct) --- */
uint32_t versal_apollo_hal_config_data(adi_apollo_device_t *device, uint8_t spi0_cs, uint8_t spi1_cs);

/* --- Device HAL config (SPI init for each device) --- */
int32_t versal_hmc7044_hal_spi_config_init(hal_spi_config_t *config);
int32_t versal_hmc7044_hal_config_data(adi_hmc7044_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

int32_t versal_adf4382_hal_spi_config_init(hal_spi_config_t *config);
int32_t versal_adf4382_hal_config_data(adi_adf4382_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

int32_t versal_adf4030_hal_spi_config_init(hal_spi_config_t *config);
int32_t versal_adf4030_hal_config_data(adi_adf4030_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

/* --- SPI Read / Write --- */
int32_t versal_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[],
                        uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config);
int32_t versal_spi_write(void *user_data, const uint8_t tx_data[],
                         uint32_t num_tx_bytes, adi_apollo_hal_txn_config_t *txn_config);

/* --- CMS-compatible SPI (for HMC7044 etc.) --- */
int32_t versal_cms_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[],
                            uint32_t num_tx_rx_bytes, adi_cms_hal_txn_config_t *txn_config);
int32_t versal_cms_spi_write(void *user_data, const uint8_t tx_data[],
                             uint32_t num_tx_bytes, adi_cms_hal_txn_config_t *txn_config);

/* --- Delay --- */
int32_t versal_wait_us(void *user_data, uint32_t time_us);

/* --- Reset Pin --- */
int32_t versal_hw_rst_pin_ctrl_apollo(void *user_data, uint8_t enable);

/* --- Logging --- */
int32_t versal_log_write(void *user_data, int32_t log_type, const char *message, va_list argp);

/* --- User Data Free --- */
int32_t versal_user_data_free(void **user_data);

/* --- FPGA Register Access --- */
int32_t versal_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data);
int32_t versal_axi_reg_write32(uint32_t reg_offset, uint32_t data);

/* --- FPGA Memory Access (DMA stubs) --- */
int32_t versal_fpga_mem_read(uint32_t mem_addr, uint32_t num_bytes, uint8_t *buffer);
int32_t versal_fpga_mem_write(uint32_t mem_addr, uint32_t num_bytes, uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* __VERSAL_HAL_H__ */
