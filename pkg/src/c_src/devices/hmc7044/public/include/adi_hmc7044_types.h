/*!
 * \brief     HMC7044 Types
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_TYPES_H__
#define __ADI_HMC7044_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include "adi_cms_api_common.h"

#define ADI_HCM7044_OSCIN_COUNT                     1
#define ADI_HMC7044_CLKIN_COUNT                     4
#define ADI_HMC7044_INT_VCO_COUNT                   2
#define ADI_HMC7044_CLKOUT_COUNT                    14
#define ADI_HMC7044_2GHZ_VCO_HZ_MIN                 2150000000ull
#define ADI_HMC7044_2GHZ_VCO_HZ_MAX                 2880000000ull
#define ADI_HMC7044_3GHZ_VCO_HZ_MIN                 2650000000ull
#define ADI_HMC7044_3GHZ_VCO_HZ_MAX                 3550000000ull

#define ADI_HMC7044_PTR_CHECK(ptr)                  if ((ptr) == ADI_INVALID_POINTER) return API_CMS_ERROR_INVALID_HANDLE_PTR
#define ADI_HMC7044_CHECK_ERR_OK(err)               if ((err) != API_CMS_ERROR_OK) return (err)
#define ADI_HMC7044_VALIDATE_RANGE(value, min, max) if ((value) < (min) || (value) > (max)) return API_CMS_ERROR_INVALID_PARAM
#define ADI_HMC7044_NULL_CHECK(val)                 if ((val) == NULL) return API_CMS_ERROR_NULL_PARAM

typedef enum {
    ADI_HMC7044_CLKOUT0    = 0x0001, /*  CLKOUT0  */
    ADI_HMC7044_SCLKOUT1   = 0x0002, /* SCLKOUT1  */
    ADI_HMC7044_CLKOUT2    = 0x0004, /*  CLKOUT2  */
    ADI_HMC7044_SCLKOUT3   = 0x0008, /* SCLKOUT3  */
    ADI_HMC7044_CLKOUT4    = 0x0010, /*  CLKOUT4  */
    ADI_HMC7044_SCLKOUT5   = 0x0020, /* SCLKOUT5  */
    ADI_HMC7044_CLKOUT6    = 0x0040, /*  CLKOUT6  */
    ADI_HMC7044_SCLKOUT7   = 0x0080, /* SCLKOUT7  */
    ADI_HMC7044_CLKOUT8    = 0x0100, /*  CLKOUT8  */
    ADI_HMC7044_SCLKOUT9   = 0x0200, /* SCLKOUT9  */
    ADI_HMC7044_CLKOUT10   = 0x0400, /*  CLKOUT10 */
    ADI_HMC7044_SCLKOUT11  = 0x0800, /* SCLKOUT11 */
    ADI_HMC7044_CLKOUT12   = 0x1000, /*  CLKOUT12 */
    ADI_HMC7044_SCLKOUT13  = 0x2000, /* SCLKOUT13 */
    ADI_HMC7044_OUTPUT_ALL = 0x3FFF
} adi_hmc7044_clkout_e;

typedef enum {
    ADI_HMC7044_CLKIN_NONE = 0x0,
    ADI_HMC7044_CLKIN0     = 0x1,
    ADI_HMC7044_CLKIN1     = 0x2,
    ADI_HMC7044_CLKIN2     = 0x4,
    ADI_HMC7044_CLKIN3     = 0x8,
    ADI_HMC7044_CLKIN_ALL  = 0XF
} adi_hmc7044_clkin_e;

typedef enum
{
    ADI_HMC7044_HMC7044_INT_VCO_SEL_DISABLED      = 0, /*!< Internal VCO Disabled / External VCO */
    ADI_HMC7044_HMC7044_INT_VCO_SEL_3GHZ          = 1, /*!< High VCO */
    ADI_HMC7044_HMC7044_INT_VCO_SEL_2GHZ          = 2  /*!< Low VCO */
} adi_hmc7044_int_vco_sel_e;

typedef enum {
    ADI_HMC7044_CLK_DIST_PRIORITY_POWER = 0,
    ADI_HMC7044_CLK_DIST_PRIORITY_NOISE = 1
} adi_hmc7044_clk_dist_priority_e;

#ifndef CLIENT_IGNORE

typedef int32_t(*adi_hmc7044_reset_pin_ctrl_t)(void *user_data, uint8_t enable);

/*!
 * \brief HMC7044 registers access descriptors
 */
typedef struct {
    void *                       user_data;
    adi_cms_spi_write_t          spi_write;            /*!< Function Pointer to HAL SPI write function */
    adi_cms_spi_read_t           spi_read;             /*!< Function Pointer to HAL SPI read function */
    adi_cms_delay_us_t           delay_us;             /*!< Function Pointer to HAL delay function */
    adi_hmc7044_reset_pin_ctrl_t reset_pin_ctrl;       /*!< Function Pointer to HAL RESETB Pin Control Function */
} adi_hmc7044_hal_t;

/*!
 * \brief Device Structure
 */
typedef struct {
    adi_hmc7044_hal_t hal_info;
} adi_hmc7044_device_t;

#endif

#endif // !__ADI_ADF4382_TYPES_H__
