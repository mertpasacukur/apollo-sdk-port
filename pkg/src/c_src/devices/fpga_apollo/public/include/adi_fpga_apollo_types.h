/*!
 * \brief     Basic FPGA Apollo types
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_TYPES_H__
#define __ADI_FPGA_APOLLO_TYPES_H__

#include "adi_fpga_apollo_common_types.h"
#include "adi_fpga_apollo_capture_types.h"
#include "adi_fpga_apollo_core_types.h"
#include "adi_fpga_apollo_gpio_types.h"
#include "adi_fpga_apollo_clk_types.h"

#ifndef CLIENT_IGNORE

typedef int32_t(*adi_apollo_fpga_reg_read_t)(uint32_t reg_offset, uint32_t *out_data);
typedef int32_t(*adi_apollo_fpga_reg_write_t)(uint32_t reg_offset, uint32_t data);
typedef int32_t(*adi_apollo_fpga_mem_read_t)(uint32_t address, uint32_t num_bytes, uint8_t *data);
typedef int32_t(*adi_apollo_fpga_mem_write_t)(uint32_t address, uint32_t num_bytes, uint8_t *data);
typedef int32_t(*adi_apollo_fpga_delay_us_t)(void *user_data, uint32_t us);
typedef int32_t(*adi_apollo_fpga_log_write_t)(void *dev_obj, int32_t log_type, const char *message, va_list argp);


/*!
 * \brief FPGA registers access descriptors
 */
typedef struct {
    adi_apollo_fpga_reg_read_t  reg_read; /*!< Platform FPGA register read function ptr. \ref adi_apollo_fpga_reg_read_t */
    adi_apollo_fpga_reg_write_t reg_write; /*!< Platform PFGA register write function ptr. \ref adi_apollo_fpga_reg_write_t */
    adi_apollo_fpga_mem_read_t  mem_read; /*!< Platform FPGA memory read function ptr. \ref adi_apollo_fpga_mem_read_t */
    adi_apollo_fpga_mem_write_t mem_write; /*!< Platform FPGA memory write function ptr. \ref adi_apollo_fpga_mem_write_t */
    adi_apollo_fpga_delay_us_t  delay_us; /*!< Platform us delay */
    adi_apollo_fpga_log_write_t log_write; /*!< Platform FPGA log write function (optional) */
} adi_apollo_fpga_hal_t;

/*!
 * \brief Device State Structure
 */
typedef struct {
	adi_fpga_design_identifier_e design_id;									/*!< FPGA Design Identifier */
	adi_fpga_jesd_param_t jrx[MAX_JESD_LINKS];								/*!< FPGA JRx (Cap data from Apollo ADC) */
	adi_fpga_jesd_param_t jtx[MAX_JESD_LINKS];								/*!< FPGA JTx (Vec data to   Apollo DAC) */
	adi_fpga_sr_fsrc_param_t rx[MAX_JESD_LINKS];
	adi_fpga_sr_fsrc_param_t tx[MAX_JESD_LINKS];
	uint32_t jtx_link_count;
	uint32_t jrx_link_count;
	adi_fpga_clk_info_t clk_info;
	adi_fpga_apollo_capture_t capture_info;					/*!< Capture raw framing info for reading back capture memory */
} adi_fpga_apollo_state_t;

/*!
 * \brief Device Structure
 */
typedef struct {
    adi_apollo_fpga_hal_t hal_info;
    adi_fpga_apollo_state_t state_info;
} adi_fpga_apollo_device_t;

#endif // !CLIENT_IGNORE

/*!
 * \brief Apollo FPGA Version Structure
 */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} adi_fpga_apollo_version_t;

#endif // !__ADI_FPGA_APOLLO_TYPES_H__
