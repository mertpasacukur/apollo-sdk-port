/*!
 * @brief     Versal FPGA access implementation.
 *
 *            FPGA register access uses Xil_In32/Xil_Out32 (in versal_hal.c).
 *            DMA memory read/write are TODO stubs — not needed initially.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#include "versal_fpga.h"
#include "versal_hal.h"
#include "xil_printf.h"

/*
 * Note: versal_axi_reg_read32 / versal_axi_reg_write32 are implemented
 * in versal_hal.c using Xil_In32/Xil_Out32 with VERSAL_FPGA_REG_BASE_ADDR.
 *
 * versal_fpga_mem_read / versal_fpga_mem_write are also in versal_hal.c
 * as TODO stubs. This file is kept for compatibility with the ADS10 build
 * structure that has a separate ads10_fpga.c.
 */
