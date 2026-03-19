/*!
 * @brief     ADS10 platform configuration and control source file.
 *
 * @copyright copyright(c) 2021 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS10__
 * @{
 */

/*============= I N C L U D E S ============*/
#define _POSIX_C_SOURCE     199309L     // To fix Error: 'CLOCK_REALTIME' undeclared.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <byteswap.h>
#include "ads10_hal.h"
#include "platform.h"
#include "hal_uio.h"


/*============= D E F I N E S ==============*/
/* log file limit */
#define MAX_FILE_NAME_LENGTH    64
#define MAX_LOG_LINE_LENGTH     1000
#define MAX_LOG_NUM_LINES       250000

/* sysbuf length for spi init */
#define SYSFS_DATALEN           32

/* definition of register offset for base address of peripheral interfaces stored within FPGA Registers. */
#define SPI_CHIP_SEL                    0x0901
#define FPGA_REG_VER_OFFSET             0x0908
#define SPI_BASE_ADDR_REG_OFFSET        0x0910
#define I2C_BASE_ADDR_REG_OFFSET        0x0911
#define FIFO_BASE_ADDR_REG_OFFSET       0x0912
#define HSCI_BASE_ADDR_REG_OFFSET       0x0913
#define HSCI_RATE_CTRL                  0x090A
#define HSCI_MASTER_CTRL_OFFSET         0x090B
#define HSCI_PHY_STATUS_OFFSET          0x090C

/* definition of AXI JESD204 register offsets */
#define AXI_FPGA_MISC_1_REG      0x10B      // Reg 0x10B
#define AXI_FPGA_DUT_RSTB        0x002      // Reg 0x10B; BitFIeld[1]
#define AXI_FPGA_DUT_TX_EN       0x00C      // Reg 0x10B; BitFIeld[3:2]
#define AXI_FPGA_DUT_RX_EN       0x030      // Reg 0x10B; BitFIeld[5:4]

/* definition of register offset for spi controller */
#define SPIM_REVID              0x0000
#define SPIM_BUF_RDDATA         0x0001 * 4
#define SPIM_MODE               0x8001 * 4
#define SPIM_XFER_COUNT         0x8002 * 4
#define SPIM_ADDR_COUNT         0x8003 * 4
#define SPIM_DATA_COUNT         0x8004 * 4
#define SPIM_TARGET             0x8005 * 4
#define SPIM_CTRL               0x8006 * 4
#define SPIM_BRAM_ADDR          0x8007 * 4          // Starting of the address
#define SPIM_RUN                0x8008 * 4
#define SPIM_STATUS             0x8009 * 4
#define SPIM_SCRATCH            0x800F * 4

/* definition of register offset for i2c controller */
#define I2CM_RX_FIFO             0x001
#define I2CM_TRANS_RESET         0x002
#define I2CM_STATUS              0x003
#define I2CM_TRANS_SETTINGS      0x010
#define I2CM_DEVICE_ADDR         0x011
#define I2CM_REG_ADDR            0x012
#define I2CM_READ_LEN            0x013
#define I2CM_TX_FIFO             0x014
#define I2CM_TRANS_CTRL          0x015
#define I2CM_ERRORS              0x020

// #define I2C_TIMEOUT_US          1000000
// #define WAIT_US                 100

/* definition of register offset for hsci controller */
#define HSCIM_REVID         0x0000
#define HSCIM_BUF_RDDATA    0x0001
#define HSCIM_MODE          0x8001
#define HSCIM_XFER_NUM      0x8002
#define HSCIM_ADDR_SIZE     0x8003
#define HSCIM_BYTE_NUM      0x8004
#define HSCIM_TARGET        0x8005

// Reg: HSCIM_CTRL
// hsci_cmd_sel [1:0]          : 2'b00 = Write, 2'b01 = Read,     2'b10 = RMW,  2'b11 = N/A
// hsci_slave_ahb_tsize [5:4]  : 2'b00 = byte,  2'b01 = halfword, 2'b10 = word, 2'b11 = N/A
#define HSCIM_CTRL          0x8006

#define HSCIM_BRAM_ADDR     0x8007
#define HSCIM_RUN           0x8008
#define HSCIM_STATUS        0x8009
#define HSCIM_LINKUP_CTRL   0x800A
#define HSCIM_TEST_CTRL     0x800B
#define HSCIM_LINKUP_STAT   0x800C
#define HSCIM_LINKUP_STAT2  0x800D
#define HSCIM_DEBUG_STAT    0x800E
#define MISO_TEST_BER       0x800F
#define HSCIM_LINK_ERR      0x8010
#define HSCIM_SCRATCH       0x801F
#define HSCIM_BUF_RDDATA_SIZE    0x7FFF
#define HSCIM_BUF_WRDATA_SIZE    HSCIM_BUF_RDDATA_SIZE - 4

/* SPI config names */
#define ADF4030_SPI_NAME    "ADF4030_SPI"
#define ADF4382_SPI_NAME    "ADF4382_SPI"
#define ADL6331_SPI_NAME    "ADL6331_SPI"
#define ADL6332_SPI_NAME    "ADL6332_SPI"
#define APOLLO_SPI0_NAME    "APOLLO_SPI0"
#define APOLLO_SPI1_NAME    "APOLLO_SPI1"
#define HMC7044_SPI_NAME    "HMC7044_SPI"
#define IS_SECONDARY_SPI0_DEVICE(spi_config) (\
       (strcmp(spi_config->name, HMC7044_SPI_NAME) == 0) \
    || (strcmp(spi_config->name, ADF4382_SPI_NAME) == 0) \
    || (strcmp(spi_config->name, ADL6331_SPI_NAME) == 0) \
    || (strcmp(spi_config->name, ADL6332_SPI_NAME) == 0) \
    || (strcmp(spi_config->name, ADF4030_SPI_NAME) == 0) \
)

/*============= D A T A ====================*/

/* definition of device memory base for register access */
static void*    g_fpga_reg_mem;         // g_axi_jesd204_mem
static void*    g_fpga_scratch_mem;
static void*    g_spi_ctrl_mem;
static void*    g_i2c_ctrl_mem;
static void*    g_hsci_ctrl_mem;


/* spi access counters */
static uint32_t g_spi_rd_cnt, g_spi_wr_cnt;

/* log file */
static FILE    *g_log_fd = NULL;
static char     g_log_file_name[MAX_FILE_NAME_LENGTH];
static uint32_t g_log_line_num;

/* time base */
static struct   timespec ts_start;

/* support for probing an address */
static int32_t probe_reg(uint32_t *reg_addr, const char *desc);
static void sigbus_handler(int sig_num);
static jmp_buf buf;

/*============= S T A T I C  F U N C T I O N S  D E C L A R A T I O N ====================*/

static int32_t ads10_hsci_write_rmw(void *user_data, const uint8_t tx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config);

/**
 * @brief   Checks if the Chip2Chip bridge between microzed and ADS10 is established.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t uz_ads10_c2c_bridge_detect(void);


/**
 * @brief       Function to read register value from ADS10 Scratch Register Map.
 *              Reads FPGA Register Memory from base address 0x44A80000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[out]  out_data    Read back value of register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_scratch_read32(uint32_t reg_offset, uint32_t *out_data) __attribute__((unused));


/**
 * @brief       Function to write register value to ADS10 Scratch Register Map.
 *              Writes FPGA Register Memory at base address 0x44A80000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[in]   data        Value to be written to the register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_scratch_write32(uint32_t reg_offset, uint32_t data) __attribute__((unused));


/**
 * @brief       Function to read register value from ADS10 SPI Master Register Map.
 *              Reads FPGA Register Memory from base address 0x44C00000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[out]  out_data    Read back value of register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_spi_reg_read32(uint32_t reg_offset, uint32_t *out_data);


/**
 * @brief       Function to write register value to ADS10 SPI Master Register Map.
 *              Writes FPGA Register Memory at base address 0x44C00000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[in]   data        Value to be written to the register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_spi_reg_write32(uint32_t reg_offset, uint32_t data);


/**
 * @brief       Function to read register value from ADS10 I2C Master Register Map.
 *              Reads FPGA Register Memory from base address 0x44A10000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[out]  out_data    Read back value of register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_i2c_reg_read32(uint32_t reg_offset, uint32_t *out_data);


/**
 * @brief       Function to write register value to ADS10 I2C Master Register Map.
 *              Writes FPGA Register Memory at base address 0x44A10000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[in]   data        Value to be written to the register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_i2c_reg_write32(uint32_t reg_offset, uint32_t data);

/**
 * @brief       Function to read register value from ADS10 HSCI Master Register Map.
 *              Reads FPGA Register Memory from base address 0x44D00000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[out]  out_data    Read back value of register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_hsci_reg_read32(uint32_t reg_offset, uint32_t *out_data);

/**
 * @brief       Function to write register value to ADS10 HSCI Master Register Map.
 *              Writes FPGA Register Memory at base address 0x44D00000 + reg_offset.
 *              AXI addressing is byte aligned hence offset is multiplied by 4.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[in]   data        Value to be written to the register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_hsci_reg_write32(uint32_t reg_offset, uint32_t data);

/**
 * @brief   Select the ADS10 SPI Master peripheral. Selection is done on Reg: 0x901.
 *
 * @param   spim_sel    Selects ADS10 SPI Master instance for Apollo (0), and other SPI devices (1).
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_spim_periph_sel(uint32_t spim_sel);


/**
 * @brief   Waits for SPI Master to be idle before starting any new transaction.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_spim_wait_idle(void);

/**
 * @brief   Waits for HSCI Master done status
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_hscim_wait_done(void);

/**
 * @brief   Waits for HSCI Master to establish an active link
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_hscim_wait_link_active(void);

/**
 * @brief   Configures SPI Master Control Register (0x8001).
 *
 * @param[in] spi_wire  Sets SPI wire mode. spi_wire = 0 (4 wire) spi_wire = 1 (3 wire).
 * @param[in] spi_msb   Sets SPI MSB mode. spi_msb = 0 (MSB_FIRST) & spi_msb = 1 (LSB_FIRST).
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_spim_configure(uint32_t spi_wire, uint32_t spi_msb);


/**
 * @brief   Waits for I2C Master to be idle before starting any new transaction.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_i2cm_wait_idle(void);


/**
 * @brief   Resets I2C Master if stuck during a transaction.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_i2cm_reset(void);


/**
 * @brief   Configures I2C Master Transaction Settings Register (0x10).
 *
 * @param[in] addr_type         Device Address type. 1b'0: 7 bit addressing; 1b'1: 10 bit addressing. @ref ads10_i2c_dev_addr_type_e for details.
 * @param[in] reg_addr_fmt      Register Address Format. 2b'00: 8 bit addressing; 2b'01: 16 bit addressing; 2b'10: 24 bit addressing; 2b'11: 32 bit addressing. @ref ads10_i2c_reg_addr_fmt_e for details.
 * @param[in] transaction_type  Transaction type. 1b'0: Write; 1b'1: Read. @ref ads10_i2c_xaction_type_e for details.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
static int32_t ads10_i2cm_configure(uint8_t addr_type, uint8_t reg_addr_fmt, uint8_t transaction_type);


/**
 * @brief   Initiates the logging functionality and creates a log file to record events.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_log_open();


/**
 * @brief   Closes the logging functionality.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
static int32_t ads10_log_close();


/**
 * @brief   Allocates memory for the platform SPI data structure.
 *
 * @return  Pointer to @ref hal_spi_config_t struct.
 */
