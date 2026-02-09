
#ifndef __ADS10_FPGA__

/*============= I N C L U D E S ============*/
#include <unistd.h>
#include <stdint.h>
#include "adi_apollo_common_types.h"

typedef enum {
    START = 0,
    NEXT = 1,
    END = 2
} adi_fpga_data_xfer_e;

typedef struct {
    uint32_t address;     /* HMC Memory Address */
    uint32_t filetype;    /* 0x1: data vector, 0x2: FPGA image */
    uint32_t filesize;    /* The size of file for downloading */
    uint32_t filenameLen; /* The length of file name, only used for FPGA image; For data vector, keep it as zero */
    uint8_t filename[64]; /* The file name of FPGA image, only used for FPGA image */
}adi_fpga_xfer_meta_t;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief       API to capture FIFO data
 *
 * \param[in]   mem_addr        FPGA HBM address where to read captured data
 *				from. Not currently used as this address is
 *				0x00000 and not configurable inside FPGA.
 * \param[in]   num_cap_bytes   No. of bytes to capture
 * \param[in]   cap_buff        Byte array to store capture data
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t  ads10_fpga_mem_read(uint32_t mem_addr, uint32_t num_cap_bytes,
			     uint8_t *cap_buff);

/**
 * \brief       API to write pattern to the FPGA data memory
 *
 * \param[in]   mem_addr        FPGA HBM address where to load the pattern.
 * \param[in]   num_bytes       No. of bytes to transmit
 * \param[in]   raw_bytes       Byte array of transmit data
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t ads10_fpga_mem_write(uint32_t mem_addr, uint32_t num_bytes,
			     uint8_t *raw_bytes);

/**
 * \brief       API to write pattern to FIFO data memory in one-shot, not chunked
 *
 * \param[in]   mem_addr        FIFO address
 * \param[in]   num_bytes       No. of bytes to transmit (MAX 20M)
 * \param[in]   raw_bytes       Byte array of transmit data
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t ads10_fpga_mem_write_alt(uint32_t mem_addr, uint32_t num_bytes,
                                        uint8_t *raw_bytes);

/**
 * \brief       Configures FPGA GPIOs to enable or disable corresponding level shifters on FMCB
 *
 * \param[in]   user_data       FPGA device or any user context for GPIO control
 * \param[in]   gpio_index      GPIO to toggle
 * \param[in]   n_gpio_state    GPIO value to set (logical NOT)
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t ads10_fpga_fmcb_aux_gpio_write(void* user_data, uint32_t gpio_index, uint8_t n_gpio_state);

/**
 * \brief       Sets FMCB SDO to FPGA SPI2 SDO
 *
 * \param[in]   user_data       FPGA context
 * \param[in]   dummy_var       Dummy Variable to match hal_spi_sdo_en function pointer
 * \param[in]   value           0 : SDO pin set to SPI2 SDO  1 : SDO pin set to AUX_GPIO_3
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t ads10_fpga_fmcb_aux_spi2_sdo_alt_enable_set(void* user_data, uint32_t dummy_var, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* #define __ADS10_FPGA__ */
