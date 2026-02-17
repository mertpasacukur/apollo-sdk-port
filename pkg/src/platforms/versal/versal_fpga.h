/*!
 * @brief     Versal FPGA access header — matches original fpga interface.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef __VERSAL_FPGA_H__
#define __VERSAL_FPGA_H__

#include <stdint.h>
#include "adi_apollo_common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Read data from FPGA memory (DMA).
 *          TODO: Not implemented — DMA not needed initially.
 */
int32_t versal_fpga_mem_read(uint32_t mem_addr, uint32_t num_cap_bytes, uint8_t *cap_buff);

/**
 * @brief   Write data to FPGA memory (DMA).
 *          TODO: Not implemented — DMA not needed initially.
 */
int32_t versal_fpga_mem_write(uint32_t mem_addr, uint32_t num_bytes, uint8_t *raw_bytes);

#ifdef __cplusplus
}
#endif

#endif /* __VERSAL_FPGA_H__ */
