/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADL6331_HAL__
 * @{
 */

#ifndef __ADL6331_HAL__
#define __ADL6331_HAL__

#include "adi_adl6331_types.h"

#define __ADL6331_CHIP_ID_COUNT                8
#define __ADL6331_SPI_MSG_SIZE                 3
#define __ADL6331_GET_ADDRESS_MSB(addr, sel)   (((addr >> 8) & 0x1) | (sel << 3))
#define __ADL6331_GET_ADDRESS_LSB(addr)        (addr & 0xFF)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Get 8-bit register value.
 *
 * \param[in]  adl6331  Context variable - Pointer to the ADL6331 device data structure
 * \param[in]  chip_id  Target chip \ref adi_adl6331_chip_id_e
 * \param[in]  reg      Register to read from
 * \param[out] data     Value of register (8-bit)
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adl6331_hal_reg_get(adi_adl6331_device_t *adl6331, adi_adl6331_chip_id_e chip_id, uint32_t reg, uint8_t *data);

/**
 * \brief  Set the 8-bit register value.
 *
 * \param[in] adl6331  Context variable - Pointer to the ADL6331 device data structure
 * \param[in] chip_id  Target chip \ref adi_adl6331_chip_id_e
 * \param[in] reg      Register to write to
 * \param[in] data     Value to write to register
 *
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_adl6331_hal_reg_set(adi_adl6331_device_t *adl6331, adi_adl6331_chip_id_e chip_id, uint32_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // !__ADL6331_HAL__
