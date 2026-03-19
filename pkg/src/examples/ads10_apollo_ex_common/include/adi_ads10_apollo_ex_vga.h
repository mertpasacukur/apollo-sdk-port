/*!
 * \brief     ADS10 Apollo FMCB Variable Gain Amplifiers config functions
 *
 *            FMCB boards require level shifters to be enabled for SPI reads on SPI2 (Artemis and Yellowstone/Sequoia)
 *            Level Shifters are enabled by SDO_OEN_1 - SDO_OEN_5 (note that enables are inverted), connected to FPGA GPIOs
 *            The hal_spi_fmcb_config_t is stored as part of the spi struct, containing the relevant info to toggle the
 *            appropriate GPIOs before and after a spi read. SPI must be set for 4-wire mode on the ADS10 and the device.
 *
 *            The VGAs are accessed using the same SPI hal, so the appropriate level shifter must be determined from the
 *            first byte of the transaction - the VGA address. The VGA addresses and corresponding GPIOs are stored in
 *            the spi_byte_0 and target struct members. For Artemis, only one level shifter is needed, so set
 *            spi_byte_0[0] to 255 and use just target[0].
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_adl6331_core.h"
#include "adi_adl6331_sigpath.h"
#include "adi_adl6332_core.h"
#include "adi_adl6332_sigpath.h"
#include "ads10_hal.h"

#ifndef __ADI_ADS10_APOLLO_EX_COMMON_VGA_H__
#define __ADI_ADS10_APOLLO_EX_COMMON_VGA_H__

/**
 * @brief Tx-VGA, ADL6331, Startup Config Struct.
 */

#define FMCB_REV_B  0
#define FMCB_REV_C  1

typedef struct {
    adi_adl6331_amp_select_e amp;
    adi_adl6331_spi_init_t spi_init;
    adi_adl6331_init_t core_init;
    adi_adl6331_trim_config_t amp_trim_config;
} adi_apollo_ex_adl6331_startup_t;

/**
 * @brief Rx-VGA, ADL6332, Startup Config Struct.
 */
typedef struct {
    adi_adl6332_amp_select_e amp;
    adi_adl6332_spi_init_t spi_init;
    adi_adl6332_init_t core_init;
    adi_adl6332_trim_config_t amp_trim_config;
} adi_apollo_ex_adl6332_startup_t;

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Top-level function that initialize device structs for ADL6331/2. The scope of these structs are limited to this function only.
 *          The function also configure the VGAs for startup with preset settings.
 *          FPGA GPIOs are used for selecting pre-programmed RF gain configurations.
 *
 * \param[in]   fpga_device             Context variable - Pointer to the FPGA device data structure
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_tx_rx_vga_initialize(adi_fpga_apollo_device_t *fpga_device);

/**
 * \brief   Initializes ADL6331 device HAL and for FMCB sets up means for external GPIO control for enabling level shifters.
 *
 * \param[in]   adl6331                 Context variable - Pointer to the ADL6331 device data structure
 * \param[in]   sdo_en_context          User device context needed for external GPIO control by sdo_en_fcn function
 * \param[in]   sdo_en_fcn              Ptr to Function that can set external GPIO as Output and it's state
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adl6331_hal_config(adi_adl6331_device_t *adl6331, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

/**
 * \brief   Initializes ADL6332 device HAL and for FMCB sets up means for external GPIO control for enabling level shifters.
 *
 * \param[in]   adl6332                 Context variable - Pointer to the ADL6332 device data structure
 * \param[in]   sdo_en_context          User device context needed for external GPIO control by sdo_en_fcn function
 * \param[in]   sdo_en_fcn              Ptr to Function that can set external GPIO as Output and it's state
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adl6332_hal_config(adi_adl6332_device_t *adl6332, void *sdo_en_context, hal_spi_sdo_en sdo_en_fcn);

/**
 * \brief   Configure Tx(ADL6331) and Rx(ADl6332) VGAs with preset startup settings.
 *
 * \param[in]   adl6331                 Context variable - Pointer to the ADL6331 device data structure
 * \param[in]   adl6332                 Context variable - Pointer to the ADL6332 device data structure
 * \param[in]   tx_vga_ids              Array of Target ADL6331 chip \ref adi_adl6331_chip_id_e
 * \param[in]   rx_vga_ids              Array of Target ADL6332 chip \ref adi_adl6332_chip_id_e
 * \param[in]   num_vgas                Pointer to total numbers of chip_id in above arrays
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_tx_rx_vga_startup(adi_adl6331_device_t *adl6331,
                                              adi_adl6332_device_t *adl6332,
                                              adi_adl6331_chip_id_e tx_vga_ids[],
                                              adi_adl6332_chip_id_e rx_vga_ids[],
                                              uint8_t *num_vgas);

/**
 * \brief   Performs startup initialization for ADL6331, Tx-VGA
 *
 * \param[in]   adl6331                 Context variable - Pointer to the ADL6331 device data structure
 * \param[in]   chip_id                 Target chip \ref adi_adl6331_chip_id_e
 * \param[in]   dev_startup             Pointer to device startup struct to initialize ADL6331
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adl6331_startup(adi_adl6331_device_t *adl6331,
                                            adi_adl6331_chip_id_e chip_id,
                                            adi_apollo_ex_adl6331_startup_t *dev_startup);

/**
 * \brief   Performs startup initialization for ADL6332, Rx-VGA
 *
 * \param[in]   adl6332                 Context variable - Pointer to the ADL6332 device data structure
 * \param[in]   chip_id                 Target chip \ref adi_adl6332_chip_id_e
 * \param[in]   dev_startup             Pointer to device startup struct to initialize ADL6332
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_ads10_apollo_ex_adl6332_startup(adi_adl6332_device_t *adl6332,
                                            adi_adl6332_chip_id_e chip_id,
                                            adi_apollo_ex_adl6332_startup_t *dev_startup);


#ifdef __cplusplus
}
#endif

#endif /* __ADI_ADS10_APOLLO_EX_COMMON_VGA_H__ */
