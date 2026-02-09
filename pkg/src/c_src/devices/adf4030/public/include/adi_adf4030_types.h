/*!
 * \brief     ADF4030 Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030
 * @{
 */

#ifndef __ADI_ADF4030_TYPES_H__
#define __ADI_ADF4030_TYPES_H__

/*============= I N C L U D E S ============*/
#ifdef __KERNEL__
    #include <linux/kernel.h>
#else
    #include <stdint.h>
#endif

#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/
#define ADI_ADF4030_PTR_CHECK(ptr)                  if ((ptr) == ADI_INVALID_POINTER) return API_CMS_ERROR_INVALID_HANDLE_PTR
#define ADI_ADF4030_NULL_CHECK(val)                 if ((val) == NULL) return API_CMS_ERROR_NULL_PARAM
#define ADI_ADF4030_CHECK_ERR_OK(err)               if ((err) != API_CMS_ERROR_OK) return (err)
#define ADI_ADF4030_VALIDATE_RANGE(value, min, max) if (value < min || value > max) return API_CMS_ERROR_INVALID_PARAM
#define ADI_ADF4030_CHECK_MASK(value, mask)         ((value & mask) == mask)

#define ADI_ADF4030_REF_FREQ_MIN        	10000000U	// 10MHz
#define ADI_ADF4030_REF_FREQ_MAX      		250000000U  // 250Mhz
#define ADI_ADF4030_PFD_FREQ_MIN        	10000000U   // 10MHz
#define ADI_ADF4030_PFD_FREQ_MAX      		20000000U   // 20Mhz
#define ADI_ADF4030_VCO_FREQ_TYP        	2500000000U	// 2500MHz
#define ADI_ADF4030_VCO_FREQ_MIN        	2375000000U	// 2375MHz
#define ADI_ADF4030_VCO_FREQ_MAX      		2625000000U	// 2625Mhz
#define ADI_ADF4030_BSYNC_FREQ_MIN        	650000U   	// 650kHz
#define ADI_ADF4030_BSYNC_FREQ_MAX      	200000000U	// 200Mhz
#define ADI_ADF4030_R_DIV_MIN 				1U
#define ADI_ADF4030_R_DIV_MAX 				31U
#define ADI_ADF4030_N_DIV_MIN 				8U
#define ADI_ADF4030_N_DIV_MAX 				255U
#define ADI_ADF4030_O_DIV_MIN 				10U
#define ADI_ADF4030_O_DIV_MAX 				4095U

/**
 * \enum    adi_adf4030_dev_id_e
 * \brief   ID of Devices supported by the driver.
 */
typedef enum  {

    ADF4030_0 = 0x00,
    ADF4030_1 = 0x01,
    ADF4030_2 = 0x02,
    ADF4030_3 = 0x03,
    ADF4030_4 = 0x04,
    ADF4030_5 = 0x05,
    ADF4030_6 = 0x06,
    ADF4030_7 = 0x07,
    ADF4030_8 = 0x08,
    ADF4030_9 = 0x09,
    ADF4030_A = 0x0A,
    ADF4030_B = 0x0B,
    ADF4030_C = 0x0C,
    ADF4030_D = 0x0D,
    ADF4030_E = 0x0E,
    ADF4030_F = 0x0F,
} adi_adf4030_dev_id_e;

/**
 * \enum    adi_adf4030_channel_id_e
 * \brief   ID of ADF4030 Bsync channels.
 */
typedef enum  {

    ADI_ADF4030_CHANNEL_ID_0 = 0x00,
    ADI_ADF4030_CHANNEL_ID_1 = 0x01,
    ADI_ADF4030_CHANNEL_ID_2 = 0x02,
    ADI_ADF4030_CHANNEL_ID_3 = 0x03,
    ADI_ADF4030_CHANNEL_ID_4 = 0x04,
    ADI_ADF4030_CHANNEL_ID_5 = 0x05,
    ADI_ADF4030_CHANNEL_ID_6 = 0x06,
    ADI_ADF4030_CHANNEL_ID_7 = 0x07,
    ADI_ADF4030_CHANNEL_ID_8 = 0x08,
    ADI_ADF4030_CHANNEL_ID_9 = 0x09,
    ADI_ADF4030_CHANNEL_ID_NUM = 0x0A
} adi_adf4030_channel_id_e;


#ifndef CLIENT_IGNORE

typedef int32_t(*adi_adf4030_spi_write_t)(void *user_data, const uint8_t *in_data, uint32_t size_bytes, adi_cms_hal_txn_config_t *txn_config);
typedef int32_t(*adi_adf4030_spi_read_t)(void *user_data, const uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes, adi_cms_hal_txn_config_t *txn_config);
typedef int32_t(*adi_adf4030_delay_us_t)(void *user_data, uint32_t us);


/*!
 * \brief ADF4030 registers access descriptors
 */
typedef struct {
    void                         *user_data;
    adi_adf4030_spi_write_t       spi_write;            /*!< Function Pointer to HAL SPI write function */
    adi_adf4030_spi_read_t        spi_read;             /*!< Function Pointer to HAL SPI read function */
    adi_adf4030_delay_us_t        delay_us;             /*!< Function Pointer to HAL delay function */
} adi_adf4030_hal_t;

/*!
 * \brief Device Structure
 */
typedef struct {
    adi_adf4030_hal_t       hal_info;
    adi_adf4030_dev_id_e    dev_id;                       /*!< Unique address of device controlled by address pins on the board */
} adi_adf4030_device_t;

#endif  // !CLIENT_IGNORE

#endif // !__ADI_ADF4030_TYPES_H__

/*! @} */
