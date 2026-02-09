/*!
 * \brief     Header file declaring APIs for ADF4030's basic core functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_CORE
 * @{
 */

#ifndef __ADI_ADF4030_CORE_H__
#define __ADI_ADF4030_CORE_H__

/*============= I N C L U D E S ============*/
#include "adi_adf4030_core_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief   Get version of the API.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[out]  apiVersion              Pointer to a struct with API version info. \ref adi_adf4030_core_version_t.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_api_ver_get(adi_adf4030_device_t *adf4030, adi_adf4030_core_version_t *apiVersion);


/**
 * \brief   Performs a soft-reset to ADF4030.
 *          Equivalent to power cycling the device in terms of the register map.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_soft_reset(adi_adf4030_device_t *adf4030);


/**
 * \brief   Initialize ADF4030 Default Registers.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_init(adi_adf4030_device_t *adf4030);


/**
 * \brief   Power up sequence of ADF4030.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_power_up(adi_adf4030_device_t *adf4030);


/**
 * \brief   System level delay / sleep in microseconds.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   delay_us                Wait / sleep time in microseconds.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_wait_us(adi_adf4030_device_t *adf4030, uint32_t delay_us);


/**
 * \brief   Perform SPI register write to device.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   address                 SPI address to which the value of data parameter shall be written.
 * \param[in]   data                    8-bit value to be written to SPI register defined by the address parameter.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_spi_reg_set(adi_adf4030_device_t *adf4030, uint16_t address, uint8_t data);


/**
 * \brief   Perform SPI register read from device.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[in]   address                 SPI address from which the value of data parameter shall be read.
 * \param[out]  data                    Pointer to an 8-bit variable to which the value of the
 *                                      SPI register at the address defined by address parameter shall be stored.
 *
 * \return  API_CMS_ERROR_OK                     API Completed Successfully.
 * \return  <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_spi_reg_get(adi_adf4030_device_t *adf4030, uint16_t address, uint8_t *data);

/**
 * \brief   Verify SPI read and write transaction to the scratchpad register.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_spi_reg_test(adi_adf4030_device_t *adf4030);

/**
 * \brief   Measure die temperature in Celsius.
 *
 * \param[in]   adf4030                 [context variable] Reference to the device.
 * \param[out]  die_temp                Measured die temperature, is expressed in °C.
 *
 * \return      API_CMS_ERROR_OK    API Completed Successfully.
 * \return      < 0                 Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adf4030_core_die_temp_get(adi_adf4030_device_t *adf4030, uint16_t *die_temp);

#ifdef __cplusplus
}
#endif

#endif // !__ADI_ADF4030_CORE_H__

/*! @} */
