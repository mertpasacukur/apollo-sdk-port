/*!
 * @brief     UIOs platform configuration header file.
 *
 * @copyright copyright(c) 2052 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS10__
 * @{
 */

#ifndef __HAL_UIO__
#define __HAL_UIO__

/*============= I N C L U D E S ============*/



/*============= D E F I N E S ==============*/
#define FPGA_REGISTERS_NAME         "uio_fpga_registers"
#define FPGA_SCRATCH_MEM_NAME       "uio_fpga_scratch_mem"
#define SPI_CONTROLLER_NAME         "uio_spi_controller"
#define I2C_CONTROLLER_NAME         "uio_i2c_controller"
#define HSCI_CONTROLLER_NAME         "uio_hsci_controller"


/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Discovers UIO devices present and accessible.
 *          Maps discovered UIOs (e.g. /dev/uioX ) to virtual address space.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t adi_uio_devices_init(void);


/**
 * @brief   Removes the UIO mappings
 *          Releases the memory.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t adi_uio_device_close(void);


/**
 * @brief   Get the starting address for the mapping of a UIO device.
 *
 * param[in]    name        Name of the UIO device the starting address we want
 *                          to get for.
 * param[in]    length      Length of the "name" string.
 *
 * @return      Non NULL address    The specified device exists and successfully
 *                                  mapped.
 * @return      NULL                The device doesn't exist or couldn't be
 *                                  mapped.
 */
void *adi_uio_device_mem_get(const char *name, uint32_t length);


/**
 * @brief   Get the UIO number (e.g. /dev/uioX) of a particular UIO device
 *
 * param[in]    name    Name of the UIO device to get the UIO number for.
 * param[out]   uio_no  The UIO device number corresponding to the \p name
 *
 * @return      API_CMS_ERROR_OK    The API completed successfully and the UIO
 *                                  device number has been returned in \p uio_no.
 * @retun       API_CMS_ERROR_ERROR  Failed to find the \p name UIO.
 * @return      <(-1)      The API failed. @ref adi_cms_error_e for details.
 */
int32_t adi_uio_device_get_no(const char *name, uint32_t *uio_no);


/**
 * @brief   Prints the list of UIO devices discovered in the system.
 *          Helper/Debug API.
 *
 * @return API_CMS_ERROR_OK     API Completed Successfully.
 * @return <0                   The pointer to the list of devices is NULL.
 */
int32_t adi_uio_devices_print(void);

#ifdef __cplusplus
}
#endif
#endif /*__HAL_UIO__*/