static hal_spi_config_t *hal_spi_config_create();


/**
 * @brief   Allocates memory for the platform HSCI data structure.
 *
 * @return  Pointer to @ref hal_hsci_config_t struct.
 */
static hal_hsci_config_t *hal_hsci_config_create();

/**
 * @brief Creates a mapping from @ref adi_cms_hal_txn_config_t to @ref adi_apollo_hal_txn_config_t
 *
 * @param txn_config
 * @return adi_apollo_hal_txn_config_t
 */
static adi_apollo_hal_txn_config_t cms_hal_to_apollo_hal_map(adi_cms_hal_txn_config_t *txn_config);

static __maybe_unused int32_t fmcb_level_shifter_disable_all(void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

/*============= S T A T I C  F U N C T I O N  D E F I N I T I O N S ====================*/


static int32_t uz_ads10_c2c_bridge_detect(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    int32_t gpio_fd, bdg0_fd, bdg1_fd;
    char buf[2];

    /* configure GPIO */
    if (access("/sys/class/gpio/gpio970/value", F_OK) != 0) {
        system("echo 970 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo out > /sys/class/gpio/gpio970/direction");
        system("echo 0 > /sys/class/gpio/gpio970/value");
    }
    if (access("/sys/class/gpio/gpio969/value", F_OK) != 0) {
        system("echo 969 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo out > /sys/class/gpio/gpio969/direction");
        system("echo 1 > /sys/class/gpio/gpio969/value");
    }
    if (access("/sys/class/gpio/gpio968/value", F_OK) != 0) {
        system("echo 968 > /sys/class/gpio/export");
        usleep(10 * 1000);
    }
    usleep(50 * 1000);

    /* configure GPIO for chip-to-chip Bridge detection
     * GPIO961 is EMIO[1]
     * GPIO964 is EMIO[4]
     * GPIO965 is EMIO[5]
     */
    if (access("/sys/class/gpio/gpio961/value", F_OK) != 0) {
        system("echo 961 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo in > /sys/class/gpio/gpio961/direction");
    }
    if (access("/sys/class/gpio/gpio964/value", F_OK) != 0) {
        system("echo 964 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo in > /sys/class/gpio/gpio964/direction");
    }
    if (access("/sys/class/gpio/gpio965/value", F_OK) != 0) {
        system("echo 965 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo in > /sys/class/gpio/gpio965/direction");
    }

    gpio_fd = open("/sys/class/gpio/gpio961/value", O_RDONLY);
    if (gpio_fd > 0) {
        read(gpio_fd, buf, 1);
        if (buf[0] == '1') {
            memset(buf, 0, sizeof(buf));
            bdg0_fd = open("/sys/class/gpio/gpio964/value", O_RDONLY);
            bdg1_fd = open("/sys/class/gpio/gpio965/value", O_RDONLY);
            if ((bdg0_fd > 0) && (bdg1_fd > 0)) {
                read(bdg0_fd, &buf[0], 1);
                read(bdg1_fd, &buf[1], 1);
                if ((buf[0] == '1') && (buf[1] == '1')) {
                    err = API_CMS_ERROR_OK;
                    printf("Chip-to-chip bridge: Ready detected.\r\n");
                }
                else {
                    printf("Chip-to-chip bridge: Not ready, %c, %c.\r\n", buf[0], buf[1]);
                }
            }
            else {
                printf("Chip-to-chip bridge: Cannot find GPIO964 or GPIO965.\r\n");
            }
        }
        else {
            printf("Chip-to-chip bridge: ADS10_DONE is not ready.\r\n");
        }
        close(gpio_fd);
    }
    else {
        printf("Chip-to-chip bridge: Cannot find GPIO961.\r\n");
    }
    return err;
}


/*******************************************************************************************/

static int32_t ads10_scratch_read32(uint32_t reg_offset, uint32_t *out_data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_fpga_scratch_mem != NULL) {
        *out_data = *((uint32_t *)((uintptr_t)g_fpga_scratch_mem + 4*reg_offset));
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_scratch_write32(uint32_t reg_offset, uint32_t data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_fpga_scratch_mem != NULL) {
        *((uint32_t *)((uintptr_t)g_fpga_scratch_mem  +  4*reg_offset)) = data;
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_spi_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_spi_ctrl_mem != NULL) {
        *out_data = *((uint32_t *)((uintptr_t)g_spi_ctrl_mem + reg_offset));
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_spi_reg_write32(uint32_t reg_offset, uint32_t data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_spi_ctrl_mem != NULL) {
        *((uint32_t *)((uintptr_t)g_spi_ctrl_mem + reg_offset)) = data;
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_i2c_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_i2c_ctrl_mem != NULL) {
        *out_data = *((uint32_t *)((uintptr_t)g_i2c_ctrl_mem + 4*reg_offset));
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_i2c_reg_write32(uint32_t reg_offset, uint32_t data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_i2c_ctrl_mem != NULL) {
        *((uint32_t *)((uintptr_t)g_i2c_ctrl_mem + 4*reg_offset)) = data;
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_hsci_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_hsci_ctrl_mem != NULL) {
        *out_data = *((uint32_t *)((uintptr_t)g_hsci_ctrl_mem + 4*reg_offset));
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_hsci_reg_write32(uint32_t reg_offset, uint32_t data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_hsci_ctrl_mem != NULL) {
        *((uint32_t *)((uintptr_t)g_hsci_ctrl_mem + 4*reg_offset)) = data;
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

static int32_t ads10_spim_periph_sel(uint32_t spim_sel)
{
    uint32_t regRead;
    ads10_axi_reg_read32(SPI_CHIP_SEL, &regRead);
    regRead = regRead & 0xFFFFFFF0;
    /* spim_sel[0:3]: 1 - Other SPI devices, 0 - Apollo SPI */
	ads10_axi_reg_write32(SPI_CHIP_SEL, (regRead | spim_sel));
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

static int32_t ads10_spim_wait_idle(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t cnt = 0;
    uint32_t start, status = 0;
    do{
        ads10_spi_reg_read32(SPIM_RUN,  &start);
        ads10_spi_reg_read32(SPIM_STATUS, &status);

        if (((start & 0x1) == 0) && ((status & 0x2) == 0))
        {
            err = API_CMS_ERROR_OK;
        }
    }while ((cnt++ < 1000) && (err != 0));

    return err;
}

/*******************************************************************************************/

static int32_t ads10_hscim_wait_done(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t cnt = 0;
    uint32_t status = 0;
    do{
        ads10_hsci_reg_read32(HSCIM_STATUS, &status);

        if ((status & 0x1) == 1) {
            err = API_CMS_ERROR_OK;
        }
        // Add delay every 100 iterations *except the 0th*
        if ((cnt != 0) && ((cnt % 100) == 0)) {
            ads10_wait_us(NULL, 100);   // adjust delay to your needs
        }
    }while ((cnt++ < 1000) && (err != 0));

    return err;
}

static int32_t ads10_hscim_wait_link_active(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t cnt = 0;
    uint32_t status = 0;
    do{
        ads10_hsci_reg_read32(HSCIM_LINKUP_STAT, &status);

        if ((status & 0x1) == 1) {
            err = API_CMS_ERROR_OK;
        }
    }while ((cnt++ < 1000) && (err != 0));

    return err;
}

/*******************************************************************************************/

static int32_t ads10_spim_configure(uint32_t spi_wire, uint32_t spi_msb)
{
    /* SPI_MASTER MODE: LSB_FIRST = 1b'0, SPI_CONTINUOUS_CLK = 1b'0, SPI_MOSI_CONTROL = 2b'11, SPI_CPOL = 1b'0, SPI_CPHA = 1b'0, SPI_WIRE_SEL = 1b'0 */
    // ads10_spi_reg_write32(SPIM_MODE,        (0x18 | (spi_wire == 1 ? 0x01 : 0x00) | (spi_msb == 1 ? 0x00 : 0x40)));
    ads10_spi_reg_write32(SPIM_MODE, (0x18 | (spi_wire << 0) | (spi_msb << 6)));

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

static int32_t ads10_i2cm_wait_idle(void)
{
    int32_t  err = API_CMS_ERROR_ERROR;
    uint32_t i = 0;
    uint32_t start = 0;
    uint32_t status = 0;

    for(i = 0; i < 100; i++) {
        ads10_i2c_reg_read32(I2CM_TRANS_CTRL, &start);
        ads10_i2c_reg_read32(I2CM_STATUS, &status);
        if (((start & 0x1) == 0) && ((status & 0x3) == 0)) {
          err = API_CMS_ERROR_OK;
          break;
        }
        ads10_wait_us(NULL, 200);
    }
    return err;
}

/*******************************************************************************************/

int32_t ads10_i2cm_reset(void)
{
    ads10_i2c_reg_write32(I2CM_TRANS_RESET, 0x01);
    ads10_wait_us(NULL, 1000);
    ads10_i2c_reg_write32(I2CM_TRANS_RESET, 0x00);
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

static int32_t ads10_i2cm_configure(uint8_t addr_type, uint8_t reg_addr_fmt, uint8_t transaction_type)
{
    /* I2C TRANSACTION SETTING: REG_ADDR_FMT_EXT = 1b'0, TRANSACTION_TYPE = 1b'0, REG_ADDR_FMT = 1b'0, ADDR_TYPE = 1b'0, */

    uint8_t regVal = 0;
    regVal = (  (((reg_addr_fmt >> 1) & 1) << 3) |
                (transaction_type << 2) |
                ((reg_addr_fmt & 1) << 1) |
                (addr_type)  );

    ads10_i2c_reg_write32(I2CM_TRANS_SETTINGS, regVal); /* 0x04 for read, 0x0 for write */

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

static int32_t ads10_log_open(void)
{
    int32_t err;
    struct tm tm = { 0 };
    time_t t = time(NULL);

    if ((g_log_fd != NULL) || (g_log_file_name[0] == '\0')) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    g_log_fd = fopen(g_log_file_name, "w+");
    if (g_log_fd == NULL) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    tm = *localtime(&t);
    err = fprintf(g_log_fd, "000.000: API(ads10) log file[%04d-%02d-%02d %02d:%02d:%02d]\n\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (err < 0) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    if (fflush(g_log_fd) < 0) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    clock_gettime(CLOCK_REALTIME, &ts_start);

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

static int32_t ads10_log_close(void)
{
    if (g_log_fd != NULL) {
        if (fflush(g_log_fd) < 0) {
           return API_CMS_ERROR_LOG_CLOSE;
        }
        if (fclose(g_log_fd) < 0) {
           return API_CMS_ERROR_LOG_CLOSE;
        }
        g_log_fd = NULL;
    }
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

static hal_spi_config_t *hal_spi_config_create(void)
{
	hal_spi_config_t *spi_config = (hal_spi_config_t *)calloc(1, sizeof(hal_spi_config_t));

	return spi_config;
}

/*******************************************************************************************/

static hal_hsci_config_t *hal_hsci_config_create(void)
{
	hal_hsci_config_t *hsci_config = (hal_hsci_config_t *)calloc(1, sizeof(hal_hsci_config_t));
	return hsci_config;
}

/*******************************************************************************************/

















/*============= H A L  F U N C T I O N S  D E F I N I T I O N ====================*/

adi_fpga_apollo_hal_config_t *ads10_apollo_hal_instance(void)
{
	adi_fpga_apollo_hal_config_t *hal_cfg = (adi_fpga_apollo_hal_config_t *)calloc(1, sizeof(adi_fpga_apollo_hal_config_t));

	hal_cfg->spi0 = hal_spi_config_create();
	hal_cfg->spi1 = hal_spi_config_create();
	hal_cfg->hsci = hal_hsci_config_create();

	return hal_cfg;
}

/*******************************************************************************************/

uint32_t ads10_apollo_hal_config_data(adi_apollo_device_t *device, uint8_t spi0_cs, uint8_t spi1_cs)
{
    if(device == NULL)
        return API_CMS_ERROR_INVALID_HANDLE_PTR;

    adi_fpga_apollo_hal_config_t *hal_config = (adi_fpga_apollo_hal_config_t *)device->hal_info.dev_hal_info;

    // Configuration required for ADS10 SPI Master setup Reg:0x8001.
    // Used in ads10_spim_configure(uint32_t spi_wire, uint32_t spi_msb).
    // SPI Master LSB First Mode (1) or MSB First Mode (0)
    // SPI Master Interface 3-wire (1) or 4-wire (0)

    hal_config->spi0->name = APOLLO_SPI0_NAME;
    hal_config->spi0->spi_cs = spi0_cs;
    hal_config->spi0->spi_lsb_first = (int)((device->hal_info.spi0_desc.spi_config.msb) == (int)SPI_MSB_FIRST) ? 0 : 1;
    hal_config->spi0->spi_3wire = ((int)(device->hal_info.spi0_desc.spi_config.sdo) == (int)SPI_SDO) ? 0 : 1;

    hal_config->spi1->name = APOLLO_SPI1_NAME;
    hal_config->spi1->spi_cs = spi1_cs;
    hal_config->spi1->spi_lsb_first = (int)((device->hal_info.spi1_desc.spi_config.msb) == (int)SPI_MSB_FIRST) ? 0 : 1;
    hal_config->spi1->spi_3wire = (int)((device->hal_info.spi1_desc.spi_config.sdo) == (int)SPI_SDO) ? 0 : 1;

    // HSCI Config Remaining.

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_hmc7044_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = HMC7044_SPI_NAME;
    config->spi_3wire = 1;     // HMC7044 only supports 3 wire SPI.
    config->spi_cs = 1;
    config->spi_lsb_first = 0; // HMC7044 only supports MSB first.

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_hmc7044_hal_config_data(adi_hmc7044_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*))
{
    int32_t err;
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *) calloc(1, sizeof(hal_spi_config_t));

    if (spi_hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;

    if (spi_hal_cfg == NULL) {
        return API_CMS_ERROR_ERROR;
    }

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK) {
        return err;
    }

    device->hal_info.user_data = spi_hal_cfg;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adf4382_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = ADF4382_SPI_NAME;
    config->spi_3wire = 0;
    config->spi_cs = 0;
    config->spi_lsb_first = 0;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adf4382_hal_config_data(adi_adf4382_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*))
{
    int32_t err;
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *) calloc(1, sizeof(hal_spi_config_t));

    if (spi_hal_cfg == NULL) {
        return API_CMS_ERROR_ERROR;
    }

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK) {
        return err;
    }

    device->hal_info.user_data = spi_hal_cfg;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adf4030_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = ADF4030_SPI_NAME;
    config->spi_3wire = 0;
    config->spi_cs = 4;
    config->spi_lsb_first = 0;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adf4030_hal_config_data(adi_adf4030_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*))
{
    int32_t err;
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *) calloc(1, sizeof(hal_spi_config_t));
    if (spi_hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK) {
        return err;
    }

    device->hal_info.user_data = spi_hal_cfg;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adl6331_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = ADL6331_SPI_NAME;
    config->spi_3wire = 0;
    config->spi_cs = 3;
    config->spi_lsb_first = 0;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adl6331_hal_config_data(adi_adl6331_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*))
{
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *) calloc(1, sizeof(hal_spi_config_t));
    if (spi_hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;

    int32_t err;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK)
    {
        return err;
    }

    device->hal_info.user_data = spi_hal_cfg;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adl6332_hal_spi_config_init(hal_spi_config_t *config)
{
    config->name = ADL6332_SPI_NAME;
    config->spi_3wire = 0;
    config->spi_cs = 4;
    config->spi_lsb_first = 0;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_adl6332_hal_config_data(adi_adl6332_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*))
{
    hal_spi_config_t *spi_hal_cfg = (hal_spi_config_t *) calloc(1, sizeof(hal_spi_config_t));
    if (spi_hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;

    int32_t err;

    if (err = spi_config_init(spi_hal_cfg), err != API_CMS_ERROR_OK)
    {
        return err;
    }

    device->hal_info.user_data = spi_hal_cfg;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_ltc2977_hal_config_data(adi_ltc2977_device_t *device)
{
    device->device_i2c_addr = LTC2977_I2C_ADDR;

    return ads10_smbus_hal_config_data(&device->smbus);
}

/*******************************************************************************************/
int32_t ads10_ltm4681_hal_config_data(adi_ltm4681_device_t *device)
{
    device->chan_01_i2c_addr = LTM4681_01_I2C_ADDR;
    device->chan_23_i2c_addr = LTM4681_23_I2C_ADDR;

    return ads10_smbus_hal_config_data(&device->smbus);
}

/*******************************************************************************************/
int32_t ads10_ltc2980_hal_config_data(adi_ltc2980_device_t *device)
{
    device->device_a_i2c_addr = LTC2980_A_I2C_ADDR;
    device->device_b_i2c_addr = LTC2980_B_I2C_ADDR;
    device->device_c_i2c_addr = LTC2980_C_I2C_ADDR;

    return ads10_smbus_hal_config_data(&device->smbus);
}

/*******************************************************************************************/
int32_t ads10_smbus_hal_config_data(adi_smbus_hal_t *smbus)
{
    hal_i2c_config_t *i2c_hal_cfg = (hal_i2c_config_t *) calloc(1, sizeof(hal_i2c_config_t));

    if (i2c_hal_cfg == NULL) {
        return API_CMS_ERROR_ERROR;
    }

    i2c_hal_cfg->mux_channel = FMC_I2C;
    i2c_hal_cfg->device_addr_type = SEVEN_BIT_ADDRESS;
    i2c_hal_cfg->reg_addr_format = EIGHT_BIT_ADDRESS;

    smbus->user_data = i2c_hal_cfg;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_hw_open(const char *log_file)
{
    int32_t err = API_CMS_ERROR_HW_OPEN;

    /* reset spi rd/wr counter */
    g_spi_rd_cnt = 0;
    g_spi_wr_cnt = 0;

    /* save log file name */
    strncpy(g_log_file_name, log_file, MAX_FILE_NAME_LENGTH);

    /* open log */
    if (err = ads10_log_open(), err != API_CMS_ERROR_OK)
        return err;

#ifdef ADS10
    /* check c2c bridge */
    if (err = uz_ads10_c2c_bridge_detect(), err != API_CMS_ERROR_OK) {
        ads10_wait_us(NULL, 1000);
        if (err = uz_ads10_c2c_bridge_detect(), err != API_CMS_ERROR_OK)
            return err;
    }
#endif /* ADS10 */

#ifdef VCU128
    printf("We're on VCU128!!!\n");
#endif /* VCU128 */

	err = adi_uio_devices_init();
    ADI_CMS_ERROR_RETURN(err);

    size_t name_size = sizeof(FPGA_REGISTERS_NAME);
    g_fpga_reg_mem = adi_uio_device_mem_get(FPGA_REGISTERS_NAME, name_size);
    name_size = sizeof(FPGA_SCRATCH_MEM_NAME);
    g_fpga_scratch_mem = adi_uio_device_mem_get(FPGA_SCRATCH_MEM_NAME, name_size);
    name_size = sizeof(SPI_CONTROLLER_NAME);
    g_spi_ctrl_mem = adi_uio_device_mem_get(SPI_CONTROLLER_NAME, name_size);
    name_size = sizeof(I2C_CONTROLLER_NAME);
    g_i2c_ctrl_mem = adi_uio_device_mem_get(I2C_CONTROLLER_NAME, name_size);
    name_size = sizeof(HSCI_CONTROLLER_NAME);
    g_hsci_ctrl_mem = adi_uio_device_mem_get(HSCI_CONTROLLER_NAME, name_size);

    if (g_fpga_reg_mem == NULL || g_spi_ctrl_mem == NULL) {
        ADI_CMS_ERROR_RETURN(API_CMS_ERROR_HW_OPEN);
    }

    /* Test access to FPGA spi. If FPGA isn't loaded then this will result in a SIGBUS signal. */
    err = probe_reg((uint32_t *)((uintptr_t)g_spi_ctrl_mem + 0x00), "SPI Master Rev");
    if (g_hsci_ctrl_mem != NULL)
        err = probe_reg((uint32_t *)((uintptr_t)g_hsci_ctrl_mem + 0x00), "HSCI Master Rev");

    /* Enable SPI and HSCI, configure spim */
    return ads10_hw_spi_hsci_en();
}

/*******************************************************************************************/

int32_t ads10_hw_close(void)
{
    int32_t err = API_CMS_ERROR_HW_CLOSE;
    char    msg[100];
    va_list argp = {0};
    struct timespec ts_now;

    clock_gettime(CLOCK_REALTIME, &ts_now);
    ts_now.tv_nsec = ts_now.tv_nsec - ts_start.tv_nsec;
    ts_now.tv_sec  = ts_now.tv_sec  - ts_start.tv_sec;
    if (ts_now.tv_sec > 0 && ts_now.tv_nsec < 0) {
        ts_now.tv_nsec += 1e9;
        ts_now.tv_sec--;
    }
    printf("%03d.%03d: ads10_hw_close(), spi_rd = %d, spi_wr = %d, total = %d \r\n", (int)ts_now.tv_sec, (int)(ts_now.tv_nsec/1e6), g_spi_rd_cnt, g_spi_wr_cnt, g_spi_rd_cnt + g_spi_wr_cnt);

    sprintf(msg, "ads10_hw_close(), spi_rd = %d, spi_wr = %d, total = %d ", g_spi_rd_cnt, g_spi_wr_cnt, g_spi_rd_cnt + g_spi_wr_cnt);
    ads10_log_write(NULL, ADI_CMS_LOG_MSG, msg, argp);

    /* close log file */
    err = ads10_log_close();

    /* Disable ADS10 SPI Master. */
    err |= ads10_axi_reg_write32(SPI_CHIP_SEL, 0x000);

    /* Unmap UIOs and release memory */
    err |= adi_uio_device_close();

    return err;
}

/*******************************************************************************************/
int32_t ads10_hw_spi_hsci_en(void)
{
    int32_t err;
    uint32_t status = 0x00;

    /* Disable the SPI Master. */
    if (err = ads10_axi_reg_write32(SPI_CHIP_SEL, 0x000), err != API_CMS_ERROR_OK)
        return err;

    /* Disable the HSCI Master. */
    if (err = ads10_axi_reg_write32(HSCI_MASTER_CTRL_OFFSET, 0x000), err != API_CMS_ERROR_OK)
        return err;

    ads10_wait_us(NULL, 1000);

    /* Enable the SPI Master. */
    if (err = ads10_axi_reg_write32(SPI_CHIP_SEL, 0x100), err != API_CMS_ERROR_OK)
        return err;

    /* Change HSCI clk to 400MHz. */
    // if (err = ads10_axi_reg_write32(HSCI_RATE_CTRL, 0x02), err != API_CMS_ERROR_OK)
    //     return err;
    // if (err = ads10_axi_reg_write32(HSCI_RATE_CTRL, 0x03), err != API_CMS_ERROR_OK)
    //     return err;

    /* Check HSCI Master PHY status. */
    if (err = ads10_axi_reg_read32(HSCI_PHY_STATUS_OFFSET, &status), err != API_CMS_ERROR_OK)
        return err;
    if (status != 0x3F) {
        printf("HSCI PHY is 0x%X, expected 0x3F\n", status);
        return API_CMS_ERROR_ERROR;
    }

    /* Enable the HSCI Master. */
    if (err = ads10_axi_reg_write32(HSCI_MASTER_CTRL_OFFSET, 0x100), err != API_CMS_ERROR_OK)
        return err;

    if (err = ads10_spim_configure(0, 0), err != API_CMS_ERROR_OK)
        return err;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/
int32_t ads10_log_write(void *user_data, int32_t log_type, const char *message, va_list argp)
{
    int32_t err = API_CMS_ERROR_LOG_WRITE;
    struct timespec ts_now;
    char log_msg[MAX_LOG_LINE_LENGTH] = {0};
    const char *log_type_str;

    if ((g_log_fd == NULL) || (g_log_file_name[0] == '\0'))
        return API_CMS_ERROR_LOG_WRITE;
    if (g_log_line_num >= MAX_LOG_NUM_LINES) {
        rewind(g_log_fd);
        g_log_line_num = 0;
    }

    log_type_str = "MESSAGE:";
    if ((log_type & ADI_CMS_LOG_WARN) > 0)
        log_type_str = "WARNING:";
    if ((log_type & ADI_CMS_LOG_ERR)  > 0)
        log_type_str = "ERROR  :";

    clock_gettime(CLOCK_REALTIME, &ts_now);
    ts_now.tv_nsec = ts_now.tv_nsec - ts_start.tv_nsec;
    ts_now.tv_sec  = ts_now.tv_sec  - ts_start.tv_sec;
    if (ts_now.tv_sec > 0 && ts_now.tv_nsec < 0) {
        ts_now.tv_nsec += 1e9;
        ts_now.tv_sec--;
    }

    err = snprintf(log_msg + strlen(log_msg), MAX_LOG_LINE_LENGTH, "%03d.%06d: %s ", (int)ts_now.tv_sec, (int)(ts_now.tv_nsec/1e3), log_type_str);
    if (err < 0)
       return API_CMS_ERROR_LOG_WRITE;

    size_t offset = strlen(log_msg);
    va_list argp_copy;
    va_copy(argp_copy, argp);
    if (vsnprintf(log_msg + offset, MAX_LOG_LINE_LENGTH, message, argp_copy) < 0) {
        va_end(argp_copy);
        return API_CMS_ERROR_LOG_WRITE;
    }
    va_end(argp_copy);

    if (fprintf(g_log_fd, "%s\n", log_msg) < 0)
        return API_CMS_ERROR_LOG_WRITE;
    if (fflush(g_log_fd) < 0)
        return API_CMS_ERROR_LOG_WRITE;
    g_log_line_num += 1;

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

int32_t ads10_wait_us(void *user_data, uint32_t time_us)
{
    if (usleep(time_us) < 0)
        return API_CMS_ERROR_DELAY_US;
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

int32_t ads10_hw_rst_pin_ctrl_apollo(void *user_data, uint8_t enable)
{
    uint32_t tmp_val = 0;
    ads10_axi_reg_read32(AXI_FPGA_MISC_1_REG, &tmp_val);
    if (enable) {
        tmp_val |= AXI_FPGA_DUT_RSTB;   // Sets
    } else {
        tmp_val &= (~AXI_FPGA_DUT_RSTB);    // Clears
    }
    ads10_axi_reg_write32(AXI_FPGA_MISC_1_REG,tmp_val);
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

int32_t ads10_user_data_free(void **user_data)
{
    free(*user_data);
    *user_data = NULL;
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

int32_t ads10_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    int32_t err = API_CMS_ERROR_NOT_SUPPORTED;

    if (g_fpga_reg_mem != NULL) {
        *out_data = *((uint32_t *)((uintptr_t)g_fpga_reg_mem + 4 * reg_offset));
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/

int32_t ads10_axi_reg_write32(uint32_t reg_offset, uint32_t data)
{
    int err = API_CMS_ERROR_NOT_SUPPORTED;;

    if (g_fpga_reg_mem != NULL) {
        *((uint32_t *)((uintptr_t)g_fpga_reg_mem  +  (4 * reg_offset))) = data;
        err = API_CMS_ERROR_OK;
    }
    return err;
}

/*******************************************************************************************/
int32_t ads10_cms_fmcb_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_cms_hal_txn_config_t *txn_config)
{
    int32_t err;
    hal_spi_config_t *spi_desc = (hal_spi_config_t *) user_data;
    hal_spi_fmcb_config_t *ls_info = (hal_spi_fmcb_config_t *)(spi_desc->user_data);
    uint8_t gpio_idx = 255;
    uint8_t i;

    if (ls_info->spi_byte_0[0] == 255) {
        gpio_idx = ls_info->target[0];
    } else {
        for (i = 0; i < HAL_SPI_FMCB_VGA_NUM; i++) {
            /*  tx_data[0] consist of address MSB, i.e. | R/W | 0 | CA2 | CA1 | CA0 | 0 | 0 | A8 |
                0xFE is a mask that represent only the R/W bit and chip addr bits, ignoring value for A8 */
            if ((tx_data[0] & 0xFE) == ls_info->spi_byte_0[i]) {
                gpio_idx = ls_info->target[i];
                break;
            }
        }
    }
    if (gpio_idx == 255) {
        return API_CMS_ERROR_SPI_SDO;
    }

    err = ls_info->sdo_en_fcn(ls_info->sdo_en_context, gpio_idx, 1);
    ADI_CMS_ERROR_RETURN(err);

    err = ads10_cms_spi_read(user_data, tx_data, rx_data, num_tx_rx_bytes, txn_config);
    ADI_CMS_ERROR_RETURN(err);

    err = ls_info->sdo_en_fcn(ls_info->sdo_en_context, gpio_idx, 0);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

/*******************************************************************************************/
int32_t ads10_cms_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_cms_hal_txn_config_t *txn_config)
{
    adi_apollo_hal_txn_config_t mapped = cms_hal_to_apollo_hal_map(txn_config);
    return ads10_spi_read(user_data, tx_data, rx_data, num_tx_rx_bytes, &mapped);
}

/*******************************************************************************************/

int32_t ads10_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    int32_t err = API_CMS_ERROR_SPI_XFER;
    int i = 0;
    g_spi_rd_cnt++;

    hal_spi_config_t *spi_desc = (hal_spi_config_t *) user_data;


    if ( strcmp(spi_desc->name, APOLLO_SPI1_NAME) == 0)
    {
        printf("SPI1 Currently not supported.\n");  // To-Do: SPI-1 Support.
        return API_CMS_ERROR_SPI_REGIO_XFER;
    }

    if ( strcmp(spi_desc->name, APOLLO_SPI0_NAME) == 0 )   // APOLLO_SPI0.
    {
        // Select ADS10 SPI Master,instance 0 for Apollo SPI0 device. ADS10 Reg x901.
        if (err = ads10_spim_periph_sel(0), err != API_CMS_ERROR_OK)
            return err;
    }

    if ( IS_SECONDARY_SPI0_DEVICE(spi_desc) )
    {
        // Select ADS10 SPI Master,instance 1 for secondary SPI0 devices. ADS10 Reg x901.
        if (err = ads10_spim_periph_sel(1), err != API_CMS_ERROR_OK)
            return err;
    }

	// Configure SPIM Reg: 0x8001.
	if (err = ads10_spim_configure(spi_desc->spi_3wire, spi_desc->spi_lsb_first), err != API_CMS_ERROR_OK)
		return err;

    uint32_t addr = 0;

    if (txn_config->addr_len == 2)// 16-bits (2-Bytes)
    {
        addr = (spi_desc->spi_lsb_first == 0)  ?    ((tx_data[0] << 8) | tx_data[1]) :  //  MSB_FIRST
                                                    ((tx_data[1] << 8) | tx_data[0]);   //  LSB_FIRST

        // Address swapped for SPIM.
        addr = bswap_16(addr);
    }

    else if ((txn_config->addr_len == 4) || (txn_config->addr_len == 8))// 32-64 bits (4-8 Bytes)
    {
        // To-Do:
        // Needed in Future.
        return API_CMS_ERROR_SPI_REGIO_XFER;
    }

    uint32_t data_len = txn_config->data_len;
    if (txn_config->is_bf_txn)
    {
        data_len = txn_config->data_len * txn_config->stream_len;
    }

    if (ads10_spim_wait_idle() == 0) {
        ads10_spi_reg_write32(SPIM_XFER_COUNT,  0x01);
        ads10_spi_reg_write32(SPIM_ADDR_COUNT,  txn_config->addr_len);
        ads10_spi_reg_write32(SPIM_DATA_COUNT,  data_len);
        ads10_spi_reg_write32(SPIM_TARGET,      0x01 << spi_desc->spi_cs);   // To-Do: Support for SPI-1 and HMC.
        ads10_spi_reg_write32(SPIM_CTRL,        0x01);   // SPI_CYCLE_DELAY[8:1] = 0 and SPI_CMD_SEL[0] = 1 (Read)

    /*  For a single and/or continuous read transaction the SPI Master needs just the address,
        the start address for the SPI Buffer will therefore be 0x8000. */
        ads10_spi_reg_write32(SPIM_BRAM_ADDR,   0x8000);    //
        ads10_spi_reg_write32(0x8000 * 4,       (addr & 0xFFFF));
        ads10_spi_reg_write32(SPIM_RUN,         1);       /* SPI Master run signal. */

        uint32_t rdData = 0;
        if (ads10_spim_wait_idle() == 0) {
            if (txn_config->data_len == 1)  // 8-bit (1-Byte)
            {
            /*  SPIM_BUF_RDDATA is 32bit wide, doing 8 bit read would replace the 1st LSB byte
                and there would still be garbage data from previous read.*/

                for (i = 0; i < data_len; i++)
                {
                    if (i%4 == 0) {
                        ads10_spi_reg_read32(SPIM_BUF_RDDATA + i, &rdData);
                    }
                    rx_data[2+i] = (rdData >> ((i%4)*8)) & 0xFF;
                }
            }
            else if (txn_config->data_len == 2) // 16-bits (2-Bytes)
            {
                ads10_spi_reg_read32(SPIM_BUF_RDDATA, &rdData);

            /*  SPIM_BUF_RDDATA is 32bit wide, doing 16 bit read would replace the 1st two LSB byte
                and there would still be garbage data from previous read.*/
                rdData = rdData & 0xFFFF;
                rdData = bswap_16(rdData);

                rx_data[2] = (rdData >> 8) & 0xFF;
                rx_data[3] = (rdData) & 0xFF;
            }
            else if (txn_config->data_len == 4) // 32-bits (4-Bytes)
            {
                ads10_spi_reg_read32(SPIM_BUF_RDDATA, &rdData);

                rdData = bswap_32(rdData);

                rx_data[2] = (rdData >> 24) & 0xFF;
                rx_data[3] = (rdData >> 16) & 0xFF;
                rx_data[4] = (rdData >> 8) & 0xFF;
                rx_data[5] = (rdData) & 0xFF;
            }
            else {
                // To-Do: 48 -64 bit support.
                // Needed in Future.
                return API_CMS_ERROR_SPI_REGIO_XFER;
            }
            if (ads10_spim_wait_idle() == 0) {
                err = API_CMS_ERROR_OK;
            }
        }
    }
    else {
        err = API_CMS_ERROR_SPI_XFER;
        printf("SPI Master busy. Transaction Failed!");
    }

    return err;
}

/*******************************************************************************************/

int32_t ads10_cms_spi_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes, adi_cms_hal_txn_config_t *txn_config)
{
    adi_apollo_hal_txn_config_t mapped = cms_hal_to_apollo_hal_map(txn_config);
    return ads10_spi_write(user_data, tx_data, num_tx_bytes, &mapped);
}

/*******************************************************************************************/

int32_t ads10_spi_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    int32_t err = API_CMS_ERROR_SPI_XFER;
    g_spi_wr_cnt++;

    hal_spi_config_t *spi_desc = (hal_spi_config_t *) user_data;


    if ( strcmp(spi_desc->name, APOLLO_SPI1_NAME) == 0)
    {
        printf("SPI1 Currently not supported.\n");  // To-Do: SPI-1 Support.
        return API_CMS_ERROR_SPI_REGIO_XFER;
    }

    if ( strcmp(spi_desc->name, APOLLO_SPI0_NAME) == 0 )   // APOLLO_SPI0.
    {
        // Select ADS10 SPI Master,instance 0 for Apollo SPI0 device. ADS10 Reg x901.
        if (err = ads10_spim_periph_sel(0), err != API_CMS_ERROR_OK)
            return err;
    }

    if ( IS_SECONDARY_SPI0_DEVICE(spi_desc) )
    {
        // Select ADS10 SPI Master,instance 0 for Apollo SPI0 device. ADS10 Reg x901.
        if (err = ads10_spim_periph_sel(1), err != API_CMS_ERROR_OK)
            return err;
    }

	// Configure SPIM Reg: 0x8001.
	if (err = ads10_spim_configure(spi_desc->spi_3wire, spi_desc->spi_lsb_first), err != API_CMS_ERROR_OK)
		return err;

    uint32_t addr = 0;

    if (txn_config->addr_len == 2)// 16-bits (2-Bytes)
    {
        addr = (spi_desc->spi_lsb_first == 0)  ?    ((tx_data[0] << 8) | tx_data[1]) :  //  MSB_FIRST
                                                    ((tx_data[1] << 8) | tx_data[0]);   //  LSB_FIRST

        // Address swapped for SPIM.
        addr = bswap_16(addr);
    }

    else if ((txn_config->addr_len == 4) || (txn_config->addr_len == 8))// 32-64 bits (4-8 Bytes)
    {
        // To-Do:
        // Needed in Future.
        return API_CMS_ERROR_SPI_REGIO_XFER;
    }


    uint32_t data = 0;

    if (txn_config->data_len == 1)  // 8-bit (1-Byte)
    {
        data = tx_data[2];
    }

    else if (txn_config->data_len == 2) // 16-bits (2-Bytes)
    {
        data = (tx_data[2] << 8) + (tx_data[3]);
        data = bswap_16(data);
    }

    else if (txn_config->data_len == 4) // 32-bits (4-Bytes)
    {
        data = (tx_data[2] << 24) + (tx_data[3] << 16) + (tx_data[4] << 8) + (tx_data[5]);
        data = bswap_32(data);
    }

    else
    {
        // To-Do: 48 -64 bit support.
        // Needed in Future.
        return API_CMS_ERROR_SPI_REGIO_XFER;
    }

    if (ads10_spim_wait_idle() == 0) {
        ads10_spi_reg_write32(SPIM_XFER_COUNT,  0x01);
        ads10_spi_reg_write32(SPIM_ADDR_COUNT,  txn_config->addr_len);
        ads10_spi_reg_write32(SPIM_DATA_COUNT,  txn_config->data_len);
        ads10_spi_reg_write32(SPIM_TARGET,      0x01 << spi_desc->spi_cs);   // To-Do: Support for SPI-1 and HMC.
        ads10_spi_reg_write32(SPIM_CTRL,        0x00);   // SPI_CYCLE_DELAY[8:1] = 0 and SPI_CMD_SEL[0] = 0 (Write)

        if (ads10_spim_wait_idle() == 0) {

            if (txn_config->data_len == 1)  // 8-bit (1-Byte)
            {
                ads10_spi_reg_write32(SPIM_BRAM_ADDR,   0x7FFF);    //
                ads10_spi_reg_write32(0x7FFF * 4,       (addr & 0xFFFF));
                ads10_spi_reg_write32(0x8000 * 4,       ((uint8_t)data & 0xFF));
                ads10_spi_reg_write32(SPIM_RUN,         1);       /* SPI Master run signal. */
            }
            else if (txn_config->data_len == 2) // 16-bits (2-Bytes)
            {
                ads10_spi_reg_write32(SPIM_BRAM_ADDR,   0x7FFF);    //
                ads10_spi_reg_write32(0x7FFF * 4,       (addr & 0xFFFF));
                ads10_spi_reg_write32(0x8000 * 4,       (data & 0xFFFF));
                ads10_spi_reg_write32(SPIM_RUN,         1);       /* SPI Master run signal. */
            }
            else if (txn_config->data_len == 4) // 32-bits (4-Bytes)
            {
                ads10_spi_reg_write32(SPIM_BRAM_ADDR,   0x7FFF);    //
                ads10_spi_reg_write32(0x7FFF * 4,       (addr & 0xFFFF));
                ads10_spi_reg_write32(0x8000 * 4,       (data));
                ads10_spi_reg_write32(SPIM_RUN,         1);       /* SPI Master run signal. */
            }
            else {
                // To-Do: 48 -64 bit support.
                // Needed in Future.
                return API_CMS_ERROR_SPI_REGIO_XFER;
            }

            if (ads10_spim_wait_idle() == 0) {
                err = API_CMS_ERROR_OK;
            }
        }
    }
    else {
        err = API_CMS_ERROR_SPI_XFER;
        printf("SPI Master busy. Transaction Failed!");
    }
    return err;
}

/*******************************************************************************************/

int32_t ads10_hsci_manual_linkup(void *user_data, uint8_t enable, uint16_t link_up_signal_bits)
{
    return ads10_hsci_reg_write32(HSCIM_LINKUP_CTRL, ((enable & 0x01) << 10) | ((link_up_signal_bits & 0x3FF)));
}

/*******************************************************************************************/

int32_t ads10_hsci_auto_linkup(void *user_data, uint8_t enable, uint8_t hscim_mosi_clk_inv, uint8_t hscim_miso_clk_inv)
{
    int32_t err = 0;
    uint32_t reg_write = ((enable & 0x01) << 11) | ((hscim_mosi_clk_inv & 0x01) << 12) | ((hscim_miso_clk_inv & 0x01) << 13);
    ads10_hsci_reg_write32(HSCIM_LINKUP_CTRL, reg_write);

    if (!enable) {
        /* Disable the HSCI Master. */
        if (err = ads10_axi_reg_write32(HSCI_MASTER_CTRL_OFFSET, 0x000), err != API_CMS_ERROR_OK)
            return err;

        ads10_wait_us(NULL, 1000);

        /* Enable the HSCI Master. */
        if (err = ads10_axi_reg_write32(HSCI_MASTER_CTRL_OFFSET, 0x100), err != API_CMS_ERROR_OK)
            return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t ads10_hsci_alink_tbl_get(void *user_data, uint16_t *hscim_alink_table)
{
    uint32_t reg_read = 0;
    if (ads10_hscim_wait_link_active() == 0) {
        // ALINK_TABLE
        ads10_hsci_reg_read32(HSCIM_LINKUP_STAT2, &reg_read);
        *hscim_alink_table = reg_read & 0xFFFF;
    } else {
        return API_CMS_ERROR_HSCI_LINK_UP;
    }
    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

int32_t ads10_hsci_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t addr = 0;
    uint32_t bram_addr = 0x1000;
    uint32_t rdData = 0;
    uint32_t i, word_size, byte_size;

    if(txn_config->addr_len !=  4)
    {
        printf("HSCI addr size not supported\n");
        return API_CMS_ERROR_ERROR;
    }
    if((txn_config->data_len * txn_config->stream_len)  >= HSCIM_BUF_RDDATA_SIZE)
    {
        printf("HSCI data size not supported\n");
        return API_CMS_ERROR_ERROR;
    }

    addr = (tx_data[3]<<24) | (tx_data[2]<<16) | (tx_data[1]<<8) | tx_data[0];

    if (ads10_hscim_wait_done() == 0) {
        //Write BRAM
        ads10_hsci_reg_write32(HSCIM_BRAM_ADDR, bram_addr);
        ads10_hsci_reg_write32(bram_addr, addr); //Fill the buffer

        ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);
        ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3); //32B Addr space supported
        ads10_hsci_reg_write32(HSCIM_BYTE_NUM, txn_config->data_len * txn_config->stream_len);  // bytes can be transferred for single transaction
        ads10_hsci_reg_write32(HSCIM_CTRL, ((0 & 0x3) << 4) | (1 & 0x3)); //HSCI SLAVE AHB TSIZE and CMD SEL

        ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction


        if (ads10_hscim_wait_done() == 0) {
        //Read BRAM
            if (txn_config->stream_len >= 2) //Stream read
            {
                word_size = (txn_config->data_len * txn_config->stream_len)/4;
                byte_size = (txn_config->data_len * txn_config->stream_len)%4;
                for (i=0; i<word_size; i++)
                {
                    ads10_hsci_reg_read32(HSCIM_BUF_RDDATA + i, &rdData);
                    rx_data[4 + (i*4)] = (rdData) & 0xFF;
                    rx_data[5 + (i*4)] = (rdData >> 8) & 0xFF;
                    rx_data[6 + (i*4)] = (rdData >> 16) & 0xFF;
                    rx_data[7 + (i*4)] = (rdData >> 24) & 0xFF;
                }
		        if (byte_size != 0)
                {
                    ads10_hsci_reg_read32(HSCIM_BUF_RDDATA + word_size, &rdData);
		        }
                for (i=0; i<byte_size; i++)
                {
                    rx_data[4 + (word_size*4) + i] = (rdData>>(8*i)) & 0xFF;
                }
                err = API_CMS_ERROR_OK;
            }
            else
            {
                if (txn_config->data_len == 1)  // 8-bit (1-Byte)
                {
                    ads10_hsci_reg_read32(HSCIM_BUF_RDDATA, &rdData);

                /*  HSCIM_BUF_RDDATA is 32bit wide, doing 8 bit read would replace the 1st LSB byte
                    and there would still be garbage data from previous read.*/
                    rdData = rdData & 0xFF;
                    rx_data[4] = rdData;
                    err = API_CMS_ERROR_OK;
                }
                else if (txn_config->data_len == 2) // 16-bits (2-Bytes)
                {
                    ads10_hsci_reg_read32(HSCIM_BUF_RDDATA, &rdData);

                /*  HSCIM_BUF_RDDATA is 32bit wide, doing 16 bit read would replace the 1st two LSB byte
                    and there would still be garbage data from previous read.*/
                    rdData = rdData & 0xFFFF;

                    rx_data[4] = (rdData >> 8) & 0xFF;
                    rx_data[5] = (rdData) & 0xFF;
                    err = API_CMS_ERROR_OK;
                }
                else if (txn_config->data_len == 4) // 32-bits (4-Bytes)
                {
                    ads10_hsci_reg_read32(HSCIM_BUF_RDDATA, &rdData);

                    rx_data[4] = (rdData >> 24) & 0xFF;
                    rx_data[5] = (rdData >> 16) & 0xFF;
                    rx_data[6] = (rdData >> 8) & 0xFF;
                    rx_data[7] = (rdData) & 0xFF;
                    err = API_CMS_ERROR_OK;
                }
                else {
                    // To-Do: 48 -64 bit support.
                    // Needed in Future.
                    return API_CMS_ERROR_HSCI_REGIO_XFER;
                }

            }
        }
        else {
            err = API_CMS_ERROR_HSCI_REGIO_XFER;
            printf("HSCI Master busy. Inner Transaction Failed!\n");
        }
    }
    else {
        err = API_CMS_ERROR_HSCI_REGIO_XFER;
        printf("HSCI Master busy. Outer Transaction Failed!\n");
    }

    return err;
}

/*******************************************************************************************/

int32_t ads10_hsci_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{

    if (txn_config->is_bf_txn == 0x81) {
        return ads10_hsci_write_rmw(user_data, tx_data, num_tx_rx_bytes, txn_config);
    }

    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t addr = 0;
    uint32_t bram_addr = 0x0001;
    uint32_t wrData = 0;
    uint32_t i, word_size, byte_size;

    if(txn_config->addr_len !=  4)
    {
        printf("HSCI addr size not supported\n");
        return API_CMS_ERROR_ERROR;
    }
    if((txn_config->data_len * txn_config->stream_len)  >=  HSCIM_BUF_WRDATA_SIZE)
    {
        printf("HSCI data size not supported\n");
        return API_CMS_ERROR_ERROR;
    }


    addr = (tx_data[3]<<24) | (tx_data[2]<<16) | (tx_data[1]<<8) | tx_data[0];

    if (ads10_hscim_wait_done() == 0) {
        //Write BRAM
        ads10_hsci_reg_write32(HSCIM_BRAM_ADDR, bram_addr);
        ads10_hsci_reg_write32(bram_addr, addr); //Fill the buffer with addr
        bram_addr = bram_addr + 1; // bram addr for data

        if (txn_config->stream_len >= 2) // Stream write
        {
            word_size = (txn_config->data_len * txn_config->stream_len)/4;
            byte_size = (txn_config->data_len * txn_config->stream_len)%4;
            for (i=0; i<word_size; i++)
            {
                wrData = (tx_data[7 + (i*4)] << 24) + (tx_data[6 + (i*4)] << 16) + (tx_data[5 + (i*4)] << 8) + (tx_data[4 + (i*4)]);
                ads10_hsci_reg_write32(bram_addr + i, wrData);
            }
            if (word_size != 0) {
                ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);
                ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3); //32B Addr space supported
                ads10_hsci_reg_write32(HSCIM_BYTE_NUM, (word_size * 4));  // bytes can be transferred for single transaction
                ads10_hsci_reg_write32(HSCIM_CTRL, ((1 & 0x3) << 4) | (0 & 0x3)); //HSCI SLAVE AHB TSIZE and CMD SEL (1 = tsize 16-bit)

                ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction

                if (ads10_hscim_wait_done() == 0) {
                    err = API_CMS_ERROR_OK;
                }
            }

            wrData = 0;
            for (i=0; i<byte_size; i++)
            {
                wrData |= (tx_data[4 + (word_size*4) + i] << (8*i));
            }
            if (byte_size != 0)
            {
                ads10_hsci_reg_write32(HSCIM_BRAM_ADDR, bram_addr);
                ads10_hsci_reg_write32(bram_addr, addr+(word_size*4)); //Fill the buffer with addr
                ads10_hsci_reg_write32(bram_addr + 1, wrData);

                ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);
                ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3); //32B Addr space supported
                ads10_hsci_reg_write32(HSCIM_BYTE_NUM, byte_size);  // bytes can be transferred for single transaction
                ads10_hsci_reg_write32(HSCIM_CTRL, ((0 & 0x3) << 4) | (0 & 0x3)); //HSCI SLAVE AHB TSIZE and CMD SEL

                ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction

                if (ads10_hscim_wait_done() == 0) {
                    err = API_CMS_ERROR_OK;
                }
            }
        }
        else
        {
            if (txn_config->data_len == 1)  // 8-bit (1-Byte)
            {
                wrData = (tx_data[4]);
                wrData = wrData & 0xFF;
                ads10_hsci_reg_write32(bram_addr, wrData);
            }
            else if (txn_config->data_len == 2) // 16-bits (2-Bytes)
            {
                wrData = (tx_data[5] << 8) + (tx_data[4]);
                wrData = wrData & 0xFFFF;
                ads10_hsci_reg_write32(bram_addr, wrData);
            }
            else if (txn_config->data_len == 4) // 32-bits (4-Bytes)
            {
                wrData = (tx_data[7] << 24) + (tx_data[6] << 16) + (tx_data[5] << 8) + (tx_data[4]);
                ads10_hsci_reg_write32(bram_addr, wrData);

            }
            else {
                // To-Do: 48 -64 bit support.
                // Needed in Future.
                return API_CMS_ERROR_HSCI_REGIO_XFER;
            }

            ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);
            ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3); //32B Addr space supported
            ads10_hsci_reg_write32(HSCIM_BYTE_NUM, txn_config->data_len);  // bytes can be transferred for single transaction
            ads10_hsci_reg_write32(HSCIM_CTRL, ((0 & 0x3) << 4) | (0 & 0x3)); //HSCI SLAVE AHB TSIZE and CMD SEL

            ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction

            if (ads10_hscim_wait_done() == 0) {
                err = API_CMS_ERROR_OK;
            }
        }
    }
    else {
        err = API_CMS_ERROR_HSCI_REGIO_XFER;
        printf("HSCI Master busy. Transaction Failed!\n");
    }

    return err;
}

static int32_t ads10_hsci_write_rmw(void *user_data, const uint8_t tx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t addr = 0;
    uint32_t bram_addr = 0x0001;
    uint32_t wrData = 0;
    uint32_t i, word_size, byte_size;

    if (txn_config->addr_len != 4) {
        printf("HSCI addr size not supported\n");
        return API_CMS_ERROR_ERROR;
    }
    if ((txn_config->data_len * txn_config->stream_len) >= HSCIM_BUF_WRDATA_SIZE) {
        printf("HSCI data size not supported\n");
        return API_CMS_ERROR_ERROR;
    }

    addr = (tx_data[3] << 24) | (tx_data[2] << 16) | (tx_data[1] << 8) | tx_data[0];

    if (ads10_hscim_wait_done() == 0) {
        //Write BRAM
        ads10_hsci_reg_write32(HSCIM_BRAM_ADDR, bram_addr);
        ads10_hsci_reg_write32(bram_addr, addr); //Fill the buffer with addr
        bram_addr = bram_addr + 1;               // bram addr for data

        if (txn_config->stream_len >= 2) // Stream write
        {
            word_size = (txn_config->data_len * txn_config->stream_len) / 4;
            byte_size = (txn_config->data_len * txn_config->stream_len) % 4;
            for (i = 0; i < 2 * word_size; i++) {
                wrData = (tx_data[7 + (i * 4)] << 24) + (tx_data[6 + (i * 4)] << 16) + (tx_data[5 + (i * 4)] << 8) + (tx_data[4 + (i * 4)]);
                ads10_hsci_reg_write32(bram_addr + i, wrData);
            }
            if (word_size != 0) {
                ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);
                ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3);                             // 32B Addr space supported
                ads10_hsci_reg_write32(HSCIM_BYTE_NUM, (2 * word_size * 4));            // bytes can be transferred for single transaction
                ads10_hsci_reg_write32(HSCIM_CTRL, ((0 & 0x3) << 4) | (2 & 0x3));       // HSCI SLAVE AHB TSIZE and CMD SEL

                ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction

                if (ads10_hscim_wait_done() == 0) {
                    err = API_CMS_ERROR_OK;
                }
            }

            wrData = 0;
            for (i = 0; i < 2 * byte_size; i++) {
                wrData |= (tx_data[4 + (2 * word_size * 4) + i] << (8 * i));
            }
            if (byte_size != 0) {
                ads10_hsci_reg_write32(HSCIM_BRAM_ADDR, bram_addr);
                ads10_hsci_reg_write32(bram_addr, addr + (2 * word_size * 4));  //Fill the buffer with addr
                ads10_hsci_reg_write32(bram_addr + 1, wrData);

                ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);
                ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3);                       // 32B Addr space supported
                ads10_hsci_reg_write32(HSCIM_BYTE_NUM, 2 * byte_size);            // bytes can be transferred for single transaction
                ads10_hsci_reg_write32(HSCIM_CTRL, ((0 & 0x3) << 4) | (2 & 0x3)); // HSCI SLAVE AHB TSIZE and CMD SEL

                ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction

                if (ads10_hscim_wait_done() == 0) {
                    err = API_CMS_ERROR_OK;
                }
            }
        } else {
            wrData = 0;
            if (txn_config->data_len == 1) // 8-bit (1-Byte)
            {
                wrData = ((tx_data[5]) << 8) | (tx_data[4]);      // mask [4], data [5]
                ads10_hsci_reg_write32(bram_addr, wrData);
            } else if (txn_config->data_len == 2) // 16-bits (2-Bytes)
            {
                wrData = (tx_data[7] << 24) + ((tx_data[6]) <<16) + (tx_data[5] << 8) + (tx_data[4]);       // mask [4], data [5], mask [6], mask [7]
                ads10_hsci_reg_write32(bram_addr, wrData);
            } else if (txn_config->data_len == 4) // 32-bits (4-Bytes)
            {
                wrData = (tx_data[7] << 24) + ((tx_data[6]) << 16) + (tx_data[5] << 8) + (tx_data[4]);      // mask [4], data [5], mask [6], mask [7]
                ads10_hsci_reg_write32(bram_addr, wrData);

                bram_addr++;

                wrData = (tx_data[11] << 24) + ((tx_data[10]) << 16) + (tx_data[9] << 8) + (tx_data[8]);    // mask [8], data [9], mask [10], mask [11]
                ads10_hsci_reg_write32(bram_addr, wrData);
            } else {
                // To-Do: 48 -64 bit support.
                // Needed in Future.
                return API_CMS_ERROR_HSCI_REGIO_XFER;
            }

            ads10_hsci_reg_write32(HSCIM_XFER_NUM, 1);                          // HSCI num of transactions
            ads10_hsci_reg_write32(HSCIM_ADDR_SIZE, 3);                         // 32B Addr space supported
            ads10_hsci_reg_write32(HSCIM_BYTE_NUM, txn_config->data_len * 2);   // bytes can be transferred for single transaction (x2 for 1 byte mask per 1 byte data)
            ads10_hsci_reg_write32(HSCIM_CTRL, ((0 & 0x3) << 4) | (2 & 0x3));   // HSCI SLAVE AHB TSIZE (0=byte) and CMD SEL (2=rmw)

            ads10_hsci_reg_write32(HSCIM_RUN, 1); //Start transaction

            if (ads10_hscim_wait_done() == 0) {
                err = API_CMS_ERROR_OK;
            }
        }
    } else {
        err = API_CMS_ERROR_HSCI_REGIO_XFER;
        printf("HSCI Master busy. Transaction Failed!\n");
    }

    return err;
}

/*******************************************************************************************/

int32_t ads10_i2c_board_name_get(uint32_t board_address, char vendor[], char board_name[], char board_rev[])
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    uint8_t common_header = 0;
    uint8_t board_manufacture_type_len = 0;
    uint8_t board_product_name_type_len = 0;
    uint8_t board_part_number_type_len = 0;
    uint8_t pcb_rev_len = 0;
    uint32_t device_addr = (board_address == 0x00) ? I2C_APOLLO_EEPROM_ADDRESS : board_address;

    hal_i2c_config_t i2c_desc = {
        .device_addr_type = SEVEN_BIT_ADDRESS,
        .reg_addr_format = EIGHT_BIT_ADDRESS,
        .mux_channel = FMC_I2C,
    };

    // Switch to selected I2C bus.
    if (err = ads10_i2c_mux_switch(&i2c_desc, i2c_desc.mux_channel), err != API_CMS_ERROR_OK)
        return err;

    if (err = ads10_i2c_read(&i2c_desc, device_addr, 0x00, &common_header, 1), err != API_CMS_ERROR_OK)
        return err;

    if (common_header != 0x01)
        return API_CMS_ERROR_ERROR;


    /* read vendor */
    if (err = ads10_i2c_read(&i2c_desc, device_addr, 0x0E, &board_manufacture_type_len, 1), err != API_CMS_ERROR_OK)
        return err;

    board_manufacture_type_len &= 0x0F;
    if (err = ads10_i2c_read(&i2c_desc, device_addr, 0x0F, (uint8_t *)vendor, board_manufacture_type_len), err != API_CMS_ERROR_OK)
        return err;
    vendor[board_manufacture_type_len] = '\0';


    /* read board name */
    if (err = ads10_i2c_read(&i2c_desc, device_addr, (0x0F + board_manufacture_type_len), &board_product_name_type_len, 1), err != API_CMS_ERROR_OK)
        return err;

    board_product_name_type_len &= 0x0F;
    if (err = ads10_i2c_read(&i2c_desc, device_addr, (0x0F + board_manufacture_type_len  + board_product_name_type_len + 2), &board_part_number_type_len, 1), err != API_CMS_ERROR_OK)
        return err;

    board_part_number_type_len &= 0x3F;
    if (err = ads10_i2c_read(&i2c_desc, device_addr, (0x0F + board_manufacture_type_len + board_product_name_type_len + 3), (uint8_t *)board_name, board_part_number_type_len), err != API_CMS_ERROR_OK)
        return err;
    board_name[board_part_number_type_len] = '\0';


    /* read board rev */
    if (err = ads10_i2c_read(&i2c_desc, device_addr, (0x0F + board_manufacture_type_len + board_product_name_type_len + board_part_number_type_len + 4), &pcb_rev_len, 1), err != API_CMS_ERROR_OK)
        return err;

    pcb_rev_len &= 0x0F;
    if (err = ads10_i2c_read(&i2c_desc, device_addr, (0x0F + board_manufacture_type_len + board_product_name_type_len + board_part_number_type_len + 5), (uint8_t *)board_rev, pcb_rev_len), err != API_CMS_ERROR_OK)
        return err;
    board_rev[0] = ' ';
    board_rev[pcb_rev_len] = '\0';

    return API_CMS_ERROR_OK;
}

/*******************************************************************************************/

int32_t ads10_i2c_mux_switch(void *user_data, uint8_t channel)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    err = ads10_i2c_write(user_data, I2C_SWITCH_ADDRESS, channel, NULL, 0);
    return err;
}

/*******************************************************************************************/

int32_t ads10_i2c_xfer(void *user_data, uint32_t device_addr, uint16_t  reg_addr, uint8_t data[], uint16_t data_num_bytes, bool is_read)
{
    int32_t err = API_CMS_ERROR_I2C_ERROR;
    hal_i2c_config_t *i2c_desc = (hal_i2c_config_t*) user_data;
    uint8_t channel = i2c_desc->mux_channel;

    // Switch to selected I2C bus.
    if (err = ads10_i2c_mux_switch(i2c_desc, channel), err != API_CMS_ERROR_OK)
        return err;

    if(is_read == true) {
        err = ads10_i2c_read(i2c_desc, device_addr, reg_addr, data, data_num_bytes);
    } else {
        err = ads10_i2c_write(i2c_desc, device_addr, reg_addr, data, data_num_bytes);
    }
    return err;
}

/*******************************************************************************************/

int32_t ads10_i2c_read(void *user_data, uint32_t device_addr, uint8_t reg_addr, uint8_t data[], uint16_t data_num_bytes)
{
    int32_t err = API_CMS_ERROR_I2C_READ;
    uint32_t fifo_data = 0;
    hal_i2c_config_t *i2c_desc = (hal_i2c_config_t*) user_data;
    uint8_t i = 0;

    if ( (device_addr != I2C_SWITCH_ADDRESS) && (device_addr != I2C_APOLLO_EEPROM_ADDRESS) ) {
        // Switch to selected I2C bus.
        if (err = ads10_i2c_mux_switch(i2c_desc, i2c_desc->mux_channel), err != API_CMS_ERROR_OK)
            return err;
    }

    // Configure transaction settings.
    if (err = ads10_i2cm_configure(i2c_desc->device_addr_type, i2c_desc->reg_addr_format, (uint8_t) READ), err != API_CMS_ERROR_OK)
        return err;

    if (ads10_i2cm_wait_idle() == 0) {
        ads10_i2c_reg_write32(I2CM_DEVICE_ADDR, device_addr);
        ads10_i2c_reg_write32(I2CM_REG_ADDR, reg_addr);
        ads10_i2c_reg_write32(I2CM_READ_LEN, data_num_bytes);
        ads10_i2c_reg_write32(I2CM_TRANS_CTRL, 1); /* start xfer */

        if (ads10_i2cm_wait_idle() == 0) {

            for (i = 0; i < data_num_bytes; ++i) {
                ads10_i2c_reg_read32(I2CM_RX_FIFO, &fifo_data);
                data[i] = fifo_data & 0xff;
            }
            err = API_CMS_ERROR_OK;
        }
        else {
            printf("I2C Read Operation Failed! Bus busy! \nI2C_M Reset!\n");
            ads10_i2cm_reset();
            return err = API_CMS_ERROR_I2C_READ;
        }
    }
    else {
        printf("I2C Bus busy! Read Operation Didn't Start! \nI2C_M Reset!\n");
        ads10_i2cm_reset();
        return err = API_CMS_ERROR_I2C_BUSY;
    }
    return err;
}

/*******************************************************************************************/

int32_t ads10_i2c_write(void *user_data, uint32_t device_addr, uint8_t reg_addr, uint8_t data[], uint16_t data_num_bytes)
{
    int32_t err = API_CMS_ERROR_I2C_WRITE;
    hal_i2c_config_t *i2c_desc = (hal_i2c_config_t*) user_data;
    uint8_t i = 0;

    if ( (device_addr != I2C_SWITCH_ADDRESS) && (device_addr != I2C_APOLLO_EEPROM_ADDRESS) ) {
        // Switch to selected I2C bus.
        if (err = ads10_i2c_mux_switch(i2c_desc, i2c_desc->mux_channel), err != API_CMS_ERROR_OK)
            return err;
    }

    // Configure transaction settings.
    if (err = ads10_i2cm_configure(i2c_desc->device_addr_type, i2c_desc->reg_addr_format, (uint8_t) WRITE), err != API_CMS_ERROR_OK)
        return err;

    if (ads10_i2cm_wait_idle() == 0) {
        ads10_i2c_reg_write32(I2CM_DEVICE_ADDR, device_addr);
        ads10_i2c_reg_write32(I2CM_REG_ADDR, reg_addr);
        ads10_i2c_reg_write32(I2CM_READ_LEN, 0);

        if (data_num_bytes == 0) {
            ads10_i2c_reg_write32(I2CM_TRANS_CTRL, 1); /* start xfer */
        }

        else {
            for (i = 0; i < data_num_bytes; ++i) {
                ads10_i2c_reg_write32(I2CM_TX_FIFO, data[i]);
            }

            ads10_i2c_reg_write32(I2CM_TRANS_CTRL, 1); /* start xfer */
        }


        if (ads10_i2cm_wait_idle() == 0) {
            err = API_CMS_ERROR_OK;
        }
        else {
            printf("I2C Write Operation Failed! Bus busy! \nI2C_M Reset!\n");
            ads10_i2cm_reset();
            return err = API_CMS_ERROR_I2C_WRITE;
        }

    }
    else {
        printf("I2C Bus busy! Write Operation Didn't Start! \nI2C_M Reset!\n");
        ads10_i2cm_reset();
        return err = API_CMS_ERROR_I2C_BUSY;
    }
    return err;
}

/*******************************************************************************************/

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

/*******************************************************************************************/

static void sigbus_handler(int sig_num)
{
    // Jump to the point setup by setjmp
    longjmp(buf, 1);
}

/*******************************************************************************************/

/* Return 0 if register probe results succeeds.
   Otherwise,-1 indicates a SIGBUS occurred and regiter address is not valid */
static int32_t probe_reg(uint32_t *reg_addr, const char *desc)
{
    uint32_t data32;

    /* Register function to be called on SIGBUS */
    signal(SIGBUS, sigbus_handler);

    // Setup jump position using buf and return 0
    if (setjmp(buf)) {
        signal(SIGBUS, SIG_DFL);
        return -1;
    }

    /* Test the address. If not response, then SIGBUS will be generated. */
    data32 = *reg_addr;

    /* Won't get here if SIGBUS detected */
    printf("%s: address: 0x%08x\r\n", desc, data32);
    signal(SIGBUS, SIG_DFL);

    return 0;
}

static int32_t fmcb_level_shifter_disable_all(void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn)
{
    uint8_t gpios[] = {SDO_OEN_1, SDO_OEN_2, SDO_OEN_3, SDO_OEN_4, SDO_OEN_5};
    int8_t i = 0;

    for (i = 0; i < 5; i++) {
        sdo_en_fcn(sdo_en_context, gpios[i], 0);
    }

    return API_CMS_ERROR_OK;
}
