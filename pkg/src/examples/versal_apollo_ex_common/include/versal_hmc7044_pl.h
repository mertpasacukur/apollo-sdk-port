/*!
 * @brief     HMC7044 Custom PL IP — 3-Wire SPI Interface
 *
 *            Since the Xilinx XSpi driver does not support 3-wire SPI
 *            required by HMC7044, a custom PL IP is used for communication.
 *            These functions provide the register read/write interface
 *            to that custom IP.
 *
 *            Paşa fills in the implementation with his PL IP driver code.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef VERSAL_HMC7044_PL_H
#define VERSAL_HMC7044_PL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief  Write a register on HMC7044 via custom PL IP (3-wire SPI).
 *
 * @param[in] address  Register address (16-bit, HMC7044 register space)
 * @param[in] data     Value to write (8-bit)
 *
 * @return 0 on success, non-zero on error
 */
int32_t hmcWrite(uint16_t address, uint8_t data);

/*!
 * @brief  Read a register from HMC7044 via custom PL IP (3-wire SPI).
 *
 * @param[in]  address  Register address (16-bit, HMC7044 register space)
 * @param[out] data     Pointer to store the read value (8-bit)
 *
 * @return 0 on success, non-zero on error
 */
int32_t hmcRead(uint16_t address, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* VERSAL_HMC7044_PL_H */
