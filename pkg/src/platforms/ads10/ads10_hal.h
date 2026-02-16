#if !defined(VERSAL_PLATFORM)
/*!
 * @brief     ADS10 platform configuration header file.
 *
 * @copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS10__
 * @{
 */

#ifndef __ADS10_HAL__
#define __ADS10_HAL__

/*============= I N C L U D E S ============*/

#include "adi_apollo_types.h"
#include "adi_adf4030_types.h"
#include "adi_adf4382_types.h"
#include "adi_adl6331_types.h"
#include "adi_adl6332_types.h"
#include "adi_hmc7044_types.h"
#include "adi_ltc2977_types.h"
#include "adi_ltm4681_types.h"
#include "adi_ltc2980_types.h"

/*============= D E F I N E S ==============*/

// Device address of I2C MUX.
#define I2C_SWITCH_ADDRESS     			0x70		/*!< The PCA9545APW I2C switch address. The I2C Master uses 7 bit device addressing. */
#define I2C_APOLLO_EEPROM_ADDRESS     	0x50		/*!< The M24C02-FDW6TP I2C eeprom address. The I2C Master uses 7 bit device addressing. */
#define LTC2977_I2C_ADDR                0x5C
#define LTM4681_01_I2C_ADDR				0x4F     	/*!< PolyPhased. */
#define LTM4681_23_I2C_ADDR				0x4E     	/*!< SinglePhased. */
#define LTC2980_A_I2C_ADDR				0x5C		/*!< LTC2980 sub-device A I2C address. */
#define LTC2980_B_I2C_ADDR				0x5D		/*!< LTC2980 sub-device B I2C address. */
#define LTC2980_C_I2C_ADDR				0x5F		/*!< LTC2980 sub-device C I2C address. */

#define HAL_SPI_FMCB_VGA_NUM 4

/* FMCB SPI LEVEL SHIFTER GPIOS */
#define SDO_OEN_1 35	// ADL6331_CHIP_ID_0 and ADL6331_CHIP_ID_1 Level Shifter Enable GPIO
#define SDO_OEN_2 36	// ADL6331_CHIP_ID_2 and ADL6331_CHIP_ID_3 Level Shifter Enable GPIO
#define SDO_OEN_3 37	// ADL6332_CHIP_ID_4 and ADL6332_CHIP_ID_5 Level Shifter Enable GPIO
#define SDO_OEN_4 38	// ADL6332_CHIP_ID_6 and ADL6332_CHIP_ID_7 Level Shifter Enable GPIO
#define SDO_OEN_5 39	// ADF4382 Level Shifter Enable GPIO

/*============= E N U M S ==============*/

/*!
 * @brief  Enumerate I2C Buses output from the PCA9545APW mux.
 */
typedef enum {
    PMU_I2C     =  0x01,			/*!< The PMU I2C bus output on the I2C switch. */
    CLOCKS_I2C  =  0x02,			/*!< the CLOCK I2C bus output on the I2C switch. */
    FMC_I2C	    =  0x04,			/*!< The FMC I2C bus output on the I2C switch. */
    OTHER_I2C   =  0x08			    /*!< The last I2C bus output on the I2C switch. */
} ads10_i2c_mux_switch_e;

/*!
 * @brief  Enumerate I2C Device Address Type setting for I2C Master.
 */
typedef enum {
    SEVEN_BIT_ADDRESS	=	0,		/*!< 7-bit device address. */
	TEN_BIT_ADDRESS		=	1		/*!< 10-bit device address. */
} ads10_i2c_dev_addr_type_e;

/*!
 * @brief  Enumerate I2C Register Address Format setting for I2C Master.
 */
typedef enum {
    ZERO_BIT_ADDRESS		 = 	 0,		/*!< No register address. */
	EIGHT_BIT_ADDRESS		 = 	 0,		/*!< One byte register address. */
	SIXTEEN_BIT_ADDRESS		 = 	 1,		/*!< Two byte register address. */
	TWENTYFOUR_BIT_ADDRESS	 = 	 2,		/*!< Three byte register address. */
	THIRTYTWO_BIT_ADDRESS	 = 	 3		/*!< Four byte register address. */
} ads10_i2c_reg_addr_fmt_e;

/*!
 * @brief  Enumerate I2C Device Transaction Type setting for I2C Master.
 */
typedef enum {
    WRITE	=	0,		/*!< Write Operation. */
	READ	=	1		/*!< Read Operation. */
} ads10_i2c_xaction_type_e;


/*============= S T R U C T S ==============*/
/**
 * @brief   Struct to store SPI settings used for configuring ADS10 SPI Master.
 *
 */
typedef struct {
	void      *user_data;
	char 	  *name;
	uint8_t   spi_cs;
    uint8_t   spi_3wire;		/*!< spi_3wire = 0 (4 wire) and spi_3wire = 1 (3 wire). */
    uint8_t   spi_lsb_first;	/*!< spi_lsb_first = 0 (MSB_FIRST) and spi_lsb_first = 1 (LSB_FIRST). */
} hal_spi_config_t;

typedef int32_t (*hal_spi_sdo_en)(void* user_data, uint32_t target, uint8_t enable);

typedef struct {
	void *sdo_en_context;                           // user data for sdo_en_fcn (FPGA context)
	hal_spi_sdo_en sdo_en_fcn;                      // Function pointer to toggle the GPIO
	uint32_t target[HAL_SPI_FMCB_VGA_NUM];          // GPIO index for each VGA pair / Register address
	uint8_t spi_byte_0[HAL_SPI_FMCB_VGA_NUM];       // Byte address for GPIOs. Set to 255 to use just the first target
} hal_spi_fmcb_config_t;


/**
 * @brief 	Struct to store HSCI settings used for configuring ADS10 HSCI Master.
 * 			TO_DO: NEEDS TO BE IMPLEMENTTED.
 *
 */
typedef struct {
	void        *user_data;
} hal_hsci_config_t;


/**
 * @brief 	Struct to store APOLLO HAL settings used for configuring ADS10 peripheral controllers.
 *
 */
typedef struct {
	void                *user_data;
	hal_spi_config_t    *spi0;		/*!< Pointer to the struct that stores APOLLO SPI0 HAL configuration setting. */
	hal_spi_config_t    *spi1;		/*!< Pointer to the struct that stores APOLLO SPI1 HAL configuration setting. */
	hal_hsci_config_t   *hsci;		/*!< Pointer to the struct that stores APOLLO HSCI HAL configuration setting. */
} adi_fpga_apollo_hal_config_t;


typedef struct {
	uint8_t 	mux_channel;
	uint8_t		device_addr_type;
	uint8_t 	reg_addr_format;
} hal_i2c_config_t;



/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Allocates memory for the platform HAL data structure.
 *
 * @return  Pointer to @ref adi_fpga_apollo_hal_config_t struct.
 */
adi_fpga_apollo_hal_config_t *ads10_apollo_hal_instance(void);


/**
 * @brief       Sets configuration values for ADS10 HAL peripheral masters for Apollo.
 *              E.g. For SPI peripheral (ADS10 SPI Master), settings like chip_select, MSB or LSB first and 3 or 4 wire SPI mode.
 * 				TO_DO: HSCI Implementation.
 * @param[in]   device    Pointer to the APOLLO device data structure
 * @param[in]   spi0_cs   Chip_select number for Apollo SPI 0 device. ADS10 SPI Master Instance will be 0 (Reg 0x901).
 * @param[in]   spi1_cs   Chip_select number for Apollo SPI 1 device. ADS10 SPI Master Instance will be 0 (Reg 0x901).
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
uint32_t ads10_apollo_hal_config_data(adi_apollo_device_t *device, uint8_t spi0_cs, uint8_t spi1_cs);

/**
 * @brief       Initializes SPI configuration for ADF4382.
 *
 * @param[in,out]   config    SPI Config
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_hmc7044_hal_spi_config_init(hal_spi_config_t *config);

/**
 * @brief       Sets configuration values for ADS10 SPI HAL peripheral master for HMC7044.
 *
 * @param[in]   device             Pointer to the HMC7044 device data structure.
 * @param[in]   spi_config_init    Handler for initializing the SPI configuration.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hmc7044_hal_config_data(adi_hmc7044_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

/**
 * @brief       Initializes SPI configuration for ADF4382.
 *
 * @param[in,out]   config    SPI Config
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adf4382_hal_spi_config_init(hal_spi_config_t *config);

/**
 * @brief       Sets configuration values for ADS10 SPI HAL peripheral master for ADF4382.
 *
 * @param[in]   device             Pointer to the ADF4382 device data structure.
 * @param[in]   spi_config_init    Handler for initializing the SPI configuration.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adf4382_hal_config_data(adi_adf4382_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

/**
 * @brief       Initializes SPI configuration for ADF4030.
 *
 * @param[in,out]   config    SPI Config
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adf4030_hal_spi_config_init(hal_spi_config_t *config);

/**
 * @brief       Sets configuration values for ADS10 SPI HAL peripheral master for ADF4030.
 *
 * @param[in]   device             Pointer to the ADF4030 device data structure.
 * @param[in]   spi_config_init    Handler for initializing the SPI configuration.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adf4030_hal_config_data(adi_adf4030_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

/**
 * @brief       Initializes SPI configuration for ADL6331.
 *
 * @param[in]   config    SPI Config
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adl6331_hal_spi_config_init(hal_spi_config_t *config);

/**
 * @brief       Sets configuration values for ADS10 SPI HAL peripheral master for ADL6331.
 *
 * @param[in]   device             Pointer to the ADL6331 device data structure.
 * @param[in]   spi_config_init    Handler for initializing the SPI configuration.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adl6331_hal_config_data(adi_adl6331_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

/**
 * @brief       Initializes SPI configuration for ADL6332.
 *
 * @param[in]   config    SPI Config
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adl6332_hal_spi_config_init(hal_spi_config_t *config);

/**
 * @brief       Sets configuration values for ADS10 SPI HAL peripheral master for ADL6332.
 *
 * @param[in]   device             Pointer to the ADL6332 device data structure.
 * @param[in]   spi_config_init    Handler for initializing the SPI configuration.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_adl6332_hal_config_data(adi_adl6332_device_t *device, int32_t(*spi_config_init)(hal_spi_config_t*));

/**
 * @brief       Sets configuration values for ADS10 HAL for LTC2977.
 *
 * @param[in]   device             Pointer to the LTC2977 device data structure.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_ltc2977_hal_config_data(adi_ltc2977_device_t * device);

/**
 * @brief       Sets configuration values for ADS10 HAL for LTM4681.
 *
 * @param[in]   device             Pointer to the LTM4681 device data structure.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_ltm4681_hal_config_data(adi_ltm4681_device_t * device);

/**
 * @brief       Sets configuration values for ADS10 HAL for LTC2980.
 *
 * @param[in]   device             Pointer to the LTC2980 device data structure.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_ltc2980_hal_config_data(adi_ltc2980_device_t * device);

/**
 * @brief       Sets configuration values for ADS10 I2C HAL peripheral master for the SMBus protcol.
 *
 * @param[in]   smbus               Pointer to the SMBus protocol HAL data structure.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_smbus_hal_config_data(adi_smbus_hal_t *smbus);

/**
 * @brief       Initializes ADS10 Platform.
 *              Starts the logging functionality.
 *              Detects active chip2chip bridge between ADS10 and microzed.
 *              Checks if all the required UIO devices on the linux are present and accessible.
 *              Maps ADS10 register memory into a virtual address space.
 *              Enables ADS10 SPI Master and configures it as per DUT SPI configuration.
 *
 * @param[in]   log_file    Name for the log file to be created to record events during API execution.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hw_open(const char *log_file);

/**
 * @brief       Enables ADS10 SPI Master, HSCI and configures it as per DUT SPI configuration
 *
 * @note        Should be called after re-loading an FPGA image (i.e. adi_fpga_image_loader_configure())
 *              to re-enable SPI and HSCI.
 * 
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hw_spi_hsci_en(void);

/**
 * @brief       Deinitialize ADS10 Platform.
 *              Stop logging functionality.
 *              Disables ADS10 SPI Master.
 *              Deallocates mapped memory for UIO devices which maps ADS10 register memory.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hw_close(void);

/**
 * @brief       Logging function to record events during API execution.
 *
 * @param[in]   user_data   Pointer to platform specific data.
 * @param[in]   log_type    Indicates type of logged message. @ref adi_cms_log_type_e.
 * @param[in]   message     String format message to be logged.
 * @param[in]   argp        Variable arguments to be passed in the above logged message.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_log_write(void *user_data, int32_t log_type, const char *message, va_list argp);

/**
 * @brief       Platform dependent delay function for specified number of microseconds.
 *
 * @param[in]   user_data   Pointer to platform specific data.
 * @param[in]   time_us     Time to delay/sleep in microseconds.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_wait_us(void *user_data, uint32_t time_us);

/**
 * @brief       Platform dependent reset pin control function
 *
 * @param[in]   user_data   Pointer to platform specific data.
 * @param[in]   enable      Indicates the desired enable/disable reset via the ADI device RESETB pin
 *                          0: indicates RESETB pin is set HIGH.
 *                          1: indicates RESETB pin is set LOW.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hw_rst_pin_ctrl_apollo(void *user_data, uint8_t enable);

/**
 * @brief       Deallocates the memory previously allocated to any device instance.
 *
 * @param[in]   user_data   Pointer to platform specific data whose memory resources needs to be released.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_user_data_free(void **user_data);

/**
 * @brief       Function to read register value from ADS10 Register Map.
 *              Reads FPGA Register Memory from base address 0x20000000 + reg_offset.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[out]  out_data    Read back value of register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data);

/**
 * @brief       Function to write register value to ADS10 Register Map.
 *              Writes FPGA Register Memory at base address 0x20000000 + reg_offset.
 *
 * @param[in]   reg_offset  Offset value of a register address.
 * @param[in]   data        Value to be written to the register located at the offset address.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t ads10_axi_reg_write32(uint32_t reg_offset, uint32_t data);

/**
 * @brief 	Platform dependent SPI hal read function.
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in] 	tx_data 		Array of data to send to SPI Master. Contains Reg Addr for SPI Read.
 * @param[out] 	rx_data 		Array to store received data from SPI Master.
 * @param[in] 	num_tx_rx_bytes Number of bytes for both tx_data and rx_data arrays.
 * @param[in] 	txn_config 		Pointer to structure describing transaction @ref adi_apollo_hal_txn_config_t.

 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config);
int32_t ads10_cms_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_cms_hal_txn_config_t *txn_config);
int32_t ads10_cms_fmcb_spi_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_cms_hal_txn_config_t *txn_config);

/**
 * @brief 	Platform dependent SPI hal write function.
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in] 	tx_data 		Array of data to send to SPI Master. Contains Reg Addr and Data for SPI Write.
 * @param[in] 	num_tx_bytes 	Number of bytes for both tx_data array.
 * @param[in] 	txn_config 		Pointer to structure describing transaction @ref adi_apollo_hal_txn_config_t.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_spi_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes, adi_apollo_hal_txn_config_t *txn_config);
int32_t ads10_cms_spi_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes, adi_cms_hal_txn_config_t *txn_config);

/**
 * @brief   Platform dependent HSCI manual linkup configuration function
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in]   enable       			Enable manual linkup.
 * @param[in]   link_up_signal_bits   	Clock signal used to achieve linkup.
 *
 * @return      API_CMS_ERROR_OK    	API Completed Successfully.
 * @return      <0                  	Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hsci_manual_linkup(void *user_data, uint8_t enable, uint16_t link_up_signal_bits);

/**
 * @brief   Platform dependent HSCI auto linkup configuration function
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in]   enable       		Enable auto linkup
 * @param[in]   hscim_mosi_clk_inv  Set 1, inverts HSCIM's mosi_clk output clk to Apollo
 * @param[in]   hscim_miso_clk_inv  Set 1, inverts HSCIM's miso_clk input clk from Apollo
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hsci_auto_linkup(void *user_data, uint8_t enable, uint8_t hscim_mosi_clk_inv, uint8_t hscim_miso_clk_inv);

/**
 * @brief   Platform dependent HSCI function to get auto link_table
 * @note	Retrive HSCIM ALINK_TABLE after Apollo's link_active is set
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[out]  hscim_alink_table	Pointer to a variable that stores link table value from HSCI_M
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hsci_alink_tbl_get(void *user_data, uint16_t *hscim_alink_table);

/**
 * @brief   Platform dependent HSCI hal read function.
 *
 * @param[in]   user_data       Pointer to platform specific data.
 * @param[in]   tx_data         Array of data to send to SPI Master. Contains Reg Addr for SPI Read.
 * @param[out]  rx_data         Array to store received data from SPI Master.
 * @param[in]   num_tx_rx_bytes Number of bytes for both tx_data and rx_data arrays.
 * @param[in]   txn_config      Pointer to structure describing transaction @ref adi_apollo_hal_txn_config_t.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hsci_read(void *user_data, const uint8_t tx_data[], uint8_t rx_data[], uint32_t num_tx_rx_bytes, adi_apollo_hal_txn_config_t *txn_config);

/**
 * @brief 	Platform dependent HSCI hal write function.
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in] 	tx_data 		Array of data to send to SPI Master. Contains Reg Addr and Data for SPI Write.
 * @param[in] 	num_tx_bytes 	Number of bytes for both tx_data array.
 * @param[in] 	txn_config 		Pointer to structure describing transaction @ref adi_apollo_hal_txn_config_t.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_hsci_write(void *user_data, const uint8_t tx_data[], uint32_t num_tx_bytes, adi_apollo_hal_txn_config_t *txn_config);

/**
 * @brief       Function to read back information about evaluation board connected to the ADS10 platform.
 *
 * @param[in] 	board_address 		I2C Device address. If 0, defaults to 0x50.
 * @param[out]  vendor      		Reads back Vendor name stored in EEPROM.
 * @param[out]  board_name  		Reads back Eval board's name stored in EEPROM.
 * @param[out]  board_rev   		Reads back Eval board's revision number stored in EEPROM.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_i2c_board_name_get(uint32_t board_address, char vendor[], char board_name[], char board_rev[]);

/**
 * @brief 	I2C mux switch to select between i2c buses.
 *
 * @param[in] 	user_data	Pointer to platform specific data.
 * @param[in] 	channel 	Selected I2C bus output on the I2C switch. @ref ads10_i2c_mux_switch_e.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_i2c_mux_switch(void *user_data, uint8_t channel);

/**
 * @brief       Perform I2C read / write transaction.
 *
 * @param[in] 		user_data 			Pointer to platform specific data.
 * @param[in]       device_addr         I2C Device address.
 * @param[in]       reg_addr            Register address / Command for the device.
 * @param[in,out]   data                Array of 8-bit data to send to or receive from I2C Master.
 * @param[in]       data_num_bytes     	Size of data used in read / write transaction in bytes.
 * @param[in]       is_read             Indicates if the I2C transaction is read(TRUE) or write(FALSE).
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_i2c_xfer(void *user_data, uint32_t device_addr, uint16_t  reg_addr, uint8_t data[], uint16_t data_num_bytes, bool is_read);

/**
 * @brief       I2C Read Function.
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in]   device_addr  	I2C Device address.
 * @param[in]   reg_addr    	I2C Device Register to be read.
 * @param[out]  data        	Read back register value stored in an array of 8-bit.
 * @param[in]   data_num_bytes  Number of bytes to read.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_i2c_read(void *user_data, uint32_t device_addr, uint8_t reg_addr, uint8_t data[], uint16_t data_num_bytes);

/**
 * @brief       I2C Write Function.
 *
 * @param[in] 	user_data 		Pointer to platform specific data.
 * @param[in]   device_addr  	I2C Device address.
 * @param[in]   reg_addr    	I2C Device Register to write.
 * @param[in]  	data        	Data to be written to device's register stored in an array of 8-bit.
 * @param[in]   data_num_bytes  Number of bytes to write.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t ads10_i2c_write(void *user_data, uint32_t device_addr, uint8_t reg_addr, uint8_t data[], uint16_t data_num_bytes);

#ifdef __cplusplus
}
#endif
#endif /*__ADS10_HAL__*/

#endif /* !defined(VERSAL_PLATFORM) */
